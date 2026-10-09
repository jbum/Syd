#!/bin/sh
# Build CSyd for WebAssembly with Emscripten (source ~/emsdk/emsdk_env.sh first).
#   build/build_wasm.sh node   csyd.js for Node, reading and writing the host's files (tests)
#   build/build_wasm.sh web    csyd.js + csyd.wasm for the browser (web/), files in memory
# Single-threaded (SYD_NO_THREADS): threads in a browser need cross-origin isolation.
set -e
cd "$(dirname "$0")/.."
target=${1:-web}
out=build/wasm-$target
mkdir -p "$out/obj"
CFLAGS="-O2 -ffp-contract=off"
for f in src/smath/*.c; do
  emcc $CFLAGS -std=c99 -Isrc/smath -include smath_names.h -Wno-unused-but-set-variable \
       -c -o "$out/obj/$(basename "$f" .c).o" "$f"
done
srcs=$(ls src/*.cpp | grep -v ssstk.cpp)
common="$CFLAGS -DSYD_NO_THREADS -Isrc -I. -Wno-register -Wno-write-strings -Wno-deprecated -w
        -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=4GB -sSTACK_SIZE=4MB"
if [ "$target" = node ]; then
  em++ $common $srcs "$out"/obj/*.o -o "$out/csyd.js" -sENVIRONMENT=node -sNODERAWFS=1
else
  em++ $common $srcs "$out"/obj/*.o -o "$out/csyd.js" -sENVIRONMENT=web,worker \
       -sMODULARIZE=1 -sEXPORT_NAME=createCSyd -sINVOKE_RUN=0 -sFORCE_FILESYSTEM=1 \
       -sEXPORTED_RUNTIME_METHODS=FS,callMain
fi
ls -la "$out"/csyd.*
