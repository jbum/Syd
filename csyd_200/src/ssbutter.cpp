// SSButter.cp - algorithm copied from CSound
//

#include "ss.h"
#include "ssbutter.h"
#include "patchowner.h"
#include <math.h>

/*
Lo Pass
Hi Pass
Band Pass
Band Reject
*/

SSButter::SSButter(ModList * mList, short h, short v) : SSModule(MT_Butter, mList, h, v)
{
	InitExp(&freqExp, "1200");
	InitExp(&bwExp, "1200*0.1");
	filterType = BT_BandPass;
	DescribeLink(0, "Signal to Filter", "sig",0,0,0xFFFF);
	DescribeLink(1, "Control Signal", "ctl",	0xFFFF,0x1111,0xeeee);
	DescribeLink(2, "Alt Control Sig #1", "ctl1",	0xFFFF,0x2222,0xdddd);
	DescribeLink(3, "Alt Control Sig #2", "ctl2",	0xFFFF,0x3333,0xcccc);
	DescribeLink(4, "Alt Control Sig #3", "ctl3",	0xFFFF,0x4444,0xbbbb);
	DescribeLink(5, "Alt Control Sig #4", "ctl4",	0xFFFF,0x5555,0xaaaa);
	DescribeLink(6, "Alt Control Sig #5", "ctl5",	0xFFFF,0x6666,0x9999);
	DescribeLink(7, "Alt Control Sig #6", "ctl6",	0xFFFF,0x7777,0x8888);
	DescribeLink(8, "Alt Control Sig #7", "ctl7",	0xFFFF,0x8888,0x7777);
	DescribeLink(9, "Alt Control Sig #8", "ctl8",	0xFFFF,0x9999,0x6666);
}

void	SSButter::Copy(SSModule *ss)
{
	SSButter *sa = (SSButter *) ss;
	SSModule::Copy(ss);
	CopyExp(&sa->freqExp, &freqExp);
	CopyExp(&sa->bwExp, &bwExp);
	this->filterType = sa->filterType;
}
/*
void SSButter::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"BUTTER %d\r",filterType);
	WriteFileLine(ar,"BUTTERF %s\r",freqExp.exp);
	WriteFileLine(ar,"BUTTERB %s\r",bwExp.exp);
}
*/
void SSButter::Load(FILE* ar) 
{
	char	*p,tbuf[512];
	p = parList->GetNextInputLine(ar,"BUTTER",tbuf);
	sscanf(p, "BUTTER %d",&filterType);
	LoadExp(ar,"BUTTERF",&freqExp);
	LoadExp(ar,"BUTTERB",&bwExp);
}

ssfloat SSButter::GenerateOutput(SSModule *callingMod)
{
	ssfloat	v,t,y,freq,bw;

	freq = SolveExp(&freqExp,callingMod);
	bw = SolveExp(&bwExp,callingMod);

	v = MixInputs(0, callingMod);

	if (freq != oldFreq || bw != oldBW) {
		ssfloat	c,d;
		switch (filterType) {
		case BT_LoPass:
			c = 1.0 / tan(pidsr * freq);
			a0 = 1.0 / (1.0 + ROOT2 * c + c * c);
			a1 = a0 + a0; 
			a2 = a0;
			b1 = 2.0 * (1.0 - c*c) * a0;
			b2 = (1.0 - ROOT2 * c + c * c) * a0;
			break;
		case BT_HiPass:
			c = tan(pidsr * freq);
			a0 = 1.0 / (1.0 + ROOT2 * c + c * c);
			a1 = -2.0 * a0; 
			a2 = a0;
			b1 = 2.0 * (c*c - 1.0) * a0;
			b2 = (1.0 - ROOT2 * c + c * c) * a0;
			break;
		case BT_BandPass:
			c = 1.0 / tan(pidsr * bw);
			d = 2.0 * cos(2.0 * pidsr * freq);
			a0 = 1.0 / (1.0 + c);
			a1 = 0.0;
			a2 = -a0;
			b1 = - c * d * a0;
			b2 = (c - 1.0) * a0;
			break;
		case BT_BandReject:
			c = tan(pidsr * bw);
			d = 2.0 * cos(2.0 * pidsr * freq);
			a0 = 1.0 / (1.0 + c);
			a1 = - d * a0;
			a2 = a0;
			b1 = a1;
			b2 = (1.0 - c) * a0;
			break;
		}
		oldBW = bw;
		oldFreq = freq;
	}
	if (bw == 0.0) {
		t = y = 0;
	}
	else {
		t = v - b1*y1 - b2*y2;			// First Section (IIR)
		y = t * a0 + a1*y1 + a2*y2;	// Second Section (FIR)
	}
	y2 = y1;
	y1 = t;
  lastRightSample = y;
	return y;
}

void SSButter::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	ResetExp(&freqExp, callingMod);
	ResetExp(&bwExp, callingMod);
	oldFreq = oldBW = -1.0;
	a0 = a1 = a2 = b1 = b2 = y1 = y2 = 0;
	pidsr = pi * parList->itsOwner->timeInc;	// pi / samplerate
}



