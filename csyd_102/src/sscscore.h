// SSCScore.h

#ifndef _H_SSCScore
#define _H_SSCScore	1

#include "ssscore.h"

class SSCScore : public SSScore {
public:
	// statics
	SydFileSpec	scoreFileSpec;

	SSCScore(ModList * mList, short h, short v);

	// Local Functions
	char *GrabParam(char *p, int pNbr, ssfloat *pStore, Boolean *gotIt);

	// Overrides
	void GenerateScore(SSModule *callingMod);
	void Load(FILE* ar);
	void Copy(SSModule *ss);
  // int ProcessDoubleClick();
  // void Save(FILE* ar);
	// char* GetLabel();
};



#endif
