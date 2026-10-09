// SSFTable.cp


#include "ss.h"
#include "ssftable.h"
#include "patchowner.h"

#define MaxTables	32
SSFTable	*ftable[32];

ssfloat GetFTableEntry(SSModule *callingMod, int tNbr, ssfloat pTime, int flags)
{
	if (ftable[tNbr] == NULL)
		return 0.0;

	if (flags & FW_Pin) {
		if (pTime < 0)
			pTime = 0;
		else if (pTime > 1)
			pTime = 1;
	}
	else if (flags & FW_Wrap) {
		if (pTime < 0)
			pTime = -pTime;
		pTime -= (int) pTime;
	}
	else {
		if (pTime > 1.0 || pTime < 0.0 || ftable[tNbr] == NULL)
			return 0.0;
	}
	if (flags & FW_Interp)
		return ftable[tNbr]->RetrieveTableValueI(pTime);
	else
		return ftable[tNbr]->RetrieveTableValue(pTime);
}

SSFTable::SSFTable(ModList * mList, short h, short v) : SSModule(MT_FTable, mList, h, v)
{
	InitExp(&tabExp, "sin(t*2*pi)");
	tabNbr = 0;
	tabSize = 100;
	table = NULL;
	DescribeLink(0, "Default Signal", "sig",	0x0000,0x0000,0xffff);
	DescribeLink(1, "Alt Signal #1", "sig1",	0x1111,0x1111,0xeeee);
	DescribeLink(2, "Alt Signal #2", "sig2",	0x2222,0x2222,0xdddd);
	DescribeLink(3, "Alt Signal #3", "sig3",	0x3333,0x3333,0xcccc);
}

void	SSFTable::Copy(SSModule *ss)
{
	SSFTable *sa = (SSFTable *) ss;
	SSModule::Copy(ss);
	CopyExp(&sa->tabExp, &tabExp);
	this->tabSize = sa->tabSize;
	this->tabNbr = sa->tabNbr;
}

/*
void SSFTable::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"FTAB %d %d\r",tabNbr,tabSize);
	WriteFileLine(ar,"FTABE %s\r",tabExp.exp);
}
*/

void SSFTable::Load(FILE* ar) 
{
	char	*p,tbuf[512];
	p = parList->GetNextInputLine(ar,"FTAB",tbuf);
	sscanf(p, "FTAB %d %d", &tabNbr, &tabSize);
	LoadExp(ar,"FTABE",&tabExp);
}

ssfloat SSFTable::GenerateOutput(SSModule *callingMod)
{
	return RetrieveTableValue(parList->pTime);
}

ssfloat SSFTable::GenerateOutputTime(SSModule *callingMod, ssfloat pTime)
{
  ssfloat retVal = RetrieveTableValue(pTime);
  lastRightSample = retVal;
  return retVal;
}

ssfloat SSFTable::RetrieveTableValue(ssfloat pTime)
{
	long	index;
	index = (long) (pTime * (tabSize-1));
	if (index < 0)
		index = 0;
	else if (index >= tabSize)
		index = tabSize - 1;
	return table[index];
}

ssfloat SSFTable::RetrieveTableValueI(ssfloat pTime)
{
	ssfloat	findex,frac;
	long	index,index2;
	findex = pTime * (tabSize-1);
	index = (long) findex;
	index2 = index+1;
	if (index >= tabSize-1)
		return table[tabSize-1];
	frac = findex - (long) findex;
	// Interpolate values
	return (1.0-frac)*table[index] + frac*table[index2];
}





void SSFTable::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	table = (ssfloat *) MyNewPtrClear(tabSize * sizeof(ssfloat));
	if (table == NULL) {
		ErrorMessage("Null Function Table (out of mem?)");
		parList->itsOwner->AbortSynthesis();
		return;
	}
	ftable[tabNbr] = this;

}

void SSFTable::FillTable(SSModule *callingMod)
{
	long	i;
	if (table) {
		parList->PushTime(0.0);
		ResetExp(&tabExp,callingMod);
		for (i = 0; i < tabSize; ++i) {
			parList->pTime = i/(double)tabSize;
			table[i] = SolveExp(&tabExp, callingMod);
		}
		parList->PopTime();
	}
}

void SSFTable::CleanUp()
{
	if (table) {
		MyDisposePtr((Ptr) table);
		table = NULL;
	}
	ftable[tabNbr] = NULL;
	SSModule::CleanUp();
}
