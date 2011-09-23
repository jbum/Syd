import java.io.*;
import java.util.*;

public class SSSampleFile extends SSModule implements SydConstants
{
  static final int SF_Interpolate = 1;
  static final int SF_Retrograde = 2; // not implemented yet

  public String  sampleFileSpec = "";
  public ExpRec  timeScaleExp = InitExp("1.0");
  public int     flags = 0;

  SSStoreFile fileStore = null;

  SSSampleFile(ModList mList)
  {
    super(MT_SampleFile, mList);
    DescribeLink(0, "Invalid Input", "???");
  }

  void Reset(SSModule callingMod)
  {
    int oe;
    super.Reset(callingMod);
    fileStore = new SSStoreFile();
    if ((oe = fileStore.SetReadFileSpec(sampleFileSpec)) != SSStoreFile.noErr)
      System.out.println("Error Opening Sample File");
    ResetExp(timeScaleExp, callingMod);
  }

  double GenerateOutput(SSModule callingMod)
  {
    double v;
    this.callingMod = callingMod;
    double timeScale = SolveExp(timeScaleExp, callingMod);
    if (fileStore != null) {
      int n = (int) (parList.pTime * fileStore.sampleRate * timeScale);
      if (fileStore.nbrChannels == 2) {
        v = fileStore.GetSampleF(n*2);
        lastRightSample = fileStore.GetSampleF(n*2+1);
      }
      else {
        v = fileStore.GetSampleF(n*fileStore.nbrChannels);
      }
    }
    else
      v = lastRightSample = 0.0;
    return v;
  }

  void CleanUp()
  {
    fileStore = null;
    super.CleanUp();
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("FSAMP " + sampleFileSpec + " 0 0 " + flags);
    ar.println("FSAMPT " + timeScaleExp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    String[] tokens = getTokens(ar,"FSAMP");
    sampleFileSpec = tokens[1];
    // skip 2,3 (vrefnum, parid)
    flags = Integer.parseInt(tokens[4]);
    timeScaleExp = LoadExp(ar,"FSAMPT");
  }

  void Copy(SSModule ss)
  {
    SSSampleFile sf = (SSSampleFile) ss;
    super.Copy(ss);
    sampleFileSpec = sf.sampleFileSpec;
    flags = sf.flags;
    CopyExp(sf.timeScaleExp, timeScaleExp);

  }

}

