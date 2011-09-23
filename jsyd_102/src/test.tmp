public class WaveTables
{
  static final int WaveTableSize = 16384;
  static double[] gSinTable = null;
  static double[] gSquareTable = null;
  static double[] gSawTable = null;

  static void InitWaveTables()
  {
    int   i;
    int   h;
    double hTime,hx;
    double hScale;
    double  max;

    if (gSinTable != null && gSquareTable != null)
        return;
    System.out.println("Initializing wave tables");

    gSinTable = new double[WaveTableSize];
    gSquareTable = new double[WaveTableSize];
    gSawTable = new double[WaveTableSize];

    hTime = Math.PI*2/(double) WaveTableSize;
    for (i = 0,hx = 0; i < WaveTableSize; ++i, hx += hTime) {
      gSinTable[i] = Math.sin(hx);
      gSquareTable[i] = 0;
      gSawTable[i] = 0;
    }


    for (h = 1; h <= 9; h += 2) { // was 9
      hScale = 1.0 / h;
      for (i = 0; i < WaveTableSize; ++i) {
        gSquareTable[i] += hScale*Math.sin((i*2*Math.PI*h)/(double) WaveTableSize);
      }
    }

    // Find Maximum for Normalization
    max = 0;
    for (i = 0; i < WaveTableSize; ++i) {
      if (Math.abs(gSquareTable[i]) > max)
        max = gSquareTable[i];
    }
    // Normalize
    // max = 1/max;
    for (i = 0; i < WaveTableSize; ++i) {
      gSquareTable[i] /= max;
    }

    for (h = 1; h <= 10; h += 1) { // was 9
      hScale = 1.0 / h;
      for (i = 0; i < WaveTableSize; ++i) {
        gSawTable[i] -= hScale*Math.sin((i*2*Math.PI*h)/(double) WaveTableSize);
      }
    }

    // Find Maximum for Normalization
    max = 0;
    for (i = 0; i < WaveTableSize; ++i) {
      if (Math.abs(gSawTable[i]) > max)
        max = gSawTable[i];
    }
    // Normalize
    // max = 1/max;
    for (i = 0; i < WaveTableSize; ++i) {
      gSawTable[i] /= max;
    }

  }
}
