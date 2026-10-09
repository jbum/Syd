// SSADSR.h
#ifndef _H_SSADSR
#define _H_SSADSR	1

#include "ssmodule.h"

// class ExpRec;

enum {ADSR_Idle, ADSR_Attack, ADSR_Decay, ADSR_Sustain, ADSR_Release};

class SSADSR : public SSModule {
	// Stored
  ExpRec    attackTExp,decayTExp,sustainTExp,releaseTExp;
  ExpRec    attackLExp,decayLExp,sustainLExp;
	ExpRec		durExp;
  int       useTrigger, interruptsOK;
  
	// Internal
  ssfloat   attackT,decayT,sustainT,releaseT;
  ssfloat   attackL,decayL,sustainL;
	ssfloat		startTime;
	ssfloat		durScale,duration;
	int			  envPhase;
  bool      lastTrigger;

public:
	// Overrides
	SSADSR(ModList * mList, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Reset(SSModule *callingMod);
	void	Load(FILE* ar);
  void  Copy(SSModule *ss);

#ifdef UI_FEATURES
  int   ProcessDoubleClick();
  void Save(FILE* ar);
#endif
private:
  void TriggerAttack(ssfloat start);
};

#endif
