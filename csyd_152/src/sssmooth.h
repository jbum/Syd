// SSSample.h
#ifndef _H_SSSmooth
#define _H_SSSmooth	1

#include "ssmodule.h"

class SSSmooth : public SSModule {
	ssfloat	lastSample, lastSampleR;
public:
	// Overrides
	SSSmooth(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void Reset(SSModule *callingMod);
};

#endif
