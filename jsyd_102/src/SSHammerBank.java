import java.util.*;
import java.io.*;


public class SSHammerBank extends SSModule implements SydConstants
{
  static final int MaxHammerBanks    = 64;
  static final int MaxKeysPerBank    = 128;
  static final int HB_SympatheticRes = 1;

  static HammerBankRec[]  hammerBanks = new HammerBankRec[MaxHammerBanks];

  public ExpRec    bNbrExp      = InitExp("0"),
            kFreqExp    = InitExp("cpsmidi(k)"),
            kAmpExp     = InitExp("1.0"),
            kAttackExp  = InitExp("0.05"),
            kDecayExp   = InitExp("0.1"),
            sustainExp  = InitExp("1.0"),
            waveformExp = InitExp("sin(t*2*pi)");

  int       bNbr = 0;
  double    sustain,
            keyNumber,
            keyInten,
            lastTime,
            lastOutput;

  HammerBankRec hb = null;  // hb in C version
  ActionRec     currentKey;

  public SSHammerBank(ModList mList)
  {
    super(MT_HammerBank, mList);
    DescribeLink(0, "Instrument 1", "i1");
    DescribeLink(1, "Instrument 2", "i2");
    DescribeLink(2, "Instrument 3", "i3");
    DescribeLink(3, "Instrument 4", "i4");
    DescribeLink(4, "Instrument 5", "i5");
    DescribeLink(5, "Instrument 6", "i6");
    DescribeLink(6, "Instrument 7", "i7");
    DescribeLink(7, "Instrument 8", "i8");
    DescribeLink(8, "Control Signal", "ctl");
    DescribeLink(9, "Alt Control Sig #1", "ctl1");
    DescribeLink(10, "Alt Control Sig #2", "ctl2");
    DescribeLink(11, "Alt Control Sig #3", "ctl3");
  }

  void InitHammerBank()
  {
    hb = null;
    if (bNbr < 0 || bNbr >= MaxHammerBanks)
      return;
    hammerBanks[bNbr] = new HammerBankRec();
    hb = hammerBanks[bNbr];
  }

  void DisposeHammerBank()
  {
    if (hb != null) {
      for (int k = 0; k < MaxKeysPerBank; ++k) {
        hb.keys[k].instr = null;
      }
    }
    if (hammerBanks[bNbr] != null) {
      hammerBanks[bNbr] = null;
    }
    hb = null;
  }

  double GetKeyValue()
  {
    return keyNumber;
  }

  double GetAValue()
  {
    return keyInten;
  }

  double GetInstParameter(int n)
  {
    switch (n) {
    case 4: return currentKey.energy;
    case 5: return currentKey.freq;
    case 6: return currentKey.velocity; // etc...
    default:  return callingMod.GetInstParameter(n);
    }
  }


  void Reset(SSModule callingMod)
  {
    int k;
    int instNbr = -1;
    // ActionPtr ar;

    super.Reset(callingMod);

    bNbr = (int) ResetExp(bNbrExp, callingMod);
    ResetExp(kFreqExp, callingMod);
    ResetExp(kAmpExp, callingMod);
    ResetExp(kAttackExp, callingMod);
    ResetExp(kDecayExp, callingMod);
    sustain = ResetExp(sustainExp, callingMod);
    ResetExp(waveformExp, callingMod);

    InitHammerBank();

    if (hb == null) {
      System.out.println("Error allocating Hammer Bank");
      parList.itsOwner.AbortSynthesis();
      return;
    }

    if (CountInputs(0) > 0) {

      Enumeration e = inputs.elements();
      int i = 0;
      while (e.hasMoreElements())
      {
        ModInput  inp = (ModInput) e.nextElement();
        if (inp.inputType == 0) {
          instNbr = i;
          break;
        }
        ++i;
      }
    }
    for (k = 0; k < MaxKeysPerBank && parList.itsOwner.windowState == WS_Synthesize; ++k) {
      // Initialize action for hammeraction #
      ActionRec ar = hb.keys[k];
      keyNumber = k;
      currentKey = ar;
      ar.energy = 0;
      ar.undampen = 0;
      ar.freq = SolveExp(kFreqExp, callingMod);
      ar.amp = SolveExp(kAmpExp, callingMod);
      ar.attack = SolveExp(kAttackExp, callingMod);
      // Decay per sample = attenpersec ^ (1 / SR)
      ar.decay = Math.pow(SolveExp(kDecayExp, callingMod),parList.itsOwner.timeInc);
      ar.waveInc = 0;
      ar.attackCtr = 0;
      ar.attackSamples = (int) (ar.attack * parList.itsOwner.mainInst.sampleRate);
      ar.attackFlag = false;
      ar.velocity = 0;
      ar.attackEnergy = 0;
      ar.decayCtr = 0;
      ar.decaySamples = (int) (0.02 * parList.itsOwner.mainInst.sampleRate);
      ar.decayFlag = false;
      ar.decayEnergy = 0;
      if (instNbr != -1) {
        ar.instr = parList.CloneInstrument(((ModInput) inputs.elementAt(instNbr)).link);
        if (ar.instr != null)
          ar.instr.ResetInstruments(this);
      }

    }
    lastTime = lastOutput = -1;
  }

  double GenerateOutput(SSModule callingMod)
  {
    double v = 0.0,vk;

    if (parList.pTime == lastTime)
      return lastOutput;
    lastTime = parList.pTime;

    if (hb != null)
    {

      double sustain = SolveExp(sustainExp, callingMod);

      for (int k = 0; k < MaxKeysPerBank; ++k) {
        ActionRec ar = hb.keys[k];
        if (ar.energy > 0.0 || ar.attackFlag) {
          currentKey = ar;
          keyNumber = k;
          if (ar.energy > 0 && !ar.decayFlag && ar.undampen+sustain == 0) {
            ar.decayFlag = true;
            ar.decayCtr = 0;
            ar.decayEnergy = ar.energy / ar.decaySamples;
          }
          if (ar.attackFlag) {
            if (ar.attackCtr < ar.attackSamples) {
              ar.energy += ar.attackEnergy;
              ++ar.attackCtr;
            }
            else
              ar.attackFlag = false;
          }
          else if (ar.decayFlag) {
            if (ar.decayCtr < ar.decaySamples) {
              ar.energy -= ar.decayEnergy;
              ++ar.decayCtr;
              if (ar.energy < 0) {
                ar.energy = 0;
                ar.decayFlag = false;
              }
            }
            else {
              ar.decayFlag = false;
              ar.energy = 0;
            }
          }
          else {
            ar.energy *= ar.decay;
          }
          keyInten = ar.energy;
          if (ar.instr != null) {
            vk = ((SSModule) ar.instr.mods.firstElement()).GenerateOutputTime(this,parList.pTime);
            vk *= ar.energy;
          }
          else {
            ar.waveInc += parList.itsOwner.timeInc * ar.freq;
            ar.waveInc -= (int) ar.waveInc;
            // Add value based on wavetable oscillator...
            parList.PushTime(ar.waveInc);
            vk = SolveExp(waveformExp, callingMod);
            parList.PopTime();
            vk *= ar.energy;
          }
          v += vk;
        }
      }
    }
    lastOutput = v;
    lastRightSample = v;
    return v;
  }


  void CleanUp()
  {
    DisposeHammerBank();
  }



  void Copy(SSModule ss)
  {
    SSHammerBank  osc = (SSHammerBank) ss;
    super.Copy(ss);
    CopyExp(osc.bNbrExp, bNbrExp);
    CopyExp(osc.kFreqExp, kFreqExp);
    CopyExp(osc.kAmpExp, kAmpExp);
    CopyExp(osc.kAttackExp, kAttackExp);
    CopyExp(osc.kDecayExp, kDecayExp);
    CopyExp(osc.sustainExp, sustainExp);
    CopyExp(osc.waveformExp, waveformExp);
    CopyExp(osc.kAttackExp, kAttackExp);
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("HAMB");
    ar.println("HAMBN " + bNbrExp.exp);
    ar.println("HAMBF " + kFreqExp.exp);
    ar.println("HAMBA " + kAmpExp.exp);
    ar.println("HAMBAt " + kAttackExp.exp);
    ar.println("HAMBD " + kDecayExp.exp);
    ar.println("HAMBS " + sustainExp.exp);
    ar.println("HAMBW " + waveformExp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    parList.GetNextInputLine(ar,"HAMB"); // unused
    bNbrExp = LoadExp(ar,"HAMBN");
    kFreqExp = LoadExp(ar,"HAMBF");
    kAmpExp = LoadExp(ar,"HAMBA");
    kAttackExp = LoadExp(ar,"HAMBAt");
    kDecayExp = LoadExp(ar,"HAMBD");
    sustainExp = LoadExp(ar,"HAMBS");
    waveformExp = LoadExp(ar,"HAMBW");
  }

  // Global function for HammerActuator
  // implement as static
  static void ActuateKey(int bNbr, int kNbr, int flags, double velocity, double undampen)
  {
    if (bNbr < 0 || bNbr >= MaxHammerBanks)
      return;
    HammerBankRec hb = hammerBanks[bNbr];
    if (hb == null)
      return;
    if (kNbr < 0 || kNbr >= MaxKeysPerBank)
      return;
    ActionRec ar = hb.keys[kNbr];
    ar.velocity = velocity * ar.amp;  // apply keyboard amp scaling here...
    if ((flags & HB_SympatheticRes) != 0)
    {
      // !!! Sympathetic resonnance
    }
    ar.undampen = undampen;
    if (undampen > 0.0) {
      ar.attackCtr = 0;
      ar.attackFlag = true;
      ar.attackEnergy = ar.velocity / (double) ( ar.attackSamples-1);
    }
  }


}

