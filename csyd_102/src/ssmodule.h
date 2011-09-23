// ssmodule.h
#ifndef _H_SSModule
#define _H_SSModule	1

#include "modlist.h"
// #include "instrmnt.h"

#ifdef UI_FEATURES
#define StandardModuleSizeH	32
#define StandardModuleSizeV	32
#endif

#define MaxInputs			32

typedef struct LinkDesc {
//	short		type;
#ifdef UI_FEATURES
	RGBColor	color;
#endif
	char		*desc;
	char		*varName;
} LinkDesc, *LinkDescPtr;

typedef struct ExpRec {
	char			exp[256];
	unsigned char	cExp[2058];
	int 		cFlags;
	ssfloat		eConst;
} ExpRec, *ExpRecPtr;

class SSModule {
	long			lastClick;
	LinkDesc		linkDesc[MaxInputs+1];

public:
	ModList *		parList;
	SSModule		*callingMod;
	char			label[32];
	int				nbrSupportedLinks;
	int				moduleType;
	int				id;

  // used for stereo mode
  ssfloat   lastRightSample;
  ssfloat   lastRightInput;


#ifdef UI_FEATURES
  bool      selected;
	Rect			bounds,cellBounds;
#endif

	struct		{
		SSModule	*link;
		int			inputType;
		int			destID;
		char		fromLabel[32];	// Temorary - used for loading
	} inputs[MaxInputs];
	int				nbrInputs;

	// Dynamic Vars
	// ssfloat			lastTime,lastOutput;

	// Replace with correct c++ stuff
	// virtual void	Initialize(short itsType, short posh, short posv);
	SSModule(short itsType, ModList * mList, short posh, short posv);
	virtual ~SSModule();

	// not intended to be overridden...
	ssfloat	MixInputs(int type, SSModule *callingMod);
	// ssfloat	MixInputsAtten(int type, ssfloat pTime);
	int		CountInputs(int type);
	int		CountInputTypes();
	void	DescribeLink(int linkNbr, char *desc, char *varName,
					  int r=0, int g=0, int b=0);
	bool	ContainsMod(int id);

	int		NameToSignalType(char *name);
	void	SetLabel(char *label);
	// int		GetPatchCommand(Point p, int linkIdx);
	void	ComputeName(int n);

	// Expressions
	int		CompileExp(ExpRecPtr exp);
	ssfloat	ResetExp(ExpRecPtr exp, SSModule *callingMod);
	ssfloat	SolveExp(ExpRecPtr exp, SSModule *callingMod);
	void	ClearExp(ExpRecPtr exp);
	void	CopyExp(ExpRecPtr src, ExpRecPtr dst);
	int		InitExp(ExpRecPtr src, char *str);
#if !macintosh
	void	InitCSExp(ExpRecPtr exp, char *str);
#endif
	int		PrintfExp(ExpRecPtr src, char *tmp,...);
	int		LoadExp(FILE* ar, char *lab, ExpRecPtr exp);

	// unlikley to be overwritten
	// virtual void	ProcessMouseClick(Point p, EventRecord *er);
	virtual LinkDescPtr GetLinkDesc(int linkType);
	// virtual void	DragNewLink(Point p, int suggestedType);
	// virtual void	DragModule(Point p);
	virtual void	CopyAll(SSModule *ss);

	//	virtual void	ShowLabel(char *text);

	// likely to be overwritten
#ifdef UI_FEATURES
  virtual void  GetInputDock(Point *p, SSModule *src);
  virtual void  GetOutputDock(Point *p, SSModule *dst);
  virtual void  GetOutputDock(Point *p, Point ref);
  virtual void  AddInput(SSModule *ss, int inputType);
  void      ComputeLinksRect(Rect *r);
  virtual void  Select(bool flag);
  virtual void  Draw(Rect *r);
  virtual void  ShowLabel(char *cStr);
	virtual void	ComputeBounds(int x, int y);
  virtual int   ProcessDoubleClick();
  virtual char* GetLabel();
  virtual bool  GetOverlay(char *overlay, int* x, int* y);
  virtual void  Save(FILE* ar);
#endif
	virtual ssfloat	GenerateOutput(SSModule *callingMod); // Uses global gTime;
  virtual ssfloat  GenerateOutputTime(SSModule *callingMod, ssfloat pTime);
  virtual ssfloat getRightSample();
  virtual ssfloat getRightInput();

	virtual bool	GetFolderSig(int n, ssfloat *retVal);
	// virtual bool	IsInFolder();
	virtual bool	IsInScore();
	virtual ssfloat GetInstParameter(int n);
	virtual ssfloat GetNoteTime();
	virtual ssfloat GetIValue();
	virtual ssfloat GetAValue();
	virtual ssfloat GetKeyValue();
	virtual void	Reset(SSModule *callingMod);
	virtual void	CleanUp();
	virtual	void	Copy(SSModule *ss);
	virtual void	Load(FILE* ar);
};

extern int gModAlloc;

#endif
