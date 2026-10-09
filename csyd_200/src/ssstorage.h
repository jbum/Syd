// SSStorage.h
#ifndef _H_SSStorage
#define _H_SSStorage	1


// Abstract class
class SSStorage {
	// Boolean	playbackQueueing,synthPlayback;
	// long	curPlayOffset;
	// Ptr		sndBuf1,sndBuf2;

public:
	long	nbrFrames;
	int   nbrChannels;
	ssfloat	sampleRate;

	SSStorage();
	// Abstracts
	virtual	OSErr GetSamples(long start, long nbrSamples, short *buffer) = 0;
	virtual OSErr StoreSample(short sample) = 0;	// converts to internal format
	virtual OSErr StoreStereoSample(short sampleL, short sampleR) = 0;	// converts to internal format
	virtual short GetSample(long i) = 0;

	// Virtuals
	virtual OSErr StartStorage(long plannedSamples, ssfloat sampleRate, Boolean listenFlag, int numChannels);
	virtual OSErr StopStorage();
	virtual void SetSampleRate(ssfloat sr);
	virtual void Reset();

	OSErr GetSamplesF(long start, long nbrSamples, ssfloat *buffer);
	OSErr StoreSampleF(ssfloat sample);
	OSErr StoreSampleFS(ssfloat sampleL, ssfloat sampleR);
	ssfloat GetSampleF(long i);
	OSErr Import(SSStorage *src);
};

#endif
