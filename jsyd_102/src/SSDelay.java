import java.io.*;
import java.util.*;

public class SSDelay extends SSModule implements SydConstants
{
  static final int DF_Recursive = 1;
  static final int SIG_Sig = 0,
                   SIG_Ctl = 1,
                   SIG_Ctl1 = 2,
                   SIG_Ctl2 = 3,
                   SIG_Ctl3 = 4;

  public ExpRec    delayExp = InitExp("0.1"),
                   a0Exp = InitExp("1.0"),
                   a1Exp = InitExp("0.5");
  public int       flags = DF_Recursive;

  // Dynamic Variables
  int        allocDelaySamples = 0;
  int        delayCounter = 0,delayIncrement = 0;
  double[]   dBuf = null;
  double     delay,a0,a1,oldDelay;

  public SSDelay(ModList mList)
  {
    super(MT_Delay, mList);

    DescribeLink(SIG_Sig, "Signal to Delay", "sig");
    DescribeLink(SIG_Ctl, "Control Signal", "ctl");
    DescribeLink(SIG_Ctl1, "Control Signal 1", "ctl1");
    DescribeLink(SIG_Ctl2, "Control Signal 2", "ctl2");
    DescribeLink(SIG_Ctl3, "Control Signal 3", "ctl3");
  }

  void CleanUp()
  {
    dBuf = null;
    allocDelaySamples = 0;
    super.CleanUp();
  }

  void  Copy(SSModule ss)
  {
    SSDelay sa = (SSDelay ) ss;
    super.Copy(ss);
    CopyExp(sa.delayExp, delayExp);
    CopyExp(sa.a0Exp, a0Exp);
    CopyExp(sa.a1Exp, a1Exp);
    flags = sa.flags;
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("DELF " + flags);
    ar.println("DELd " + delayExp.exp);
    ar.println("DELa0 " + a0Exp.exp);
    ar.println("DELa1 " + a1Exp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    String[] tokens = getTokens(ar,"DEL");
    if (tokens[0].equals("DEL"))
    {
      // Old Style

      double dly = Double.parseDouble(tokens[1]);
      double gain = Double.parseDouble(tokens[2]);
      int f = Integer.parseInt(tokens[3]);

      delayExp = InitExp(Double.toString(dly));
      a0Exp = InitExp(Double.toString(1.0/(1.0+gain)));
      a1Exp = InitExp(Double.toString(gain/(1.0+gain)));
      flags = f != 0? DF_Recursive : 0;
    }
    else { // DELF
      flags = Integer.parseInt(tokens[1]);
      delayExp = LoadExp(ar,"DELd");
      a0Exp = LoadExp(ar,"DELa0");
      a1Exp = LoadExp(ar,"DELa1");
    }
  }

  void   Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    delay = ResetExp(delayExp,callingMod);
    a0 = ResetExp(a0Exp,callingMod);
    a1 = ResetExp(a1Exp,callingMod);
    // !!! Find a more reliable way to compute
    allocDelaySamples = (int) Math.abs(delay * parList.itsOwner.mainInst.sampleRate) + 22000;
    delayIncrement = (int) Math.abs(delay * parList.itsOwner.mainInst.sampleRate);
    delayCounter = 0;
    dBuf = new double[allocDelaySamples];
    if (dBuf == null) {
      System.out.println("Can't allocate delay buffer");
      parList.itsOwner.AbortSynthesis();
      return;
    }
    for (delayCounter = 0; delayCounter < allocDelaySamples; ++delayCounter)
      dBuf[delayCounter] = 0.0;
    delayCounter = 0;
    oldDelay = delay;
  }

  double GenerateOutput(SSModule callingMod)
  {
    if (dBuf == null)
      return 0.0;

    double x,y;

    delay = SolveExp(delayExp,callingMod);
    a0 = SolveExp(a0Exp,callingMod);
    a1 = SolveExp(a1Exp,callingMod);

    if (oldDelay != delay) {
      int oldMax = allocDelaySamples;
      delayIncrement = (int) Math.abs(delay * parList.itsOwner.mainInst.sampleRate);
      if (delayIncrement > allocDelaySamples) {
        allocDelaySamples = delayIncrement+22000;
        double[] nb = new double[allocDelaySamples];

        if (nb == null) {
          System.out.println("Can't allocate delay buffer");
          parList.itsOwner.AbortSynthesis();
          return 0.0;
        }
        if (dBuf != null) {
          // rotate buffer
          System.arraycopy(dBuf,delayCounter, nb,0,oldMax - delayCounter);
          System.arraycopy(dBuf,0, nb,oldMax - delayCounter,delayCounter);
        }
        dBuf = nb;
        delayCounter = 0;
      }
      oldDelay = delay;
    }

    x = MixInputs(SIG_Sig, callingMod);
    y = a0*x + a1*dBuf[(delayCounter + delayIncrement) % allocDelaySamples];
    if ((flags & DF_Recursive) > 0)
      dBuf[delayCounter] = y;
    else
      dBuf[delayCounter] = x;
    delayCounter--;
    if (delayCounter < 0)
      delayCounter = allocDelaySamples-1;
    // Currently, Mono Reverb Only...
    lastRightSample =  y;
    return y;
  }


}