// SSOscillator.cp
//

#include "ss.h"
#include "ssoscillator.h"
// #include "MainWin.h"
#include "patchowner.h"
#include "expmgr.h"

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

	if (parList->pTime == lastTime)
		return lastOutput;

	lastTime = parList->pTime;

	bf = SolveExp(&fExp, callingMod);

	// Compute waveInc = position in wave from 0-1
	// waveInc is current position in wave from 0-1
	// timeInc is how much time has passed
	// bf is frequency

	// JAB - fixed to output -1 to 1
	// Note: timeInc = 1 / SampleRate
	waveInc += (bf == HUGE_VAL? 0 : parList->itsOwner->timeInc * bf);
	if (waveInc < 0)
		waveInc -= floor(waveInc);
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
		v = SolveExp(&wExp, callingMod);
		parList->PopTime();
		break;
	}

	v *= SolveExp(&aExp, callingMod);
	lastOutput = v;
  lastRightSample = v;
	return v;
}

// Whether RenderBlock can stand in for GenerateOutput: no inputs, a built-in waveform, and
// frequency and amplitude that ExpMgr::EvalVector can evaluate.
bool SSOscillator::CanRenderBlock()
{
	return nbrInputs == 0 && waveType != WT_Expression &&
		   ExpMgr::Vectorizable(&fExp) && ExpMgr::Vectorizable(&aExp);
}

// GenerateOutput at n times (see SSModule::RenderBlock). The frequency and amplitude are
// evaluated a block at a time; the phase still steps sample by sample.
void SSOscillator::RenderBlock(SSModule *callingMod, const ssfloat *times, int n, ssfloat *out,
							   ssfloat *outR)
{
	ssfloat		bf[256], amp[256];
	ssfloat		timeInc = parList->itsOwner->timeInc;

	for (int k0 = 0; k0 < n; k0 += 256) {
		int		m = (n - k0 < 256)? n - k0 : 256;
		SolveExpVector(&fExp, callingMod, times + k0, m, bf);
		SolveExpVector(&aExp, callingMod, times + k0, m, amp);
		for (int i = 0; i < m; ++i) {
			ssfloat	v = 0.0;
			if (times[k0 + i] == lastTime) {		// as GenerateOutput, asked twice for a time
				out[k0 + i] = lastOutput;
				outR[k0 + i] = lastRightSample;
				continue;
			}
			lastTime = times[k0 + i];
			waveInc += (bf[i] == HUGE_VAL? 0 : timeInc * bf[i]);
			if (waveInc < 0)
				waveInc -= floor(waveInc);
			waveInc -= (int) waveInc;
			switch (waveType) {
			case WT_Sine:		v = gSinTable[(int)(waveInc*WaveTableSize)];				break;
			case WT_Sawtooth:	v = (waveInc*2)-1;											break;
			case WT_Square:		v = waveInc < .5 ? 1.0 : -1.0;								break;
			case WT_Triangle:	v = waveInc < .5 ? waveInc*4-1 : (4-4*waveInc)-1;			break;
			case WT_BLSquare:	v = gSquareTable[(int)(waveInc*WaveTableSize)];			break;
			}
			v *= amp[i];
			lastOutput = v;
			lastRightSample = v;
			out[k0 + i] = v;
			outR[k0 + i] = v;
		}
	}
	parList->InitTime(times[n - 1]);
}

void SSOscillator::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	waveInc = ResetExp(&pExp, callingMod);	// phase
	waveInc -= (int) waveInc;

	ResetExp(&fExp, callingMod);
	ResetExp(&aExp, callingMod);
	ResetExp(&wExp, callingMod);
	lastTime = lastOutput = -1;
  if (gVerbose > 1)
  	LogMessage("OSC: WaveType: %d\n", waveType);
}

void SSOscillator::Copy(SSModule *ss)
{
	SSOscillator	*osc = (SSOscillator *) ss;
	SSModule::Copy(ss);
	waveInc = osc->waveInc;
	waveType = osc->waveType;
	CopyExp(&osc->fExp, &fExp);
	CopyExp(&osc->aExp, &aExp);
	CopyExp(&osc->pExp, &pExp);
	CopyExp(&osc->wExp, &wExp);
}

#define Style	1

/*
void SSOscillator::Save(FILE* ar)
{
	SSModule::Save(ar);

	WriteFileLine(ar,"OSC %d,(%s),(%s),(%s),(%s)\r",
					(int) waveType,
					fExp.exp,aExp.exp,pExp.exp,wExp.exp);
}
*/

void SSOscillator::Load(FILE* ar)
{
	char				*p,tbuf[256];
	int					wType=0;

	p = parList->GetNextInputLine(ar,"OSC",tbuf);

#if Style == 1
	// Ver 1.0 Style
	sscanf(p, "OSCI %d",&wType);
	waveType = wType;
	LoadExp(ar,"OSCF",&fExp);
	LoadExp(ar,"OSCA",&aExp);
	LoadExp(ar,"OSCP",&pExp);
	if (waveType == WT_Expression) {
		// JSyd wrote this line as OSCW
		char *w = parList->GetNextInputLine(ar,"WEXP",tbuf);
		if (!w)
			w = parList->GetNextInputLine(ar,"OSCW",tbuf);
		if (w)
			InitExp(&wExp, w+5);
	}
#elif Style == 2
	if (p[3] == 'I') {
		// Ver 1.0 Style
		sscanf(p, "OSCI %d",&wType);
		waveType = wType;
		LoadExp(ar,"OSCF",&fExp);
		LoadExp(ar,"OSCA",&aExp);
		LoadExp(ar,"OSCP",&pExp);
		if (waveType == WT_Expression) {
			LoadExp(ar,"WEXP", &wExp);
		}
		parList->itsOwner->dirty = true;
	}
	else {
		// Ver 2.0 Style
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
#endif
}
