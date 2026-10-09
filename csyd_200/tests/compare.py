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
  similar different samples, same sound: where 2.0 draws different random numbers (a note's
          modules have their own when its score renders in blocks), the waveforms differ, but
          the loudness over time and the spectrum differ from 1.5.2's no more than 2.0's own
          renders with other seeds differ from each other (or than ENV_DB and BAND_DB, if that
          is more). Listen to these to be sure.
  CHECK   anything else; listen, or find out why

Every render uses SYD_SEED so randomized noise is reproducible. The reference's times are
recorded when it is made (tests/ref/times.json), so a compare needn't re-run it.
"""
import argparse, json, os, subprocess, sys, time

import numpy as np

MAX_LSB = 4      # 16-bit steps
MIN_SNR = 90.0   # dB
ENV_DB = 1.5     # mean difference of the loudness in 50 ms windows, where either is above -60 dB
BAND_DB = 3.0    # largest difference of the long-term spectrum in third-octave bands

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


def render(binary, path, secs, outdir, seed=12345, name=None):
    out = os.path.join(outdir, name or path.replace("/", "__").replace(".syd", ".wav"))
    t = time.perf_counter()
    r = subprocess.run([binary, "-q", "-d", secs, "-o", out, os.path.basename(path)],
                       cwd=os.path.join(ROOT, os.path.dirname(path)), capture_output=True,
                       env={**os.environ, "SYD_SEED": str(seed)})
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
        grade, detail = grade_render(out, os.path.join(refdir, os.path.basename(out)),
                                     lambda seed: render(os.path.join(HERE, "..", "csyd"), path, secs,
                                                         outdir, seed, "seed.wav")[0])
        rt = ref_times[path]
        tr, tn = tr + rt, tn + dt
        grades[grade] = grades.get(grade, 0) + 1
        print(f"{grade:5} {rt:8.2f}s -> {dt:8.2f}s  x{rt / max(dt, 1e-6):5.1f}  {path}  {detail}")
        if grade == "CHECK":
            checks.append(path)
    summary = ", ".join(f"{n} {g}" for g, n in sorted(grades.items()))
    print(f"\n{summary}; total {tr:.1f}s -> {tn:.1f}s (x{tr / max(tn, 1e-6):.2f})")
    sys.exit(1 if checks else 0)


def grade_render(out, ref, render_seed):
    """(grade, detail) for a render against 1.5.2's; both are 16-bit WAVs with 44-byte headers.
    render_seed(seed) renders the patch again with another seed and returns the file."""
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
    if max_lsb <= MAX_LSB or snr >= MIN_SNR:
        return "ok", detail
    rate, channels = header_format(b)
    env, band = sound_difference(x, y, rate, channels)
    # how much 2.0's renders vary with the seed alone
    env_ok, band_ok = ENV_DB, BAND_DB
    for k in (1, 2, 3):
        z = np.frombuffer(open(render_seed(12345 + 7919 * k), "rb").read()[44:], dtype="<i2")
        e, bd = sound_difference(x, z.astype(np.float64), rate, channels)
        env_ok, band_ok = max(env_ok, e), max(band_ok, bd)
    detail += (f"; loudness {env:.1f} dB, spectrum {band:.1f} dB apart "
               f"(other seeds: up to {env_ok:.1f}, {band_ok:.1f})")
    return ("similar" if env <= env_ok and band <= band_ok else "CHECK"), detail


def header_format(wav):
    """(sample rate, channels) from a canonical 44-byte WAV header."""
    return int.from_bytes(wav[24:28], "little"), int.from_bytes(wav[22:24], "little")


def sound_difference(x, y, rate, channels):
    """How different two renders sound, as (loudness dB, spectrum dB): the mean difference of
    their RMS levels in 50 ms windows where either is above -60 dB, and the largest difference
    of their average spectra in third-octave bands from 50 Hz to 16 kHz (or Nyquist)."""
    x = x.reshape(-1, channels).mean(axis=1)
    y = y.reshape(-1, channels).mean(axis=1)
    win = max(1, int(rate * 0.05))
    n = len(x) // win * win
    def levels(s):
        rms = np.sqrt(np.mean(s[:n].reshape(-1, win) ** 2, axis=1))
        return 20 * np.log10(np.maximum(rms, 1e-9) / 32768)
    lx, ly = levels(x), levels(y)
    loud = (lx > -60) | (ly > -60)
    env = float(np.mean(np.abs(lx - ly)[loud])) if loud.any() else 0.0
    def spectrum(s):
        seg = 4096
        m = len(s) // seg
        if m == 0:
            return None
        frames = s[:m * seg].reshape(m, seg) * np.hanning(seg)
        return np.mean(np.abs(np.fft.rfft(frames, axis=1)) ** 2, axis=0)
    sx, sy = spectrum(x), spectrum(y)
    if sx is None:
        return env, 0.0
    freqs = np.fft.rfftfreq(4096, 1 / rate)
    band, worst, f = 0.0, 0.0, 50.0
    top = min(16000.0, rate / 2)
    total = max(sx.sum(), sy.sum())
    while f * 2 ** (1 / 3) <= top:
        sel = (freqs >= f) & (freqs < f * 2 ** (1 / 3))
        ex, ey = sx[sel].sum(), sy[sel].sum()
        if sel.any() and max(ex, ey) > total * 1e-5:		# bands that carry sound
            worst = max(worst, abs(10 * np.log10(max(ex, 1e-30) / max(ey, 1e-30))))
        f *= 2 ** (1 / 3)
    return env, worst


main()
