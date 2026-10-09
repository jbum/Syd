// Windows version of PatchOwner


#include "ss.h"
#include "patchowner.h"
#include "expmgr.h"
#include "ssoutput.h"
#include "ssstorage.h"
#include "ssstoremem.h"
#include "ssstorefile.h"

/*
#include "../Syd.h"
#include "../SydDoc.h"
#include "../SydView.h"
#include "../WaveViewPane.h"
*/

#ifdef UI_FEATURES
PatchOwner::PatchOwner(CSydApp* itsOwner)
#else
PatchOwner::PatchOwner()
#endif
{
	// this->itsOwner = itsOwner;

	expMgr = new ExpMgr();
	storeFile = new SSStoreFile();
	storeMem = new SSStoreMem();
	storageMod = storeMem;
	storeToFile = false;
	outMod = NULL;

//	dirty = false;
	gTime = 0;
	timeInc = 0;
	windowState = WS_Idle;
	mainInst = NULL;


#ifdef UI_FEATURES
	dragging = false;
	dragMod = NULL;
	linkDragging = false;
	linkTarget = linkSource = NULL;
	linkType = 0;
	showMemory = false;
	listenFlag = false;
	isGraphing = true;
#else
  showMemory = false;
  listenFlag = false;
  isGraphing = false;
#endif

	InitSynth();

#ifdef UI_FEATURES
	patchCursor = theApp.LoadCursor(IDC_PATCHCORDCURSOR);
	openhandCursor = theApp.LoadCursor(IDC_OPENHANDCURSOR);
	closedhandCursor = theApp.LoadCursor(IDC_CLOSEDHANDCURSOR);
	pointyhandCursor = theApp.LoadCursor(IDC_POINTYHANDCURSOR);
	arrowCursor = theApp.LoadStandardCursor(IDC_ARROW);
	crosshairCursor = theApp.LoadStandardCursor(IDC_CROSS);
	hourglassCursor = theApp.LoadStandardCursor(IDC_WAIT);
#endif
}

void PatchOwner::InitSynth()
{
	SSModule	*outMod;
	DisposeAllModules();
	mainInst = new ModList(this);

	windowState = WS_Idle;
	outMod = new SSOutput(mainInst,0,0);
	if (outMod == NULL)
		ErrorMessage("Couldn't allocate output module");
	else
		mainInst->mods[mainInst->nbrModules++] = outMod;
}

void PatchOwner::DisposeAllModules()
{
	if (mainInst)
		mainInst->DisposeModuleList();
}

void PatchOwner::AssignGlobal(int gNbr, ssfloat value)
{
	if (gNbr >= 0 && gNbr < MaxGlobals)
		globalVars[gNbr] = value;
}

ssfloat PatchOwner::RetrieveGlobal(int gNbr)
{
	if (gNbr >= 0 && gNbr < MaxGlobals)
		return globalVars[gNbr];
	else
		return 0.0;
}

void PatchOwner::InitGlobals()
{
	int	i;
	for (i = 0; i < MaxGlobals; ++i) {
		globalVars[i] = 0;
	}
}

void PatchOwner::AbortSynthesis()	// Trigger Abort
{
	windowState = WS_Abort;
}

void PatchOwner::PerformSynthesis()
{
	InitWaveTables();

	LogMessage("Starting Synthesis... [%d]\n",gModAlloc);
	if (showMemory)
		LogMessage("%s\n",StatString());
	InitGlobals();

	outMod = mainInst->GetOutputModule();
	if (outMod == NULL)
		return;

	mainInst->sampleRate = ((SSOutput *) outMod)->sampleRate;
	mainInst->sampleDuration = ((SSOutput *) outMod)->sampleDuration;

#ifdef USE_STK
  Stk::setSampleRate( mainInst->sampleRate );
  Stk::setRawwavePath( "rawwaves/" );
#endif

	SetOutputType(((SSOutput *) outMod)->outputType,
	              ((SSOutput *) outMod)->isStereo,
	              &((SSOutput *) outMod)->outFileSpec);


  // Was beginning output here...

	gTime = 0.0;
	timeInc = 1 / mainInst->sampleRate;

//	SetCursor(*cursors[CT_Watch1]);

	windowState = WS_Synthesize;

  nbrSamples = (long) (mainInst->sampleRate * mainInst->sampleDuration);
	mainInst->ResetInstruments(outMod);
  nbrSamples = (long) (mainInst->sampleRate * mainInst->sampleDuration);

  // Moved this here, so that score generation has a chance to reset score
  if (storageMod->StartStorage(nbrSamples,mainInst->sampleRate,listenFlag,((SSOutput *) outMod)->isStereo != 0? 2 : 1) != noErr)
    return;


	sampleNbr = 0;
	startSynthTime = TickCount();
#if UI_FEATURES
	lastGraphTime = startSynthTime;
#endif
	// !! Setup thread/idle here!
	// itsOwner->SetTimer(SYNTH_TIMER, SYNTH_TIMER_INTERVAL, NULL);
}

void PatchOwner::SetOutputType(int fileType, int isStereo, SydFileSpec *fsSpec)
{
	if (fileType != OM_MEMORY) {
		if (!storeToFile) {
#ifdef UI_FEATURES
			storageMod->StopPlayback();
#endif
			storeFile->CloseFile();
			// storeFile->GetFileSpec();
			storeFile->SetWriteFileSpec(fileType, isStereo, fsSpec);
			if (storeFile->Import(storageMod) == noErr) {
				storageMod = storeFile;
				storeToFile = true;
			}
		}
		else {
			storeFile->CloseFile();
			storeFile->SetWriteFileSpec(fileType, isStereo, fsSpec);
			// storeFile->GetFileSpec();
			storeFile->nbrFrames = 0;
		}
	}
	else {
		if (storeToFile) {
#ifdef UI_FEATURES
			storageMod->StopPlayback();
#endif
			if (storeMem->Import(storageMod) == noErr) {
				storageMod = storeMem;
        storageMod->nbrChannels = isStereo > 0? 2 : 1;
				storeToFile = false;
			}
		}
	}
}

void PatchOwner::SynthIdle()
{
	long	t = TickCount();

#ifdef UI_FEATURES
	SpinCursor();
#endif

#if __profile__
	ProfilerSetStatus(true);
#endif

  if (((SSOutput *) outMod)->isStereo != 0)
  {
    ssfloat vL, vR;

    while (windowState == WS_Synthesize && sampleNbr < nbrSamples &&
        ( (sampleNbr & 0x0FF) > 0 ||
           TickCount() - t < (TICKS_PER_SEC*5) )
        ) {
      gTime = (ssfloat) sampleNbr / mainInst->sampleRate;
      vL = ((SSOutput *) outMod)->GenerateOutputTime(NULL,gTime);	// will output from 0 - 1
      vR = ((SSOutput *) outMod)->getRightSample();	// will output from 0 - 1
      storageMod->StoreSampleFS(vL, vR);
      ++sampleNbr;
    }
  }
  else {
  	ssfloat	v;
    while (windowState == WS_Synthesize && sampleNbr < nbrSamples &&
        ( (sampleNbr & 0x0FF) > 0 ||
           TickCount() - t < (TICKS_PER_SEC*5) )
        ) {
      gTime = (ssfloat) sampleNbr / mainInst->sampleRate;
      v = ((SSOutput *) outMod)->GenerateOutputTime(NULL,gTime);	// will output from 0 - 1
      storageMod->StoreSampleF(v);
      ++sampleNbr;
    }
  }
  
#if __profile__
	ProfilerSetStatus(false);
#endif
	if (sampleNbr >= nbrSamples || windowState != WS_Synthesize)
		CleanUpSynthesis();
#if UI_FEATURES
	else if (isGraphing &&
			(t = TickCount()) - lastGraphTime > TICKS_PER_SEC)
	{
		lastGraphTime = t;
		gWaveViewPane->GraphSamples(this,nbrSamples);
	}
#endif
}

void PatchOwner::CleanUpSynthesis()
{
	storageMod->StopStorage();
	windowState = WS_Idle;
	mainInst->CleanUpInstruments();

	LogMessage("...Ending Synthesis [%d]\n",gModAlloc);
	if (showMemory)
		LogMessage("%s\n",StatString());

	// Display and play samples
#ifdef UI_FEATURES
	SetCursor(arrowCursor);
	itsOwner->PrintToHelp("Elapsed: %6.2f secs", (TickCount() - startSynthTime) / (double) TICKS_PER_SEC);
#endif
	LogMessage("Elapsed: %6.2f secs\n", (TickCount() - startSynthTime) / (double) TICKS_PER_SEC);
#ifdef UI_FEATURES
	if (isGraphing)
		gWaveViewPane->GraphSamples(this,storageMod->nbrFrames);
#endif
#ifdef UI_FEATURES
	// KILL THREAD HERE...
	itsOwner->KillTimer(SYNTH_TIMER);
#endif
}

char* PatchOwner::StatString()
{
	static char tempS[64];
	gLastFreeMem = -1;
	printf(tempS, "%ld (%ld) [%d/%d]",gLastFreeMem,gLastDelta,gAllocCtr,gModAlloc);
	return tempS;
}

void PatchOwner::Playback()
{
	if (windowState != WS_Synthesize) {
#ifdef UI_FEATURES
		storageMod->StopPlayback();
		storageMod->ss_PlaySound();
#endif
	}
}
