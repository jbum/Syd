import java.util.*;
import java.io.*;


public class SSHammerActuator extends SSModule implements SydConstants
{
  public ExpRec    bNbrExp     = InitExp("0"),
            keyNbrExp   = InitExp("p5"),
            triggerExp  = InitExp("1"),
            velocityExp = InitExp("p4/128"),
            undampenExp = InitExp("1");

  // Dynamic
  int     bNbr;
  boolean lastTrigger;

  public SSHammerActuator(ModList mList)
  {
    super(MT_HammerActuator, mList);
    DescribeLink(0, "Default Signal", "sig");
    DescribeLink(1, "Alt Signal #1", "sig1");
    DescribeLink(2, "Alt Signal #2", "sig2");
    DescribeLink(3, "Alt Signal #3", "sig3");
    DescribeLink(4, "Control Signal", "ctl");
    DescribeLink(5, "Alt Control Sig #1", "ctl1");
    DescribeLink(6, "Alt Control Sig #2", "ctl2");
    DescribeLink(7, "Alt Control Sig #3", "ctl3");
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);

    bNbr = (int) ResetExp(bNbrExp, callingMod);
    ResetExp(keyNbrExp, callingMod);
    ResetExp(triggerExp, callingMod);
    ResetExp(velocityExp, callingMod);
    ResetExp(undampenExp, callingMod);
    lastTrigger = false;
  }

  double GenerateOutput(SSModule callingMod)
  {
    double trigger = SolveExp(triggerExp, callingMod);
    if (trigger > 0.0) {
      if (!lastTrigger) { // Perform Trigger, and wait for signal to drop
        int keyNbr = (int) SolveExp(keyNbrExp, callingMod);
        double velocity = SolveExp(velocityExp, callingMod);
        double undampen = SolveExp(undampenExp, callingMod);
        // !! add sympathetic res flag later...
        SSHammerBank.ActuateKey(bNbr,(int) keyNbr,0,velocity,undampen);
      }
      lastTrigger = true;
    }
    else
      lastTrigger = false;
    return 0.0;
  }


  void Copy(SSModule ss)
  {
    SSHammerActuator  hama = (SSHammerActuator) ss;
    super.Copy(ss);
    CopyExp(hama.bNbrExp, bNbrExp);
    CopyExp(hama.keyNbrExp, keyNbrExp);
    CopyExp(hama.triggerExp, triggerExp);
    CopyExp(hama.velocityExp, velocityExp);
    CopyExp(hama.undampenExp, undampenExp);
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("HAMA");
    ar.println("HAMAN " + bNbrExp.exp);
    ar.println("HAMAK " + keyNbrExp.exp);
    ar.println("HAMAT " + triggerExp.exp);
    ar.println("HAMAV " + velocityExp.exp);
    ar.println("HAMAU " + undampenExp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    parList.GetNextInputLine(ar,"HAMA");
    bNbrExp = LoadExp(ar,"HAMAN");
    keyNbrExp = LoadExp(ar,"HAMAK");
    triggerExp = LoadExp(ar,"HAMAT");
    velocityExp = LoadExp(ar,"HAMAV");
    undampenExp = LoadExp(ar,"HAMAU");
  }


}

