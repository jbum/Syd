// SSHammerActuator.cp
//

#include "ss.h"
#include "sshammeractuator.h"
#include "sshammerbank.h"

#include <math.h>

// void	SSHammerActuator::Initialize(short itsType, short h, short v)
SSHammerActuator::SSHammerActuator(ModList * mList, short h, short v) : SSModule(MT_HammerActuator, mList, h, v)
{
	InitExp(&bNbrExp,"0");
	InitExp(&keyNbrExp,"p5");
	InitExp(&triggerExp,"1");
	InitExp(&velocityExp,"p4/128");
	InitExp(&undampenExp,"1");
	DescribeLink(0, "Default Signal", "sig",	0x0000,0x0000,0xffff);
	DescribeLink(1, "Alt Signal #1", "sig1",	0x1111,0x1111,0xeeee);
	DescribeLink(2, "Alt Signal #2", "sig2",	0x2222,0x2222,0xdddd);
	DescribeLink(3, "Alt Signal #3", "sig3",	0x3333,0x3333,0xcccc);
	DescribeLink(4, "Control Signal", "ctl",	0xFFFF,0x1111,0xeeee);
	DescribeLink(5, "Alt Control Sig #1", "ctl1",	0xFFFF,0x2222,0xdddd);
	DescribeLink(6, "Alt Control Sig #2", "ctl2",	0xFFFF,0x3333,0xcccc);
	DescribeLink(7, "Alt Control Sig #3", "ctl3",	0xFFFF,0x4444,0xbbbb);
}

void SSHammerActuator::Reset(SSModule *callingMod)
{
	SSModule::Reset(callingMod);

	bNbr = (int) ResetExp(&bNbrExp, callingMod);
	ResetExp(&keyNbrExp, callingMod);
	ResetExp(&triggerExp, callingMod);
	ResetExp(&velocityExp, callingMod);
	ResetExp(&undampenExp, callingMod);
	lastTrigger = false;
}

ssfloat	SSHammerActuator::GenerateOutput(SSModule *callingMod)
{
	ssfloat	trigger;
	trigger = SolveExp(&triggerExp,callingMod);
	if (trigger > 0.0) {
		if (!lastTrigger) {	// Perform Trigger, and wait for signal to drop
			ssfloat velocity,undampen;
			int		keyNbr;
			keyNbr = (int) SolveExp(&keyNbrExp, callingMod);
			velocity = SolveExp(&velocityExp, callingMod);
			undampen = SolveExp(&undampenExp, callingMod);
			// !! add sympathetic res flag later...
			ActuateKey(bNbr,(int) keyNbr,0,velocity,undampen);
		}
		lastTrigger = true;
	}
	else
		lastTrigger = false;
	return 0.0;
}


void SSHammerActuator::CleanUp()
{
}



void SSHammerActuator::Copy(SSModule *ss)
{
	SSHammerActuator	*hama = (SSHammerActuator *) ss;
	SSModule::Copy(ss);
	CopyExp(&hama->bNbrExp, &bNbrExp);
	CopyExp(&hama->keyNbrExp, &keyNbrExp);
	CopyExp(&hama->triggerExp, &triggerExp);
	CopyExp(&hama->velocityExp, &velocityExp);
	CopyExp(&hama->undampenExp, &undampenExp);
}

/*
void SSHammerActuator::Save(FILE* ar)
{
	SSModule::Save(ar);
	WriteFileLine(ar,"HAMA\r");
  WriteFileLine(ar,"HAMAN %s\r",bNbrExp.exp);
  WriteFileLine(ar,"HAMAK %s\r",keyNbrExp.exp);
  WriteFileLine(ar,"HAMAT %s\r",triggerExp.exp);
  WriteFileLine(ar,"HAMAV %s\r",velocityExp.exp);
  WriteFileLine(ar,"HAMAU %s\r",undampenExp.exp);
}
*/

void SSHammerActuator::Load(FILE* ar) 
{
	char				*p,tbuf[256];
	int					var=0;

	p = parList->GetNextInputLine(ar,"HAMA", tbuf);
	LoadExp(ar,"HAMAN",&bNbrExp);
	LoadExp(ar,"HAMAK",&keyNbrExp);
	LoadExp(ar,"HAMAT",&triggerExp);
	LoadExp(ar,"HAMAV",&velocityExp);
	LoadExp(ar,"HAMAU",&undampenExp);
}
