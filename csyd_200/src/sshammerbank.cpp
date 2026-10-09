// SSHammerBank.cp
//

#include "ss.h"
#include "sshammerbank.h"
// #include "MainWin.h"
#include "ssoutput.h"
#include "patchowner.h"

#include <math.h>

// Global array of hammerbanks
HammerBankPtr	hammerBanks[MaxHammerBanks];

// void	SSHammerBank::Initialize(short itsType, short h, short v)
SSHammerBank::SSHammerBank(ModList * mList, short h, short v) : SSModule(MT_HammerBank, mList, h, v)
{
	InitExp(&bNbrExp,"0");
	InitExp(&kFreqExp,"cpsmidi(k)");
	InitExp(&kAmpExp,"1.0");
	InitExp(&kAttackExp,"0.05");
	InitExp(&kDecayExp,"0.1");
	InitExp(&sustainExp,"1.0");
	InitExp(&waveformExp,"sin(t*2*pi)");
	DescribeLink(0, "Instrument 1", "i1",0xFFFF,0x0000,0xFFFF);
	DescribeLink(1, "Instrument 2", "i2",0xDDDD,0x0000,0xDDDD);
	DescribeLink(2, "Instrument 3", "i3",0xBBBB,0x0000,0xBBBB);
	DescribeLink(3, "Instrument 4", "i4",0x9999,0x0000,0x9999);
	DescribeLink(4, "Instrument 5", "i5",0x7777,0x0000,0x7777);
	DescribeLink(5, "Instrument 6", "i6",0x5555,0x0000,0x5555);
	DescribeLink(6, "Instrument 7", "i7",0x3333,0x0000,0x3333);
	DescribeLink(7, "Instrument 8", "i8",0x1111,0x0000,0x1111);
	DescribeLink(8, "Control Signal", "ctl",	0xFFFF,0x1111,0xeeee);
	DescribeLink(9, "Alt Control Sig #1", "ctl1",	0xFFFF,0x2222,0xdddd);
	DescribeLink(10, "Alt Control Sig #2", "ctl2",	0xFFFF,0x3333,0xcccc);
	DescribeLink(11, "Alt Control Sig #3", "ctl3",	0xFFFF,0x4444,0xbbbb);
	hb = NULL;
	bNbr = 0;
}

void SSHammerBank::InitHammerBank()
{
	hb = NULL;
	if (bNbr < 0 || bNbr >= MaxHammerBanks)
		return;
	hammerBanks[bNbr] = (HammerBankPtr) MyNewPtrClear(sizeof(HammerBankRec));
	hb = hammerBanks[bNbr];
}

void SSHammerBank::DisposeHammerBank()
{
	if (hb) {
		int	k;
		for (k = 0; k < MaxKeysPerBank; ++k) {
			if (hb->keys[k].instr) {
				delete hb->keys[k].instr;
				hb->keys[k].instr = NULL;
			}
		}
	}
	if (hammerBanks[bNbr] != NULL) {
		MyDisposePtr((Ptr) hammerBanks[bNbr]);
		hammerBanks[bNbr] = NULL;
	}
	hb = NULL;
}

ssfloat SSHammerBank::GetKeyValue()
{
	return keyNumber;
}

ssfloat SSHammerBank::GetAValue()
{
	return keyInten;
}

ssfloat SSHammerBank::GetInstParameter(int n)
{
	switch (n) {
	case 4:	return currentKey->energy;		break;
	case 5:	return currentKey->freq;		break;
	case 6:	return currentKey->velocity;	break;	// etc...
	default:	return callingMod->GetInstParameter(n);
	}	
}


void SSHammerBank::Reset(SSModule *callingMod)
{
	int	k;
	int	instNbr = -1;
	ActionPtr	ar;

	SSModule::Reset(callingMod);

	bNbr = (int) ResetExp(&bNbrExp, callingMod);
	ResetExp(&kFreqExp, callingMod);
	ResetExp(&kAmpExp, callingMod);
	ResetExp(&kAttackExp, callingMod);
	ResetExp(&kDecayExp, callingMod);
	sustain = ResetExp(&sustainExp, callingMod);
	ResetExp(&waveformExp, callingMod);

	InitHammerBank();

	if (hb == NULL) {
		LogMessage("Error allocating Hammer Bank\n");
		parList->itsOwner->AbortSynthesis();
		return;
	}
	if (CountInputs(0) > 0) {
		int	i;
		for (i = 0; i < nbrInputs; ++i) {
			if (inputs[i].inputType == 0) {
				instNbr = i;
				break;
			}
		}
	}
	for (k = 0,ar=hb->keys; k < MaxKeysPerBank && parList->itsOwner->windowState == WS_Synthesize; ++k,++ar) {
		// Initialize action for hammeraction #
		keyNumber = k;
		currentKey = ar;
		ar->energy = 0;
		ar->undampen = 0;
		ar->freq = SolveExp(&kFreqExp, callingMod);
		ar->amp = SolveExp(&kAmpExp, callingMod);
		ar->attack = SolveExp(&kAttackExp, callingMod);
		// Decay per sample = attenpersec ^ (1 / SR)
		ar->decay = pow(SolveExp(&kDecayExp, callingMod),parList->itsOwner->timeInc);
		ar->waveInc = 0;
		ar->attackCtr = 0;
		ar->attackSamples = (long) (ar->attack * parList->itsOwner->mainInst->sampleRate);
		ar->attackFlag = 0;
		ar->velocity = 0;
		ar->attackEnergy = 0;
		ar->decayCtr = 0;
		ar->decaySamples = (long) (0.02 * parList->itsOwner->mainInst->sampleRate);
		ar->decayFlag = false;
		ar->decayEnergy = false;
		if (instNbr != -1) {
			ar->instr = parList->CloneInstrument(inputs[instNbr].link);
			if (ar->instr)
				ar->instr->ResetInstruments(this);
		}

	}	
	lastTime = lastOutput = -1;
}

ssfloat	SSHammerBank::GenerateOutput(SSModule *callingMod)
{
	ssfloat	v = 0.0,vk,sustain;
	int		k;
	ActionPtr	ar;

	if (parList->pTime == lastTime)
		return lastOutput;
	lastTime = parList->pTime;

	if (hb == NULL)
		goto ReturnLabel;

	sustain = SolveExp(&sustainExp, callingMod);

	for (k = 0,ar=hb->keys; k < MaxKeysPerBank; ++k,++ar) {
		if (ar->energy > 0.0 || ar->attackFlag) {
			currentKey = ar;
			keyNumber = k;
			if (ar->energy > 0 && !ar->decayFlag && ar->undampen+sustain == 0) {
				ar->decayFlag = true;
				ar->decayCtr = 0;
				ar->decayEnergy = ar->energy / ar->decaySamples;
			}
			if (ar->attackFlag) {
				if (ar->attackCtr < ar->attackSamples) {
					ar->energy += ar->attackEnergy;
					++ar->attackCtr;
				}
				else
					ar->attackFlag = 0;
			}
			else if (ar->decayFlag) {
				if (ar->decayCtr < ar->decaySamples) {
					ar->energy -= ar->decayEnergy;
					++ar->decayCtr;
					if (ar->energy < 0) {
						ar->energy = 0;
						ar->decayFlag = false;
					}
				}
				else {
					ar->decayFlag = 0;
					ar->energy = 0;
				}
			}
			else {
				ar->energy *= ar->decay;
			}
			keyInten = ar->energy;
			if (ar->instr) {
				vk = ((SSModule *) ar->instr->mods[0])->GenerateOutputTime(this,parList->pTime);
				vk *= ar->energy;
			}
			else {
				ar->waveInc += parList->itsOwner->timeInc * ar->freq;
				ar->waveInc -= (int) ar->waveInc;
				// Add value based on wavetable oscillator...
				parList->PushTime(ar->waveInc);
				vk = SolveExp(&waveformExp, callingMod);
				parList->PopTime();
				vk *= ar->energy;
			}
			v += vk;
		}
	}
ReturnLabel:
	lastOutput = v;
	lastRightSample = v;
	return v;
}


void SSHammerBank::CleanUp()
{
	DisposeHammerBank();
}



void SSHammerBank::Copy(SSModule *ss)
{
	SSHammerBank	*osc = (SSHammerBank *) ss;
	SSModule::Copy(ss);
	CopyExp(&osc->bNbrExp, &bNbrExp);
	CopyExp(&osc->kFreqExp, &kFreqExp);
	CopyExp(&osc->kAmpExp, &kAmpExp);
	CopyExp(&osc->kAttackExp, &kAttackExp);
	CopyExp(&osc->kDecayExp, &kDecayExp);
	CopyExp(&osc->sustainExp, &sustainExp);
	CopyExp(&osc->waveformExp, &waveformExp);
	CopyExp(&osc->kAttackExp, &kAttackExp);
}

/*
void SSHammerBank::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"HAMB\r");
	WriteFileLine(ar,"HAMBN %s\r",bNbrExp.exp);
	WriteFileLine(ar,"HAMBF %s\r",kFreqExp.exp);
  WriteFileLine(ar,"HAMBA %s\r",kAmpExp.exp);
  WriteFileLine(ar,"HAMBAt %s\r",kAttackExp.exp);
  WriteFileLine(ar,"HAMBD %s\r",kDecayExp.exp);
  WriteFileLine(ar,"HAMBS %s\r",sustainExp.exp);
  WriteFileLine(ar,"HAMBW %s\r",waveformExp.exp);
}
*/

void SSHammerBank::Load(FILE* ar) 
{
	char				*p,tbuf[256];
	int					var=0;

	p = parList->GetNextInputLine(ar,"HAMB",tbuf);
	LoadExp(ar,"HAMBN",&bNbrExp);
	LoadExp(ar,"HAMBF",&kFreqExp);
	LoadExp(ar,"HAMBA",&kAmpExp);
	LoadExp(ar,"HAMBAt",&kAttackExp);
	LoadExp(ar,"HAMBD",&kDecayExp);
	LoadExp(ar,"HAMBS",&sustainExp);
	LoadExp(ar,"HAMBW",&waveformExp);
}

// Global function for HammerActuator
// implement as static
void ActuateKey(int bNbr, int kNbr, int flags, ssfloat velocity, ssfloat undampen)
{
	HammerBankPtr	hb;
	ActionPtr		ar;
	if (bNbr < 0 || bNbr >= MaxHammerBanks)
		return;
	hb = hammerBanks[bNbr];
	if (hb == NULL)
		return;
	if (kNbr < 0 || kNbr >= MaxKeysPerBank)
		return;
	ar = &hb->keys[kNbr];
	ar->velocity = velocity * ar->amp;	// apply keyboard amp scaling here...
	if (flags & HB_SympatheticRes) {
		// Sympathetic resonnance
	}
	ar->undampen = undampen;
	if (undampen > 0.0) {
		ar->attackCtr = 0;
		ar->attackFlag = true;
		ar->attackEnergy = ar->velocity / (double) ( ar->attackSamples-1);
	}
}
