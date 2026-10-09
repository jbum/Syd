/*  History File


* * *
Wed 10/29
	Renamed to "Syd"
	Added "A" for amplitude values of hammerbank instruments.

Tues 10/28
	Fixed bug with butter filter not working properly
	if bandwidth gets set to 0 (generates INF for some
	coefficients, and NANs for y1 and y2, which propagate).

Tues 10/21
	Added variants of ftab() to support wrapping,pinning
	and interpolation.
		ftabw(),ftabp(),ftabi(),ftabiw(),ftabip()
	Added 'r' variable for sample rate.
	Added pi2 and E
	Added clarinetWS and bell to patches.
	Fixed bug with FTables not initializing properly on first pass.
	// Added envelope generation functions:
	Added linen(t,atk,dur,dcy) function.
	Added linenr(t,atk,dcy,atten)
	Added limit(v,low,high)
	Made a SS webpage.

Mon 10/20
	Debugged Hammerbank some.
	Created Risset Clarinet and Bell patches.

Sun 10/19
	Added Global Assignment unit.
	Added HammerBank and HammerActuator, for simulating
	resonating string instruments such as harps and pianos.
	Added support for "i" in both randomscores (note number)
	and hammerbank (inten).
	"K" in hammerbank is keynumber.

Sat 10/18
	Version 1.0.5

	Modified delay unit to use expressions, added a0,a1.  Modified some
	patches that refer to it.

	Changed threshhold unit to use expressions.
	Added Ctl inputs to Amplifier
	Changed "sig1" inputs to filters to Ctl Inputs
	Updated the docs.
	
	Added support for 8-bit mono files for input.
	Added support for setting output file from output module.
	Removed "Output" menus.

Fri 10/17

	Fixed Random Score / Score system to use numbers from 1-n for instruments.
	"0" will use first available instrument.
	Fixed bug with parsing .syn files with old style random scores.
	Fixed bug with parsing expressions with spaces from.
	Fixed bug in which scores referring to non-existent instruments cause crashes
	during synthesis.
	Removed some of the less interesting patches from the Samples folder.

Thu 10/16
	Version 1.0.4
	
Wed 10/15
	Added Butterworth filters (lopass,hipass,bandpass,bandreject)
	Increased parameters in random score to 16.
	Removed BPFilter (butterworth does it okay)
	Replaced FIR with 2OS (more flexible).
	Added "clarinet" and "choir" patches to examples.
	Added parameters to "rscore" module.

Tues 10/14
	Version 1.0.3
	
Tues 10/14 (early morning)
	Fixed bug in BL-Square.
	Modified BPFilter to do better BandPass filter ala Moore (side frequencies
	drop to zero using BW)

Sat 10/11 (early am)
	Fixed some bugs which caused crashes in 68k version.
	Added initialization feedback in log window.
	Sped up wave table init a bit.
	
Tues 10/7
	Added Perry Cook's Maraca sim from CMJ 21.3 (fall 1997)

Mon 10/6
	Added general expressions to Filters (FIR)
	Fixed bug with graph window (Y values were inverted).

Sat 10/4
	Added support for '+' in scores (used in "orbit" score)
	Added sign() function.
	Modified ADSR
	Experimented with granular synth, simple fm and
	wave terrains.

10/3/97
	Version	1.0.2
	Added real time audio preview ("listen").
	Fixed various bugs with SampleFile.
	Fixed MemoryStore to use sliding window, so we can preview long patches without
	saving to disk nor using huge amounts of memory.

10/2/97
	Added support for various kinds of output using generalized output module (SSStorage)
	Added SampleFile Module with flags and timescale exp.
	Fixed bug in which modules could be dropped on gadget bar.

9/30/97
	Added necessary fixes to compiled 68k FPU version.
	There is a bug triggered by using precompiled headers with FPU turned off - made a bypass by adding
	"SSSound.h" file.
	
	Added support for interleaved tempo statements in a score file (still some potential 
	problems here - but better - still not supporting multi-tempo statements as described
	in CSound docs..

9/28/97
	Version 1.0.1

	Added "68k" version.
	Increased gadget panel to two rows.
	Sped up audio graphics rendering.
	Added Karplus/Strong plucked instrument.
	Added "Log Window" feature for debugging.

9/26/97
	Release as 1.0

	Fixed bug in reverb unit.
	Fixed bug in instrument dealocation.
	Fixed bug with P-Input units.

9/19/97
	Changed meaning of "^" in expressions.  Added verbal "mod","xor","not","and","or" 
	to suppliment C-style ops.

	Improved patching interface a bit (2-3 pixel grace area, and shorter time out).
	Fixed patching highlighting bug.

	Added "pow2"

9/16/97
	Made "ModList" a legitimate object, which frees up modules from
	having to refer to "mw".
	
	Added support for multiple windows opt-Click to open a sub-module.
	
	UNTESTED - Added support for FTables and ftab(n,t) in expressions.

9/15/97
	Fixed bug with "input" modules.
	Added "P-Inputs"
	Improved cosmetics a bit.
	Added "parList" system so that nested folder inputs work correctly.
	Expressions only calculate a signal once.

9/14/97
	Fixed bug with saving files.

	Added "Input" module.

9/12/97
	Added system to support input signals into a folder instrument via 'Fn'

	Added support for nested score modules.
	
	Made random scores more flexible by using expressions.
	
	Random scores can use "n" for interpolation of successive notes.

9/11/97
	Fixed bug with 'n' variable.
	Fixed Score Modules to show names
	Began implementing folders.
	
9/10/97
	Fixed some bugs.

	Made Synthesis Asynchronous and Interruptable

	Added "Stop" button, rearranged buttons a bit.
	
	Added 'n' variable for "note time" to be used in wave tables.

	Added octmidi and cpsmidi functions.

9/9/97
	Made cpspch() function which is equiv to CSound version.
	Also made equiv octpch, cpsoct, pchoct, and octcps.
	Added minimal support for CSound scores.
		
9/8/97
	Made RandScore and CScore subclasses of "Score".
	Added density and other variables for RandScore (all constants for now).


9/4/97
	Added icon for eventual "folder" module - this will enable nesting of instruments.

	Added Score Module which accepts inputs from various instruments, and generates
	notes using a CSound score file.  (Note: scores can't be treated as instruments yet,
	because parameter system doesn't save old values on a stack - something to consider...)
	
	Added parameter system for passing score params to instruments.
	
	Changed amplifier to use expressions.
	
	Reduced size of gadgets.
	
	Added some useful debugging stuff (show Memory command, NewPtr/DisposePtr
	shells etc.)
	
	Fixed some memory leaks.

9/2/97
	Fixed problem with graphics module.
	
8/29/97
	Fixed to support closing of main window without quitting application.
	Fixed bug in which outputMod wasn't being reasigned properly.
	Fixed bug in patching perfectly horizontal lines.
	Added fbn(), gnoise3d() and gturb3d() functions.
	Added 'd' variable (duration of note event).

8/28/97
	Optimized Rendering to use offscreen graphics.
	Added minimal graph window
		Click = Zoom In
		Opt-Click = Zoom Out
		Cmd-Click = Reset
		Left/Right = Scroll

8/27/97
	Fixed Expressions to accept named parameters from calling module.
	
	Removed hard coded forumulas for FM and AM from oscillators and replaced
	with more flexible expression based system.

8/26/97
	Fixed  patches to be unit-defined.  Changed patch format so that link types
	are named ("sig" "am" "trig" etc.).
	
	Added pull-down menu interface for changing patch types.
	
	Modified file format to use more symbols, and be more readable.
		x Change module ID and Type to symbols
		x Autoname modules (mTypeN)
		x Change IN line to use symbols
		x Get rid of #INs
		x Add punctuation for x,y and links
		x support old format
		x kill trailing zeros on floats
		MOD mOsc1 OSC (283 128 315 160)
		IN mSamp2 -> fm
		IN mOsc2 -> am
		OSC 220.0 2.0 1.0 0 0.0

	Converted patches to new system:
	
	Implemented Dirty Flag and Confirm Saves
	
	Fixed a bug in sample + hold.

8/25/97
	Changed time implementation so that time value is based back thru
	solution tree.  This will enable, among other things, wave table modules
	using general expressions.


	Moved most expression stuff to "ExpMgr" so we can harness expressions from
	various points within the program.
	
	Added 'g' variable (global time)
	
	Added Expression based wavetables for oscillators
	
8/18/97
	Started keeping this file - We'll call the old version 1.0
	
	Fixed oscillators to output -1 to 1, instead of 0-1, since this was
	screwing with amplification and amplitude modulation.
	
	Fixed some bugs in waveforms.
	
	Added Phase Parameter.

	Added Params for Noise generator to make reproduceable noise.
	
	Changed Sin to wavetable.,
	
	Added Band Limited Square wave.
	
	Implememented ADSR


**/