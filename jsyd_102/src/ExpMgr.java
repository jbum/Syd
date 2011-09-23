import java.io.*;

public class ExpMgr
{

  static final int EF_IsConstant      = 1;
  static final int EF_NoParams        = 2;
  static final int EF_NoTime          = 4;
  static final int EF_NeedsSolving    = 8;

  static final int LenCompiledExp     = 2058;
  static final int MaxFloats          = 32; // max floats in expression
  static final int MaxParenNesting    = 16;
  static final int MaxSubExpressions  = 16;
  static final char PICHAR            = 0xb9;
  static final char MAJORCHAR         = 0x81;
  static final char MINORCHAR         = 0x82;
  static final char MINORHCHAR        = 0x83;

  static final double  c1 = (55*Math.pow(2,0.25))/64;
  static final double  log2 = Math.log(2);
  static final double  pi2 = Math.PI * 2;
  static final double  exp1 = Math.E;
  static final double  myNil = 0.0;

  // evaluation stuff
  int                  stackCtr;
  double[]             stackF = new double[32];
  double[]             lSigs = new double[32];
  double[]             fSigs = new double[32];

  static final int      OP_None           = 0,
                        OP_Integer        = 1,
                        OP_Sqrt           = 2,
                        OP_Log            = 3,
                        OP_Log10          = 4,
                        OP_Fabs           = 5,
                        OP_Sign           = 6,
                        OP_Exp            = 7,
                        OP_Cos            = 8,
                            OP_Sin        = 9,
                            OP_Tan        = 10,
                        OP_ACos           = 11,
                            OP_ASin       = 12,
                            OP_ATan       = 13,
                        OP_Cosh           = 14,
                            OP_Sinh       = 15,
                            OP_Tanh       = 16,
                        OP_Sin2           = 17,
                            OP_Cos2       = 18,
                            OP_Sin3       = 19,
                            OP_Cos3       = 20,
                        OP_Mandel         = 21,
                            OP_MandelCPM  = 22,
                            OP_Mandel3    = 23,
                            OP_Julia      = 24,
                            OP_Noise      = 25,
                            OP_Turb       = 26,
                        OP_Noise3D        = 27,
                            OP_Turb3D     = 28,
                            OP_Dragon     = 29,
                        OP_FBM            = 30,
                            OP_GTurb3D    = 31,
                            OP_GNoise3D   = 32,
                        OP_Cond           = 33,
                            OP_Angle      = 34,
                            OP_Dist       = 35,
                            OP_Fib        = 36,
                            OP_Prime      = 37,
                        OP_cpspch         = 38,
                        OP_cpsoct         = 39,
                        OP_octpch         = 40,
                        OP_octcps         = 41,
                        OP_pchoct         = 42,
                        OP_octmidi        = 43,
                        OP_cpsmidi        = 44,
                        OP_FTab           = 45,
                            OP_FTabW      = 46,
                            OP_FTabP      = 47,
                        OP_FTabI          = 48,
                            OP_FTabIW     = 49,
                            OP_FTabIP     = 50,
                        OP_linen          = 51,
                            OP_linenr     = 52,
                            OP_limit      = 53,
                        OP_Negate         = 54,
                            OP_Not        = 55,
                            OP_Invert     = 56,
                        OP_Power          = 57,
                            OP_Power2     = 58,
                        OP_Multiply       = 59,
                        OP_Divide         = 60,
                        OP_Modulo         = 61,
                        OP_Add            = 62,
                            OP_Subtract   = 63,
                        OP_And            = 64,
                            OP_Or         = 65,
                            OP_Xor        = 66,
                        OP_Equal          = 67,
                            OP_Greater    = 68,
                            OP_Less       = 69,
                            OP_Gte        = 70,
                            OP_Lse        = 71,
                            OP_Neq        = 72,
                        OP_LogicAnd       = 73,
                          OP_LogicOr      = 74,
                        OP_SNote          = 75,
                        OP_FNote          = 76,
                      NbrOperators        = 77;

  CompileVars      gCV, cr;

int[][] opData = // operator/function precedence table
{            // prec  prefix
  {OP_None,       0,  0},
  {OP_Integer,    0,  1},
  {OP_Sqrt,       0,  1},
  {OP_Log,        0,  1},
  {OP_Log10,      0,  1},
  {OP_Fabs,       0,  1},
  {OP_Sign,       0,  1},
  {OP_Exp,        0,  1},
  {OP_Cos,        0,  1},
  {OP_Sin,        0,  1},
  {OP_Tan,        0,  1},
  {OP_ACos,       0,  1},
  {OP_ASin,       0,  1},
  {OP_ATan,       0,  1},
  {OP_Cosh,       0,  1},
  {OP_Sinh,       0,  1},
  {OP_Tanh,       0,  1},
  {OP_Sin2,       0,  1},
  {OP_Cos2,       0,  1},
  {OP_Sin3,       0,  1},
  {OP_Cos3,       0,  1},
  {OP_Mandel,     0,  1},
  {OP_MandelCPM,  0,  1},
  {OP_Mandel3,    0,  1},
  {OP_Julia,      0,  1},
  {OP_Noise,      0,  1},
  {OP_Turb,       0,  1},
  {OP_Noise3D,    0,  1},
  {OP_Turb3D,     0,  1},
  {OP_Dragon,     0,  1},
  {OP_Cond,       0,  1},
  {OP_Angle,      0,  1},
  {OP_Dist,       0,  1},
  {OP_Fib,        0,  1},
  {OP_Prime,      0,  1},
  {OP_cpspch,     0,  1},
  {OP_cpsoct,     0,  1},
  {OP_octpch,     0,  1},
  {OP_octcps,     0,  1},
  {OP_pchoct,     0,  1},
  {OP_octmidi,    0,  1},
  {OP_cpsmidi,    0,  1},
  {OP_FTab,       0,  1},
  {OP_FTabW,      0,  1},
  {OP_FTabP,      0,  1},
  {OP_FTabI,      0,  1},
  {OP_FTabIW,     0,  1},
  {OP_FTabIP,     0,  1},
  {OP_linen,      0,  1},
  {OP_linenr,     0,  1},
  {OP_limit,      0,  1},
//  OP_Lum,       0,  1},
//  OP_Red,       0,  1},
//  OP_Green,     0,  1},
//  OP_Blue,      0,  1},
//  OP_Color,     0,  1},
//  OP_Saturation,  0,  1},
  {OP_Negate,     1,  1},
  {OP_Not,        1,  1},
  {OP_Invert,     1,  1},
  {OP_Power,      2,  0},
  {OP_Power2,     2,  0},
  {OP_Multiply,   3,  0},
  {OP_Divide,     4,  0},
  {OP_Modulo,     5,  0},
  {OP_Add,        6,  0},
  {OP_Subtract,   6,  0},
  {OP_And,        7,  0},
  {OP_Or,         7,  0},
  {OP_Xor,        7,  0},
  {OP_Equal,      8,  0},
  {OP_Greater,    9,  0},
  {OP_Less,       9,  0},
  {OP_Gte,        9,  0},
  {OP_Lse,        9,  0},
  {OP_Neq,        9, 0},
  {OP_LogicAnd,   10, 0},
  {OP_LogicOr,    10, 0},
  {OP_SNote,     0, 1},
  {OP_FNote,     0, 1}
};

  // Fractal Stuff
  int       lastMandelVal = 0;
  double    lastMandelCPM = 0;
  double    lastMandelX = -2,lastMandelY = -2;
  int       recur = 64;

  // Noise stuff
  int       xA[] = new int[256],
            yA[] = new int[256],
            zA[] = new int[256];


  public ExpMgr()
  {
    gCV = new CompileVars();
    cr = gCV;
    for (int i = 0; i < 256; ++i)
    {
      DoubleRandom();
      xA[i] = GetSRand();
      DoubleRandom();
      yA[i] = GetSRand();
      DoubleRandom();
      zA[i] = GetSRand();

    }
  }

  boolean isalpha(char b)
  {
     return (b >= 'A' && b <= 'Z') ||
            (b >= 'a' && b <= 'z');
  }

  boolean isalnum(char b)
  {
     return (b >= 'A' && b <= 'Z') ||
            (b >= 'a' && b <= 'z') ||
            (b >= '0' && b <= '9');
  }

  boolean isdigit(char b)
  {
     return (b >= '0' && b <= '9');
  }

  char tolower(char b)
  {
    if (b >= 'A' && b <= 'Z')
      return (char) (b + ('a' - 'A'));
    else
      return b;
  }

  public int CompileExp(SSModule callingMod, ExpRec exp)
  {
    double      v;
    boolean     noTimeFlag = false;

    exp.cFlags = EF_IsConstant | EF_NoParams | EF_NoTime;
    cr.beginSubExpFlag = true;
    cr.spChars = (exp.exp + "\0").toCharArray();
    cr.spIdx = 0; // index into (unsigned char *) exp;
    cr.dpChars = new char[ExpMgr.LenCompiledExp];
    cr.dpIdx = 0;
    cr.dpFloats = new double[ExpMgr.MaxFloats];
    cr.dpfIdx = 0;
    cr.pCtr = 0;
    cr.opCtr[cr.pCtr] = 0;
    cr.loCtr[cr.pCtr] = 0;

    // System.out.println("Compile " + cr.spIdx + ", " + cr.spChars.length);

    while (cr.spChars.length > cr.spIdx &&
           cr.spChars[cr.spIdx] != 0 &&
           cr.spChars[cr.spIdx] != '\r' &&
           cr.spChars[cr.spIdx] != '\n')
    {
      // System.out.println("CompileA " + cr.spIdx);
      if (isalpha(cr.spChars[cr.spIdx]) || cr.spChars[cr.spIdx] == '?' || cr.spChars[cr.spIdx] == PICHAR) {
        if (ParseInputSignal(callingMod) == 1) {
          exp.cFlags &= ~(EF_NoParams | EF_NoTime | EF_IsConstant);
          continue;
        }
        if (ParseParam('F') == 1) { // Folder Signal
          exp.cFlags &= ~(EF_NoParams | EF_NoTime | EF_IsConstant);
          continue;
        }
        if (ParseFunction(exp.cFlags) == 1) {
          continue;
        }
        if (ParseParam('P') == 1) { // Instrument Param
          exp.cFlags &= ~(EF_NoParams | EF_IsConstant | EF_NoTime);
          continue;
        }
        if (ParseParam('G') == 1) { // Global Variable
          exp.cFlags &= ~(EF_NoParams | EF_NoTime | EF_IsConstant);
          continue;
        }
        if (ParseVariable(exp) == 1)
          continue;
      }

      if ((cr.spChars[cr.spIdx] == '.' || (cr.spChars[cr.spIdx] >= '0' && cr.spChars[cr.spIdx] <= '9')) ||
        ((cr.spChars[cr.spIdx] == '-' || cr.spChars[cr.spIdx] == '+') && cr.beginSubExpFlag &&
        (cr.spChars[cr.spIdx+1] == '.' || (cr.spChars[cr.spIdx+1] >= '0' && cr.spChars[cr.spIdx+1] <= '9')))) {
        boolean negateFlag=false;
        if (cr.spChars[cr.spIdx] == '-') {
          negateFlag = true;
          ++cr.spIdx;
        }
        else if (cr.spChars[cr.spIdx] == '+') {
          // 1/28/93 Ignore unary +
          ++cr.spIdx;
        }
        v = MyAtoF(cr);

        /* If constant is preceded by unary negation, negate it */
        if (negateFlag)
          v = -v;
        cr.dpChars[cr.dpIdx++] = 'N'; // was using V# switched to Nn
        // Check if we have a previous float literal which matches
        boolean gotOne = false;
        int     idx = 0;
        for (int i = 0; i < cr.dpfIdx; ++i) {
          if (cr.dpFloats[i] == v)
          {
             gotOne = true;
             idx = i;
             break;
          }
        }
        if (!gotOne) {
           idx = cr.dpfIdx;
           cr.dpFloats[cr.dpfIdx++] = v;
        }
        cr.dpChars[cr.dpIdx++] = (char) idx;
        ResolveSubExp();

      }
      else {

        char c = cr.spChars[cr.spIdx];
        if (isalpha(c))
          c = tolower(c);

        switch (c) {
        case '+': RecordOp(OP_Add);   ++cr.spIdx; break;
        case '/': RecordOp(OP_Divide);    ++cr.spIdx; break;
        case '%': RecordOp(OP_Modulo);    ++cr.spIdx; break;
        case '&':
          if (cr.spChars[cr.spIdx+1] == '&') {
            RecordOp(OP_LogicAnd);
            ++cr.spIdx;
          }
          else
            RecordOp(OP_And);
          ++cr.spIdx;
          break;
        case '|':
          if (cr.spChars[cr.spIdx+1] == '|') {
            RecordOp(OP_LogicOr);
            ++cr.spIdx;
          }
          else
            RecordOp(OP_Or);
          ++cr.spIdx;
          break;
        case '^':
          RecordOp(OP_Power);   // Was XOR
          ++cr.spIdx;
          break;
        case '>':
          if (cr.spChars[cr.spIdx+1] == '=') {
            RecordOp(OP_Gte);
            ++cr.spIdx;
          }
          else
            RecordOp(OP_Greater);
          ++cr.spIdx;
          break;
        case '<':
          if (cr.spChars[cr.spIdx+1] == '>') {
            RecordOp(OP_Neq);
            ++cr.spIdx;
          }
          else if (cr.spChars[cr.spIdx+1] == '=') {
            RecordOp(OP_Lse);
            ++cr.spIdx;
          }
          else
            RecordOp(OP_Less);
          ++cr.spIdx;
          break;
        case '=':
          if (cr.spChars[cr.spIdx+1] == '=') {
            RecordOp(OP_Equal);
            ++cr.spIdx;
          }
          else
            RecordOp(OP_Equal);
          ++cr.spIdx;
          break;

        case '~':
          RecordOp(OP_Invert);
          ++cr.spIdx;
          break;

        case '!':
          if (cr.spChars[cr.spIdx+1] == '=') {
            RecordOp(OP_Neq);
            ++cr.spIdx;
          }
          else
            RecordOp(OP_Not);
          ++cr.spIdx;
          break;
        case '-':
          if (cr.beginSubExpFlag)
            RecordOp(OP_Negate);
          else
            RecordOp(OP_Subtract);
          ++cr.spIdx;
          break;
        case '*':
          if (cr.spChars[cr.spIdx+1] == '*') {
            RecordOp(OP_Power);
            ++cr.spIdx;
            ++cr.spIdx;
          }
          else {
            RecordOp(OP_Multiply);
            ++cr.spIdx;
          }
          break;
        case '(':
          // *(dp++) = '(';
          ++cr.spIdx;
          if (cr.pCtr < 16-1) {
            ++cr.pCtr;
            cr.opCtr[cr.pCtr] = 0;
            cr.loCtr[cr.pCtr] = 0;
          }
          else
            return -1;
          break;
        case ')':
          // *(dp++) = ')';
          ++cr.spIdx;
          if (cr.pCtr > 0) {
            --cr.pCtr;
            ResolveSubExp();
          }
          else
            return -1;
          break;
          /* Process Parens */
        case ',':
          /* Skip over comma */
          ++cr.spIdx;
          cr.beginSubExpFlag = true;
          break;
        case '[':
        case ']':
          ++cr.spIdx;   //  used to hint notime
          noTimeFlag = true;
          break;
        default:
          ++cr.spIdx;
          break;
        }
      }
    }
    cr.dpChars[cr.dpIdx++] = 0;
    if (noTimeFlag)
     exp.cFlags |= EF_NoTime;

    exp.cExp = new char[cr.dpIdx];
    System.arraycopy(cr.dpChars, 0, exp.cExp, 0, cr.dpIdx);
    exp.fExp = new double[cr.dpfIdx];
    System.arraycopy(cr.dpFloats, 0, exp.fExp, 0, cr.dpfIdx);

    return 0;
  }


  int ParseInputSignal(SSModule mod)
  {
    String  varName = "";
    int     si = 0;
    int     n;

    while (isalnum(cr.spChars[cr.spIdx+si]) || cr.spChars[cr.spIdx+si] == '_') {
      varName += tolower(cr.spChars[cr.spIdx+si]);
      ++si;
    }
    if (si == 0)
      return 0;
    for (n = 0; n < mod.nbrSupportedLinks; ++n) {
      LinkDesc ld = mod.linkDesc[n];
      if (varName.equals(ld.varName)) {
        cr.spIdx += varName.length();
        cr.dpChars[cr.dpIdx++] = 'L';
        cr.dpChars[cr.dpIdx++] = (char) n;
        ResolveSubExp();
        return 1;
      }
    }
    return 0;
  }

  int ParseFunction(int compileFlags)
  {
    // Don't need to modify compileFlags unless we start using a function which
    // returns randomized (unpredictable) results, otherwise, we need only
    // check the args to the functions

    int   opType = 0;
    String fName = "";
    char[] fNameB = new char[32];
    int   si = 0;

    while (isalnum(cr.spChars[cr.spIdx+si]) || cr.spChars[cr.spIdx+si] == '_') {
      fNameB[si] = tolower(cr.spChars[cr.spIdx+si]);
      ++si;
    }
    if (si == 0)
      return 0;
    fName = new String(fNameB,0,si);

    if (fName.equals("int"))     opType = OP_Integer;
    else if (fName.equals("sqrt"))   opType = OP_Sqrt;
    else if (fName.equals("log"))    opType = OP_Log;
    else if (fName.equals("log10"))  opType = OP_Log10;
    else if (fName.equals("abs"))    opType = OP_Fabs;
    else if (fName.equals("fabs"))   opType = OP_Fabs;
    else if (fName.equals("sign"))   opType = OP_Sign;
    else if (fName.equals("exp"))    opType = OP_Exp;
    else if (fName.equals("cos"))    opType = OP_Cos;
    else if (fName.equals("sin"))    opType = OP_Sin;
    else if (fName.equals("tan"))    opType = OP_Tan;
    else if (fName.equals("acos"))   opType = OP_ACos;
    else if (fName.equals("asin"))   opType = OP_ASin;
    else if (fName.equals("atan"))   opType = OP_ATan;
    else if (fName.equals("cosh"))   opType = OP_Cosh;
    else if (fName.equals("sinh"))   opType = OP_Sinh;
    else if (fName.equals("tanh"))   opType = OP_Tanh;
    else if (fName.equals("sin2"))   opType = OP_Sin2;
    else if (fName.equals("cos2"))   opType = OP_Cos2;
    else if (fName.equals("sin3"))   opType = OP_Sin3;
    else if (fName.equals("cos3"))   opType = OP_Cos3;
    else if (fName.equals("mand") ||
         fName.equals("mandel")) opType = OP_Mandel;
    else if (fName.equals("man3"))   opType = OP_Mandel3;
    else if (fName.equals("manc"))   opType = OP_MandelCPM;
    else if (fName.equals("mcpm"))   opType = OP_MandelCPM;
    else if (fName.equals("julia"))  opType = OP_Julia;
    else if (fName.equals("dragon")) opType = OP_Dragon;
    else if (fName.equals("fbm"))    opType = OP_FBM;
    else if (fName.equals("gturb3d"))  opType = OP_GTurb3D;
    else if (fName.equals("gnoise3d")) opType = OP_GNoise3D;
    else if (fName.equals("fib"))    opType = OP_Fib;
    else if (fName.equals("prime"))  opType = OP_Prime;

    else if (fName.equals("cpspch")) opType = OP_cpspch;
    else if (fName.equals("cpsoct")) opType = OP_cpsoct;
    else if (fName.equals("octpch")) opType = OP_octpch;
    else if (fName.equals("octcps")) opType = OP_octcps;
    else if (fName.equals("pchoct")) opType = OP_pchoct;
    else if (fName.equals("octmidi"))  opType = OP_octmidi;
    else if (fName.equals("cpsmidi"))  opType = OP_cpsmidi;

    else if (fName.equals("noise"))  opType = OP_Noise;
    else if (fName.equals("noise3d"))  opType = OP_Noise3D;
    else if (fName.equals("turb"))   opType = OP_Turb;
    else if (fName.equals("turb3d")) opType = OP_Turb3D;
    else if (fName.equals("cond"))   opType = OP_Cond;
    else if (fName.equals("angle"))  opType = OP_Angle;
    else if (fName.equals("ftab"))   opType = OP_FTab;
    else if (fName.equals("ftabw"))  opType = OP_FTabW;
    else if (fName.equals("ftabp"))  opType = OP_FTabP;
    else if (fName.equals("ftabi"))  opType = OP_FTabI;
    else if (fName.equals("ftabiw")) opType = OP_FTabIW;
    else if (fName.equals("ftabip")) opType = OP_FTabIP;
    else if (fName.equals("linen"))  opType = OP_linen;
    else if (fName.equals("linenr")) opType = OP_linenr;
    else if (fName.equals("limit"))  opType = OP_limit;
    else if (fName.equals("mod"))    opType = OP_Modulo;
    else if (fName.equals("xor"))    opType = OP_Xor;
    else if (fName.equals("and"))    opType = OP_And;
    else if (fName.equals("or"))   opType = OP_Or;
    else if (fName.equals("not"))    opType = OP_Not;
    else if (fName.equals("pow2"))   opType = OP_Power2;
    else if (fName.equals("pow"))    opType = OP_Power;
    else if (fName.equals("snote")) opType = OP_SNote;
    else if (fName.equals("fnote")) opType = OP_FNote;
    else if (fName.equals("dist") ||
         fName.equals("distance")) opType = OP_Dist;
    else if (fName.equals("pi"))
    {
      cr.dpChars[cr.dpIdx++] = 'V';
      cr.dpChars[cr.dpIdx++] = PICHAR;
      cr.spIdx += 2;
      ResolveSubExp();
      return 1;
    }
    else if (fName.equals("pi2"))
    {
      cr.dpChars[cr.dpIdx++] = 'V';
      cr.dpChars[cr.dpIdx++] = PICHAR+1;
      cr.spIdx += 3;
      ResolveSubExp();
      return 1;
    }
    else if (fName.equals("major"))
    {
      cr.dpChars[cr.dpIdx++] = 'V';
      cr.dpChars[cr.dpIdx++] = MAJORCHAR;
      cr.spIdx += 5;
      ResolveSubExp();
      return 1;
    }
    else if (fName.equals("minor"))
    {
      cr.dpChars[cr.dpIdx++] = 'V';
      cr.dpChars[cr.dpIdx++] = MINORCHAR;
      cr.spIdx += 5;
      ResolveSubExp();
      return 1;
    }
    else if (fName.equals("minorh"))
    {
      cr.dpChars[cr.dpIdx++] = 'V';
      cr.dpChars[cr.dpIdx++] = MINORHCHAR;
      cr.spIdx += 6;
      ResolveSubExp();
      return 1;
    }
    else
      return 0;
    RecordOp(opType);
    cr.spIdx += fName.length();
    return 1;
  }


  // These next three do the same thing...
  int ParseParam(char sigType)
  {
    char c = cr.spChars[cr.spIdx];
    if (isalpha(c))
      c = tolower(c);
    if (c == tolower(sigType) && isdigit(cr.spChars[cr.spIdx+1])) {
      int param = 0;
      cr.spIdx++;
      while (isdigit(cr.spChars[cr.spIdx])) {
        param *= 10;
        param += cr.spChars[cr.spIdx] - '0';
        cr.spIdx++;
      }
      cr.dpChars[cr.dpIdx++] = sigType;
      cr.dpChars[cr.dpIdx++] = (char) param;
      ResolveSubExp();
      return 1;
    }
    return 0;
  }

  int ParseVariable(ExpRec exp)
  {
    char c;
    char  var = 0;

    c = cr.spChars[cr.spIdx];
    if (isalpha(c))
      c = tolower(c);

    switch (c) {
    case 'd': // total note duration
    case 'r': // sample rate
    case 'e': // exp(1)
      var = (char) c;
      break;
    case 'g': // global time ctr
    case 'n': // note time ctr
    case 't': // local time ctr
    case 'k': // K value (used for hammer instruments key#)
    case 'i': // I Value (used for random scores)
    case 'a': // A Value (used for hammer instrument amplitude)
    case 'm': // M value (max notes in random score module)
      var = (char) c;
      exp.cFlags &= ~(EF_IsConstant | EF_NoTime);
      break;
    case 'v':
      var = (char) c;// input signal
      exp.cFlags &= ~(EF_IsConstant | EF_NoParams | EF_NoTime);
      break;
    case '?':
      var = (char) c;
      exp.cFlags &= ~(EF_IsConstant | EF_NoTime);
      break;
    case PICHAR:
      var = (char) c;
      break;
    }

    if (var != 0) {

      cr.dpChars[cr.dpIdx++] = 'V';
      cr.dpChars[cr.dpIdx++] = var;
      cr.spIdx++;
      ResolveSubExp();
      return 1;
    }
    return 0;
  }

  double MyAtoF(CompileVars cr)
  {
    boolean decimal=false,negate=false;
    double  div = 1,v = 0.0;

    // 1/11
    if (cr.spChars[cr.spIdx] == '-') {
      negate = true;
      ++cr.spIdx;
    }
    while (cr.spIdx < cr.spChars.length &&
           (cr.spChars[cr.spIdx] == '.' ||
            (cr.spChars[cr.spIdx] >= '0' && cr.spChars[cr.spIdx] <= '9')
            )
          )
    {
      if (cr.spChars[cr.spIdx] == '.') {
        decimal = true;
        div = 1;
      }
      else {
        if (decimal) {
          div *= 10;
          v += (cr.spChars[cr.spIdx]-'0') / div;
        }
        else {
          v *= 10;
          v += (cr.spChars[cr.spIdx]-'0');
        }
      }
      ++cr.spIdx;
    }
    if (negate)
      v = -v;
    return v;
  }

  void ResolveSubExp()
  {
    while (cr.opCtr[cr.pCtr] > 0) {
      cr.dpChars[cr.dpIdx++] = 'O';

      --cr.opCtr[cr.pCtr];
      cr.lastOpStack[cr.pCtr][cr.loCtr[cr.pCtr]] = cr.dpIdx;
      cr.loCtr[cr.pCtr]++;
      cr.dpChars[cr.dpIdx++] = (char) (cr.opStack[cr.pCtr][cr.opCtr[cr.pCtr]]);
    }
    cr.beginSubExpFlag = false;
  }

  void RecordOp(int newOp)
  {
    // If we encounter x+y*z, we grab back the + operator and put it back on the
    // stack.
    // 1/7/92 Don't do this if newOp is a prefix operator or function.
    while (cr.loCtr[cr.pCtr] > 0 &&
         cr.lastOpStack[cr.pCtr][cr.loCtr[cr.pCtr]-1] == (cr.dpIdx - 1) &&
         // precedence
         opData[cr.dpChars[cr.dpIdx-1]][1] > opData[newOp][1] &&
         // prefix flag
         opData[newOp][2] == 0)
    {
      cr.opStack[cr.pCtr][cr.opCtr[cr.pCtr]] = cr.dpChars[cr.dpIdx-1];
      ++cr.opCtr[cr.pCtr];
      cr.dpIdx -= 2;
      --cr.loCtr[cr.pCtr];
    }
    cr.opStack[cr.pCtr][cr.opCtr[cr.pCtr]] = (char) newOp;
    ++cr.opCtr[cr.pCtr];
    cr.beginSubExpFlag = true;
  }

  double PopF()
  {
    return stackF[--stackCtr];
  }

  void PushF(double v)
  {
    stackF[stackCtr++] = v;
  }

  int PopL()
  {
    return (int) stackF[--stackCtr];
  }

  double EvalF(SSModule callingModule, ExpRec compexp)  // !!!
  {
    if (compexp.exp == null || compexp.exp.length() == 0)
      return 0;
    try {
    int     lv1,lv2;
    double  v1,v2,v3;
    int     lFlags = 0, fFlags = 0;
    double  pTime = callingModule.parList.pTime;
    int     epIdx = 0;

    while (compexp.cExp[epIdx] != 0)
    {
      switch (compexp.cExp[epIdx++]) {
      case 'L': // inputs to module: sig, am, etc.
        {
          int linkNbr = compexp.cExp[epIdx++];
          if (callingModule != null) {
            if ((lFlags & (1L << linkNbr)) > 0) {
              PushF(lSigs[linkNbr]);
            }
            else {
              double sig = callingModule.MixInputs(linkNbr,callingModule.callingMod);
              lSigs[linkNbr] = sig;
              lFlags |= (1L << linkNbr);
              PushF(sig);
            }
          }
          else
            PushF(0.0);
        }
        break;
      case 'P': // Instrument Parameters (Pn)
        {
          int paramNbr = compexp.cExp[epIdx++];
          if (callingModule != null && callingModule.callingMod != null)
            PushF(callingModule.callingMod.GetInstParameter(paramNbr));
          else
            PushF(0.0);
        }
        break;
      case 'G': // Global Variable (Gn)
        {
          int paramNbr = compexp.cExp[epIdx++];
          if (callingModule != null && callingModule.parList != null && callingModule.parList.itsOwner != null)
            PushF(callingModule.parList.itsOwner.RetrieveGlobal(paramNbr));
          else
            PushF(0.0);
        }
        break;
      case 'F': // Folder Input Signals (Fn)
        {
          int paramNbr = compexp.cExp[epIdx++];
          double sig = 0.0;
          if (callingModule != null) {
            if ((fFlags & (1L << paramNbr)) > 0) {
              PushF(fSigs[paramNbr]);
            }
            else {
              SSModule mod = (SSModule) callingModule.parList.mods.firstElement();
              try {
                sig = mod.GetFolderSig(paramNbr);
              }
              catch (Exception e) {
              }
              fFlags |= (1L << paramNbr);
              fSigs[paramNbr] = sig;
            }
          }
          PushF(sig);
        }
        break;
      case 'O': // Operator/Function
        switch (compexp.cExp[epIdx++]) {
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
          PushF(v2 == 0.0? Double.POSITIVE_INFINITY : (v1/v2));
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
          PushF(Math.pow(v1,v2));
          break;
        case OP_Power2:
          v1 = PopF();
          PushF(Math.pow(2.0,v1));
          break;
        case OP_Integer:
          lv1 = PopL();
          PushF(lv1);
          break;
        case OP_And:
          lv1 = PopL();
          lv2 = PopL();
          PushF(lv1 & lv2);
          break;
        case OP_Or:
          lv1 = PopL();
          lv2 = PopL();
          PushF(lv1 | lv2);
          break;
        case OP_Xor:
          lv1 = PopL();
          lv2 = PopL();
          PushF(lv1 ^ lv2);
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
          if (v1 < 0)
            v1 = -v1;
          PushF(Math.sqrt(v1));
          break;
/*        case OP_Mandel:
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
            double v3,v4;
            v4 = PopF();
            v3 = PopF();
            v2 = PopF();
            v1 = PopF();
            PushF(CalcJulia(v1,v2,v3,v4));
          }
          break;
        case OP_Dragon:
          {
            double v3,v4;
            v4 = PopF();
            v3 = PopF();
            v2 = PopF();
            v1 = PopF();
            PushF(CalcDragon(v1,v2,v3,v4));
          }
          break;
        case OP_FBM:
          {
            double v3,v4,v5,v6;
            v6 = PopF();
            v5 = PopF();
            v4 = PopF();
            v3 = PopF();
            v2 = PopF();
            v1 = PopF();
            PushF(fBm(v1,v2,v3,v4,v5,v6));
          }
          break;
*/
        case OP_Fib:
          lv1 = PopL();
          PushF(ExpFuncs.Fibonacci(lv1));
          break;
        case OP_Prime:
          lv1 = PopL();
          PushF(ExpFuncs.isPrime(lv1));
          break;
        case OP_cpspch:
          v1 = PopF();
          PushF(ExpFuncs.cpspch(v1));
          break;
        case OP_cpsoct:
          v1 = PopF();
          PushF(ExpFuncs.cpsoct(v1));
          break;
        case OP_octpch:
          v1 = PopF();
          PushF(ExpFuncs.octpch(v1));
          break;
        case OP_octcps:
          v1 = PopF();
          PushF(ExpFuncs.octcps(v1));
          break;
        case OP_pchoct:
          v1 = PopF();
          PushF(ExpFuncs.pchoct(v1));
          break;
        case OP_octmidi:
          v1 = PopF();
          PushF(ExpFuncs.octmidi(v1));
          break;
        case OP_cpsmidi:
          v1 = PopF();
          PushF(ExpFuncs.cpsmidi(v1));
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
        case OP_Turb3D:
          v3 = PopF();
          v2 = PopF();
          v1 = PopF();
          PushF(Turbulence3D(v1,v2,v3));
          break;
/*
        case OP_GNoise3D:
          v3 = PopF();
          v2 = PopF();
          v1 = PopF();
          PushF(gnoise(v1,v2,v3));
          break;
        case OP_GTurb3D:
          v3 = PopF();
          v2 = PopF();
          v1 = PopF();
          PushF(GTurbulence3D(v1,v2,v3));
          break;
*/
        case OP_Angle:
          v2 = PopF();
          v1 = PopF();
          PushF(ExpFuncs.Angle(v1,v2));
          break;
        case OP_Dist:
          v2 = PopF();
          v1 = PopF();
          PushF(ExpFuncs.Distance(v1,v2));
          break;
        case OP_Cond:
          {
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
          PushF(Math.cos(v1));
          break;
        case OP_Sin:
          v1 = PopF();
          PushF(Math.sin(v1));
          break;
        case OP_Sin2:
          v1 = PopF();
          v1 = Math.sin(v1);
          PushF(v1*v1);
          break;
        case OP_Cos2:
          v1 = PopF();
          v1 = Math.cos(v1);
          PushF(v1*v1);
          break;
        case OP_Sin3:
          v1 = PopF();
          v1 = Math.sin(v1);
          PushF(v1*v1*v1);
          break;
        case OP_Cos3:
          v1 = PopF();
          v1 = Math.cos(v1);
          PushF(v1*v1*v1);
          break;
        case OP_FTab:
          v2 = PopF();
          v1 = PopF();
          PushF(SSFTable.GetFTableEntry(callingModule,(int) v1, v2,SSFTable.FW_NoWrap));
          break;
        case OP_FTabW:
          v2 = PopF();
          v1 = PopF();
          PushF(SSFTable.GetFTableEntry(callingModule,(int) v1, v2,SSFTable.FW_Wrap));
          break;
        case OP_FTabP:
          v2 = PopF();
          v1 = PopF();
          PushF(SSFTable.GetFTableEntry(callingModule,(int) v1, v2,SSFTable.FW_Pin));
          break;
        case OP_FTabI:
          v2 = PopF();
          v1 = PopF();
          PushF(SSFTable.GetFTableEntry(callingModule,(int) v1, v2,SSFTable.FW_NoWrap|SSFTable.FW_Interp));
          break;
        case OP_FTabIW:
          v2 = PopF();
          v1 = PopF();
          PushF(SSFTable.GetFTableEntry(callingModule,(int) v1, v2,SSFTable.FW_Wrap|SSFTable.FW_Interp));
          break;
        case OP_FTabIP:
          v2 = PopF();
          v1 = PopF();
          PushF(SSFTable.GetFTableEntry(callingModule,(int) v1, v2,SSFTable.FW_Pin|SSFTable.FW_Interp));
          break;
        case OP_linen:
          {
            double v4;
            v4 = PopF();
            v3 = PopF();
            v2 = PopF();
            v1 = PopF();
            PushF(ExpFuncs.linen(v1,v2,v3,v4));
          }
          break;
        case OP_linenr:
          {
            double v4;
            v4 = PopF();
            v3 = PopF();
            v2 = PopF();
            v1 = PopF();
            PushF(ExpFuncs.linenr(v1,v2,v3,v4));
          }
          break;
        case OP_limit:
          v3 = PopF();
          v2 = PopF();
          v1 = PopF();
          PushF(ExpFuncs.limit(v1,v2,v3));
          break;

        case OP_Log:
          v1 = PopF();
          PushF(Math.log(v1));
          break;
        case OP_Log10:
          v1 = PopF();
          PushF(Math.log10(v1));
          break;
        case OP_Fabs:
          v1 = PopF();
          PushF(Math.abs(v1));
          break;
        case OP_Sign:
          v1 = PopF();
          PushF(v1 < 0? -1.0 : 1.0);
          break;
        case OP_Exp:
          v1 = PopF();
          PushF(Math.exp(v1)); // Mac version used expf??
          break;
        case OP_Tan:
          v1 = PopF();
          PushF(Math.tan(v1));
          break;
        case OP_ACos:
          v1 = PopF();
          PushF(Math.acos(v1));
          break;
        case OP_ASin:
          v1 = PopF();
          PushF(Math.asin(v1));
          break;
        case OP_ATan:
          v1 = PopF();
          PushF(Math.atan(v1));
          break;
        case OP_Cosh:
          v1 = PopF();
          PushF(Math.cosh(v1));
          break;
        case OP_Sinh:
          v1 = PopF();
          PushF(Math.sinh(v1));
          break;
        case OP_Tanh:
          v1 = PopF();
          PushF(Math.tanh(v1));
          break;
        case OP_SNote:
          v2 = PopF();
          v1 = PopF();
          PushF(ExpFuncs.scaleNote((int) v1,(int) v2));
          break;
        case OP_FNote:
          v3 = PopF();
          v2 = PopF();
          v1 = PopF();
          PushF(ExpFuncs.scaleNoteF((int) v1,(int) v2,(int) v3));
          break;
        } // end operator switch
        break;
      case 'N':
        {
          int fNbr = compexp.cExp[epIdx++];
          if (fNbr < compexp.fExp.length)
            PushF(compexp.fExp[fNbr]);
          else {
            System.out.println("Invalid Float Index");
            PushF(0.0);
          }
        }
        break;

      case 'V':
        switch (compexp.cExp[epIdx++]) {
        case 't':
          PushF(pTime);
          break;
        case 'd':
          if (callingModule != null)
            PushF(callingModule.parList.itsOwner.mainInst.sampleDuration);
          break;
        case 'v':
          PushF(callingModule.MixInputs(-1,callingModule.callingMod));
          break;
        case 'g':
          if (callingModule != null)
            PushF(callingModule.parList.itsOwner.gTime);
          break;
        case 'n':
          PushF(callingModule.GetNoteTime());
          break;
        case 'k':
          PushF(callingModule.GetKeyValue());
          break;
        case 'i':
          PushF(callingModule.GetIValue());
          break;
        case 'a':
          PushF(callingModule.GetAValue());
          break;
        case 'r':
          if (callingModule != null)
            PushF(callingModule.parList.itsOwner.mainInst.sampleRate);
          break;
        case 'e':
          PushF(exp1);
          break;
        case '?':
          PushF(DoubleRandom());
          break;
        case 'm':
          PushF(callingModule.GetMValue());
          break;
        case PICHAR:
          PushF(Math.PI);
          break;
        case PICHAR+1:
          PushF(pi2);
          break;
        case MAJORCHAR:
          PushF((double) 0xAB5); // bitmask for major scale
          break;
        case MINORCHAR:
          PushF((double) 0x5AD); // bitmask for minor scale
          break;
        case MINORHCHAR:
          PushF((double) 0x9AD); // bitmask for minor enharmonic scale
          break;
        } // end V switch
        break;
      } // end evalf switch
    } // end while
    if (stackCtr > 0)
      return PopF();
    } // end try
    catch (Exception e) {
      System.out.println("Exp error: " + e.toString());
      e.printStackTrace();
    }
    return 0.0;
  } // end EvalF function

  // Random stuff
  //
  //
  static final int R_A = 16807;
  static final int R_M = 2147483647;
  static final int R_Q = 127773;
  static final int R_R = 2836;
  static int  gSeed = 1;

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static int GetSRand()
  {
    return gSeed;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static void MySRand(int s)
  {
    gSeed = s;
    if (gSeed == 0)
      gSeed = 1;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static void Randomize()
  {
    MySRand((int) (System.currentTimeMillis() & 0xFFFFFFFF));
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static int LongRandom()
  {
    int  hi,lo,test;

    hi   = gSeed / R_Q;
    lo   = gSeed % R_Q;
    test = R_A * lo - R_R * hi;
    if (test > 0)
      gSeed = test;
    else
      gSeed = test + R_M;
    return gSeed;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static double DoubleRandom()
  {
    return LongRandom() / (double) R_M;
  }


  double RandPoint(int x, int y)
  {
    int  sr;
    double  r;
    if (x < 0)
      x = -x;
    if (y < 0)
      y = -y;
    sr = GetSRand();
    MySRand(yA[y%256] ^ xA[x%256]);
    r = DoubleRandom();
    MySRand(sr);
    return r;
  }

  double RandPoint3D(int x, int y, int z)
  {
    int  sr;
    double  r;
    if (x < 0)
      x = -x;
    if (y < 0)
      y = -y;
    if (z < 0)
      z = -z;
    sr = GetSRand();
    MySRand(xA[x%256] ^ yA[y%256] ^ zA[z%256]);
    r = DoubleRandom();
    MySRand(sr);
    return r;
  }

  double Noise(double x, double y)
  {
    double  dx,dy,x1,x2;
    int   ix,iy;

    if (x < 0)
      x = -x;

    if (y < 0)
      y = -y;

    ix = (int) x;
    iy = (int) y;
    dx = x - ix;
    dy = y - iy;

    // To avoid linear interpolation artifacts due to sharp corners, we round
    // the edges (ease-in and ease-out)
    dx *= dx*(3-2*dx);
    dy *= dy*(3-2*dy);

    x1 = (1-dx)*RandPoint(ix,iy)   + dx*RandPoint(ix+1,iy);
    x2 = (1-dx)*RandPoint(ix,iy+1) + dx*RandPoint(ix+1,iy+1);
    return (1-dy)*x1 + dy*x2;
  }

  double Noise3D(double x, double y, double z)
  {
    double  dx,dy,dz,j1,j2,j3,k1,k2,k3;
    int   ix,iy,iz;

    if (x < 0)
      x = -x;

    if (y < 0)
      y = -y;

    if (z < 0)
      z = -z;

    ix = (int) x;
    iy = (int) y;
    iz = (int) z;
    dx = x - ix;
    dy = y - iy;
    dz = z - iz;

    // To avoid linear interpolation artifacts due to sharp corners, we round
    // the edges (ease-in and ease-out)
    dx *= dx*(3-2*dx);
    dy *= dy*(3-2*dy);
    dz *= dz*(3-2*dz);

    j1 = (1-dx)*RandPoint3D(ix,iy,iz)   + dx*RandPoint3D(ix+1,iy,iz);
    j2 = (1-dx)*RandPoint3D(ix,iy+1,iz) + dx*RandPoint3D(ix+1,iy+1,iz);
    j3 = (1-dy)*j1 + dy*j2;

    if (dz == 0.0)
      return j3;
    else {
      k1 = (1-dx)*RandPoint3D(ix,iy,iz+1)   + dx*RandPoint3D(ix+1,iy,iz+1);
      k2 = (1-dx)*RandPoint3D(ix,iy+1,iz+1) + dx*RandPoint3D(ix+1,iy+1,iz+1);
      k3 = (1-dy)*k1 + dy*k2;
      return (1-dz)*j3 + dz*k3;
    }
  }

  // Fractal Disturbance
  // Returns -1 <= r < 1

  double Turbulence(double x, double y)
  {
    double  turb, s, limit;

    turb = -1.0;
    s = 1;
    limit = 1.0/256;

    while (s > limit) {
      turb += s * Noise(x/s,y/s);
      s /= 2;
    }

    return turb;
  }

  double Turbulence3D(double x, double y, double z)
  {
    double  turb, s, limit;

    turb = -1.0;
    s = 1;
    limit = 1.0/256;

    while (s > limit) {
      turb += s * Noise3D(x/s,y/s,z/s);
      s /= 2;
    }

    return turb;
  }

}
