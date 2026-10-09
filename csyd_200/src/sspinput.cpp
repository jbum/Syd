// Folder Input

// sspinput.cp

#include "ss.h"
#include "sspinput.h"
// #include "MainWin.h"

SSPInput::SSPInput(ModList * mList, short h, short v) : SSModule(MT_PInput, mList, h, v)
{
	pNbr = 1;
	ClearExp(&defExp);
	strcpy(desc,"input value");
	DescribeLink(0, "INVALID", "???",0xFFFF,0xFFFF,0xFFFF);
}

void	SSPInput::Copy(SSModule *ss)
{
	SSPInput *sa = (SSPInput *) ss;
	SSModule::Copy(ss);
	CopyExp(&sa->defExp, &defExp);
	pNbr = sa->pNbr;
	strcpy(desc,sa->desc);
}


/*
void SSPInput::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"PIDEF %s\r",defExp.exp);
	WriteFileLine(ar,"PINN %d\r",pNbr);
	WriteFileLine(ar,"PIND %s\r",desc);
}
*/

void SSPInput::Load(FILE* ar)
{
	char	*p, tbuf[256];
	LoadExp(ar,"PIDEF", &defExp);

	p = parList->GetNextInputLine(ar,"PINN", tbuf);
	sscanf(p,"PINN %d",&pNbr);

	p = parList->GetNextInputLine(ar,"PIND", tbuf);
	sscanf(p,"PIND %s",desc);
}

ssfloat SSPInput::GenerateOutput(SSModule *callingMod)
{
	return pValue;
}

void SSPInput::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);

	this->callingMod = callingMod;

	ResetExp(&defExp,callingMod);

	if (callingMod->IsInScore()) {
		pValue = callingMod->GetInstParameter(pNbr);
	}
	else {
		pValue = SolveExp(&defExp,callingMod);
	}
  lastRightSample = pValue;
}

#ifdef UI_FEATURES

char*	SSPInput::GetLabel()
{
	if (desc[0])
		return desc;
	else
		return NULL;
}

Boolean SSPInput::GetOverlay(char *overlay, int *x, int *y)
{
	*x = cellBounds.left+11;
	*y = cellBounds.top+20;
	sprintf(overlay,"%d",pNbr);
	return true;
}
#endif

