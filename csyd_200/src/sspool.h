// sspool.h: run work across the CPU's cores.
#ifndef _H_SSPool
#define _H_SSPool	1

#include <functional>

extern int gThreads;	// threads ParallelFor uses, the caller's included; 1: none

// Call fn(i) for every i in [0, n), on gThreads threads, and return when all have returned.
// Each thread takes the next i as it finishes one, so uneven work balances out. Called from
// inside fn, it runs serially.
void ParallelFor(int n, const std::function<void(int)> &fn);

#endif
