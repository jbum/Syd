import java.util.*;
import java.io.*;


public class SSNoise extends SSModule implements SydConstants
{
	public int	randomize = 1;
	public int	startSeed = 1;

	int 	  seed;


  public SSNoise(ModList mList)
  {
    super(MT_Noise, mList);
  }

	double GenerateOutput(SSModule callingMod)
	{
    double retVal = DoubleRandom();
    lastRightSample = retVal;
    return retVal;
	}

  void  Copy(SSModule ss)
  {
    SSNoise sa = (SSNoise) ss;
    super.Copy(ss);
    startSeed = sa.startSeed;
    randomize = sa.randomize;
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("RND " + randomize + " " + startSeed);
  }

  void Load(BufferedReader ar) throws IOException
  {
    int         rnd=1,seed=0;

    String tokens[] = getTokens(ar,"RND");
    randomize = Integer.parseInt(tokens[1]);
    startSeed = Integer.parseInt(tokens[2]);
  }

  void Reset(SSModule callingMod)
  {
    super.Reset(callingMod);
    if (randomize != 0)
      Randomize();
    else
      MySRand(startSeed);
  }

  static final int R_A = 16807;
  static final int R_M = 2147483647;
  static final int R_Q = 127773;
  static final int R_R = 2836;
  static int  gSeed = 1;

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  int GetSRand()
  {
    return seed;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void MySRand(int s)
  {
    seed = s;
    if (seed == 0)
      seed = 1;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void Randomize()
  {
    MySRand((int) (System.currentTimeMillis() & 0xFFFFFFFF));
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  int LongRandom()
  {
    int  hi,lo,test;

    hi   = seed / R_Q;
    lo   = seed % R_Q;
    test = R_A * lo - R_R * hi;
    if (test > 0)
      seed = test;
    else
      seed = test + R_M;
    return seed;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 double DoubleRandom()
  {
    return LongRandom() / (double) R_M;
  }

}

