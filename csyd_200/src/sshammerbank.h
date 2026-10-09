// SSHammerBank.h
#ifndef _H_SSHammerBank
#define _H_SSHammerBank	1

#include "ssmodule.h"
#include "expmgr.h"

// #define HammerBankDLOG		220

#define MaxHammerBanks		64
#define MaxKeysPerBank		128
#define HB_SympatheticRes	1

typedef struct {
	ModList	*instr;
	ssfloat	energy;	// String Energy - decays from 1.0 to 0
	ssfloat	decay;	// Decay for energy
	ssfloat	freq;	// frequency for this key
	ssfloat	amp;	// amplitude scale for this key
	ssfloat	undampen;	// typically 1 when undamped, 0 when damped
	ssfloat	waveInc;
	ssfloat	attack;
	ssfloat	velocity;
	ssfloat	attackEnergy,decayEnergy;
	long	attackCtr,attackSamples,decayCtr,decaySamples;
	Boolean	attackFlag,decayFlag;
} ActionRec, *ActionPtr;

typedef struct {
	ActionRec	keys[MaxKeysPerBank];
} HammerBankRec, *HammerBankPtr;

class SSHammerBank : public SSModule {
public:
	// Static
	ExpRec		bNbrExp,kFreqExp,kAmpExp,
				kDecayExp,sustainExp,waveformExp,kAttackExp;

	// Dynamic
	int				bNbr;
	ssfloat			sustain,keyNumber,keyInten,lastTime,lastOutput;
	HammerBankPtr	hb;
	ActionPtr		currentKey;

	// Local Functions
	void InitHammerBank();
	void DisposeHammerBank();

	// Overrides
	SSHammerBank(ModList * mList, short h, short v);

	void	Initialize(short itsType, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Reset(SSModule *callingMod);
	void	CleanUp();
	void	Copy(SSModule *ss);
	void	Load(FILE* ar);
	ssfloat	GetKeyValue();	// Changed from GetKValue on Mac due to name conflict
	ssfloat	GetAValue();
	ssfloat GetInstParameter(int n);
  // void  Save(FILE* ar);
  // int   ProcessDoubleClick();
};

// Global Functions
void ActuateKey(int bNbr, int kNbr, int flags, ssfloat velocity, ssfloat undampen);

#endif
