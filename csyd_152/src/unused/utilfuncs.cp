// UtilFuncs.c

#include "SS.h"

#include <math.h>
#include <stdarg.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Random number generator
//       Source:  Stephen K. Park and Keith W. Miller, "Random Number
//   Generators:  Good Ones Are Hard to Find", Communications of the ACM,
//                vol. 31, p. 1192  (October 1988).

#define	R_A	16807L
#define	R_M	2147483647L
#define R_Q	127773L
#define R_R	2836L

long 	gSeed = 1;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long GetSRand(void)
{
	return gSeed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MySRand(long s)
{
	gSeed = s;
	if (gSeed == 0)
		gSeed = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Randomize(void)
{
	unsigned long t;

	GetDateTime(&t);
	MySRand(t);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long LongRandom(void)
{
	long	hi,lo,test;

	hi   = gSeed / R_Q;
	lo   = gSeed % R_Q;
	test = R_A * lo - R_R * hi;
	if (test > 0)
    gSeed = test;
	else
	  gSeed = test + R_M;
	return gSeed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ssfloat DoubleRandom(void)
{
	return LongRandom() / (ssfloat) R_M;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

short MyRandom(short max)
{
	return (short) (DoubleRandom() * max);
}

#define ErrorALRT	10001

void PrintfItem(DialogPtr dp, int item, char *tmp,...)
{
	va_list		args;
	char		msg[256];

	va_start(args,tmp);
	vsprintf(msg,tmp,args);
	va_end(args);
	CtoPstr(msg);
	SetText(dp,item,(StringPtr) msg);
}

int ScanfItem(DialogPtr dp, int item, char *tmp,...)
{
	va_list		args;
	char		msg[256];
	int			n;

#if !THINK_C
	int vsscanf(char *, const char *, _Va_list);
#endif
	return 0;
	
	GetText(dp,item,(StringPtr) msg);
	PtoCstr((StringPtr) msg);

	va_start(args,tmp);
#if !THINK_C
	n = vsscanf(msg,tmp,args);
#endif
	va_end(args);
	return n;
}

void SetText(DialogPtr dial, int item, StringPtr text)
{
	Handle handle;
	short type;
	Rect r;

	GetDItem(dial,item,&type,&handle,&r);
	SetIText(handle,text);
}

void GetText(DialogPtr dial, int item, StringPtr text)
{
	Handle handle;
	short type;
	Rect r;

	GetDItem(dial,item,&type,&handle,&r);
	GetIText(handle,(StringPtr) text);
}

// Set a control's state (such as a checkbox or a radio button)
//
void SetControl(DialogPtr dial, int item, int value)
{
	Handle handle;
	short type;
	Rect r;

	GetDItem(dial,item,&type,&handle,&r);
	SetCtlValue((ControlHandle) handle,value);
}

// Get a control's state (such as a checkbox or a radio button)
//
int GetControl(DialogPtr dial,register int item)
{
	Handle handle;
	short type;
	Rect r;

	GetDItem(dial,item,&type,&handle,&r);
	return GetCtlValue((ControlHandle) handle);
}

// Toggle a control's state (such as a checkbox or a radio button)
//
void ToggleControl(DialogPtr dial, int item)
{
	int value;
	value = GetControl(dial,item);
	SetControl(dial,item,!value);
}

Boolean PtInLine(Point p, Point sPos, Point ePos)
{
	Rect	r;
	int		xD,yD,xD2,yD2;

	Pt2Rect(sPos,ePos,&r);
	InsetRect(&r,-1,-1);
	if (!PtInRect(p,&r))
		return false;

	xD2 = p.h - sPos.h;
	yD2 = p.v - sPos.v;
	if (xD2 == 0 && yD2 == 0)
		return true;

	xD = ePos.h - sPos.h;
	yD = ePos.v - sPos.v;

	if (xD == 0 && yD == 0)
		return EqualPt(p,sPos);

	if ((xD < 0) != (xD2 < 0) ||
		(yD < 0) != (yD2 < 0))
			return false;

	if (abs(xD) > abs(yD)) {
		// Define slope as yD/xD
		if (xD2 == 0)
			return false;
		return fabs((ssfloat)yD/xD - (ssfloat)yD2/xD2) < 0.1;
	}
	else {
		// Define slope as xD/yD
		if (yD2 == 0)
			return false;
		return fabs((ssfloat)xD/yD - (ssfloat)xD2/yD2) < 0.1;
	}
}

ssfloat GetFloatItem(DialogPtr dp, short itemNbr)
{
	char	tmp[32];
	double	dmy = 0.0;
	GetText(dp,itemNbr,(StringPtr) tmp);
	PtoCstr((StringPtr) tmp);
	sscanf(tmp,"%lf",&dmy);
	return (ssfloat) dmy;
}

int GetIntItem(DialogPtr dp, short itemNbr)
{
	char	tmp[32];
	int		dmy = 0;
	GetText(dp,itemNbr,(StringPtr) tmp);
	PtoCstr((StringPtr) tmp);
	sscanf(tmp,"%d",&dmy);
	return dmy;
}

void GetStrItem(DialogPtr dp, short itemNbr,char *str)
{
	GetText(dp,itemNbr,(StringPtr) str);
	PtoCstr((StringPtr) str);
}

void ShortToLSB(int16 *sn)
{
	uint16 *n = (uint16 *) sn;
	*n = (uint16)( ((*n >> 8) & 0x00FFL) |	((*n << 8)  & 0xFF00L));
}

void LongToLSB(int32 *sn)
{
	uint32 *n = (uint32 *) sn;
	*n =  ((*n >> 24) & 0x000000FFL) |
			((*n >> 8)  & 0x0000FF00L) |
			((*n << 8)  & 0x00FF0000L) |
			((*n << 24) & 0xFF000000L);
}

void CheckAllocation(void *ptr)
{
	if (ptr == NULL)
		ErrorExit("Out of Memory");
}

int	gAllocCtr;

Ptr	MyNewPtrClear(long size)
{
	Ptr	p;
	++gAllocCtr;
	p = NewPtrClear(size);
	if (p == NULL) {
		long	freeMem;
		freeMem = FreeMem();
		DebugFunc("\pNULL Ptr Allocation");
	}
	return p;
}

void MyDisposePtr(Ptr p)
{
	if (p == NULL) {
		ErrorExit("Dispose NULL Ptr Problem");
	}
	--gAllocCtr;
	if (gAllocCtr < 0) {
		ErrorExit("DisposePtr Problem");
	}
	DisposePtr(p);
}

void MyAssert(Boolean fact)
{
	if (!fact)
		DebugFunc("\pAssertion Failed");
}

void DebugFunc(StringPtr param)
{
	SysBreakStr(param);
}

void ErrorExit(char *str,...)
{
	char tbuf[128];
	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	CtoPstr(tbuf);
	ParamText((StringPtr) tbuf,"\p","\p","\p");
	Alert(ErrorALRT,NULL);
	exit(1);
}

void ErrorMessage(char *str,...)
{
	char tbuf[128];
	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	CtoPstr(tbuf);
	ParamText((StringPtr) tbuf,"\p","\p","\p");
	Alert(ErrorALRT,NULL);
}

void WriteFileLine(short fRefNum, char *tmp,...)
{
	char 	tbuf[512];
	long	count;
	va_list args;
	va_start(args,tmp);
	vsprintf(tbuf,tmp,args);
	va_end(args);
	count = strlen(tbuf);
	if (fRefNum)
		FSWrite(fRefNum,&count,tbuf);
}

Boolean CanPaste(long type)
{
	long	scrapOffset;
	return GetScrap(NULL,type,&scrapOffset) > 0;
}

int grepstr(char *pat, char *str)
{
	if (re_comp(pat))	// Compile Error?
		return 0L;
	return re_exec(str);
}

// Note: this function must follow a grep
char *grepsub(char *rep, char *tbuf)
{
	if (re_subs(rep,tbuf) == 0)		// 6/13/95 - added error check
		strcpy(tbuf,rep);
	return tbuf;
}
