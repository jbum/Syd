public class RenderTest implements SydConstants
{
  static  PatchOwner  itsPatch;
  static  String      sydSpec;
  static  double      gDuration = 0;
  static  double      gSampleRate = 0;
  static  String      gPatchFilename = null;
  static  String      gOutputFilename = "";

  public static void SyntaxExit()
  {
    System.out.println("RenderTest [options] patchfile\n");
    System.out.println("Options:\n");
    System.out.println("    -o file        output filename (supports .wav and .aif)");
    System.out.println("    -d duration    duration (seconds or MM:SS)");
    System.out.println("    -s samplerate  sampling rate");
    System.out.println("    -v #           verbosity level");
    System.out.println("    -x             score can override duration");
//    System.out.println("    -q             quiet output (same as -v 0)");
//    System.out.println("    -x             allow score to override duration");
    System.exit(1);
  }

	public static void main(String[] args)
	{
		System.out.println("Render Test");
    if (args != null) {
       for (int i = 0; i < args.length; ++i)
       {
          System.out.println("Arg: " + args[i]);
          if (args[i].startsWith("-"))
          {
            if (args[i].equals("-d")) {
              gDuration = Double.parseDouble(args[++i]);
            }
            else if (args[i].equals("-s")) {
              gSampleRate = Double.parseDouble(args[++i]);
            }
            else if (args[i].equals("-o")) {
              gOutputFilename = args[++i];
            }
            else if (args[i].equals("-v")) {
              PatchOwner.gVerbose = Integer.parseInt(args[++i]);
            }
            else if (args[i].equals("-x")) {
              PatchOwner.gDurationOverride = true;
            }
            else {
              SyntaxExit();
            }
          }
          else if (gPatchFilename == null)
          {
            gPatchFilename = args[i];
          }
          else {
            SyntaxExit();
          }
       }
    }
    if (gPatchFilename == null)
       SyntaxExit();
    sydSpec = gPatchFilename;


   PatchOwner.gOutputFilename = gOutputFilename;
   PatchOwner.gDuration = gDuration;
   PatchOwner.gSampleRate = gSampleRate;
   PatchOwner.sydSpec = sydSpec;

   itsPatch = new PatchOwner();


    System.out.println("Reading Patch\n");
    itsPatch.mainInst.OpenSpec(sydSpec);
    System.out.println("Beginning Synthesis\n");
    try {
      itsPatch.PerformSynthesis();
      while (itsPatch.sampleNbr < itsPatch.nbrSamples && itsPatch.windowState == WS_Synthesize)
      {
        if (itsPatch.sampleNbr > 0)
          System.out.println(itsPatch.sampleNbr*100.0/itsPatch.nbrSamples + "%");
        itsPatch.SynthIdle();
      }
      if (itsPatch.sampleNbr < itsPatch.nbrSamples)
        itsPatch.CleanUpSynthesis();
    }
    catch (Exception e)
    {
      System.out.println("Uncaught exception during synthesis: " + e.toString());
      e.printStackTrace();
    }
 }
}