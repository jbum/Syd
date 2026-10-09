// ssscore.cp

#include "ss.h"
#include "ssscore.h"
// #include "mainwin.h"
#include "patchowner.h"
#include "ssoutput.h"
#include <math.h>
#include "sspool.h"
#include "ssoscillator.h"
#include <algorithm>
#include <chrono>

// The note a thread is rendering for a score (RenderVoice). Its modules ask the score for the
// note's parameters and time, which sample by sample are in instParams and noteTime; rendering
// in blocks, several threads render notes of one score at once, so each keeps its own.
struct VoiceContext {
	SSScore			*score;
	NoteEventPtr	params;
	ssfloat			noteTime;
};
static thread_local VoiceContext *tVoice;

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
	if (blockMode && parList == parList->itsOwner->mainInst)
		return BlockOutput(pTime);
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
  lastRightSample = vR;
	return v;
}

ssfloat SSScore::GetInstParameter(int n)
{
	NoteEventPtr	instParams = this->instParams;
	ssfloat			noteTime = this->noteTime;

	if (tVoice && tVoice->score == this) {
		instParams = tVoice->params;
		noteTime = tVoice->noteTime;
	}
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
	if (tVoice && tVoice->score == this)
		return tVoice->noteTime;
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
				ap->instr->noteSeed = HashSeed(parList->noteSeed,
											   (sectionIndex << 20) + curSection->noteNbr);
				ap->instr->ownRandom = blockMode || parList->ownRandom;
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
	blockMode = gBlockSize > 0 && BlockSafe();
	blkTimes.clear();
	blkPos = 0;
	if (gVerbose > 1)
		LogMessage("Score %s: %s\n", label, blockMode? "block rendering" : "sample by sample");
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
		// Stable: notes starting together keep the order they were made in. qsort leaves their
		// order to the C library, which differs between platforms, and with it the order notes
		// are added in and the random numbers each gets (SSModule::Random).
		std::stable_sort(sp->notes, sp->notes + sp->nbrNotes,
						 [](NoteEventPtr a, NoteEventPtr b) { return a->p2 < b->p2; });
	}
}

// To be overridden
void SSScore::GenerateScore(SSModule *callingMod)
{
}

void SSScore::PostScoreProcessing()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Block rendering
//
// Sample by sample, a score renders every sounding note for one sample, then the next sample.
// With thousands of notes sounding, each note's modules fall out of the cache between samples.
// RenderBlock renders each note for a block of samples instead, then adds the notes together
// sample by sample in the order GenerateOutput would have, so the result is the same to the
// bit: each note computes the same samples in the same order as before (oscillator phase,
// envelopes, filter and delay state all carry from one block to the next), and the sums add
// the same numbers in the same order.
//
// The notes' samples are computed in a different order relative to each other, so this is only
// done when nothing they compute is shared between them (BlockSafe): no global variables or
// time, and no modules that share state (a note's random numbers are its own).

int gBlockSize = 256;

// Whether the modules feeding m only touch state of their own note. A score's own expressions
// are solved when its score is generated, which for a score directly in our instruments is when
// one of our notes starts (in time order, before a block renders), so those may use anything; a
// score further down starts its notes while ours render, so its expressions must pass too.
// (Random numbers pass: a note's modules have their own, SSModule::Random.)
static bool SubtreeBlockSafe(SSModule *m, bool inNestedScore, int depth)
{
	if (m == NULL || depth > 64) {
		if (gVerbose > 1)
			LogMessage("  a missing input, or instruments nested too deep\n");
		return false;
	}
	switch (m->moduleType) {
	case MT_RandScore:
	case MT_CScore:
		if (inNestedScore && m->ExpUnsafe()) {
			if (gVerbose > 1)
				LogMessage("  %s: a nested score's expression shares state between notes\n", m->label);
			return false;
		}
		for (int i = 0; i < m->nbrInputs; ++i)
			if (!SubtreeBlockSafe(m->inputs[i].link, true, depth + 1))
				return false;
		return true;
	case MT_Oscillator:
	case MT_Envelope:
	case MT_Mixer:
	case MT_Filter:
	case MT_Butter:
	case MT_Smooth:
	case MT_Noise:			// has its own random numbers
	case MT_Delay:
	case MT_Threshhold:
	case MT_SampleAndHold:
	case MT_Amplifier:
	case MT_Inverter:
	case MT_Expression:
	case MT_Pluck:			// random numbers of their own (SSModule::Random)
	case MT_Maraca:
		break;
	default:				// hammers share strings, the rest files, globals or folder signals
		if (gVerbose > 1)
			LogMessage("  %s: a module that shares state between notes\n", m->label);
		return false;
	}
	if (m->ExpUnsafe()) {
		if (gVerbose > 1)
			LogMessage("  %s: an expression that shares state between notes\n", m->label);
		return false;
	}
	for (int i = 0; i < m->nbrInputs; ++i)
		if (!SubtreeBlockSafe(m->inputs[i].link, inNestedScore, depth + 1))
			return false;
	return true;
}

Boolean SSScore::BlockSafe()
{
	for (int i = 0; i < nbrInputs; ++i)
		if (!SubtreeBlockSafe(inputs[i].link, false, 0))
			return false;
	return true;
}

// GenerateOutput at the top level, where time is the sample number over the sample rate: play
// out a block, rendering the next one when it runs out.
ssfloat SSScore::BlockOutput(ssfloat pTime)
{
	if (blkPos < (int) blkTimes.size() && pTime == blkTimes[blkPos]) {
		lastRightSample = blkR[blkPos];
		return blkL[blkPos++];
	}
	if (blkPos > 0 && pTime == blkTimes[blkPos - 1]) {		// asked twice for one sample
		lastRightSample = blkR[blkPos - 1];
		return blkL[blkPos - 1];
	}
	PatchOwner	*po = parList->itsOwner;
	long		s = po->sampleNbr;
	if (pTime != (ssfloat) s / po->mainInst->sampleRate)
		ErrorExit("Score %s: asked for time %g out of sequence while rendering in blocks "
				  "(run with -b 0)", label, pTime);
	long		n = po->nbrSamples - s;
	if (n > gBlockSize)
		n = gBlockSize;
	if (n < 1)
		n = 1;
	blkTimes.resize(n);
	blkL.resize(n);
	blkR.resize(n);
	for (long k = 0; k < n; ++k)
		blkTimes[k] = (ssfloat) (s + k) / po->mainInst->sampleRate;
	RenderBlock(blkTimes.data(), (int) n, blkL.data(), blkR.data());
	blkPos = 1;
	lastRightSample = blkR[0];
	return blkL[0];
}

// What GenerateOutput would return (and leave in lastRightSample) at each of n times.
void SSScore::RenderBlock(const ssfloat *times, int n, ssfloat *outL, ssfloat *outR)
{
	int		done = 0;
	ssfloat	pTime = parList->pTime;		// the modules around us are still at this time

	while (done < n)
		done += RenderSegment(times + done, n - done, outL + done, outR + done);
	parList->pTime = pTime;
}

// Render from times[0] up to the end of the block or of the score section, whichever is first
// (a new section frees the notes of the last, which this block may still be rendering), and
// return the number of samples rendered.
int SSScore::RenderSegment(const ssfloat *times, int n, ssfloat *outL, ssfloat *outR)
{
	std::vector<ScoreVoice>	voices;
	std::vector<int>		slotVoice;		// the voice of each active note, -1 if it doesn't sound
	std::vector<int>		segStart, orderStart, order;	// summing order, from each change
	std::vector<ModList *>	dead;			// notes ended in this block, freed after rendering
	int						k;

	parList->pTime = times[0];
	if (!scoreDone && times[0] >= curSection->sectionStart + curSection->sectionLength)
		NextSection(times[0]);
	if (scoreDone) {				// GenerateOutput returns 0 and leaves lastRightSample as it is
		for (k = 0; k < n; ++k) {
			outL[k] = 0.0;
			outR[k] = lastRightSample;
		}
		return n;
	}

	for (int i = 0; i < nbrActiveNotes; ++i) {
		ActiveNotePtr ap = &activeNotes[i];
		if (ap->instr != NULL && ap->instr->nbrModules >= 1) {
			ScoreVoice sv = {ap->instr, ap->noteInfo, 0, -1};
			voices.push_back(sv);
			slotVoice.push_back((int) voices.size() - 1);
		}
		else
			slotVoice.push_back(-1);
	}

	// Which notes sound at each sample, and in what order GenerateOutput would add them: start
	// and end notes as it does, but leave the rendering for later.
	for (k = 0; k < n; ++k) {
		ssfloat	pTime = times[k];
		bool	changed = (k == 0);

		if (k > 0 && pTime >= curSection->sectionStart + curSection->sectionLength)
			break;
		while (curSection->noteNbr < curSection->nbrNotes &&
			   curSection->notes[curSection->noteNbr]->p2 + curSection->sectionStart <= pTime &&
			   parList->itsOwner->windowState == WS_Synthesize)
		{
			parList->pTime = pTime;			// as when GenerateOutput starts a note
			ActivateNextNote(pTime);
			ActiveNotePtr ap = &activeNotes[nbrActiveNotes - 1];
			if (ap->instr != NULL && ap->instr->nbrModules >= 1) {
				ScoreVoice sv = {ap->instr, ap->noteInfo, k, -1};
				voices.push_back(sv);
				slotVoice.push_back((int) voices.size() - 1);
			}
			else
				slotVoice.push_back(-1);
			changed = true;
		}
		int	i = 0;
		while (i < nbrActiveNotes) {
			ActiveNotePtr ap = &activeNotes[i];
			if (curSection->sectionStart + ap->noteInfo->p2 + ap->noteInfo->p3 <= pTime) {
				// KillNote, without freeing the instrument yet
				if (slotVoice[i] >= 0)
					voices[slotVoice[i]].k1 = k;
				if (ap->instr)
					dead.push_back(ap->instr);
				activeNotes[i] = activeNotes[nbrActiveNotes - 1];
				slotVoice[i] = slotVoice[nbrActiveNotes - 1];
				slotVoice.pop_back();
				--nbrActiveNotes;
				changed = true;
			}
			else
				++i;
		}
		if (changed) {
			segStart.push_back(k);
			orderStart.push_back((int) order.size());
			for (int j = 0; j < nbrActiveNotes; ++j)
				if (slotVoice[j] >= 0)
					order.push_back(slotVoice[j]);
		}
	}
	int		len = k;
	ssfloat	sectionStart = curSection->sectionStart;

	// Each note, for the samples it sounds
	size_t	nv = voices.size();
	std::vector<ssfloat>	vL(nv * len), vR(nv * len);
	for (size_t v = 0; v < nv; ++v)
		if (voices[v].k1 < 0)
			voices[v].k1 = len;
	// Hand the threads the slowest notes first (by what each took last block, new ones first),
	// so none is left finishing a slow one alone at the end
	std::vector<int>		byCost(nv);
	std::vector<double>		cost(nv);
	for (size_t v = 0; v < nv; ++v) {
		auto it = voiceCost.find(voices[v].instr);
		cost[v] = (it == voiceCost.end())? 1e9 : it->second;
		byCost[v] = (int) v;
	}
	std::sort(byCost.begin(), byCost.end(), [&](int a, int b) { return cost[a] > cost[b]; });
	ParallelFor((int) nv, [&](int i) {
		int		v = byCost[i];
		auto	t0 = std::chrono::steady_clock::now();
		RenderVoice(voices[v], times, sectionStart, &vL[v * len], &vR[v * len]);
		cost[v] = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
	});
	voiceCost.clear();
	for (size_t v = 0; v < nv; ++v)
		voiceCost[voices[v].instr] = cost[v];

	// Their sum at each sample, in GenerateOutput's order
	for (size_t g = 0; g < segStart.size(); ++g) {
		int		kA = segStart[g], kB = (g + 1 < segStart.size())? segStart[g + 1] : len;
		int		oA = orderStart[g], oB = (g + 1 < segStart.size())? orderStart[g + 1] : (int) order.size();
		for (k = kA; k < kB; ++k) {
			ssfloat	v = 0.0, r = 0.0;
			for (int o = oA; o < oB; ++o) {
				v += vL[order[o] * len + k];
				r += vR[order[o] * len + k];
			}
			outL[k] = v;
			outR[k] = r;
		}
	}
	lastRightSample = outR[len - 1];

	for (size_t d = 0; d < dead.size(); ++d)
		delete dead[d];
	return len;
}

// One note's samples k0 to k1, into outL and outR at the same indices.
void SSScore::RenderVoice(ScoreVoice &sv, const ssfloat *times, ssfloat sectionStart,
						  ssfloat *outL, ssfloat *outR)
{
	SSModule		*root = sv.instr->mods[0];
	VoiceContext	ctx = {this, sv.note, 0.0}, *outer = tVoice;

	tVoice = &ctx;
	if ((root->moduleType == MT_RandScore || root->moduleType == MT_CScore) &&
		((SSScore *) root)->blockMode)
	{
		// A score as the instrument: it renders the note's block itself
		std::vector<ssfloat>	nt(sv.k1 - sv.k0);
		for (int k = sv.k0; k < sv.k1; ++k)
			nt[k - sv.k0] = times[k] - (sectionStart + sv.note->p2);
		if (!nt.empty()) {
			root->callingMod = this;
			((SSScore *) root)->RenderBlock(nt.data(), (int) nt.size(), outL + sv.k0, outR + sv.k0);
		}
	}
	else if (root->CanRenderBlock()) {
		// The instrument renders the note's block itself, 256 samples at a time
		int		n = sv.k1 - sv.k0;
		ssfloat	nt[256];
		for (int k0 = 0; k0 < n; k0 += 256) {
			int	m = (n - k0 < 256)? n - k0 : 256;
			for (int i = 0; i < m; ++i)
				nt[i] = times[sv.k0 + k0 + i] - (sectionStart + sv.note->p2);
			ctx.noteTime = nt[m - 1];
			root->RenderBlock(this, nt, m, outL + sv.k0 + k0, outR + sv.k0 + k0);
		}
		root->parList->InitTime(ctx.noteTime);
	}
	else {
		for (int k = sv.k0; k < sv.k1; ++k) {
			ctx.noteTime = times[k] - (sectionStart + sv.note->p2);
			outL[k] = root->GenerateOutputTime(this, ctx.noteTime);
			outR[k] = root->getRightSample();
		}
	}
	tVoice = outer;
}
