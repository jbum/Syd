#ifndef _H_SydLocal
#define _H_SydLocal	1

// #include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>

#define kMaxFilenameLength  512
#define kMaxLineLength      512

typedef long			      int32;
typedef unsigned long	  uint32;
typedef short			      int16;
typedef unsigned short	uint16;
typedef bool            Boolean;

// typedef double			ssfloat;
// float is slightly faster on OSX, but may have issues....
typedef double			ssfloat;

#define WINFLOAT		1

typedef	struct {
	unsigned char e[2];
	unsigned char f[8];
} SSExtended80;

typedef struct {
  char name[kMaxFilenameLength];
} SydFileSpec;

void dtox80(ssfloat *s, SSExtended80 *f);
ssfloat x80tod(SSExtended80 *f);

typedef int		OSErr;
typedef char*	Ptr;

typedef struct {
  unsigned int  red,green,blue;
} RGBColor;

typedef struct {
  int h,v;
} Point;

typedef struct {
  int left,top,right,bottom;
} Rect;

typedef struct {
		uint32			  ckID;
		uint32			  ckSize;
		int16			    numChannels;
		uint32			  numSampleFrames;
		int16			    sampleSize;
		SSExtended80	sampleRate;
} AIFFHeader;

typedef struct {
		int16	wFormatTag;
		int16	wChannels;
		int32	dwSamplesPerSec;
		int32	dwAvgBytesPerSec;
		int16	wBlockAlign;
		int16	wSampleSize;
} WaveHeader;

// File Stuff - needs to be implemented
void GetEOF(FILE* fRef, long *size);
Boolean FSRead(FILE* fRef, long *size, Ptr buffer);
Boolean FSWrite(FILE* fRef, long *size, Ptr buffer);
// void ReportFileException(char *where, CFileException *fe);

enum {fsFromStart, fsFromMark, fsFromEnd};
int  SetFPos(FILE* fRef,int mode, long offset);
void GetFPos(FILE* fRef, long *offset);
//BOOL UseLocalPath(FSSpec *fsSpec);
//CString TruncatePathIfLocal(CString filename);
//CString TranslateFilename(CString filename, char *oldSuffix, char *newSuffix);

#define TickCount()		clock()
#define TICKS_PER_SEC	CLOCKS_PER_SEC

Ptr   NewPtrClear(size_t size);
void  DisposePtr(Ptr ptr);

/***
// Memory Stuff
#define BlockMove(src,dst,len)	memcpy(dst,src,len)
***/

#define noErr				0

// Math
#define sssin	sin
#define sspow	pow
#define pi					3.1415926535897932


#endif
