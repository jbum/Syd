// SSOscillator.cp
//
// Oscillator output is normalized to 0-1, to make it easier to
// use for amplitude control etc.

#include "SS.h"
#include "SSOscillator.h"
#include "MainWin.h"

#include <math.h>

// void	SSOscillator::Initialize(short itsType, short h, short v)
SSOscillator::SSOscillator(ModList * mList, short h, short v) : SSModule(MT_Oscillator, mList, h, v)
{
	InitExp(&wExp,"");
	InitExp(&fExp,"440");
	InitExp(&aExp,"1");
	InitExp(&pExp,"0");
	waveInc = 0;
	waveType = WT_Sine;
	
//	Old Hard Coded Constants
//	baseFreq = 440.0;		// Add Slider/Dialog for setting this
//	fmWidth = 1.0/6.0;		// Add Slider/Dialog for setting this
//	amWidth = 1.0;
//	phase=0.0;

	DescribeLink(0, "Amplitude Modulation", "am",0,0,0xFFFF);
	DescribeLink(1, "Frequency Modulation", "fm",0,0xFFFF,0);
	DescribeLink(2, "FM Gain", "fmgain",0xFFFF,0x8888,0);
	DescribeLink(3, "AM Gain", "amgain",0,0x8888,0xFFFF);
}

ssfloat	SSOscillator::GenerateOutput(SSModule *callingMod)
{
	ssfloat	v,bf;	// fm,fmw,amw;
	
	bf = SolveExp(&fExp, callingMod, PASS_Signal);

	// Compute waveInc = position in wave from 0-1
	// waveInc is current position in wave from 0-1
	// timeInc is how much time has passed
	// bf is frequency

	// JAB - fixed to output -1 to 1
	// Note: timeInc = 1 / SampleRate
	waveInc += parList->itsOwner->timeInc * bf;
	waveInc -= (int) waveInc;

	switch (waveType) {
	case WT_Sine:	
		// v = sssin(pi*2*waveInc);	
		v = gSinTable[(int)(waveInc*WaveTableSize)];
		break;
	case WT_Sawtooth:
		v = (waveInc*2)-1;
		break;
	case WT_Square:		
		v = waveInc < .5 ? 1.0 : -1.0;
		break;
	case WT_Triangle:	
		v = waveInc < .5 ? waveInc*4-1 : (4-4*waveInc)-1;
		break;
	case WT_BLSquare:
		v = gSquareTable[(int)(waveInc*WaveTableSize)];
		break;
	case WT_Expression:
		parList->PushTime(waveInc);
		v = SolveExp(&wExp, callingMod, PASS_WaveTable);
		parList->PopTime();
		break;
	}

	v *= SolveExp(&aExp, callingMod, PASS_Signal);

	return v;
}

void SSOscillator::Reset(SSModule *callingMod)
{
	inherited::Reset(callingMod);
	waveInc = SolveExp(&pExp, callingMod, PASS_Init);	// phase
	waveInc -= (int) waveInc;

	SolveExp(&fExp, callingMod, PASS_Init);
	SolveExp(&aExp, callingMod, PASS_Init);
	SolveExp(&wExp, callingMod, PASS_Init);
}

#define OD_Freq			3
#define OD_Amp			4
#define OD_Phase		5
#define OD_Sine			6
#define OD_Expression	12

int SSOscillator::ProcessDoubleClick()
{
	GrafPtr		savePort;
	DialogPtr	dp;
	short		itemHit;
	short		i;
	short		myWaveType;
	
	GetPort(&savePort);

	if ((dp = GetNewDialog (OscDLOG, NULL, (WindowPtr) -1)) == NULL)
		return Cancel;

	PrintfItem(dp, OD_Expression, "%s", wExp.exp);
	PrintfItem(dp,OD_Freq,"%s",		fExp.exp);
	PrintfItem(dp,OD_Amp,"%s",		aExp.exp);
	PrintfItem(dp,OD_Phase,"%s",	pExp.exp);

	myWaveType = waveType;
	for (i = 0; i < WT_NbrWaveTypes; ++i)
		SetControl(dp, OD_Sine+i, myWaveType == i);

	SelIText(dp,OD_Freq,0,32767);

	SetDialogDefaultItem(dp, OK);
	SetDialogCancelItem(dp, Cancel);
	SetDialogTracksCursor(dp,true);

	ShowWindow(dp);
	SetPort(dp);
	SetCursor(&qd.arrow);
	ShowCursor();
ContinueDialog:
	do {
		ModalDialog(NULL, &itemHit);
		if (itemHit >= OD_Sine &&
			itemHit < OD_Sine+WT_NbrWaveTypes)
		{
			SetControl(dp, OD_Sine+myWaveType, false);
			myWaveType = itemHit - OD_Sine;
			SetControl(dp, OD_Sine+myWaveType, true);
		}
	} while (itemHit != OK && itemHit != Cancel);
	if (itemHit == OK) {
		if (myWaveType == WT_Expression) {
			if (GetDlogExp(dp, OD_Expression, &wExp))
				goto ContinueDialog;
		}
		waveType = myWaveType;
		if (GetDlogExp(dp, OD_Freq, &fExp))
			goto ContinueDialog;
		if (GetDlogExp(dp, OD_Amp, &aExp))
			goto ContinueDialog;
		if (GetDlogExp(dp, OD_Phase, &pExp))
			goto ContinueDialog;
	}
	DisposeDialog(dp);
	SetPort(savePort);
	return itemHit;
}

void SSOscillator::Copy(SSModule *ss)
{
	SSOscillator	*osc = (SSOscillator *) ss;
	inherited::Copy(ss);
	waveInc = osc->waveInc;
	waveType = osc->waveType;
	CopyExp(&osc->fExp, &fExp);
	CopyExp(&osc->aExp, &aExp);
	CopyExp(&osc->pExp, &pExp);
	CopyExp(&osc->wExp, &wExp);
}

void SSOscillator::Save(short fRefNum)
{
	inherited::Save(fRefNum);

/*
// Vers 1.0 style
	WriteFileLine(fRefNum,"OSCI %d\r",(int) waveType);
	WriteFileLine(fRefNum,"OSCF %s\r",fExp.exp);
	WriteFileLine(fRefNum,"OSCA %s\r",aExp.exp);
	WriteFileLine(fRefNum,"OSCP %s\r",pExp.exp);
	if (waveType == WT_Expression)
		WriteFileLine(fRefNum,"WEXP %s\r",wExp.exp);
*/
	WriteFileLine(fRefNum,"OSC %d,(%s),(%s),(%s),(%s)\r",
					(int) waveType,
					fExp.exp,aExp.exp,pExp.exp,wExp.exp);
}

void SSOscillator::Load(short fRefNum) 
{
	char				*p;
	int					wType=0;

	p = parList->GetNextInputLine(fRefNum,"OSC");
	if (p[3] == 'I') {
		// Ver 1.0 Style
		sscanf(p, "OSCI %d",&wType);
		waveType = wType;
		LoadExp(fRefNum,"OSCF",&fExp);
		LoadExp(fRefNum,"OSCA",&aExp);
		LoadExp(fRefNum,"OSCP",&pExp);
		if (waveType == WT_Expression) {
			LoadExp(fRefNum,"WEXP", &wExp);
		}
		parList->itsOwner->dirty = true;
	}
	else {
		char	tExp[256];
		if (!grepstr("OSC ([0-9]+),\((.*)\),\((.*)\),\((.*)\),\((.*)\)",p))
			LogMessage("Bad Oscillator Line");
		else {
			waveType = atoi(grepsub("$1",tExp));
			InitExp(&fExp,grepsub("$2",tExp));
			InitExp(&aExp,grepsub("$3",tExp));
			InitExp(&pExp,grepsub("$4",tExp));
			InitExp(&wExp,grepsub("$5",tExp));
		}
	}
}