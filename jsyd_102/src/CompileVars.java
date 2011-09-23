// import java.io.*;

public class CompileVars
{
  char[][] opStack;//     opStack[MaxParenNesting][MaxSubExpressions];
  short[]   opCtr;  // [MaxParenNesting];   // Operator Eval Counter
  short[]   loCtr; // [MaxParenNesting];   // Last Op Counter
  short     pCtr = 0;           // Paren Counter
  char[]    spChars;
  char[]    dpChars;
  int       spIdx = 0; // , dpIdx = 0; // unsigned char   *sp,*dp;
  int       dpIdx = 0;
  double[]  dpFloats;
  int       dpfIdx = 0;
  int[][]   lastOpStack; // [MaxParenNesting][MaxSubExpressions];  // was unsigned char ptr
  boolean   beginSubExpFlag = false;

  // DataOutputStream  dos = null;
  // ByteArrayOutputStream bos = null;

  CompileVars()
  {
    opStack = new char[ExpMgr.MaxParenNesting][];
    lastOpStack = new int[ExpMgr.MaxParenNesting][];
    opCtr = new short[ExpMgr.MaxParenNesting];
    loCtr = new short[ExpMgr.MaxParenNesting];
    for (int i = 0; i < ExpMgr.MaxParenNesting; ++i)
    {
      opStack[i] = new char[ExpMgr.MaxSubExpressions];
      lastOpStack[i] = new int[ExpMgr.MaxSubExpressions];
      for (int j = 0; j < ExpMgr.MaxSubExpressions; ++j)
      {
        opStack[i][j] = 0;
        lastOpStack[i][j] = 0;
      }
    }
    // dos = new DataOutputStream(new ByteArrayOutputStream());
  }
}