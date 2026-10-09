# CSyd 1.5.2

1.5.1 with its bugs fixed and nothing else changed: it renders every patch in `tests/corpus.txt`
exactly as 1.5.1 does (`tests/check_vs_151.py`), except where 1.5.1 had no single answer.

- **64-bit builds.** `int32`/`uint32` were `long`, which is 8 bytes on 64-bit macOS and Linux, so
  WAV and AIFF headers were written with 8-byte fields and the files didn't open. They are now
  exactly 32 bits, and the header code uses them throughout.
- **Stereo headers.** WAV files gave a block align of 2 and AIFF files 1 channel whatever the
  channel count, so stereo files were misread.
- **Run-to-run differences.** A module can be evaluated before its `Reset()` runs, and several
  leave members to `Reset()`. 1.5.1 read whatever the heap held, so a patch such as
  drones+melody rendered differently from one run to the next. Modules now start zeroed, which
  is how 1.5.1 renders a freshly allocated instrument.
- **Reproducible noise.** `-r seed` (or `SYD_SEED`) fixes the seed of noise modules that ask
  for a randomized one, which otherwise come from the clock.
- **Stray output.** A leftover debug line printed `+` for every sample between 9 and 12
  seconds in which a score had no notes sounding.
- **Build.** A Makefile for current compilers (the old one forced a 32-bit build). STK support
  is off unless built with `-DUSE_STK` and the STK headers.
