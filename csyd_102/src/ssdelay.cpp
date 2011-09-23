// ssdelay.cp

#include "ss.h"
#include "ssdelay.h"
#include "patchowner.h"

#include <math.h>

enum {SIG_Sig, SIG_Ctl, SIG_Ctl1,SIG_Ctl2,SIG_Ctl3};

SSDelay::SSDelay(ModList * mList, short h, short v) : SSModule(MT_Delay, mList, h, v)
{
	InitExp(&delayExp,"0.1");
	InitExp(&a0Exp,"1.0");
	InitExp(&a1Exp,"0.5");
	allocDelaySamples = 0;
	dBuf = NULL;
	delayCounter = 0;
	delayIncrement = 0;
	flags = DF_Recursive;
	DescribeLink(SIG_Sig, "Signal to Delay", "sig",0,0,0xFFFF);
	DescribeLink(SIG_Ctl, "Control Signal", "ctl",0xFFFF,0x8888,0x8888);
	DescribeLink(SIG_Ctl1, "Control Signal 1", "ctl1",0xFFFF,0x8888,0);
	DescribeLink(SIG_Ctl2, "Control Signal 2", "ctl2",0xFFFF,0x8888,0);
	DescribeLink(SIG_Ctl3, "Control Signal 3", "ctl3",0xFFFF,0x8888,0);
}

void SSDelay::CleanUp()
{
	if (dBuf) {
		MyDisposePtr(dBuf);
		dBuf = NULL;
	}
	allocDelaySamples = 0;
	SSModule::CleanUp();
}

void	SSDelay::Copy(SSModule *ss)
{
	SSDelay *sa = (SSDelay *) ss;
	SSModule::Copy(ss);
	CopyExp(&sa->delayExp,&delayExp);
	CopyExp(&sa->a0Exp,&a0Exp);
	CopyExp(&sa->a1Exp,&a1Exp);
	flags = sa->flags;
}

/*
void SSDelay::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"DELF %d\r",flags);
	WriteFileLine(ar,"DELd %s\r",delayExp.exp);
	WriteFileLine(ar,"DELa0 %s\r",a0Exp.exp);
	WriteFileLine(ar,"DELa1 %s\r",a1Exp.exp);
}
*/

void SSDelay::Load(FILE* ar) 
{
	char				*p,tbuf[512];
	p = parList->GetNextInputLine(ar,"DEL",tbuf);
	if (p[3] == ' ') {
		// Old Style
		double				dly=0,gain=0;
		int					f=0;
		sscanf(p, "DEL %lf %lf %d",&dly,&gain,&f);
		PrintfExp(&delayExp,"%g",dly);
		PrintfExp(&a0Exp,"%g",1.0/(1.0+gain));
		PrintfExp(&a1Exp,"%g",gain/(1.0+gain));
		if (f)
			flags = DF_Recursive;
		else
			flags = 0;
	}
	else {
		sscanf(p, "DELF %d",&flags);
		LoadExp(ar,"DELd",&delayExp);
		LoadExp(ar,"DELa0",&a0Exp);
		LoadExp(ar,"DELa1",&a1Exp);
	}
}

void   SSDelay::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	delay = ResetExp(&delayExp,callingMod);
	a0 = ResetExp(&a0Exp,callingMod);
	a1 = ResetExp(&a1Exp,callingMod);
	// !!! Find a more reliable way to compute
	allocDelaySamples = (long) fabs(delay * parList->itsOwner->mainInst->sampleRate) + 22000;
	delayIncrement = (long) fabs(delay * parList->itsOwner->mainInst->sampleRate);
	delayCounter = 0;
	dBuf = (ssfloat *) MyNewPtrClear(sizeof(ssfloat) * allocDelaySamples);
	if (dBuf == NULL) {
		ErrorMessage("Can't allocate delay buffer");
		parList->itsOwner->AbortSynthesis();
		return;
	}
	for (delayCounter = 0; delayCounter < allocDelaySamples; ++delayCounter)
		dBuf[delayCounter] = 0.0;
	delayCounter = 0;
	oldDelay = delay;
}

ssfloat SSDelay::GenerateOutput(SSModule *callingMod)
{
	if (dBuf == NULL)
		return 0.0;

	ssfloat	x,y;

	delay = SolveExp(&delayExp,callingMod);
	a0 = SolveExp(&a0Exp,callingMod);
	a1 = SolveExp(&a1Exp,callingMod);

	if (oldDelay != delay) {
		int	oldMax = allocDelaySamples;
		delayIncrement = (long) fabs(delay * parList->itsOwner->mainInst->sampleRate);
		if (delayIncrement > allocDelaySamples) {
			ssfloat	*nb;
			allocDelaySamples = delayIncrement+22000;
			nb = (ssfloat *) MyNewPtrClear(sizeof(ssfloat) * allocDelaySamples);
			if (nb == NULL) {
				ErrorMessage("Can't allocate delay buffer");
				parList->itsOwner->AbortSynthesis();
				return 0.0;
			}
			if (dBuf != NULL) {
				// rotate buffer
				memcpy(nb,dBuf+delayCounter,sizeof(ssfloat) * (oldMax - delayCounter));
        memcpy(nb+(oldMax - delayCounter),dBuf,sizeof(ssfloat) * delayCounter);
        MyDisposePtr(dBuf);
			}
			dBuf = nb;
			delayCounter = 0;
		}
		oldDelay = delay;
	}

	x = MixInputs(SIG_Sig, callingMod);
	y = a0*x + a1*dBuf[(delayCounter + delayIncrement) % allocDelaySamples];
	if ((flags & DF_Recursive) > 0)
		dBuf[delayCounter] = y;
	else
		dBuf[delayCounter] = x;
	delayCounter--;
	if (delayCounter < 0)
		delayCounter = allocDelaySamples-1;

  // Mono reverb only!!!
  lastRightSample = y;
	return y;
}



