// SSFilter.cp- basic FIR filter

#include "ss.h"
#include "ssfilter.h"

SSFilter::SSFilter(ModList * mList, short h, short v) : SSModule(MT_Filter, mList, h, v)
{
	InitExp(&a0Exp, "0.5");
	InitExp(&a1Exp, "0.5");
	InitExp(&a2Exp, "0");
	InitExp(&b1Exp, "0");
	InitExp(&b2Exp, "0");

	flags = 0;
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

void	SSFilter::Copy(SSModule *ss)
{
	SSFilter *sa = (SSFilter *) ss;
	SSModule::Copy(ss);
	CopyExp(&sa->a0Exp, &a0Exp);
	CopyExp(&sa->a1Exp, &a1Exp);
	CopyExp(&sa->a2Exp, &a2Exp);
	CopyExp(&sa->b1Exp, &b1Exp);
	CopyExp(&sa->b2Exp, &b2Exp);
	this->flags = sa->flags;
}

/*
void SSFilter::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"FLT %d\r",flags);
  WriteFileLine(ar,"FLTA0 %s\r",a0Exp.exp);
  WriteFileLine(ar,"FLTA1 %s\r",a1Exp.exp);
  WriteFileLine(ar,"FLTA2 %s\r",a2Exp.exp);
  WriteFileLine(ar,"FLTB1 %s\r",b1Exp.exp);
  WriteFileLine(ar,"FLTB2 %s\r",b2Exp.exp);
}
*/

void SSFilter::Load(FILE* ar) 
{
	char				*p,tbuf[256];
	p = parList->GetNextInputLine(ar,"FLT",tbuf);
	sscanf(p, "FLT %d",&flags);
	LoadExp(ar,"FLTA0",&a0Exp);
	LoadExp(ar,"FLTA1",&a1Exp);
	LoadExp(ar,"FLTA2",&a2Exp);
	LoadExp(ar,"FLTB1",&b1Exp);
	LoadExp(ar,"FLTB2",&b2Exp);
}

ssfloat SSFilter::GenerateOutput(SSModule *callingMod)
{
	ssfloat	x,y,a0,a1,a2,b1,b2;

	a0 = SolveExp(&a0Exp,callingMod);
	a1 = SolveExp(&a1Exp,callingMod);
	a2 = SolveExp(&a2Exp,callingMod);
	b1 = SolveExp(&b1Exp,callingMod);
	b2 = SolveExp(&b2Exp,callingMod);

	x = MixInputs(0, callingMod);

	y = a0*x + a1*oldX[0] + a2*oldX[1] +
				b1*oldY[0] + b2*oldY[1];

	oldX[1] = oldX[0];
	oldX[0] = x;
	
	oldY[1] = oldY[0];
	oldY[0] = y;
  lastRightSample = y;
	return y;
}

void SSFilter::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	ResetExp(&a0Exp, callingMod);
	ResetExp(&a1Exp, callingMod);
	ResetExp(&a2Exp, callingMod);
	ResetExp(&b1Exp, callingMod);
	ResetExp(&b2Exp, callingMod);
	oldX[0] = 0;
	oldX[1] = 0;
	oldY[0] = 0;
	oldY[1] = 0;
}


