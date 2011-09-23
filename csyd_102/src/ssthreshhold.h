// SSThreshhold.h
#ifndef _H_SSThreshhold
#define _H_SSThreshhold	1

#include "ssmodule.h"

class SSThreshhold : public SSModule {
	ExpRec		cutOffExp;
public:
	// Overrides
	SSThreshhold(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	int		ProcessDoubleClick();
	void	Copy(SSModule *ss);
	void	Save(FILE* ar);
	void	Load(FILE* ar);
	void	Reset(SSModule *callingMod);
};

#endif
