// SSInverter.cp

#include "ss.h"
#include "ssinverter.h"
// #include "MainWin.h"

SSInverter::SSInverter(ModList * mList, short h, short v) : SSModule(MT_Inverter, mList, h, v)
{
	DescribeLink(0, "Signal to Invert", "sig",0,0,0xFFFF);
}


ssfloat SSInverter::GenerateOutput(SSModule *callingMod)
{
	ssfloat retVal = - MixInputs(-1, callingMod);
	lastRightSample = -lastRightInput;
	return retVal;
}
