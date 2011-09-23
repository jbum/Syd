// SSStoreFile.cp
//
// Storage module for buffered file-based storage.
//
// For fix for genenal purpose AIFF input (e.g. 8 bit)
#include "ss.h"
#include "ssstorefile.h"
#include "ssoutput.h"
#include "../sydlocal.h"
#include <math.h>

void SwapShort(short *ss)
{
	uint16 *n = (uint16 *) ss;
	*n = (uint16)( ((*n >> 8) & 0x00FFL) |	((*n << 8)  & 0xFF00L));
}

void SwapLong(long *sl)
{
	uint32 *n = (uint32 *) sl;
	*n =  ((*n >> 24) & 0x000000FFL) |
			((*n >> 8)  & 0x0000FF00L) |
			((*n << 8)  & 0x00FF0000L) |
			((*n << 24) & 0xFF000000L);
}

void SwapDouble(double *d1)
{
	char	temp[10],*p;
	p = (char *) d1;
	temp[0] = p[9];
	temp[1] = p[8];
	temp[2] = p[7];
	temp[3] = p[6];
	temp[4] = p[5];
	temp[5] = p[4];
	temp[6] = p[3];
	temp[7] = p[2];
	temp[8] = p[1];
	temp[9] = p[0];
	*d1 = *((double *) &temp[0]);
}

#define ShortToLSB(x)	SwapShort(x)
#define LongToLSB(x)	SwapLong(x)
#define ShortToMSB(x)	SwapShort(x)
#define LongToMSB(x)	SwapLong(x)

#if IS_BIGENDIAN
#define LocalToLSBShort(x)	SwapShort(x)
#define LocalToLSBLong(x)	SwapLong(x)
#define LSBShortToLocal(x)	SwapShort(x)
#define LSBLongToLocal(x)	SwapLong(x)
#define LocalToMSBShort(x)	(x)
#define LocalToMSBLong(x)	(x)
#define LocalToMSBDouble(x)	(x)
#define MSBShortToLocal(x)	(x)
#define MSBLongToLocal(x)	(x)
#define MSBDoubleToLocal(x)	(x)
#else
#define LocalToLSBShort(x)	(x)
#define LocalToLSBLong(x)	(x)
#define LSBShortToLocal(x)	(x)
#define LSBLongToLocal(x)	(x)
#define LocalToMSBShort(x)	SwapShort(x)
#define LocalToMSBLong(x)	SwapLong(x)
#define LocalToMSBDouble(x)	(x)
#define MSBShortToLocal(x)	SwapShort(x)
#define MSBLongToLocal(x)	SwapLong(x)
#define MSBDoubleToLocal(x)	(x)
#endif

SSStoreFile::SSStoreFile()
{
	memPos = 0L;
	outbuf = NULL;
	fRef = NULL;
	fsSpec.name[0] = 0;

	fileType = FT_WAVE;

  inType = ST_SIGNED | ST_16BIT | ST_MONO | ST_WAVE;
  outType = ST_SIGNED | ST_16BIT | ST_MONO | ST_WAVE;

	inCache = NULL;
	inCachePos = -1;
	writing = false;
}

OSErr SSStoreFile::SetReadFileSpec(SydFileSpec *fileSpec)
{
	OSErr	oe;

	CloseFile();		// 
	
	fsSpec = *fileSpec;

	if ((oe= OpenFile()) != noErr)
		return oe;
	if ((oe = ReadHeader()) != noErr)
		return oe;
	return noErr;
}

OSErr SSStoreFile::SetWriteFileSpec(int inFileType, int isStereo, SydFileSpec *fileSpec)
{
  this->nbrChannels = isStereo? 2 : 1;

	fsSpec = *fileSpec;
	if (inFileType == OM_AIFF)
		fileType = FT_AIFF;
	else if (inFileType == OM_WAVE)
		fileType = FT_WAVE;
	if (fileType == FT_AIFF) {
		inType = ST_SIGNED | ST_16BIT | ST_MONO | ST_AIFF;
		outType = ST_SIGNED | ST_16BIT | ST_MONO | ST_AIFF;
	}
	else {
		inType = ST_SIGNED | ST_16BIT | ST_MONO | ST_WAVE;
		outType = ST_SIGNED | ST_16BIT | ST_MONO | ST_WAVE;
	}
  if (isStereo != 0) {
    inType &= ~(ST_MONO);
    inType |= ST_STEREO;
    outType &= ~(ST_MONO);
    outType |= ST_STEREO;
  }
	return noErr;
}

SSStoreFile::~SSStoreFile()
{
	CloseFile();
	if (outbuf != NULL) {
		MyDisposePtr((Ptr) outbuf);
		outbuf = NULL;
	}
	if (inCache != NULL) {
		MyDisposePtr((Ptr) inCache);
		inCache = NULL;
	}
}

OSErr SSStoreFile::OpenFile()	// open file for input
{
	// LogMessage("Opening File for Read...\n");

  fRef = fopen(fsSpec.name,"rb");
  if (fRef == NULL) {
    ErrorMessage("Can't open file for input (%s)",fsSpec.name);
    return -1;
  }
	return noErr;
}

OSErr SSStoreFile::CloseFile()
{
	OSErr	oe = noErr;

	if (fRef) {
    fclose(fRef);
    fRef = NULL;
	}
	return oe;
}

OSErr	SSStoreFile::ReadHeader()	// Set FileDataStart
{
	long	fileSize,dataPos,dataSize,count,fileSig;
	char	inbuf[40];

	GetEOF(fRef, &fileSize);

	count = 4;
	// FSRead(fRef, &count, (char *) &fileSig);
  fread((char *) &fileSig,4,1,fRef);
	// SetFPos(fRef, fsFromStart, 0L);
  fseek(fRef, 0L, SEEK_SET);
  
	MSBLongToLocal(&fileSig);
  if (fileSig == CKID_FORM) //  'FORM'
		fileType = FT_AIFF;
	else
		fileType = FT_WAVE;


	if (fileType == FT_AIFF) {
		inType = ST_SIGNED | ST_8BIT | ST_MONO | ST_AIFF;
    // AIFF, COMM
    FindIFFChunk(CKID_AIFF, CKID_COMM, &dataPos,&dataSize);
		if (dataSize > 26 - 8)
			dataSize = 26 - 8;

		// FSRead(fRef,&dataSize,(Ptr) &inbuf[0]);
    fread((Ptr) &inbuf[0],1,dataSize,fRef);
		aHdr.numChannels = *((int16 *) &inbuf[0]);
		aHdr.numSampleFrames = *((uint32 *) &inbuf[2]);
		aHdr.sampleSize = *((int16 *) &inbuf[6]);
		memcpy(&aHdr.sampleRate, &inbuf[8], 10);
//		aHdr.sampleRate = *((double *) &inbuf[8]);

		MSBShortToLocal(&aHdr.numChannels);
		MSBShortToLocal(&aHdr.sampleSize);
		MSBLongToLocal((long *) &aHdr.ckID);
		MSBLongToLocal((long *) &aHdr.ckSize);
		MSBLongToLocal((long *) &aHdr.numSampleFrames);
		MSBDoubleToLocal(&aHdr.sampleRate);

// #if !macintosh
		// !!!!!!!
//		aHdr.sampleRate = 22050.0;
// #endif

#if GENERATINGPOWERPC || WINFLOAT
		{
			ssfloat	sr;
			sr = x80tod(&aHdr.sampleRate);
			sampleRate = sr;
		}
#else
#if GENERATING68881
		{
			SSextended96	sr;
			d80tod96(&aHdr.sampleRate,&sr);
			sampleRate = sr;
		}
#else
		sampleRate = aHdr.sampleRate;
#endif
#endif
		nbrFrames = aHdr.numSampleFrames;
		if (aHdr.sampleSize == 16)
			inType |= ST_16BIT;
		if (aHdr.numChannels == 2)
			inType |= ST_STEREO;
		// SetFPos(fRef,fsFromStart,0L);
    fseek(fRef, 0L, SEEK_SET);
    FindIFFChunk(CKID_AIFF, CKID_SSND,&dataPos,&dataSize);
		fileDataStart = dataPos + sizeof(long)*2;
		fileDataLength = dataSize - sizeof(long)*2;
	}
	else {
		long	chunkHdr[2];
		//  Wave Header
		// SetFPos(fRef, fsFromStart, 20);
    fseek(fRef, 20L, SEEK_SET);
		dataSize = sizeof(WaveHeader);
		// FSRead(fRef,&dataSize,(Ptr) &wHdr);
    fread((Ptr) &wHdr, 1, dataSize, fRef);

		LSBShortToLocal(&wHdr.wFormatTag);
		LSBShortToLocal(&wHdr.wChannels);
		LSBLongToLocal(&wHdr.dwSamplesPerSec);
		LSBLongToLocal(&wHdr.dwAvgBytesPerSec);
		LSBShortToLocal(&wHdr.wBlockAlign);
		LSBShortToLocal(&wHdr.wSampleSize);

		dataSize = 8;
		// FSRead(fRef,&dataSize,(Ptr) &chunkHdr[0]);
    fread((Ptr) &chunkHdr[0],1,dataSize, fRef);
		LSBLongToLocal(&chunkHdr[0]);
		LSBLongToLocal(&chunkHdr[1]);

		inType = ST_UNSIGNED | ST_8BIT | ST_MONO | ST_WAVE;

		if (wHdr.wChannels == 2)
			inType |= ST_STEREO;
		if (wHdr.wSampleSize == 16)
			inType |= ST_16BIT | ST_SIGNED;

		sampleRate = wHdr.dwSamplesPerSec;
		fileDataLength = nbrFrames = chunkHdr[1];
		if (inType & ST_16BIT)
			nbrFrames /= 2;
		fileDataStart = 20 + sizeof(WaveHeader) + 8;
		fileDataLength = dataSize;
	}
	return noErr;
}

OSErr SSStoreFile::WriteHeader()
{
	char	outbuf[40];

	// SetFPos(fRef,fsFromStart,0L);
  fseek(fRef, 0L, SEEK_SET);

	if (fileType == FT_AIFF) {
		long 			ckID,ckSize,byteSize,count,outputByteLength;
		ssfloat			dSampleRate;

		outputByteLength = nbrFrames;
		if (outType & ST_16BIT)
			outputByteLength <<= 1;
		if (outType & ST_STEREO)
			outputByteLength <<= 1;

		byteSize = outputByteLength;

		memset(&aHdr,0,sizeof(AIFFHeader));
		aHdr.ckID = CKID_COMM;
		aHdr.ckSize = 18;
		aHdr.numChannels = 1;
		aHdr.numSampleFrames = nbrFrames;
		// 7/1/93 JAB
		aHdr.sampleSize = 16;

		// Convert to X80 format
		dSampleRate = sampleRate;
#if GENERATINGPOWERPC || WINFLOAT
		dtox80(&dSampleRate, &aHdr.sampleRate);
#else
#if GENERATING68881
		{
			SSextended96	sr = dSampleRate;
			d96tod80(&sr, &aHdr.sampleRate);
		}
#else
		aHdr.sampleRate = dSampleRate;
#endif
#endif
		LocalToMSBShort(&aHdr.numChannels);
		LocalToMSBShort(&aHdr.sampleSize);
		LocalToMSBLong((long *) &aHdr.ckID);
		LocalToMSBLong((long *) &aHdr.ckSize);
		LocalToMSBLong((long *) &aHdr.numSampleFrames);
		LocalToMSBDouble(&aHdr.sampleRate);

		// memcpy(outbuf, &aHdr, 26);

		*((uint32 *) &outbuf[0]) = aHdr.ckID;
		*((uint32 *) &outbuf[4]) = aHdr.ckSize;
		*((int16 *) &outbuf[8]) = aHdr.numChannels;
		*((uint32 *) &outbuf[10]) = aHdr.numSampleFrames;
		*((int16 *) &outbuf[14]) = aHdr.sampleSize;
		// *((double *) &outbuf[16]) = aHdr.sampleRate;
		memcpy(&outbuf[16],&aHdr.sampleRate,10);

		ckID = CKID_FORM;
		ckSize = 4 + 8 + 18 + 8 + (8 + byteSize);	// 2/25/91
		LocalToMSBLong(&ckID);
		LocalToMSBLong(&ckSize);
		count = sizeof(long);	fwrite((Ptr) &ckID,1,count,fRef);
    count = sizeof(long); fwrite((Ptr) &ckSize,1,count,fRef);
		ckID = CKID_AIFF;
		LocalToMSBLong(&ckID);
    count = sizeof(long); fwrite((Ptr) &ckID,1,count,fRef);
		
    count = 26;       fwrite((Ptr) &outbuf[0],1,count,fRef);

		ckID = CKID_SSND;
		ckSize = byteSize+8;
		LocalToMSBLong(&ckID);
		LocalToMSBLong(&ckSize);
    count = sizeof(long); fwrite((Ptr) &ckID,1,count,fRef);
    count = sizeof(long); fwrite((Ptr) &ckSize,1,count,fRef);
		ckID = 0;		/* Write offset,blocksize */
		ckSize = 0;
		LocalToMSBLong(&ckID);
		LocalToMSBLong(&ckSize);
    count = sizeof(long); fwrite((Ptr) &ckID,1,count,fRef);
    count = sizeof(long); fwrite((Ptr) &ckSize,1,count,fRef);
#if !macintosh
    fflush(fRef);
#endif
		fileDataStart = ftell(fRef);
	}
	else if (fileType == FT_WAVE) {
		long	ckID,ckSize,byteSize,count,outputByteLength;

		outputByteLength = nbrFrames;
		if (outType & ST_16BIT)
			outputByteLength <<= 1;
		if (outType & ST_STEREO)
			outputByteLength <<= 1;

		memset(&wHdr,0,sizeof(WaveHeader));
		byteSize = outputByteLength;

		wHdr.wFormatTag = 1;	LocalToLSBShort(&wHdr.wFormatTag);

		wHdr.wChannels = this->nbrChannels;
		LocalToLSBShort(&wHdr.wChannels);

		wHdr.wBlockAlign = 2;
		LocalToLSBShort(&wHdr.wBlockAlign);

		// 7/1/93 JAB
		wHdr.wSampleSize = 16;
		LocalToLSBShort(&wHdr.wSampleSize);

		wHdr.dwSamplesPerSec = (long) sampleRate;
		LocalToLSBLong(&wHdr.dwSamplesPerSec);

		wHdr.dwAvgBytesPerSec = (long) (sampleRate*2*this->nbrChannels);
		LocalToLSBLong(&wHdr.dwAvgBytesPerSec);
		
		ckID = CKID_RIFF;
		ckSize = byteSize + 8 + sizeof(WaveHeader) + 8 + 4;
		LocalToMSBLong(&ckID);
		LocalToLSBLong(&ckSize);

    count = sizeof(long); fwrite((Ptr) &ckID,1,count,fRef);
    count = sizeof(long); fwrite((Ptr) &ckSize,1,count,fRef);
    ckID = CKID_WAVE;
		LocalToMSBLong(&ckID);
    count = sizeof(long); fwrite((Ptr) &ckID,1,count,fRef);
    ckID = CKID_fmt_;
		ckSize = sizeof(WaveHeader);
		LocalToMSBLong(&ckID);
		LocalToLSBLong(&ckSize);
    count = sizeof(long); fwrite((Ptr) &ckID,1,count,fRef);
    count = sizeof(long); fwrite((Ptr) &ckSize,1,count,fRef);
    count = sizeof(WaveHeader); fwrite((Ptr) &wHdr,1,count,fRef);
		ckID = CKID_data;
		ckSize = byteSize;
		LocalToMSBLong(&ckID);
		LocalToLSBLong(&ckSize);
    count = sizeof(long); fwrite((Ptr) &ckID,1,count,fRef);
    count = sizeof(long); fwrite((Ptr) &ckSize,1,count,fRef);
#if !macintosh
		fflush(fRef);
#endif
    fileDataStart = ftell(fRef);
	}
	return noErr;
}

short SSStoreFile::GetSample(long i)
{
	static int lastmode;

	// Invalid input cache if we are writing...
	if (writing && memPos < nbrFrames*nbrChannels) {
		FlushOutputBuffer();
		inCachePos = -1;
	}
	
	// Force reget if we've been writing
	if (lastmode != writing)
	{
		lastmode = writing;
		inCachePos = -1;
	}

	// Allocate Cache if Necessary
	if (inCache == NULL) {
		inCache = (short *) MyNewPtrClear(sizeof(short *) * InCacheSize);
		if (inCache == NULL)
			return 0;
		inCachePos = -1;	// Force reload of cache
	}

	// Load Cache if Necessary
	if (inCachePos == -1 || i < inCachePos || i >= inCachePos+InCacheSize)
	{
		inCachePos = i;
		GetSamples(inCachePos,InCacheSize,inCache);
	}
	return inCache[i-inCachePos];
}

// Get Samples as 16-bit signed values
OSErr SSStoreFile::GetSamples(long start, long nbrSamples, short *outBuffer)
{
	OSErr	oe=0;
	long	count;
	long	startOffset = 0;	// position in buffer to start writing

	// If writing, flush the output buffer, so we can read all samples from the file
	if (writing && memPos < nbrFrames*nbrChannels) {
		FlushOutputBuffer();
	}

	if (fRef == 0) {
		if ((oe= OpenFile()) != noErr)
			return oe;
		if ((oe = ReadHeader()) != noErr)
			return oe;
	}

	if (this->nbrFrames == 0)
		return -1;
		
	if (nbrSamples == 0)
		return noErr;

	// Clear output buffer
	memset(outBuffer,0,sizeof(short)*nbrSamples);

	if (start >= this->nbrFrames*this->nbrChannels)
		return noErr;

	if (start < 0) {
		startOffset = -start;
		start = 0;
		nbrSamples -= startOffset;		
	}	
	if (startOffset >= nbrSamples)
		return noErr;

	if (start+nbrSamples > this->nbrFrames*this->nbrChannels)
		nbrSamples = this->nbrFrames*this->nbrChannels - start;

	if (inType & ST_16BIT) {
		// SetFPos(fRef, fsFromStart, fileDataStart+start*sizeof(short));
    fseek(fRef, fileDataStart+start*sizeof(short), SEEK_SET);
		count= nbrSamples*sizeof(short);
		// oe = FSRead(fRef,&count,(Ptr) (outBuffer+startOffset));
    if (fread((Ptr) (outBuffer+startOffset),1,count,fRef) != count)
      oe = -1;

		if (fileType == FT_WAVE) {
			long	i;
			short	*op = outBuffer+startOffset;
			for (i = 0; i < nbrSamples; ++i,++op) {
				// Convert to big-endian
				LSBShortToLocal(op);
			}
		}
		else if (fileType == FT_AIFF) {
			long	i;
			short	*op = outBuffer+startOffset;
			for (i = 0; i < nbrSamples; ++i,++op) {
				// Convert to big-endian
				MSBShortToLocal(op);
			}
		}
	}
	else 
	{
		// SetFPos(fRef, fsFromStart, fileDataStart+start*sizeof(char));
    fseek(fRef, fileDataStart+start*sizeof(char), SEEK_SET);
		count= nbrSamples*sizeof(char);
		// oe = FSRead(fRef,&count,(Ptr) (outBuffer+startOffset));
    if (fread((Ptr) (outBuffer+startOffset),1,count,fRef) != count)
      oe = -1;
		// Convert 8-bit to 16 bit
		{
			unsigned char	*sp = (unsigned char *) (outBuffer+startOffset);
			short			v,signBit;
			short			*dp = (short *) (outBuffer+startOffset);
			long			i;
			sp += nbrSamples-1;
			dp += nbrSamples-1;
			if (inType & ST_SIGNED)
				signBit = 0x80;
			else
				signBit = 0x00;
			for (i = 0; i < nbrSamples; ++i) 
			{
				v = *sp ^ signBit;				// convert to 8-bit unsigned
				*dp = (v | (v << 8)) ^ 0x8000;	// convert to 16-bit signed
				--sp;
				--dp;
			}
		}
	}
	return oe;
}

OSErr SSStoreFile::FlushOutputBuffer()
{
	long	bCount,start,sCount,i;
	OSErr	oe = noErr;

	if (fRef == 0) {
		ErrorExit("File is closed");
		return -1;		// File is closed
	}
	start = memPos;
	sCount = nbrFrames*nbrChannels - start;
	if (sCount) {
		bCount = sCount * sizeof(short);
		if (fileType == FT_WAVE) {
			for (i = 0; i < sCount; ++i)
				LocalToLSBShort(&outbuf[i]);
		}
		else if (fileType == FT_AIFF) {
			for (i = 0; i < sCount; ++i)
				LocalToMSBShort(&outbuf[i]);
		}
		// SetFPos(fRef, fsFromStart, (long) (fileDataStart+start*sizeof(short)));
    fseek(fRef, (long) (fileDataStart+start*sizeof(short)), SEEK_SET);
    // oe = FSWrite(fRef, &bCount, (Ptr) outbuf);
    if (fwrite((Ptr) outbuf, 1, bCount, fRef) != bCount)
      oe = -1;
		fflush(fRef);
	}
	memPos = nbrFrames*nbrChannels;
	return oe;
}

/*
Boolean SSStoreFile::GetFileSpec()
{
#if macintosh
	StringPtr			defName = "\pUntitled.aiff";
	StandardFileReply 	reply;

//	fsSpec.vRefNum = 0;
//	fsSpec.parID = 0;
	StandardPutFile("\pExport To:",defName,&reply);
	if (!reply.sfGood)
		return false;
	fsSpec = reply.sfFile;
#endif
	return true;
}
*/

OSErr SSStoreFile::StartStorage(long plannedSamples,ssfloat sampleRate, Boolean listenFlag, int nbrChannels)	// Start Buffered Storage
{
	OSErr	oe = noErr;

	SSStorage::StartStorage(plannedSamples,sampleRate,listenFlag, nbrChannels);

	SetSampleRate(sampleRate);

	// Was using "planned samples here"

	CloseFile();

	// If No File Descriptor, Get One from User
	if (fsSpec.name[0] == 0) {
		// if (!GetFileSpec())
			return -1;
	}
	// Set memory buffer to correct size
	if (outbuf == NULL)
	{
		outbuf = (short *) MyNewPtrClear(sizeof(short*) * OutputBufferSize);
		if (outbuf == NULL) {
			ErrorMessage("Out of Memory");
			return -1;
		}
	}
	memPos = 0;
	nbrFrames = 0;

	// Open the File for Output
  fRef = fopen(fsSpec.name,"w+");
  if (fRef == NULL)
  {
		ErrorMessage("Can't open file for output");
		return true;
	}

	oe = WriteHeader();
	writing = true;
	return oe;
}

OSErr SSStoreFile::StoreSample(short sample)
{
	if (nbrFrames+1 > memPos+OutputBufferSize)
		FlushOutputBuffer();
	outbuf[(nbrFrames-memPos)] = sample;
	++nbrFrames;
	return noErr;
}

OSErr SSStoreFile::StoreStereoSample(short sampleL, short sampleR)
{
	if (nbrFrames*nbrChannels+2 > memPos+OutputBufferSize)
		FlushOutputBuffer();
	outbuf[(nbrFrames*nbrChannels-memPos)] = sampleL;
	outbuf[(nbrFrames*nbrChannels+1-memPos)] = sampleR;
	++nbrFrames;
	return noErr;
}

OSErr SSStoreFile::StopStorage()	// Stop Buffered Storage (flush buffer)
{
	OSErr	oe;
	if ((oe = FlushOutputBuffer()) != noErr)
		return oe;
	if ((oe = WriteHeader()) != noErr)
		return oe;
	if ((oe = CloseFile()) != noErr)
		return oe;
	writing = false;
	SSStorage::StopStorage();
	return noErr;
}

Boolean	SSStoreFile::FindIFFChunk(long formID, long chunkID, long *dataPos, long *dataSize)
{
	long	iffHdr[2];
	long	count;
	long	fPos;
	fPos = 0;
	while (1) {
		count = 8;
		fPos += 8;
    // if (FSRead(fRef,&count,(Ptr) iffHdr) != noErr)
    if (fread((Ptr) iffHdr,1,count,fRef) != count)
			return false;
	
		MSBLongToLocal(&iffHdr[0]);
		MSBLongToLocal(&iffHdr[1]);

		if (iffHdr[0] == chunkID) {
			*dataPos = fPos;
			*dataSize = iffHdr[1];
			return true;
		}
		else {
			switch (iffHdr[0]) {
			case CKID_FORM:
				count = 4;
				fPos += 4;
        // if (FSRead(fRef,&count,(Ptr) iffHdr) != noErr)
        if (fread((Ptr) iffHdr,1,count,fRef) != count)
					return false;
				MSBLongToLocal(&iffHdr[0]);
				MSBLongToLocal(&iffHdr[1]);
				if (iffHdr[0] != formID) {
					count = iffHdr[1] - 4;
					if (count & 1)
						++count;
					fPos += count;
          // if (SetFPos(fRef,fsFromMark,count) != noErr)
          if (fseek(fRef,count, SEEK_CUR) != 0)
						return false;
				}
				break;
			default:
				count = iffHdr[1];
				if (count & 1)
					++count;
				fPos += count;
				// if (SetFPos(fRef,fsFromMark,count) != noErr)
        if (fseek(fRef,count, SEEK_CUR) != 0)
					return false;
			}
		}
	}
}

#if WINFLOAT

// Converts local floating point representation
// to IEEE 10 byte extended precision double, as
// used in AIFF format
// See Spreadsheet IEEE for an explanation

void dtox80(ssfloat *s, SSExtended80 *ext)
{
	int			p2,e,i,mask,offset;
	ssfloat	v = *s, targetv, f;

	// figure out nearest power of 2
	p2 = int(log(pow(v,(1/log(2)))));
	// compute e
	e = 16383+p2;
	
	*((short *) &ext->e[0]) = e;
	LocalToMSBShort((short *) &ext->e[0]);

	f = 0;
	targetv = pow(2,p2)*(1+f);

	mask = 0x80;
	offset = 0;

	ext->f[offset] |= mask;	// Set i bit
	mask >>= 1;

	for (i = 1; i <= 63 && targetv < v; ++i)
	{
		if (pow(2,p2)*(1+f+pow(2,-i)) <= v)
		{
			f += pow(2,-i);
			targetv = pow(2,p2)*(1+f);
			ext->f[offset] |= mask;
		}
		mask >>= 1;
		if (mask == 0) {
			mask = 0x80;
			offset++;
		}
	}
}

ssfloat x80tod(SSExtended80 *ext)
{
	short	e;
	int		sign,p2,i,mask,offset;
	ssfloat	f,v;

	e = *((short *) &ext->e[0]);
	MSBShortToLocal(&e);
	sign = ((e & 0x8000) > 0)? -1 : 1;
	e &= 0x7FFF;	// Clear sign bit

	p2 = e - 16383;
	f = 0;

	mask = 0x40;
	offset = 0;

	for (i = 1; i <= 63; ++i) {
		if ((ext->f[offset] & mask) > 0)
			f += pow(2,-i);

		mask >>= 1;
		if (mask == 0) {
			mask = 0x80;
			offset++;
		}
	}
	
	v = sign*pow(2,p2)*(1 + f);
	return v;
}

#endif
