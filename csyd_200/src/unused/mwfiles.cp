// MainWin File Handling

#include "SS.h"
#include "MainWin.h"
#include "ModList.h"

Boolean MainWindow::SaveAs(void)
{
	StandardFileReply reply;
	StringPtr			defName = "\pUntitled.syn";

	WindowPtr theWin = (WindowPtr) &this->theWin;

	if (fsSpec.name[0] == 0) {
		fsSpec.vRefNum = 0;
		fsSpec.parID = 0;
		BlockMove(defName,fsSpec.name,defName[0]+1);
	}
	StandardPutFile("\pSave To:",fsSpec.name,&reply);
	if (!reply.sfGood)
		return false;

	fsSpec = reply.sfFile;

	SetWTitle(theWin, fsSpec.name);

	return Save();
}

/*
void MainWindow::ExportAs(void)
{
	StandardFileReply reply;
	StringPtr			defName = "\pUntitled.aiff";

	if (esSpec.name[0] == 0) {
		esSpec.vRefNum = 0;
		esSpec.parID = 0;
		BlockMove(defName,esSpec.name,defName[0]+1);
	}
	StandardPutFile("\pExport To:",esSpec.name,&reply);
	if (!reply.sfGood)
		return;
	esSpec = reply.sfFile;
	ExportToFile(&esSpec);
}
*/

Boolean MainWindow::Save(void)
{
	short	fRefNum;
	OSErr	oe;
	if (fsSpec.name[0] == 0) {
		return SaveAs();
	}
	FSpDelete(&fsSpec);
	oe = FSpCreate(&fsSpec,'wAve','TEXT',0);
	if (oe != noErr)
		return true;	// Error check needed!!!
	oe = FSpOpenDF(&fsSpec,fsRdWrPerm,&fRefNum);
	if (oe != noErr)
		return true;	// Error check needed !!!

	mainInst->Save(fRefNum);

	FSClose(fRefNum);
	dirty = false;
	return true;
}

void MainWindow::Open(void)
{
	StandardFileReply	reply;
	WindowPtr theWin = (WindowPtr) &this->theWin;

	SFTypeList	typeList = {'TEXT'};
	
	if (!ConfirmClose())
		return;
	StandardGetFile(NULL,1,typeList,&reply);
	if (!reply.sfGood)
		return;

	// OpenSpec(&reply.sfFile);
	Open(&reply.sfFile);
}

void MainWindow::Open(FSSpec *spec)
{
	WindowPtr theWin = (WindowPtr) &this->theWin;
	fsSpec = *spec;
	mainInst->OpenSpec(&fsSpec);
	SetWTitle(theWin, fsSpec.name);
	FullRefresh();
}

/*
ModuleList *MainWindow::LoadInstrument(FSSpec *spec)
{
	ModuleList	*nList;
	nList = (ModuleListPtr) MyNewPtrClear(sizeof(ModuleList));
	if (nList == NULL)
		return NULL;
	nList->nbrModules = 0;
	OpenSpec(nList, spec);
	return nList;

}
*/


// !!! Modify for "Open Main Spec"
/*
void MainWindow::OpenSpecMMM(FSSpec *fsSpec)
{
	WindowPtr theWin = (WindowPtr) &this->theWin;


}
*/


void MainWindow::New(void)
{
	SSReset();
}

