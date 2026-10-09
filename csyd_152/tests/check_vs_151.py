#!/usr/bin/env python3
"""Check that 1.5.2 renders the corpus exactly as 1.5.1 did.

  tests/build_151.sh            build tests/bin/csyd151 (once)
  tests/check_vs_151.py         render the corpus with both and compare the audio

1.5.1 wrote broken headers on 64-bit platforms (8-byte fields: an 88-byte WAV header where
1.5.2 writes the standard 44), so only the sample data is compared. Every render uses SYD_SEED
so randomized noise is reproducible.

Expected: every patch matches except the ones in NONDETERMINISTIC, which 1.5.1 rendered
differently from run to run (an oscillator read before its Reset saw recycled heap memory);
1.5.2 renders them as 1.5.1 renders a fresh instrument, every time.
"""
import os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
NONDETERMINISTIC = {"csyd_102/patches/drones+melody.syd", "jsyd_102/patches/drones+melody.syd",
                    "jsyd_102/JSydBeta/patches/drones_melody.syd"}


def corpus():
    for line in open(os.path.join(HERE, "corpus.txt")):
        line = line.strip()
        if line and not line.startswith("#"):
            yield line.rsplit(" ", 1)


def render(binary, path, secs, out):
    subprocess.run([binary, "-q", "-d", secs, "-o", out, os.path.basename(path)],
                   cwd=os.path.join(ROOT, os.path.dirname(path)), capture_output=True, check=True,
                   env={**os.environ, "SYD_SEED": "12345"})
    return open(out, "rb").read()


def main():
    out = os.path.join(HERE, "out")
    os.makedirs(out, exist_ok=True)
    unexpected = []
    for path, secs in corpus():
        name = path.replace("/", "__").replace(".syd", ".wav")
        old = render(os.path.join(HERE, "bin", "csyd151"), path, secs, os.path.join(out, "151-" + name))[88:]
        new = render(os.path.join(HERE, "..", "csyd"), path, secs, os.path.join(out, "152-" + name))[44:]
        same = old == new
        print(f"{'same' if same else 'DIFF'}{' (expected)' if path in NONDETERMINISTIC else ''}  {path}")
        if same == (path in NONDETERMINISTIC):
            unexpected.append(path)
    print(f"\n{len(unexpected)} unexpected: {unexpected}" if unexpected else "\nall as expected")
    sys.exit(1 if unexpected else 0)


main()
