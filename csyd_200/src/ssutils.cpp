#include "ss.h"

/*
void WriteFileLine(FILE* ar, char *str,...)
{
	char	tbuf[512];
	va_list	args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	if (tbuf[0] != 0 && tbuf[strlen(tbuf)-1] != '\n')
		strcat(tbuf,"\n");
	fputs(tbuf, ar);
}
*/

// Use a dialog here??
void ErrorMessage(char *str,...)
{
	char	tbuf[256];
	va_list	args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	LogMessage("ERROR: %s\n",tbuf);
}

void DebugFunc(char *str,...)
{
	char	tbuf[256];
	va_list	args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	LogMessage("DEBUG: %s\n",tbuf);
//	TRACE(tbuf);
}

void ErrorExit(char *str,...)
{
	char	tbuf[256];
	va_list	args;

	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	LogMessage("DEBUG: %s\n",tbuf);
	exit(0);
//	TRACE(tbuf);
	/// !!! Need to exit!
}

/*
BOOL LocalPtInRect(Point p, Rect *r)
{
	if (p.h >= r->left && p.h < r->right &&
		p.v >= r->top && p.v < r->bottom)
		return TRUE;
	else
		return FALSE;
}

void LocalOffsetRect(Rect *r, int xd, int yd)
{
	r->left += xd;
	r->right += xd;
	r->top += yd;
	r->bottom += yd;
}

void LocalInsetRect(Rect *r, int xd, int yd)
{
	r->left += xd;
	r->right -= xd;
	r->top += yd;
	r->bottom -= yd;
}

void LocalUnionRect(Rect *r1, Rect *r2, Rect *r3)
{
	r3->right = r1->right > r2->right? r1->right : r2->right;
	r3->left = r1->left > r2->left? r2->left : r1->left;
	r3->bottom = r1->bottom > r2->bottom? r1->bottom : r2->bottom;
	r3->top = r1->top > r2->top? r2->top : r1->top;
}

BOOL LocalSectRect(Rect *r1, Rect *r2, Rect *r3)
{
	CRect	cr1,cr2,cr3;
	BOOL	result;
	cr1.SetRect(r1->left,r1->top,r1->right,r1->bottom);
	cr2.SetRect(r2->left,r2->top,r2->right,r2->bottom);
	result = cr3.IntersectRect(cr1,cr2);
	LocalSetRect(r3,cr3.top,cr3.left,cr3.right,cr3.bottom);
	return result;
}

void LocalSetRect(Rect *r, int left, int top, int right, int bottom)
{
	r->left = left;
	r->top = top;
	r->right = right;
	r->bottom = bottom;
}

void LocalPt2Rect(Point p1, Point p2, Rect *r)
{
	r->left = p2.h < p1.h? p2.h : p1.h;
	r->right = p2.h > p1.h? p2.h : p1.h;
	r->top = p2.v < p1.v? p2.v : p1.v;
	r->bottom = p2.v > p1.v? p2.v : p1.v;
}

BOOL LocalEqualPt(Point p1, Point p2)
{
	return (p1.h == p2.h && p1.v == p2.v);
}
*/

std::atomic<int> gAllocCtr;

void *MyNewPtrClear(size_t size)
{
	void *p;
  ++gAllocCtr;
	p = (void *) malloc(size);
	if (p) {
		memset(p,0,size);
	}
  else {
    LogMessage("NULL Ptr Allocation");
  }
	return p;
}

void	MyDisposePtr(void *ptr)
{
  --gAllocCtr;
  if (gAllocCtr < 0) {
    ErrorExit("DisposePtr Problem");
  }
	free(ptr);
}

void GetEOF(FILE *cFile, long *fileSize)
{
	long	curPos = ftell(cFile);
	fseek(cFile,0,SEEK_END);
	*fileSize = ftell(cFile);
	fseek(cFile,curPos,SEEK_SET);
}

unsigned char*SuckFilefromSpec(SydFileSpec *fsSpec)
{
	FILE *cFile;
	unsigned char *p;
	long			fileSize,bytesRead;
  extern SydFileSpec sydSpec;

	if ((cFile = fopen(fsSpec->name,"rb")) == NULL)
	{
      char  oname[512], iname[512];
      char  *p;
      strcpy(iname,fsSpec->name);
      strcpy(oname, sydSpec.name);
      if ((p = strrchr(oname,'/')) != NULL)
         *(p+1) =0;
      else if ((p = strrchr(oname,'\\')) != NULL)
         *(p+1) =0;
      else
        oname[0] = 0;
      strcat(oname,iname);
      if ((cFile = fopen(oname,"rb")) == NULL)
      {
    		ErrorMessage("SuckFile problem: %s", fsSpec->name);
    		return NULL;
      }
	}
	GetEOF(cFile, &fileSize);

	p = (unsigned char *) MyNewPtrClear(fileSize+1);
	if (p != NULL) {
		bytesRead = fread(p,1,fileSize,cFile);
		if (bytesRead != fileSize) {
			ErrorMessage("SuckFile problem: only %d of %d bytes read from %s", bytesRead,fileSize,fsSpec->name);
			MyDisposePtr(p);
			p = NULL;
		}
	}
	return p;
}

thread_local int	gLogIndent;		// per thread: notes start and end on several

void AddLogIndent(int i)
{
	gLogIndent += i;
}

void LogMessage(char *str,...)
{
	char	tbuf[512];
  extern int gVerbose;

	va_list	args;
  
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

  if (gVerbose) {
    if (gLogIndent) {
      char	sbuf[128];
      memset(sbuf,' ',gLogIndent*2);
      sbuf[gLogIndent*2] = 0;
      fputs(sbuf,stdout);
    }
    fputs(tbuf,stdout);
    fflush(stdout);
  }
}

#define	R_A	16807L
#define	R_M	2147483647L
#define R_Q	127773L
#define R_R	2836L

int32 	gSeed = 1;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int32 GetSRand(void)
{
	return gSeed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MySRand(int32 s)
{
	gSeed = s;
	if (gSeed == 0)
		gSeed = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Nonzero: the seed Randomize() uses instead of the clock (-r, or SYD_SEED),
// so patches that ask for unpredictable noise still render reproducibly.
int32 gFixedSeed = 0;

int32 RandomizeSeed(void)
{
	return gFixedSeed? gFixedSeed : (int32) time(NULL);
}

void Randomize(void)
{
	MySRand(RandomizeSeed());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A seed for the random number generator (1 to R_M-1) mixed from two numbers, for streams
// that must be the same however many threads render: see SSModule::Random.
int32 HashSeed(int32 a, int32 b)
{
	uint32 h = (uint32) a * 0x9E3779B1u ^ (uint32) b;
	h ^= h >> 16;	h *= 0x85EBCA6Bu;
	h ^= h >> 13;	h *= 0xC2B2AE35u;
	h ^= h >> 16;
	return (int32) (h % (uint32) (R_M - 1)) + 1;
}

// What MySRand(seed) then DoubleRandom() return, without touching the global seed, so
// threads can use it.
ssfloat DoubleRandomFrom(int32 seed)
{
	int32	hi,lo,test;

	if (seed == 0)
		seed = 1;
	hi   = seed / R_Q;
	lo   = seed % R_Q;
	test = R_A * lo - R_R * hi;
	if (test > 0)
		seed = test;
	else
		seed = test + R_M;
	return seed / (ssfloat) R_M;
}

int32 LongRandom(void)
{
	int32	hi,lo,test;

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
