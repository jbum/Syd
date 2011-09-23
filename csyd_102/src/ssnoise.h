// SSNoise.h
#ifndef _H_SSNoise
#define _H_SSNoise	1

#include "ssmodule.h"

#define	R_A	16807L
#define	R_M	2147483647L
#define R_Q	127773L
#define R_R	2836L



class SSNoise : public SSModule {
	Boolean	randomize;
	int32	startSeed;
	int32 	seed;

public:
	// Overrides
	SSNoise(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	int ProcessDoubleClick();
	void Copy(SSModule *ss);
	void Save(FILE* ar);
	void Load(FILE* ar);
	long LongRandom(void);
	void MySRand(int32 s);
	void Randomize(void);
	ssfloat DoubleRandom(void);
	void Reset(SSModule *callingMod);

};

#endif
