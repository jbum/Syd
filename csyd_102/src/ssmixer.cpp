// ssmixer.cp

// Mixes audio signals, and attenuates properly to prevent clipping

#include "ss.h"
#include "ssmixer.h"

SSMixer::SSMixer(ModList * mList, short h, short v) : SSModule(MT_Mixer, mList, h, v)
{
	DescribeLink(0, "Signal to Mix", "sig",0,0,0xFFFF);
}

ssfloat SSMixer::GenerateOutput(SSModule *callingMod)
{
	ssfloat retVal =  MixInputsAtten(-1, callingMod);
  lastRightSample = lastRightInput;
  return retVal;
}

// Attenuated Mix (used by Mixer module)
ssfloat SSMixer::MixInputsAtten(int type, SSModule *callingMod)
{
	this->callingMod = callingMod;

	if (nbrInputs == 0)
		return 0.0;

	else if (nbrInputs == 1) {
	  ssfloat retVal;
		if (type == -1 || inputs[0].inputType == type) {
			retVal = inputs[0].link->GenerateOutput(this);
  	  lastRightInput = inputs[0].link->getRightSample();
    }
		else {
		  lastRightInput = 0;
			retVal = 0;
    }
    return retVal;
	}
	else {
		int		n,i;
		ssfloat	v, vR;

		v = 0.0;
		vR = 0.0;
		for (i = n = 0; i < nbrInputs; ++i) {
			if (type == -1 || inputs[i].inputType == type) {
				v += inputs[i].link->GenerateOutput(this);
				vR += inputs[i].link->getRightSample();
				n++;
			}
		}
		if (n > 1) {
			v /= n;
			vR /= n;
    }
    lastRightInput = vR;
		return	v;
	}
}

