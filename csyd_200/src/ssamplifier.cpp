// SSAmplifier.cp

#include "ss.h"
#include "expmgr.h"
#include "ssamplifier.h"
#include "patchowner.h"

enum {SIG_Sig, SIG_Ctl, SIG_Ctl1,SIG_Ctl2,SIG_Ctl3};

SSAmplifier::SSAmplifier(ModList * mList, short h, short v) : SSModule(MT_Amplifier, mList, h, v)
{
	InitExp(&scaleExp, "1.0");
	InitExp(&offsetExp,"0.0");
	InitExp(&panExp,"0.0");
//	scale = 1.0;
//	offset = 0.0;
	DescribeLink(SIG_Sig, "Signal to Amplify", "sig",0,0,0xFFFF);
	DescribeLink(SIG_Ctl, "Control Signal", "ctl",0xFFFF,0x8888,0x8888);
	DescribeLink(SIG_Ctl1, "Control Signal 1", "ctl1",0xFFFF,0x8888,0);
	DescribeLink(SIG_Ctl2, "Control Signal 2", "ctl2",0xFFFF,0x8888,0);
	DescribeLink(SIG_Ctl3, "Control Signal 3", "ctl3",0xFFFF,0x8888,0);
}

void	SSAmplifier::Copy(SSModule *ss)
{
	SSAmplifier *sa = (SSAmplifier *) ss;
	SSModule::Copy(ss);
	// scale = sa->scale;
	// offset = sa->offset;
	CopyExp(&sa->scaleExp, &scaleExp);
	CopyExp(&sa->offsetExp, &offsetExp);
	CopyExp(&sa->panExp, &panExp);
}

#ifdef SAVE_SUPPORT
void SSAmplifier::Save(FILE* ar)
{
	SSModule::Save(ar);
	if (strcmp(panExp.exp,"0.0") != 0 && strcmp(panExp.exp,"0") != 0)
	   WriteFileLine(ar,"AMPP %s\r",panExp.exp);
	WriteFileLine(ar,"AMPS %s\r",scaleExp.exp);
	WriteFileLine(ar,"AMPO %s\r",offsetExp.exp);
}
#endif

void SSAmplifier::Load(FILE* ar)
{
	char				*p,tbuf[256];
	int					wType=0;
	p = parList->GetNextInputLine(ar,"AMP",tbuf);
	if (p[3] == ' ') {
		// Old Style Input
		double sc=0,of=0;
		sscanf(p, "AMP %lf %lf",&sc,&of);
		PrintfExp(&scaleExp, "%lg", sc);
		PrintfExp(&offsetExp, "%lg", of);
		// parList->itsOwner->SetModifiedFlag();  // dirty = true;
	}
	else {
		char	tStr[256];
		if (strncmp(p,"AMPP ",5) == 0) {
  		sscanf(p, "AMPP %s",tStr);
  		InitExp(&panExp,tStr);
  		LoadExp(ar,"AMPS",&scaleExp);
 		}
		else {
  		sscanf(p, "AMPS %s",tStr);
  		InitExp(&scaleExp,tStr);
    }
 		LoadExp(ar,"AMPO",&offsetExp);
 	}
}


ssfloat SSAmplifier::GenerateOutput(SSModule *callingMod)
{
	ssfloat	scale,offset,pan,vL,vR;
	scale = SolveExp(&scaleExp, callingMod);
	offset = SolveExp(&offsetExp, callingMod);
  pan = SolveExp(&panExp, callingMod);
	vL =  MixInputs(SIG_Sig, callingMod)*scale + offset;
	vR = lastRightInput*scale + offset;

  // printf("pan = %g  sig=%g exp=%s\n", pan, vL, panExp.exp);

	if (pan < -1) pan = -1;
	else if (pan > 1) pan = 1;

  if (pan < 0.0)
  {
    vR *= pan+1;
  }
  else if (pan > 0.0)
  {
    vL *= 1-pan;
  }
  lastRightSample = vR;
	return vL;
}

bool SSAmplifier::CanRenderBlock()
{
	return ExpMgr::Vectorizable(&scaleExp) && ExpMgr::Vectorizable(&offsetExp) &&
		   ExpMgr::Vectorizable(&panExp) && InputsRenderBlock(SIG_Sig);
}

// GenerateOutput at n times (n <= 256; see SSModule::RenderBlock)
void SSAmplifier::RenderBlock(SSModule *callingMod, const ssfloat *times, int n, ssfloat *out,
							  ssfloat *outR)
{
	ssfloat	scale[256], offset[256], pan[256], mix[256], mixR[256];
	int		count;

	SolveExpVector(&scaleExp, callingMod, times, n, scale);
	SolveExpVector(&offsetExp, callingMod, times, n, offset);
	SolveExpVector(&panExp, callingMod, times, n, pan);
	MixInputsBlock(SIG_Sig, times, n, mix, mixR, &count);
	for (int i = 0; i < n; ++i) {
		ssfloat	vL = mix[i]*scale[i] + offset[i];
		ssfloat	vR = mixR[i]*scale[i] + offset[i];
		ssfloat	p = pan[i];
		if (p < -1) p = -1;
		else if (p > 1) p = 1;
		if (p < 0.0)
			vR *= p+1;
		else if (p > 0.0)
			vL *= 1-p;
		out[i] = vL;
		outR[i] = vR;
	}
	lastRightInput = mixR[n - 1];
	lastRightSample = outR[n - 1];
}




void SSAmplifier::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);
	ResetExp(&scaleExp, callingMod);
	ResetExp(&offsetExp, callingMod);
	ResetExp(&panExp, callingMod);
}
