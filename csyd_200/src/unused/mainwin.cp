// MainWin.cp - inherited from ObjWin...


#include <OSUtils.h>
#include <math.h>

#include "SS.h"
#include "MainWin.h"
#include "GraphWin.h"
#include "DeckSounds.h"
#include "SSMenus.h"
#include "ExpMgr.h"
#include "SSModule.h"
#include "SSStoreFile.h"
#include "SSStoreMem.h"
#include "ModList.h"

// Individual module types
MainWindow::MainWindow() : ObjectWindow(MainWindowID, false)
{
	WindowPtr	theWin;
	short		i;
	short		posx=64,posy=64;

	if (mw) {
		WindowPtr	curWin = (WindowPtr) &mw->theWin;
		Point		pos;
		SetPort(curWin);
		pos = topLeft(curWin->portRect);
		LocalToGlobal(&pos);
		posx = pos.h + 32;
		posy = pos.v + 32;
	}
	mw = this;

	expMgr = new ExpMgr();
	storeFile = new SSStoreFile();
	storeMem = new SSStoreMem();
	storageMod = storeMem;
	storeToFile = false;

	dirty = false;
	showMemory = false;
	listenFlag = false;
	nbrSamples = 0;
	mainInst = NULL;
	fsSpec.name[0] = 0;
	esSpec.name[0] = 0;
	// readPtr = NULL;
	// instParams = NULL;
	// defaultInstParams.nbrOptParams = 2;
	// defaultInstParams.p1 = 0;
	// defaultInstParams.p2 = 0.0;
	// defaultInstParams.p3 = 2.0;
	// defaultInstParams.op[0] = 0.5;
	// defaultInstParams.op[1] = 440.0;

	for (i = 0; i < CT_NbrCursors; ++i)
		cursors[i] = GetCursor(128+i);

	theWin = (WindowPtr) &this->theWin;

	SetWTitle(theWin, "\pUntitled");
	SizeWindow(theWin,DefWindowWidth,DefWindowHeight,0);	
	MoveWindow(theWin, posx, posy, 0);

	SetPort(theWin);

	TextFont(geneva);
	TextSize(9);

	InitGraphics();

	ShowWindow(theWin);
	
	forceFullRefresh = true;
	// InitSS();
	InitSSDraw();
	InitSynth();
	SetActiveButtons();
}

MainWindow::~MainWindow()
{
	DisposeAllModules();

	if (expMgr != NULL)
		delete expMgr;

	if (storeMem) {
		delete storeMem;
		storeMem = NULL;
	}

	if (storeFile) {
		delete storeFile;
		storeFile = NULL;
	}

	if (offWorld)
		DisposeGWorld(offWorld);

	if (mainInst) {
		delete mainInst;
		// DisposePtr((Ptr) mainInst);
		mainInst = NULL;
	}
	mw = NULL;
}

void MainWindow::Activate(Boolean activeFlag)
{
	inherited::Activate(activeFlag);
	if (activeFlag)
		mw = this;
	else {
		SetCursor(&qd.arrow);
	}
}

void MainWindow::InitGraphics()
{
	OSErr		oe;
	WindowPtr	theWin = (WindowPtr) &this->theWin;
	CGrafPtr	curPort;
	GDHandle	curDevice;

	SetPort(theWin);

	TextFont(geneva);
	TextSize(9);

	if ((oe = NewGWorld(&offWorld,0,&theWin->portRect,NULL,NULL,0)) != noErr) {
		ErrorExit("Out of Mem");
		return;
	}
	offMap = GetGWorldPixMap(offWorld);
	LockPixels(offMap);

	GetGWorld(&curPort,&curDevice);
	SetGWorld(offWorld,NULL);
	TextFont(geneva);
	TextSize(9);
	EraseRect(&theWin->portRect);
	SetGWorld(curPort,curDevice);
}

#define SaveChangesALRT	10000

Boolean MainWindow::ConfirmClose()
{
	StringPtr	fileName = "\pUntitled";
	if (dirty) {
		SetCursor(&qd.arrow);
		if (fsSpec.name[0] != 0)
			fileName = fsSpec.name;
		ParamText(fileName, "\p","\p","\p");
		switch (Alert(SaveChangesALRT, NULL)) {
		case 1:		
			return Save();
		case 2:
			return false;
		case 3:
			// Discard
			return true;
		}
	}
	return true;
}

void MainWindow::SSReset()
{
	GrafPtr	theWin = (WindowPtr) &this->theWin;
	// !!! Dispose of current modules
	fsSpec.name[0] = 0;

	storageMod = storeMem;
	storeToFile = false;

	InitSynth();
	SetWTitle(theWin,"\pUntitled");
	FullRefresh();
}

void MainWindow::RefreshRect(Rect *rr)
{
	Rect		r;
	GrafPtr	theWin = (WindowPtr) &this->theWin,savePort;

	SectRect(rr,&theWin->portRect,&r);
	if (!EmptyRect(&r)) {
		GetPort(&savePort);
		SetPort(theWin);
		CopyBits((BitMap *) *offMap, &theWin->portBits, 
				&r,&r,srcCopy,NULL);
		SetPort(savePort);
	}
}

void MainWindow::Draw(Rect	*rr)
{
	GrafPtr	theWin = (WindowPtr) &this->theWin;
	SetPort(theWin);
	if (forceFullRefresh) {
		PartialRefresh(rr);
		forceFullRefresh = false;
	}
	else {
		RefreshRect(rr);
	}
}


void MainWindow::FullRefresh()
{
	GrafPtr	theWin = (WindowPtr) &this->theWin;
	PartialRefresh(&theWin->portRect);
}

void MainWindow::PartialRefresh(Rect *rr)
{
	GrafPtr	theWin = (WindowPtr) &this->theWin,savePort;
	CGrafPtr	curPort;
	GDHandle	curDevice;

	GetPort(&savePort);
	SetPort(theWin);

	GetGWorld(&curPort,&curDevice);
	SetGWorld(offWorld,NULL);
	Paint(rr);
	SetGWorld(curPort,curDevice);

	RefreshRect(rr);
	SetPort(savePort);

}



void MainWindow::Paint(Rect *rr)
{
	Rect		r;
	RGBColor	fc;
	GrafPtr	theWin = (WindowPtr) &this->theWin;

	RgnHandle	rgn;

	rgn = NewRgn();
	GetClip(rgn);
	ClipRect(rr);

	r = theWin->portRect;
	fc.red = fc.blue = fc.green = 0x8888;
	RGBForeColor(&fc);
	PaintRect(&r);

	r.top = gpRect.bottom;
	InsetRect(&r,4,4);
	DrawTableFrame(&r, true);
	InsetRect(&r,1,1);
	DrawTableFrame(&r, false);
	
	fc.red = fc.blue = fc.green = 0x0000;
	RGBForeColor(&fc);

	DrawGadgetPanel(rr);
	DrawWindowButtons(rr);
	RefreshModules(rr);

	SetClip(rgn);
	DisposeRgn(rgn);
}

long	gLastFreeMem,gLastDelta;
extern int gAllocCtr;

void MainWindow::Idle(EventRecord *theEvent)
{
	Point	p;
	GrafPtr	savePort;
	GrafPtr	theWin = (WindowPtr) &this->theWin;
	SSModule	*smod;
	Boolean	hasCmdKey = (theEvent->modifiers & cmdKey) > 0;

	GetPort(&savePort);
	SetPort(theWin);

	if (windowState == WS_Synthesize) {
		SynthIdle();
		return;
	}
	else
		storageMod->PlayIdle();

	if (!active)
		return;

	GetMouse(&p);
	

	if (PtInRect(p,&theWin->portRect) && !gSuspended) {

		// HelpIdle(p);

		if (PtInRect(p,&gpRect)) {
			GadgetIdle(p);
		}
		else {
			smod = mainInst->GetPickedModule(p);
			if (smod) {
				Rect	outputArea;
				outputArea = smod->cellBounds;
				outputArea.right += 4;
				outputArea.left = outputArea.right-7;
				if (PtInRect(p,&outputArea)) {
					SetCursor(*cursors[CT_PatchCord]);
				}
				else {
					SetCursor(*cursors[CT_OpenHand]);
				}
				SetHelp(mainInst->GetModuleDesc(smod->moduleType));
			}
			else {
				int			n,i;
				Point		sp,dp;
				LinkDescPtr	lp;
				SSModule	*srcMod, *dstMod;
				static char helpStr[64];

				for (n = 0; n < mainInst->nbrModules; ++n) {
					srcMod = mainInst->mods[n];
					for (i = 0; i < srcMod->nbrInputs; ++i) {
						dstMod = srcMod->inputs[i].link;
						dstMod->GetOutputDock(&dp,srcMod);
						srcMod->GetInputDock(&sp,dstMod);
						if (PtInLine(p,sp,dp) && 
							(lp = srcMod->GetLinkDesc(srcMod->inputs[i].inputType)) != NULL) {
							sprintf(helpStr,"%s (%s)",lp->desc,lp->varName);
							CtoPstr(helpStr);
							if (hasCmdKey)
								SetCursor(*cursors[CT_CutPatch]);
							else
								SetCursor(*cursors[CT_PatchCord]);
							SetHelp((StringPtr) helpStr);
							return;
						}
					}
				}
				SetHelp(NULL);
				SetCursor(*cursors[CT_PointyHand]);
			}
		}
	}
	else
		SetCursor(&qd.arrow);
}

char *MainWindow::StatString()
{
	static char	tempS[64];
	long	t;

	t = FreeMem();
	if (t != gLastFreeMem) {
		gLastDelta = t - gLastFreeMem;
		gLastFreeMem = t;
	}

	sprintf(tempS,"%ld (%ld) [%d/%d]",gLastFreeMem,gLastDelta,gAllocCtr,gModAlloc);
	return tempS;
}

void MainWindow::HandleClick(Point where, EventRecord *theEvent)
{
	Point		p;
	GrafPtr		theWin = (WindowPtr) &this->theWin;
	SSModule	*smod;
	Boolean		hasCmdKey;
	Boolean		repatchFlag;
	
	p = where;
	SetPort(theWin);
	GlobalToLocal(&p);

	hasCmdKey = (theEvent->modifiers & cmdKey) > 0;

	if (HandleButtonClick(p))
		return;

	if (windowState == WS_Synthesize)
		return;

	if (HandleGadgetPanelClick(p))
		return;
	smod = mainInst->GetPickedModule(p);
	if (smod) {
		ProcessModuleClick(smod,p,theEvent);
		return;
	}
	else
		DeselectAll();

	// Check for clicks on links...

	// Cur GrafPort = theWin at this point

	Point		sp,dp,testP;
	int			n,i,action;
	SSModule	*dstMod,*srcMod;
	for (n = 0; n < mainInst->nbrModules; ++n) {
		srcMod = mainInst->mods[n];
		for (i = 0; i < mainInst->mods[n]->nbrInputs; ++i) {
			dstMod = srcMod->inputs[i].link;
			dstMod->GetOutputDock(&dp,srcMod);
			srcMod->GetInputDock(&sp,dstMod);
			if (PtInLine(p,sp,dp)) {
				// Copy Link Type
				// 0+ = change link type
				// -1 = nothing
				// -2 = delete


				// Track mouse for 1/2 second....
				// If it moves, repatch
				if (hasCmdKey)
					repatchFlag = false;
				else {
					long	t;
					repatchFlag = false;
					t= TickCount();
					while (TickCount() - t < 20 && !repatchFlag) {
						GetMouse(&testP);
						if (abs(testP.h - p.h) > 3 ||
							abs(testP.v - p.v) > 3)
						{
							repatchFlag = true;
						}
					}
				}

				// REPATCH if mouse is in motion OR command key
				if (repatchFlag) {
					int	oldLinkType = srcMod->inputs[i].inputType;
					// Delete Link
					srcMod->inputs[i] = srcMod->inputs[srcMod->nbrInputs-1];
					--srcMod->nbrInputs;
					FullRefresh();
					if (dstMod){
						GetMouse(&testP);
						DragNewLink(dstMod,testP,oldLinkType);
					}
				}
				else {
					action = srcMod->GetPatchCommand(p,i);
					if (action == -2) {
						// Delete Link
						srcMod->inputs[i] = srcMod->inputs[srcMod->nbrInputs-1];
						--srcMod->nbrInputs;
					}
					else if (action >= 0) {
						srcMod->inputs[i].inputType = action;
					}
					FullRefresh();
				}
				return;
			}
		}
	}
}


void MainWindow::InitSSDraw()
{
	RGBColor	fc;
	short		i;
	fc.red = fc.green = fc.blue = 0x8888;
	frameColor = fc;
	fc.red = fc.green = fc.blue = 0xAAAA;
	frameLtColor = fc;
	fc.red = fc.green = fc.blue = 0x6666;
	frameDkColor = fc;

	for (i = 0; i < MT_NbrModules; ++i) {
		if (mDesc[i].iconNbr) {
			mDesc[i].itsIcon = GetCIcon(mDesc[i].iconNbr);
			mDesc[i].itsSmallIcon = GetCIcon(mDesc[i].iconNbr+100);
		}	
	}
	
	InitGadgetPanel();
}


void MainWindow::DrawTableFrame(Rect *inFrame, Boolean invert)
{
	RGBColor	fc;
	Rect		r = *inFrame;

	GetForeColor(&fc);

	if (invert)
		RGBForeColor(&frameDkColor);
	else
		RGBForeColor(&frameLtColor);
	MoveTo(r.left,r.top);
	LineTo(r.right,r.top);
	MoveTo(r.left,r.top);
	LineTo(r.left,r.bottom);

	if (invert)
		RGBForeColor(&frameLtColor);
	else
		RGBForeColor(&frameDkColor);
	MoveTo(r.right,r.bottom);
	LineTo(r.right,r.top);
	MoveTo(r.right,r.bottom);
	LineTo(r.left,r.bottom);
	
	RGBForeColor(&fc);
}

void MainWindow::ProcessKey(EventRecord *er)
{
	char	ch;
	ch = er->message & charCodeMask;
	if (ch == '\b')
		Delete();
	if (ch == '.' || ch == 0x1b) {
		if (windowState == WS_Synthesize)
			CleanUpSynthesis();
	}
}

void MainWindow::SpinCursor()
{
	static	int	ctr;
	SetCursor(*cursors[CT_Watch1+ctr]);
	ctr = (ctr + 1) & 3;
}

void MainWindow::AdjustMenus()
{
	Boolean	hasSelection = HasSelection();
	inherited::AdjustMenus();
	if (hasSelection)
	{
		MyEnableMenuItem(gEditMenu, EM_Cut, true);
		MyEnableMenuItem(gEditMenu, EM_Copy, true);
		MyEnableMenuItem(gEditMenu, EM_Clear, true);
	}
	if (dirty)
		MyEnableMenuItem(gFileMenu, FM_Save, true);
	MyEnableMenuItem(gFileMenu, FM_SaveAs, true);

	// MyEnableMenuItem(gFileMenu, FM_Export, true);
	MyEnableMenuItem(gFileMenu, FM_Close, true);

	CheckItem(gOptionsMenu, OM_ShowMemory, showMemory);
	CheckItem(gOptionsMenu, OM_Listen, listenFlag);
	
//	CheckItem(gOutputMenu, OutM_Memory, storeToFile == false);
//	CheckItem(gOutputMenu, OutM_AIFFFile, storeToFile == true && storeFile->fileType == FT_AIFF);
//	CheckItem(gOutputMenu, OutM_WaveFile, storeToFile == true && storeFile->fileType == FT_WAVE);
}

void MainWindow::ProcessCommand(short menuID, short menuItem)
{
	switch (menuID) {
	case FileMENU:
		switch (menuItem) {
		case FM_Save:		Save();			return;
		case FM_SaveAs:		SaveAs();		return;
		// case FM_Export:		ExportAs();		return;
		}
		break;
	case EditMENU:
		switch (menuItem) {
		case EM_Cut:		Cut();			return;
		case EM_Clear:		Delete();		return;
		case EM_Duplicate:	Duplicate();	return;
		case EM_SelectAll:	SelectAll();	return;
		}
		break;
	case OptionsMENU:
		switch (menuItem) {
		case OM_ShowMemory:
			showMemory = !showMemory;
			LogMessage("Show Memory %s\r",showMemory? "On" : "Off");
			return;
		case OM_Listen:
			listenFlag = !listenFlag;
			LogMessage("Listen %s\r",listenFlag? "On" : "Off");
			return;
		}
		break;
//	case OutputMENU:
//		switch (menuItem) {
//		case OutM_Memory:
//		case OutM_AIFFFile:
//			SetOutputType(menuItem == OutM_AIFFFile? true : false);
//			return;
//		}
//		break;
	}
	inherited::ProcessCommand(menuID, menuItem);
}