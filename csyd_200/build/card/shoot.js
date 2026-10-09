#!/usr/bin/env node
// Screenshot a page once it says it's ready (document.title === "ready"), by driving
// headless Chrome over the DevTools protocol.  Chrome's own --screenshot waits for the
// page to go idle, which the editor (always animating) never does.
// usage: shoot.js <url> <out.png> <width> <height> <profile dir>
const { spawn } = require("child_process"), fs = require("fs"), path = require("path");
const [url, out, W, H, profile] = process.argv.slice(2);
const CHROME = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome";
const chrome = spawn(CHROME, ["--headless=new", "--disable-gpu", "--hide-scrollbars", "--remote-debugging-port=0",
  `--user-data-dir=${profile}`, `--window-size=${W},${H}`, "--force-device-scale-factor=1", "about:blank"], { stdio: "ignore" });
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const fail = (msg) => { console.error(msg); chrome.kill(); process.exit(1); };
setTimeout(() => fail("timed out"), 60000).unref();

(async () => {
  const portFile = path.join(profile, "DevToolsActivePort");
  while (!fs.existsSync(portFile) || !fs.readFileSync(portFile, "utf8").includes("\n")) await sleep(100);
  const port = fs.readFileSync(portFile, "utf8").split("\n")[0];
  const targets = await (await fetch(`http://127.0.0.1:${port}/json`)).json();
  const ws = new WebSocket(targets.find((t) => t.type === "page").webSocketDebuggerUrl);
  await new Promise((r) => ws.addEventListener("open", r));
  let id = 0;
  const pending = new Map();
  ws.addEventListener("message", (e) => {
    const m = JSON.parse(e.data);
    if (pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); }
  });
  const send = (method, params = {}) => new Promise((r) => { pending.set(++id, r); ws.send(JSON.stringify({ id, method, params })); });
  await send("Emulation.setDeviceMetricsOverride", { width: +W, height: +H, deviceScaleFactor: 1, mobile: false });
  await send("Page.navigate", { url });
  for (;;) {
    const r = await send("Runtime.evaluate", { expression: "document.title" });
    if (r.result && r.result.result && r.result.result.value === "ready") break;
    await sleep(200);
  }
  await sleep(800);                                     // let images and the waveform settle
  const shot = await send("Page.captureScreenshot", { format: "png", clip: { x: 0, y: 0, width: +W, height: +H, scale: 1 } });
  fs.writeFileSync(out, Buffer.from(shot.result.data, "base64"));
  ws.close();
  chrome.kill();
  process.exit(0);
})().catch((e) => fail(String(e)));
