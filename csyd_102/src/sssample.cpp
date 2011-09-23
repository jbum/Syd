// sssampleandhold.cp

#include "ss.h"
#include "sssample.h"
// #include "MainWin.h"

SSSampleAndHold::SSSampleAndHold(ModList * mList, short h, short v) : SSModule(MT_SampleAndHold, mList, h, v)
{
	lastTrigger = false;
	// !!! Fix (patches) so that sampled signal is a0..
	DescribeLink(0, "Sampled Signal", "sig",0,0,0xFFFF);
	DescribeLink(1, "Trigger Signal", "trig",0xFFFF,0x0000,0x0000);
}

ssfloat SSSampleAndHold::GenerateOutput(SSModule *callingMod)
{
	ssfloat	v;

	// Oscillators don't work properly unless this is called
	// everytime (although it IS inefficient...)

	v = MixInputs(0,callingMod);

	if (MixInputs(1,callingMod) >= .5) {
		if (!lastTrigger) {
			lastOutput = v;
			lastRightSample = v;
    }
		lastTrigger = true;
	}
	else
		lastTrigger = false;
	return lastOutput;
}

void SSSampleAndHold::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	lastTrigger = false;
  lastOutput = 0.0;
  lastRightSample = 0.0;
}
