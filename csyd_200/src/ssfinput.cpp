// Folder Input

// SSFInput.cp

#include "ss.h"
#include "ssfinput.h"

SSFInput::SSFInput(ModList * mList, short h, short v) : SSModule(MT_FInput, mList, h, v)
{
	fNbr = 0;
	ClearExp(&defExp);
	strcpy(desc,"input value");
	DescribeLink(0, "INVALID", "???",0xFFFF,0xFFFF,0xFFFF);
}

void	SSFInput::Copy(SSModule *ss)
{
	SSFInput *sa = (SSFInput *) ss;
	SSModule::Copy(ss);
	CopyExp(&sa->defExp, &defExp);
	fNbr = sa->fNbr;
	strcpy(desc,sa->desc);
}

/*
void SSFInput::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"FIDEF %s\r",defExp.exp);
	WriteFileLine(ar,"FINN %d\r",fNbr);
	WriteFileLine(ar,"FIND %s\r",desc);
}
*/

void SSFInput::Load(FILE* ar)
{
	char	*p,tbuf[256];
	LoadExp(ar,"FIDEF", &defExp);

	p = parList->GetNextInputLine(ar,"FINN",tbuf);
	sscanf(p,"FINN %d",&fNbr);

	p = parList->GetNextInputLine(ar,"FIND",tbuf);
	sscanf(p,"FIND %s",desc);
}

ssfloat SSFInput::GenerateOutput(SSModule *callingMod)
{
	ssfloat	v;
	this->callingMod = callingMod;
	if (!parList->mods[0]->GetFolderSig(fNbr, &v))
		v = SolveExp(&defExp,callingMod);
  lastRightSample = v;
	return v;
}

void SSFInput::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	ResetExp(&defExp,callingMod);
}

#ifdef UI_FEATURES
char* SSFInput::GetLabel()
{
	return desc;
}

Boolean SSFInput::GetOverlay(char *overlay, int* x, int* y)
{
	*x = cellBounds.left+11;
	*y = cellBounds.top+20;
	sprintf(overlay,"%d",fNbr);
	return true;
}
#endif
