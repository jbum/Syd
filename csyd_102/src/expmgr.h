// ExpMgr.h
#ifndef _H_ExpMgr
#define _H_ExpMgr	1

#include "ss.h"
#include "ssmodule.h"


#define LenCompiledExp		2058

#define MaxParenNesting		16
#define MaxSubExpressions	16
#define PICHAR				0xb9
#define TruncXToLong(d)	((long) (d))

// Flags returned by compiler
#define EF_IsConstant		1
#define EF_NoParams			2
#define EF_NoTime			4
#define EF_NeedsSolving		8

// Noise Stuff

#define TABSIZE          256
#define TABMASK          (TABSIZE-1)
#define PERM(x)          perm[(x)&TABMASK]
#define INDEX(ix,iy,iz)  PERM((ix)+PERM((iy)+PERM(iz)))

// #define RANDMASK  0x7fff
#define FLOOR(x) ((int)(x) - ((x) < 0 && (x) != (int)(x)))
#define CEIL(x) ((int)(x) + ((x) > 0 && (x) != (int)(x)))
#define CLAMP(x,a,b) ((x) =< (a) ? (a) : ((x) >= (b) ? (b) : (x)))
#define LERP(t,x0,x1)  ((x0) + (t)*((x1)-(x0)))

#define PULSE(a,b,x) (step((a),(x)) - step((b),(x)))
#define boxstep(a,b,x) clamp(((x)-(a))/((b)-(a)),0,1)

extern unsigned char perm[TABSIZE];	/* see perm.c */

extern double catrom2(double d);		/* see catrom2.c */


typedef struct {
	short			opStack[MaxParenNesting][MaxSubExpressions];
	short			opCtr[MaxParenNesting];		// Operator Eval Counter
	short			loCtr[MaxParenNesting];		// Last Op Counter	
	short			pCtr;						// Paren Counter
	unsigned char 	*sp,*dp;
	unsigned char 	*lastOpStack[MaxParenNesting][MaxSubExpressions];
	bool			beginSubExpFlag;
} CompileVars;


class ExpMgr 
{
	CompileVars		*cr;	// Used only by compiler

	// Fractal stuff - used by EvalF
	int				lastMandelVal;
	ssfloat			lastMandelCPM;
	ssfloat			lastMandelX,lastMandelY;
	int				recur;

	// Noise stuff - used by EvalF
	long			xA[256],yA[256],zA[256];

	CompileVars		gCV;
	ssfloat			myNIL;
	ssfloat			c1;		// base frequency for pitch->cps calculations
	ssfloat			log2;	// log(2.0)
	ssfloat			pi2;	// pi*2
	ssfloat			exp1;	// E

public:
	// Overrides
	ExpMgr();

	// Expression Execution
	ssfloat	EvalF(SSModule *callingModule, unsigned char *cExp);

	// Expression Compiling
	int		CompileExp(SSModule *callingModule, char *exp, unsigned char *cExp, int *compileFlags);
	int		ParseFunction(int *compileFlags);
	int		ParseVariable(int *compileFlags);
	int		ParseInputSignal(SSModule *mod);
	int		ParseFolderSignal();
	int		ParseInstrumentParam();
	int		ParseGlobalVar();
	void	RecordOp(short newOp);
	void	ResolveSubExp();
	unsigned char *MyAtoF(unsigned char *sp, ssfloat *rv);

	// Functions
	int	CalcMandel(ssfloat p0, ssfloat q0);
	ssfloat	CalcMandelCPM(ssfloat p0, ssfloat q0);
	int	CalcDragon(ssfloat x0, ssfloat y0, ssfloat p0, ssfloat q0);
	int	CalcMandel3(ssfloat p0, ssfloat q0);
	int	CalcJulia(ssfloat x, ssfloat y, ssfloat p0, ssfloat q0);
	ssfloat RandPoint(short x, short y);
	ssfloat RandPoint3D(short x, short y, short z);
	ssfloat Noise(ssfloat x, ssfloat y);
	ssfloat Noise3D(ssfloat x, ssfloat y, ssfloat z);
	ssfloat Turbulence(ssfloat x, ssfloat y);
	ssfloat Turbulence3D(ssfloat x, ssfloat y, ssfloat z);
	ssfloat Angle(ssfloat xd, ssfloat yd);
	ssfloat Distance(ssfloat xd, ssfloat yd);
	long Fibonacci(long x);
	long isPrime(long n);
	ssfloat fBm(ssfloat x, ssfloat y, ssfloat z, 
		 ssfloat H, ssfloat lacunarity, ssfloat octaves);
	ssfloat GTurbulence3D(ssfloat x, ssfloat y, ssfloat z);
	ssfloat gnoise(ssfloat x, ssfloat y, ssfloat z);

	// Pitch Converters
	ssfloat cpspch(ssfloat pch);
	ssfloat cpsoct(ssfloat oct);
	ssfloat pchoct(ssfloat oct);
	ssfloat octpch(ssfloat pch);
	ssfloat octcps(ssfloat cps);
	ssfloat octmidi(ssfloat cps);
	ssfloat cpsmidi(ssfloat cps);

	// Music functions
	ssfloat linen(ssfloat t, ssfloat atk, ssfloat dur, ssfloat dcy);
	ssfloat linenr(ssfloat t, ssfloat atk, ssfloat dcy, ssfloat atdec);
	ssfloat limit(ssfloat v, ssfloat low, ssfloat high);
};

enum { PinMethod, WrapMethod};
enum { RGBMode=0x0000, GrayscaleMode=0x0100, HSBMode=0x0200 };

enum Operators 
	 {OP_None, 
	  OP_Integer, OP_Sqrt,OP_Log,OP_Log10,OP_Fabs,OP_Sign,OP_Exp,
	  OP_Cos,OP_Sin,OP_Tan,
	  OP_ACos,OP_ASin,OP_ATan,
	  OP_Cosh,OP_Sinh,OP_Tanh,
	  OP_Sin2, OP_Cos2, OP_Sin3, OP_Cos3,
	  OP_Mandel, OP_MandelCPM, OP_Mandel3, OP_Julia, OP_Noise, OP_Turb, 
	  OP_Noise3D, OP_Turb3D, OP_Dragon,
	  OP_FBM, OP_GTurb3D, OP_GNoise3D,
	  OP_Cond, OP_Angle, OP_Dist, OP_Fib, OP_Prime,
	  OP_cpspch,
	  OP_cpsoct,
	  OP_octpch,
	  OP_octcps,
	  OP_pchoct,
	  OP_octmidi,
	  OP_cpsmidi,
	  OP_FTab, OP_FTabW, OP_FTabP,
	  OP_FTabI, OP_FTabIW, OP_FTabIP,
	  OP_linen, OP_linenr, OP_limit,
	  // OP_Lum, OP_Red, OP_Green, OP_Blue, OP_Color, OP_Saturation,
	  OP_Negate, OP_Not, OP_Invert,
	  OP_Power, OP_Power2, 
	  OP_Multiply, 
	  OP_Divide, 
	  OP_Modulo,
	  OP_Add, OP_Subtract, 
	  OP_And, OP_Or, OP_Xor,
	  OP_Equal, OP_Greater, OP_Less, OP_Gte, OP_Lse, OP_Neq,
	  OP_LogicAnd, OP_LogicOr,
	  NbrOperators};

typedef struct  {
	char	type;
	char	precedence;
	char	prefixFlag;
} OpData;

#endif

