// SSOutput.h
#ifndef _H_SSOutput
#define _H_SSOutput	1

#include "ssmodule.h"

enum {	OM_MEMORY, OM_AIFF, OM_WAVE};

class SSOutput : public SSModule {
// Static
public:
	ssfloat	sampleDuration,sampleRate;
	int		outputType;	// was Flags
	int   isStereo;
  SydFileSpec outFileSpec;

public:
	// Overrides
	SSOutput(ModList * mList, short h, short v);
	// int		ProcessDoubleClick();
	// void	Save(FILE* ar);
	void	Load(FILE* ar);
	// ssfloat	GenerateOutput(SSModule *callingMod);
	// ssfloat	GenerateOutputTime(SSModule *callingMod, ssfloat pTime);
	ssfloat GetInstParameter(int n);
	// char*	GetLabel();

};

#endif
