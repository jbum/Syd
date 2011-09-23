import java.text.NumberFormat;

public class SynthThread implements Runnable, SydConstants
{
	private	Thread itsThread;

  PatchOwner  itsPatch;
  JSydApp     itsApp;
  static NumberFormat  pFormat= NumberFormat.getPercentInstance();

	public SynthThread(JSydApp itsApp, PatchOwner itsPatch)
  {

    this.itsApp = itsApp;
    this.itsPatch = itsPatch;
    try {
        itsApp.wavePanel.InitWave(itsPatch);
        itsApp.sp.ActivateNodeEditor(null);
        itsPatch.PerformSynthesis();
        itsThread = new Thread(this);
        itsThread.start();

    }
    catch (Exception e) {
        System.out.println("Uncaught exception starting synthesis: " + e.toString());
        e.printStackTrace();
    }
  }

  public void run()
  {
    try {
      while (itsPatch.sampleNbr < itsPatch.nbrSamples && itsPatch.windowState == WS_Synthesize)
      {
        if (itsPatch.sampleNbr > 0)
          System.out.println(pFormat.format(itsPatch.sampleNbr/(double)itsPatch.nbrSamples));
        itsPatch.SynthIdle();
        itsApp.wavePanel.GraphSamples(itsPatch, itsPatch.nbrSamples);
        itsThread.sleep(10);
      }
      if (itsPatch.sampleNbr < itsPatch.nbrSamples)
        itsPatch.CleanUpSynthesis();
      if (PatchOwner.gVerbose > 1)
      {
        System.out.println("storage nbrFrames = " + itsPatch.storageMod.nbrFrames + ", nbrChannels= " + itsPatch.storageMod.nbrChannels);
      }
      itsApp.wavePanel.GraphSamples(itsPatch, itsPatch.storageMod.nbrFrames);
    }
    catch (Exception e) {
        System.out.println("Uncaught exception during synthesis: " + e.toString());
        e.printStackTrace();
    }
  }


}