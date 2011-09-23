// ssmixer.h
#ifndef _H_SSMixer
#define _H_SSMixer	1

#include "ssmodule.h"

class SSMixer : public SSModule {
public:
	// Overrides
	SSMixer(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	ssfloat MixInputsAtten(int type, SSModule *callingMod);
};

#endif
