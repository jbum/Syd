// SSSound.h

// This is mainly to avoid some bugs with ExtSoundHeader as defined in
// sound.h, which may have the wrong settings for the AIFFSampleRate, depending
// on the precompiled header in use by Metrowerks.
#ifndef _H_SSSound
#define _H_SSSound	1

#if macintosh

#include <Sound.h>

#if GENERATING68K && GENERATING68881

struct _SSextended80 {
	short							exp;
	short							man[4];
};

// 68k FPU
typedef struct _SSextended80 SSextended80;
typedef long double SSextended96;

void d96tod80(SSextended96 *src, SSextended80 *dst);
void d80tod96(SSextended80 *src, SSextended96 *dst);

#else

// 68k No FPU or PowerPC
typedef extended80 SSextended80;
typedef extended96 SSextended96;

#endif

struct SSExtSoundHeader {
	Ptr								samplePtr;					/*if nil then samples are in sample area*/
	unsigned long					numChannels;				/*number of channels,  ie mono = 1*/
	UnsignedFixed					sampleRate;					/*sample rate in Apples Fixed point representation*/
	unsigned long					loopStart;					/*same meaning as regular SoundHeader*/
	unsigned long					loopEnd;					/*same meaning as regular SoundHeader*/
	unsigned char					encode;						/*data structure used , stdSH, extSH, or cmpSH*/
	unsigned char					baseFrequency;				/*same meaning as regular SoundHeader*/
	unsigned long					numFrames;					/*length in total number of frames*/
	SSextended80					AIFFSampleRate;				/*IEEE sample rate*/
	Ptr								markerChunk;				/*sync track*/
	Ptr								instrumentChunks;			/*AIFF instrument chunks*/
	Ptr								AESRecording;
	unsigned short					sampleSize;					/*number of bits in sample*/
	unsigned short					futureUse1;					/*reserved by Apple*/
	unsigned long					futureUse2;					/*reserved by Apple*/
	unsigned long					futureUse3;					/*reserved by Apple*/
	unsigned long					futureUse4;					/*reserved by Apple*/
	unsigned char					sampleArea[1];				/*space for when samples follow directly*/
};
typedef struct SSExtSoundHeader SSExtSoundHeader;

typedef SSExtSoundHeader *SSExtSoundHeaderPtr;

typedef struct {
		uint32	ckID;
		uint32	ckSize;
		int16 numChannels;
		uint32 numSampleFrames;
		int16 sampleSize;
		SSextended80 sampleRate;
} AIFFHeader;

typedef struct {
		int16	wFormatTag;
		int16	wChannels;
		int32	dwSamplesPerSec;
		int32	dwAvgBytesPerSec;
		int16	wBlockAlign;
		int16	wSampleSize;
} WaveHeader;

#endif

#endif