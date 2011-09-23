// SSButter.h
#ifndef _H_SSButter
#define _H_SSButter	1

#include "ssmodule.h"

enum {BT_LoPass, BT_HiPass, BT_BandPass, BT_BandReject};
#define ROOT2 (1.4142135623730950488)

class SSButter : public SSModule {
	// Static
	ExpRec		freqExp,bwExp;
	int			filterType;

	// Dynamic
	ssfloat		oldFreq,oldBW;
	ssfloat		pidsr,a0,a1,a2,b1,b2,y1,y2;
public:
	// Overrides
	SSButter(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Reset(SSModule *callingMod);
	int		ProcessDoubleClick();
	void	Copy(SSModule *ss);
	void	Save(FILE* ar);
	void	Load(FILE* ar);
};

#endif
