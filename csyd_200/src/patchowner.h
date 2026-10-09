#ifndef _H_PatchOwner

#define _H_PatchOwner	1


// This was mostly UI stuff in original syd.
//
//

class ExpMgr;
class ModList;
class SSModule;
class SSStorage;
class SSStoreMem;
class SSStoreFile;

#ifdef UI_FEATURES
class CSydApp; // was CSydView
#endif


#define MaxGlobals		256

enum { WS_Idle, WS_Synthesize, WS_Playback, WS_Abort};

class PatchOwner {
private:
	// Module Dragging

#ifdef UI_FEATURES
	bool		dragging;
	SSModule*	dragMod;

	// Link Dragging
	bool		linkDragging;
	Point		linkAnchor,linkPoint;
	SSModule	*linkTarget,*linkSource;
	int			linkType;

	// Popup Menus for Links
	SSModule	*menuMod;
	int			menuLinkNbr;
#endif

public:
  int     windowState;
	ExpMgr*		expMgr;
	ssfloat		gTime,timeInc;
	ModList		*mainInst;
	ssfloat		globalVars[MaxGlobals];

	SSModule*		outMod;
	SSStorage*		storageMod;
	SSStoreMem*		storeMem;
	SSStoreFile*	storeFile;
	bool			storeToFile;

	long			nbrSamples;
	long			sampleNbr;
	long			startSynthTime;
	char*			StatString();
	long			gLastFreeMem, gLastDelta;
  bool      listenFlag, showMemory, isGraphing;
#ifdef UI_FEATURES
  long      lastGraphTime;
	CSydApp*	itsOwner; // was CSydView
#endif

private:
	// void FullRefresh();
	void InitSynth(void);
	void DisposeAllModules();
	void InitGlobals();
	// void StartLinkDrag(SSModule* src, Point p, int linkType);
	// void StartGadgetDrag(SSModule *nn);
	void SetOutputType(int fileType, int isStereo, SydFileSpec *fsSpec);
	// void PlayBuffer();
	// void SpinCursor();

public:
#ifdef UI_FEATURES
  PatchOwner(CSydApp* itsOwner);
#else
  PatchOwner();
#endif
	// void RefreshModules(CDC* pDC);	// Windows-specific
	// void PartialRefresh(Rect *rr);	// Passed from SSModule
	// void ShowDrag(CDC* pDC);
	// void HandleGadgetDrag(CPoint point);
	// void HandleModuleClick(SSModule* nn, Point p, int nFlags);
	// void HandleLinksClick(Point p, int nFlags);
	// void HandleModuleDClick(SSModule* nn, Point p, int nFlags);
	// int HandleMouseMove(CPoint point);
	// bool HandleDrag(CPoint point);
	// void HandleStopDrag(void);
	// void LinkMenuCommand(int cmd);
	// void DeselectAll();
	void Delete();
	void PerformSynthesis();
	void SynthIdle();
	void AbortSynthesis();
	void CleanUpSynthesis();
	void Playback();
	void AssignGlobal(int gNbr, ssfloat value);
	ssfloat RetrieveGlobal(int gNbr);
	// void GetMaxDimensions(Point* p);
	// void ToggleListen();
	// void ToggleGraphing();
};

#endif
