// MainWin.h - inherits from ObjectWin
#ifndef _H_MainWin
#define _H_MainWin	1

#if macintosh

// #include <QDOffscreen.h>

#include "ObjectWindow.h"
// #include "SSModule.h"
// #include "ExpMgr.h"
// #include "SSStoreMem.h"
// #include "SSStoreFile.h"

#define MainWindowID		128

#define MaxGlobals			256

// This should match order of menus (in order for
//	duplicate to work...

// Forward Refs
class SSModule;
class ExpMgr;
class SSStorage;
class SSStoreMem;
class SSStoreFile;
class ModList;

#define NEF_Tempo	1

typedef struct NoteEvent {	// Parsed note event
	int		nbrOptParams;
	int		flags;		// might be tempo or wave table event
	int		p1;
	ssfloat	p2,p3;	// start, duration
	ssfloat	op[1];	// P4 thru N
} NoteEvent, *NoteEventPtr;

enum	{CT_PointyHand,	CT_Eyedropper,	CT_CrossHair,
		 CT_Pencil,	CT_PaintBrush,	CT_IBeam,
		 CT_Eraser, CT_PaintBucket, CT_OpenHand,
		 CT_ClosedHand,	CT_Watch1, CT_Watch2,
		 CT_Watch3, CT_Watch4, CT_PatchCord,
		 CT_CutPatch, CT_NbrCursors};

extern struct ModuleDesc mDesc[];


#define HelpPanelHeight		20


#define BottomMargin		16
#define RightMargin			16

enum {WS_Idle,WS_Synthesize,WS_Playback, WS_Abort};

class MainWindow : ObjectWindow
{
	
	// Rendering stuff
	GWorldPtr			offWorld;
	PixMapHandle		offMap;

	RGBColor			frameColor,
						frameLtColor,
						frameDkColor;

	// Module stuff
	// long				allocBufferSize;


	// Gadget Panel Stuff
	Rect				gpRect,helpRect;
	StringPtr			helpPtr;

	// Export stuff
	long				dataLength;
	int					inType,outType;
	long				outSampleRate;
	long				dataOffset;
	// NoteEvent			defaultInstParams;
	// ssfloat				defaultInstFreq;	// keep with previous param

	// Synthesis Stuff
	// short				*op;
	int32				sx;
	SSModule			*outMod;
	SSStoreMem			*storeMem;
	SSStoreFile			*storeFile;
	Boolean				storeToFile;
	long				startSynthTime,lastGraphTime;
	ssfloat				globalVars[MaxGlobals];

public:
	int					windowState;
	long				nbrSamples;
	SSStorage			*storageMod;
	// Ptr					sampleOutput;
	Boolean				dirty,showMemory,listenFlag;
	ModList				*mainInst;
	ExpMgr				*expMgr;
	CursHandle			cursors[CT_NbrCursors];
	ssfloat				gTime,timeInc;
	Boolean				forceFullRefresh;
	FSSpec				fsSpec,esSpec;

	MainWindow();
	~MainWindow();

	// void			InitializeMainWindow();
	void			SSReset(void);
	void			SpinCursor();

	// File Stuff
	Boolean			ConfirmClose();
	void			SetOutputType(Boolean fileFlag, FSSpec *fileSpec);
	Boolean			SaveAs(void);
	Boolean			Save();
	void			Open();
	// void			OpenSpec(ModuleList *mList, FSSpec *spec);
	void			Open(FSSpec *spec);
	void			New();
	// void			WriteFileLine(short fRefNum, char *tmp,...);
	// char			*GetNextInputLine(char *pat);
	// void			LoadModule(ModuleList *mList);
	// void			ExportToFile(FSSpec *outFile);

	// Graphics
	void			InitSSDraw(void);
	void			RefreshRect(Rect *rr);
	void			Paint(Rect *rr);
	void			InitGraphics();
	void			RefreshModules(Rect *rr);
	void			FullRefresh(void);
	void			PartialRefresh(Rect *rr);
	void			DrawTableFrame(Rect *inFrame, Boolean invert);


	// Buttons
	void			DrawWindowButton(short buttonNbr, Boolean flag);
	void			SetActiveButtons(void);
	void			DrawWindowButtons(Rect *rr);
	Boolean			HandleButtonClick(Point p);
	void			PushButton(short buttonNbr);

	// Gadget Panel
	void			InitGadgetPanel();
	void			DrawGadgetPanel(Rect *rr);
	Boolean			HandleGadgetPanelClick(Point p);
	void			SetHelp(StringPtr ptr);
	void			PrintToHelp(char *tmp,...);
	void			GadgetIdle(Point p);

	// Module Drags
	void			ProcessModuleClick(SSModule *nn, Point p, EventRecord *er);
	void			DragModule(SSModule *nn, Point p);
	void			DragNewLink(SSModule *nn, Point p, int suggestedType);

	// Edits
	void			Cut();
	void			Delete();
	void			DeselectAll();
	void			SelectAll();
	void			Duplicate();
	Boolean			HasSelection();

	// Synth Stuff
	void			InitSynth(void);
	// void			InitTables();
	void			PerformSynthesis();
	void 			SynthIdle();
	void			CleanUpSynthesis();
	void			AbortSynthesis();

	void			PlayBuffer();
	void			DisposeAllModules();
	void			AssignGlobal(int gNbr, ssfloat value);
	ssfloat			RetrieveGlobal(int gNbr);
	void			InitGlobals();

	// Debugging
	char			*StatString();

	// Overrides
	void			Dispose(void);
	void			Draw(Rect *rr);
	void			HandleClick(Point p, EventRecord *theEvent);
	void			Idle(EventRecord *theEvent);
	void			ProcessKey(EventRecord *theEvent);
	void			Activate(Boolean activeFlag);
	void			ProcessCommand(short menuID, short menuItem);
	void			AdjustMenus();
};

extern MainWindow *mw;

#endif

#endif