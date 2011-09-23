// SSExpression.h
#ifndef _H_SSExpression
#define _H_SSExpression	1

#include "ssmodule.h"
#include "expmgr.h"

class SSExpression : public SSModule {
	// Inputs and outputs of compiler
	ExpRec				exp;

public:
	// Overrides
	SSExpression(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Copy(SSModule *ss);
	void	Save(FILE* ar);
	void	Load(FILE* ar);
	void	Reset(SSModule *callingMod);
//  int   ProcessDoubleClick();
//	char*	GetLabel();
//	void	ComputeBounds(int x, int y);
//	void	Draw(Rect *rr);

};

#endif
