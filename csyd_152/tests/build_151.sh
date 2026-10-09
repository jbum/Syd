#!/bin/sh
# Build CSyd 1.5.1 (tests/bin/csyd151) from ../csyd_102/src, unmodified
# except that the clock seed in Randomize() honours SYD_SEED, as 1.5.2's -r does, so patches
# with randomized noise render reproducibly. Same flags as the 1.5.2 Makefile.
set -e
here=$(cd "$(dirname "$0")" && pwd)
tmp=$(mktemp -d)
mkdir "$tmp"/src
cp "$here"/../../csyd_102/src/*.cpp "$here"/../../csyd_102/src/*.h "$tmp"/src/
cp "$here"/../../csyd_102/sydlocal.h "$tmp"/
rm -f "$tmp"/src/ssstk.cpp
sed -i '' 's/time(NULL)/(getenv("SYD_SEED")? atoi(getenv("SYD_SEED")) : time(NULL))/' "$tmp"/src/ssutils.cpp "$tmp"/src/ssnoise.cpp
mkdir -p "$here"/bin
c++ -O2 -ffp-contract=off -include stdlib.h -I"$tmp"/src -w -o "$here"/bin/csyd151 "$tmp"/src/*.cpp -lm
rm -rf "$tmp"
echo "built $here/bin/csyd151"
