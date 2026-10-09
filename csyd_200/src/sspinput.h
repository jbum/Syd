// SSPInput.h
#ifndef _H_SSPInput
#define _H_SSPInput	1

#include "ssmodule.h"
#include "expmgr.h"

class SSPInput : public SSModule {
	// Dynamic
	ssfloat				pValue;

	// Inputs and outputs of compiler
	int					pNbr;
	char				desc[64];
	ExpRec				defExp;

public:
	// Overrides
	SSPInput(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Copy(SSModule *ss);
	void	Load(FILE* ar);
	void	Reset(SSModule *callingMod);
#ifdef UI_FEATURES
  int   ProcessDoubleClick();
  void  Save(FILE* ar);
	char*	SSPInput::GetLabel();
	Boolean SSPInput::GetOverlay(char *overlay, int *x, int *y);
#endif

};

#endif
