// ssmaraca.cp
//
// Maraca output is normalized to 0-1, to make it easier to
// use for amplitude control etc.

#include "ss.h"
#include "ssmaraca.h"
#include "patchowner.h"

#include <math.h>

// void	SSMaraca::Initialize(short itsType, short h, short v)
SSMaraca::SSMaraca(ModList * mList, short h, short v) : SSModule(MT_Maraca, mList, h, v)
{
	InitExp(&resfreqExp,"3200");
	InitExp(&respoleExp,"0.96");
	InitExp(&probExp,"1/16");
	InitExp(&sysdecayExp,"0.999");
	InitExp(&snddecayExp,"0.95");
	DescribeLink(0, "INVALID", "???",0xFFFF,0xFFFF,0xFFFF);


}

void SSMaraca::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);

	resfreq = ResetExp(&resfreqExp, callingMod);
	respole = ResetExp(&respoleExp, callingMod);
	prob = ResetExp(&probExp, callingMod);
	sysdecay = ResetExp(&sysdecayExp, callingMod);
	snddecay = ResetExp(&snddecayExp, callingMod);

	numBeans = 64;
	gain = log((double) numBeans) / log(4.0) * 40.0 / numBeans;
	output[0] = output[1] = 0.0;
	input = 0.0;
	shakeEnergy = 0;
	temp = 0.0;
	sndLevel = 0;
	coeffs[0] = -respole * 2.0 * cos(resfreq * 2.0 * pi * parList->itsOwner->timeInc);
	coeffs[1] = respole * respole;
	i = 0;
}

ssfloat	SSMaraca::GenerateOutput(SSModule *callingMod)
{
	if (temp < 2*pi) {
		// shake over 50 ms and add energy
		temp += (2*pi*parList->itsOwner->timeInc) / 0.05;
		shakeEnergy += (1.0 - cos(temp));
	}

	// Compute exponential system decay
	shakeEnergy *= sysdecay;
	
	if (DoubleRandom() < prob)
		sndLevel += gain*shakeEnergy;

	input = sndLevel * (DoubleRandom()*2 - 1);
	sndLevel *= snddecay;

	// do gourd resonance filter calc
	// Note: this is a good bandpass filter!
	input -= output[0]*coeffs[0];
	input -= output[1]*coeffs[1];
	output[1] = output[0];
	output[0] = input;

	++i;

	ssfloat retVal = (output[0] - output[1])/32000.0;
  lastRightSample = retVal;
  return retVal;
}


void SSMaraca::CleanUp()
{
}




void SSMaraca::Copy(SSModule *ss)
{
	SSMaraca	*osc = (SSMaraca *) ss;
	SSModule::Copy(ss);
	CopyExp(&osc->resfreqExp, &resfreqExp);
	CopyExp(&osc->respoleExp, &respoleExp);
	CopyExp(&osc->probExp, &probExp);
	CopyExp(&osc->sysdecayExp, &sysdecayExp);
	CopyExp(&osc->snddecayExp, &snddecayExp);
}

/*
void SSMaraca::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"MARI\r");
  WriteFileLine(ar,"MARRF %s\r",resfreqExp.exp);
  WriteFileLine(ar,"MARRP %s\r",respoleExp.exp);
  WriteFileLine(ar,"MARP %s\r",probExp.exp);
  WriteFileLine(ar,"MARSysD %s\r",sysdecayExp.exp);
  WriteFileLine(ar,"MARSndD %s\r",snddecayExp.exp);
}
*/

void SSMaraca::Load(FILE* ar) 
{
	char				*p,tbuf[256];
	int					var=0;

	p = parList->GetNextInputLine(ar,"MARI",tbuf);
	LoadExp(ar,"MARRF",&resfreqExp);
	LoadExp(ar,"MARRP",&respoleExp);
	LoadExp(ar,"MARP",&probExp);
	LoadExp(ar,"MARSysD",&sysdecayExp);
	LoadExp(ar,"MARSndD",&snddecayExp);
}
