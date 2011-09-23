import java.io.*;
import java.util.*;

public class SSFolder extends SSModule implements SydConstants
{
  public String  instFileSpec = "";
  ModList instr = null;

  SSFolder(ModList mList)
  {
    super(MT_Folder, mList);
    DescribeLink(0, "Folder Input", "f0");
    DescribeLink(1, "Folder Input #1", "f1");
    DescribeLink(2, "Folder Input #2", "f2");
    DescribeLink(3, "Folder Input #3", "f3");
    DescribeLink(4, "Folder Input #4", "f4");
    DescribeLink(5, "Folder Input #5", "f5");
    DescribeLink(6, "Folder Input #6", "f6");
    DescribeLink(7, "Folder Input #7", "f7");
    DescribeLink(8, "Folder Input #8", "f8");
    DescribeLink(9, "Folder Input #9", "f9");
    DescribeLink(10, "Folder Input #10", "f10");
    DescribeLink(11, "Folder Input #11", "f11");
    DescribeLink(12, "Folder Input #12", "f12");
    DescribeLink(13, "Folder Input #13", "f13");
    DescribeLink(14, "Folder Input #14", "f14");
    DescribeLink(15, "Folder Input #15", "f15");
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    instr = new ModList(parList.itsOwner, instFileSpec);
    if (instr== null)
    {
      System.out.println("Can't find Instrument: " + instFileSpec);
      parList.itsOwner.AbortSynthesis();
    }
    else if (instr.mods.size() == 0)
    {
      System.out.println("Null Instrument");
      parList.itsOwner.AbortSynthesis();
    }
    instr.ResetInstruments(this);
  }

  double GenerateOutput(SSModule callingMod)
  {
    double retVal;
    this.callingMod = callingMod;
    if (instr != null) {
      if (instr.mods.firstElement() != null) {
        retVal = ((SSModule) instr.mods.firstElement()).GenerateOutputTime(this,parList.pTime);
        lastRightSample = ((SSModule) instr.mods.firstElement()).getRightSample();
      }
      else {
        // !!! Abort Synthesis
        parList.itsOwner.AbortSynthesis();
        System.out.println("Null Instrument");
        retVal = lastRightSample =0.0;
      }
    }
    else
        retVal = lastRightSample =0.0;
    return retVal;
  }

  double GetFolderSig(int n) throws Exception
  {
    if (CountInputs(n) > 0) {
      return MixInputs(n, callingMod);
    }
    else
     throw new Exception("No Inputs To Folder");
  }


  void CleanUp()
  {
    instr = null;
    super.CleanUp();
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("CSCO " + instFileSpec); // yes, it's CSCO - historical accident
  }

  void Load(BufferedReader ar) throws IOException
  {
    instFileSpec = getFirstToken(ar,"CSCO"); // yes, it's CSCO - seems odd but that's what syd uses
  }

  void Copy(SSModule ss)
  {
    SSFolder sf = (SSFolder) ss;
    super.Copy(ss);
    instFileSpec = sf.instFileSpec;
  }

}

