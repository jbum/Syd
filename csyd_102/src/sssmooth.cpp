// sssmooth.cp
#include "ss.h"
#include "sssmooth.h"
// #include "mainwin.h"

SSSmooth::SSSmooth(ModList * mList, short h, short v) : SSModule(MT_Smooth, mList, h, v)
{
	lastSample = 0;
	lastSampleR = 0;
	DescribeLink(0, "Signal to Smooth", "sig",0,0,0xFFFF);
}

ssfloat SSSmooth::GenerateOutput(SSModule *callingMod)
{
	ssfloat	v,av;
	v = MixInputs(-1, callingMod);
	av = (lastSample + v) / 2;
  
  lastRightSample = (lastSampleR + lastRightInput) / 2; // stereo right...


	lastSample = v;
  lastSampleR = lastRightInput; // smooth mem right

	return av;
}

void SSSmooth::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	lastSample = 0;
	lastSampleR = 0;
}
