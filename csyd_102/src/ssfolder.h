// SSFolder.h

#ifndef _H_SSFolder

#define _H_SSFolder	1

#include "ssmodule.h"

class SSFolder : public SSModule {
public:
	// locals
	ModList	* instr;

	// statics
  SydFileSpec instFileSpec;

	SSFolder(ModList * mList, short h, short v);

	// Overrides
	void Load(FILE* ar);
	void Reset(SSModule *callingMod);
	void CleanUp();
	ssfloat GenerateOutput(SSModule *callingMod);
	Boolean GetFolderSig(int n, ssfloat *retVal);
	void Copy(SSModule *ss);
  // Boolean IsInFolder();
//  int ProcessDoubleClick();
//  void Save(FILE* ar);
//  void Draw(Rect *rr);
//  void ComputeBounds(int x, int y);
//  char* GetLabel();
};



#endif
