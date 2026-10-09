// SSDelay.h
#ifndef _H_SSDelay
#define _H_SSDelay	1

#include "ssmodule.h"

#define DF_Recursive	1

class SSDelay : public SSModule {
	// Stored Variables
	ExpRec		delayExp,a0Exp,a1Exp;
	int			flags;

	// Dynamic Variables
	long		allocDelaySamples;
	long		delayCounter,delayIncrement;
	ssfloat		*dBuf;
	ssfloat		delay,a0,a1,oldDelay;
public:
	// Overrides
	SSDelay(ModList * mList, short h, short v);
	// void	Dispose();
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Reset(SSModule *callingMod);
	void	CleanUp(void);
	int		ProcessDoubleClick();
	void	Copy(SSModule *ss);
	void	Save(FILE* ar);
	void	Load(FILE* ar);
};

#endif
