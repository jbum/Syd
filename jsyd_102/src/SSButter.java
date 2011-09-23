import java.io.*;
import java.util.*;

public class SSButter extends SSModule implements SydConstants
{
  static final int BT_LoPass = 0;
  static final int BT_HiPass = 1;
  static final int BT_BandPass = 2;
  static final int BT_BandReject = 3;

  static final double ROOT2 = Math.sqrt(2);

  public ExpRec    freqExp = InitExp("1200"),
                   bwExp = InitExp("1200*0.1");
  public int       filterType = BT_BandPass;

  // Dynamic
  double   oldFreq,oldBW;
  double   pidsr,a0,a1,a2,b1,b2,y1,y2;


  public SSButter(ModList mList)
  {
    super(MT_Butter, mList);
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
    SSButter sa = (SSButter) ss;
    super.Copy(ss);
    CopyExp(sa.freqExp, freqExp);
    CopyExp(sa.bwExp, bwExp);
    this.filterType = sa.filterType;
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("BUTTER " + filterType);
    ar.println("BUTTERF " + freqExp.exp);
    ar.println("BUTTERB " + bwExp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    filterType = Integer.parseInt( getFirstToken(ar,"BUTTER"));
    freqExp = LoadExp(ar,"BUTTERF");
    bwExp = LoadExp(ar,"BUTTERB");
  }

  double GenerateOutput(SSModule callingMod)
  {
    double t,y;

    double freq = SolveExp(freqExp, callingMod);
    double bw = SolveExp(bwExp, callingMod);

    double v = MixInputs(0, callingMod);

    if (freq != oldFreq || bw != oldBW) {
      double c,d;
      switch (filterType) {
      case BT_LoPass:
        c = 1.0 / Math.tan(pidsr * freq);
        a0 = 1.0 / (1.0 + ROOT2 * c + c * c);
        a1 = a0 + a0;
        a2 = a0;
        b1 = 2.0 * (1.0 - c*c) * a0;
        b2 = (1.0 - ROOT2 * c + c * c) * a0;
        break;
      case BT_HiPass:
        c = Math.tan(pidsr * freq);
        a0 = 1.0 / (1.0 + ROOT2 * c + c * c);
        a1 = -2.0 * a0;
        a2 = a0;
        b1 = 2.0 * (c*c - 1.0) * a0;
        b2 = (1.0 - ROOT2 * c + c * c) * a0;
        break;
      case BT_BandPass:
        c = 1.0 / Math.tan(pidsr * bw);
        d = 2.0 * Math.cos(2.0 * pidsr * freq);
        a0 = 1.0 / (1.0 + c);
        a1 = 0.0;
        a2 = -a0;
        b1 = - c * d * a0;
        b2 = (c - 1.0) * a0;
        break;
      case BT_BandReject:
        c = Math.tan(pidsr * bw);
        d = 2.0 * Math.cos(2.0 * pidsr * freq);
        a0 = 1.0 / (1.0 + c);
        a1 = - d * a0;
        a2 = a0;
        b1 = a1;
        b2 = (1.0 - c) * a0;
        break;
      }
      oldBW = bw;
      oldFreq = freq;
    }
    if (bw == 0.0) {
      t = y = 0;
    }
    else {
      t = v - b1*y1 - b2*y2;      // First Section (IIR)
      y = t * a0 + a1*y1 + a2*y2; // Second Section (FIR)
    }
    y2 = y1;
    y1 = t;
    lastRightSample = y;
    return y;
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    ResetExp(freqExp, callingMod);
    ResetExp(bwExp, callingMod);
    oldFreq = oldBW = -1.0;
    a0 = a1 = a2 = b1 = b2 = y1 = y2 = 0;
    pidsr = Math.PI * parList.itsOwner.timeInc;  // pi / samplerate
  }

}

