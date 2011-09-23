import java.io.*;

abstract public class SSStorage
{
  double  sampleRate;
  int     nbrFrames;
  int     nbrChannels;

  public SSStorage()
  {
     nbrFrames = 0;
     sampleRate = 22050;
     nbrChannels = 1;
  }

  void SetSampleRate(double sr)
  {
    sampleRate = sr;
  }

  int StoreSampleF(double v)
  {
    if (v < -1.0) // Perform clipping
      v = -1.0;
    else if (v > 1.0)
      v = 1.0;
    return StoreSample((short) (v * 0x0007FFF));
  }

  int StoreSampleFS(double vL, double vR)
  {
    if (vL < -1.0) // Perform clipping
      vL = -1.0;
    else if (vL > 1.0)
      vL = 1.0;
    return StoreStereoSample((short) (vL * 0x0007FFF),(short) (vR * 0x0007FFF));
  }

  double GetSampleF(int n)
  {
    short iv;
    iv = GetSample(n);
    return iv/(double) 0x7FFF;
  }

  void Reset()
  {
   nbrFrames = 0;
  }

  int StartStorage(int plannedSamples, double sampleRate, boolean listenFlag, int nbrChannels)
  {
    SetSampleRate(sampleRate);
    nbrFrames = 0;
    this.nbrChannels = nbrChannels;
    return 0;
  }

  int StopStorage()
  {
      return 0;
  }

  int StoreSample(short v)
  {
      return 0;
  }

  int StoreStereoSample(short vL, short vR)
  {
      return 0;
  }

  short GetSample(int idx)
  {
      return 0;
  }

  int GetSamples(int start, int nbrSamples, short[] buffer)
  {
    return 0;
  }

  int Import(SSStorage src)
  {
    short[] inBuf = new short[32000];
    int  memPos,sampleCount,count;
    memPos = 0;
    sampleCount = src.nbrFrames;

    // System.out.println("SS Import");
    StartStorage(src.nbrFrames,src.sampleRate,false,src.nbrChannels);
    while (sampleCount > 0) {
      if (sampleCount >= 32000)
        count = 32000;
      else
        count = sampleCount;
      src.GetSamples(memPos,count,inBuf);
      for (int i= 0; i < count; ++i)
        StoreSample(inBuf[i]);
      sampleCount -= count;
    }
    inBuf = null;
    StopStorage();
    return 0;
  }

  abstract InputStream GetInputStream() throws IOException;

}
