
public class SSInverter extends SSModule implements SydConstants
{
  public SSInverter(ModList mList)
  {
    super(MT_Inverter, mList);
    DescribeLink(0, "Signal to Invert", "sig");
  }

	double GenerateOutput(SSModule callingMod)
	{
		double retVal = - MixInputs(-1, callingMod);
    lastRightSample = -lastRightInput;
    return retVal;
	}

}

