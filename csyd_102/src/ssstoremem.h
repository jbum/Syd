// SSStoreMem.h
#ifndef _H_SSStoreMem
#define _H_SSStoreMem	1

#include "ssstorage.h"

// Size of sliding window, should be long enough 
// to allow playback to catch up.
#define MaxAllocSamples		(44100*10)

// Abstract class
class SSStoreMem : public SSStorage {
	long	memStartWindow;
	short	*buffer;
public:
	SSStoreMem();
	~SSStoreMem();

	// Overrides
	OSErr GetSamples(long start, long nbrSamples, short *buffer);
	OSErr StoreSample(short sample);	// converts to internal format
	OSErr StoreStereoSample(short sampleL, short sampleR);	// converts to internal format
	short GetSample(long i);
	OSErr StartStorage(long plannedSamples,ssfloat sampleRate, Boolean listenFlag, int nbrChannels);	// Start Buffered Storage
};

#endif
