// ModList.h
#ifndef _H_ModList
#define _H_ModList	1

// #include "carchive.h"

#define MaxModules	256
#define MaxTimes	16

enum  {MT_Output,MT_Oscillator, MT_Envelope, MT_Mixer, MT_Filter,
	   MT_Butter, MT_Smooth, MT_Noise, MT_Delay, MT_Threshhold,
	   MT_SampleAndHold, MT_Amplifier, MT_Inverter, MT_Expression, MT_Folder,
	   MT_RandScore,  MT_CScore, MT_FInput, MT_PInput, MT_FTable,
	   MT_SampleFile, MT_Pluck, MT_Maraca, MT_HammerBank, MT_HammerActuator,
	   MT_GAssign, 
     MT_SkiniScore,
#ifdef USE_STK
     MT_STK,
#endif
	   MT_NbrModules};

typedef struct ModuleDesc {
	short		itsType;
	char		*name;
	char		*desc;
} ModuleDesc;

extern struct ModuleDesc mDesc[];

class MainWindow;		// Forward reference
class SSModule;
class PatchOwner;

class ModList {
	// File stuff
	ssfloat 			pTimes[MaxTimes];
	int					timeStackCtr;

public:
	int			nbrModules;
	ssfloat		sampleDuration;
	ssfloat		sampleRate;
	SSModule	*mods[MaxModules];
	PatchOwner	*itsOwner;
	ssfloat		pTime;

//	ModList(MainWindow *itsOwner);	// replaces LoadInstrument
//	ModList(MainWindow *itsOwner, FSSpec *spec);		// Create from Spec
	ModList(PatchOwner* itsOwner);
	ModList(PatchOwner* itsOwner, SydFileSpec *fsSpec);
	~ModList();

	ModList *CloneInstrument(SSModule *rootModule);

	bool OpenSpec(SydFileSpec *fsSpec);
	bool OpenArchive(FILE* ar);
	bool Save(FILE *ar);

	void ResolveModuleLinks();
	SSModule *GetModule(int id);
	int GetModuleID(char *label);
	bool ContainsNamedMod(char *name);
	void AddModule(int n);
	void LoadModule(FILE* ar);
	void ResetInstruments(SSModule *callingMod);
	void CleanUpInstruments();
	void DisposeModuleList();
	char *GetNextInputLine(FILE *ar, char *pat, char *buf);
	char *GetNextInputLine(char *pat);
	SSModule *GetPickedModule(Point p);
	SSModule *GetOutputModule();
	void DeleteLinks(SSModule *ss);

	// Module Descriptions
	char *GetModuleName(int n);
	char *GetModuleDesc(int n);
	int NameToModuleType(char *name);
	void Delete();
	void InitTime(ssfloat timeVal);
	void PushTime(ssfloat timeVal);
	void PopTime();
};



#endif
