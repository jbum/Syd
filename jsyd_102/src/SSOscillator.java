import java.util.*;
import java.io.*;


public class SSOscillator extends SSModule implements SydConstants
{
  static final int WT_Sine         = 0;
  static final int WT_Sawtooth     = 1;
  static final int WT_Square       = 2;
  static final int WT_Triangle     = 3;
  static final int WT_BLSquare     = 4;
  static final int WT_Expression   = 5;
  static final int WT_NbrWaveTypes = 6;

  // static
  public int     waveType;
  public ExpRec  wExp, fExp, aExp, pExp;

  // dynamic
  double   waveInc;
  double   lastTime,lastOutput;

  public SSOscillator(ModList mList)
  {
    super(MT_Oscillator, mList);
    wExp = InitExp("");
    fExp = InitExp("440");
    aExp = InitExp("1");
    pExp = InitExp("0");
    waveInc = 0;
    waveType = WT_Sine;
    DescribeLink(0, "Amplitude Modulation", "am");
    DescribeLink(1, "Frequency Modulation", "fm");
    DescribeLink(2, "FM Gain", "fmgain");
    DescribeLink(3, "AM Gain", "amgain");
  }

  double GenerateOutput(SSModule callingMod)
  {
    double v, bf; // fm,fmw,amw;

    if (parList.pTime == lastTime)
      return lastOutput;

    lastTime = parList.pTime;

    bf = SolveExp(fExp, callingMod);

    // Compute waveInc = position in wave from 0-1
    // waveInc is current position in wave from 0-1
    // timeInc is how much time has passed
    // bf is frequency

    // JAB - fixed to output -1 to 1
    // Note: timeInc = 1 / SampleRate
    waveInc += (bf > Double.MAX_VALUE? 0 : parList.itsOwner.timeInc * bf);
    if (waveInc < 0)
      waveInc -= Math.floor(waveInc);
    waveInc -= (int) waveInc;

    switch (waveType) {
    case WT_Sine:
      try {
        v = WaveTables.gSinTable[(int)(waveInc*WaveTables.WaveTableSize)];
      }
      catch (Exception e) {
        // System.out.println("SSOsc: Problem with waveInc: " + waveInc + ", " + e.toString());
        v = 0;
        waveInc = 0;
      }
      break;
    case WT_Sawtooth:
      // v = WaveTables.gSawTable[(int)(waveInc*WaveTables.WaveTableSize)];
      v = (waveInc*2)-1;
      break;
    case WT_Square:
      v = waveInc < .5 ? 1.0 : -1.0;
      break;
    case WT_Triangle:
      v = waveInc < .5 ? waveInc*4-1 : (4-4*waveInc)-1;
      break;
    case WT_BLSquare:
      v = WaveTables.gSquareTable[(int)(waveInc*WaveTables.WaveTableSize)];
      break;
    case WT_Expression:
      parList.PushTime(waveInc);
      v = SolveExp(wExp, callingMod);
      parList.PopTime();
      break;
    default:
      v = 0;
      break;
    }

    v *= SolveExp(aExp, callingMod);
    lastOutput = v;
    lastRightSample = v;
    return v;
  }

  void Reset(SSModule callingMod)
  {
    if (PatchOwner.gVerbose > 1)
     System.out.println("OSC: WaveType: " + waveType);

    super.Reset(callingMod);
    waveInc = ResetExp(pExp, callingMod);  // phase
    waveInc -= (int) waveInc;

    ResetExp(fExp, callingMod);
    ResetExp(aExp, callingMod);
    ResetExp(wExp, callingMod);
    lastTime = lastOutput = -1;
    // if (gVerbose > 1)
    //  System.out.println("OSC: WaveType: " + waveType);
  }

  void Copy(SSModule ss)
  {
    SSOscillator  osc = (SSOscillator) ss;
    super.Copy(ss);
    waveInc = osc.waveInc;
    waveType = osc.waveType;
    CopyExp(osc.fExp, fExp);
    CopyExp(osc.aExp, aExp);
    CopyExp(osc.pExp, pExp);
    CopyExp(osc.wExp, wExp);
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("OSCI " + waveType);
    ar.println("OSCF " + fExp.exp);
    ar.println("OSCA " + aExp.exp);
    ar.println("OSCP " + pExp.exp);
    if (waveType == WT_Expression)
      ar.println("OSCW " + wExp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    waveType = Integer.parseInt(getFirstToken(ar,"OSCI"));
    // System.out.println("OSC: Parsing expressions");
    fExp = LoadExp(ar,"OSCF");
    aExp = LoadExp(ar,"OSCA");
    pExp = LoadExp(ar,"OSCP");
    if (waveType == WT_Expression) {
      wExp = LoadExp(ar,"OSCW");
    }
  }

}


