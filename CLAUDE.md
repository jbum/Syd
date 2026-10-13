# Syd / CSyd 2.0 — working notes

Context for continuing the CSyd 2.0 and syd.jbum.com work (started 2026-10-08/09).
Owner: Jim Bumgardner (jbum). The 2.0 work (branch `csyd2`) was merged into `master`
of the public github.com/jbum/Syd on 2026-10-12. The repo is public — keep server details out of it.

## Layout

| Dir | What |
|---|---|
| `csyd_102/` | Original CSyd 1.5.1. Untouched reference; its `patches/` are most of the corpus. |
| `jsyd_102/` | JSyd, the Java/Swing patch editor (2006–09). Ideas + icon art for the web editor. Its uncommitted `build.xml` change and stray `JSydBeta/*.wav` predate this work — leave them alone, don't commit them. |
| `csyd_152/` | 1.5.1 with bugs fixed only (64-bit int32/WAV headers, stereo headers, zeroed modules, `-r` seed). See `csyd_152/CHANGES.md`. `tests/check_vs_151.py`. The reference that 2.0 is graded against. |
| `csyd_200/` | CSyd 2.0: fast, threaded, own math, WebAssembly, web editor. Everything below is here. |

## CSyd 2.0 (native)

Build: `cd csyd_200 && make` → `./csyd`. Options added over 1.5: `-b` block size, `-j` threads,
`-r` seed (or `SYD_SEED`), `-M` max seconds, `-P` progress to stderr, `-d` duration, `-o` out.

What changed vs 1.5.2 (commits f22f079..e91dc61):
- Expressions compiled once to bytecode (shared decoded program), p1..pn cached.
- `src/smath/`: vendored musl libm (`sm_*` names), built with `-ffp-contract=off` (no FMA),
  so every platform — including wasm — renders the same bits. (Apple's `pow(x,2)` ≠ `x*x`.)
- Scores render notes in 256-sample blocks across a thread pool (`SSScore::RenderBlock`,
  `sspool.*`), summed in original order. Only voices whose subtree is block-safe
  (`BlockSafe` whitelist; `EF_BlockUnsafe` per expression); hammer patches stay per-sample.
- Per-note random streams (`HashSeed`, `SSModule::Random`) only in block mode.
- Oscillator/amp/mixer `RenderBlock` with vector expression evaluation (`ExpMgr::EvalVector`).
- `stable_sort` of score notes (qsort made wasm ≠ native); `FOpenRead` case-insensitive.
- Reads JSyd's `OSCW` wave-expression line as well as `WEXP` (7 JSyd-saved patches).
- Speed: whole corpus ~4× faster than 1.5.2 single-threaded; testWhitneyChromatic 30.3 s →
  6.4 s (1 thread) → 1.5 s (8 threads).

### Fidelity rules (jbum's call)
"Sounds the same" beats bit-exact for random/rounding changes, but regressions must be caught:
- `tests/compare.py [--make-ref] [-k WORD]` — grades every corpus patch vs 1.5.2:
  exact / ok (≤4 LSB or ≥90 dB SNR) / similar (loudness + third-octave spectrum within 2.0's own
  seed-to-seed spread) / CHECK. Last result: 69 exact, 16 similar, 0 CHECK.
  A/B pairs for listening: `tests/listen/`.
- `tests/compare_wasm.py` — wasm vs native must be **byte-identical** (all 85 are).
- `tests/roundtrip.js [secs]` (node) — the web editor's reader/writer: every corpus patch,
  saved as-is and with every module rewritten, must render byte-identical (85/85).
- `tests/bench.sh` — best-of-3 timings. `tests/corpus.txt` — 85 patches (Whitney family sampled).
- Known, left alone: 1.5.1's 'F' token double-push bug.

### Gotchas
- Many sources are **CRLF** (e.g. `modlist.cpp`) and some **Latin-1**; patch files too (a Mac
  Roman π byte appears in expressions). Edit with a script that preserves line endings and
  encoding; `str.replace` on decoded LF text silently misses.
- `register` is an error in C++17 — wasm build uses `-Wno-register`.
- Module labels max 31 chars, no spaces; `GetNextInputLine` matches line *prefixes* in order and
  a missing line crashes 1.5 — the editor always writes complete modules.

## WebAssembly + web (syd.jbum.com)

- Emscripten 6.0.12 in `~/emsdk` (brew's failed): `source ~/emsdk/emsdk_env.sh`.
- `build/build_wasm.sh node|web` — single-threaded (`SYD_NO_THREADS`), MODULARIZE `createCSyd`,
  MEMFS. Browser threads would need COOP/COEP headers (not done).
- `build/build_web.py` → `build/web/` (gitignored): rebuilds wasm, copies pages, examples
  (from the corpus, with descriptions from `csyd_102/patches/aaa_read.me.txt`) and score files.
- `build/deploy_web.sh` → rsync `--delete` to `$DEPLOY_DEST` or the destination in `build/deploy.dest` (not committed).
- Full publish: `source ~/emsdk/emsdk_env.sh && python3 build/build_web.py && build/deploy_web.sh`.

### Pages (source in `csyd_200/web/`)
- `index.html` — **the patch editor** (site root). First visit (nothing in localStorage
  `syd-editor`) opens `shepard.syd`; afterwards reopens the last patch.
- `about.html` — project history (1997 → 2.0), the Whitney Music Box and its video, the GitHub link.
  Linked from the editor's header and the renderer.
- `csyd.html` — the plain renderer, served at `/csyd/` (shared files referenced as `../`).
- `editor.html` on the site is a generated redirect stub (the editor's first address).
- `syd.js` — .syd parse/serialize + module catalog (types, fields, inputs, JSyd link colours,
  palette order). Untouched modules write back their raw lines; edited ones (`dirty`) write the
  modern format. Works in node (tests) and browser. Text is Latin-1 (`bytesToText`/`textToBytes`).
- `worker.js` — runs `csyd.wasm` per render (fresh instance; accepts patch bytes).
- `assets/` — JSyd's original `iconbacks.png` (4 shapes × plain/selected) and `unitlabels.png`
  (26 stamps), plus `card.png` (social card).

### Editor architecture (`web/index.html`)
- SVG desk is an endless canvas: `view {x,y}` + `zoom`; `toPatch()` maps screen→patch from that
  state (not the svg CTM, which can lag a frame). `draw()` rebuilds the svg; `drawSoon()` via rAF.
- Pointer drags use `track(e, move, up, abort)`; captured on the persistent `#desk` element
  (iPad touches die if their element is redrawn away). A second finger aborts any drag and pinches.
- Pan/zoom: touch 1-finger on empty desk pans, press-and-hold = selection box, tap deselects,
  2-finger pinch; mouse Option/Space/middle-drag pans, wheel pans, ⌘/Ctrl-wheel or trackpad
  pinch zooms (Safari gesture events too). `autoFit` refits on resize until zoomed by hand.
- Undo/redo = JSON snapshots (`remember()` before every change). Inspector edits params via
  `setParam` (marks module dirty). Source tab edits raw text.
- **Render hook**: `render(text, {name, seconds, seed, files, onProgress})` → `{wav, log}`;
  Preview = first N s, Render all = whole patch (cap 600 s). Waveform panel `showWav`.
- Synthesis is **always** csyd.wasm — never JSyd's Java engine (jbum's requirement).

### Hosting
- Public static site (jbum chose public, no auth). Server details (host, vhost, paths, logs) are in
  the uncommitted `CLAUDE.local.md`: the repo is public, so they stay out of it.

### Social card
- Tags (og:* + twitter summary_large_image) in both pages; image `assets/card.png?v=3`
  (bump `v` when the card changes so caches refetch).
- `build/card/make_card.py [--variant light|dark]` (needs built `build/web`, Pillow, Chrome) loads
  the live editor in an iframe with its chrome hidden, so the card is a plain screenshot of the
  Shepard patch, the title "Syd – experimental software synth" and the address; `build/card/shoot.js`
  captures it via the DevTools protocol (Chrome's `--screenshot` never fires: the editor never
  idles). jbum chose the dark variant (2026-10-09) over an earlier neon/Gemini-style card: keep
  it plain, no taglines or glow.

## Not done yet / ideas
- Folder units: editing the sub-patch a folder loads (only its filename is editable now).
- Expression-language help/reference beside fields (variables, functions, `?`, `p1..pn`, `sig`, `ctl`…).
- Browser multi-threading (COOP/COEP + pthreads build).
- jbum may still listen to the A/B pairs in `csyd_200/tests/listen/`.

## Working style jbum likes
- Commit per milestone; messages say *why* (see `git log`), ending with the Co-Authored-By /
  Claude-Session trailers the session provides.
- Verify in a real browser (Chrome) and with the test harnesses before calling something done;
  he uses an iPad too, so consider touch.
