// SSStoreFile.h
#ifndef _H_SSStoreFile
#define _H_SSStoreFile	1

#include "ssstorage.h"
// #include "decksounds.h"

#define OutputBufferSize	32000L
#define InCacheSize			16000L

/* Audio Types */
#define ST_UNDEFINED	0
#define ST_SIGNED		2			/* Set for Signed Audio (0=0) */
#define ST_UNSIGNED		0			/* Don't Set for unsigned Audio (128=0) */
#define ST_16BIT		4			/* Set for 16 bit Audio */
#define ST_8BIT			0			/* Don't Set for 8 bit Audio */
#define ST_STEREO		8			/* Set for Stereo */
#define ST_MONO			0			/* Don't set for Mono */
#define ST_TOWNS		16			/* Set for FM Towns */
#define ST_AIFF			32			/* Set for AIFF */
#define ST_WAVE			64			/* Set for WAVE */

#define CKID_FORM 0x464f524d
#define CKID_AIFF 0x41494646
#define CKID_COMM 0x434f4d4d
#define CKID_SSND 0x53534e44
#define CKID_RIFF 0x52494646
#define CKID_WAVE 0x57415645
#define CKID_fmt_ 0x666d7420
#define CKID_data 0x64617461

enum {FT_AIFF, FT_WAVE};

// Abstract class
class SSStoreFile : public SSStorage {
	short	*inCache;
	long	inCachePos;

	short	*outbuf;		// used for memory-buffered samples
	long	memPos;			// memory position
	FILE*	fRef;       // was CFile
	SydFileSpec	fsSpec;
	long	fileDataStart,fileDataLength;
	AIFFHeader	aHdr;
	WaveHeader	wHdr;
	int		inType,outType;
	Boolean	writing;

public:
	int		fileType;

	SSStoreFile();
	~SSStoreFile();

	OSErr FlushOutputBuffer();
	OSErr OpenFile();
	OSErr CloseFile();
	OSErr ReadHeader();
	OSErr WriteHeader();
	Boolean GetFileSpec();
	Boolean	FindIFFChunk(long formID, long chunkID, long *dataPos, long *dataSize);
	OSErr SetReadFileSpec(SydFileSpec *fsSpec);
  OSErr SetWriteFileSpec(int fileType, int isStereo, SydFileSpec *fsSpec);

	// Overrides
	OSErr GetSamples(long start, long nbrSamples, short *buffer);
	short GetSample(long i);
	OSErr StoreSample(short sample);	// converts to internal format
	OSErr StoreStereoSample(short sampleL, short sampleR);	// converts to internal format
	OSErr StartStorage(long plannedSamples, ssfloat sampleRate, Boolean listenFlag, int nbrChannels);
	OSErr StopStorage();
};

#endif
