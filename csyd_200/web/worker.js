// CSyd render worker: renders a patch with csyd.wasm and posts back the WAV.
//
// in:  {patch: {name, text}, files: [{name, data: ArrayBuffer}], seconds, maxSeconds, seed}
// out: {type: "progress", value: 0..1}
//      {type: "done", wav: ArrayBuffer, log: string}
//      {type: "error", message: string, log: string}

importScripts("csyd.js");

let wasmBinary = null;

onmessage = async (e) => {
  const { patch, files, seconds, maxSeconds, seed } = e.data;
  const log = [];
  try {
    if (!wasmBinary)
      wasmBinary = await (await fetch("csyd.wasm")).arrayBuffer();
    // A fresh instance each render: CSyd keeps its state in globals
    const csyd = await createCSyd({
      wasmBinary,
      print: (line) => log.push(line),
      printErr: (line) => {
        const m = /^progress ([\d.]+)/.exec(line);
        if (m) postMessage({ type: "progress", value: parseFloat(m[1]) });
        else log.push(line);
      },
    });
    csyd.FS.mkdir("/work");
    csyd.FS.chdir("/work");
    for (const f of files) csyd.FS.writeFile(f.name, new Uint8Array(f.data));
    csyd.FS.writeFile(patch.name, patch.text);

    const args = ["-q", "-P", "-M", String(maxSeconds), "-o", "/work/csyd-out.wav"];
    if (seconds) args.push("-d", String(seconds));
    if (seed) args.push("-r", String(seed));
    args.push(patch.name);
    let status = 0;
    try {
      status = csyd.callMain(args);
    } catch (err) {
      status = err && err.status !== undefined ? err.status : 1;
      if (!(err && err.name === "ExitStatus")) log.push(String(err));
    }
    let wav = null;
    try { wav = csyd.FS.readFile("/work/csyd-out.wav"); } catch (_) {}
    if (status !== 0 || !wav || wav.length <= 44) {
      postMessage({ type: "error", message: "CSyd couldn't render this patch.", log: log.join("\n") });
      return;
    }
    postMessage({ type: "done", wav: wav.buffer, log: log.join("\n") }, [wav.buffer]);
  } catch (err) {
    postMessage({ type: "error", message: String(err), log: log.join("\n") });
  }
};
