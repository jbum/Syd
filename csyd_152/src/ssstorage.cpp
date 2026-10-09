// ssstorage.c

#include "ss.h"
#include "ssstorage.h"

SSStorage::SSStorage()
{
	nbrFrames = 0;
	sampleRate = 22050;
	nbrChannels = 1;
}

void SSStorage::SetSampleRate(ssfloat sr)
{
	sampleRate = sr;
}
#ifdef UI_FEATURES
OSErr SSStorage::GetSamplesF(long start, long nbrSamples, ssfloat *buf)
{
	short	*sBuffer,*sp;
	ssfloat	*dp;
	OSErr	oe;
	long	i;
	sBuffer = ((short *) (buf+nbrSamples)) - nbrSamples;
	oe = GetSamples(start,nbrSamples,sBuffer);
	if (oe != noErr)
		return oe;
	sp = sBuffer;
	dp= buf;
	for (i = 0; i < nbrSamples; ++i) {
		*(dp++) = *(sp++) / (double) 0x7FFF;
	}
	return noErr;
}
#endif

OSErr SSStorage::StoreSampleF(ssfloat v)
{
	OSErr	oe;
	if (v < -1.0)	// Perform clipping
		v = -1.0;
	else if (v > 1.0)
		v = 1.0;
	oe = StoreSample((short) (v * 0x0007FFF));
	return oe;
}

OSErr SSStorage::StoreSampleFS(ssfloat vL, ssfloat vR)
{
	OSErr	oe;
	if (vL < -1.0)	// Perform clipping
		vL = -1.0;
	else if (vL > 1.0)
		vL = 1.0;
	if (vR < -1.0)	// Perform clipping
		vR = -1.0;
	else if (vR > 1.0)
		vR = 1.0;
	oe = StoreStereoSample((short) (vL * 0x0007FFF),(short) (vR * 0x0007FFF));
	return oe;
}


ssfloat SSStorage::GetSampleF(long n)
{
	short	iv;
	iv = GetSample(n);
	return iv/(double) 0x7FFF;
}

void SSStorage::Reset()
{
	nbrFrames = 0;
}

OSErr SSStorage::StartStorage(long plannedSamples, ssfloat sampleRate, Boolean listenFlag, int nbrChannels)
{
	SetSampleRate(sampleRate);
	nbrFrames = 0;
  this->nbrChannels = nbrChannels;
	return noErr;
}

OSErr SSStorage::StopStorage()
{
	return noErr;
}

OSErr SSStorage::Import(SSStorage *src)
{
	short	*inBuf;
	long	memPos,sampleCount,count,i;
	inBuf = (short *) MyNewPtrClear(sizeof(short *) * 32000L);
	if (inBuf == NULL) {
		ErrorMessage("Can't allocate buffer\n");
		return -1;
	}
	memPos = 0;
	sampleCount = src->nbrFrames;

	StartStorage(src->nbrFrames,src->sampleRate,false, src->nbrChannels);
	while (sampleCount) {
		if (sampleCount >= 32000L)
			count = 32000L;
		else
			count = sampleCount;
		src->GetSamples(memPos,count,inBuf);
		for (i= 0; i < count; ++i)
			StoreSample(inBuf[i]);
		sampleCount -= count;
	}
	MyDisposePtr((Ptr) inBuf);
	StopStorage();
	return noErr;
}
