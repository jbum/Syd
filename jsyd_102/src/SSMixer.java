import java.util.*;


public class SSMixer extends SSModule implements SydConstants
{
  public SSMixer(ModList mList)
  {
    super(MT_Mixer, mList);
    DescribeLink(0, "Signal to Mix", "sig");
  }

  double GenerateOutput(SSModule callingMod)
  {
    double retVal = MixInputsAtten(-1, callingMod);
    lastRightSample = lastRightInput;
    return retVal;
  }

  // Attenuated Mix (used by Mixer module)
  double MixInputsAtten(int type, SSModule callingMod)
  {
    this.callingMod = callingMod;

    int i = 0;
    int n = 0;
    double v = 0.0, vR = 0.0;
    Enumeration e = inputs.elements();
    while (e.hasMoreElements())
    {
      ModInput  inp = (ModInput) e.nextElement();
      if (type == -1 || inp.inputType == type) {
        v += inp.link.GenerateOutput(this);
        vR += inp.link.getRightSample();
        n++;
      }
      ++i;
    }
    if (n > 1) {
      v /= n;
      vR /= n;
    }
    lastRightInput = vR;
    return  v;
  }
}

