#ifndef _H_SS
#define _H_SS

#define SYD_TITLE   "CSyd"
#define SYD_VERSION "1.5.1"
#define SYD_AUTHOR  "Jim Bumgardner (www.krazydad.com)"

#include "../sydlocal.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
// #include <types.h>
#include <time.h>
#include <stdarg.h>

#define WaveTableSize		16384

// extern bool	    gQuitFlag,gSuspended;
extern ssfloat	*gSinTable,*gSquareTable;
extern int		  gAllocCtr;
extern int      gVerbose;
extern bool     gDurationOverride;

// Utils
// Boolean PtInLine(Point p, Point sPos, Point ePos);

/*
#if macintosh
int ScanfItem(DialogPtr dp, int item, char *tmp,...);
void PrintfItem(DialogPtr dp, int item, char *tmp,...);
void SetText(DialogPtr dial, int item, StringPtr text);
void GetText(DialogPtr dial, int item, StringPtr text);
void SetControl(DialogPtr dial, int item, int value);
int GetControl(DialogPtr dial,register int item);
void ToggleControl(DialogPtr dial, int item);
ssfloat GetFloatItem(DialogPtr dp, short itemNbr);
int GetIntItem(DialogPtr dp, short itemNbr);
void GetStrItem(DialogPtr dp, short itemNbr,char *str);
#endif
*/

void CheckAllocation(void	*ptr);

void InitWaveTables();

void *MyNewPtrClear(size_t size);
void MyDisposePtr(void *p);

// Reporting...
void DebugFunc(char *param,...);
void MyAssert(bool fact);
void ErrorExit(char *str,...);
void ErrorMessage(char *str,...);
void LogMessage(char *str,...);
void AddLogIndent(int n);

//void AppProcessCommand(short menuID, short menuItem);
//void AppAdjustMenus(void);
//Boolean CanPaste(long type);

/*
#if macintosh
void WriteFileLine(short fRefNum, char *tmp,...);
#else
void WriteFileLine(FILE* ar, char *tmp,...);
#endif
*/
unsigned char*SuckFilefromSpec(SydFileSpec *fsSpec);
// void WriteFileLine(FILE *ar, char *tmp,...);


// void ShowVersionNumber();

// RegExps
/*
char *re_comp(char *pat);
int re_exec(register char *lp);
void re_modw(register char *s);
int re_subs(register char *src, register char *dst);

// Hi Level
int grepstr(char *pat, char *str);  // Returns non-zero if match
char *grepsub(char *rep, char *tbuf);
*/


#define NEF_Tempo	1

typedef struct NoteEvent {	// Parsed note event
	int		nbrOptParams;
	int		flags;		// might be tempo or wave table event
	int		p1;
	ssfloat	p2,p3;	// start, duration
	ssfloat	op[1];	// P4 thru N
} NoteEvent, *NoteEventPtr;

#define Assert(exp)		if (!(exp))	DebugStr("\pAssertion Failed")


int32 GetSRand(void);
void MySRand(int32 s);
int32 LongRandom(void);
ssfloat DoubleRandom(void);
void Randomize(void);

#endif
