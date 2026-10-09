#!/usr/bin/env python3
"""Check the WebAssembly build renders the corpus exactly as the native one.

  build/build_wasm.sh node && tests/compare_wasm.py [-k WORD]

Both use CSyd's own math (src/smath), so they should agree to the bit; the WebAssembly build
is single-threaded, which block rendering makes no difference to.
"""
import argparse, os, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
NATIVE = os.path.join(HERE, "..", "csyd")
WASM = os.path.join(HERE, "..", "build", "wasm-node", "csyd.js")


def render(cmd, path, secs, out):
    t = time.perf_counter()
    subprocess.run(cmd + ["-q", "-r", "12345", "-d", secs, "-o", out, os.path.basename(path)],
                   cwd=os.path.join(ROOT, os.path.dirname(path)), capture_output=True, check=True)
    return open(out, "rb").read(), time.perf_counter() - t


ap = argparse.ArgumentParser()
ap.add_argument("-k")
a = ap.parse_args()
os.makedirs(os.path.join(HERE, "out"), exist_ok=True)
bad, tn, tw = [], 0.0, 0.0
for line in open(os.path.join(HERE, "corpus.txt")):
    line = line.strip()
    if not line or line.startswith("#"):
        continue
    path, secs = line.rsplit(" ", 1)
    if a.k and a.k.lower() not in path.lower():
        continue
    native, dn = render([NATIVE], path, secs, os.path.join(HERE, "out", "native.wav"))
    wasm, dw = render(["node", WASM], path, secs, os.path.join(HERE, "out", "wasm.wav"))
    tn, tw = tn + dn, tw + dw
    same = native == wasm
    print(f"{'same' if same else 'DIFF'}  native {dn:6.2f}s  wasm {dw:6.2f}s  {path}")
    if not same:
        bad.append(path)
print(f"\n{len(bad)} differ; native {tn:.1f}s, wasm {tw:.1f}s")
sys.exit(1 if bad else 0)
