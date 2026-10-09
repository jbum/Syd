// SSmaraca.h
#ifndef _H_SSMaraca
#define _H_SSMaraca	1

#include "ssmodule.h"
#include "expmgr.h"

// #define MaracaDLOG		217


class SSMaraca : public SSModule {
public:
	// Static
	ExpRec		resfreqExp,respoleExp,probExp,sysdecayExp,snddecayExp;

	// Possibly static later...
	long	numBeans;

	// Dynamic
	ssfloat	resfreq,respole,prob,sysdecay,snddecay;
	ssfloat	gain,output[2],input,shakeEnergy,temp,sndLevel;
	ssfloat	coeffs[2];
	long	i;

	// Overrides
	SSMaraca(ModList * mList, short h, short v);

	void	Initialize(short itsType, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	int		ProcessDoubleClick();
	void	Reset(SSModule *callingMod);
	void	CleanUp();
	void	Copy(SSModule *ss);
	void	Save(FILE* ar);
	void	Load(FILE* ar);
};

#endif
