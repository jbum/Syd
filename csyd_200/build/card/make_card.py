#!/usr/bin/env python3
"""Make the social-media card, web/assets/card.png (1200x630): the Shepard patch as the
editor draws it, beside a spectrogram of what it sounds like.  Needs ./csyd built,
numpy and Pillow, and Google Chrome (rendered headless).  usage: build/card/make_card.py"""
import json, os, subprocess, tempfile, wave
import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.abspath(os.path.join(HERE, "..", ".."))
ROOT = os.path.dirname(SRC)
PATCH = os.path.join(ROOT, "csyd_102", "patches", "shepard.syd")
OUT = os.path.join(SRC, "web", "assets", "card.png")
CHROME = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
W, H = 1200, 630
SPEC_W = 760                          # the spectrogram fills the card's right side

tmp = tempfile.mkdtemp(prefix="sydcard-")

# 24 s of the patch: three turns of its 8 s cycle
wav = os.path.join(tmp, "shepard.wav")
subprocess.run([os.path.join(SRC, "csyd"), "-q", "-r", "7", "-d", "24", "-o", wav, os.path.basename(PATCH)],
               cwd=os.path.dirname(PATCH), check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
w = wave.open(wav)
sr = w.getframerate()
x = np.frombuffer(w.readframes(w.getnframes()), dtype=np.int16).astype(float) / 32768

# Spectrogram on a log-frequency axis (the oscillators sweep 60-240 Hz), coloured from
# the desk through the site's teal to the pale yellow of JSyd's oscillator icon
n, hop = 8192, 128
win = np.hanning(n)
S = np.array([np.abs(np.fft.rfft(x[i:i + n] * win)) for i in range(0, len(x) - n, hop)]).T
freqs = np.fft.rfftfreq(n, 1 / sr)
f = np.geomspace(52, 280, H)
img = np.array([np.interp(f, freqs, col) for col in S.T]).T[::-1]
level = np.clip((20 * np.log10(img / img.max() + 1e-9) + 55) / 55, 0, 1)
stops = np.array([[0.0, 0x1d, 0x1c, 0x1a], [0.45, 0x1f, 0x4d, 0x42], [0.75, 0x5f, 0xb3, 0x9a], [1.0, 0xf4, 0xf1, 0xb0]])
rgb = np.stack([np.interp(level, stops[:, 0], stops[:, c]) for c in (1, 2, 3)], axis=-1).astype(np.uint8)
Image.fromarray(rgb).resize((SPEC_W, H), Image.LANCZOS).save(os.path.join(tmp, "spec.png"))

patch_text = open(PATCH, "rb").read().decode("latin-1")
web = os.path.join(SRC, "web")
html = """<!doctype html><html><head><meta charset="utf-8"><style>
  html, body { margin: 0; width: %(W)dpx; height: %(H)dpx; overflow: hidden; background: #1d1c1a; }
  body { position: relative; font-family: -apple-system, "Helvetica Neue", sans-serif; color: #ece8e1; }
  #spec { position: absolute; right: 0; top: 0; width: %(SPEC_W)dpx; height: %(H)dpx;
    -webkit-mask-image: linear-gradient(to right, transparent 0, #000 260px); }
  #text { position: absolute; left: 64px; top: 50px; }
  h1 { font-size: 104px; line-height: 1; margin: 0; letter-spacing: -0.03em; font-weight: 800; }
  p { font-size: 33px; line-height: 1.25; margin: 14px 0 0; color: #d8d3ca; max-width: 520px; font-weight: 500; }
  #patch { position: absolute; left: 40px; top: 268px; }
  #cap { position: absolute; left: 64px; bottom: 44px; font-size: 22px; color: #a29c91; }
  #url { position: absolute; right: 48px; bottom: 40px; font-size: 30px; font-weight: 700; color: #f4f1b0;
    text-shadow: 0 1px 8px rgba(0,0,0,.6); }
</style></head><body>
<img id="spec" src="spec.png">
<div id="text"><h1>Syd</h1><p>Patch together a synthesizer and hear it, right in your browser.</p></div>
<svg id="patch" width="560" height="290" xmlns="http://www.w3.org/2000/svg"></svg>
<div id="cap">The Shepard&ndash;Risset patch, and its endlessly falling tones</div>
<div id="url">syd.jbum.com</div>
<script src="file://%(web)s/syd.js"></script>
<script>
const ASSETS = "file://%(web)s/assets/";
const p = Syd.parse(%(patch)s);
const svg = document.getElementById("patch"), NS = "http://www.w3.org/2000/svg", NODE = 40, S = 0.97;
const el = (n, a, par) => { const e = document.createElementNS(NS, n); for (const k in a) e.setAttribute(k, a[k]); (par || svg).appendChild(e); return e; };
let x0 = 1e9, y0 = 1e9;
for (const m of p.modules) { x0 = Math.min(x0, m.x); y0 = Math.min(y0, m.y); }
const g = el("g", { transform: `scale(${S}) translate(${20 - x0},${16 - y0})` });
const by = (l) => p.modules.find((m) => m.label === l);
for (const d of p.modules) for (const i of d.inputs) {
  const s = by(i.from), across = !(d.x < s.x + NODE);
  const [a, b, c, e] = across ? [s.x + NODE - 8, s.y + NODE / 2, d.x + 8, d.y + NODE / 2]
                              : [s.x + NODE / 2, s.y + NODE - 8, d.x + NODE / 2, d.y + 8];
  el("path", { d: `M${a},${b}L${c},${e}`, stroke: "#000", "stroke-width": 3, opacity: .55, transform: "translate(1,1)" }, g);
  el("path", { d: `M${a},${b}L${c},${e}`, stroke: Syd.linkColor(i.link), "stroke-width": 3, "stroke-linecap": "round" }, g);
}
for (const m of p.modules) {
  const t = Syd.TYPES[m.type];
  const back = el("svg", { x: m.x, y: m.y, width: NODE, height: NODE, viewBox: `${t.icon * 64} 0 64 64` }, g);
  el("image", { href: ASSETS + "iconbacks.png", width: 256, height: 128 }, back);
  const st = el("svg", { x: m.x, y: m.y, width: NODE, height: NODE, viewBox: `${t.stamp * 64} 0 64 64` }, g);
  el("image", { href: ASSETS + "unitlabels.png", width: 1664, height: 64 }, st);
}
</script></body></html>""" % {"W": W, "H": H, "SPEC_W": SPEC_W, "web": web, "patch": json.dumps(patch_text)}
page = os.path.join(tmp, "card.html")
open(page, "w").write(html)

subprocess.run([CHROME, "--headless=new", "--disable-gpu", "--hide-scrollbars", "--allow-file-access-from-files",
                f"--window-size={W},{H}", "--force-device-scale-factor=1", "--virtual-time-budget=3000",
                f"--screenshot={OUT}", "file://" + page], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
print("wrote", OUT, Image.open(OUT).size)
