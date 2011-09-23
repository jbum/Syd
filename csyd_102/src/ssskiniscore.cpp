// sscscore.cp

#include "ss.h"
#include "ssskiniscore.h"
#include "patchowner.h"
#include "ssoutput.h"

SSSkiniScore::SSSkiniScore(ModList * mList, short h, short v) : SSScore(MT_SkiniScore, mList, h, v)
{
	scoreFileSpec.name[0] = 0;
}

char *SSSkiniScore::GrabParam(char *p, int pNbr, ssfloat *pStore, Boolean *gotIt)
{
	static	ssfloat lastP[16];

	while (*p && (*p == ' ' || *p == '\t'))
		++p;
	if (*p == '\r' || *p == '\n') {
		*gotIt = false;
		return p;
	}
  if (*p == 'm') 
  {
      *pStore = atoi(p+1);
      while (*p && !isspace(*p))
       ++p;
  }
  else if (tolower(*p) >= 'a' && tolower(*p) <= 'g')
  {
    static char nbase[] = {0,2,3,5,7,8,10};
    static int octave = 4;
    int nval = nbase[tolower(*p)-'a'];
    ++p;
    if (*p == '#')
    {
      ++nval;
      ++p;
    }
    else if (*p == 'b') {
      --nval;
      ++p;
    }
    if (isdigit(*p))
      octave = *p - '0';
    *pStore = 21 + nval + 12*octave;
  }
  else if (isdigit(*p) || (*p == '.' && isdigit(p[1])) )
  {
		*pStore = atof((char *) p);
		while (*p && !isspace(*p))
			++p;
	}
	lastP[pNbr] = *pStore;
	*gotIt = true;
	return p;
}

void SSSkiniScore::GenerateScore(SSModule *callingMod)
{
	unsigned char *buffer;
	ScoreSectionPtr	sp;
	NoteEventPtr	np;
	char		*p;
  ssfloat ampMult = 1;
  Boolean gotIt;

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
		else if (*p == ';' || *p == '#' || strncmp(p,"//", 2) == 0)
		{
			while (*p && *p != '\r' && *p != '\n')
				++p;
		}
    else if (strncmp(p,"AmpMult", 7) == 0)  // beats per minute
    {
      p += 7;
      p = GrabParam(p,1,&ampMult,&gotIt);
    }
		else if (strncmp(p,"BPM", 3) == 0)	// beats per minute
		{
			ssfloat	p1,p2;
			Boolean	gotIt;
			p += 3;
			p = GrabParam(p,1,&p1,&gotIt);

			sp->timeStart = 0;
			sp->tempoStart = 0;
			sp->tempoScale = 60 / p1;
			// np = AddNoteEvent(sp,NEF_Tempo,((int) p1)-1,p2,0,0);
			while (*p && *p != '\r' && *p != '\n')
				++p;
		}
    else if (strncmp(p,"Note", 4) == 0) // note event
		{
	//  If 'i', parse instrument  '+' in p2 = p2+p3   '.' = previous field  
									// '<' is for ramping (mark 'begin ramp')
									//  '>' treat as for ramping as well

      // LogMessage("Got Note\n");
			ssfloat	p1,p2,p3,p4;
			ssfloat optP[16];
			int	nbrOpts = 0;
			int n,i;

			p += 4;

			p = GrabParam(p,1,&p1,&gotIt); // start
			p = GrabParam(p,2,&p2,&gotIt); // duration
      p = GrabParam(p,3,&optP[0],&gotIt); // amp (300)
      p = GrabParam(p,4,&optP[1],&gotIt); // pitch (mN or F)
			n = 4;
      nbrOpts = 2;
			do {
				p = GrabParam(p,n,&optP[nbrOpts],&gotIt);
				++n;
				if (gotIt)
					++nbrOpts;
			} while (gotIt);
      optP[0] = optP[0]*128*ampMult/1000; // Repair Amplitude
			np = AddNoteEvent(sp,0,1,p1,p2,nbrOpts);
			for (i = 0; i < nbrOpts; ++i)
				np->op[i] = optP[i];
		}
    else if (strncmp(p,"Synth", 5) == 0 ||
             strncmp(p,"Inst", 4) == 0)
    {
      while (*p && *p != '\r' && *p != '\n')
        ++p;
    }
		else {
			ErrorMessage("Skini: Unrecognized line in score: %.20s...",p);
			// parList->itsOwner->AbortSynthesis();
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


void SSSkiniScore::Load(FILE* ar) 
{
	char				*p,tbuf[kMaxLineLength];
	// int					vRefNum;
	// long				parID;
	char				tempS[kMaxFilenameLength];

	p = parList->GetNextInputLine(ar,"SKINISCO",tbuf);
  // sscanf(p, "CSCO %s %d %ld",tempS,&vRefNum,&parID);
  sscanf(p, "SKINISCO %s",tempS);
	// scoreFileSpec.vRefNum = vRefNum;
	// scoreFileSpec.parID = parID;
	strcpy(scoreFileSpec.name,tempS);
}

void SSSkiniScore::Copy(SSModule *ss)
{
  SSSkiniScore *sc = (SSSkiniScore *) ss;
	SSScore::Copy(ss);
	this->scoreFileSpec = sc->scoreFileSpec;
}
