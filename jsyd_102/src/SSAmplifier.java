import java.io.*;
import java.util.*;

public class SSAmplifier extends SSModule implements SydConstants
{
  public ExpRec      scaleExp = InitExp("1.0"),
                     offsetExp = InitExp("0.0"),
                     panExp   = InitExp("0.0");

  static final int SIG_Sig = 0,
                   SIG_Ctl = 1,
                   SIG_Ctl1 = 2,
                   SIG_Ctl2 = 3,
                   SIG_Ctl3 = 4;


  public SSAmplifier(ModList mList)
  {
    super(MT_Amplifier, mList);
    DescribeLink(SIG_Sig, "Signal to Amplify", "sig");
    DescribeLink(SIG_Ctl, "Control Signal", "ctl");
    DescribeLink(SIG_Ctl1, "Control Signal 1", "ctl1");
    DescribeLink(SIG_Ctl2, "Control Signal 2", "ctl2");
    DescribeLink(SIG_Ctl3, "Control Signal 3", "ctl3");
  }

  void  Copy(SSModule ss)
  {
    SSAmplifier sa = (SSAmplifier) ss;
    super.Copy(ss);
    CopyExp(sa.scaleExp, scaleExp);
    CopyExp(sa.offsetExp, offsetExp);
    CopyExp(sa.panExp, panExp);
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    if (!panExp.exp.equals("0.0"))
      ar.println("AMPP " + panExp.exp);
    ar.println("AMPS " + scaleExp.exp);
    ar.println("AMPO " + offsetExp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    int         wType=0;
    String[] tokens = getTokens(ar,"AMP");

    if (tokens[0].equals("AMP")) {
      // Old Style Input
      scaleExp = InitExp(tokens[1]);
      offsetExp = InitExp(tokens[2]);
    }
    else {

      if (tokens[0].equals("AMPP"))  // Pan was added later...
      {
        panExp = InitExp(tokens[1]);
        scaleExp = LoadExp(ar, "AMPS");
      }
      else // token must equal AMPS
        scaleExp = InitExp(tokens[1]);
      offsetExp = LoadExp(ar,"AMPO");
    }
  }

  double GenerateOutput(SSModule callingMod)
  {
    double  scale,offset, pan, vL, vR;
    scale = SolveExp(scaleExp, callingMod);
    offset = SolveExp(offsetExp, callingMod);
    pan = SolveExp(panExp, callingMod);

    // System.out.println("Resolving " + panExp.exp + " = " + pan + ", " + scaleExp.exp + " = " + scale);
    // System.out.println("samp parlist->pTime = " + parList.pTime + ", " + panExp.exp + " = " + pan);

    vL =  MixInputs(SIG_Sig, callingMod)*scale + offset;
    vR = lastRightInput*scale + offset;

    if (pan < -1)
      pan = -1;
    else if (pan > 1)
      pan = 1;

    if (pan < 0.0)
    {
      vR *= pan+1;
    }
    else if (pan > 0.0)
    {
      vL *= 1-pan;
    }
    lastRightSample = vR;

    return vL;
  }


  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    ResetExp(scaleExp, callingMod);
    ResetExp(offsetExp, callingMod);
    ResetExp(panExp, callingMod);
  }

}

