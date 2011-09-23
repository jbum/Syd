// SSCScore.h

#ifndef _H_SSSkiniScore
#define _H_SSSkiniScore	1

#include "ssscore.h"

class SSSkiniScore : public SSScore {
public:
	// statics
	SydFileSpec	scoreFileSpec;

	SSSkiniScore(ModList * mList, short h, short v);

	// Local Functions
	char *GrabParam(char *p, int pNbr, ssfloat *pStore, Boolean *gotIt);

	// Overrides
	void GenerateScore(SSModule *callingMod);
	void Load(FILE* ar);
	void Copy(SSModule *ss);
};



#endif
