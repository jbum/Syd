// ssscore.cp

#include "ss.h"
#include "ssscore.h"
// #include "mainwin.h"
#include "patchowner.h"
#include "ssoutput.h"
#include <math.h>

/* Todo:
	Deal properly with sections with no notes
 */

SSScore::SSScore(short modType, ModList * mList, short h, short v) : SSModule(modType, mList, h, v)
{
	
	DescribeLink(0, "Instrument 1", "i1",0xFFFF,0x0000,0xFFFF);
	DescribeLink(1, "Instrument 2", "i2",0xDDDD,0x0000,0xDDDD);
	DescribeLink(2, "Instrument 3", "i3",0xBBBB,0x0000,0xBBBB);
	DescribeLink(3, "Instrument 4", "i4",0x9999,0x0000,0x9999);
	DescribeLink(4, "Instrument 5", "i5",0x7777,0x0000,0x7777);
	DescribeLink(5, "Instrument 6", "i6",0x5555,0x0000,0x5555);
	DescribeLink(6, "Instrument 7", "i7",0x3333,0x0000,0x3333);
	DescribeLink(7, "Instrument 8", "i8",0x1111,0x0000,0x1111);

	// local vars
	nbrSections = 0;
	sections = NULL;
	curSection = NULL;
	sectionIndex = 0;
//	nextNote = NULL;
	nbrActiveNotes= 0;
	allocActiveNotes = 0;
	activeNotes = NULL;
	curTempo = 60.0;
	scoreDone = true;
	noteTime = 0.0;
}

void SSScore::CleanUp()
{
	ScoreSectionPtr	sp;
	int			i,n;

	TerminateActiveNotes();

	sp = sections;
	for (i =0 ; i < nbrSections; ++i,++sp) 
	{
		for (n = 0; n < sp->nbrNotes; ++n) {
			MyDisposePtr((Ptr) sp->notes[n]);
			sp->notes[n] = NULL;
		}
		if (sp->notes) {
			MyDisposePtr((Ptr) sp->notes);
			sp->notes = NULL;
		}
	}
	if (sections) {
		MyDisposePtr((Ptr) sections);
		sections = NULL;
	}
	nbrSections = 0;
	if (activeNotes) {
		MyDisposePtr((Ptr) activeNotes);
		activeNotes = NULL;
	}
	nbrActiveNotes = 0;
	allocActiveNotes = 0;

	SSModule::CleanUp();
}

// Might be useful to display first, and give opp. to change it
ssfloat	SSScore::GenerateOutput(SSModule *callingMod)
{
	register ActiveNotePtr	ap;
	register ssfloat			v, vR;
	register int				activeNoteNbr;
	register ssfloat			pTime = parList->pTime;

	this->callingMod = callingMod;
	if (scoreDone)
		return 0.0;
	if (pTime >= curSection->sectionStart+curSection->sectionLength) {
    // only happens if multiple sections...
    // LogMessage("Activating next section at %.2f\n", pTime);
		NextSection(pTime);
		if (scoreDone)
			return 0.0;
	}
	// given the time, determine which instruments are sounding
	while (curSection->noteNbr < curSection->nbrNotes && 
			curSection->notes[curSection->noteNbr]->p2 + curSection->sectionStart <= pTime &&
			parList->itsOwner->windowState == WS_Synthesize)
	{
	  // if (parList->itsOwner->gTime >= 9 && parList->itsOwner->gTime <= 12)
	  //   LogMessage("Activating next note at pt=%.2f, gt=%.3f\n", pTime, parList->itsOwner->gTime);
		ActivateNextNote(pTime);
	}

	if (parList->itsOwner->windowState != WS_Synthesize)
		return 0.0;

	// call each instrument's Generate Output with the note time
	// add the results
	v = 0.0;
	vR = 0.0;
	activeNoteNbr = 0;
	ap = activeNotes;
	while (activeNoteNbr < nbrActiveNotes && parList->itsOwner->windowState == WS_Synthesize)
	{
		// Has Note Elapsed?  If so, kill it
		if (curSection->sectionStart + ap->noteInfo->p2 + ap->noteInfo->p3 <= pTime)
		{
			KillNote(activeNoteNbr);
		}
		else { // otherwise, play it, if the instrument has modules
			if (ap->instr != NULL && ap->instr->nbrModules >= 1) {
				noteTime = pTime - (curSection->sectionStart+ap->noteInfo->p2);
				this->instParams = ap->noteInfo;
				v += ((SSModule *) ap->instr->mods[0])->GenerateOutputTime(this,noteTime);
				vR += ((SSModule *) ap->instr->mods[0])->getRightSample();
			}
			++activeNoteNbr;
			++ap;
		}
	}
	// if (v == 0 && parList->itsOwner->gTime >= 9 && parList->itsOwner->gTime <= 10)
	//  printf("+");
  if (parList->itsOwner->gTime >= 9 && parList->itsOwner->gTime <= 12 && activeNoteNbr == 0)
    printf("+");
  lastRightSample = vR;
	return v;
}

ssfloat SSScore::GetInstParameter(int n)
{
	if (instParams == NULL)
		return 0.0;

	switch (n) {
	case 0:	 return noteTime;
	case 1:	 return	instParams->p1;
	case 2:	 return	instParams->p2;
	case 3:	 return	instParams->p3;
	default: 
		if (n - 4 < instParams->nbrOptParams)
			return instParams->op[n-4];
		else
			return 0.0;
	}	
}

Boolean SSScore::IsInScore()
{
	return true;
}

// Does double duty: when generating notes, it's the time of each note.
// When generating score, it's the ratio of note/maxnotes
//
ssfloat SSScore::GetNoteTime()
{
	return noteTime;
}

void SSScore::ActivateNextNote(ssfloat pTime)
{
	if (nbrActiveNotes+1 > allocActiveNotes) {
		// Allocate more notes if needed
		allocActiveNotes += 8;
		ActiveNotePtr	ap = (ActiveNotePtr) MyNewPtrClear(sizeof(ActiveNote)*allocActiveNotes);
    if (ap == NULL)
		  ErrorMessage("Failed to activate note");
		if (nbrActiveNotes) {
      memcpy(ap, activeNotes, sizeof(ActiveNote)*nbrActiveNotes);
			MyDisposePtr((Ptr) activeNotes);
			activeNotes = NULL;
		}
		activeNotes = ap;			
	}
	// Add note to active notes, and initialize it
	InitializeActiveNote(nbrActiveNotes,curSection->notes[curSection->noteNbr]);
	++nbrActiveNotes;

	// increment note ptr or set to null
	++curSection->noteNbr;
//	if (curSection->noteNbr >= curSection->nbrNotes)
//		nextNote = NULL;
//	else
//		++nextNote;
}

void SSScore::InitializeActiveNote(int i, NoteEventPtr	np)
{
	// LogMessage("Initializing Note\r");
	int	instNbr;

	if (np->flags & NEF_Tempo) {
		curTempo = np->p2;
	}
	else {
		ActiveNotePtr	ap = &activeNotes[i];

		ap->noteInfo = np;
		ap->instr = NULL;

		instNbr = np->p1 -1;
		if (CountInputs(instNbr) == 0)
			LogMessage("A note refers to a non-existent instrument\r");
		else {
			for (i = 0; i < nbrInputs; ++i) {
				if (instNbr == -1 || inputs[i].inputType == instNbr) {
					// ??? We *could* support multiple instruments per note...
					// ??? But we're not...
					ap->instr = parList->CloneInstrument(inputs[i].link);
					break;
				}
			}
			// Reset the Instrument
			if (ap->instr != NULL) {
				noteTime = 0;
				this->instParams = ap->noteInfo;
				ap->instr->ResetInstruments(this);
			}
			else {
				LogMessage("A note refers to a non-existent instrument\n");
			}
		}
	}
}

void SSScore::KillNote(int i)
{
	if (i < 0 || i >= nbrActiveNotes)
		return;

	// LogMessage("Killing Note\r");
	ActiveNotePtr ap = &activeNotes[i];

	// Dispose of the instrument instance
	if (ap->instr) {
		AddLogIndent(2);
		delete ap->instr;
		AddLogIndent(-2);
		ap->instr = NULL;
	}

	// Swap last note in
	activeNotes[i] = activeNotes[nbrActiveNotes-1];
	--nbrActiveNotes;
}

void SSScore::TerminateActiveNotes()
{
	AddLogIndent(2);
	while (nbrActiveNotes)
		KillNote(0);
	AddLogIndent(-2);
}

void SSScore::EndScore()
{
	scoreDone = true;
	// release memory
	CleanUp();
	curSection = NULL;
//	nextNote = NULL;
	sectionIndex = 0;
}

void SSScore::NextSection(ssfloat pTime)
{
	TerminateActiveNotes();
	++sectionIndex;
	if (sectionIndex >= nbrSections) {
		EndScore();
		return;
	}
	++curSection;
	curSection->sectionStart = pTime;
	curSection->noteNbr = 0;
//	nextNote = curSection->notes;
}

void SSScore::AddSection()
{
	ScoreSectionPtr	sp2;
	sp2 = (ScoreSectionPtr) MyNewPtrClear(sizeof(ScoreSection)*(nbrSections+1));
  if (sp2 == NULL)
		  ErrorMessage("Failed to allocated section");
	if (nbrSections) {
    memcpy(sp2,sections,sizeof(ScoreSection)*(nbrSections));
		MyDisposePtr((Ptr) sections);
		sections = NULL;
	}
	sections = sp2;
	sections[nbrSections].tempoScale = 1.0;
	sections[nbrSections].tempoStart = 0.0;
	sections[nbrSections].timeStart = 0.0;
	nbrSections++;
}

NoteEventPtr SSScore::AddNoteEvent(ScoreSectionPtr sp, int flags, int inst, 
							ssfloat start, ssfloat dur, int nbrOptParams)
{
	int	nbrExtraParams = 0;
	NoteEventPtr	np;
	ssfloat	endNoteTime;
	if (sp->nbrNotes+1 > sp->allocNotes) {
		NoteEventPtr	*nnp;
		sp->allocNotes = sp->nbrNotes + 16;
		nnp = (NoteEventPtr *) MyNewPtrClear(sizeof(NoteEventPtr *) * sp->allocNotes);
		if (nnp == NULL)
		  ErrorMessage("Failed to allocated note buffer");
		if (sp->nbrNotes) {
      memcpy(nnp, sp->notes, sizeof(NoteEventPtr *) * sp->nbrNotes);
			MyDisposePtr((Ptr) sp->notes);
			sp->notes = NULL;
		}
		sp->notes = nnp;
	}
	if (nbrOptParams > 1)
		nbrExtraParams = nbrOptParams-1;
	endNoteTime= start+dur;
	if (endNoteTime > sp->sectionLength)
		sp->sectionLength = endNoteTime;
	sp->notes[sp->nbrNotes] = (NoteEventPtr) MyNewPtrClear(sizeof(NoteEvent)+sizeof(ssfloat)*nbrExtraParams);
  if (sp->notes[sp->nbrNotes] == NULL)
		  ErrorMessage("Failed to allocated extra parms");
	np = sp->notes[sp->nbrNotes];
	np->nbrOptParams = nbrOptParams;
	np->flags = flags;
	np->p1 = inst;
	np->p2 = (start-sp->tempoStart)*sp->tempoScale + sp->timeStart;
	np->p3 = dur*sp->tempoScale;
	++sp->nbrNotes;
	return np;
}


// Load in score, sort it
// !! Note this is the only part that is file dependent
//   - We should also make some other score modules which
//   - Overwrite this part....

void SSScore::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	nbrSections = 0;
	sections = NULL;
	curTempo = 60.0;
	noteTime = 0.0;
}

void SSScore::InitScore(SSModule *callingMod)
{
  if (gVerbose > 1)
    printf("Init score: %s\n", label);
	GenerateScore(callingMod);

	//
	// Sort Pass
	//
	SortScore();
	
	//
	// Resolve Ramps Pass
	//
	PostScoreProcessing();
	
	// Initialize for sound generation
	//	
	nbrActiveNotes = 0;
	allocActiveNotes = 0;
	activeNotes = NULL;
	sectionIndex = 0;
	if (nbrSections > 0) {
		curSection = sections;
		curSection->sectionStart = 0.0;
		curSection->noteNbr = 0;
//		nextNote = curSection->notes;
		scoreDone = false;
	}
	else {
		EndScore();
	}
}

int CompareScoreEvents(const void *e1, const void *e2);

int CompareScoreEvents(const void *e1, const void *e2)
{
	double	val;
	NoteEventPtr	n1 = *((NoteEventPtr *) e1);
	NoteEventPtr	n2 = *((NoteEventPtr *) e2);
	val = n1->p2 - n2->p2;
	if (val < 0)
		return -1;
	else if (val > 0)
		return 1;
	else
		return 0;
}

void SSScore::SortScore()
{
	int	i;
	ScoreSectionPtr	sp;

	for (i =0,sp=sections ; i < nbrSections; ++i,++sp) 
	{
		qsort(sp->notes, sp->nbrNotes, sizeof(NoteEventPtr *), CompareScoreEvents);
	}
}

// To be overridden
void SSScore::GenerateScore(SSModule *callingMod)
{
}

void SSScore::PostScoreProcessing()
{
}
