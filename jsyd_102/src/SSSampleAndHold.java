import java.io.*;

public class SSSampleAndHold extends SSModule implements SydConstants
{
  boolean lastTrigger = false;
  double lastOutput = 0.0;


  public SSSampleAndHold(ModList mList)
  {
    super(MT_SampleAndHold, mList);
    DescribeLink(0, "Sampled Signal", "sig");
    DescribeLink(1, "Trigger Signal", "trig");
  }


	double GenerateOutput(SSModule callingMod)
	{
    // Oscillators don't work properly unless this is called
    // everytime (although it IS inefficient...)
    double v = MixInputs(0,callingMod);

    if (MixInputs(1,callingMod) >= .5) {
      if (!lastTrigger) {
        lastOutput = v;
        lastRightSample = v;
      }
      lastTrigger = true;
    }
    else
      lastTrigger = false;

    return lastOutput;
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    lastTrigger = false;
    lastOutput = 0.0;
    lastRightSample = 0.0;
  }

}


