// Folder Input

// SSGAssign.cp

#include "ss.h"
#include "ssgassign.h"
#include "patchowner.h"
// #include "MainWin.h"

SSGAssign::SSGAssign(ModList * mList, short h, short v) : SSModule(MT_GAssign, mList, h, v)
{
	InitExp(&gNbrExp,"0");
	InitExp(&valExp,"sig");
	strcpy(desc,"var");
	DescribeLink(0, "Default Signal", "sig",	0x0000,0x0000,0xffff);
	DescribeLink(1, "Alt Signal #1", "sig1",	0x1111,0x1111,0xeeee);
	DescribeLink(2, "Alt Signal #2", "sig2",	0x2222,0x2222,0xdddd);
	DescribeLink(3, "Alt Signal #3", "sig3",	0x3333,0x3333,0xcccc);
}

void	SSGAssign::Copy(SSModule *ss)
{
	SSGAssign *sa = (SSGAssign *) ss;
	SSModule::Copy(ss);
	CopyExp(&sa->valExp, &valExp);
	CopyExp(&sa->gNbrExp, &gNbrExp);
	strcpy(desc,sa->desc);
}

/*
void SSGAssign::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"GASSN %s\r",gNbrExp.exp);
	WriteFileLine(ar,"GASSV %s\r",valExp.exp);
	WriteFileLine(ar,"GASSD %s\r",desc);
}
*/

void SSGAssign::Load(FILE* ar)
{
	char	*p;
	char	temp[256];
	LoadExp(ar,"GASSN", &gNbrExp);
	LoadExp(ar,"GASSV", &valExp);
	p = parList->GetNextInputLine(ar,"GASSD",temp);
	sscanf(p,"GASSD %s",desc);
}

ssfloat SSGAssign::GenerateOutput(SSModule *callingMod)
{
	ssfloat	v,n;
	n = SolveExp(&gNbrExp, callingMod);
	v = SolveExp(&valExp, callingMod);
	parList->itsOwner->AssignGlobal((int) n, v);
  lastRightSample = 0.0;
	return 0.0;
}

void SSGAssign::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);

	this->callingMod = callingMod;

	ResetExp(&gNbrExp, callingMod);
	ResetExp(&valExp, callingMod);
}

#ifdef UI_FEATURES
Boolean SSGAssign::GetOverlay(char *overlay, int* x, int* y)
{
	*x = cellBounds.left+20;
	*y = cellBounds.top+21;
	strcpy(overlay,gNbrExp.exp);
	return true;
}

char* SSGAssign::GetLabel()
{
	return desc;
}
#endif
