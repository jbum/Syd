// SSExpression.cp

#include "ss.h"
#include "ssexpression.h"

SSExpression::SSExpression(ModList * mList, short h, short v) : SSModule(MT_Expression, mList, h, v)
{
	ClearExp(&exp);

	DescribeLink(0, "Default Signal", "sig",	0x0000,0x0000,0xffff);
	DescribeLink(1, "Alt Signal #1", "sig1",	0x1111,0x1111,0xeeee);
	DescribeLink(2, "Alt Signal #2", "sig2",	0x2222,0x2222,0xdddd);
	DescribeLink(3, "Alt Signal #3", "sig3",	0x3333,0x3333,0xcccc);
	DescribeLink(4, "Alt Signal #4", "sig4",	0x4444,0x4444,0xbbbb);
	DescribeLink(5, "Alt Signal #5", "sig5",	0x5555,0x5555,0xaaaa);
	DescribeLink(6, "Alt Signal #6", "sig6",	0x6666,0x6666,0x9999);
	DescribeLink(7, "Alt Signal #7", "sig7",	0x7777,0x7777,0x8888);
	DescribeLink(8, "Alt Signal #8", "sig8",	0x8888,0x8888,0x7777);
	DescribeLink(9, "Alt Signal #9", "sig9",	0x9999,0x9999,0x6666);
	DescribeLink(10, "Alt Signal #10", "sig10",	0xaaaa,0xaaaa,0x5555);
	DescribeLink(11, "Alt Signal #11", "sig11",	0xbbbb,0xbbbb,0x4444);
	DescribeLink(12, "Alt Signal #12", "sig12",	0xcccc,0xcccc,0x3333);
	DescribeLink(13, "Alt Signal #13", "sig13",	0xdddd,0xdddd,0x2222);
	DescribeLink(14, "Alt Signal #14", "sig14",	0xeeee,0xeeee,0x1111);
	DescribeLink(15, "Alt Signal #15", "sig15",	0xffff,0xffff,0x0000);
}

void	SSExpression::Copy(SSModule *ss)
{
	SSExpression *sa = (SSExpression *) ss;
	SSModule::Copy(ss);
	CopyExp(&sa->exp, &exp);
}

/*
void SSExpression::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"EXP %s\r",exp.exp);
}
*/

void SSExpression::Load(FILE* ar) 
{
	LoadExp(ar,"EXP", &exp);
}

ssfloat SSExpression::GenerateOutput(SSModule *callingMod)
{
	this->callingMod = callingMod;
  ssfloat retVal = SolveExp(&exp,callingMod);
  lastRightSample = retVal;
  return retVal;
}

void SSExpression::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	ResetExp(&exp,callingMod);
}

/*
char* SSExpression::GetLabel()
{
	return exp.exp;
}
*/


