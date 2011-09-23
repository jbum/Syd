import java.io.*;

public class SSThreshhold extends SSModule implements SydConstants
{
  public ExpRec    cutOffExp = InitExp("0.5");


  public SSThreshhold(ModList mList)
  {
    super(MT_Threshhold, mList);
    DescribeLink(0, "Signal for Threshhold", "sig");
  }

  void  Copy(SSModule ss)
  {
    SSThreshhold sa = (SSThreshhold) ss;
    super.Copy(ss);
    CopyExp(sa.cutOffExp, cutOffExp);
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("THR " + cutOffExp.exp);
  }


  void Load(BufferedReader ar) throws IOException
  {
   cutOffExp = LoadExp(ar,"THR");
  }

	double GenerateOutput(SSModule callingMod)
	{
    double cutOff;
    cutOff = SolveExp(cutOffExp,callingMod);
    double retVal = MixInputs(-1,callingMod) >= cutOff? 1.0 : 0.0;
    lastRightSample = lastRightInput >= cutOff? 1.0 : 0.0;
    return retVal;
	}

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    ResetExp(cutOffExp, callingMod);
  }

}


