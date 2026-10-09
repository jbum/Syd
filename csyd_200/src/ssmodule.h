// ssmodule.h
#ifndef _H_SSModule
#define _H_SSModule	1

#include "modlist.h"
#include <new>
#include <atomic>
#include <vector>
#include <stdlib.h>
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

// A compiled expression decoded for ExpMgr::EvalExp: the tokens it evaluates often (constants,
// time, note parameters, arithmetic) as instructions of their own, and every other token as
// XI_Token, run by ExpMgr::Step from its place in cExp.
enum { XI_End = 0, XI_Const, XI_Time, XI_Param, XI_Add, XI_Sub, XI_Mul, XI_Div, XI_Neg, XI_Pow,
	   XI_Square, XI_Token };
#define MaxExpInstrs	64

typedef struct ExpInstr {
	ssfloat			k;		// XI_Const: the constant
	unsigned short	op;
	unsigned short	at;		// XI_Param: the parameter number; XI_Token: its offset in cExp
} ExpInstr;

typedef struct ExpRec {
	char			exp[256];
	unsigned char	cExp[2058];
	int 		cFlags;
	ssfloat		eConst;
	// Decoded once when compiled and shared, read-only, by every copy (each note of an
	// instrument has its own modules): NULL to evaluate cExp with EvalF.
	const ExpInstr	*prog;
} ExpRec, *ExpRecPtr;

class SSModule {
	long			lastClick;
	LinkDesc		linkDesc[MaxInputs+1];

public:
	// Modules start zeroed, as fresh memory from the OS is. Several leave members to Reset(),
	// and a module can be evaluated before its Reset runs (another module's Reset solves an
	// expression that reads it): an oscillator then sees lastTime == 0 == pTime and returns its
	// lastOutput, 0. Patches rely on that. When new returned recycled memory instead, 1.5.1
	// read a stale lastTime and lastOutput and rendered some patches differently on each run.
	static void *operator new(size_t n)	{ void *p = calloc(1, n); if (!p) throw std::bad_alloc(); return p; }
	static void operator delete(void *p)	{ free(p); }

	ModList *		parList;
	SSModule		*callingMod;
	char			label[32];
	int				nbrSupportedLinks;
	int				moduleType;
	int				id;
	std::vector<ExpRec *>	exps;	// this module's expressions (InitExp), for ExpUnsafe

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
	bool			ExpUnsafe();	// an expression of this one fails ExpMgr::BlockSafe
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

extern std::atomic<int> gModAlloc;

#endif
