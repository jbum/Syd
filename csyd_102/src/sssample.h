// SSSample.h
#ifndef _H_SSSampleAndHold
#define _H_SSSampleAndHold	1

#include "ssmodule.h"

class SSSampleAndHold : public SSModule {
// Dynamic
	Boolean	lastTrigger;
	ssfloat lastOutput;
public:
	// Overrides
	SSSampleAndHold(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void Reset(SSModule *callingMod);
};

#endif
