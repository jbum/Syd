// SSAmplifier.h
#ifndef _H_SSAmplifier
#define _H_SSAmplifier	1

#include "ssmodule.h"
#include "expmgr.h"

class SSAmplifier : public SSModule {
	// Statics
	ExpRec			scaleExp, offsetExp, panExp;
	// ssfloat		scale,offset; (old constant versions)
public:
	// Overrides
	SSAmplifier(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Reset(SSModule *callingMod);
	int		ProcessDoubleClick();
	void	Copy(SSModule *ss);
	void	Load(FILE* ar);
  // void  Save(FILE* ar);
};

#endif
