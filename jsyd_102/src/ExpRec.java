public class ExpRec
{
	String exp;    // original expression
	char[] cExp;   // compiled expression
  double[] fExp; // floats in compiled expression
	int	cFlags;
	double	eConst;

  public ExpRec(String strExp)
  {
    Init(strExp, new char[ExpMgr.LenCompiledExp], new double[ExpMgr.MaxFloats], 0, 0.0);
  }


  public ExpRec(String strExp, char[] cExp, double[] fExp, int cFlags, double eConst)
  {
    Init(strExp, cExp, fExp, cFlags, eConst);
  }


  public ExpRec(ExpRec exp) // copy constructor
  {
    this(exp.exp, exp.cExp, exp.fExp, exp.cFlags, exp.eConst);
  }

  public void Init(String strExp, char[] cExp, double[] fExp, int cFlags, double eConst)
  {
    this.exp = strExp;
    this.cExp = cExp;
    this.fExp = fExp;
    this.cFlags = cFlags;
    this.eConst = eConst;
  }

}

