import java.io.*;
import java.util.*;

public class SSFilter extends SSModule implements SydConstants
{
  static final int FF_Recursive = 1;

  public ExpRec    a0Exp = InitExp("0.5"),
            a1Exp = InitExp("0.5"),
            a2Exp = InitExp("0"),
            b1Exp = InitExp("0"),
            b2Exp = InitExp("0");
  public int       flags = 0;

  // dynamic
  double[] oldY = new double[2],
           oldX = new double[2];

  public SSFilter(ModList mList)
  {
    super(MT_Filter, mList);
    DescribeLink(0, "Signal to Filter", "sig");
    DescribeLink(1, "Control Signal", "ctl");
    DescribeLink(2, "Alt Control Sig #1", "ctl1");
    DescribeLink(3, "Alt Control Sig #2", "ctl2");
    DescribeLink(4, "Alt Control Sig #3", "ctl3");
    DescribeLink(5, "Alt Control Sig #4", "ctl4");
    DescribeLink(6, "Alt Control Sig #5", "ctl5");
    DescribeLink(7, "Alt Control Sig #6", "ctl6");
    DescribeLink(8, "Alt Control Sig #7", "ctl7");
    DescribeLink(9, "Alt Control Sig #8", "ctl8");
 }

  void  Copy(SSModule ss)
  {
    SSFilter sa = (SSFilter ) ss;
    super.Copy(ss);
    CopyExp(sa.a0Exp, a0Exp);
    CopyExp(sa.a1Exp, a1Exp);
    CopyExp(sa.a2Exp, a2Exp);
    CopyExp(sa.b1Exp, b1Exp);
    CopyExp(sa.b2Exp, b2Exp);
    this.flags = sa.flags;
  }


  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("FLT " + flags);
    ar.println("FLTA0 " + a0Exp.exp);
    ar.println("FLTA1 " + a1Exp.exp);
    ar.println("FLTA2 " + a2Exp.exp);
    ar.println("FLTB1 " + b1Exp.exp);
    ar.println("FLTB2 " + b2Exp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    flags = Integer.parseInt(getFirstToken(ar,"FLT"));
    a0Exp = LoadExp(ar,"FLTA0");
    a1Exp = LoadExp(ar,"FLTA1");
    a2Exp = LoadExp(ar,"FLTA2");
    b1Exp = LoadExp(ar,"FLTB1");
    b2Exp = LoadExp(ar,"FLTB2");
  }

  double GenerateOutput(SSModule callingMod)
  {

    double a0 = SolveExp(a0Exp,callingMod);
    double a1 = SolveExp(a1Exp,callingMod);
    double a2 = SolveExp(a2Exp,callingMod);
    double b1 = SolveExp(b1Exp,callingMod);
    double b2 = SolveExp(b2Exp,callingMod);

    double x = MixInputs(0, callingMod);

    double y = a0*x + a1*oldX[0] + a2*oldX[1] +
          b1*oldY[0] + b2*oldY[1];

    oldX[1] = oldX[0];
    oldX[0] = x;

    oldY[1] = oldY[0];
    oldY[0] = y;
    lastRightSample = y;
    return y;
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    ResetExp(a0Exp, callingMod);
    ResetExp(a1Exp, callingMod);
    ResetExp(a2Exp, callingMod);
    ResetExp(b1Exp, callingMod);
    ResetExp(b2Exp, callingMod);
    oldX[0] = 0;
    oldX[1] = 0;
    oldY[0] = 0;
    oldY[1] = 0;
  }

}