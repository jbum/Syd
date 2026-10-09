// SSSampleFile.h

#ifndef _H_SSSampleFile

#define _H_SSSampleFile	1

#include "ssmodule.h"
#include "expmgr.h"
// #include "mainwin.h"
#include "ssstorefile.h"

#define SF_Interpolate	1
#define SF_Retrograde	2	// Not implemented yet

class SSSampleFile : public SSModule {
public:
	// locals
	SSStoreFile	*fileStore;
	// statics
	SydFileSpec	sampleFileSpec;
	ExpRec	timeScaleExp;
	int		flags;

	SSSampleFile(ModList * mList, short h, short v);

	// Overrides
	void Load(FILE* ar);
	void Reset(SSModule *callingMod);
	void CleanUp();
	ssfloat GenerateOutput(SSModule *callingMod);
	void Copy(SSModule *ss);
  /*
	char *GetLabel();
  int ProcessDoubleClick();
  void Save(FILE* ar);
  */
};



#endif
