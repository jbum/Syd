import java.io.*;
import java.util.*;

public class SSFTable extends SSModule implements SydConstants
{
  static final int FW_NoWrap = 0;
  static final int FW_Wrap   = 1;
  static final int FW_Pin    = 2;
  static final int FW_Interp = 4;
  static final int MaxTables = 32;
  static SSFTable[] ftable = new SSFTable[MaxTables];

  public ExpRec        tabExp;
  public int           tabSize,tabNbr;

  double[]      table;

  static double GetFTableEntry(SSModule callingMod, int tNbr, double pTime, int flags)
  {
    if (ftable[tNbr] == null)
      return 0.0;

    if ((flags & FW_Pin) != 0) {
      if (pTime < 0)
        pTime = 0;
      else if (pTime > 1)
        pTime = 1;
    }
    else if ((flags & FW_Wrap) != 0) {
      if (pTime < 0)
        pTime = -pTime;
      pTime -= (int) pTime;
    }
    else {
      if (pTime > 1.0 || pTime < 0.0 || ftable[tNbr] == null)
        return 0.0;
    }
    if ((flags & FW_Interp) != 0)
      return ftable[tNbr].RetrieveTableValueI(pTime);
    else
      return ftable[tNbr].RetrieveTableValue(pTime);
  }

  public SSFTable(ModList mList)
  {
    super(MT_FTable, mList);
    tabExp = InitExp("sin(t*2*pi)");
    tabNbr = 0;
    tabSize = 100;
    table = null;
    DescribeLink(0, "Default Signal", "sig");
    DescribeLink(1, "Alt Signal #1", "sig1");
    DescribeLink(2, "Alt Signal #2", "sig2");
    DescribeLink(3, "Alt Signal #3", "sig3");
  }

  void Copy(SSModule ss)
  {
    SSFTable sa = (SSFTable) ss;
    super.Copy(ss);
    CopyExp(sa.tabExp, tabExp);
    this.tabSize = sa.tabSize;
    this.tabNbr = sa.tabNbr;
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("FTAB " + tabNbr + " " + tabSize);
    ar.println("FTABE " + tabExp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    String[] tokens = getTokens(ar,"FTAB");
    tabNbr = Integer.parseInt(tokens[1]);
    tabSize = Integer.parseInt(tokens[2]);
    tabExp = LoadExp(ar,"FTABE");
  }

  double GenerateOutput(SSModule callingMod)
  {
    double retVal = RetrieveTableValue(parList.pTime);
    lastRightSample = retVal;
    return retVal;
  }

  double GenerateOutputTime(SSModule callingMod, double pTime)
  {
    return RetrieveTableValue(pTime);
  }

  double RetrieveTableValue(double pTime)
  {
    int  index;
    index = (int) (pTime * (tabSize-1));
    if (index < 0)
      index = 0;
    else if (index >= tabSize)
      index = tabSize - 1;
    return table[index];
  }

  double RetrieveTableValueI(double pTime)
  {
    double findex,frac;
    int  index,index2;
    findex = pTime * (tabSize-1);
    index = (int) findex;
    index2 = index+1;
    if (index >= tabSize-1)
      return table[tabSize-1];
    frac = findex - (int) findex;
    // Interpolate values
    return (1.0-frac)*table[index] + frac*table[index2];
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    table = new double[tabSize];
    if (table == null) {
      System.out.println("Null Function Table (out of mem?)");
      parList.itsOwner.AbortSynthesis();
      return;
    }
    ftable[tabNbr] = this;
  }

  void FillTable(SSModule callingMod)
  {
    if (table != null) {
      parList.PushTime(0.0);
      ResetExp(tabExp, callingMod);
      for (int i = 0; i < tabSize; ++i) {
        parList.pTime = i/(double)tabSize;
        table[i] = SolveExp(tabExp, callingMod);
      }
      parList.PopTime();
    }
  }

  void CleanUp()
  {
    table = null;
    ftable[tabNbr] = null;
    super.CleanUp();
  }
}
