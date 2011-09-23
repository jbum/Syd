// SSModule.c
#include "ss.h"
#include "ssmodule.h"
#include "modlist.h"
#include "patchowner.h"
#include "expmgr.h"
#include "ssfolder.h"

long	gID;
int		gModAlloc;

SSModule::SSModule(short itsType, ModList * mList, short h, short v)
{
	int	i;

	++gModAlloc;
	parList = mList;

#ifdef UI_FEATURES
	LocalSetRect(&cellBounds,0,0,StandardModuleSizeH,StandardModuleSizeV);
	LocalOffsetRect(&cellBounds,h,v);
  bounds = cellBounds;
#endif

	do {
		id = gID++;
	} while (parList->GetModule(id) != NULL);
	nbrInputs = 0;
	// selected = false;
	moduleType = itsType;
	nbrSupportedLinks = 1;
	lastRightSample = 0.0;
	lastRightInput = 0.0;
	callingMod = NULL;
	DescribeLink(0, "Audio Signal", "sig");
	for (i = 1; i <= MaxInputs; ++i)
		DescribeLink(i, "Undefined", "?", 0xFFFF,0xFFFF,0xFFFF);
	nbrSupportedLinks = 1;
	ComputeName(parList->nbrModules+1);
}

#ifdef UI_FEATURES
void SSModule::ComputeBounds(int x, int y)
{
	LocalSetRect(&cellBounds,0,0,StandardModuleSizeH,StandardModuleSizeV);
	LocalOffsetRect(&cellBounds,x,y);
	bounds = cellBounds;
	if (GetLabel() != NULL) {
		bounds.left -= 30;
		bounds.right += 30;
		bounds.bottom += 16;
	}
}
#endif

SSModule::~SSModule()
{
	AddLogIndent(2);
	CleanUp();
	AddLogIndent(-2);
	--gModAlloc;
}

void SSModule::ComputeName(int n)
{
	sprintf(label,"m%s%d",parList->GetModuleName(moduleType),n);
	label[1] = toupper(label[1]);
}

void SSModule::DescribeLink(int linkNbr, char *desc, char *varName,
					  int r, int g, int b)	
{
#ifdef UI_FEATURES
	linkDesc[linkNbr].color.red = r;
	linkDesc[linkNbr].color.green = g;
	linkDesc[linkNbr].color.blue = b;
#endif
	linkDesc[linkNbr].desc = desc;
	linkDesc[linkNbr].varName = varName;
	if (linkNbr >= nbrSupportedLinks)
		nbrSupportedLinks = linkNbr + 1;
}

LinkDescPtr SSModule::GetLinkDesc(int linkType)
{
	if (linkType > nbrSupportedLinks)
		linkType = 0;
	return &linkDesc[linkType];
}

int SSModule::NameToSignalType(char *name)
{
	int i;
	for (i = 0 ; i < nbrSupportedLinks; ++i)
		if (strcmp(linkDesc[i].varName,name) == 0)
			return i;
	return MaxInputs;	// Unknown
}

void SSModule::CopyAll(SSModule *mod)
{
	int	i;
	Copy(mod);
	// Copy Name & ID
	strcpy(this->label, mod->label);
	this->id = mod->id;

	// Copy Links (generic)
	for (i = 0; i < mod->nbrInputs; ++i) {
		inputs[i] = mod->inputs[i];
		inputs[i].link = NULL;
		inputs[i].destID = -1;
	}
	nbrInputs = mod->nbrInputs;
}

// Recursive!  Returns TRUE if linked to module ID
Boolean SSModule::ContainsMod(int id)
{
	if (this->id == id)
		return true;
	int i;
	for (i = 0; i < nbrInputs; ++i) {
		if (inputs[i].link->ContainsMod(id))
			return true;
	}
	return false;
}

void	SSModule::Copy(SSModule *mod)
{
#ifdef UI_FEATURES
	bounds = mod->bounds;
	cellBounds = mod->cellBounds;
#endif
}

void	SSModule::Load(FILE* ar)
{
}

void	SSModule::SetLabel(char *label)
{
	strcpy(this->label,label);
}



// Relevent to main window module list only
#ifdef UI_FEATURES

void  SSModule::Save(FILE* ar)
{
  short i;
  WriteFileLine(ar,"MOD %s %s (%d %d %d %d)\r",
            label,
            parList->GetModuleName(moduleType),
#ifdef UI_FEATURES
            (int) cellBounds.left,
            (int) cellBounds.top,
            (int) cellBounds.right,
            (int) cellBounds.bottom
#else
            0,0,0,0
#endif
            );
  if (nbrInputs) {
    for (i = 0; i < nbrInputs; ++i) {
      LinkDescPtr ld = GetLinkDesc(inputs[i].inputType);
      SSModule  *mod = parList->GetModule(inputs[i].link->id);
      WriteFileLine(ar,"IN %s -> %s\r",
            mod->label,
            ld->varName);
    }
  }
}

void SSModule::ShowLabel(char *text)
{
}
// Relevent to main window module list only
void SSModule::ComputeLinksRect(Rect *r)
{
	Rect	r2;
	int		i,n;
	Point	sp,dp;
	SSModule	**mp;

	*r = bounds;

	// for each of our links
	// GetInputDock(&sp);
	for (i = 0; i < nbrInputs; ++i) {
		inputs[i].link->GetOutputDock(&dp,this);
		GetInputDock(&sp,inputs[i].link);
		LocalPt2Rect(sp,dp,&r2);
		LocalInsetRect(&r2,-1,-1);
		LocalUnionRect(r,&r2,r);
	}

	for (n = 0,mp=parList->mods; n < parList->nbrModules; ++n,++mp) {
		for (i = 0; i < (*mp)->nbrInputs; ++i) {
			if ((*mp)->inputs[i].link == this) {
				(*mp)->GetInputDock(&dp,this);
				GetOutputDock(&sp,*mp);
				LocalPt2Rect(sp,dp,&r2);
				LocalInsetRect(&r2,-1,-1);
				LocalUnionRect(r,&r2,r);
			}
		}
	}
}
#endif

// Virtual function
ssfloat	SSModule::GenerateOutput(SSModule *callingMod)
{
	this->callingMod = callingMod;
	ssfloat retVal = MixInputs(-1, callingMod);
  lastRightSample = lastRightInput;
	return retVal;
}

ssfloat SSModule::getRightSample()
{
  return lastRightSample;
}

ssfloat SSModule::getRightInput()
{
  return lastRightInput;
}

ssfloat SSModule::GenerateOutputTime(SSModule *callingMod, ssfloat pTime)
{
  ssfloat v;
  parList->InitTime(pTime);
  v = GenerateOutput(callingMod);
  return v;
}


// Used to allow a folder instrument to retrieve input signals to the folder
Boolean SSModule::GetFolderSig(int n, ssfloat *retVal)
{
	if (callingMod && callingMod != this)
		return callingMod->GetFolderSig(n,retVal);
	else
		return false;
}

/*
Boolean SSModule::IsInFolder()
{
	if (callingMod && callingMod != this)
		return callingMod->IsInFolder();
	else
		return false;
}
*/

Boolean SSModule::IsInScore()
{
	if (callingMod && callingMod != this)
		return callingMod->IsInScore();
	else
		return false;
}


ssfloat SSModule::GetNoteTime()
{
	if (callingMod && callingMod != this)
		return callingMod->GetNoteTime();
	else
		return parList->itsOwner->gTime;
}

ssfloat SSModule::GetKeyValue()
{
	if (callingMod && callingMod != this)
		return callingMod->GetKeyValue();
	else
		return 0;
}

ssfloat SSModule::GetIValue()
{
	if (callingMod && callingMod != this)
		return callingMod->GetIValue();
	else
		return 0;
}

ssfloat SSModule::GetAValue()
{
	if (callingMod && callingMod != this)
		return callingMod->GetAValue();
	else
		return 1;
}


// Used to retreive parameters from the enclosing score module
ssfloat SSModule::GetInstParameter(int n)
{
  extern ssfloat gInstrumentParam[];
	if (callingMod && callingMod != this)
		return callingMod->GetInstParameter(n);
	else
		return gInstrumentParam[n];
}


// Regular Mix (no attenutation)
ssfloat SSModule::MixInputs(int type, SSModule *callingMod)
{
	ssfloat v = 0.0, vR = 0.0;
	int	i;

	this->callingMod = callingMod;

	for (i = 0; i < nbrInputs; ++i) {
		if (type == -1 || inputs[i].inputType == type) {
			if (inputs[i].link == NULL)
				DebugFunc("Missing Input");
			v += inputs[i].link->GenerateOutput(this);
      vR += inputs[i].link->getRightSample();

		}
	}
  lastRightInput = vR;
	return	v;
}

int SSModule::CountInputs(int type)
{
	int		n,i;

	for (i = n = 0; i < nbrInputs; ++i) {
		if (type == -1 || inputs[i].inputType == type)
			n++;
	}
	return	n;
}

int SSModule::CountInputTypes()
{
	int		n=0,i;
	long	iFlags = 0;
	
	for (i = 0; i < nbrInputs; ++i) {
		if ((iFlags & (1L << inputs[i].inputType)) == 0) {
			++n;
			iFlags |= (1L <<inputs[i].inputType);
		}
	}
	return	n;
}


#ifdef UI_FEATURES
void SSModule::Select(Boolean selectFlag)
{
	selected = selectFlag;
	parList->itsOwner->PartialRefresh(&cellBounds);
	
}

void SSModule::AddInput(SSModule *ss, int inputType) 
{
	if (ss == this || nbrInputs >= MaxInputs)
		return;
	inputs[nbrInputs].link = ss;
	inputs[nbrInputs].destID = ss->id;
	inputs[nbrInputs].inputType = inputType;
	strcpy(inputs[nbrInputs].fromLabel,ss->label);
	nbrInputs++;
}

void SSModule::GetInputDock(Point *p, SSModule *src)
{
	if ((cellBounds.top < src->cellBounds.bottom ||
		abs(cellBounds.left - src->cellBounds.right) > 
		abs(cellBounds.top - src->cellBounds.bottom)) &&
		!(cellBounds.left < src->cellBounds.right))
	{
		p->h = cellBounds.left;
		p->v = (cellBounds.top + cellBounds.bottom)/2;
	}
	else {
		p->v = cellBounds.top;
		p->h = (cellBounds.left + cellBounds.right)/2;
	}
}

void SSModule::GetOutputDock(Point *p, SSModule *dst)
{
	if ((dst->cellBounds.top < cellBounds.bottom ||
		abs(dst->cellBounds.left - cellBounds.right) > abs(dst->cellBounds.top - cellBounds.bottom)) &&
		!(cellBounds.left > dst->cellBounds.left)) 
	{
		p->h = cellBounds.right;
		p->v = (cellBounds.top + cellBounds.bottom)/2;
	}
	else {
		p->v = cellBounds.bottom;
		p->h = (cellBounds.left + cellBounds.right)/2;
	}
}

void SSModule::GetOutputDock(Point *p, Point ref)
{
	if ((ref.h < cellBounds.bottom ||
		abs(ref.h - cellBounds.right) > abs(ref.v - cellBounds.bottom)) &&
		!(cellBounds.left > ref.h)) {
		p->h = cellBounds.right;
		p->v = (cellBounds.top + cellBounds.bottom)/2;
	}
	else {
		p->v = cellBounds.bottom;
		p->h = (cellBounds.left + cellBounds.right)/2;
	}
}

int SSModule::ProcessDoubleClick()
{
	return Cancel;
}

#endif


void	SSModule::Reset(SSModule *callingMod)
{
	this->callingMod = callingMod;
}

void	SSModule::CleanUp()
{
}

// Convienience routines for built-in expressions
//
int SSModule::CompileExp(ExpRecPtr exp)
{
	int	result;
	result = parList->itsOwner->expMgr->CompileExp(this, exp->exp,exp->cExp,&exp->cFlags);
	if (result == 0 && exp->cFlags & EF_IsConstant)
		exp->eConst = parList->itsOwner->expMgr->EvalF(this, exp->cExp);
	else
		exp->eConst = 0;
	return result;
}

ssfloat SSModule::ResetExp(ExpRecPtr exp, SSModule *callingMod)
{
	this->callingMod = callingMod;
	exp->eConst = parList->itsOwner->expMgr->EvalF(this, exp->cExp);
	exp->cFlags |= EF_NeedsSolving;
	return exp->eConst;
}

ssfloat SSModule::SolveExp(ExpRecPtr exp, SSModule *callingMod)
{
	this->callingMod = callingMod;

	// 12/3/98 - modified to always solve expressions on resets and 
	//           on first sample
	if (exp->cFlags & EF_NeedsSolving)
	{
		exp->eConst = parList->itsOwner->expMgr->EvalF(this, exp->cExp);
		if (exp->cFlags & (EF_IsConstant | EF_NoTime))
			exp->cFlags &= ~EF_NeedsSolving;
	}
	return exp->eConst;
}

void SSModule::ClearExp(ExpRecPtr exp)
{
	exp->exp[0] = 0;
	exp->cExp[0] = 0;
	exp->cFlags = 0;
	exp->eConst = 0.0;
}

void SSModule::CopyExp(ExpRecPtr src, ExpRecPtr dst)
{
	strcpy((char *) dst->exp, (char *) src->exp);
	memcpy(dst->cExp,src->cExp,LenCompiledExp);
	dst->cFlags = src->cFlags;
	dst->eConst = src->eConst;
}

int SSModule::InitExp(ExpRecPtr exp, char *str)
{
	char	*sp,*dp;

	// Modified to correct for ending CRs

	dp = (char *) exp->exp;
	sp = str;
	while (*sp && *sp != '\r' && *sp != '\n') {
		*(dp++) = *(sp++);
	}
	*dp = 0;
	return CompileExp(exp);
}

int SSModule::PrintfExp(ExpRecPtr exp, char *str,...)
{
	char tbuf[128];
	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	return InitExp(exp,tbuf);
}

int SSModule::LoadExp(FILE* ar, char *lab, ExpRecPtr exp)
{
	char	tmp[32];
	char	*p;
	char	tempBuf[512];
	sprintf(tmp,"%s %%s\r",lab);
	p = parList->GetNextInputLine(ar,lab,tempBuf);
	// Avoid scanf problem with spaces in strings
	return InitExp(exp,p+(strlen(lab)+1));
}

/*
char* SSModule::GetLabel()
{
	return NULL;
}

Boolean SSModule::GetOverlay(char *overlay, int* x, int* y)
{
	*overlay = 0;
	*x = 0;
	*y = 0;
	return false;
}
*/
