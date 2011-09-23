// SSRandScore.h


#ifndef _H_SSRandScore

#define _H_SSRandScore	1

#include "ssscore.h"

#define MaxParams	16

class SSRandScore : public SSScore {
public:
	// statics
	ExpRec	nbrNotesExp;
	ExpRec	pExp[MaxParams];

	// Dynamics
	ssfloat	noteIndex;

	// Constructor
	SSRandScore(ModList * mList, short h, short v);

	// Local
	void AddRandomNoteToScore(ScoreSectionPtr sp, int inst, 
							ssfloat start, ssfloat dur, ssfloat *p); 
	int	CountActiveExpressions();
	// Overrides

	void GenerateScore(SSModule *callingMod);
	void Save(FILE* ar);
	void Load(FILE* ar);
	int ProcessDoubleClick();
	void Copy(SSModule *ss);
	ssfloat GetIValue();
};



#endif
