// ssthreshhold.cp

#include "ss.h"
#include "ssthreshhold.h"

SSThreshhold::SSThreshhold(ModList * mList, short h, short v) : SSModule(MT_Threshhold, mList, h, v)
{
	InitExp(&cutOffExp,"0.5");
	DescribeLink(0, "Signal for Threshhold", "sig",0,0,0xFFFF);
}

void	SSThreshhold::Copy(SSModule *ss)
{
	SSThreshhold *sa = (SSThreshhold *) ss;
	SSModule::Copy(ss);
	CopyExp(&sa->cutOffExp,&cutOffExp);
}

/*
void SSThreshhold::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"THR %s\r",cutOffExp.exp);
}
*/

void SSThreshhold::Load(FILE* ar) 
{
	LoadExp(ar,"THR",&cutOffExp);
}


ssfloat SSThreshhold::GenerateOutput(SSModule *callingMod)
{
	ssfloat	retVal,cutOff;
	cutOff = SolveExp(&cutOffExp,callingMod);
	retVal = MixInputs(-1,callingMod) >= cutOff? 1 : 0;
  lastRightSample = lastRightInput >= cutOff? 1.0 : 0.0;
	return retVal;
}

void SSThreshhold::Reset(SSModule *callingMod)
{
	ResetExp(&cutOffExp,callingMod);
}

