import java.io.*;
import java.util.*;

public class SSFInput extends SSModule implements SydConstants
{
  public int           fNbr = 0;
  public String        desc = "input value";
  public ExpRec        defExp = InitExp("");

  public SSFInput(ModList mList)
  {
    super(MT_FInput, mList);
    DescribeLink(0, "INVALID", "???");
  }

  void  Copy(SSModule ss)
  {
    SSFInput sa = (SSFInput) ss;
    super.Copy(ss);
    CopyExp(sa.defExp, defExp);
    fNbr = sa.fNbr;
    desc = sa.desc;
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("FIDEF " + defExp.exp);
    ar.println("FINN " + fNbr);
    ar.println("FIND " + desc);
  }

  void Load(BufferedReader ar) throws IOException
  {
    defExp = LoadExp(ar,"FIDEF");
    fNbr = Integer.parseInt(getFirstToken(ar,"FINN"));
    desc = LoadString(ar,"FIND"); // This needs to read entire rest of line
  }

  double GenerateOutput(SSModule callingMod)
  {
    double v;
    this.callingMod = callingMod;
    try {
      v = ((SSModule) parList.mods.firstElement()).GetFolderSig(fNbr);
    }
    catch (Exception e)
    {
      v = SolveExp(defExp, callingMod);
    }
    lastRightSample = v;
    return v;
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    ResetExp(defExp, callingMod);
  }

}

