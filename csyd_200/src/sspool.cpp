// sspool.cpp

#include "sspool.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

int gThreads = 1;

namespace {

struct Pool {
	std::mutex					m;
	std::condition_variable		start, finished;
	std::vector<std::thread>	workers;
	const std::function<void(int)> *fn = nullptr;
	int							n = 0;
	std::atomic<int>			next{0};
	int							busy = 0;		// workers still on the current job
	unsigned					generation = 0;	// counts jobs, so a worker knows a new one came

	void Work()
	{
		for (int i; (i = next.fetch_add(1)) < n; )
			(*fn)(i);
	}

	void Worker()
	{
		unsigned seen = 0;
		for (;;) {
			std::unique_lock<std::mutex> lock(m);
			start.wait(lock, [&] { return generation != seen; });
			seen = generation;
			lock.unlock();
			Work();
			lock.lock();
			if (--busy == 0)
				finished.notify_one();
		}
	}
};

Pool			*gPool;
thread_local bool tInParallel;

}

void ParallelFor(int n, const std::function<void(int)> &fn)
{
	if (gThreads <= 1 || n <= 1 || tInParallel) {
		for (int i = 0; i < n; ++i)
			fn(i);
		return;
	}
	if (gPool == nullptr) {
		gPool = new Pool;
		for (int t = 1; t < gThreads; ++t) {
			gPool->workers.emplace_back([] { tInParallel = true; gPool->Worker(); });
			gPool->workers.back().detach();
		}
	}
	Pool &p = *gPool;
	{
		std::lock_guard<std::mutex> lock(p.m);
		p.fn = &fn;
		p.n = n;
		p.next = 0;
		p.busy = (int) p.workers.size();
		++p.generation;
	}
	p.start.notify_all();
	tInParallel = true;
	p.Work();
	tInParallel = false;
	std::unique_lock<std::mutex> lock(p.m);
	p.finished.wait(lock, [&] { return p.busy == 0; });
}
