import java.io.*;
import java.util.*;

public class SSPInput extends SSModule implements SydConstants
{
  public int           pNbr = 1;
  public String        desc = "input value";
  public ExpRec        defExp = InitExp("");

  // dynamic
  double        pValue;

  public SSPInput(ModList mList)
  {
    super(MT_PInput, mList);
    DescribeLink(0, "INVALID", "???");
  }

  void  Copy(SSModule ss)
  {
    SSPInput sa = (SSPInput) ss;
    super.Copy(ss);
    CopyExp(sa.defExp, defExp);
    pNbr = sa.pNbr;
    desc = sa.desc;
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("PIDEF " + defExp.exp);
    ar.println("PINN " + pNbr);
    ar.println("PIND " + desc);
  }

  void Load(BufferedReader ar) throws IOException
  {
    defExp = LoadExp(ar,"PIDEF");
    pNbr = Integer.parseInt(getFirstToken(ar,"PINN"));
    desc = LoadString(ar,"PIND");
  }

  double GenerateOutput(SSModule callingMod)
  {
    return pValue;
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    ResetExp(defExp, callingMod);
    if (callingMod.IsInScore()) {
      pValue = callingMod.GetInstParameter(pNbr);
    }
    else {
      pValue = SolveExp(defExp, callingMod);
    }
    lastRightSample = pValue;
  }

}

