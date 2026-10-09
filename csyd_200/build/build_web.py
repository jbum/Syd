#!/usr/bin/env python3
"""Assemble the browser version in build/web: the page, the worker, csyd.js and csyd.wasm
(build/build_wasm.sh web), and example patches with the score files they read.
Serve build/web with any static file server; everything renders in the visitor's browser."""
import json, os, re, shutil, subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.abspath(os.path.join(HERE, ".."))
ROOT = os.path.abspath(os.path.join(SRC, ".."))
OUT = os.path.join(HERE, "web")

# The examples: the corpus's patches that load, one per name (csyd_102's first)
SKIP = {"wheel.syd": "csyd_102/patches"}        # csyd_102's wheel doesn't load; JSyd's does
DATA = ["csyd_102/patches/prelude.sco", "csyd_102/patches/stars.skini", "csyd_102/patches/test.skini"]

subprocess.run([os.path.join(HERE, "build_wasm.sh"), "web"], check=True)
shutil.rmtree(OUT, ignore_errors=True)
os.makedirs(os.path.join(OUT, "examples"))
for f in ("index.html", "worker.js"):
    shutil.copy(os.path.join(SRC, "web", f), OUT)
for f in ("csyd.js", "csyd.wasm"):
    shutil.copy(os.path.join(HERE, "wasm-web", f), OUT)

# Descriptions from the old read-me: "Name.syn" then indented lines
desc, name = {}, None
for line in open(os.path.join(ROOT, "csyd_102/patches/aaa_read.me.txt"), errors="replace"):
    m = re.match(r"^(\S+)\.syn\s*$", line)
    if m:
        name = m.group(1).lower()
        desc[name] = ""
    elif name and line.strip():
        desc[name] = (desc[name] + " " + line.strip()).strip()
    elif not line.strip():
        name = None

patches, seen = [], set()
for line in open(os.path.join(SRC, "tests/corpus.txt")):
    line = line.strip()
    if not line or line.startswith("#"):
        continue
    path = line.rsplit(" ", 1)[0]
    base = os.path.basename(path)
    if base in seen or SKIP.get(base) == os.path.dirname(path):
        continue
    seen.add(base)
    shutil.copy(os.path.join(ROOT, path), os.path.join(OUT, "examples", base))
    patches.append({"name": base[:-4], "file": base, "description": desc.get(base[:-4].lower(), "")})
patches.sort(key=lambda p: p["name"].lower())
for d in DATA:
    shutil.copy(os.path.join(ROOT, d), os.path.join(OUT, "examples"))
json.dump({"patches": patches, "files": [os.path.basename(d) for d in DATA]},
          open(os.path.join(OUT, "examples", "examples.json"), "w"), indent=1)
print(f"build/web: {len(patches)} example patches")
