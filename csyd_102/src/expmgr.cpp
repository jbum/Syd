// ExpMgr.cp

#include "ss.h"
#include "expmgr.h"
#include "patchowner.h"
#include "ssftable.h"
#include <math.h>
// #include "MainWin.h"

OpData opData[NbrOperators] = {
	OP_None,		0,	0,
	OP_Integer,		0,	true,
	OP_Sqrt,		0,	true,
	OP_Log,			0,	true,
	OP_Log10,		0,	true,
	OP_Fabs,		0,	true,
	OP_Sign,		0,	true,
	OP_Exp,			0,	true,
	OP_Cos,			0,	true,
	OP_Sin,			0,	true,
	OP_Tan,			0,	true,
	OP_ACos,		0,	true,
	OP_ASin,		0,	true,
	OP_ATan,		0,	true,
	OP_Cosh,		0,	true,
	OP_Sinh,		0,	true,
	OP_Tanh,		0,	true,
	OP_Sin2,		0,	true,
	OP_Cos2,		0,	true,
	OP_Sin3,		0,	true,
	OP_Cos3,		0,	true,
	OP_Mandel,		0,	true,
	OP_MandelCPM,	0,	true,
	OP_Mandel3,		0,	true,
	OP_Julia,		0,	true,
	OP_Noise,		0,	true,
	OP_Turb,		0,	true,
	OP_Noise3D,		0,	true,
	OP_Turb3D,		0,	true,
	OP_Dragon,		0,	true,
	OP_Cond,		0,	true,
	OP_Angle,		0,	true,
	OP_Dist,		0,	true,
	OP_Fib,			0,	true,
	OP_Prime,		0,	true,
	OP_cpspch,		0,	true,
	OP_cpsoct,		0,	true,
	OP_octpch,		0,	true,
	OP_octcps,		0,	true,
	OP_pchoct,		0,	true,
	OP_octmidi,		0,	true,
	OP_cpsmidi,		0,	true,
	OP_FTab,		0,	true,
	OP_FTabW,		0,	true,
	OP_FTabP,		0,	true,
	OP_FTabI,		0,	true,
	OP_FTabIW,		0,	true,
	OP_FTabIP,		0,	true,
	OP_linen,		0,	true,
	OP_linenr,		0,	true,
	OP_limit,		0,	true,
//	OP_Lum,			0,	true,
//	OP_Red,			0,	true,
//	OP_Green,		0,	true,
//	OP_Blue,		0,	true,
//	OP_Color,		0,	true,
//	OP_Saturation,	0,	true,
	OP_Negate,		1,	true,
	OP_Not,			1,	true,
	OP_Invert,		1,	true,
	OP_Power,		2,	false,
	OP_Power2,		2,	false,
	OP_Multiply,	3,	false,
	OP_Divide,		4,	false,
	OP_Modulo,		5,	false,
	OP_Add,			6,	false,
	OP_Subtract,	6,	false,
	OP_And,			7,	false,
	OP_Or,			7,	false,
	OP_Xor,			7,	false,
	OP_Equal,		8,	false,
	OP_Greater,		9,	false,
	OP_Less,		9,	false,
	OP_Gte,			9,	false,
	OP_Lse,			9,	false,
	OP_Neq,			9, false,
	OP_LogicAnd, 	10,	false,
	OP_LogicOr,		10,	false
};

ExpMgr::ExpMgr()
{
	myNIL = 0.0;
	memset(&gCV, 0, sizeof(CompileVars));
	cr = &gCV;
	recur = 64;

	// Fractal Init
	lastMandelX = -2;
	lastMandelY = -2;
	lastMandelVal = 0;

	// 1/25/93 Noise Initialization
	{
		extern long	gSeed;
		int		i;
		gSeed = 1;
		for (i = 0; i < 256; ++i) {
			DoubleRandom();
			xA[i] = gSeed;
			DoubleRandom();
			yA[i] = gSeed;
			DoubleRandom();
			zA[i] = gSeed;
		}
	}

	// 9/9/97 Pitch Init for C1, relative to A5
	c1 = (55*pow(2,0.25))/64;
	log2 = log(2.0);
	pi2 = pi*2;
	exp1 = exp(1.0);
}

/* Return Non-Zero if False */
int	ExpMgr::CompileExp(SSModule *callingMod, char *exp, unsigned char *compexp, int *compileFlags)
{
	ssfloat			v;
	int			  	c;
	Boolean			noTimeFlag = false;
	// int				result;
	// short			calcMethod;

	*compileFlags = EF_IsConstant | EF_NoParams | EF_NoTime;

	cr->beginSubExpFlag = true;
	cr->sp = (unsigned char *) exp;
	cr->dp = (unsigned char *) compexp;
	cr->pCtr = 0;
	cr->opCtr[cr->pCtr] = 0;
	cr->loCtr[cr->pCtr] = 0;

	while (*cr->sp && *cr->sp != '\r' && *cr->sp != '\n') {
		if (isalpha(cr->sp[0]) || cr->sp[0] == '?' || cr->sp[0] == PICHAR) {
			if (ParseInputSignal(callingMod) == 1) {
				*compileFlags &= ~(EF_NoParams | EF_NoTime | EF_IsConstant);
				continue;
			}
			if (ParseFolderSignal() == 1) {
				*compileFlags &= ~(EF_NoParams | EF_NoTime | EF_IsConstant);
				continue;
			}
			if (ParseFunction(compileFlags) == 1) {
				continue;
			}
			if (ParseInstrumentParam() == 1) {
				*compileFlags &= ~(EF_NoParams | EF_IsConstant | EF_NoTime);
				continue;
			}
			if (ParseGlobalVar() == 1) {
				*compileFlags &= ~(EF_NoParams | EF_NoTime | EF_IsConstant);
				continue;
			}
			if (ParseVariable(compileFlags) == 1)
				continue;
		}
		if ((*cr->sp == '.' || (*cr->sp >= '0' && *cr->sp <= '9')) ||
			((*cr->sp == '-' || *cr->sp == '+') && cr->beginSubExpFlag &&
			(*(cr->sp+1) == '.' || (*(cr->sp+1) >= '0' && *(cr->sp+1) <= '9')))) {
			Boolean	negateFlag=false;
			if (*cr->sp == '-') {
				negateFlag = true;
				++cr->sp;
			}
			else if (*cr->sp == '+') {
				// 1/28/93 Ignore unary +
				++cr->sp;
			}

			cr->sp = MyAtoF(cr->sp,&v);

			/* If constant is preceded by unary negation, negate it */
			if (negateFlag)
				v = -v;

			*(cr->dp++) = 'V';
			*(cr->dp++) = '#';
			*((ssfloat *) cr->dp) = v;
			cr->dp += sizeof(ssfloat);
			ResolveSubExp();

		}
		else {

			c = *cr->sp;
			if (isalpha(c))
				c = tolower(c);

			switch (c) {
			case '+':	RecordOp(OP_Add);		++cr->sp;	break;
			case '/':	RecordOp(OP_Divide);		++cr->sp;	break;
			case '%':	RecordOp(OP_Modulo);		++cr->sp;	break;
			case '&':
				if (cr->sp[1] == '&') {
					RecordOp(OP_LogicAnd);
					++cr->sp;
				}
				else
					RecordOp(OP_And);
				++cr->sp;
				break;
			case '|':
				if (cr->sp[1] == '|') {
					RecordOp(OP_LogicOr);
					++cr->sp;
				}
				else
					RecordOp(OP_Or);
				++cr->sp;
				break;
			case '^':
				RecordOp(OP_Power);		// Was XOR
				++cr->sp;
				break;
			case '>':
				if (cr->sp[1] == '=') {
					RecordOp(OP_Gte);
					++cr->sp;
				}
				else
					RecordOp(OP_Greater);
				++cr->sp;
				break;
			case '<':
				if (cr->sp[1] == '>') {
					RecordOp(OP_Neq);
					++cr->sp;
				}
				else if (cr->sp[1] == '=') {
					RecordOp(OP_Lse);
					++cr->sp;
				}
				else
					RecordOp(OP_Less);
				++cr->sp;
				break;
			case '=':
				if (cr->sp[1] == '=') {
					RecordOp(OP_Equal);
					++cr->sp;
				}
				else
					RecordOp(OP_Equal);
				++cr->sp;
				break;

			case '~':
				RecordOp(OP_Invert);
				++cr->sp;
				break;

			case '!':
				if (cr->sp[1] == '=') {
					RecordOp(OP_Neq);
					++cr->sp;
				}
				else
					RecordOp(OP_Not);
				++cr->sp;
				break;
			case '-':
				if (cr->beginSubExpFlag)
					RecordOp(OP_Negate);
				else
					RecordOp(OP_Subtract);
				++cr->sp;
				break;
			case '*':
				if (cr->sp[1] == '*') {
					RecordOp(OP_Power);
					++cr->sp;
					++cr->sp;
				}
				else {
					RecordOp(OP_Multiply);
					++cr->sp;
				}
				break;
			case '(':
				// *(dp++) = '(';
				++cr->sp;
				if (cr->pCtr < 16-1) {
					++cr->pCtr;
					cr->opCtr[cr->pCtr] = 0;
					cr->loCtr[cr->pCtr] = 0;
				}
				else
					return -1;
				break;
			case ')':
				// *(dp++) = ')';
				++cr->sp;
				if (cr->pCtr > 0) {
					--cr->pCtr;
					ResolveSubExp();
				}
				else
					return -1;
				break;
				/* Process Parens */
			case ',':
				/* Skip over comma */
				++cr->sp;
				cr->beginSubExpFlag = true;
				break;
			case '[':
			case ']':
				++cr->sp;		//  used to hint notime
				noTimeFlag = true;
				break;
			default:
				++cr->sp;
				break;
			}
		}
	}
	*cr->dp = 0;
	if (noTimeFlag)
		*compileFlags |= EF_NoTime;
	return 0;
}


int ExpMgr::ParseInputSignal(SSModule *mod)
{
	char		varName[32];
	int			si = 0;
	int			n;
	LinkDescPtr	ld;

	while (isalnum(cr->sp[si]) || cr->sp[si] == '_') {
		varName[si] = tolower(cr->sp[si]);
		++si;
	}
	if (si == 0)
		return 0;
	varName[si] = 0;
	for (n = 0; n < mod->nbrSupportedLinks; ++n) {
		ld = mod->GetLinkDesc(n);
		if (strcmp(varName,ld->varName) == 0) {
			cr->sp += strlen(varName);
			*(cr->dp++) = 'L';
			*(cr->dp++) = n;
			ResolveSubExp();
			return 1;
		}
	}
	return 0;
}

int ExpMgr::ParseInstrumentParam()
{
	int	c;
	c = *cr->sp;
	if (isalpha(c))
		c = tolower(c);
	if (c == 'p' && isdigit(cr->sp[1])) {
		int	param = 0;
		cr->sp++;
		while (isdigit(*cr->sp)) {
			param *= 10;
			param += *cr->sp - '0';
			++cr->sp;
		}
		*(cr->dp++) = 'P';
		*(cr->dp++) = param;
		ResolveSubExp();
		return 1;
	}
	return 0;
}

int ExpMgr::ParseGlobalVar()
{
	int	c;
	c = *cr->sp;
	if (isalpha(c))
		c = tolower(c);
	if (c == 'g' && isdigit(cr->sp[1])) {
		int	param = 0;
		cr->sp++;
		while (isdigit(*cr->sp)) {
			param *= 10;
			param += *cr->sp - '0';
			++cr->sp;
		}
		*(cr->dp++) = 'G';
		*(cr->dp++) = param;
		ResolveSubExp();
		return 1;
	}
	return 0;
}

int ExpMgr::ParseFolderSignal()
{
	int	c;
	c = *cr->sp;
	if (isalpha(c))
		c = tolower(c);
	if (c == 'f' && isdigit(cr->sp[1])) {
		int	param = 0;
		cr->sp++;
		while (isdigit(*cr->sp)) {
			param *= 10;
			param += *cr->sp - '0';
			++cr->sp;
		}
		*(cr->dp++) = 'F';
		*(cr->dp++) = param;
		ResolveSubExp();
		return 1;
	}
	return 0;
}

int ExpMgr::ParseVariable(int *compileFlags)
{
	int	c,var=0;

	c = *cr->sp;
	if (isalpha(c))
		c = tolower(c);

	switch (c) {
	case 'd':	// total note duration
	case 'r':	// sample rate
	case 'e':	// exp(1)
		var = c;
		break;
	case 'g':	// global time ctr
	case 'n':	// note time ctr
	case 't':	// local time ctr
	case 'k':	// K value (used for hammer instruments key#)
	case 'i':	// I Value (used for random scores)
	case 'a':	// A Value (used for hammer instrument amplitude)
		var = c;
		*compileFlags &= ~(EF_IsConstant | EF_NoTime);
		break;
	case 'v':
		var = c;// input signal
		*compileFlags &= ~(EF_IsConstant | EF_NoParams | EF_NoTime);
		break;
	case '?':
		var = c;
		*compileFlags &= ~(EF_IsConstant | EF_NoTime);
		break;
	case PICHAR:
		var = c;
		break;
	}

	if (var) {
		*(cr->dp++) = 'V';
		cr->sp++;
		*(cr->dp++) = var;
		ResolveSubExp();
		/* Process Variable */
		return 1;
	}
	return 0;
}


int ExpMgr::ParseFunction(int *compileFlags)
{
	// Don't need to modify compileFlags unless we start using a function which
	// returns randomized (unpredictable) results, otherwise, we need only
	// check the args to the functions

	int		opType = 0;
	char	fName[32];
	int		si = 0;

	while (isalnum(cr->sp[si]) || cr->sp[si] == '_') {
		fName[si] = tolower(cr->sp[si]);
		++si;
	}
	if (si == 0)
		return 0;
	fName[si] = 0;

	if (strcmp("int",fName) == 0)			opType = OP_Integer;
	else if (strcmp("sqrt",fName) == 0)		opType = OP_Sqrt;
	else if (strcmp("log",fName) == 0)		opType = OP_Log;
	else if (strcmp("log10",fName) == 0)	opType = OP_Log10;
	else if (strcmp("abs",fName) == 0)		opType = OP_Fabs;
	else if (strcmp("fabs",fName) == 0)		opType = OP_Fabs;
	else if (strcmp("sign",fName) == 0)		opType = OP_Sign;
	else if (strcmp("exp",fName) == 0)		opType = OP_Exp;
	else if (strcmp("cos",fName) == 0)		opType = OP_Cos;
	else if (strcmp("sin",fName) == 0)		opType = OP_Sin;
	else if (strcmp("tan",fName) == 0)		opType = OP_Tan;
	else if (strcmp("acos",fName) == 0)		opType = OP_ACos;
	else if (strcmp("asin",fName) == 0)		opType = OP_ASin;
	else if (strcmp("atan",fName) == 0)		opType = OP_ATan;
	else if (strcmp("cosh",fName) == 0)		opType = OP_Cosh;
	else if (strcmp("sinh",fName) == 0)		opType = OP_Sinh;
	else if (strcmp("tanh",fName) == 0)		opType = OP_Tanh;
	else if (strcmp("sin2",fName) == 0)		opType = OP_Sin2;
	else if (strcmp("cos2",fName) == 0)		opType = OP_Cos2;
	else if (strcmp("sin3",fName) == 0)		opType = OP_Sin3;
	else if (strcmp("cos3",fName) == 0)		opType = OP_Cos3;
	else if (strcmp("mand",fName) == 0 ||
			 strcmp("mandel",fName) == 0)	opType = OP_Mandel;
	else if (strcmp("man3",fName) == 0)		opType = OP_Mandel3;
	else if (strcmp("manc",fName) == 0)		opType = OP_MandelCPM;
	else if (strcmp("mcpm",fName) == 0)		opType = OP_MandelCPM;
	else if (strcmp("julia",fName) == 0)	opType = OP_Julia;
	else if (strcmp("dragon",fName) == 0)	opType = OP_Dragon;
	else if (strcmp("fbm",fName) == 0)		opType = OP_FBM;
	else if (strcmp("gturb3d",fName) == 0)	opType = OP_GTurb3D;
	else if (strcmp("gnoise3d",fName) == 0)	opType = OP_GNoise3D;
	else if (strcmp("fib",fName) == 0)		opType = OP_Fib;
	else if (strcmp("prime",fName) == 0)	opType = OP_Prime;

	else if (strcmp("cpspch",fName) == 0)	opType = OP_cpspch;
	else if (strcmp("cpsoct",fName) == 0)	opType = OP_cpsoct;
	else if (strcmp("octpch",fName) == 0)	opType = OP_octpch;
	else if (strcmp("octcps",fName) == 0)	opType = OP_octcps;
	else if (strcmp("pchoct",fName) == 0)	opType = OP_pchoct;
	else if (strcmp("octmidi",fName) == 0)	opType = OP_octmidi;
	else if (strcmp("cpsmidi",fName) == 0)	opType = OP_cpsmidi;

	else if (strcmp("noise",fName) == 0)	opType = OP_Noise;
	else if (strcmp("noise3d",fName) == 0)	opType = OP_Noise3D;
	else if (strcmp("turb",fName) == 0)		opType = OP_Turb;
	else if (strcmp("turb3d",fName) == 0)	opType = OP_Turb3D;
	else if (strcmp("cond",fName) == 0)		opType = OP_Cond;
	else if (strcmp("angle",fName) == 0)	opType = OP_Angle;
	else if (strcmp("ftab",fName) == 0)		opType = OP_FTab;
	else if (strcmp("ftabw",fName) == 0)	opType = OP_FTabW;
	else if (strcmp("ftabp",fName) == 0)	opType = OP_FTabP;
	else if (strcmp("ftabi",fName) == 0)	opType = OP_FTabI;
	else if (strcmp("ftabiw",fName) == 0)	opType = OP_FTabIW;
	else if (strcmp("ftabip",fName) == 0)	opType = OP_FTabIP;
	else if (strcmp("linen",fName) == 0)	opType = OP_linen;
	else if (strcmp("linenr",fName) == 0)	opType = OP_linenr;
	else if (strcmp("limit",fName) == 0)	opType = OP_limit;
	else if (strcmp("mod",fName) == 0)		opType = OP_Modulo;
	else if (strcmp("xor",fName) == 0)		opType = OP_Xor;
	else if (strcmp("and",fName) == 0)		opType = OP_And;
	else if (strcmp("or",fName) == 0)		opType = OP_Or;
	else if (strcmp("not",fName) == 0)		opType = OP_Not;
	else if (strcmp("pow2",fName) == 0)		opType = OP_Power2;
	else if (strcmp("pow",fName) == 0)		opType = OP_Power;
	else if (strcmp("dist",fName) == 0 ||
			 strcmp("distance",fName) == 0)	opType = OP_Dist;
	else if (strcmp("pi",fName) == 0)
	{
		*(cr->dp++) = 'V';
		cr->sp += 2;
		*(cr->dp++) = PICHAR;
		ResolveSubExp();
		return 1;
	}
	else if (strcmp("pi2",fName) == 0)
	{
		*(cr->dp++) = 'V';
		cr->sp += 3;
		*(cr->dp++) = PICHAR+1;
		ResolveSubExp();
		return 1;
	}
	else
		return 0;
	RecordOp(opType);
	cr->sp += strlen(fName);
	return 1;
}


void ExpMgr::RecordOp(short newOp)
{
	// If we encounter x+y*z, we grab back the + operator and put it back on the
	// stack.
	// 1/7/92 Don't do this if newOp is a prefix operator or function.
	while (cr->loCtr[cr->pCtr] > 0 &&
		   cr->lastOpStack[cr->pCtr][cr->loCtr[cr->pCtr]-1] == (cr->dp - 1) &&
		   opData[*(cr->dp-1)].precedence > opData[newOp].precedence &&
		   !opData[newOp].prefixFlag)
	{
		cr->opStack[cr->pCtr][cr->opCtr[cr->pCtr]] = *(cr->dp-1);
		++cr->opCtr[cr->pCtr];
		cr->dp -= 2;
		--cr->loCtr[cr->pCtr];
	}
	cr->opStack[cr->pCtr][cr->opCtr[cr->pCtr]] = newOp;
	++cr->opCtr[cr->pCtr];
	cr->beginSubExpFlag = true;
}

void ExpMgr::ResolveSubExp()
{
	while (cr->opCtr[cr->pCtr] > 0) {
		*(cr->dp++) = 'O';

		--cr->opCtr[cr->pCtr];
		cr->lastOpStack[cr->pCtr][cr->loCtr[cr->pCtr]] = cr->dp;
		cr->loCtr[cr->pCtr]++;
		*(cr->dp++) = (unsigned char) (cr->opStack[cr->pCtr][cr->opCtr[cr->pCtr]]);
	}
	cr->beginSubExpFlag = false;
}

unsigned char *ExpMgr::MyAtoF(unsigned char *sp, ssfloat *rv)
{
	Boolean decimal=0,negate=0;
	ssfloat	div,v;
	v = myNIL;

	// 1/11
	if (*sp == '-') {
		negate = true;
		++sp;
	}
	while (*sp == '.' || (*sp >= '0' && *sp <= '9')) {
		if (*sp == '.') {
			decimal = 1;
			div = 1;
		}
		else {
			if (decimal) {
				div *= 10;
				v += (*sp-'0') / div;
			}
			else {
				v *= 10;
				v += (*sp-'0');
			}
		}
		++sp;
	}
	if (negate)
		v = -v;
	*rv = v;
	return sp;
}

#define PopF()		stackF[--stackCtr]

#define PushF(v)	stackF[stackCtr++] = (v)

#define PopL()		TruncXToLong(PopF());

// Note: This function must be re-entrant!!!

ssfloat ExpMgr::EvalF(SSModule *callingModule, unsigned char *compexp)
{
	register unsigned char *ep = compexp;
	register ssfloat		v1,v2,v3;
	long				lv1,lv2;
	int					stackCtr;
	ssfloat				stackF[32];
	long				lFlags = 0L,fFlags = 0L;
	ssfloat				lSigs[32],fSigs[32];
	register ssfloat	pTime = callingModule->parList->pTime;

	// 11/6 initialize stack ctr to 0
	stackCtr = 0;

	while (*ep) {
		switch (*(ep++)) {
		case 'L':	// Inputs to Module (sig, am, fm etc.)
			{
				int	linkNbr = *(ep++);
				if (callingModule) {
					if ((lFlags & (1L << linkNbr)) > 0) {
						PushF(lSigs[linkNbr]);
					}
					else {
						ssfloat sig = callingModule->MixInputs(linkNbr,callingModule->callingMod);
						lSigs[linkNbr] = sig;
						lFlags |= (1L << linkNbr);
						PushF(sig);
					}
				}
				else
					PushF(0.0);
			}
			break;
		case 'P':	// Instrument Parameters (Pn)
			{
				int	paramNbr = *(ep++);
				if (callingModule && callingModule->callingMod)
					PushF(callingModule->callingMod->GetInstParameter(paramNbr));
				else
					PushF(0.0);
			}
			break;
		case 'G':	// Global Variable (Gn)
			{
				int	paramNbr = *(ep++);
				if (callingModule && callingModule->parList && callingModule->parList->itsOwner)
					PushF(callingModule->parList->itsOwner->RetrieveGlobal(paramNbr));
				else
					PushF(0.0);
			}
			break;
		case 'F':	// Folder Input Signals (Fn)
			{
				int	paramNbr = *(ep++);
				ssfloat	sig = 0.0;
				if (callingModule) {
					if ((fFlags & (1L << paramNbr)) > 0) {
						PushF(fSigs[paramNbr]);
					}
					else {
						callingModule->parList->mods[0]->GetFolderSig(paramNbr, &sig);
						fFlags |= (1L << paramNbr);
						fSigs[paramNbr] = sig;
					}
				}
				PushF(sig);
			}
			break;
		case 'O':
			switch (*(ep++)) {
			case OP_Add:
				v1 = PopF();
				v2 = PopF();
				PushF(v1+v2);
				break;
			case OP_Subtract:
				v2 = PopF();
				v1 = PopF();
				PushF(v1-v2);
				break;
			case OP_Negate:
				v1 = PopF();
				PushF(-v1);
				break;
			case OP_Multiply:
				v1 = PopF();
				v2 = PopF();
				PushF(v1*v2);
				break;
			case OP_Divide:
				v2 = PopF();
				v1 = PopF();
				PushF(v2 == 0.0? HUGE_VAL : (v1/v2));
				break;
			case OP_Modulo:
				lv2 = PopL();
				lv1 = PopL();
				// 9/21/92
				PushF(lv2 == 0.0? 0.0 : (lv1%lv2));
				break;
			case OP_Power:
				v2 = PopF();
				v1 = PopF();
				PushF(pow(v1,v2));
				break;
			case OP_Power2:
				v1 = PopF();
				PushF(pow(2.0,v1));
				break;
			case OP_Integer:
				lv1 = PopL();
				PushF(lv1);
				break;
			case OP_And:
				lv1 = PopL();
				lv2 = PopL();
				PushF(lv1&lv2);
				break;
			case OP_Or:
				lv1 = PopL();
				lv2 = PopL();
				PushF(lv1|lv2);
				break;
			case OP_Xor:
				lv1 = PopL();
				lv2 = PopL();
				PushF(lv1^lv2);
				break;
			case OP_Invert:
				lv1 = PopL();
				PushF(~lv1);
				break;
			case OP_Equal:
				v2 = PopF();
				v1 = PopF();
				PushF(v1==v2? 1.0 : 0.0);
				break;
			case OP_Neq:
				v2 = PopF();
				v1 = PopF();
				PushF(v1==v2? 0.0 : 1.0);
				break;
			case OP_Greater:
				v2 = PopF();
				v1 = PopF();
				PushF(v1>v2? 1.0 : 0.0);
				break;
			case OP_Gte:
				v2 = PopF();
				v1 = PopF();
				PushF(v1>=v2? 1.0 : 0.0);
				break;
			case OP_Less:
				v2 = PopF();
				v1 = PopF();
				PushF(v1<v2? 1.0 : 0.0);
				break;
			case OP_Lse:
				v2 = PopF();
				v1 = PopF();
				PushF(v1<=v2? 1.0 : 0.0);
				break;
			case OP_Not:
				v1 = PopF();
				PushF(v1==0? 1.0 : 0.0);
				break;
			case OP_LogicAnd:
				v2 = PopF();
				v1 = PopF();
				PushF((v1 != 0.0 && v2 != 0.0)? 1.0 : 0.0);
				break;
			case OP_LogicOr:
				v2 = PopF();
				v1 = PopF();
				PushF((v1 != 0.0 || v2 != 0.0)? 1.0 : 0.0);
				break;
			case OP_Sqrt:
				v1 = PopF();
				PushF(sqrt(v1));
				break;
			case OP_Mandel:
				v2 = PopF();
				v1 = PopF();
				PushF(CalcMandel(v1,v2));
				break;
			case OP_MandelCPM:
				v2 = PopF();
				v1 = PopF();
				PushF(CalcMandelCPM(v1,v2));
				break;
			case OP_Mandel3:
				v2 = PopF();
				v1 = PopF();
				PushF(CalcMandel3(v1,v2));
				break;
			case OP_Julia:
				{
					ssfloat v3,v4;
					v4 = PopF();
					v3 = PopF();
					v2 = PopF();
					v1 = PopF();
					PushF(CalcJulia(v1,v2,v3,v4));
				}
				break;
			case OP_Dragon:
				{
					ssfloat v3,v4;
					v4 = PopF();
					v3 = PopF();
					v2 = PopF();
					v1 = PopF();
					PushF(CalcDragon(v1,v2,v3,v4));
				}
				break;
			case OP_FBM:
				{
					ssfloat v3,v4,v5,v6;
					v6 = PopF();
					v5 = PopF();
					v4 = PopF();
					v3 = PopF();
					v2 = PopF();
					v1 = PopF();
					PushF(fBm(v1,v2,v3,v4,v5,v6));
				}
				break;
			case OP_Fib:
				lv1 = PopL();
				PushF(Fibonacci(lv1));
				break;
			case OP_Prime:
				lv1 = PopL();
				PushF(isPrime(lv1));
				break;
			case OP_cpspch:
				v1 = PopF();
				PushF(cpspch(v1));
				break;
			case OP_cpsoct:
				v1 = PopF();
				PushF(cpsoct(v1));
				break;
			case OP_octpch:
				v1 = PopF();
				PushF(octpch(v1));
				break;
			case OP_octcps:
				v1 = PopF();
				PushF(octcps(v1));
				break;
			case OP_pchoct:
				v1 = PopF();
				PushF(pchoct(v1));
				break;
			case OP_octmidi:
				v1 = PopF();
				PushF(octmidi(v1));
				break;
			case OP_cpsmidi:
				v1 = PopF();
				PushF(cpsmidi(v1));
				break;
			case OP_Noise:
				v2 = PopF();
				v1 = PopF();
				PushF(Noise(v1,v2));
				break;
			case OP_Turb:
				v2 = PopF();
				v1 = PopF();
				PushF(Turbulence(v1,v2));
				break;
			case OP_Noise3D:
				v3 = PopF();
				v2 = PopF();
				v1 = PopF();
				PushF(Noise3D(v1,v2,v3));
				break;
			case OP_GNoise3D:
				v3 = PopF();
				v2 = PopF();
				v1 = PopF();
				PushF(gnoise(v1,v2,v3));
				break;
			case OP_Turb3D:
				v3 = PopF();
				v2 = PopF();
				v1 = PopF();
				PushF(Turbulence3D(v1,v2,v3));
				break;
			case OP_GTurb3D:
				v3 = PopF();
				v2 = PopF();
				v1 = PopF();
				PushF(GTurbulence3D(v1,v2,v3));
				break;
			case OP_Angle:
				v2 = PopF();
				v1 = PopF();
				PushF(Angle(v1,v2));
				break;
			case OP_Dist:
				v2 = PopF();
				v1 = PopF();
				PushF(Distance(v1,v2));
				break;
			case OP_Cond:
				{
					ssfloat	v3;
					v3 = PopF();
					v2 = PopF();
					v1 = PopF();
					if (v1 != 0.0)
						PushF(v2);
					else
						PushF(v3);
				}
				break;
			case OP_Cos:
				v1 = PopF();
				PushF(cos(v1));
				break;
			case OP_Sin:
				v1 = PopF();
				PushF(sin(v1));
				break;
			case OP_Sin2:
				v1 = PopF();
				v1 = sin(v1);
				PushF(v1*v1);
				break;
			case OP_Cos2:
				v1 = PopF();
				v1 = cos(v1);
				PushF(v1*v1);
				break;
			case OP_Sin3:
				v1 = PopF();
				v1 = sin(v1);
				PushF(v1*v1*v1);
				break;
			case OP_Cos3:
				v1 = PopF();
				v1 = cos(v1);
				PushF(v1*v1*v1);
				break;
			case OP_FTab:
				v2 = PopF();
				v1 = PopF();
				PushF(GetFTableEntry(callingModule,(int) v1, v2,FW_NoWrap));
				break;
			case OP_FTabW:
				v2 = PopF();
				v1 = PopF();
				PushF(GetFTableEntry(callingModule,(int) v1, v2,FW_Wrap));
				break;
			case OP_FTabP:
				v2 = PopF();
				v1 = PopF();
				PushF(GetFTableEntry(callingModule,(int) v1, v2,FW_Pin));
				break;
			case OP_FTabI:
				v2 = PopF();
				v1 = PopF();
				PushF(GetFTableEntry(callingModule,(int) v1, v2,FW_NoWrap|FW_Interp));
				break;
			case OP_FTabIW:
				v2 = PopF();
				v1 = PopF();
				PushF(GetFTableEntry(callingModule,(int) v1, v2,FW_Wrap|FW_Interp));
				break;
			case OP_FTabIP:
				v2 = PopF();
				v1 = PopF();
				PushF(GetFTableEntry(callingModule,(int) v1, v2,FW_Pin|FW_Interp));
				break;
			case OP_linen:
				{
					ssfloat	v4;
					v4 = PopF();
					v3 = PopF();
					v2 = PopF();
					v1 = PopF();
					PushF(linen(v1,v2,v3,v4));
				}
				break;
			case OP_linenr:
				{
					ssfloat	v4;
					v4 = PopF();
					v3 = PopF();
					v2 = PopF();
					v1 = PopF();
					PushF(linenr(v1,v2,v3,v4));
				}
				break;
			case OP_limit:
				v3 = PopF();
				v2 = PopF();
				v1 = PopF();
				PushF(limit(v1,v2,v3));
				break;
			case OP_Log:
				v1 = PopF();
				PushF(log(v1));
				break;
			case OP_Log10:
				v1 = PopF();
				PushF(log10(v1));
				break;
			case OP_Fabs:
				v1 = PopF();
				PushF(fabs(v1));
				break;
			case OP_Sign:
				v1 = PopF();
				PushF(v1 < 0? -1.0 : 1.0);
				break;
			case OP_Exp:
				v1 = PopF();
				PushF(exp(v1));	// Mac version used expf??
				break;
			case OP_Tan:
				v1 = PopF();
				PushF(tan(v1));
				break;
			case OP_ACos:
				v1 = PopF();
				PushF(acos(v1));
				break;
			case OP_ASin:
				v1 = PopF();
				PushF(asin(v1));
				break;
			case OP_ATan:
				v1 = PopF();
				PushF(atan(v1));
				break;
			case OP_Cosh:
				v1 = PopF();
				PushF(cosh(v1));
				break;
			case OP_Sinh:
				v1 = PopF();
				PushF(sinh(v1));
				break;
			case OP_Tanh:
				v1 = PopF();
				PushF(tanh(v1));
				break;
/* example input function
			case OP_Red:
				{
					unsigned char *pPtr;
					ssfloat			l;
					lv2 = PopL();
					lv1 = PopL();
					pPtr = GetPixelOffset(lv1,lv2);
					if (gStuff->planes == 3 || gStuff->planes == 4)
						l = (unsigned short) pPtr[0];
					else
						l = (unsigned short) pPtr[0];

					PushF(l);
				}
				break;
*/
			}
			break;
		case 'V':
			switch (*(ep++)) {
			case '#':	// preserve even alignment
				// 10/26/92 moved up here in an attempt to speed up
				v1 = *((ssfloat *) ep);
				ep += sizeof(ssfloat);
				PushF(v1);
				break;
			case 't':
				PushF(pTime);
				break;
			case 'd':
				if (callingModule)
					PushF(callingModule->parList->itsOwner->mainInst->sampleDuration);
#if macintosh
				else
					PushF(mw->mainInst->sampleDuration);
#endif
				break;
			case 'v':
				PushF(callingModule->MixInputs(-1,callingModule->callingMod));
				break;
			case 'g':
				if (callingModule)
					PushF(callingModule->parList->itsOwner->gTime);
#if macintosh
				else
					PushF(mw->gTime);
#endif
				break;
			case 'n':
				PushF(callingModule->GetNoteTime());
				break;
			case 'k':
				PushF(callingModule->GetKeyValue());
				break;
			case 'i':
				PushF(callingModule->GetIValue());
				break;
			case 'a':
				PushF(callingModule->GetAValue());
				break;
			case 'r':
				if (callingModule)
					PushF(callingModule->parList->itsOwner->mainInst->sampleRate);
#if macintosh
				else
					PushF(mw->mainInst->sampleRate);
#endif
				break;
			case 'e':
				PushF(exp1);
				break;
			case '?':
				// 9/15 Better random number
				// 1/24/93 Even Better
				PushF(DoubleRandom());
				break;
			case PICHAR:
				// 10/26/92 Added PI support
				PushF(pi);
				break;
			case PICHAR+1:
				// 10/26/92 Added PI support
				PushF(pi2);
				break;
				// 1/24/93 Added Center X and Center Y
			}
			break;
		}
		/* break; */
	}
	if (stackCtr > 0)
		return PopF();
	else
		return 0.0;
}

