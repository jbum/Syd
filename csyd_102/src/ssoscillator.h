// SSOscillator.h
#ifndef _H_SSOscillator
#define _H_SSOscillator	1

#include "ssmodule.h"
#include "expmgr.h"

// #define OscDLOG		200

enum {WT_Sine, WT_Sawtooth, WT_Square, WT_Triangle, WT_BLSquare, WT_Expression, WT_NbrWaveTypes};

class SSOscillator : public SSModule {
public:
	// Static
	int			waveType;
	ExpRec		wExp,fExp,aExp,pExp;

	// Dynamic
	ssfloat		waveInc;
	ssfloat		lastTime,lastOutput;

	// Overrides
	SSOscillator(ModList * mList, short h, short v);

	void	Initialize(short itsType, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod); // Uses global gTime;
	int		ProcessDoubleClick();
	void	Reset(SSModule *callingMod);
	void	Copy(SSModule *ss);
	void	Save(FILE* ar);
	void	Load(FILE* ar);
};

#endif
