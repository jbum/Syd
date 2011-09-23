import java.io.*;
import java.util.*;

/*
ADSR 0.1 1 0.3 0.5 0.5 0.5 0.1
ADSRD 1.0

new style:
(evaluate expressions at time of trigger)

ADSR2 flags  (1 = use trigger, 2 = interruptok)
ADSRAT exp
ADSRAL exp
ADSRDT exp
ADSRDL exp
ADSRST exp
ADSRSL exp
ADSRRT exp
ADSRD  dur


 */



public class SSADSR extends SSModule implements SydConstants
{
  static final int ADSR_Idle    = 0;
  static final int ADSR_Attack  = 1;
  static final int ADSR_Decay   = 2;
  static final int ADSR_Sustain = 3;
  static final int ADSR_Release = 4;


  // Stored
  public ExpRec   durExp;
  public ExpRec   attackTExp,decayTExp,sustainTExp,releaseTExp;
  public ExpRec   attackLExp,decayLExp,sustainLExp;
  public int      useTrigger, interruptsOK;

  // Internal
  public double   attackT,decayT,sustainT,releaseT;
  public double   attackL,decayL,sustainL;
  double   startTime;
  double   durScale,duration;
  int     envPhase;
  boolean    lastTrigger = false;


  public SSADSR(ModList mList)
  {
    super(MT_Envelope, mList);
    attackT = 0.1;  attackL = 1.0;
    decayT = 0.2; decayL = .7;
    sustainT = 0.6; sustainL = .6;
    releaseT = 0.1;
    envPhase = 0;
    useTrigger = 0;
    interruptsOK = 0;
    durExp = InitExp("1.0");
    attackTExp = InitExp("0.1");
    attackLExp = InitExp("1.0");
    decayTExp = InitExp("0.2");
    decayLExp = InitExp("0.7");
    sustainTExp = InitExp("0.6");
    sustainLExp = InitExp("0.6");
    releaseTExp = InitExp("0.1");
    DescribeLink(0, "Trigger Signal", "trig");
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
//    ar.println("ADSR2


    ar.println("ADSR2 " + (useTrigger+interruptsOK*2));
    ar.println("ADSRAT " + attackTExp.exp);
    ar.println("ADSRAL " + attackLExp.exp);
    ar.println("ADSRDT " + decayTExp.exp);
    ar.println("ADSRDL " + decayLExp.exp);
    ar.println("ADSRST " + sustainTExp.exp);
    ar.println("ADSRSL " + sustainLExp.exp);
    ar.println("ADSRRT " + releaseTExp.exp);
    ar.println("ADSRD " + durExp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    String[] tokens = getTokens(ar,"ADSR");
    if (tokens[0].equals("ADSR")) {
      // Old Style
      attackTExp =  InitExp(tokens[1]);
      attackLExp =  InitExp(tokens[2]);
      decayTExp =   InitExp(tokens[3]);
      decayLExp =   InitExp(tokens[4]);
      sustainTExp = InitExp(tokens[5]);
      sustainLExp = InitExp(tokens[6]);
      releaseTExp = InitExp(tokens[7]);
      durExp = LoadExp(ar,"ADSRD");
      useTrigger = 0;
      interruptsOK = 0;
    }
    else {  // ADSR2 (new format)
      int flags = Integer.parseInt(tokens[1]);
      useTrigger = (flags & 1) > 0? 1 : 0;
      interruptsOK = (flags & 2) > 0? 1 : 0;
      attackTExp =  LoadExp(ar,"ADSRAT");
      attackLExp =  LoadExp(ar,"ADSRAL");
      decayTExp =   LoadExp(ar,"ADSRDT");
      decayLExp =   LoadExp(ar,"ADSRDL");
      sustainTExp = LoadExp(ar,"ADSRST");
      sustainLExp = LoadExp(ar,"ADSRSL");
      releaseTExp = LoadExp(ar,"ADSRRT");
      durExp = LoadExp(ar,"ADSRD");
    }
  }

  double GenerateOutput(SSModule callingMod)
  {
    double retVal = 0.0;
    double deltaT;
    double pTime = parList.pTime;

    if (useTrigger != 0) {
      // Figure out if triggering

      if (MixInputs(0,callingMod) >= .5)
      {
        if (!lastTrigger &&
             (envPhase == ADSR_Idle || interruptsOK != 0))
        {
            TriggerAttack(pTime);
            lastTrigger = true;
        }
      }
      else
        lastTrigger = false;

    }

    while (true)
    {
      deltaT = (pTime - startTime)*durScale;

      switch (envPhase) {
      case ADSR_Idle: // Not firing
        break;
      case ADSR_Attack:
        // Attack - go from 0 to attackL in time AttachT
        if (deltaT < attackT) {
          retVal = attackL * deltaT / attackT;
        }
        else {
          ++envPhase;
          startTime = pTime;
          continue;
        }
        break;
      case ADSR_Decay:
        // Decay - go from attackL to decayL in time DecayT
        if (deltaT < decayT) {
          retVal = attackL + (decayL - attackL) * deltaT / decayT;
        }
        else {
          ++envPhase;
          startTime = pTime;
          continue;
        }
        break;
      case ADSR_Sustain:
        // Sustain - go from decayL to sustainL in time SustainT
        if (deltaT < sustainT) {
          retVal = decayL + (sustainL - decayL) * deltaT / sustainT;
        }
        else {
          ++envPhase;
          startTime = pTime;
          continue;
        }
        break;
      case ADSR_Release:
        // Release - go from sustainL to 0 in time ReleaseT
        if (deltaT < releaseT) {
          retVal = sustainL - sustainL * deltaT / releaseT;
        }
        else {
          envPhase = ADSR_Idle;
          retVal = 0.0;
        }
        break;
      }
      break; // out of while loop
    }
    lastRightSample = retVal;
    return retVal;
  }

  void TriggerAttack(double start)
  {
    duration = ResetExp(durExp, callingMod);
    attackT = SolveExp(attackTExp, callingMod);
    attackL = SolveExp(attackLExp, callingMod);
    decayT = SolveExp(decayTExp, callingMod);
    decayL = SolveExp(decayLExp, callingMod);
    sustainT = SolveExp(sustainTExp, callingMod);
    sustainL = SolveExp(sustainLExp, callingMod);
    releaseT = SolveExp(releaseTExp, callingMod);
    durScale = 1/duration;
    envPhase = ADSR_Attack;
    startTime = start;
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    duration = ResetExp(durExp, callingMod);
    lastTrigger = false;
    if (duration > 0 && useTrigger == 0) {
      TriggerAttack(0.0);
    }
    else {
      durScale = 1;
      envPhase = ADSR_Idle;
      startTime = 0;
    }
  }

//  public ExpRec   durExp;
//  public ExpRec   attackTExp,decayTExp,sustainTExp,releaseTExp;
//  public ExpRec   attackLExp,decayLExp,sustainLExp;
//  public int      useTrigger, interruptsOK;

  void Copy(SSModule as)
  {
    SSADSR  adsr = (SSADSR) as;
    super.Copy(as);
    useTrigger = adsr.useTrigger;
    interruptsOK = adsr.interruptsOK;
    CopyExp(adsr.durExp, durExp);
    CopyExp(adsr.attackTExp, attackTExp);
    CopyExp(adsr.attackLExp, attackLExp);
    CopyExp(adsr.decayTExp, decayTExp);
    CopyExp(adsr.decayLExp, decayLExp);
    CopyExp(adsr.sustainTExp, sustainTExp);
    CopyExp(adsr.sustainLExp, sustainLExp);
    CopyExp(adsr.releaseTExp, releaseTExp);

  }

}

