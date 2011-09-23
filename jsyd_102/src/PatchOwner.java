import java.io.*;
import java.text.NumberFormat;

public class PatchOwner implements SydConstants
{

  // These are set from command line parameters
  static double   gDuration;
  static double   gSampleRate;
  static String   gOutputFilename;
  static int      gVerbose = 1;
  static boolean  gDurationOverride = false;

  static String   sydSpec;      // patch filename..
  static NumberFormat  pFormat= NumberFormat.getNumberInstance();

  ExpMgr  expMgr;
  int     windowState;
  double  gTime, timeInc;
  ModList mainInst;
  double[]  globalVars;
  SSModule    outMod;
  SSStorage   storageMod;
  SSStoreMem  storeMem;
  SSStoreFile storeFile;
  boolean   storeToFile;
  int       nbrSamples;
  int       sampleNbr;
  long      startSynthTime;
  long      gLastFreeMem;
  long      gLastDelta;
  boolean   listenFlag, isGraphing;

  public PatchOwner()
  {
    globalVars = new double[MaxGlobals];
    expMgr = new ExpMgr();
    storeFile = new SSStoreFile();
    storeMem = new SSStoreMem();
    storageMod = storeMem;
    storeToFile = false;
    outMod = null;
    gTime = 0;
    timeInc = 0;
    windowState = WS_Idle;
    mainInst = null;
    listenFlag = false;
    isGraphing = false;

    InitSynth();
  }

  void InitSynth()
  {
    DisposeAllModules();
    mainInst = new ModList(this);

    windowState = WS_Idle;
    SSModule outMod = new SSOutput(mainInst);
    mainInst.mods.add(outMod);
  }

  void DisposeAllModules()
  {
    if (mainInst != null)
      mainInst.DisposeModuleList();
  }

  void AssignGlobal(int gNbr, double value)
  {
    if (gNbr >= 0 && gNbr < MaxGlobals)
      globalVars[gNbr] = value;
  }

  double RetrieveGlobal(int gNbr)
  {
    if (gNbr >= 0 && gNbr < MaxGlobals)
      return globalVars[gNbr];
    else
      return 0.0;
  }

  void InitGlobals()
  {
    for (int i = 0; i < MaxGlobals; ++i) {
      globalVars[i] = 0;
    }
  }

  void AbortSynthesis() // Trigger Abort
  {
    windowState = WS_Abort;
  }

  void PerformSynthesis() throws IOException
  {
    WaveTables.InitWaveTables();

    System.out.println("Starting Synthesis...");

    InitGlobals();

    outMod = mainInst.GetOutputModule();
    if (outMod == null)
      return;

    SSOutput  myOutMod = (SSOutput) outMod;

    mainInst.sampleRate = myOutMod.sampleRate;
    mainInst.sampleDuration = myOutMod.sampleDuration;

    // System.out.println("Setting output type: " + myOutMod.outFileSpec);
    SetOutputType(myOutMod.outputType, myOutMod.isStereo, myOutMod.outFileSpec);
    // Was beginning output here...

    gTime = 0.0;
    timeInc = 1 / mainInst.sampleRate;

    windowState = WS_Synthesize;

    nbrSamples = (int) (mainInst.sampleRate * mainInst.sampleDuration);
    if (PatchOwner.gVerbose > 1)
      System.out.println("PatchOwner: PerformSynthesis: ResetInstruments");
    mainInst.ResetInstruments(outMod);
    nbrSamples = (int) (mainInst.sampleRate * mainInst.sampleDuration);

    // Moved this here, so that score generation has a chance to reset score
    if (storageMod.StartStorage(nbrSamples, mainInst.sampleRate,listenFlag,myOutMod.isStereo != 0? 2 : 1) != 0)
      return;

    sampleNbr = 0;
    startSynthTime = System.currentTimeMillis();
  }

  void SetOutputType(int fileType, int isStereo, String fsSpec) throws IOException
  {
    if (fileType != SSOutput.OM_MEMORY) {
      if (!storeToFile) {
        storeFile.CloseFile();
        // storeFile->GetFileSpec();
        storeFile.SetWriteFileSpec(fileType, isStereo, fsSpec);
        // if (storeFile.Import(storageMod) == 0) {
          storageMod = storeFile;
          storeToFile = true;
        // }
      }
      else {
        storeFile.CloseFile();
        storeFile.SetWriteFileSpec(fileType, isStereo, fsSpec);
        // storeFile->GetFileSpec();
        storeFile.nbrFrames = 0;
      }
    }
    else {
      if (storeToFile) {
        // if (storeMem.Import(storageMod) == 0) {
          storageMod = storeMem;
          storageMod.nbrChannels = isStereo > 0? 2 : 1;
          storeToFile = false;
        // }
      }
    }
  }

  void SynthIdle()
  {
    long  t = System.currentTimeMillis();

    if (((SSOutput) outMod).isStereo != 0)
    {
      double  vL, vR;
      while (windowState == WS_Synthesize && sampleNbr < nbrSamples &&
          ( (sampleNbr & 0x0FF) > 0 || // this line helps minimize calls to currentTimeMillis
             System.currentTimeMillis() - t < 5000)
          )
      {
        gTime = (double) sampleNbr / mainInst.sampleRate;
        vL = ((SSOutput) outMod).GenerateOutputTime(null, gTime);  // will output from 0 - 1
        vR = ((SSOutput) outMod).getRightSample();
        storageMod.StoreSampleFS(vL,vR);
        ++sampleNbr;
      }
    }
    else {
      double v;
      while (windowState == WS_Synthesize && sampleNbr < nbrSamples &&
          ( (sampleNbr & 0x0FF) > 0 || // this line helps minimize calls to currentTimeMillis
             System.currentTimeMillis() - t < 5000)
          )
      {
        gTime = (double) sampleNbr / mainInst.sampleRate;
        v = ((SSOutput) outMod).GenerateOutputTime(null, gTime);  // will output from 0 - 1
        storageMod.StoreSampleF(v);
        ++sampleNbr;
      }
    }

    if (sampleNbr >= nbrSamples || windowState != WS_Synthesize)
      CleanUpSynthesis();
  }

  void CleanUpSynthesis()
  {
    if (windowState == WS_Idle)
      return;
    System.out.println("Cleanup Start");
    storageMod.StopStorage();
    windowState = WS_Idle;
    mainInst.CleanUpInstruments();

    System.out.println("Cleanup ...Ending Synthesis");

    // Display and play samples
    System.out.println("Elapsed: " +
                      pFormat.format((System.currentTimeMillis() - startSynthTime) / 1000.0) +
                       " secs");
  }

}
