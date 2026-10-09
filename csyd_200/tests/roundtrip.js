#!/usr/bin/env node
// Checks the patch editor's reader and writer (web/syd.js) against CSyd itself: every
// corpus patch is read and written back, once as is and once with every module
// rewritten from its parsed fields, and each copy must render the same bytes as the
// original.  usage: tests/roundtrip.js [seconds]   (needs ./csyd built)
const fs = require("fs"), path = require("path"), os = require("os"), { execFileSync } = require("child_process");
const Syd = require("../web/syd.js");
const HERE = path.resolve(__dirname, ".."), ROOT = path.resolve(HERE, "..");
const secs = process.argv[2] || "2";
const work = fs.mkdtempSync(path.join(process.env.TMPDIR || os.tmpdir(), "syd-rt-"));

function render(dir, name, text) {
  // a scratch copy of the patch's folder, so scores and samples resolve
  const d = fs.mkdtempSync(path.join(work, "p-"));
  for (const f of fs.readdirSync(dir)) {
    try { if (fs.statSync(path.join(dir, f)).isFile()) fs.symlinkSync(path.join(dir, f), path.join(d, f)); } catch (_) {}
  }
  fs.rmSync(path.join(d, name), { force: true });
  fs.writeFileSync(path.join(d, name), Buffer.from(Syd.textToBytes(text)));
  const out = path.join(d, "out.wav");
  try {
    execFileSync(path.join(HERE, "csyd"), ["-q", "-r", "7", "-d", secs, "-o", out, name], { cwd: d, stdio: "ignore", timeout: 120000 });
    return fs.readFileSync(out);
  } catch (e) { return null; }
}

let ok = 0, bad = 0;
for (const line of fs.readFileSync(path.join(HERE, "tests/corpus.txt"), "utf8").split("\n")) {
  if (!line.trim() || line.startsWith("#")) continue;
  const rel = line.trim().split(/\s+/)[0], file = path.join(ROOT, rel);
  const text = Syd.bytesToText(fs.readFileSync(file));
  let patch;
  try { patch = Syd.parse(text); } catch (e) { console.log("PARSE  " + rel + ": " + e.message); ++bad; continue; }
  const same = Syd.serialize(patch);
  for (const m of patch.modules) m.dirty = true;
  const rewritten = Syd.serialize(patch);
  const orig = render(path.dirname(file), path.basename(file), text);
  const a = render(path.dirname(file), path.basename(file), same);
  const b = render(path.dirname(file), path.basename(file), rewritten);
  const verdict = !orig ? "NOREF" : (orig.equals(a || Buffer.alloc(0)) ? "" : "AS-IS ") + (orig.equals(b || Buffer.alloc(0)) ? "" : "REWRITE");
  if (verdict) { console.log((verdict + "      ").slice(0, 14) + rel); ++bad; } else ++ok;
}
console.log(`${ok} identical, ${bad} not`);
fs.rmSync(work, { recursive: true, force: true });
