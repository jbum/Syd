#!/usr/bin/env python3
"""Check CSyd 2.0 against 1.5.2, byte for byte, and time both.

  tests/compare.py --make-ref      build ../csyd_152 and render tests/ref/ with it (once)
  tests/compare.py [-k WORD]       render tests/out/ with ./csyd and compare

2.0 is a speed-up, so every patch in tests/corpus.txt must render exactly as 1.5.2 does.
Every render uses SYD_SEED so randomized noise is reproducible. The reference's times are
recorded when it is made (tests/ref/times.json), so a compare needn't re-run it.
"""
import argparse, json, os, subprocess, sys, time

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
    fails, tr, tn = [], 0.0, 0.0
    for path, secs in corpus(a.k):
        out, dt = render(os.path.join(HERE, "..", "csyd"), path, secs, outdir)
        same = open(out, "rb").read() == open(os.path.join(refdir, os.path.basename(out)), "rb").read()
        rt = ref_times[path]
        tr, tn = tr + rt, tn + dt
        print(f"{'ok  ' if same else 'DIFF'} {rt:8.2f}s -> {dt:8.2f}s  x{rt / max(dt, 1e-6):5.1f}  {path}")
        if not same:
            fails.append(path)
    print(f"\n{len(fails)} differ; total {tr:.1f}s -> {tn:.1f}s (x{tr / max(tn, 1e-6):.2f})")
    sys.exit(1 if fails else 0)


main()
