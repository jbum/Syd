// SSHammerActuator.h
#ifndef _H_SSHammerActuator
#define _H_SSHammerActuator	1

#include "ssmodule.h"
#include "expmgr.h"

// #define HammerActuatorDLOG		221


class SSHammerActuator : public SSModule {
public:
	// Static
	ExpRec		bNbrExp,keyNbrExp,triggerExp,velocityExp,undampenExp;
	// Dynamic
	int		bNbr;
	Boolean	lastTrigger;

	// Overrides
	SSHammerActuator(ModList * mList, short h, short v);

	void	Initialize(short itsType, short h, short v);
	ssfloat	GenerateOutput(SSModule *callingMod);
	void	Reset(SSModule *callingMod);
	void	CleanUp();
	void	Copy(SSModule *ss);
	void	Load(FILE* ar);
  // int   ProcessDoubleClick();
  // void  Save(FILE* ar);
};

#endif
