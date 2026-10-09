// ssrandscore.cpp

#include "ss.h"
#include "ssrandscore.h"
#include <math.h>

SSRandScore::SSRandScore(ModList * mList, short h, short v) : SSScore(MT_RandScore, mList, h, v)
{
	int	i;
	InitExp(&nbrNotesExp, "p3 + 1");
	InitExp(&pExp[0],"1");						// instrument
	InitExp(&pExp[1],"?*(p3-1)");				// start time
	InitExp(&pExp[2],"0.5+?*0.5");				// duration
	InitExp(&pExp[3],"0.25 + ?*0.25");			// amplitude (suggestion)
	InitExp(&pExp[4],"55*2**(?*4.0)");			// frequency (suggestion)
	for (i = 5; i < MaxParams; ++i)
		InitExp(&pExp[i],"0");					// extra parameters		
}

void SSRandScore::AddRandomNoteToScore(ScoreSectionPtr sp, int inst, 
							ssfloat start, ssfloat dur, 
							ssfloat *p)
{
	NoteEventPtr	np;
	int	nbrExtras,i;

	nbrExtras = CountActiveExpressions() - 3;

	np = AddNoteEvent(sp, 0, inst, start, dur, nbrExtras);
	for (i = 0; i < nbrExtras; ++i)
		np->op[i] = p[i];
}

// Called from Reset
void SSRandScore::GenerateScore(SSModule *callingMod)
{
	// ssfloat	t;
	int		n,maxNotes,i;
	int		nbrInstruments = CountInputTypes();
	int		inst;
	ssfloat	dur,start,p[MaxParams-4],totalDur;
	ScoreSectionPtr	sp;

	// Initialize Expressions
	ResetExp(&nbrNotesExp, callingMod);
	for (i = 0; i < MaxParams; ++i)
		ResetExp(&pExp[i], callingMod);

	nbrSections = 0;
	sections = NULL;
	AddSection();

	sp = sections;
	sp->sectionStart = 0.0;
	sp->tempoScale = 1.0;
	sp->tempoStart = 0.0;
	sp->timeStart = 0.0;

	// Possible variables:
	// 		Event Density (average events per second)
	//			min dur, max dur
	//			min freq, max freq
	//									!!! may need to pass time here...
	maxNotes = (int) SolveExp(&nbrNotesExp, callingMod);
	totalDur = callingMod->GetInstParameter(3);

	for (n = 0; n < maxNotes; ++n) {

		noteTime = n;	// Allows retrieval via "N"
		noteIndex = n;	// Allows retrieval via "I"

		inst = (int) SolveExp(&pExp[0], callingMod);
		start = SolveExp(&pExp[1], callingMod);
		dur = SolveExp(&pExp[2], callingMod);

		if (start+dur > totalDur)
			dur = totalDur - start;

		for (i = 4; i < MaxParams; ++i)
			p[i-4] = SolveExp(&pExp[i-1], callingMod);

		AddRandomNoteToScore(sp, inst,start,dur,p);
	}
}

ssfloat SSRandScore::GetIValue()
{
	return noteIndex;
}


int SSRandScore::CountActiveExpressions()
{
	int	i = 0;
	for (i = MaxParams-1; i >= 0; --i) {
		if (strcmp((char *) pExp[i].exp,"0") != 0)
			return i+1;
	}
	return 0;
}

/*
void SSRandScore::Save(FILE* ar)
{
	int	i,nbrActiveExpressions;
	SSScore::Save(ar);
	nbrActiveExpressions = CountActiveExpressions();
	WriteFileLine(ar,"RSCO %d\r",nbrActiveExpressions);
	WriteFileLine(ar,"RSCON %s\r",nbrNotesExp.exp);
	for (i = 0; i < nbrActiveExpressions; ++i)
		WriteFileLine(ar,"RSCO%d %s\r",i+1,pExp[i].exp);
}
*/

void SSRandScore::Load(FILE* ar) 
{
	char	*p,tbuf[512];
	int		i,nbrActiveExpressions;

	p = parList->GetNextInputLine(ar,"RSCO",tbuf);
	if (p[4] == '#') {	// Old Style
		InitExp(&nbrNotesExp, p+6);
		nbrActiveExpressions = 8;
	}
	else {
		sscanf(p, "RSCO %d",&nbrActiveExpressions);
		LoadExp(ar,"RSCON",&nbrNotesExp);
	}
	for (i = 0; i < nbrActiveExpressions; ++i) {
		char	pStr[8];
		sprintf(pStr,"RSCO%d",i+1);
		LoadExp(ar,pStr,&pExp[i]);
	}
	for (i = nbrActiveExpressions; i < MaxParams; ++i) {
		InitExp(&pExp[i],"0");
	}
}



void SSRandScore::Copy(SSModule *ss)
{
	int	i;
	SSRandScore *sr = (SSRandScore *) ss;
	SSScore::Copy(ss);
	CopyExp(&sr->nbrNotesExp, &nbrNotesExp);
	for (i = 0; i < MaxParams; ++i)
		CopyExp(&sr->pExp[i], &pExp[i]);
}
