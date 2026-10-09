// SSOscillator.h
#ifndef _H_SSPluck
#define _H_SSPluck	1

#include "ssmodule.h"
#include "expmgr.h"

// #define PluckDLOG		215

// Variants
enum { KP_Normal };

class SSPluck : public SSModule {
public:
	// Static
	int			variant;
	ExpRec		freqExp,durExp,ampExp,decayExp;

	// Dynamic
	ssfloat		dur,rho,S,S1,C,x1,y1,z;
	long		p,count;
	ssfloat		*d,*sp;
	Boolean		error;

	// Overrides
	SSPluck(ModList * mList, short h, short v);

	void	Initialize(short itsType, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Reset(SSModule *callingMod);
	void	CleanUp();
	void	Copy(SSModule *ss);
	void	Load(FILE* ar);
  // void  Save(FILE* ar);
  // int   ProcessDoubleClick();
};

#endif
