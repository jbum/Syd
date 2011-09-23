// SSADSR.cp
// #include "stdafx.h"	// Windows Only

// SSModule.c

#include "ss.h"
#include "ssadsr.h"

SSADSR::SSADSR(ModList * mList, short h, short v) : SSModule(MT_Envelope, mList, h, v)
{
  InitExp(&attackTExp, "0.1");
  InitExp(&attackLExp, "1.0");
  InitExp(&decayTExp, "0.2");
  InitExp(&decayLExp, "0.7");
  InitExp(&sustainTExp, "0.6");
  InitExp(&sustainLExp, "0.6");
  InitExp(&releaseTExp, "0.1");
  InitExp(&durExp,"1.0");
  useTrigger = 0;
  interruptsOK = 0;
	envPhase = 0;
	DescribeLink(0, "Trigger Signal (unused)", "trig",0xFFFF,0x0000,0x0000);
}

ssfloat SSADSR::GenerateOutput(SSModule *callingMod)
{
	ssfloat	retVal = 0.0;
	ssfloat deltaT;
	ssfloat pTime = parList->pTime;

  if (useTrigger != 0) {
    // Figure out if triggering

    if (MixInputs(0,callingMod) >= .5)
    {
      if (!lastTrigger &&
           (envPhase == ADSR_Idle || interruptsOK != 0))
      {
          TriggerAttack(pTime);
          lastTrigger = true;
      }
    }
    else
      lastTrigger = false;

  }

Recompute:
	deltaT = (pTime - startTime)*durScale;

	switch (envPhase) {
	case ADSR_Idle:	// Not firing
		break;
	case ADSR_Attack:
		// Attack - go from 0 to attackL in time AttachT
		if (deltaT < attackT) {
			retVal = attackL * deltaT / attackT;
		}
		else {
			++envPhase;
			startTime = pTime;
			goto Recompute;
		}
		break;
	case ADSR_Decay:
		// Decay - go from attackL to decayL in time DecayT
		if (deltaT < decayT) {
			retVal = attackL + (decayL - attackL) * deltaT / decayT;
		}
		else {
			++envPhase;
			startTime = pTime;
			goto Recompute;
		}
		break;
	case ADSR_Sustain:
		// Sustain - go from decayL to sustainL in time SustainT
		if (deltaT < sustainT) {
			retVal = decayL + (sustainL - decayL) * deltaT / sustainT;
		}
		else {
			++envPhase;
			startTime = pTime;
			goto Recompute;
		}
		break;
	case ADSR_Release:
		// Release - go from sustainL to 0 in time ReleaseT
		if (deltaT < releaseT) {
			retVal = sustainL - sustainL * deltaT / releaseT;
		}
		else {
			envPhase = ADSR_Idle;
			retVal = 0.0;
		}
		break;
	}
  lastRightSample = retVal;
	return retVal;
}

void SSADSR::TriggerAttack(ssfloat start)
{
  duration = ResetExp(&durExp, callingMod);
  attackT = SolveExp(&attackTExp, callingMod);
  attackL = SolveExp(&attackLExp, callingMod);
  decayT = SolveExp(&decayTExp, callingMod);
  decayL = SolveExp(&decayLExp, callingMod);
  sustainT = SolveExp(&sustainTExp, callingMod);
  sustainL = SolveExp(&sustainLExp, callingMod);
  releaseT = SolveExp(&releaseTExp, callingMod);
  durScale = 1/duration;
  envPhase = ADSR_Attack;
  startTime = start;
}

void SSADSR::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	duration = ResetExp(&durExp, callingMod);
  lastTrigger = false;
  if (duration > 0 && useTrigger == 0) {
    TriggerAttack(0.0);
	}
	else {
		durScale = 1;
		envPhase = ADSR_Idle;
		startTime = 0;
	}
}

#ifdef UI_FEATURES
void SSADSR::Save(FILE* ar)
{
	SSModule::Save(ar);
  WriteFileLine(ar,"ADSR2 %d\r",useTrigger+interruptsOK*2);
  WriteFileLine(ar,"ADSRAT %s\r", attackTExp.exp);
  WriteFileLine(ar,"ADSRAL %s\r", attackLExp.exp);
  WriteFileLine(ar,"ADSRDT %s\r", decayTExp.exp);
  WriteFileLine(ar,"ADSRDL %s\r", decayLExp.exp);
  WriteFileLine(ar,"ADSRST %s\r", sustainTExp.exp);
  WriteFileLine(ar,"ADSRSL %s\r", sustainLExp.exp);
  WriteFileLine(ar,"ADSRRT %s\r", releaseTExp.exp);
  WriteFileLine(ar,"ADSRD %s\r",durExp.exp);
}
#endif

void SSADSR::Load(FILE* ar)
{
	char				*p,tbuf[256],ebuf[32];
	double				at=0.0,al=0.0,dt=0.0,dl=0.0,st=0.0,sl=0.0,rt=0.0;
	p = parList->GetNextInputLine(ar,"ADSR",tbuf);
  if (p[4] == ' ') {
    sscanf(p, "ADSR %lf %lf %lf %lf %lf %lf %lf",
      &at,&al,&dt,&dl,&st,&sl,&rt);
    PrintfExp(&attackTExp, "%g", at);
    PrintfExp(&attackLExp, "%g", al);
    PrintfExp(&decayTExp, "%g", dt);
    PrintfExp(&decayLExp, "%g", dl);
    PrintfExp(&sustainTExp, "%g", st);
    PrintfExp(&sustainLExp, "%g", sl);
    PrintfExp(&releaseTExp, "%g", rt);
    LoadExp(ar,"ADSRD",&durExp);
    useTrigger = 0;
    interruptsOK = 0;
  }
  else {
    int flags;
    sscanf(p, "ADSR2 %d",&flags);
    useTrigger = (flags & 1) > 0;
    interruptsOK = (flags & 2) > 0;
    LoadExp(ar,"ADSRAT",&attackTExp);
    LoadExp(ar,"ADSRAL",&attackLExp);
    LoadExp(ar,"ADSRDT",&decayTExp);
    LoadExp(ar,"ADSRDL",&decayLExp);
    LoadExp(ar,"ADSRST",&sustainTExp);
    LoadExp(ar,"ADSRSL",&sustainLExp);
    LoadExp(ar,"ADSRRT",&releaseTExp);
    LoadExp(ar,"ADSRD",&durExp);
  }
}  
void SSADSR::Copy(SSModule *ss)
{
  SSADSR *sa = (SSADSR *) ss;
  SSModule::Copy(ss);
  CopyExp(&sa->attackTExp,&attackTExp);
  CopyExp(&sa->attackLExp,&attackLExp);
  CopyExp(&sa->decayTExp,&decayTExp);
  CopyExp(&sa->decayLExp,&decayLExp);
  CopyExp(&sa->sustainTExp,&sustainTExp);
  CopyExp(&sa->sustainLExp,&sustainLExp);
  CopyExp(&sa->releaseTExp,&releaseTExp);
  CopyExp(&sa->durExp, &durExp);
  useTrigger = sa->useTrigger;
  interruptsOK = sa->interruptsOK;
}
  
