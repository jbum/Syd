// ModList.cp  (Incorporate lots of the mList-related code from main window)
#include "ss.h"
#include "patchowner.h"
#include "modlist.h"
#include "allmodules.h"

#include "assert.h"  // gcc

struct ModuleDesc mDesc[MT_NbrModules] = {
{	MT_Output,		"out",		"Output"		},
{	MT_Oscillator,				"osc",		"Oscillator"	},
{	MT_Envelope,				"env",		"Envelope Generator"	},
{	MT_Mixer,					"mix",		"Mixer"	},
{	MT_Filter,					"filt",		"2nd-Order Filter"	},
{	MT_Butter,					"butter", 	"Butterworth Filter"	},
{	MT_Smooth,					"smooth",	"Smoother"	},
{	MT_Noise,					"noise",	"Noise Generator"	},
{	MT_Delay,					"delay",	"Reverb / Effects"	},
{	MT_Threshhold,			"thresh",	"Threshhold Unit"	},
{	MT_SampleAndHold, "shold",	"Sample and Hold"	},
// {	MT_Sequencer,			"seq",		"\pSequencer"	},
{	MT_Amplifier,			"amp",		"Amplifier"	},
{	MT_Inverter,			"inv",		"Inverter"	},
{	MT_Expression,  			"exp",		"Expression"	},
{	MT_Folder,  				"folder",	"Folder"		},
{	MT_RandScore,  				"rscore",	"Random Score"	},
{	MT_CScore,  				"cscore",	"CSound Score"	},
{	MT_FInput,					"finput",	"Folder Input"	},
{	MT_PInput,					"pinput",	"Score Parameter"	},
{	MT_FTable,					"ftable",	"Function Table"	},
{	MT_SampleFile,			"sample",	"Sample File"	},
{	MT_Pluck,					"pluck",	"Karplus/Strong Plucked String Sound"	},
{	MT_Maraca,					"maraca",	"Perry Cook's Maraca Simulation"	},
{	MT_HammerBank,			"hammerbank", "Hammer Bank"	},
{	MT_HammerActuator, 	"hammeract",  "Hammer Actuator"	},
{	MT_GAssign,				"gassign",	"Global Variable Assignment"	},
{ MT_SkiniScore,       "skiniscore", "Skini (STK) Score"  },
#ifdef USE_STK
{ MT_STK,               "stk",       "Test STK-based Instrument" }
#endif
};

ModList::ModList(PatchOwner* itsOwner)	// replaces LoadInstrument
{
	nbrModules = 0;
	this->itsOwner = itsOwner;
	sampleDuration = 2.0;
	sampleRate = 22050;
}

ModList::ModList(PatchOwner* itsOwner, SydFileSpec *fsSpec)	// replaces LoadInstrument
{
	nbrModules = 0;
	this->itsOwner = itsOwner;
	sampleDuration = 2.0;
	sampleRate = 22050;

	OpenSpec(fsSpec);
}

ModList::~ModList()
{
	DisposeModuleList();
}

ModList *ModList::CloneInstrument(SSModule *rootModule)
{
	int	n;
	ModList 	*nList;
	SSModule		*nn;

	nList = new ModList(itsOwner);	// was passing itsOwner in Mac version
	if (nList== NULL)
		return NULL;

	nList->nbrModules = 0;
	nList->AddModule(rootModule->moduleType);

	nn = nList->mods[nList->nbrModules-1];
	nn->CopyAll(rootModule);	// Includes name & links

	for (n = 0; n < nbrModules; ++n) {
		// If module is in subtree
		if (rootModule->ContainsMod(this->mods[n]->id) &&
			!nList->ContainsNamedMod(this->mods[n]->label)) {
			nList->AddModule(this->mods[n]->moduleType);
			nn = nList->mods[nList->nbrModules-1];
			nn->CopyAll(this->mods[n]);
		}
	}

	// Resolve Links
	nList->ResolveModuleLinks();

	return nList;
}



bool ModList::OpenSpec(SydFileSpec *fsSpec)
{
	FILE	*readFile;
  extern SydFileSpec sydSpec;

	if ((readFile = fopen(fsSpec->name,"rb")) == NULL)
	{
      char  oname[512], iname[512];
      char  *p;
      strcpy(iname,fsSpec->name);
      strcpy(oname, sydSpec.name);
      if ((p = strrchr(oname,'/')) != NULL)
         *(p+1) =0;
      else if ((p = strrchr(oname,'\\')) != NULL)
         *(p+1) =0;
      else
        oname[0] = 0;
      strcat(oname,iname);
      if ((readFile = fopen(oname,"rb")) == NULL)
      {
  			ErrorMessage("Can't open file: %s", fsSpec->name);
  			return true;
      }
	}
	// CArchive		ar(&readFile, CArchive::load);

	return OpenArchive(readFile);
}

bool ModList::OpenArchive(FILE* ar)
{
	char			*p,temp[256];
	int				n,i;

	p = GetNextInputLine(ar,"MODS",temp);
	if (p) {
		sscanf(p,"MODS %d",&n);
		DisposeModuleList();
		for (i = 0; i < n; ++i) {
			LoadModule(ar);
		}
	}

	// Swap Output Module to 0
	for (i = 0; i < nbrModules; ++i) {
		if (mods[i]->moduleType == MT_Output) {
			SSModule	*temp;
			temp = mods[0];
			mods[0] = mods[i];
			mods[i] = temp;
		}
	}

	// Resolve Links
	ResolveModuleLinks();

	fclose(ar); // added 5/29/06

	return false;
}

/*
bool ModList::Save(FILE* ar)
{
	int			n;
	SSModule	**mp;

	WriteFileLine(ar, "MODS %d\r",(int)nbrModules);

	// Rename modules to avoid collisions
	for (n = 0,mp=mods; n < nbrModules; ++n, ++mp) {
		(*mp)->ComputeName(n+1);
	}

	// Save 'em
	for (n = 0,mp=mods; n < nbrModules; ++n,++mp) {
		(*mp)->Save(ar);
	}
	return true;
}
*/

void ModList::ResolveModuleLinks()
{
	int	n,i;
	for (n = 0; n < nbrModules; ++n) {
		for (i = 0; i < mods[n]->nbrInputs; ++i) {
			if (mods[n]->inputs[i].destID == -1)
				mods[n]->inputs[i].destID = GetModuleID(mods[n]->inputs[i].fromLabel);
			mods[n]->inputs[i].link = GetModule(mods[n]->inputs[i].destID);
		}
	}
}

SSModule *ModList::GetModule(int id)
{
	int			n;
	for (n = 0; n < nbrModules; ++n) {
		if (mods[n]->id == id)
			return mods[n];
	}
	return NULL;
}

int ModList::GetModuleID(char *label)
{
	int			n;
	for (n = 0; n < nbrModules; ++n) {
		if (strcmp(label,mods[n]->label) == 0)
			return mods[n]->id;
	}
	return -1;
}

Boolean ModList::ContainsNamedMod(char *name)
{
	int	n;
	for (n = 0; n < nbrModules; ++n)
		if (strcmp(mods[n]->label,name) == 0)
			return true;
	return false;
}

void ModList::AddModule(int n)
{
	SSModule	*nn=NULL;
	short		x=10,y=60;

	switch (n) {
	case MT_Output:		nn = new SSOutput(this,x,y);			break;
	case MT_Oscillator:	nn = new SSOscillator(this,x,y);		break;
	case MT_Amplifier:	nn = new SSAmplifier(this,x,y);			break;
	case MT_Envelope:	nn = new SSADSR(this,x,y);				break;
	case MT_Inverter:	nn = new SSInverter(this,x,y);			break;
	case MT_Noise:		nn = new SSNoise(this,x,y);				break;
	case MT_Threshhold:	nn = new SSThreshhold(this,x,y);		break;
	case MT_SampleAndHold: nn = new SSSampleAndHold(this,x,y);	break;
	case MT_Delay:		nn = new SSDelay(this,x,y);				break;
	case MT_Filter:		nn = new SSFilter(this,x,y);			break;
	case MT_Butter:		nn = new SSButter(this,x,y);			break;
	case MT_Smooth:		nn = new SSSmooth(this,x,y);			break;
	case MT_Expression:	nn = new SSExpression(this,x,y);		break;
	case MT_Mixer:		nn = new SSMixer(this,x,y);				break;
	case MT_RandScore:	nn = new SSRandScore(this,x,y);			break;
	case MT_CScore:		nn = new SSCScore(this,x,y);			break;
	case MT_Folder:		nn = new SSFolder(this,x,y);			break;
	case MT_FInput:		nn = new SSFInput(this,x,y);			break;
	case MT_PInput:		nn = new SSPInput(this,x,y);			break;
	case MT_FTable:		nn = new SSFTable(this,x,y);			break;
	case MT_SampleFile:	nn = new SSSampleFile(this,x,y);		break;
	case MT_Pluck:		nn = new SSPluck(this,x,y);				break;
	case MT_Maraca:		nn = new SSMaraca(this,x,y);			break;
	case MT_HammerBank:	nn = new SSHammerBank(this,x,y);		break;
	case MT_HammerActuator:	nn = new SSHammerActuator(this,x,y);		break;
	case MT_GAssign:	nn = new SSGAssign(this,x,y);			break;
  case MT_SkiniScore:   nn = new SSSkiniScore(this,x,y);      break;
#ifdef USE_STK
	case MT_STK:			nn = new SSStk(this,x,y);			break;
#endif
	}
	if (nn) {
		mods[nbrModules++] = nn;
	}
	else
		ErrorMessage("Couldn't allocate module");
}

void ModList::LoadModule(FILE* ar)
{
	char		*p,temp[256];
	int			id=0,moduleType=0,x1=0,y1=0,x2=0,y2=0,nbrInputs=0;
	SSModule	*nn;
	short		i;
	Boolean		oldFormat = false;
	char		modLabel[32],modTypeName[32];

	int			linkID,destID,inputType;
	char		inputName[32],fromLabel[32];


	p = GetNextInputLine(ar,"MOD",temp);
	if (isdigit(p[4])) {
		// itsOwner->SetModifiedFlag();	// itsOwner->dirty = true;
		oldFormat = true;
		sscanf(p,"MOD %d %d %d %d %d %d %d",&id,
				&moduleType,&x1,&y1,&x2,&y2,&nbrInputs);
		sprintf(modLabel,"m%s%d",GetModuleName(moduleType),nbrModules+1);
		modLabel[1] = toupper(modLabel[1]);
	}
	else {
		sscanf(p,"MOD %s %s (%d %d %d %d)",
				modLabel,modTypeName,
				&x1,&y1,&x2,&y2);
		nbrInputs = 0;
		moduleType = NameToModuleType(modTypeName);
		id = nbrModules;
	}
	AddModule(moduleType);

	nn = mods[nbrModules-1];
	nn->SetLabel(modLabel);
	nn->id = id;

#if UI_FEATURES
	// SetRect(&nn->cellBounds,x1,y1,x2,y2);
	nn->ComputeBounds(x1,y1);
#endif

	if (oldFormat) {
		for (i = 0; i < nbrInputs; ++i) {
			p= GetNextInputLine(ar,"IN",temp);
			sscanf(p,"IN %d %d %s",&linkID,&destID,&inputName);
			inputType = nn->NameToSignalType(inputName);

			nn->inputs[i].link = NULL;
			nn->inputs[i].inputType = inputType;
			nn->inputs[i].destID = destID;
		}
	}
	else {
		nbrInputs = 0;
		while ((p = GetNextInputLine(ar,"IN",temp)) != NULL) {
			sscanf(p,"IN %s -> %s",&fromLabel,&inputName);
			inputType = nn->NameToSignalType(inputName);
			if (inputType == MaxInputs) // signal may have been renamed...
				inputType = 0;
      printf("Modlist: Adding input type %d, label=%s\n",
            inputType,fromLabel);

			strcpy(nn->inputs[nbrInputs].fromLabel,fromLabel);
			nn->inputs[nbrInputs].link = NULL;
			nn->inputs[nbrInputs].inputType = inputType;
			nn->inputs[nbrInputs].destID = -1;
			++nbrInputs;
		}

	}
	nn->nbrInputs = nbrInputs;
	nn->Load(ar);
}

void ModList::ResetInstruments(SSModule *callingMod)
{
	int	n;

	InitTime(0);
	// Reset Modules
	for (n = 0; n < nbrModules; ++n)
		mods[n]->Reset(callingMod);

	// Fill FTables (which may depend on reset modules)
	for (n = 0; n < nbrModules; ++n)
		if (mods[n]->moduleType == MT_FTable)
			((SSFTable *) mods[n])->FillTable(callingMod);

	// Generate Scores (which may depend on ftables, etc.)
	for (n = 0; n < nbrModules; ++n)
		if (mods[n]->moduleType == MT_RandScore ||
			mods[n]->moduleType == MT_CScore ||
      mods[n]->moduleType == MT_SkiniScore)
			((SSScore *) mods[n])->InitScore(callingMod);

}

void ModList::CleanUpInstruments()
{
	int	n;
	for (n = 0; n < nbrModules; ++n)
		mods[n]->CleanUp();
}

void ModList::DisposeModuleList()
{
	int	n;
	for (n = 0; n < nbrModules; ++n) {
		mods[n]->CleanUp();	// Required!! because you can't use virtual function from a destructor
		delete mods[n];
	}
	nbrModules = 0;
}

// useful if we change file i/o models
char *ModList::GetNextInputLine(FILE* ar, char *pat, char *buf)
{
	char	*p;
	static char	tbuf[512];
	int		ctr=0;
	while (1)
	{
		++ctr;
		assert(ctr < 10);

		if (tbuf[0] == 0)
		{
			if ((p = fgets(tbuf,511,ar)) == NULL)
				return NULL;
		}
		if (tbuf[0] == 0 ||
			(tbuf[0] == '#' || tbuf[0] == ';' || tbuf[0] == '\n' ||
			 tbuf[0] == '\r'))
		{
			tbuf[0] = 0;
			continue;
		}
		if (strncmp(tbuf,pat,strlen(pat)) == 0)
		{
			strcpy(buf, tbuf);
			tbuf[0] = 0;
			return buf;
		}
		return NULL;
	}
}


char *ModList::GetModuleName(int n)
{
	return mDesc[n].name;
}

char *ModList::GetModuleDesc(int n)
{
	return mDesc[n].desc;
}

int ModList::NameToModuleType(char *name)
{
	int	i;
	for (i = 0; i < MT_NbrModules; ++i)
		if (strcmp(mDesc[i].name,name) == 0)
			return i;
	return -1;
}

#ifdef UI_FEATURES
SSModule *ModList::GetPickedModule(Point p)
{
	int	i;
	for (i = nbrModules-1; i >= 0; --i) {
		if (LocalPtInRect(p, &mods[i]->cellBounds)) {
			return mods[i];
		}
	}
	return NULL;
}
#endif

SSModule *ModList::GetOutputModule()
{
	int	i;
	for (i =0 ; i < nbrModules; ++i)
		if (mods[i]->moduleType == MT_Output)
			return mods[i];
	return NULL;
}

void ModList::DeleteLinks(SSModule *ss)
{
	int			n,i;
	SSModule	**mp;
	mp = mods;
	for (n = 0; n < nbrModules; ++n,++mp) {
		for (i = 0; i < (*mp)->nbrInputs; ++i) {
			if ((*mp)->inputs[i].link == ss) {
				(*mp)->inputs[i] = (*mp)->inputs[(*mp)->nbrInputs-1];
				--((*mp)->nbrInputs);
				--i;
			}
		}
	}
}

#ifdef UI_FEATURES
void ModList::Delete()
{
	int	n;
	for (n = 0; n < nbrModules; ++n) {
		if (mods[n]->selected && mods[n]->moduleType != MT_Output) {
			DeleteLinks(mods[n]);
			// mainInst->mods[n]->Dispose();
			mods[n]->CleanUp();	// !! Required - can't call virtual function from destuctor
			delete mods[n];
			mods[n] = mods[nbrModules-1];
			--nbrModules;
			--n;
		}
	}
}
#endif

void ModList::InitTime(ssfloat timeVal)
{
	timeStackCtr = 0;
	pTime = timeVal;
}

void ModList::PushTime(ssfloat timeVal)
{
	pTimes[timeStackCtr++] = pTime;
	pTime = timeVal;
}

void ModList::PopTime()
{
	pTime = pTimes[--timeStackCtr];
}
