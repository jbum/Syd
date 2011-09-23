// SSGAssign.h
#ifndef _H_SSGAssign
#define _H_SSGAssign	1

#include "ssmodule.h"
#include "expmgr.h"

class SSGAssign : public SSModule {
	// Inputs and outputs of compiler
	ExpRec				gNbrExp;
	ExpRec				valExp;
	char				desc[64];

public:
	// Overrides
	SSGAssign(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Copy(SSModule *ss);
	void	Load(FILE* ar);
	void	Reset(SSModule *callingMod);
#ifdef UI_FEATURES
  int    ProcessDoubleClick();
  void Save(FILE* ar);
	char*	GetLabel();
	Boolean GetOverlay(char *overlay, int* x, int* y);
#endif
};

#endif
