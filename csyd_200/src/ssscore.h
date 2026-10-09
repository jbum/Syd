// ssscore.h
#ifndef _H_SSScore
#define _H_SSScore	1

#include "ssmodule.h"
#include <vector>
#include <unordered_map>

typedef struct ScoreSection {
	int				nbrNotes,noteNbr,allocNotes;
	ssfloat			sectionStart;
	ssfloat			sectionLength;
	ssfloat			tempoScale,tempoStart,timeStart;
	NoteEventPtr	*notes;
} ScoreSection, *ScoreSectionPtr;

typedef struct ActiveNote {
	NoteEventPtr	noteInfo;
	ModList			*instr;
} ActiveNote, *ActiveNotePtr;

// A note sounding during part of a block (see SSScore::RenderSegment): samples k0 to k1.
struct ScoreVoice {
	ModList			*instr;
	NoteEventPtr	note;
	int				k0, k1;
};

class SSScore : public SSModule {
public:
	// dynamics
	int				nbrSections;
	ScoreSectionPtr	sections;
	ScoreSectionPtr	curSection;
	int				sectionIndex;
	ssfloat			noteTime;
	NoteEventPtr	instParams;

	// NoteEventPtr	*nextNote;
	int				nbrActiveNotes,allocActiveNotes;
	ActiveNotePtr	activeNotes;
	Boolean			scoreDone;
	ssfloat			curTempo;

	// Block rendering: notes rendered a block of samples at a time (see RenderBlock)
	Boolean			blockMode;
	std::vector<ssfloat>	blkTimes, blkL, blkR;	// the block being played out, at the top level
	int				blkPos;
	std::unordered_map<ModList *, double>	voiceCost;	// seconds each note took last block

	// Overrides
	SSScore(short modType, ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Reset(SSModule *callingMod);
	void	CleanUp();
	ssfloat GetInstParameter(int n);
	ssfloat GetNoteTime();
	Boolean	IsInScore();

	// Block rendering
	Boolean	BlockSafe();
	ssfloat	BlockOutput(ssfloat pTime);
	void	RenderBlock(const ssfloat *times, int n, ssfloat *outL, ssfloat *outR);
	int		RenderSegment(const ssfloat *times, int n, ssfloat *outL, ssfloat *outR);
	void	RenderVoice(ScoreVoice &sv, const ssfloat *times, ssfloat sectionStart,
						ssfloat *outL, ssfloat *outR);

	// Internal
	void ActivateNextNote(ssfloat pTime);
	void KillNote(int noteIndex);
	void TerminateActiveNotes();
	void EndScore();
	void InitScore(SSModule *callingMod);
	void NextSection(ssfloat pTime);
	void AddSection();
	void InitializeActiveNote(int i, NoteEventPtr	np);

	NoteEventPtr AddNoteEvent(ScoreSectionPtr sp, int flags, int inst, 
							ssfloat start, ssfloat dur, int nbrOptParams);
	void SortScore();
	// void SetInstrumentParameters(NoteEventPtr noteParams);

	// Score Virtuals
	virtual void GenerateScore(SSModule *callingMod);
	virtual void PostScoreProcessing();

};

#endif
