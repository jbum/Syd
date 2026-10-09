// sscscore.cp

#include "ss.h"
#include "sscscore.h"
#include "patchowner.h"
#include "ssoutput.h"

SSCScore::SSCScore(ModList * mList, short h, short v) : SSScore(MT_CScore, mList, h, v)
{
	scoreFileSpec.name[0] = 0;
	// scoreFileSpec.vRefNum = 0;
	// scoreFileSpec.parID = 0;
}

char *SSCScore::GrabParam(char *p, int pNbr, ssfloat *pStore, Boolean *gotIt)
{
	static	ssfloat lastP[16];

	while (*p && (*p == ' ' || *p == '\t'))
		++p;
	if (*p == '\r' || *p == '\n') {
		*gotIt = false;
		return p;
	}
	if (*p == '+' && pNbr == 2) {
		*pStore = lastP[2]+lastP[3];
		++p;
	}
	else if (*p == '.' && !isdigit(p[1])) {
		*pStore = lastP[pNbr];
		++p;
	}
	else {
		*pStore = atof((char *) p);
		while (*p && !isspace(*p))
			++p;
	}
	lastP[pNbr] = *pStore;
	*gotIt = true;
	return p;
}

void SSCScore::GenerateScore(SSModule *callingMod)
{
	unsigned char *buffer;
	ScoreSectionPtr	sp;
	NoteEventPtr	np;
	char		*p;

	// Open the file
	buffer = SuckFilefromSpec(&scoreFileSpec);	// Locally implemented
	if (buffer == NULL) {
		ErrorMessage("Can't open score file");
		parList->itsOwner->AbortSynthesis();
		return;
	}
	
	p = (char *) buffer;
	// Reset vars
	nbrSections = 0;
	sections = NULL;
	AddSection();

	sp = sections;
	sp->sectionStart = 0.0;
	sp->tempoScale = 1.0;
	sp->tempoStart = 0.0;
	sp->timeStart = 0.0;

	while (*p && parList->itsOwner->windowState == WS_Synthesize) {
		if (isspace(*p))
			++p;
		else if (*p == ';' || *p == '#')
		{
			while (*p && *p != '\r' && *p != '\n')
				++p;
		}
		else if (*p == 't')	// tempo
		{
			ssfloat	p1,p2;
			Boolean	gotIt;
			++p;
			p = GrabParam(p,1,&p1,&gotIt);
			p = GrabParam(p,2,&p2,&gotIt);

			sp->timeStart = (p1 - sp->tempoStart) * sp->tempoScale + sp->timeStart;
			sp->tempoStart = p1;
			sp->tempoScale = 60 / p2;
			// np = AddNoteEvent(sp,NEF_Tempo,((int) p1)-1,p2,0,0);
			while (*p && *p != '\r' && *p != '\n')
				++p;
		}
		else if (*p == 'f')	// table  (f0 is used to create silence)
		{
			// !!! Add wave table event NEF_Function
			while (*p && *p != '\r' && *p != '\n')
				++p;
		}
		else if (*p == 'i')	// note event
		{
	//  If 'i', parse instrument  '+' in p2 = p2+p3   '.' = previous field  
									// '<' is for ramping (mark 'begin ramp')
									//  '>' treat as for ramping as well
			ssfloat	p1,p2,p3;
			ssfloat optP[16];
			int	nbrOpts = 0;
			int n,i;
			Boolean	gotIt;
			++p;
			p = GrabParam(p,1,&p1,&gotIt);
			p = GrabParam(p,2,&p2,&gotIt);
			p = GrabParam(p,3,&p3,&gotIt);
			n = 4;
			do {
				p = GrabParam(p,n,&optP[nbrOpts],&gotIt);
				++n;
				if (gotIt)
					++nbrOpts;
			} while (gotIt);
			np = AddNoteEvent(sp,0,(int) p1,p2,p3,nbrOpts);
			for (i = 0; i < nbrOpts; ++i)
				np->op[i] = optP[i];
		}
		else if (*p == 's')	// section end
		{
			ScoreSectionPtr	lastS = sp;
			AddSection();
			sp = sections + (nbrSections-1);
			sp->sectionStart = lastS->sectionStart+lastS->sectionLength;
			sp->tempoScale = 1.0;
			sp->tempoStart = 0.0;
			sp->timeStart = 0.0;
			while (*p && *p != '\r' && *p != '\n')
				++p;
		}
		else if (*p == 'e')	// end of score
		{			
			break;
		}
		else {
			ErrorMessage("Invalid line in score: %.20s",p);
			parList->itsOwner->AbortSynthesis();
		}
	}
  if (gDurationOverride) {
    ssfloat scoreDuration = (sp->sectionStart + sp->sectionLength)*sp->tempoScale;
    SSOutput *outMod = (SSOutput *) (parList->itsOwner->outMod);
    if (outMod != NULL && scoreDuration > outMod->sampleDuration)
    {
      LogMessage("Overriding duration %g -> %g\n", outMod->sampleDuration, scoreDuration);
      outMod->sampleDuration = scoreDuration;
      parList->itsOwner->mainInst->sampleDuration = scoreDuration;
    }
  }
	MyDisposePtr(buffer);
	buffer = NULL;
}



/*
void SSCScore::Save(FILE* ar)
{

	SSScore::Save(ar);

#if macintosh
	char	tempS[64];

	BlockMove(scoreFileSpec.name,tempS,scoreFileSpec.name[0]+1);
	PtoCstr((StringPtr) tempS);
	WriteFileLine(ar,"CSCO %s %d %ld\r",
					tempS,
					(int) scoreFileSpec.vRefNum, 
					scoreFileSpec.parID);
#else
	WriteFileLine(ar,"CSCO %s 0 0\r",scoreFileSpec.name);
#endif
}
*/

void SSCScore::Load(FILE* ar) 
{
	char				*p,tbuf[kMaxLineLength];
	// int					vRefNum;
	// long				parID;
	char				tempS[kMaxFilenameLength];

	p = parList->GetNextInputLine(ar,"CSCO",tbuf);
  // sscanf(p, "CSCO %s %d %ld",tempS,&vRefNum,&parID);
  sscanf(p, "CSCO %s",tempS);
	// scoreFileSpec.vRefNum = vRefNum;
	// scoreFileSpec.parID = parID;
	strcpy(scoreFileSpec.name,tempS);
}

/*
char* SSCScore::GetLabel()
{
	return scoreFileSpec.name;
}
*/

void SSCScore::Copy(SSModule *ss)
{
	SSCScore *sc = (SSCScore *) ss;
	SSScore::Copy(ss);
	this->scoreFileSpec = sc->scoreFileSpec;
}
