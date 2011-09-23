import java.io.*;

public class SSExpression extends SSModule implements SydConstants
{
  public ExpRec exp;

  public SSExpression(ModList mList)
  {
    super(MT_Expression, mList);
    exp = InitExp("");

    DescribeLink(0, "Default Signal", "sig");
    DescribeLink(1, "Alt Signal #1", "sig1");
    DescribeLink(2, "Alt Signal #2", "sig2");
    DescribeLink(3, "Alt Signal #3", "sig3");
    DescribeLink(4, "Alt Signal #4", "sig4");
    DescribeLink(5, "Alt Signal #5", "sig5");
    DescribeLink(6, "Alt Signal #6", "sig6");
    DescribeLink(7, "Alt Signal #7", "sig7");
    DescribeLink(8, "Alt Signal #8", "sig8");
    DescribeLink(9, "Alt Signal #9", "sig9");
    DescribeLink(10, "Alt Signal #10", "sig10");
    DescribeLink(11, "Alt Signal #11", "sig11");
    DescribeLink(12, "Alt Signal #12", "sig12");
    DescribeLink(13, "Alt Signal #13", "sig13");
    DescribeLink(14, "Alt Signal #14", "sig14");
    DescribeLink(15, "Alt Signal #15", "sig15");
  }
  void  Copy(SSModule ss)
  {
    SSExpression sa = (SSExpression) ss;
    super.Copy(ss);
    CopyExp(sa.exp, exp);
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("EXP " + exp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    exp = LoadExp(ar,"EXP");
  }

  double GenerateOutput(SSModule callingMod)
  {
    this.callingMod = callingMod;
    double retVal = SolveExp(exp, callingMod);
    lastRightSample = retVal;
    return retVal;
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    ResetExp(exp, callingMod);
  }

}

