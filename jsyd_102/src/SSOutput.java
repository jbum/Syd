import java.io.*;
import java.util.*;


public class SSOutput extends SSModule implements SydConstants
{
  static final int OM_MEMORY  = 0;
  static final int OM_AIFF    = 1;
  static final int OM_WAVE    = 2;

  public double  sampleRate = 22050;
  public double  sampleDuration = 2.0;
  public int     outputType = OM_MEMORY;
  public int     isStereo = 0;
  public String  outFileSpec = "untitled.wav";

  public SSOutput(ModList mList)
  {
    super(MT_Output, mList);
    DescribeLink(0, "Output Signal", "sig");
  }

/*
  double GenerateOutput(SSModule callingMod)
  {
    return MixInputs(-1, callingMod);
  }

  // Override - output module keeps stereo separate
  double GenerateStereoOutput(SSModule callingMod)
  {
    double retVal = MixInputs(-1, callingMod);
    lastRightSample = lastRightInput;
    return retVal;
  }
*/

  static  int recurseCheck = 0;

  double GetInstParameter(int n)
  {
    if (callingMod != null && callingMod != this)
    {
      ++recurseCheck;
      if (recurseCheck > 32) {
        System.out.println("Recursion Problem");
        return 0;
      }
      double v = callingMod.GetInstParameter(n);
      --recurseCheck;
      return v;
    }
    else {
        switch (n) {
        case 0:   return parList.itsOwner.gTime;      // Note time
        case 1:   return 1.0;                         // Instrument #
        case 2:   return 0.0;                         // Start Time
        case 3:   return parList.itsOwner.mainInst.sampleDuration; // Duration
        case 4:   return 1.0;                         // Amp (opt)
        case 5:   return 440.0;                       // Pitch (opt)
        default:  return 0.0;                           // (opt)
      }
    }
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    if (isStereo == 0) // keep it backward compatible if possible...
    {
      ar.println("OUTPUT " + sampleDuration + " " +
                  sampleRate + " " +
                  outputType + " 0 0 " +
                  outFileSpec);
    }
    else {
      ar.println("OUTS " + isStereo + " " +
                  sampleDuration + " " +
                  sampleRate + " " +
                  outputType + " " +
                  outFileSpec);
    }
  }

  void Load(BufferedReader ar) throws IOException
  {
    double        sd=0,sr=0;

    String[] tokens = getTokens(ar,"OUT");

    if (tokens[0].equals("OUT"))
    {
      sampleDuration = Double.parseDouble(tokens[1]);
      sampleRate = Double.parseDouble(tokens[2]);
      outFileSpec = "untitled.wav";
      outputType = OM_WAVE;
      isStereo = 0;
    }
    else if (tokens[0].equals("OUTS"))
    {
      isStereo = Integer.parseInt(tokens[1]);
      sampleDuration = Double.parseDouble(tokens[2]);
      sampleRate = Double.parseDouble(tokens[3]);
      outputType = Integer.parseInt(tokens[4]);
      outFileSpec = tokens[5];
    }
    else { // OUTPUT...
      sampleDuration = Double.parseDouble(tokens[1]);
      sampleRate = Double.parseDouble(tokens[2]);
      outputType = Integer.parseInt(tokens[3]);
      // st.nextToken(); // vRefNum
      // st.nextToken(); // parID
      outFileSpec = tokens[6];
      isStereo = 0;

    }
    // CSYD 5-29-06
    // FORCE WAVE OUTPUT IF MEMORY TYPE IS USED
    if (outputType == OM_MEMORY) {
      System.out.println("Redirecting patch output from memory to file");
      outputType = OM_WAVE;
      outFileSpec = "untitled.wav";
    }

    System.out.println("G: " + sampleDuration + ", " + sampleRate);

    // CSYD 5-29-06 - override output options if specified on command line
    if (PatchOwner.gDuration > 0 && PatchOwner.gDuration != sampleDuration)
    {
        System.out.println("Overriding sample duration " + sampleDuration + " -> " + PatchOwner.gDuration + " seconds");
        sampleDuration = PatchOwner.gDuration;
    }
    if (PatchOwner.gSampleRate > 0 && PatchOwner.gSampleRate != sampleRate)
    {
        System.out.println("Overriding sample rate " + sampleRate + " -> " + PatchOwner.gSampleRate);
        sampleRate = PatchOwner.gSampleRate;
    }
    if (PatchOwner.gOutputFilename.length() > 0)
    {
        outFileSpec = PatchOwner.gOutputFilename;
        if (outFileSpec.endsWith(".aif") || outFileSpec.endsWith(".aiff"))
        {
          System.out.println("Overriding output file to AIFF");
          outputType = OM_AIFF;
        }
        else
        {
          System.out.println("Overriding output file to WAV");
          outputType = OM_WAVE;
        }
    }
    // System.out.println("Outputting to " + outFileSpec);
  }
}
