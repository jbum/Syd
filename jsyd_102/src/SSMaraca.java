import java.io.*;
import java.util.*;

public class SSMaraca extends SSModule implements SydConstants
{

  public ExpRec    resfreqExp = InitExp("3200"),
                    respoleExp = InitExp("0.96"),
                    probExp = InitExp("1/16"),
                    sysdecayExp = InitExp("0.999"),
                    snddecayExp = InitExp("0.95");

  // Dynamic Variables
  int        numBeans;

  // Dynamic
  double resfreq,respole,prob,sysdecay,snddecay;
  double gain,input,shakeEnergy,temp,sndLevel;
  double[] coeffs = {0,0},
            output = {0,0};
  int  i;

  public SSMaraca(ModList mList)
  {
    super(MT_Maraca, mList);

    DescribeLink(0, "INVALID", "???");
  }

  void  Copy(SSModule ss)
  {
    SSMaraca osc = (SSMaraca ) ss;
    super.Copy(ss);
    CopyExp(osc.resfreqExp, resfreqExp);
    CopyExp(osc.respoleExp, respoleExp);
    CopyExp(osc.probExp, probExp);
    CopyExp(osc.sysdecayExp, sysdecayExp);
    CopyExp(osc.snddecayExp, snddecayExp);
  }


  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("MARI");
    ar.println("MARRF " + resfreqExp.exp);
    ar.println("MARRP " + respoleExp.exp);
    ar.println("MARP " + probExp.exp);
    ar.println("MARSysD " + sysdecayExp.exp);
    ar.println("MARSndD " + snddecayExp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    parList.GetNextInputLine(ar,"MARI"); // unused
    resfreqExp = LoadExp(ar,"MARRF");
    respoleExp = LoadExp(ar,"MARRP");
    probExp = LoadExp(ar,"MARP");
    sysdecayExp = LoadExp(ar,"MARSysD");
    snddecayExp = LoadExp(ar,"MARSndD");
  }

  void   Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    resfreq = ResetExp(resfreqExp, callingMod);
    respole = ResetExp(respoleExp, callingMod);
    prob = ResetExp(probExp, callingMod);
    sysdecay = ResetExp(sysdecayExp, callingMod);
    snddecay = ResetExp(snddecayExp, callingMod);

    numBeans = 64;
    gain = Math.log((double) numBeans) / Math.log(4.0) * 40.0 / numBeans;
    output[0] = output[1] = 0.0;
    input = 0.0;
    shakeEnergy = 0;
    temp = 0.0;
    sndLevel = 0;
    coeffs[0] = -respole * 2.0 * Math.cos(resfreq * 2.0 * Math.PI * parList.itsOwner.timeInc);
    coeffs[1] = respole * respole;
    i = 0;
  }

  double GenerateOutput(SSModule callingMod)
  {
    if (temp < 2*Math.PI) {
      // shake over 50 ms and add energy
      temp += (2*Math.PI*parList.itsOwner.timeInc) / 0.05;
      shakeEnergy += (1.0 - Math.cos(temp));
    }

    // Compute exponential system decay
    shakeEnergy *= sysdecay;

    if (ExpMgr.DoubleRandom() < prob)
      sndLevel += gain*shakeEnergy;

    input = sndLevel * (ExpMgr.DoubleRandom()*2 - 1);
    sndLevel *= snddecay;

    // do gourd resonance filter calc
    // Note: this is a good bandpass filter!
    input -= output[0]*coeffs[0];
    input -= output[1]*coeffs[1];
    output[1] = output[0];
    output[0] = input;

    ++i;

    double retVal = (output[0] - output[1])/32000.0;
    lastRightSample = retVal;
    return retVal;
  }


}