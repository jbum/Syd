# CSyd 2.0
#
# -ffp-contract=off keeps the compiler from fusing multiply-adds, which would round
# differently from 1.5.1 (tests/check_vs_151.py) and from one optimisation to the next.
# STK support (ssstk.cpp) needs -DUSE_STK and the STK headers.

CXX      ?= c++
CXXFLAGS ?= -O2
CXXFLAGS += -ffp-contract=off -Isrc -I. -Wno-write-strings -Wno-deprecated
SRCS     := $(filter-out src/ssstk.cpp,$(wildcard src/*.cpp))
OBJS     := $(SRCS:src/%.cpp=obj/%.o)

csyd: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) -lm

obj/%.o: src/%.cpp $(wildcard src/*.h) sydlocal.h | obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj:
	mkdir -p obj

clean:
	rm -rf obj csyd

.PHONY: clean
