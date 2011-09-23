// ssoutput.cp

#include "ss.h"
#include "ssoutput.h"
#include "patchowner.h"
#include <string.h>

SSOutput::SSOutput(ModList * mList, short h, short v) : SSModule(MT_Output, mList, h, v)
{
	sampleRate = 22050;
	sampleDuration = 2.0;
	strcpy(outFileSpec.name, "untitled.wav");
	outputType = OM_WAVE;
	isStereo = 0;
//	outFileSpec.vRefNum = 0;
//	outFileSpec.parID = 0;
	DescribeLink(0, "Output Signal", "sig",0,0,0xFFFF);
}

// ssfloat	SSOutput::GenerateOutput(SSModule *callingMod)
// {
// 	return MixInputs(-1, callingMod);
// }

static	int	recurseCheck;

ssfloat SSOutput::GetInstParameter(int n)
{
  extern ssfloat gInstrumentParam[];
  
	if (callingMod != NULL && callingMod != this) {
		ssfloat	v;
		++recurseCheck;
		if (recurseCheck > 32)
			ErrorExit("Recursion Problem");
		v = callingMod->GetInstParameter(n);
		--recurseCheck;
		return v;
	}
	else {
	  	switch (n) {
	  	case 0:		return parList->itsOwner->gTime;			// Note time
	  	case 1:		return 1.0;					// Instrument #
	  	case 2:		return 0.0;					// Start Time
	  	case 3:		return parList->itsOwner->mainInst->sampleDuration;	// Duration
      default:  
        return gInstrumentParam[n];
		}
	}
}

#ifdef UI_FEATURES

void SSOutput::Save(FILE* ar)
{
	char	tempS[64];
	SSModule::Save(ar);
	strcpy(tempS, outFileSpec.name);
/*	WriteFileLine(ar,"OUTPUT %lg %lg %d %d %ld %s\r",
					sampleDuration,
					sampleRate,
					outputType,
					(int) outFileSpec.vRefNum,
					outFileSpec.parID,
					tempS); */
  if (isStereo == 0) { // keep things backward compatible, if possible...
    WriteFileLine(ar,"OUTPUT %lg %lg %d 0 0 %s\r",
          sampleDuration,
          sampleRate,
          outputType,
          tempS);
  }
  else {
    WriteFileLine(ar,"OUTS %d %lg %lg %d 0 0 %s\r",
          isStereo,
          sampleDuration,
          sampleRate,
          outputType,
          tempS);
  }
}

char* SSOutput::GetLabel()
{
  char  *fName;
  if (outputType != OM_MEMORY)
    fName = outFileSpec.name;
  else
    fName = "(memory)";
  return fName;
}

#endif

void SSOutput::Load(FILE* ar)
{
	char				*p,tbuf[512];
	double				sd=0,sr=0;
  extern ssfloat  gDuration;
  extern int      gSampleRate;
  extern char     gOutputFilename[];
  
  
	p = parList->GetNextInputLine(ar,"OUT",tbuf);

	if (p[3] == ' ') {
		sscanf(p, "OUT %lf %lf",&sd,&sr);
		sampleDuration = sd;
		sampleRate = sr;
		strcpy(outFileSpec.name, "untitled.wav");
		outputType = OM_WAVE;
		isStereo = 0;
		// outFileSpec.vRefNum = 0;
		// outFileSpec.parID = 0;
	}
	else if (p[3] == 'S') { // OUTS
		char tempS[64];
		int  is;
		sscanf(p, "OUTS %d %lf %lf %d %s",
			&is,&sd,&sr,&outputType,tempS);
		isStereo = is;
		sampleDuration = sd;
		sampleRate = sr;
		strcpy(outFileSpec.name,tempS);

    // CSYD 5-29-06
    // FORCE WAVE OUTPUT IF MEMORY TYPE IS USED
    if (outputType == OM_MEMORY) {
      LogMessage("Redirecting patch output from memory to file\n");
      outputType = OM_WAVE;
      strcpy(outFileSpec.name, "untitled.wav");
    }
	}
	else {
		int	vRefNum;
		long parID;
		char				tempS[64];
		sscanf(p, "OUTPUT %lf %lf %d %d %ld %s",
			&sd,&sr,&outputType,&vRefNum,&parID,tempS);
		sampleDuration = sd;
		sampleRate = sr;
		isStereo = 0;
		// outFileSpec.vRefNum = vRefNum;
		// outFileSpec.parID = parID;
		strcpy(outFileSpec.name,tempS);

    // CSYD 5-29-06
    // FORCE WAVE OUTPUT IF MEMORY TYPE IS USED
    if (outputType == OM_MEMORY) {
      LogMessage("Redirecting patch output from memory to file\n");
      outputType = OM_WAVE;
      strcpy(outFileSpec.name, "untitled.wav");
    }
	}
  
  // CSYD 5-29-06 - override output options if specified on command line
  if (gDuration > 0 && gDuration != sampleDuration)
  {
      LogMessage("Overriding sample duration %g -> %g seconds\n", sampleDuration, gDuration);
      sampleDuration = gDuration;
  }
  if (gSampleRate > 0 && gSampleRate != sampleRate)
  {
      LogMessage("Overriding sample rate %g -> %d\n", sampleRate, gSampleRate);
      sampleRate = gSampleRate;
  }
  if (gOutputFilename[0]) 
  {
      strcpy(outFileSpec.name, gOutputFilename);
      char *p;
      p = strrchr(outFileSpec.name,'.');
      if (p != NULL) {
        if (strcmp(p+1,"aif") == 0 || strcmp(p+1,"aiff") == 0)
        {
          LogMessage("Overriding output file to AIFF\n");
          outputType = OM_AIFF;
        }
        else
        {
          LogMessage("Overriding output file to WAV\n");
          outputType = OM_WAVE;
        }
      }
    
  }
  LogMessage("Outputting to %s\n", outFileSpec.name);
}

