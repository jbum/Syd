// #include "stdafx.h"	// Windows Only
// SSPluck.cp
//
// Pluck output is normalized to 0-1, to make it easier to
// use for amplitude control etc.

#include "ss.h"
#include "sspluck.h"
// #include "MainWin.h"
#include "patchowner.h"

#include <math.h>

// void	SSPluck::Initialize(short itsType, short h, short v)
SSPluck::SSPluck(ModList * mList, short h, short v) : SSModule(MT_Pluck, mList, h, v)
{
	InitExp(&freqExp,"440");
	InitExp(&durExp,"2");
	InitExp(&ampExp,"1");
	InitExp(&decayExp,"10");
	variant = KP_Normal;
	DescribeLink(0, "INVALID", "???",0xFFFF,0xFFFF,0xFFFF);
	error = false;
	d = NULL;
}

void SSPluck::Reset(SSModule *callingMod)
{
	ssfloat		f1,amp,Q,loop,delay,D,Gf,G,R;
	SSModule::Reset(callingMod);

  // Test to use predictable buffer
  // MySRand(100);

	f1 = ResetExp(&freqExp, callingMod);
	dur = ResetExp(&durExp, callingMod);
	Q = ResetExp(&decayExp, callingMod);
	amp = ResetExp(&ampExp, callingMod);

	if (Q == 0.0)
		Q = 0.00001; // bug fix 10/5/98

  error = false;

  if (f1 == 0 || f1 > 20000)
  {
    // ErrorMessage("Error Resetting Pluck: f=%.2f dur=%.2f amp=%.2f", f1, dur, amp);
    error = true;
    return;
  }


	rho = 1.0;
	S = 0.5;
	C = 0.9999;
	R = parList->itsOwner->mainInst->sampleRate;
	loop = R / f1;
	p = (int) loop;
	Gf = pow(10.0, -Q / (20.0 * f1 * dur));	// loop gain required at any req for a Q db loss over dur seconds
	G = cos(pi * f1 / R);				// loop gain without adjustments to rho and S

	// If smaller gain needed, reduce with rho, otherwise, stretch with S
	if (Gf <= G)
		rho = Gf/G;
	else {
		double	cosf1,a,b,c,D,a2,S2;
		cosf1 = cos(2*pi*f1/R);
		a = 2.0 - 2.0*cosf1;
		b = 2.0*cosf1 - 2.0;
		c = 1.0 - Gf*Gf;
		D = sqrt(b*b-4.0*a*c);
		a2 = 2.0*a;
		S1 = (-b + D)/a2;	// quadratic formula
		S2 = (-b - D)/a2;
		if (S1 > 0.0 && S1 <= 0.5)
			S = S1;
		else
			S = S2;
	}
	delay = p+S;	// approx loop delay
	if (delay > loop)
		delay = --p + S;
	D = loop - delay;
	C = (1.0 - D) / (1.0 + D);

	if (S <= 0.0 || S > 0.5 || rho < 0.0 || rho > 1.0 || fabs(C) >= 1.0) 
	{
		error = true;
		LogMessage("Pluck Error\n");
	}
	d = (ssfloat *) MyNewPtrClear(sizeof(ssfloat) * p);
	if (d == NULL)
	  ErrorMessage("Failed to allocated note");

  if (1)
	{
		int	i;
		ssfloat	m;

		// Initialize delay line
		if (variant == 0 || variant == 1 || variant == 2) {
			for (i = 0; i < p; ++i)
				d[i] = 2.0*DoubleRandom() - 1.0;
		} else if (variant == 3) {
			int	var = 1+(variant-1)/2;
			for (i = 0; i < p; ++i)
				d[i] = 2.0*parList->itsOwner->expMgr->Noise((var*i)/(ssfloat)p,DoubleRandom()*256) - 1.0;
		}
		else {
			int	var = 1+(variant-2)/2;
			for (i = 0; i < p; ++i)
				d[i] = 2.0*parList->itsOwner->expMgr->Turbulence((var*i)/(ssfloat)p,DoubleRandom()*256) - 1.0;
		}
		// Compute average
		m = 0.0;
		for (i = 0; i < p; ++i)
			m += d[i];
		m /= p;

		// Subtract from table
		for (i = 0; i < p; ++i)
			d[i] -= m;

		// Normalize and scale to amplitude
		m = 0.0;
		for (i = 0; i < p; ++i)
			if (m < fabs(d[i]))
				m = fabs(d[i]);	// bug fix from book...
		m = amp/m;
		for (i = 0; i < p; ++i)
			d[i] *= m;
	}
	dur *= R;
	S1 = 1.0 - S;
	count = 0;
	sp = d;
	x1 = y1 = z = 0.0;
}

ssfloat	SSPluck::GenerateOutput(SSModule *callingMod)
{
	ssfloat	x,y;

  // if (parList->itsOwner->gTime > 5 && parList->itsOwner->gTime < 5.001) {
  //  DebugFunc("Test");
  // }

//  if (error && d == NULL) {
//    LogMessage("Attempting to reset Pluck %.5f, f=%.2f\n", parList->itsOwner->gTime);
//    error = false;
//    Reset(callingMod);
//  }
  
	if (error || d == NULL) { // Seems to be happening with great frequency...
    // LogMessage("Pluck err %.5f\n", parList->itsOwner->gTime);
		return 0.0;
  }
	if (count >= dur) {  // Not happening...
//    LogMessage("!");
		return 0.0;
  }

	x = *sp;
	if (variant == 2 || (variant == 1 && DoubleRandom() < 0.5))
		y = rho * (S1 * x - S * x1);		// filter1
	else
		y = rho * (S1 * x + S * x1);		// filter1
	z = C * (y - z) + y1;				// filter2
	*sp = z;
	x1 = x;
	y1 = y;
	++sp;
	++count;
	if (sp >= d + p)
		sp = d;
  lastRightSample = x;
//  if (x == 0.000 && parList->itsOwner->gTime > 5) {
//    LogMessage("*");
//  }
	return x;
}


void SSPluck::CleanUp()
{
	if (d) {
		MyDisposePtr((Ptr) d);
		d = NULL;
	}
}


void SSPluck::Copy(SSModule *ss)
{
	SSPluck	*osc = (SSPluck *) ss;
	SSModule::Copy(ss);
	variant = osc->variant;
	CopyExp(&osc->freqExp, &freqExp);
	CopyExp(&osc->ampExp, &ampExp);
	CopyExp(&osc->durExp, &durExp);
	CopyExp(&osc->decayExp, &decayExp);
}

/*
void SSPluck::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"PLKI %d\r",(int) variant);
  WriteFileLine(ar,"PLKF %s\r",freqExp.exp);
  WriteFileLine(ar,"PLKD %s\r",durExp.exp);
  WriteFileLine(ar,"PLKA %s\r",ampExp.exp);
  WriteFileLine(ar,"PLKd %s\r",decayExp.exp);
}
*/

void SSPluck::Load(FILE* ar) 
{
	char				*p,tbuf[512];
	int					var=0;

	p = parList->GetNextInputLine(ar,"PLKI",tbuf);
	sscanf(p, "PLKI %d",&var);
	variant = var;
	LoadExp(ar,"PLKF",&freqExp);
	LoadExp(ar,"PLKD",&durExp);
	LoadExp(ar,"PLKA",&ampExp);
	LoadExp(ar,"PLKd",&decayExp);
}
