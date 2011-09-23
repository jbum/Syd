// ssstoremem.cp
//
// Simple storage module for memory-based storage.

#include "ss.h"
#include "ssstoremem.h"

SSStoreMem::SSStoreMem()
{
	buffer = (short *) MyNewPtrClear(sizeof(short ) * MaxAllocSamples);
	memStartWindow = 0;
}

SSStoreMem::~SSStoreMem()
{
	if (buffer != NULL) {
		MyDisposePtr((Ptr) buffer);
		nbrFrames = 0;
		buffer = NULL;
	}
}

OSErr SSStoreMem::GetSamples(long start, long nbrSamples, short *outBuffer)
{
	// 10/11 Switched to long for 68k compat
	long	i;

	for (i = 0; i < nbrSamples; ++i)
		outBuffer[i] = GetSample(start+i);

	return noErr;
}

short SSStoreMem::GetSample(long i)
{
	if (i < memStartWindow || i >= memStartWindow+MaxAllocSamples || i >= nbrFrames)
		return 0;
	else
		return buffer[i-memStartWindow];
}

OSErr SSStoreMem::StoreSample(short sample)
{
	long	n = nbrFrames-memStartWindow;
	if (n < 0 || n >= MaxAllocSamples)
		ErrorMessage("Oy!");
	else
		buffer[n] = sample;
	++nbrFrames;
	if (nbrFrames+1024 >= memStartWindow+MaxAllocSamples) {
    memcpy(&buffer[0],&buffer[1024],(MaxAllocSamples-1024)*sizeof(short));
		memStartWindow += 1024;
	}
	return noErr;
}

OSErr SSStoreMem::StoreStereoSample(short sampleL, short sampleR)
{
	long	n = nbrFrames-memStartWindow;
	if (n < 0 || n >= MaxAllocSamples)
		ErrorMessage("Oy!");
	else {
		buffer[n] = sampleL;
		buffer[n+1] = sampleR;
  }
	++nbrFrames;
	if (nbrFrames*2+1024 >= memStartWindow+MaxAllocSamples) {
    memcpy(&buffer[0],&buffer[1024],(MaxAllocSamples-1024)*sizeof(short));
		memStartWindow += 1024;
	}
	return noErr;
}


OSErr SSStoreMem::StartStorage(long plannedSamples,ssfloat sampleRate, Boolean listenFlag, int nbrChannels)	// Start Buffered Storage
{
	OSErr	oe = noErr;
	oe = SSStorage::StartStorage(plannedSamples,sampleRate,listenFlag, nbrChannels);
	memStartWindow = 0;
	nbrFrames = 0;
	return oe;
}
