// SSFolder.cp

#include "ss.h"
#include "ssfolder.h"
#include "ssoutput.h"
#include "patchowner.h"

SSFolder::SSFolder(ModList * mList, short h, short v) : SSModule(MT_Folder, mList, h, v)
{
	instFileSpec.name[0] = 0;
//	instFileSpec.vRefNum = 0;
//	instFileSpec.parID = 0;
	instr = NULL;
	DescribeLink(0, "Folder Input", "f0",	0x0000,0x0000,0xffff);
	DescribeLink(1, "Folder Input #1", "f1",	0x1111,0x1111,0xeeee);
	DescribeLink(2, "Folder Input #2", "f2",	0x2222,0x2222,0xdddd);
	DescribeLink(3, "Folder Input #3", "f3",	0x3333,0x3333,0xcccc);
	DescribeLink(4, "Folder Input #4", "f4",	0x4444,0x4444,0xbbbb);
	DescribeLink(5, "Folder Input #5", "f5",	0x5555,0x5555,0xaaaa);
	DescribeLink(6, "Folder Input #6", "f6",	0x6666,0x6666,0x9999);
	DescribeLink(7, "Folder Input #7", "f7",	0x7777,0x7777,0x8888);
	DescribeLink(8, "Folder Input #8", "f8",	0x8888,0x8888,0x7777);
	DescribeLink(9, "Folder Input #9", "f9",	0x9999,0x9999,0x6666);
	DescribeLink(10, "Folder Input #10", "f10",	0xaaaa,0xaaaa,0x5555);
	DescribeLink(11, "Folder Input #11", "f11",	0xbbbb,0xbbbb,0x4444);
	DescribeLink(12, "Folder Input #12", "f12",	0xcccc,0xcccc,0x3333);
	DescribeLink(13, "Folder Input #13", "f13",	0xdddd,0xdddd,0x2222);
	DescribeLink(14, "Folder Input #14", "f14",	0xeeee,0xeeee,0x1111);
	DescribeLink(15, "Folder Input #15", "f15",	0xffff,0xffff,0x0000);
}

void SSFolder::Reset(SSModule *callingMod)
{
	OSErr	oe;
	SSModule::Reset(callingMod);
	instr = new ModList(parList->itsOwner);
	AddLogIndent(2);
	oe = instr->OpenSpec(&instFileSpec);
	if (oe != noErr)
	{
		ErrorMessage("Can't find Instrument: %s", instFileSpec.name);
		parList->itsOwner->AbortSynthesis();
	}
	else if (instr->nbrModules == 0)
	{
		ErrorMessage("Null Instrument");
		parList->itsOwner->AbortSynthesis();
	}
	instr->ResetInstruments(this);
	AddLogIndent(-2);
}

ssfloat SSFolder::GenerateOutput(SSModule *callingMod)
{
  ssfloat retVal;
	this->callingMod = callingMod;
	if (instr) {
		if (instr->mods[0]) {
			retVal = ((SSModule *) instr->mods[0])->GenerateOutputTime(this,parList->pTime);
      lastRightSample = ((SSModule *) instr->mods[0])->getRightSample();
    }
		else {
			// !!! Abort Synthesis
			parList->itsOwner->AbortSynthesis();
			ErrorMessage("Null Instrument");
			retVal = lastRightSample = 0.0;
		}
	}
	else
		  retVal = lastRightSample = 0.0;
  return retVal;
}

Boolean SSFolder::GetFolderSig(int n, ssfloat *retVal)
{
	if (CountInputs(n) > 0) {
		*retVal = MixInputs(n, callingMod);
		return true;
	}
	else
		return false;
}


void SSFolder::CleanUp()
{
	if (instr) {
		AddLogIndent(2);
		delete instr;
		AddLogIndent(-2);
		instr = NULL;
	}
	SSModule::CleanUp();
}



/*
void SSFolder::Save(FILE *ar)
{
	SSModule::Save(ar);

	WriteFileLine(ar,"CSCO %s 0 0\r",
					instFileSpec.name);

}

Boolean SSFolder::IsInFolder()
{
  return true;
}


*/
void SSFolder::Load(FILE* ar)
{
	char				*p,tbuf[256];
	int					vRefNum;
	long				parID;
	char				tempS[64];

	p = parList->GetNextInputLine(ar,"CSCO",tbuf);
  // sscanf(p, "CSCO %s %d %ld",tempS,&vRefNum,&parID);
  sscanf(p, "CSCO %s",tempS);
//	instFileSpec.vRefNum = vRefNum;
//	instFileSpec.parID = parID;
	strcpy(instFileSpec.name,tempS);
}

/*
char* SSFolder::GetLabel()
{
	return instFileSpec.name;
}
*/

void SSFolder::Copy(SSModule *ss)
{
	SSFolder *sf = (SSFolder *) ss;
	SSModule::Copy(ss);
	instFileSpec = sf->instFileSpec;
}
