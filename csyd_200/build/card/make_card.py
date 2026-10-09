#!/usr/bin/env python3
"""Make the social-media card, web/assets/card.png (1200x630): a plain screenshot of the
Shepard patch as the editor draws it (build/web, loaded live in headless Chrome with the
page's chrome hidden), the title and the address, and nothing else.

Needs build/web (build/build_web.py), Pillow and Google Chrome.
usage: build/card/make_card.py [--variant light|dark] [--out file.png]"""
import argparse, functools, http.server, os, subprocess, tempfile, threading
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.abspath(os.path.join(HERE, "..", ".."))
APP = os.path.join(SRC, "build", "web")
W, H = 1200, 630

ap = argparse.ArgumentParser()
ap.add_argument("--variant", choices=["light", "dark"], default="dark")
ap.add_argument("--out", default=os.path.join(SRC, "web", "assets", "card.png"))
args = ap.parse_args()

if not os.path.exists(os.path.join(APP, "index.html")):
    raise SystemExit("build/web is missing: run build/build_web.py first")
tmp = tempfile.mkdtemp(prefix="sydcard-")
os.symlink(APP, os.path.join(tmp, "app"))

# light: the diagram fills the card under a one-line title; the desk's own grey.
# dark: title and address in a left column, the diagram framed on the right.
STYLE = {
    "light": dict(theme="light", bg="#faf8f4", ink="#1f1d1a", muted="#6b665e", line="#e2ddd3",
                  text="position:absolute; left:48px; top:36px;",
                  title="font-size:40px;", url="position:absolute; right:48px; top:48px; font-size:26px;",
                  frame="position:absolute; left:48px; top:118px; width:1104px; height:512px; border-radius:10px 10px 0 0;",
                  fw=1104, fh=512),
    "dark": dict(theme="dark", bg="#171614", ink="#ece8e1", muted="#a29c91", line="#3d3a36",
                  text="position:absolute; left:56px; top:56px; width:330px;",
                  title="font-size:44px; line-height:1.15;", url="position:absolute; left:56px; bottom:56px; font-size:24px;",
                  frame="position:absolute; left:416px; top:56px; width:728px; height:518px; border-radius:10px;",
                  fw=728, fh=518),
}[args.variant]

html = """<!doctype html><html><head><meta charset="utf-8"><style>
  html, body { margin: 0; width: %(W)dpx; height: %(H)dpx; overflow: hidden; }
  body { position: relative; background: %(bg)s; color: %(ink)s;
    font-family: -apple-system, "Helvetica Neue", Helvetica, Arial, sans-serif; }
  #text { %(text)s }
  #title { %(title)s font-weight: 600; letter-spacing: -0.01em; margin: 0; }
  #title b { font-weight: 700; }
  #url { %(url)s color: %(muted)s; }
  #frame { %(frame)s overflow: hidden; border: 1px solid %(line)s; box-sizing: border-box; }
  iframe { width: %(fw)dpx; height: %(fh)dpx; border: 0; display: block; }
</style></head><body>
<div id="text"><h1 id="title"><b>Syd</b> &ndash; experimental software synth</h1></div>
<div id="url">syd.jbum.com</div>
<div id="frame"><iframe id="app"></iframe></div>
<script>
// the editor on a first visit (so it opens the Shepard patch), showing only its desk
localStorage.removeItem("syd-editor");
localStorage.setItem("syd-names", "1");
const frame = document.getElementById("app");
frame.src = "app/index.html";
frame.onload = async () => {
  const w = frame.contentWindow, d = w.document;
  d.documentElement.dataset.theme = "%(theme)s";
  const css = d.createElement("style");
  css.textContent = "header, #palette, #inspector, #preview, #zoom { display: none !important; }";
  d.head.appendChild(css);
  for (let i = 0; i < 100 && w.eval("patchName") !== "shepard.syd"; ++i) await new Promise((r) => setTimeout(r, 50));
  await new Promise((r) => setTimeout(r, 300));
  w.eval("selection = new Set(); fit();");
  document.title = "ready";
};
</script></body></html>""" % dict(STYLE, W=W, H=H)
open(os.path.join(tmp, "card.html"), "w").write(html)

# serve tmp (the card, with the app beside it) so the card can reach into the editor
class Quiet(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *a):
        pass
handler = functools.partial(Quiet, directory=tmp)
server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), handler)
threading.Thread(target=server.serve_forever, daemon=True).start()
url = f"http://127.0.0.1:{server.server_address[1]}/card.html"
subprocess.run(["node", os.path.join(HERE, "shoot.js"), url, args.out, str(W), str(H), os.path.join(tmp, "profile")],
               check=True, timeout=120)
server.shutdown()
print("wrote", args.out, Image.open(args.out).size)
