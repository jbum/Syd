// SSFTable.h
#ifndef _H_SSFTable
#define _H_SSFTable	1

#include "ssmodule.h"
#include "expmgr.h"

#define FW_NoWrap	0
#define FW_Wrap		1
#define FW_Pin		2
#define FW_Interp	4

class SSFTable : public SSModule {
	// Dynamics
	ssfloat			*table;
	// Statics
	ExpRec			tabExp;
	int				tabSize,tabNbr;
	// ssfloat		scale,offset; (old constant versions)
public:
	// Locals
	ssfloat	RetrieveTableValue(ssfloat pTime);
	ssfloat	RetrieveTableValueI(ssfloat pTime);
	void	FillTable(SSModule *callingMod);

	// Overrides
	SSFTable(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	ssfloat	GenerateOutputTime(SSModule *callingMod, ssfloat pTime);
	void	Reset(SSModule *callingMod);
	void	CleanUp();
	int		ProcessDoubleClick();
	void	Copy(SSModule *ss);
	void	Save(FILE* ar);
	void	Load(FILE* ar);
};

ssfloat GetFTableEntry(SSModule *callingMod, int tNbr, ssfloat pTime, int flags);

#endif
