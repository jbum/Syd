// SSFInput.h
#ifndef _H_SSFInput
#define _H_SSFInput	1

#include "ssmodule.h"
#include "expmgr.h"

class SSFInput : public SSModule {
	// Inputs and outputs of compiler
	int					fNbr;
	char				desc[64];
	ExpRec				defExp;

public:
	// Overrides
	SSFInput(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Copy(SSModule *ss);
	void	Load(FILE* ar);
	void	Reset(SSModule *callingMod);
#ifdef UI_FEATURES
  void  ComputeBounds(int x, int y);
  void  Draw(Rect *rr);
  int    ProcessDoubleClick();
  void Save(FILE* ar);
	char*	GetLabel();
	Boolean GetOverlay(char *overlay, int* x, int* y);
#endif
};

#endif
