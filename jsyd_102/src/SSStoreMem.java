import java.io.*;

public class SSStoreMem extends SSStorage
{
  static final int MaxAllocSamples = 44100*10*2;
  static final int noErr          = 0;
  int     memStartWindow = 0;
  short[] buffer = new short[MaxAllocSamples];


  int GetSamples(int start, int nbrSamples, short[] outBuffer)
  {
    // 10/11 Switched to long for 68k compat
    for (int i = 0; i < nbrSamples; ++i)
      outBuffer[i] = GetSample(start+i);

    return noErr;
  }

  short GetSample(int i)
  {
    if (i < memStartWindow || i >= memStartWindow+MaxAllocSamples || i >= nbrFrames*nbrChannels)
      return 0;
    else
      return buffer[i-memStartWindow];
  }

  int StoreSample(short sample)
  {
    int  n = nbrFrames*nbrChannels-memStartWindow;
    if (n < 0 || n >= MaxAllocSamples)
      System.out.println("SSStoreMem:StoreSample indexing error");
    else
      buffer[n] = sample;
    ++nbrFrames;
    if (nbrFrames+1024 >= memStartWindow+MaxAllocSamples) {
      System.arraycopy(buffer, 1024, buffer, 0, MaxAllocSamples-1024);
      memStartWindow += 1024;
    }
    return noErr;
  }

  int StoreStereoSample(short sample, short sampleR)
  {
    int  n = nbrFrames*2-memStartWindow;
    if (n < 0 || n >= MaxAllocSamples)
      System.out.println("SSStoreMem:StoreSample indexing error");
    else {
      buffer[n] = sample;
      buffer[n+1] = sampleR;
    }
    ++nbrFrames;
    if (nbrFrames*2+1024 >= memStartWindow+MaxAllocSamples) {
      System.arraycopy(buffer, 1024, buffer, 0, MaxAllocSamples-1024);
      memStartWindow += 1024;
    }
    return noErr;
  }


  int StartStorage(int plannedSamples,double sampleRate, boolean listenFlag, int nbrChannels)  // Start Buffered Storage
  {
    int oe = noErr;
    oe = super.StartStorage(plannedSamples,sampleRate,listenFlag, nbrChannels);
    memStartWindow = 0;
    nbrFrames = 0;
    // System.out.println("Nbr Channels = " + nbrChannels);
    return oe;
  }

  // Convert to WAVE file in-memory, for playback purposes.
  InputStream GetInputStream() throws IOException
  {
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    DataOutputStream dos = new DataOutputStream(bos);
       // output the data to ods
    int byteSize = nbrFrames*nbrChannels*2;
    dos.writeInt(SSStoreFile.CKID_RIFF);
    dos.writeInt(LocalToLSBLong(byteSize+8+16+8+4));
    dos.writeInt(SSStoreFile.CKID_WAVE);
    dos.writeInt(SSStoreFile.CKID_fmt_);
    dos.writeInt(LocalToLSBLong(16));
    dos.writeShort(LocalToLSBShort((short) 1)); // wFormatTag
    dos.writeShort(LocalToLSBShort((short) nbrChannels)); // wChannels
    dos.writeInt(LocalToLSBLong((int) sampleRate)); // dwSamplesPerSec
    dos.writeInt(LocalToLSBLong((int) (sampleRate*2*nbrChannels))); // dwAvgBytesPerSec
    dos.writeShort(LocalToLSBShort((short) 2)); // wBlockAlign
    dos.writeShort(LocalToLSBShort((short) 16)); // wSampleSize
    dos.writeInt(SSStoreFile.CKID_data);
    dos.writeInt(LocalToLSBLong(byteSize));
    for (int i = 0; i < nbrFrames*nbrChannels; ++i) {
      dos.writeShort(LocalToLSBShort(buffer[i]));
    }
    return new ByteArrayInputStream(bos.toByteArray());
  }

  short LocalToLSBShort(short v)
  {
    return (short) ( ((v >> 8) & 0x00FF) | ((v << 8) & 0xFF00));
  }

  int LocalToLSBLong(int v)
  {
    return  ((v >> 24) & 0x000000FF) |
            ((v >> 8)  & 0x0000FF00) |
            ((v << 8)  & 0x00FF0000) |
            ((v << 24) & 0xFF000000);
  }

}
