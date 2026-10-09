// SSInverter.h
#ifndef _H_SSInverter
#define _H_SSInverter	1

#include "ssmodule.h"

class SSInverter : public SSModule {
public:
	// Overrides
	SSInverter(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
};

#endif
