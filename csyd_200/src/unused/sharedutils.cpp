// SharedUtils.c  - used on both mac and windows

#include "ss.h"

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
	MySRand(time(NULL));
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

/*
Boolean PtInLine(Point p, Point sPos, Point ePos)
{
	Rect	r;
	int		xD,yD,xD2,yD2;

	LocalPt2Rect(sPos,ePos,&r);
	LocalInsetRect(&r,-1,-1);
	if (!LocalPtInRect(p,&r))
		return false;

	xD2 = p.h - sPos.h;
	yD2 = p.v - sPos.v;
	if (xD2 == 0 && yD2 == 0)
		return true;

	xD = ePos.h - sPos.h;
	yD = ePos.v - sPos.v;

	if (xD == 0 && yD == 0)
		return LocalEqualPt(p,sPos);

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
*/

void ShortToMSB(int16 *sn)
{
	uint16 *n = (uint16 *) sn;
	*n = (uint16)( ((*n >> 8) & 0x00FFL) |	((*n << 8)  & 0xFF00L));
}

void LongToMSB(int32 *sn)
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
//		long	freeMem;
//		freeMem = FreeMem();
		DebugFunc("NULL Ptr Allocation");
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
		DebugFunc("Assertion Failed");
}

#if 0

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

#endif

#if 0

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

#endif

