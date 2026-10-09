// ssscore.h
#ifndef _H_SSScore
#define _H_SSScore	1

#include "ssmodule.h"

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

	// Overrides
	SSScore(short modType, ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Reset(SSModule *callingMod);
	void	CleanUp();
	ssfloat GetInstParameter(int n);
	ssfloat GetNoteTime();
	Boolean	IsInScore();

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
