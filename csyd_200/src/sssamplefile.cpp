// SSSampleFile.cp

#include "ss.h"
#include "sssamplefile.h"
// #include "mainwin.h"
#include "ssoutput.h"

SSSampleFile::SSSampleFile(ModList * mList, short h, short v) : SSModule(MT_SampleFile, mList, h, v)
{
	sampleFileSpec.name[0] = 0;
	// sampleFileSpec.vRefNum = 0;
	// sampleFileSpec.parID = 0;
	InitExp(&timeScaleExp, "1.0");
	flags = 0;
	fileStore = NULL;
	DescribeLink(0, "Invalid Input", "???",	0xffff,0xffff,0xffff);
}

void SSSampleFile::Reset(SSModule *callingMod)
{
	OSErr	oe;

	SSModule::Reset(callingMod);

	AddLogIndent(2);
	fileStore = new SSStoreFile();
	if ((oe = fileStore->SetReadFileSpec(&sampleFileSpec)) != noErr)
		ErrorMessage("Error Opening Sample File");
	AddLogIndent(-2);
	ResetExp(&timeScaleExp, callingMod);
}

ssfloat SSSampleFile::GenerateOutput(SSModule *callingMod)
{
	long	n;
	ssfloat	v,timeScale;
	this->callingMod = callingMod;
	timeScale = SolveExp(&timeScaleExp, callingMod);
	if (fileStore != NULL) {
		n = (long) (parList->pTime * fileStore->sampleRate * timeScale);
    v = fileStore->GetSampleF(n*fileStore->nbrChannels);
		if (fileStore->nbrChannels == 2) {
		  lastRightSample = fileStore->GetSampleF(n*fileStore->nbrChannels+1);
		}
	}
	else
		v = lastRightSample = 0.0;
	return v;
}

void SSSampleFile::CleanUp()
{
	if (fileStore) {
		delete fileStore;
		fileStore = NULL;
	}
	SSModule::CleanUp();
}


/*
void SSSampleFile::Save(FILE* ar)
{

	SSModule::Save(ar);

	WriteFileLine(ar,"FSAMP %s %d %ld %d\r",
					sampleFileSpec.name,
//          (int) sampleFileSpec.vRefNum, 
//          sampleFileSpec.parID,
          0,0,
					flags);
	WriteFileLine(ar,"FSAMPT %s\r",timeScaleExp.exp);
}
*/
void SSSampleFile::Load(FILE* ar) 
{
	char				*p,tbuf[512];
	int					vRefNum;
	long				parID;
	char				tempS[64];

	p = parList->GetNextInputLine(ar,"FSAMP",tbuf);
	sscanf(p, "FSAMP %s %d %ld %d",tempS,&vRefNum,&parID,&flags);
//	sampleFileSpec.vRefNum = vRefNum;
//	sampleFileSpec.parID = parID;
	strcpy(sampleFileSpec.name,tempS);
	LoadExp(ar,"FSAMPT",&timeScaleExp);
}

/*
char*	SSSampleFile::GetLabel()
{
	return sampleFileSpec.name;
}
*/

void SSSampleFile::Copy(SSModule *ss)
{
	SSSampleFile *sf = (SSSampleFile *) ss;
	SSModule::Copy(ss);
	sampleFileSpec = sf->sampleFileSpec;
	flags = sf->flags;
	CopyExp(&sf->timeScaleExp, &timeScaleExp);
}
