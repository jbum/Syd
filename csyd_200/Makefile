# CSyd 2.0
#
# -ffp-contract=off keeps the compiler from fusing multiply-adds, so a render rounds the
# same on every platform and at every optimisation level; src/smath (CSyd's own copy of
# musl's math) does the same for pow, sin and the rest. STK support (ssstk.cpp) needs
# -DUSE_STK and the STK headers.

CXX      ?= c++
CXXFLAGS ?= -O2
CXXFLAGS += -ffp-contract=off -Isrc -I. -Wno-write-strings -Wno-deprecated
SRCS     := $(filter-out src/ssstk.cpp,$(wildcard src/*.cpp))
MSRCS    := $(wildcard src/smath/*.c)
OBJS     := $(SRCS:src/%.cpp=obj/%.o) $(MSRCS:src/smath/%.c=obj/smath/%.o)
CC       ?= cc
MFLAGS   := -O2 -std=c99 -ffp-contract=off -Isrc/smath -include smath_names.h \
            -Wno-unused-but-set-variable

csyd: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) -lm

obj/%.o: src/%.cpp $(wildcard src/*.h) sydlocal.h | obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/smath/%.o: src/smath/%.c $(wildcard src/smath/*.h) | obj
	$(CC) $(MFLAGS) -c -o $@ $<

obj:
	mkdir -p obj/smath

clean:
	rm -rf obj csyd

.PHONY: clean
