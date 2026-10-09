// ssstk.cp
//
// STK output is normalized to 0-1, to make it easier to
// use for amplitude control etc.

#include "ss.h"
#include "ssstk.h"
// #include "mainwin.h"
#include "patchowner.h"

#include "mandolin.h" // stk

#include <math.h>


// void	SSStk::Initialize(short itsType, short h, short v)
SSStk::SSStk(ModList * mList, short h, short v) : SSModule(MT_STK, mList, h, v)
{
	InitExp(&freqExp,"440");
	InitExp(&durExp,"2");
	InitExp(&ampExp,"1");
	InitExp(&decayExp,"10");
	variant = STK_Normal;
	DescribeLink(0, "INVALID", "???",0xFFFF,0xFFFF,0xFFFF);
	error = false;
	d = NULL;
	b3 = NULL;

}

void SSStk::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);

  // Move this to general init...

	ssfloat f1 = ResetExp(&freqExp, callingMod);
	ssfloat amp = ResetExp(&ampExp, callingMod);
	if (b3)
		((Mandolin *) b3)->clear();
	else
		b3 = new Mandolin(5.0f);
	b3->noteOn(f1, amp);
}

ssfloat	SSStk::GenerateOutput(SSModule *callingMod)
{
	return b3->tick();
}

void SSStk::CleanUp()
{
	if (b3) {
		delete b3;
		b3 = NULL;
	}
}

void SSStk::Copy(SSModule *ss)
{
	SSStk	*osc = (SSStk *) ss;
	SSModule::Copy(ss);
	variant = osc->variant;
	CopyExp(&osc->freqExp, &freqExp);
	CopyExp(&osc->ampExp, &ampExp);
	CopyExp(&osc->durExp, &durExp);
	CopyExp(&osc->decayExp, &decayExp);
}

/*
void SSStk::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"STKI %d\r",(int) variant);
	WriteFileLine(ar,"STKF %s\r",freqExp.exp);
	WriteFileLine(ar,"STKD %s\r",durExp.exp);
	WriteFileLine(ar,"STKA %s\r",ampExp.exp);
	WriteFileLine(ar,"STKd %s\r",decayExp.exp);
}
*/

void SSStk::Load(FILE* ar)
{
	char				*p,tbuf[512];
	int					var=0;

	p = parList->GetNextInputLine(ar,"STKI",tbuf);
	sscanf(p, "STKI %d",&var);
	variant = var;
	LoadExp(ar,"STKF",&freqExp);
	LoadExp(ar,"STKD",&durExp);
	LoadExp(ar,"STKA",&ampExp);
	LoadExp(ar,"STKd",&decayExp);
}
