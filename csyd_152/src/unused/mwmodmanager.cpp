// SSModManager.cp

#include <OSUtils.h>
#include <math.h>

#if GENERATINGPOWERPC
#include <fp.h>
#endif

#include "SS.h"
// #include "MainWin.h"
#include "GraphWin.h"
#include "DeckSounds.h"
#include "SSSound.h"
#include "SSMenus.h"
#include "SSStoreFile.h"
#include "SSStoreMem.h"
#include "ModList.h"

#if __profile__
#include <profiler.h>		// MW Profiling
#endif

#include "SSOutput.h"

// Note: Most of these should be moved to ModList

// Module stuff

void MainWindow::InitSynth()
{
	GrafPtr		theWin = (WindowPtr) &this->theWin;
	SSModule	*outMod;

	DisposeAllModules();
	mainInst = new ModList(this);
//	if (mainInst == NULL)
//		mainInst = (ModuleList *) MyNewPtrClear(sizeof(ModuleList));;
//	mainInst->nbrModules = 0;
//	mainInst->sampleDuration = 2.0;
//	mainInst->sampleRate = 22050.0;

	windowState = WS_Idle;

	outMod = new SSOutput(mainInst,theWin->portRect.right-80,theWin->portRect.bottom/2);
	if (outMod == NULL)
		ErrorMessage("Couldn't allocate output module");
	else
		mainInst->mods[mainInst->nbrModules++] = outMod;
}

#if GENERATING68881

void d96tod80(SSextended96 *src, SSextended80 *dst)
{
	Ptr	s = (Ptr) src;
	Ptr d = (Ptr) dst;
	d[0] = s[0];
	d[1] = s[1];
	d[2] = s[4];
	d[3] = s[5];
	d[4] = s[6];
	d[5] = s[7];
	d[6] = s[8];
	d[7] = s[9];
	d[8] = s[10];
	d[9] = s[11];
}

void d80tod96(SSextended80 *src, SSextended96 *dst)
{
	Ptr	s = (Ptr) src;
	Ptr d = (Ptr) dst;
	d[0] = s[0];
	d[1] = s[1];
	d[2] = 0;
	d[3] = 0;
	d[4] = s[2];
	d[5] = s[3];
	d[6] = s[4];
	d[7] = s[5];
	d[8] = s[6];
	d[9] = s[7];
	d[10] = s[8];
	d[11] = s[9];
}

#endif

void MainWindow::SetOutputType(Boolean fileFlag, FSSpec *fsSpec)	// Output to AIFF file
{
	if (fileFlag) {
		if (!storeToFile) {
			storageMod->StopPlayback();
			storeFile->CloseFile();
			// storeFile->GetFileSpec();
			storeFile->SetWriteFileSpec(fsSpec);
			if (storeFile->Import(storageMod) == noErr) {
				storageMod = storeFile;
				storeToFile = true;
			}
		}
		else {
			storeFile->CloseFile();
			storeFile->SetWriteFileSpec(fsSpec);
			// storeFile->GetFileSpec();
			storeFile->nbrFrames = 0;
		}
	}
	else {
		if (storeToFile) {
			storageMod->StopPlayback();
			if (storeMem->Import(storageMod) == noErr) {
				storageMod = storeMem;
				storeToFile = false;
			}
		}
	}
}


void MainWindow::PerformSynthesis()
{
	// Initialize default parameters (for preview - overwritten by score module)
	// defaultInstParams.p1 = 0;
	// defaultInstParams.p2 = 0.0;
	// defaultInstParams.p3 = mainInst->sampleDuration;
	// SetInstrumentParameters(0,&defaultInstParams);

	LogMessage("Starting Synthesis... [%d]\r",gModAlloc);
	if (showMemory)
		LogMessage("%s\r",StatString());

	InitGlobals();

	outMod = mainInst->GetOutputModule();
	if (outMod == NULL)
		return;

	mainInst->sampleRate = ((SSOutput *) outMod)->sampleRate;
	mainInst->sampleDuration = ((SSOutput *) outMod)->sampleDuration;
	SetOutputType((((SSOutput *) outMod)->flags & OM_AIFF) > 0,&((SSOutput *) outMod)->outFileSpec);

	nbrSamples = mainInst->sampleRate * mainInst->sampleDuration;

	if (storageMod->StartStorage(nbrSamples,mainInst->sampleRate,listenFlag) != noErr)
		return;

	gTime = 0.0;
	timeInc = 1 / mainInst->sampleRate;
	
	SetCursor(*cursors[CT_Watch1]);

	windowState = WS_Synthesize;

	mainInst->ResetInstruments(outMod);

	sx = 0;
	startSynthTime = TickCount();
	lastGraphTime = startSynthTime;
	// windowState = WS_Synthesize;
}

void MainWindow::SynthIdle()
{
	long	t = TickCount();
	ssfloat	v;
	SpinCursor();

#if __profile__
	ProfilerSetStatus(true);
#endif

	while (sx < nbrSamples && TickCount() - t < 30 &&
			windowState == WS_Synthesize) {
		gTime = (ssfloat) sx / mainInst->sampleRate;
		// noteTime = gTime;
		v = ((SSOutput *) outMod)->GenerateOutputTime(NULL,gTime);	// will output from 0 - 1
		storageMod->StoreSampleF(v);
		++sx;
	}
#if __profile__
	ProfilerSetStatus(false);
#endif
	// sh->numFrames = sx;
	if (sx >= nbrSamples || windowState != WS_Synthesize)
		CleanUpSynthesis();
	else if (gw != NULL && (t = TickCount()) - lastGraphTime > 180) {
		lastGraphTime = t;
		gw->GraphSamples(storageMod,nbrSamples);
	}
}

void MainWindow::CleanUpSynthesis()
{
	storageMod->StopStorage();
	windowState = WS_Idle;
	mainInst->CleanUpInstruments();

	LogMessage("...Ending Synthesis [%d]\r",gModAlloc);
	if (showMemory)
		LogMessage("%s\r",StatString());

	// Display and play samples
	SetCursor(&qd.arrow);
	PrintToHelp("Elapsed: %6.2f secs", (TickCount() - startSynthTime) / 60.0);
	LogMessage("Elapsed: %6.2f secs\r", (TickCount() - startSynthTime) / 60.0);
	if (gw)
		gw->GraphSamples(storageMod,storageMod->nbrFrames);
}


void MainWindow::AbortSynthesis()	// Trigger Abort
{
	windowState = WS_Abort;
}

void MainWindow::PlayBuffer()
{
	storageMod->PlaySound();
}

void MainWindow::RefreshModules(Rect *rr)
{
	int			n,i;
	Rect		dmy;
	Point		sp,dp;
	SSModule	**mp;
	LinkDescPtr	ld;
	RGBColor	blackColor={0,0,0};
	SSModule	*srcMod, *dstMod;

	// GrafPtr	theWin = (GrafPtr) &this->theWin;

	// SetPort(theWin);
	mp = mainInst->mods;
	for (n = 0; n < mainInst->nbrModules; ++n,++mp) {
		srcMod = *mp;
		if (SectRect(rr,&srcMod->bounds,&dmy))
			srcMod->Draw(rr);
		// srcMod->GetInputDock(&sp);
		for (i = 0; i < srcMod->nbrInputs; ++i) {
			dstMod = srcMod->inputs[i].link;
			dstMod->GetOutputDock(&dp,srcMod);
			srcMod->GetInputDock(&sp,dstMod);
			Pt2Rect(sp,dp,&dmy);
			InsetRect(&dmy,-1,-1);
			if (SectRect(&dmy,rr,&dmy)) {
				ld = (*mp)->GetLinkDesc(srcMod->inputs[i].inputType);
				if (ld != NULL)
					RGBForeColor(&ld->color);
				else
					RGBForeColor(&blackColor);
				MoveTo(sp.h,sp.v);
				LineTo(dp.h,dp.v);
				RGBForeColor(&blackColor);
			}
		}
	}
	// Refresh Game using Current State
}

void MainWindow::DeselectAll()
{
	GrafPtr		theWin = (GrafPtr) &this->theWin;
	int			n;
	SSModule	**mp;
	SetPort(theWin);
	mp = mainInst->mods;
	for (n = 0; n < mainInst->nbrModules; ++n,++mp) {
		if ((*mp)->selected)
			(*mp)->Select(false);
	}
}

void MainWindow::SelectAll()
{
	GrafPtr		theWin = (GrafPtr) &this->theWin;
	int			n;
	SSModule	**mp;
	SetPort(theWin);
	mp = mainInst->mods;
	for (n = 0; n < mainInst->nbrModules; ++n,++mp) {
		if (!(*mp)->selected)
			(*mp)->Select(true);
	}
}

void MainWindow::Duplicate()
{
	// Get currently selected module, and duplicate it, copying
	// settings
	int			n,oldNbrModules;
	extern		long gID;

	SSModule	*nn;
	oldNbrModules = mainInst->nbrModules;

	for (n = 0; n < oldNbrModules; ++n) {
		if (mainInst->mods[n]->selected && mainInst->mods[n]->moduleType != MT_Output) {
			mainInst->AddModule(mainInst->mods[n]->moduleType);
			if (mainInst->nbrModules > oldNbrModules) {
				nn = mainInst->mods[mainInst->nbrModules-1];
				nn->Copy(mainInst->mods[n]);
				nn->ComputeBounds(nn->cellBounds.left+16,nn->cellBounds.top+16);
				// OffsetRect(&nn->bounds, 16, 16);
			}
		}
	}
	DeselectAll();
	for (n = oldNbrModules; n < mainInst->nbrModules; ++n) {
		mainInst->mods[n]->Select(true);
	}
	FullRefresh();
}

Boolean MainWindow::HasSelection()
{
	int		n;
	for (n = 0; n < mainInst->nbrModules; ++n) {
		if (mainInst->mods[n]->selected)
			return true;
	}
	return false;
}


void MainWindow::Cut()
{
	// Copy it...
	Delete();
}

void MainWindow::Delete()
{
	mainInst->Delete();
	dirty = true;
	FullRefresh();
}

void MainWindow::DisposeAllModules()
{
	if (mainInst)
		mainInst->DisposeModuleList();
}

void MainWindow::AssignGlobal(int gNbr, ssfloat value)
{
	if (gNbr >= 0 && gNbr < MaxGlobals)
		globalVars[gNbr] = value;
}

ssfloat MainWindow::RetrieveGlobal(int gNbr)
{
	if (gNbr >= 0 && gNbr < MaxGlobals)
		return globalVars[gNbr];
	else
		return 0.0;
}

void MainWindow::InitGlobals()
{
	int	i;
	for (i = 0; i < MaxGlobals; ++i) {
		globalVars[i] = 0;
	}
}