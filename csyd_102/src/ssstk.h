// SSStk.h
#ifndef _H_SSStk
#define _H_SSStk	1

// #include "Stk.h"
#include "Instrmnt.h"

#include "ssmodule.h"
#include "expmgr.h"

// #define STK_DLOG		215

// Variants
enum { STK_Normal };

class SSStk : public SSModule {
public:

	// Static
	int			variant;
	ExpRec		freqExp,durExp,ampExp,decayExp;

	// Dynamic
	Instrmnt *b3;

	ssfloat		dur,rho,S,S1,C,x1,y1,z;
	long		p,count;
	ssfloat		*d,*sp;
	Boolean		error;

	// Overrides
	SSStk(ModList * mList, short h, short v);

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
