// SSFilter.h - 2nd Order Section
//
#ifndef _H_SSFilter
#define _H_SSFilter	1

#include "ssmodule.h"

#define FF_Recursive	1

class SSFilter : public SSModule {
	// Static
	ExpRec		a0Exp,a1Exp,a2Exp,b1Exp,b2Exp;
	int			flags;

	// Dynamic
	ssfloat		oldY[2],oldX[2];
public:
	// Overrides
	SSFilter(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Reset(SSModule *callingMod);
	int		ProcessDoubleClick();
	void	Copy(SSModule *ss);
	void	Save(FILE* ar);
	void	Load(FILE* ar);
};

#endif
