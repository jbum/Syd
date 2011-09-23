import java.io.*;
import java.util.*;

public class SSGAssign extends SSModule implements SydConstants
{
  public String        desc = "var";
  public ExpRec        gNbrExp = InitExp("0"),
                        valExp  = InitExp("sig");

  public SSGAssign(ModList mList)
  {
    super(MT_GAssign, mList);
    DescribeLink(0, "Default Signal", "sig");
    DescribeLink(1, "Alt Signal #1", "sig1");
    DescribeLink(2, "Alt Signal #2", "sig2");
    DescribeLink(3, "Alt Signal #3", "sig3");
  }

  void  Copy(SSModule ss)
  {
    SSGAssign sa = (SSGAssign) ss;
    super.Copy(ss);
    CopyExp(sa.valExp, valExp);
    CopyExp(sa.gNbrExp, gNbrExp);
    desc = sa.desc;
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("GASSN " + gNbrExp.exp);
    ar.println("GASSV " + valExp.exp);
    ar.println("GASSD " + desc);
  }


  void Load(BufferedReader ar) throws IOException
  {
    gNbrExp = LoadExp(ar,"GASSN");
    valExp = LoadExp(ar,"GASSV");
    desc = getFirstToken(ar,"GASSD");
  }

  double GenerateOutput(SSModule callingMod)
  {
    double n = SolveExp(gNbrExp, callingMod);
    double v = SolveExp(valExp, callingMod);
    parList.itsOwner.AssignGlobal((int) n, v);
    lastRightSample = 0.0;
    return 0.0;
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    ResetExp(gNbrExp, callingMod);
    ResetExp(valExp, callingMod);
  }

}

