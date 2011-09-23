public class ExpFuncs
{

  static double Angle(double xd, double yd)
  {
    return Math.atan2(yd,xd);
  }

  // Returns length of line which begins at origin and ends at xd,yd

  static double Distance(double xd, double yd)
  {
    return Math.sqrt(xd*xd+yd*yd);
  }


  static int Fibonacci(int x)
  {
    int n = 1,n1=0,tmp;
    while (x-- > 0) {
      tmp = n1;
      n1 = n;
      n = n + tmp;
    }
    return n;
  }

  static int isPrime(int n)
  {
    int  x,l;
    if (n < 4)
      return 1;
    if ((n & 1) == 0)
      return 0;
    x = 3;
    l = n/x;
    while (x < l) {
      if (x*l == n)
        return 0;
      x += 2;
      l = n/x;
    }
    return 1;
  }

    static final int[] cntBits = {0,1,1,2,    // 0,1,2,3
                          1,2,2,3,    // 4,5,6,7
                          1,2,2,3,    // 8,9,10,11
                          2,3,3,4};   // 12,13,14,15

  static int scaleNote(int mask, int n)
  {
    // Treats mask as a 12-bit mask of "on" pitches for a scale
    // MAJOR: 0xAB5   2741
    // MINOR: 0x5AD   1453
    // MINOR2:0x9AD   2477
    // WHOLENOTES: 0x555
    // compute for starting at 0, the
    // Count bits for lower 12-bits
    try {
      int nbrNotesInScale = cntBits[mask & 0x000F] +
                            cntBits[(mask & 0x00F0)>>4] +
                            cntBits[(mask & 0x0F00)>>8];
    // System.out.println("ScaleNote: " + mask + " " + " " + n + ", nbrNotes=" + nbrNotesInScale);
      int note = 12*(n/nbrNotesInScale);
      n %= nbrNotesInScale;
      while (n > 0) {
        --n;
          do {
            ++note;
            mask >>= 1;
          } while (mask != 0 && (mask & 1) == 0);
      }
      // System.out.println("ScaleNote: " + note);
      return note;
    }
    catch (Exception e) {
      System.out.println("Exception in scalenote: " + e.toString());
    }
    return 0;
  }

  static double scaleNoteF(int mask, int n, int baseNote)
  {
    double v = cpsmidi(scaleNote(mask,n)+baseNote);
    // System.out.println("scalenotef: " + v);
    return v;
  }

  static double cpspch(double pch) // convert pch to cps
  {
    double i,f;
    i = (int) pch;
    f = (pch - i)*100/12.0;
    return ExpMgr.c1 * Math.pow(2,i+f);
  }

  static double cpsoct(double oct)
  {
    return ExpMgr.c1 * Math.pow(2,oct);
  }

  static double pchoct(double oct)
  {
    double i,f;
    i = (int) oct;
    f = (oct - i)*12/100.0;
    return i + f;
  }

  static double octpch(double pch)
  {
    double i,f;
    i = (int) pch;
    f = (pch - i)*100/12.0;
    return i + f;
  }

  static double octcps(double cps)
  {
    double octpow;
    octpow = cps/ExpMgr.c1;
    return Math.log(octpow) / ExpMgr.log2;  // compute base 2 logarithm of octpow (log(octpow) / log(2))
  }

  static double octmidi(double mp)
  {
    return 3.0 + mp / 12.0;
  }

  static double cpsmidi(double mp)
  {
    return cpsoct(octmidi(mp));
  }

  static double linen(double t, double atk, double dur, double dcy)
  {
    double sdcy;
    if (t < 0 || t > dur)
      return 0.0;
    if (dcy > dur)
      dcy = dur;
    if (atk > 0 && t < atk) {
      return t/atk;
    }
    sdcy = dur-dcy;
    if (sdcy < 0)
      sdcy = 0;
    if (dcy > 0 && t >= sdcy) {
      return 1.0 - (t-sdcy)/dcy;
    }
    return 1.0;
  }

  static double linenr(double t, double atk, double dcy, double atdec)
  {
    if (t < 0 || t > atk+dcy)
      return 0.0;
    if (atk > 0 && t < atk) {
      return t/atk;
    }
    t -= atk;
    return Math.pow(atdec,t/dcy);
  }

  static double limit(double v, double low, double high)
  {
    if (high < low) {
      return (high-low)/2;
    }
    if (v < low)
      return low;
    if (v > high)
      return high;
    return v;
  }

}

