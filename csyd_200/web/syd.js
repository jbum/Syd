// syd.js: the .syd patch format, for the patch editor (and for node, to test it).
//
// A patch is {preamble, modules}. Each module is
//   {label, type, x, y, w, h, inputs: [{from, link}], params: {...}, raw: [lines], dirty}
// where params are the fields the inspector edits and raw holds the module's own lines
// as read.  An untouched module writes its raw lines back unchanged, so opening and
// saving a patch never rewrites what the editor didn't touch; an edited one (dirty)
// writes the format CSyd 1.5 and JSyd both read.
//
// Text is Latin-1 throughout: one character per byte, so the odd Mac Roman byte in an
// old expression survives the trip.  Use bytesToText / textToBytes at the edges.

(function (exports) {
"use strict";

// The module types, in CSyd's order (modlist.cpp mDesc), with JSyd's descriptions,
// icons and inputs (SydPanel.InitUnitDefs).  icon: 0 round (sources), 1 D (processors),
// 2 square (scores and data), 3 speaker (output).  stamp: the column in unitlabels.png.
const ctl = (n) => [["ctl", "Control Signal"]].concat(
  Array.from({ length: n }, (_, i) => ["ctl" + (i + 1), "Alt Control Sig #" + (i + 1)]));
const sigs = (n) => [["sig", "Default Signal"]].concat(
  Array.from({ length: n }, (_, i) => ["sig" + (i + 1), "Alt Signal #" + (i + 1)]));
const insts = Array.from({ length: 8 }, (_, i) => ["i" + (i + 1), "Instrument " + (i + 1)]);
const folderIns = [["f0", "Folder Input"]].concat(
  Array.from({ length: 15 }, (_, i) => ["f" + (i + 1), "Folder Input #" + (i + 1)]));

const WAVES = ["Sine", "Sawtooth", "Square", "Triangle", "BL Square", "Expression"];
const BUTTERS = ["Low Pass", "High Pass", "Band Pass", "Band Reject"];

// Parameter fields.  kind: exp (an expression), int, num, text (one word), file,
// choice (an index into options), check (0/1)
const E = (key, label, def, extra) => Object.assign({ key, label, kind: "exp", def }, extra);
const F = (key, label, kind, def, extra) => Object.assign({ key, label, kind, def }, extra);

const TYPES = {
  out: { desc: "Output", icon: 3, stamp: 0, inputs: [["sig", "Output Signal"]], fields: [
    F("stereo", "Channels", "choice", 0, { options: ["Mono", "Stereo"] }),
    F("duration", "Duration (seconds)", "num", 2),
    F("rate", "Sample rate", "num", 22050),
    F("outType", "Output to", "choice", 2, { options: ["Memory", "AIFF File", "WAV File"], hidden: true }),
    F("file", "Audio file", "file", "untitled.wav", { hidden: true }),
  ] },
  osc: { desc: "Oscillator", icon: 0, stamp: 1, inputs: [
    ["am", "Amplitude Modulation"], ["fm", "Frequency Modulation"], ["fmgain", "FM Gain"], ["amgain", "AM Gain"]],
    fields: [
      F("wave", "Waveform", "choice", 0, { options: WAVES }),
      E("freq", "Frequency", "440"), E("amp", "Amplitude", "1"), E("phase", "Phase", "0"),
      E("wexp", "Wave expression", "", { when: (p) => p.wave == 5 }),
    ] },
  env: { desc: "Envelope Generator", icon: 0, stamp: 2, inputs: [["trig", "Trigger Signal"]], fields: [
    E("at", "Attack time", "0.1"), E("al", "Attack level", "1"),
    E("dt", "Decay time", "0.2"), E("dl", "Decay level", "0.7"),
    E("st", "Sustain time", "0.6"), E("sl", "Sustain level", "0.6"),
    E("rt", "Release time", "0.1"), E("dur", "Duration", "1.0"),
    F("useTrigger", "Use trigger", "check", 0), F("interruptsOK", "Interrupts OK", "check", 0),
  ] },
  mix: { desc: "Mixer", icon: 1, stamp: 3, inputs: [["sig", "Signal to Mix"]], fields: [] },
  filt: { desc: "2nd-Order Filter", icon: 1, stamp: 4, inputs: [["sig", "Signal to Filter"]].concat(ctl(8)), fields: [
    E("a0", "a0", "0.5"), E("a1", "a1", "0.5"), E("a2", "a2", "0"), E("b1", "b1", "0"), E("b2", "b2", "0"),
    F("flags", "Flags", "int", 0, { hidden: true }),
  ] },
  butter: { desc: "Butterworth Filter", icon: 1, stamp: 5, inputs: [["sig", "Signal to Filter"]].concat(ctl(8)), fields: [
    F("ftype", "Filter type", "choice", 2, { options: BUTTERS }),
    E("freq", "Frequency", "1200"), E("bw", "Bandwidth", "1200*0.1"),
  ] },
  smooth: { desc: "Smoother", icon: 1, stamp: 6, inputs: [["sig", "Signal to Smooth"]], fields: [] },
  noise: { desc: "Noise Generator", icon: 0, stamp: 7, inputs: [], fields: [
    F("randomize", "Randomize", "check", 1), F("seed", "Seed", "int", 0),
  ] },
  delay: { desc: "Reverb / Effects", icon: 1, stamp: 8, inputs: [["sig", "Signal to Delay"]].concat(ctl(3)), fields: [
    E("delay", "Delay", "0.1"), E("a0", "a0", "1.0"), E("a1", "a1", "0.5"),
    F("feedback", "Feedback", "check", 1),
  ] },
  thresh: { desc: "Threshhold Unit", icon: 1, stamp: 9, inputs: [["sig", "Signal for Threshhold"]], fields: [
    E("cutoff", "Cutoff", "0.5"),
  ] },
  shold: { desc: "Sample and Hold", icon: 1, stamp: 10, inputs: [["sig", "Sampled Signal"], ["trig", "Trigger Signal"]], fields: [] },
  amp: { desc: "Amplifier", icon: 1, stamp: 11, inputs: [["sig", "Signal to Amplify"]].concat(ctl(3)), fields: [
    E("scale", "Amplify", "1.0"), E("offset", "Offset", "0.0"), E("pan", "Pan (-1 to 1)", "0.0"),
  ] },
  inv: { desc: "Inverter", icon: 1, stamp: 12, inputs: [["sig", "Signal to Invert"]], fields: [] },
  exp: { desc: "Expression", icon: 0, stamp: 13, inputs: sigs(15), fields: [E("exp", "Expression", "sig")] },
  folder: { desc: "Folder", icon: 0, stamp: 14, inputs: folderIns, fields: [
    F("file", "Patch file", "file", "mypatch.syd"),
  ] },
  rscore: { desc: "Random Score", icon: 2, stamp: 15, inputs: insts, fields: [
    E("n", "# Events", "p3 + 1"),
    E("p1", "p1 (instrument #)", "1"), E("p2", "p2 (start time)", "?*(p3-1)"),
    E("p3", "p3 (duration)", "0.5+?*0.5"), E("p4", "p4", "0.25 + ?*0.25"),
    E("p5", "p5", "55*2**(?*4.0)"),
  ].concat(Array.from({ length: 11 }, (_, i) => E("p" + (i + 6), "p" + (i + 6), "0"))) },
  cscore: { desc: "CSound Score", icon: 2, stamp: 16, inputs: insts, fields: [
    F("file", "Score file", "file", "myscore.sco"),
  ] },
  finput: { desc: "Folder Input", icon: 2, stamp: 17, inputs: [], fields: [
    F("n", "F# (0-n)", "int", 0), E("def", "Default", "0"), F("desc", "Description", "text", "input"),
  ] },
  pinput: { desc: "Score Parameter", icon: 2, stamp: 18, inputs: [], fields: [
    F("n", "P# (0-n)", "int", 0), E("def", "Default", "0"), F("desc", "Description", "text", "input"),
  ] },
  ftable: { desc: "Function Table", icon: 2, stamp: 19, inputs: sigs(3), fields: [
    F("n", "Table # (0-n)", "int", 0), F("size", "Length", "int", 100), E("exp", "Function", "sin(t*2*pi)"),
  ] },
  sample: { desc: "Sample File", icon: 0, stamp: 20, inputs: [], fields: [
    F("file", "Sample file", "file", "mysample.wav"), F("interp", "Interpolate", "check", 0),
    E("timeScale", "Time scale", "1.0"),
  ] },
  pluck: { desc: "Karplus/Strong Plucked String", icon: 0, stamp: 21, inputs: [], fields: [
    E("freq", "Frequency", "440"), E("dur", "Duration", "2"), E("amp", "Amplitude", "1"),
    E("decay", "Decay (dB)", "10"), F("variant", "Variant", "int", 0, { hidden: true }),
  ] },
  maraca: { desc: "Perry Cook's Maraca", icon: 0, stamp: 22, inputs: [], fields: [
    E("resFreq", "Resonance frequency", "3200"), E("resPole", "Resonance pole", "0.96"),
    E("prob", "Probability", "1/16"), E("sysDecay", "System decay", "0.999"),
    E("sndDecay", "Sound decay", "0.95"),
  ] },
  hammerbank: { desc: "Hammer Bank", icon: 0, stamp: 23, inputs: insts.concat(ctl(3)), fields: [
    E("bank", "Bank #", "0"), E("freq", "k → frequency", "cpsmidi(k)"), E("amp", "k → amplitude", "1.0"),
    E("attack", "k → attack", "0.05"), E("decay", "k → decay", "0.1"), E("sustain", "k → sustain", "1.0"),
    E("wave", "Waveform", "sin(t*2*pi)"),
  ] },
  hammeract: { desc: "Hammer Actuator", icon: 2, stamp: 24, inputs: sigs(3).concat(ctl(3)), fields: [
    E("bank", "Bank #", "0"), E("key", "Key #", "p5"), E("trigger", "Trigger", "1"),
    E("velocity", "Velocity", "p4/128"), E("undampen", "Undampen", "1"),
  ] },
  gassign: { desc: "Global Variable Assignment", icon: 2, stamp: 25, inputs: sigs(3), fields: [
    E("n", "G# (0-n)", "0"), E("value", "Value", "sig"), F("desc", "Description", "text", "var"),
  ] },
  skiniscore: { desc: "Skini (STK) Score", icon: 2, stamp: 16, inputs: insts, fields: [
    F("file", "Skini file", "file", "myscore.skini"),
  ] },
};

// JSyd's palette order: sources and scores first, then processors
const PALETTE = ["osc", "noise", "pluck", "maraca", "sample", "env", "exp", "hammerbank",
  "mix", "amp", "inv", "filt", "butter", "smooth", "delay", "thresh", "shold",
  "rscore", "cscore", "skiniscore", "ftable", "pinput", "folder", "finput", "gassign", "hammeract", "out"];

// Link colours, from JSyd: by input name
function linkColor(name) {
  const fixed = { sig: "#0000ff", am: "#0000ff", fm: "#00c000", fmgain: "#ff8800", amgain: "#0088ff",
    trig: "#ff0000", ctl: "#ff11ee", f0: "#0000ff" };
  if (fixed[name]) return fixed[name];
  let m = /^ctl(\d+)$/.exec(name);
  if (m) return "#ff" + hex2(0x11 * (+m[1] + 1)) + hex2(0xee - 0x11 * +m[1]);
  m = /^(sig|f)(\d+)$/.exec(name);
  if (m) return "#" + hex2(0x11 * m[2]) + hex2(0x11 * m[2]) + hex2(0xff - 0x11 * m[2]);
  m = /^i(\d+)$/.exec(name);
  if (m) return "#" + hex2(0xff - 0x22 * (m[1] - 1)).repeat(1) + "00" + hex2(0xff - 0x22 * (m[1] - 1));
  return "#888888";
}
function hex2(n) { return Math.max(0, Math.min(255, n)).toString(16).padStart(2, "0"); }

// C's %g, which CSyd uses to turn old numeric fields into expressions
function fmtG(x) {
  if (x === 0) return "0";
  if (!isFinite(x)) return String(x);
  const exp = Math.floor(Math.log10(Math.abs(x)));
  let s;
  if (exp < -4 || exp >= 6) {
    s = x.toExponential(5).replace(/\.?0+e/, "e");
    s = s.replace(/e([+-])(\d)$/, "e$10$2");
  } else {
    s = x.toFixed(Math.max(0, 5 - exp)).replace(/(\.\d*?)0+$/, "$1").replace(/\.$/, "");
  }
  return s;
}

// ---- reading -------------------------------------------------------------------

function bytesToText(bytes) {
  let s = "";
  for (let i = 0; i < bytes.length; i += 8192)
    s += String.fromCharCode.apply(null, bytes.subarray(i, i + 8192));
  return s;
}
function textToBytes(text) {
  const b = new Uint8Array(text.length);
  for (let i = 0; i < text.length; ++i) {
    const c = text.charCodeAt(i);
    b[i] = c < 256 ? c : 63;              // '?' for anything Latin-1 can't hold
  }
  return b;
}

const isSkip = (l) => l === "" || l[0] === "#" || l[0] === ";";

// The rest of a "KEY value" line, as CSyd's LoadExp takes it
const rest = (line, key) => line.length > key.length ? line.slice(key.length + 1) : "";
const words = (line) => line.trim().split(/\s+/);

// Fill a module's params from its raw lines.  Like CSyd, lines are read in order by
// prefix; unlike CSyd, a missing line leaves the default rather than failing.
function readParams(m) {
  const t = TYPES[m.type];
  const p = {};
  for (const f of t.fields) p[f.key] = f.def;
  const lines = m.raw.filter((l) => !isSkip(l));
  let i = 0;
  const next = (key) => (i < lines.length && lines[i].startsWith(key)) ? lines[i++] : null;
  const exp = (key, field) => { const l = next(key); if (l !== null) p[field] = rest(l, key); };
  const num = (s, d) => { const v = parseFloat(s); return isFinite(v) ? v : d; };
  const int = (s, d) => { const v = parseInt(s, 10); return isFinite(v) ? v : d; };
  let l, w;

  switch (m.type) {
  case "out":
    l = next("OUT");
    if (!l) break;
    w = words(l);
    if (l[3] === " ") { p.duration = num(w[1], 2); p.rate = num(w[2], 22050); p.outType = 2; p.stereo = 0; }
    else if (l[3] === "S") {
      p.stereo = int(w[1], 0) ? 1 : 0; p.duration = num(w[2], 2); p.rate = num(w[3], 22050);
      p.outType = int(w[4], 2); p.file = w[5] || "untitled.wav";
    } else {
      p.duration = num(w[1], 2); p.rate = num(w[2], 22050); p.outType = int(w[3], 2);
      p.file = w[6] || "untitled.wav"; p.stereo = 0;
    }
    break;
  case "osc":
    l = next("OSC");
    if (!l) break;
    if (l[3] === "I") {
      p.wave = int(words(l)[1], 0);
      exp("OSCF", "freq"); exp("OSCA", "amp"); exp("OSCP", "phase");
      if (p.wave == 5) { exp("WEXP", "wexp"); exp("OSCW", "wexp"); }
    } else {
      // "OSC n,(f),(a),(p),(w)", which this CSyd doesn't read
      const g = /^OSC ([0-9]+),\((.*)\),\((.*)\),\((.*)\),\((.*)\)/.exec(l);
      if (g) { p.wave = +g[1]; p.freq = g[2]; p.amp = g[3]; p.phase = g[4]; p.wexp = g[5]; }
    }
    break;
  case "env":
    l = next("ADSR");
    if (!l) break;
    if (l[4] === " ") {
      w = words(l).slice(1).map((s) => fmtG(num(s, 0)));
      [p.at, p.al, p.dt, p.dl, p.st, p.sl, p.rt] = w.concat(Array(7).fill("0")).slice(0, 7);
      exp("ADSRD", "dur");
      p.useTrigger = 0; p.interruptsOK = 0;
    } else {
      const fl = int(words(l)[1], 0);
      p.useTrigger = fl & 1 ? 1 : 0; p.interruptsOK = fl & 2 ? 1 : 0;
      for (const [k, f] of [["ADSRAT", "at"], ["ADSRAL", "al"], ["ADSRDT", "dt"], ["ADSRDL", "dl"],
                            ["ADSRST", "st"], ["ADSRSL", "sl"], ["ADSRRT", "rt"], ["ADSRD", "dur"]])
        exp(k, f);
    }
    break;
  case "filt":
    l = next("FLT");
    if (l) p.flags = int(words(l)[1], 0);
    exp("FLTA0", "a0"); exp("FLTA1", "a1"); exp("FLTA2", "a2"); exp("FLTB1", "b1"); exp("FLTB2", "b2");
    break;
  case "butter":
    l = next("BUTTER");
    if (l) p.ftype = int(words(l)[1], 2);
    exp("BUTTERF", "freq"); exp("BUTTERB", "bw");
    break;
  case "noise":
    l = next("RND");
    if (l) { w = words(l); p.randomize = int(w[1], 1) > 0 ? 1 : 0; p.seed = int(w[2], 0); }
    break;
  case "delay":
    l = next("DEL");
    if (!l) break;
    if (l[3] === " ") {
      w = words(l);
      const dly = num(w[1], 0), gain = num(w[2], 0);
      p.delay = fmtG(dly); p.a0 = fmtG(1 / (1 + gain)); p.a1 = fmtG(gain / (1 + gain));
      p.feedback = int(w[3], 0) ? 1 : 0;
    } else {
      p.feedback = int(words(l)[1], 0) & 1;
      exp("DELd", "delay"); exp("DELa0", "a0"); exp("DELa1", "a1");
    }
    break;
  case "thresh": exp("THR", "cutoff"); break;
  case "amp":
    l = next("AMP");
    if (!l) break;
    if (l[3] === " ") {
      w = words(l);
      p.scale = fmtG(num(w[1], 0)); p.offset = fmtG(num(w[2], 0));
    } else {
      // CSyd reads AMPP and a first AMPS with %s: the expression's first word
      if (l.startsWith("AMPP ")) { p.pan = words(l)[1] || ""; exp("AMPS", "scale"); }
      else p.scale = words(l)[1] || "";
      exp("AMPO", "offset");
    }
    break;
  case "exp": exp("EXP", "exp"); break;
  case "folder":
  case "cscore":
    l = next("CSCO");
    if (l) p.file = words(l)[1] || p.file;
    break;
  case "skiniscore":
    l = next("SKINISCO");
    if (l) p.file = words(l)[1] || p.file;
    break;
  case "rscore": {
    l = next("RSCO");
    if (!l) break;
    let n;
    if (l[4] === "#") { p.n = l.slice(6); n = 8; }
    else { n = int(words(l)[1], 0); exp("RSCON", "n"); }
    for (let k = 1; k <= 16; ++k) {
      if (k <= n) exp("RSCO" + k, "p" + k);
      else p["p" + k] = "0";
    }
    break;
  }
  case "finput":
  case "pinput": {
    const P = m.type === "finput" ? "FI" : "PI";
    exp(P + "DEF", "def");
    l = next(P + "NN"); if (l) p.n = int(words(l)[1], 0);
    l = next(P + "ND"); if (l) p.desc = words(l)[1] || "";
    break;
  }
  case "ftable":
    l = next("FTAB");
    if (l) { w = words(l); p.n = int(w[1], 0); p.size = int(w[2], 100); }
    exp("FTABE", "exp");
    break;
  case "sample":
    l = next("FSAMP");
    if (l) { w = words(l); p.file = w[1] || p.file; p.interp = int(w[4], 0) ? 1 : 0; }
    exp("FSAMPT", "timeScale");
    break;
  case "pluck":
    l = next("PLKI");
    if (l) p.variant = int(words(l)[1], 0);
    exp("PLKF", "freq"); exp("PLKD", "dur"); exp("PLKA", "amp"); exp("PLKd", "decay");
    break;
  case "maraca":
    next("MARI");
    exp("MARRF", "resFreq"); exp("MARRP", "resPole"); exp("MARP", "prob");
    exp("MARSysD", "sysDecay"); exp("MARSndD", "sndDecay");
    break;
  case "hammerbank":
    next("HAMB");
    exp("HAMBN", "bank"); exp("HAMBF", "freq"); exp("HAMBA", "amp"); exp("HAMBAt", "attack");
    exp("HAMBD", "decay"); exp("HAMBS", "sustain"); exp("HAMBW", "wave");
    break;
  case "hammeract":
    next("HAMA");
    exp("HAMAN", "bank"); exp("HAMAK", "key"); exp("HAMAT", "trigger");
    exp("HAMAV", "velocity"); exp("HAMAU", "undampen");
    break;
  case "gassign":
    exp("GASSN", "n"); exp("GASSV", "value");
    l = next("GASSD"); if (l) p.desc = words(l)[1] || "";
    break;
  }
  m.params = p;
  // JSyd wrote the wave expression as OSCW, which CSyd doesn't read: rewrite it
  if (m.type === "osc" && lines.some((x) => x.startsWith("OSCW"))) m.dirty = true;
}

// The module's lines in the current format, from its params
function writeParams(m) {
  const p = m.params, out = [];
  const L = (s) => out.push(s);
  const word = (s, d) => (String(s).trim().split(/\s+/)[0] || d);
  switch (m.type) {
  case "out": L(`OUTS ${p.stereo ? 1 : 0} ${p.duration} ${p.rate} ${p.outType} ${word(p.file, "untitled.wav")}`); break;
  case "osc":
    L("OSCI " + p.wave); L("OSCF " + p.freq); L("OSCA " + p.amp); L("OSCP " + p.phase);
    if (p.wave == 5) L("WEXP " + p.wexp);
    break;
  case "env":
    L("ADSR2 " + ((p.useTrigger ? 1 : 0) + (p.interruptsOK ? 2 : 0)));
    L("ADSRAT " + p.at); L("ADSRAL " + p.al); L("ADSRDT " + p.dt); L("ADSRDL " + p.dl);
    L("ADSRST " + p.st); L("ADSRSL " + p.sl); L("ADSRRT " + p.rt); L("ADSRD " + p.dur);
    break;
  case "filt":
    L("FLT " + p.flags);
    L("FLTA0 " + p.a0); L("FLTA1 " + p.a1); L("FLTA2 " + p.a2); L("FLTB1 " + p.b1); L("FLTB2 " + p.b2);
    break;
  case "butter": L("BUTTER " + p.ftype); L("BUTTERF " + p.freq); L("BUTTERB " + p.bw); break;
  case "noise": L(`RND ${p.randomize ? 1 : 0} ${p.seed | 0}`); break;
  case "delay": L("DELF " + (p.feedback ? 1 : 0)); L("DELd " + p.delay); L("DELa0 " + p.a0); L("DELa1 " + p.a1); break;
  case "thresh": L("THR " + p.cutoff); break;
  case "amp":
    // AMPP and the AMPS that follows the first line are read with %s: no spaces there
    L("AMPP " + String(p.pan).replace(/\s+/g, "")); L("AMPS " + p.scale); L("AMPO " + p.offset);
    break;
  case "exp": L("EXP " + p.exp); break;
  case "folder": case "cscore": L("CSCO " + word(p.file, "untitled")); break;
  case "skiniscore": L("SKINISCO " + word(p.file, "untitled")); break;
  case "rscore": {
    let n = 0;
    for (let k = 16; k >= 1; --k) if (String(p["p" + k]).trim() !== "0") { n = k; break; }
    L("RSCO " + n); L("RSCON " + p.n);
    for (let k = 1; k <= n; ++k) L("RSCO" + k + " " + p["p" + k]);
    break;
  }
  case "finput": case "pinput": {
    const P = m.type === "finput" ? "FI" : "PI";
    L(P + "DEF " + p.def); L(P + "NN " + (p.n | 0)); L(P + "ND " + word(p.desc, "input"));
    break;
  }
  case "ftable": L(`FTAB ${p.n | 0} ${p.size | 0}`); L("FTABE " + p.exp); break;
  case "sample": L(`FSAMP ${word(p.file, "untitled.wav")} 0 0 ${p.interp ? 1 : 0}`); L("FSAMPT " + p.timeScale); break;
  case "pluck":
    L("PLKI " + (p.variant | 0)); L("PLKF " + p.freq); L("PLKD " + p.dur); L("PLKA " + p.amp); L("PLKd " + p.decay);
    break;
  case "maraca":
    L("MARI"); L("MARRF " + p.resFreq); L("MARRP " + p.resPole); L("MARP " + p.prob);
    L("MARSysD " + p.sysDecay); L("MARSndD " + p.sndDecay);
    break;
  case "hammerbank":
    L("HAMB"); L("HAMBN " + p.bank); L("HAMBF " + p.freq); L("HAMBA " + p.amp); L("HAMBAt " + p.attack);
    L("HAMBD " + p.decay); L("HAMBS " + p.sustain); L("HAMBW " + p.wave);
    break;
  case "hammeract":
    L("HAMA"); L("HAMAN " + p.bank); L("HAMAK " + p.key); L("HAMAT " + p.trigger);
    L("HAMAV " + p.velocity); L("HAMAU " + p.undampen);
    break;
  case "gassign": L("GASSN " + p.n); L("GASSV " + p.value); L("GASSD " + word(p.desc, "var")); break;
  }
  return out;
}

// Parse patch text.  Throws an Error with a line number for what CSyd couldn't load.
function parse(text) {
  const lines = text.split(/\r\n|\r|\n/);
  if (lines.length && lines[lines.length - 1] === "") lines.pop();
  const patch = { preamble: [], modules: [], eol: /\r\n/.test(text) ? "\r\n" : /\r/.test(text) ? "\r" : "\n" };
  let i = 0;
  while (i < lines.length && !lines[i].startsWith("MODS")) patch.preamble.push(lines[i++]);
  if (i === lines.length) throw new Error("This isn't a Syd patch: it has no MODS line.");
  ++i;
  let cur = null;
  for (; i < lines.length; ++i) {
    const line = lines[i];
    if (line.startsWith("MOD ")) {
      // "MOD label type (x1 y1 x2 y2)"; CSyd 1.0's numeric form isn't supported
      const g = /^MOD\s+(\S+)\s+(\S+)\s*\(\s*(-?\d+)\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)\s*\)/.exec(line);
      if (!g) throw new Error(`Line ${i + 1}: can't read this module line: ${line}`);
      if (!TYPES[g[2]]) throw new Error(`Line ${i + 1}: unknown module type "${g[2]}"`);
      cur = { label: g[1], type: g[2], x: +g[3], y: +g[4], w: g[5] - g[3], h: g[6] - g[4],
              inputs: [], raw: [], dirty: false };
      patch.modules.push(cur);
    } else if (!cur) {
      if (!isSkip(line)) throw new Error(`Line ${i + 1}: expected a MOD line`);
      patch.preamble.push(line);
    } else if (line.startsWith("IN") && cur.raw.length === 0) {
      const g = /^IN\s+(\S+)\s+->\s+(\S+)/.exec(line);
      if (!g) throw new Error(`Line ${i + 1}: can't read this input line: ${line}`);
      cur.inputs.push({ from: g[1], link: g[2] });
    } else {
      cur.raw.push(line);
    }
  }
  // Trailing comment lines belong to no module
  for (const m of patch.modules) {
    while (m.raw.length && isSkip(m.raw[m.raw.length - 1]) && m === patch.modules[patch.modules.length - 1])
      (patch.trailer = patch.trailer || []).unshift(m.raw.pop());
    readParams(m);
  }
  return patch;
}

function serialize(patch) {
  const out = patch.preamble.slice();
  out.push("MODS " + patch.modules.length);
  for (const m of patch.modules) {
    out.push(`MOD ${m.label} ${m.type} (${Math.round(m.x)} ${Math.round(m.y)} ${Math.round(m.x + m.w)} ${Math.round(m.y + m.h)})`);
    for (const inp of m.inputs) out.push(`IN ${inp.from} -> ${inp.link}`);
    for (const l of m.dirty ? writeParams(m) : m.raw) out.push(l);
  }
  for (const l of patch.trailer || []) out.push(l);
  return out.join(patch.eol || "\n") + (patch.eol || "\n");
}

// A new module of a type, labelled the JSyd way (mOsc3), not colliding with any in use
function newModule(patch, type, x, y) {
  const t = TYPES[type];
  const base = "m" + type[0].toUpperCase() + type.slice(1);
  const used = new Set(patch.modules.map((m) => m.label));
  let n = patch.modules.length + 1;
  while (used.has(base + n)) ++n;
  const params = {};
  for (const f of t.fields) params[f.key] = f.def;
  return { label: base + n, type, x, y, w: 40, h: 40, inputs: [], raw: [], params, dirty: true };
}

function newPatch() {
  const p = { preamble: [], modules: [], eol: "\n" };
  const out = newModule(p, "out", 420, 160);
  out.params.duration = 4;
  out.params.rate = 44100;
  p.modules.push(out);
  const osc = newModule(p, "osc", 260, 160);
  p.modules.push(osc);
  out.inputs.push({ from: osc.label, link: "sig" });
  return p;
}

// Input names a module of this type accepts (mixers take any number of "sig")
function inputNames(type) { return TYPES[type].inputs.map((x) => x[0]); }

// Data files a patch reads (scores, samples, folder patches)
function dataFiles(patch) {
  const files = [];
  for (const m of patch.modules)
    if (["folder", "cscore", "skiniscore", "sample"].includes(m.type) && m.params.file) files.push(m.params.file);
  return files;
}

// The output's length in seconds
function duration(patch) {
  const o = patch.modules.find((m) => m.type === "out");
  return o ? +o.params.duration : NaN;
}

Object.assign(exports, { TYPES, PALETTE, WAVES, linkColor, fmtG, bytesToText, textToBytes,
  parse, serialize, readParams, writeParams, newModule, newPatch, inputNames, dataFiles, duration });
})(typeof module !== "undefined" ? module.exports : (window.Syd = {}));
