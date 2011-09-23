import java.io.*;
import java.util.*;

public class SSPluck extends SSModule implements SydConstants
{
	static final int KP_Normal = 0;	// the only type, at the moment

	// static
  public int   variant = KP_Normal;
  public ExpRec    freqExp = InitExp("440"),
         durExp = InitExp("2"),
         ampExp = InitExp("1"),
         decayExp = InitExp("10");

	// dynamic
	double		  dur,rho,S,S1,C,x1,y1,z;
	int 		    p,count;
	double[]		d = null;
  int         spIdx = 0;
	boolean		  error = false;

  public SSPluck(ModList mList)
  {
    super(MT_Pluck, mList);

    DescribeLink(0, "INVALID", "???");
  }

  void Reset(SSModule callingMod)
  {
    double   f1,amp,Q,loop,delay,D,Gf,G,R;
    super.Reset(callingMod);

    if (PatchOwner.gVerbose > 1)
      System.out.println("Resetting pluck, calling mod = " + callingMod.label + ", cmm = " + callingMod.callingMod.label);

    f1 = ResetExp(freqExp, callingMod);
    dur = ResetExp(durExp, callingMod);
    Q = ResetExp(decayExp, callingMod);
    amp = ResetExp(ampExp, callingMod);

    if (Q == 0.0)
      Q = 0.00001; // bug fix 10/5/98

    // System.out.println("KP: f1 = " + f1 + ", freqExp = " + freqExp.exp);
    if (f1 == 0 || f1 > 20000)
    {
      error = true;
      return;
    }

    rho = 1.0;
    S = 0.5;
    C = 0.9999;
    R = parList.itsOwner.mainInst.sampleRate;
    loop = R / f1;
    p = (int) loop;
    error = false;


    Gf = Math.pow(10.0, -Q / (20.0 * f1 * dur)); // loop gain required at any req for a Q db loss over dur seconds
    G = Math.cos(Math.PI * f1 / R);       // loop gain without adjustments to rho and S

    // If smaller gain needed, reduce with rho, otherwise, stretch with S
    if (Gf <= G)
      rho = Gf/G;
    else {
      double  cosf1,a,b,c,D2,a2,S2;
      cosf1 = Math.cos(2*Math.PI*f1/R);
      a = 2.0 - 2.0*cosf1;
      b = 2.0*cosf1 - 2.0;
      c = 1.0 - Gf*Gf;
      D2 = Math.sqrt(b*b-4.0*a*c);
      a2 = 2.0*a;
      S1 = (-b + D2)/a2; // quadratic formula
      S2 = (-b - D2)/a2;
      if (S1 > 0.0 && S1 <= 0.5)
        S = S1;
      else
        S = S2;
    }
    delay = p+S;  // approx loop delay
    if (delay > loop)
      delay = --p + S;
    D = loop - delay;
    C = (1.0 - D) / (1.0 + D);

    if (S <= 0.0 || S > 0.5 || rho < 0.0 || rho > 1.0 || Math.abs(C) >= 1.0)
    {
      error = true;
      System.out.println("Pluck Error");
    }
    d = new double[p];
    {
      int i;
      double m;

      // Initialize delay line
      if (variant == 0 || variant == 1 || variant == 2) {
        for (i = 0; i < p; ++i)
          d[i] = 2.0*ExpMgr.DoubleRandom() - 1.0;
      } else if (variant == 3) {
        int var = 1+(variant-1)/2;
        for (i = 0; i < p; ++i)
          d[i] = 2.0*parList.itsOwner.expMgr.Noise((var*i)/(double)p,ExpMgr.DoubleRandom()*256) - 1.0;
      }
      else {
        int var = 1+(variant-2)/2;
        for (i = 0; i < p; ++i)
          d[i] = 2.0*parList.itsOwner.expMgr.Turbulence((var*i)/(double)p,ExpMgr.DoubleRandom()*256) - 1.0;
      }
      // Compute average
      m = 0.0;
      for (i = 0; i < p; ++i)
        m += d[i];
      m /= p;

      // Subtract from table
      for (i = 0; i < p; ++i)
        d[i] -= m;

      // Normalize and scale to amplitude
      m = 0.0;
      for (i = 0; i < p; ++i)
        if (m < Math.abs(d[i]))
          m = Math.abs(d[i]); // bug fix from book...
      m = amp/m;
      for (i = 0; i < p; ++i)
        d[i] *= m;
    }
    dur *= R;
    S1 = 1.0 - S;
    count = 0;
    spIdx = 0; // was sp = d
    x1 = y1 = z = 0.0;
  }

  double GenerateOutput(SSModule callingMod)
  {
    double x,y;
    if (error || d == null)
      return 0.0;
    if (count >= dur)
      return 0.0;

    x = d[spIdx];
    if (variant == 2 || (variant == 1 && ExpMgr.DoubleRandom() < 0.5))
      y = rho * (S1 * x - S * x1);    // filter1
    else
      y = rho * (S1 * x + S * x1);    // filter1
    z = C * (y - z) + y1;       // filter2
    d[spIdx] = z;
    x1 = x;
    y1 = y;
    ++spIdx;
    ++count;
    if (spIdx >= p)
      spIdx = 0;
    lastRightSample = x;
    return x;
  }


  void CleanUp()
  {
    d = null;
  }


  void Copy(SSModule ss)
  {
    SSPluck osc = (SSPluck) ss;
    super.Copy(ss);
    variant = osc.variant;
    CopyExp(osc.freqExp, freqExp);
    CopyExp(osc.ampExp, ampExp);
    CopyExp(osc.durExp, durExp);
    CopyExp(osc.decayExp, decayExp);
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("PLKI " + variant);
    ar.println("PLKF " + freqExp.exp);
    ar.println("PLKD " + durExp.exp);
    ar.println("PLKA " + ampExp.exp);
    ar.println("PLKd " + decayExp.exp);
  }

  void Load(BufferedReader ar) throws IOException
  {
    variant = Integer.parseInt(getFirstToken(ar,"PLKI"));
    freqExp = LoadExp(ar,"PLKF");
    durExp = LoadExp(ar,"PLKD");
    ampExp = LoadExp(ar,"PLKA");
    decayExp = LoadExp(ar,"PLKd");
  }


}
