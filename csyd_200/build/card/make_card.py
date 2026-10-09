#!/usr/bin/env python3
"""Make the social-media card, web/assets/card.png (1200x630), in the neon style of the
Gemini mock-up but from the real thing: the editor itself (build/web, loaded live in
headless Chrome and tilted into the card) with the Shepard patch open and its waveform
in the preview, over neon traces of that patch's own spectrogram and waveform.

Needs ./csyd and build/web built (build/build_web.py), numpy and Pillow, and Google
Chrome.  usage: build/card/make_card.py"""
import functools, http.server, json, os, subprocess, tempfile, threading, wave
import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.abspath(os.path.join(HERE, "..", ".."))
ROOT = os.path.dirname(SRC)
PATCH = os.path.join(ROOT, "csyd_102", "patches", "shepard.syd")
APP = os.path.join(SRC, "build", "web")
OUT = os.path.join(SRC, "web", "assets", "card.png")
CHROME = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
W, H = 1200, 630

if not os.path.exists(os.path.join(APP, "index.html")):
    raise SystemExit("build/web is missing: run build/build_web.py first")
tmp = tempfile.mkdtemp(prefix="sydcard-")
os.symlink(APP, os.path.join(tmp, "app"))


def render(seconds, name):
    path = os.path.join(tmp, name)
    subprocess.run([os.path.join(SRC, "csyd"), "-q", "-r", "7", "-d", str(seconds), "-o", path, os.path.basename(PATCH)],
                   cwd=os.path.dirname(PATCH), check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    w = wave.open(path)
    return w.getframerate(), np.frombuffer(w.readframes(w.getnframes()), dtype=np.int16).astype(float) / 32768


# What the editor's preview shows: the patch's 8 s
render(8, "shepard.wav")

# Neon spectrogram: 24 s (three turns of the patch's cycle) on a log-frequency axis,
# as an RGBA image whose glissandi glow magenta to violet to cyan over transparency
sr, x = render(24, "long.wav")
n, hop = 8192, 128
win = np.hanning(n)
S = np.array([np.abs(np.fft.rfft(x[i:i + n] * win)) for i in range(0, len(x) - n, hop)]).T
freqs = np.fft.rfftfreq(n, 1 / sr)
f = np.geomspace(52, 280, H)
img = np.array([np.interp(f, freqs, col) for col in S.T]).T[::-1]
level = np.clip((20 * np.log10(img / img.max() + 1e-9) + 40) / 40, 0, 1)
hue = np.linspace(0, 1, img.shape[1])[None, :].repeat(H, 0)       # magenta at left, cyan at right
stops = np.array([[0.0, 0xff, 0x3c, 0xd2], [0.5, 0x9b, 0x5c, 0xff], [1.0, 0x3c, 0xe8, 0xff]])
rgba = np.zeros(img.shape + (4,), np.uint8)
for c in range(3):
    rgba[..., c] = np.interp(hue, stops[:, 0], stops[:, c + 1])
rgba[..., 3] = (level ** 1.6 * 255).astype(np.uint8)
Image.fromarray(rgba, "RGBA").resize((W, H), Image.LANCZOS).save(os.path.join(tmp, "spec.png"))

# A scope trace: 60 ms of the waveform, where its four sines beat against each other
start = int(sr * 5.0)
scope = x[start:start + int(sr * 0.06)]
scope = (scope / np.abs(scope).max()).round(4).tolist()

html = """<!doctype html><html><head><meta charset="utf-8"><style>
  html, body { margin: 0; width: %(W)dpx; height: %(H)dpx; overflow: hidden; }
  body { position: relative; background: #05060f; color: #fff;
    font-family: "Avenir Next", "Helvetica Neue", sans-serif; }
  .glow { position: absolute; border-radius: 50%%; filter: blur(70px); }
  #spec { position: absolute; inset: 0; opacity: .55;
    -webkit-mask-image: linear-gradient(to right, transparent 25%%, #000 65%%); }
  #stars { position: absolute; inset: 0; }
  #scope { position: absolute; left: 0; top: 410px;
    -webkit-mask-image: linear-gradient(to right, transparent 34%%, #000 52%%); }
  #logo { position: absolute; left: 60px; top: 26px; font: 124px/1 "Arial Rounded MT Bold", sans-serif;
    color: #dffcff; letter-spacing: 2px;
    text-shadow: 0 0 6px #7ff6ff, 0 0 18px #2fd8ff, 0 0 42px #18a8ff, 0 0 80px #1170ff; }
  #kicker { position: absolute; left: 64px; top: 168px; font-size: 21px; font-weight: 700; letter-spacing: .06em; color: #e9f4ff; }
  #head { position: absolute; left: 60px; top: 206px; font-family: "Avenir Next Condensed"; font-weight: 800;
    font-size: 84px; line-height: .92; letter-spacing: .01em; }
  #head .lime { color: #c6ff3a; text-shadow: 0 0 22px rgba(198,255,58,.45); }
  #sub { position: absolute; left: 64px; top: 474px; font-size: 22px; line-height: 1.3; color: #d6dcf0; font-weight: 500; }
  #cta { position: absolute; left: 64px; top: 538px; padding: 13px 28px; border-radius: 12px; background: #eafffb;
    color: #0a1a2a; font-weight: 800; font-size: 22px; letter-spacing: .03em;
    box-shadow: 0 0 0 2px #7ff6ff, 0 0 24px rgba(80, 230, 255, .7); }
  #url { position: absolute; left: 290px; top: 551px; font-size: 22px; color: #c9d2ea; font-weight: 600; }
  #stage { position: absolute; left: 548px; top: 104px; width: 1280px; height: 800px; perspective: 2400px; }
  #win { width: 1280px; height: 800px; transform-origin: 0 0;
    transform: scale(.49) rotateY(-17deg) rotateX(5deg) rotateZ(-1.5deg); border-radius: 18px; overflow: hidden;
    border: 2px solid rgba(160, 200, 255, .5); background: #171614;
    box-shadow: 0 0 0 1px #000, 0 0 50px rgba(120, 90, 255, .55), 0 0 120px rgba(40, 200, 255, .35), 0 40px 80px rgba(0,0,0,.8); }
  #bar { height: 34px; background: #26252b; display: flex; align-items: center; gap: 8px; padding: 0 14px; }
  #bar i { width: 12px; height: 12px; border-radius: 50%%; display: block; }
  #bar span { margin-left: 160px; flex: 0 0 420px; height: 22px; border-radius: 6px; background: #1a1920;
    color: #a9a6b8; font: 13px/22px -apple-system, sans-serif; text-align: center; }
  iframe { width: 1280px; height: 766px; border: 0; display: block; }
</style></head><body>
<div class="glow" style="left:-120px;top:280px;width:520px;height:420px;background:rgba(170,40,255,.35)"></div>
<div class="glow" style="left:380px;top:-160px;width:620px;height:360px;background:rgba(20,140,255,.30)"></div>
<div class="glow" style="left:820px;top:380px;width:520px;height:360px;background:rgba(255,40,190,.28)"></div>
<img id="spec" src="spec.png">
<canvas id="stars" width="%(W)d" height="%(H)d"></canvas>
<canvas id="scope" width="%(W)d" height="200"></canvas>
<div id="stage"><div id="win">
  <div id="bar"><i style="background:#ff5f57"></i><i style="background:#febc2e"></i><i style="background:#28c840"></i>
    <span>syd.jbum.com</span></div>
  <iframe id="app"></iframe>
</div></div>
<div id="logo">Syd</div>
<div id="kicker">THE SOFTWARE SYNTHESIZER WEB APP</div>
<div id="head"><span class="lime">PATCH.<br>CREATE.</span><br>PLAY.</div>
<div id="sub">A modular synthesizer, right in your browser.</div>
<div id="cta">TRY IT NOW</div>
<div id="url">syd.jbum.com</div>
<script>
// stars
const sc = document.getElementById("stars").getContext("2d");
let seed = 7;
const rnd = () => (seed = (seed * 16807) %% 2147483647) / 2147483647;
for (let i = 0; i < 160; ++i) {
  sc.fillStyle = `rgba(${200 + rnd() * 55},${200 + rnd() * 55},255,${0.15 + rnd() * 0.6})`;
  sc.beginPath(); sc.arc(rnd() * %(W)d, rnd() * %(H)d, rnd() * 1.3, 0, 7); sc.fill();
}
// the scope trace, in glowing magenta-to-lime-to-cyan
const scope = %(scope)s, cv = document.getElementById("scope"), g = cv.getContext("2d");
const grad = g.createLinearGradient(0, 0, %(W)d, 0);
grad.addColorStop(0, "#ff3cd2"); grad.addColorStop(0.45, "#c6ff3a"); grad.addColorStop(1, "#3ce8ff");
for (const [w, a, b] of [[14, .12, 18], [6, .35, 8], [2.2, 1, 0]]) {
  g.save(); g.strokeStyle = grad; g.lineWidth = w; g.globalAlpha = a; g.shadowColor = "#9b5cff"; g.shadowBlur = b;
  g.beginPath();
  scope.forEach((v, i) => { const px = i / (scope.length - 1) * %(W)d, py = 100 - v * 70; i ? g.lineTo(px, py) : g.moveTo(px, py); });
  g.stroke(); g.restore();
}
// the editor itself, on a first visit (so it opens the Shepard patch), dark, with its
// rendered waveform in the preview and an oscillator selected
localStorage.removeItem("syd-editor");
localStorage.setItem("syd-names", "1");
const frame = document.getElementById("app");
frame.src = "app/index.html";
frame.onload = async () => {
  const w = frame.contentWindow;
  w.document.documentElement.dataset.theme = "dark";
  for (let i = 0; i < 100 && w.eval("patchName") !== "shepard.syd"; ++i) await new Promise((r) => setTimeout(r, 50));
  const wav = await (await fetch("shepard.wav")).arrayBuffer();
  w.eval("selection = new Set([byLabel('mOsc10')]); fit(); inspect();");
  w.showWav(wav, 8);
  w.eval("status('Rendered 8 s in 1.4 s (5.7× real time).')");
  document.title = "ready";
};
</script></body></html>""" % {"W": W, "H": H, "scope": json.dumps(scope)}
open(os.path.join(tmp, "card.html"), "w").write(html)

# serve tmp (the card, with the app beside it) so the card can reach into the editor
class Quiet(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *a):
        pass
handler = functools.partial(Quiet, directory=tmp)
server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), handler)
threading.Thread(target=server.serve_forever, daemon=True).start()
url = f"http://127.0.0.1:{server.server_address[1]}/card.html"
subprocess.run(["node", os.path.join(HERE, "shoot.js"), url, OUT, str(W), str(H), os.path.join(tmp, "profile")],
               check=True, timeout=120)
server.shutdown()
print("wrote", OUT, Image.open(OUT).size)
