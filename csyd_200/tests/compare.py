#!/usr/bin/env python3
"""Check CSyd 2.0 against 1.5.2 and time both.

  tests/compare.py --make-ref      build ../csyd_152 and render tests/ref/ with it (once)
  tests/compare.py [-k WORD]       render tests/out/ with ./csyd and compare

2.0 is a speed-up and must sound the same as 1.5.2, not be bit-identical to it: its own math
routines and a different order of evaluation can move a sample by a rounding step. So each
render is graded:

  exact   byte-identical
  ok      inaudible: every sample within MAX_LSB of 1.5.2's, or the difference more than
          MIN_SNR dB below the signal
  CHECK   anything else; listen, or find out why

Every render uses SYD_SEED so randomized noise is reproducible. The reference's times are
recorded when it is made (tests/ref/times.json), so a compare needn't re-run it.
"""
import argparse, json, os, subprocess, sys, time

import numpy as np

MAX_LSB = 4      # 16-bit steps
MIN_SNR = 90.0   # dB

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))   # the repo: patch paths are relative to it
REF_DIR = os.path.join(ROOT, "csyd_152")


def corpus(word):
    for line in open(os.path.join(HERE, "corpus.txt")):
        line = line.strip()
        if line and not line.startswith("#"):
            path, secs = line.rsplit(" ", 1)
            if not word or word.lower() in path.lower():
                yield path, secs


def render(binary, path, secs, outdir):
    out = os.path.join(outdir, path.replace("/", "__").replace(".syd", ".wav"))
    t = time.perf_counter()
    r = subprocess.run([binary, "-q", "-d", secs, "-o", out, os.path.basename(path)],
                       cwd=os.path.join(ROOT, os.path.dirname(path)), capture_output=True,
                       env={**os.environ, "SYD_SEED": "12345"})
    dt = time.perf_counter() - t
    if r.returncode != 0 or not os.path.exists(out):
        sys.exit(f"{binary} failed on {path}:\n{(r.stdout + r.stderr).decode(errors='replace')}")
    return out, dt


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--make-ref", action="store_true")
    ap.add_argument("-k", help="only patches whose path contains this")
    a = ap.parse_args()
    refdir, outdir = os.path.join(HERE, "ref"), os.path.join(HERE, "out")
    times_path = os.path.join(refdir, "times.json")
    if a.make_ref:
        subprocess.run(["make", "-s", "-C", REF_DIR], check=True)
        os.makedirs(refdir, exist_ok=True)
        times = json.load(open(times_path)) if os.path.exists(times_path) else {}
        for path, secs in corpus(a.k):
            _, times[path] = render(os.path.join(REF_DIR, "csyd"), path, secs, refdir)
            print(f"{times[path]:8.2f}s  {path}")
        json.dump(times, open(times_path, "w"), indent=1)
        return
    os.makedirs(outdir, exist_ok=True)
    ref_times = json.load(open(times_path))
    grades, checks, tr, tn = {}, [], 0.0, 0.0
    for path, secs in corpus(a.k):
        out, dt = render(os.path.join(HERE, "..", "csyd"), path, secs, outdir)
        grade, detail = grade_render(out, os.path.join(refdir, os.path.basename(out)))
        rt = ref_times[path]
        tr, tn = tr + rt, tn + dt
        grades[grade] = grades.get(grade, 0) + 1
        print(f"{grade:5} {rt:8.2f}s -> {dt:8.2f}s  x{rt / max(dt, 1e-6):5.1f}  {path}  {detail}")
        if grade == "CHECK":
            checks.append(path)
    summary = ", ".join(f"{n} {g}" for g, n in sorted(grades.items()))
    print(f"\n{summary}; total {tr:.1f}s -> {tn:.1f}s (x{tr / max(tn, 1e-6):.2f})")
    sys.exit(1 if checks else 0)


def grade_render(out, ref):
    """(grade, detail) for a render against 1.5.2's; both are 16-bit WAVs with 44-byte headers."""
    a, b = open(out, "rb").read(), open(ref, "rb").read()
    if a == b:
        return "exact", ""
    if a[:44] != b[:44]:
        return "CHECK", "header differs"
    x = np.frombuffer(a[44:], dtype="<i2").astype(np.float64)
    y = np.frombuffer(b[44:], dtype="<i2").astype(np.float64)
    d = x - y
    max_lsb = int(np.abs(d).max())
    noise = np.sqrt(np.mean(d * d))
    signal = np.sqrt(np.mean(y * y))
    snr = 20 * np.log10(signal / noise) if signal > 0 else 0.0
    detail = f"max {max_lsb} LSB, SNR {snr:.0f} dB, {np.count_nonzero(d) / len(d):.1%} of samples"
    return ("ok" if max_lsb <= MAX_LSB or snr >= MIN_SNR else "CHECK"), detail


main()
