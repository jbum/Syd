import java.io.*;
import java.util.*;

public class SSSmooth extends SSModule implements SydConstants
{
  // dynamic
  double lastSample = 0;
  double lastSampleR = 0;

  public SSSmooth(ModList mList)
  {
    super(MT_Smooth, mList);
    DescribeLink(0, "Signal to Smooth", "sig");
  }

  double GenerateOutput(SSModule callingMod)
  {
    double v = MixInputs(-1, callingMod);
    double av = (lastSample + v) / 2;
    lastRightSample = (lastSampleR + lastRightInput) / 2; // stereo right...
    lastSample = v;   // smooth mem left
    lastSampleR = lastRightInput; // smooth mem right
    return av;
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    lastSample = 0;
    lastSampleR = 0;
  }
}
