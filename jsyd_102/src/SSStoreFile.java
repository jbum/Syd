import java.io.*;


public class SSStoreFile extends SSStorage
{
  static final int InCacheSize  = 16000;
  static final int OutCacheSize = 32000;

  static final int CKID_FORM = 0x464f524d;
  static final int CKID_AIFF = 0x41494646;
  static final int CKID_COMM = 0x434f4d4d;
  static final int CKID_SSND = 0x53534e44;
  static final int CKID_RIFF = 0x52494646;
  static final int CKID_WAVE = 0x57415645;
  static final int CKID_fmt_ = 0x666d7420;
  static final int CKID_data = 0x64617461;

  static final int ST_UNDEFINED   = 0;
  static final int ST_SIGNED      = 2;     /* Set for Signed Audio (0=0) */
  static final int ST_UNSIGNED    = 0;     /* Don't Set for unsigned Audio (128=0) */
  static final int ST_16BIT       = 4;     /* Set for 16 bit Audio */
  static final int ST_8BIT        = 0;     /* Don't Set for 8 bit Audio */
  static final int ST_STEREO      = 8;     /* Set for Stereo */
  static final int ST_MONO        = 0;     /* Don't set for Mono */
  static final int ST_TOWNS       = 16;      /* Set for FM Towns */
  static final int ST_AIFF        = 32;      /* Set for AIFF */
  static final int ST_WAVE        = 64;      /* Set for WAVE */

  static final int FT_AIFF        = 0;
  static final int FT_WAVE        = 1;

  static final int noErr          = 0;

  RandomAccessFile  fRef = null;
  String            fsSpec = "";
  int               inType = ST_SIGNED | ST_16BIT | ST_MONO | ST_WAVE;
  int               outType = ST_SIGNED | ST_16BIT | ST_MONO | ST_WAVE;
  int               fileType = FT_WAVE;

  short[]           inCache = null;
  short[]           outbuf = null;
  int               memPos = 0;
  int               inCachePos = -1;
  int               fileDataStart;
  int               fileDataLength;
  boolean           writing = false;
  AIFFInfo          aHdr = new AIFFInfo();
  WAVEInfo          wHdr = new WAVEInfo();



  public SSStoreFile()
  {
    super();
  }

  int CloseFile()
  {
    System.out.println("Close File");

    if (fRef != null) {
      try {
        synchronized(this)
        {
          fRef.close();
          fRef = null;
        }
      }
      catch (IOException e)
      {
        return -1;
      }
    }
    return 0;
  }

  int SetReadFileSpec(String fileSpec)
  {
    // System.out.println("Set read filespec: " + fileSpec);
     int oe;
     synchronized(this)
     {
       CloseFile();
       fsSpec = fileSpec;
       if ((oe= OpenFile()) != noErr)
          return oe;
       if ((oe = ReadHeader()) != noErr)
          return oe;
     }
     return 0;
  }

  int SetWriteFileSpec(int inFileType, int isStereo, String fileSpec)
  {
    this.nbrChannels = 1 + isStereo;

    // System.out.println("Set write filespec: " + fileSpec);
    fsSpec = fileSpec;
    if (inFileType == SSOutput.OM_AIFF)
      fileType = FT_AIFF;
    else if (inFileType == SSOutput.OM_WAVE)
      fileType = FT_WAVE;
    if (fileType == FT_AIFF) {
      inType = ST_SIGNED | ST_16BIT | ST_MONO | ST_AIFF;
      outType = ST_SIGNED | ST_16BIT | ST_MONO | ST_AIFF;
    }
    else {
      inType = ST_SIGNED | ST_16BIT | ST_MONO | ST_WAVE;
      outType = ST_SIGNED | ST_16BIT | ST_MONO | ST_WAVE;
    }
    if (isStereo != 0) {
      inType &= ~(ST_MONO);
      inType |= ST_STEREO;
      outType &= ~(ST_MONO);
      outType |= ST_STEREO;
    }
    return 0;
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

  short LSBShortToLocal(short v)
  {
    return LocalToLSBShort(v);
  }

  int LSBLongToLocal(int v)
  {
    return LocalToLSBLong(v);
  }

  int OpenFile() // open file for input
  {
    // LogMessage("Opening File for Read...\n");
    try {
      synchronized(this) {
        System.out.println("Opening file for read");
        fRef = new RandomAccessFile(fsSpec, "r");
      }
    }
    catch (IOException e)
    {
      System.out.println("Can't open file for input (" + fsSpec + ")");
      return -1;
    }
    return 0;
  }

  int ReadHeader()
  {
    try {
      synchronized(this) {
        System.out.println("Reading header");
        long fileSize = fRef.length();
        int count = 4;
        int fileSig = fRef.readInt();
        fRef.seek(0);
        if (fileSig == CKID_FORM) // 'FORM'
          fileType = FT_AIFF;
        else if (fileSig == CKID_RIFF)
          fileType = FT_WAVE;
        else {
           System.out.println("Unknown file type: " + fsSpec);
           return -1;
        }
        if (fileType == FT_AIFF) {
            inType = ST_SIGNED | ST_8BIT | ST_MONO | ST_AIFF;
            IFFChunk ck = FindIFFChunk(CKID_AIFF, CKID_COMM);
            if (ck.dataSize > 26-8)
              ck.dataSize = 26-8;
            // byte[] inbuf = new byte[ck.dataSize];
            // fRef.read(inbuf);
            aHdr.numChannels = fRef.readShort();
            aHdr.numSampleFrames = fRef.readInt();
            aHdr.sampleSize = fRef.readShort();
            aHdr.sampleRate = Read80BitDouble(); // a SANE (IEEE 754 extended) double
            nbrFrames = aHdr.numSampleFrames;
            if (aHdr.sampleSize == 16)
              inType |= ST_16BIT;
            if (aHdr.numChannels == 2)
              inType |= ST_STEREO;
            fRef.seek(0);
            ck = FindIFFChunk(CKID_AIFF, CKID_SSND);
            fileDataStart = ck.dataPos = 8;
            fileDataLength = ck.dataSize - 8;
        }
        else {  // FT_WAVE
            inType = ST_UNSIGNED | ST_8BIT | ST_MONO | ST_WAVE;

            fRef.seek(20);

            wHdr.wFormatTag = LSBShortToLocal(fRef.readShort());
            wHdr.wChannels = LSBShortToLocal(fRef.readShort());
            wHdr.dwSamplesPerSec = LSBLongToLocal(fRef.readInt());
            wHdr.dwAvgBytesPerSec = LSBLongToLocal(fRef.readInt());
            wHdr.wBlockAlign = LSBShortToLocal(fRef.readShort());
            wHdr.wSampleSize = LSBShortToLocal(fRef.readShort());

            int chunkID = fRef.readInt();
            int dataLength = LSBLongToLocal(fRef.readInt());

            if (wHdr.wChannels == 2)
              inType |= ST_STEREO;
            if (wHdr.wSampleSize == 16)
              inType |= ST_16BIT | ST_SIGNED;


            sampleRate = wHdr.dwSamplesPerSec;
            fileDataLength = nbrFrames = dataLength;
            if ((inType & ST_16BIT) != 0)
              nbrFrames /= 2;
            if ((inType & ST_STEREO) != 0)
              nbrFrames /= 2;
            fileDataStart = 20 + 16 + 8;
            fileDataLength = (int) fileSize - fileDataStart;
        }
      }
    }
    catch (IOException e)
    {
         System.out.println("Read header problem with " + fsSpec + ": " + e.toString());
         return -1;
    }
    catch (Exception e)
    {
         System.out.println("Read header problem with " + fsSpec + ": " + e.toString() + " fref="+fRef);
         return -1;
    }
    if (PatchOwner.gVerbose > 1)
      System.out.println("RH NbrFrames = " + nbrFrames);
    return 0;
  }

  int WriteHeader()
  {
    try {
      synchronized(this)
      {
        System.out.println("Writing header");
        fRef.seek(0);
        // System.out.println("WriteHeader done seek");
        int byteSize = nbrFrames;
        if ((outType & ST_16BIT) != 0)
          byteSize <<= 1;
        if ((outType & ST_STEREO) != 0)
          byteSize <<= 1;

        if (fileType == FT_AIFF) {

          fRef.writeInt(CKID_FORM);
          fRef.writeInt(4+8+18+8+(8+byteSize));
          fRef.writeInt(CKID_AIFF);
          fRef.writeInt(CKID_COMM);
          fRef.writeInt(18);
          fRef.writeShort((outType & ST_STEREO) != 0? 2 : 1); // num channels
          fRef.writeInt(nbrFrames);
          fRef.writeShort(16); // sample size
          Write80BitDouble(sampleRate);
          fRef.writeInt(CKID_SSND);
          fRef.writeInt(byteSize+8);
          fRef.writeInt(0); // offset, blocksize
          fRef.writeInt(0); // offset, blocksize
        }
        else {
          fRef.writeInt(CKID_RIFF);
          fRef.writeInt(LocalToLSBLong(byteSize+8+16+8+4));
          fRef.writeInt(CKID_WAVE);
          fRef.writeInt(CKID_fmt_);
          fRef.writeInt(LocalToLSBLong(16));
          fRef.writeShort(LocalToLSBShort((short) 1)); // wFormatTag
          fRef.writeShort(LocalToLSBShort((short) nbrChannels)); // wChannels
          fRef.writeInt(LocalToLSBLong((int) sampleRate)); // dwSamplesPerSec
          fRef.writeInt(LocalToLSBLong((int) (sampleRate*2*nbrChannels))); // dwAvgBytesPerSec
          fRef.writeShort(LocalToLSBShort((short) 2)); // wBlockAlign
          fRef.writeShort(LocalToLSBShort((short) 16)); // wSampleSize
          fRef.writeInt(CKID_data);
          fRef.writeInt(LocalToLSBLong(byteSize));
        }
        // System.out.println("WriteHeader done write");
        fileDataStart = (int) fRef.getFilePointer();
        // System.out.println("FileDataStart = " + fileDataStart);
      }
    }
    catch (IOException e)
    {
         System.out.println("Write header problem with " + fsSpec + ": " + e.toString());
         return -1;
    }
    return 0;
  }

  IFFChunk FindIFFChunk(int formID, int chunkID) throws IOException
  {
    int ckID, ckSize;
    int count;
    IFFChunk ck = new IFFChunk();
    while (true) {
      ckID = fRef.readInt();
      ckSize = fRef.readInt();
      if (ckID == chunkID) {
        ck.dataPos = (int) fRef.getFilePointer();
        ck.dataSize = ckSize;
        return ck;
      }
      else {
        switch (ckID) {
        case CKID_FORM:
          ckID = fRef.readInt();
          if (ckID == formID)
            continue;
          count = ckSize - 4;
          break;
        default:
          count = ckSize;
          break;
        }
        if ((count & 1) != 0)
          ++count;
        fRef.seek(fRef.getFilePointer()+count);
      }
    }
  }

  // SANE or (IEEE 754 extended)
  void Write80BitDouble(double v) throws IOException
  {
    byte[] vb = new byte[10];

    int exponent = (int) (Math.log(v)/Math.log(2))+16383;
    double  mantissa = v/Math.pow(2,exponent) - 1;

    // we are converting to a normalized IEEE 80 bit extended double - see
    //
    // high bit of byte[0] is always 0 (sign)
    // lower 15 bits of first two bytes are exponent
    // high bit of byte[2] is always 1 (this indicates 'normalized')
    // remaining 23 bits are mantissa (binary fraction)


    // compute 15-bit integer representation of matissa
    int imantissa = 0;
    for (int bit = 1; bit < 15; ++bit) // only need about 15 bits of precision..
    {
       if (mantissa > 1/Math.pow(2,bit))
       {
            imantissa &= (1 << (15 - bit));
            mantissa -= 1/(Math.pow(2,bit));
       }
    }
    vb[0] = (byte) ((exponent >> 8) & 0x0FF);
    vb[1] = (byte) (exponent & 0x0FF);
    vb[2] = (byte) (0x80 | ((imantissa >> 8) & 0x0FF)); // output imantissa + normalized bit
    vb[3] = (byte) (imantissa & 0x0FF);
    fRef.write(vb);
  }

  double Read80BitDouble() throws IOException
  {
    byte[] fb = new byte[8]; // e[2] f[8]
    short e = fRef.readShort();
    fRef.read(fb);
    int sign = ((e * 0x8000) > 0)? -1 : 1;
    e &= 0x7FFF;

    int p2 = e-16383;
    int mask = 0x40;
    int offset = 0;
    double f = 0;

    for (int i = 1; i <= 63; ++i)
    {
      if ((fb[offset] & mask) > 0)
        f += Math.pow(2,-i);
      mask >>= 1;
      if (mask == 0) {
        mask = 0x80;
        offset++;
      }
    }
   return sign*Math.pow(2,p2)*(1+f);
  }

  static boolean lastmode;

  short GetSample(int i)
  {
    try {

      synchronized(this)
      {
        // Invalid input cache if we are writing...
        if (writing && memPos < nbrFrames*nbrChannels) {
          FlushOutputBuffer();
          inCachePos = -1;
        }
    
        // Force reget if we've been writing
        if (lastmode != writing)
        {
          lastmode = writing;
          inCachePos = -1;
        }
    
        // Allocate Cache if Necessary
        if (inCache == null) {
          inCache = new short[InCacheSize];
          if (inCache == null)
            return 0;
          inCachePos = -1;  // Force reload of cache
        }
    
        // Load Cache if Necessary
        if (inCachePos == -1 || i < inCachePos || i >= inCachePos+InCacheSize)
        {
          inCachePos = i;
          GetSamples(inCachePos,InCacheSize,inCache);
        }
        return inCache[i-inCachePos];
       }
    }
    catch (Exception e)
    {
       System.out.println("Error reading from " + fsSpec + ": " + e.toString());
    }
    return 0;
  }

  int GetSamples(int start, int nbrSamples, short[] outBuffer)
  {
    int  oe=0;
    int  count;
    int  startOffset = 0;  // position in buffer to start writing

    try {

      synchronized(this)
      {
        // System.out.println("Getting Samples");

        // If writing, flush the output buffer, so we can read all samples from the file
        if (writing && memPos < nbrFrames*nbrChannels) {
          FlushOutputBuffer();
        }

        if (fRef == null) {
            if ((oe = OpenFile()) != noErr)
              return oe;
            if ((oe = ReadHeader()) != noErr)
              return oe;
        }

        if (nbrFrames == 0)
          return -1;

        if (nbrSamples == 0)
          return noErr;

        // Clear output buffer
        for (int i = 0; i < outBuffer.length; ++i)
          outBuffer[i] = 0;

        if (start >= nbrFrames*nbrChannels)
          return noErr;

        if (start < 0) {
          startOffset = -start;
          start = 0;
          nbrSamples -= startOffset;
        }
        if (startOffset >= nbrSamples)
          return noErr;

        if (start+nbrSamples > nbrFrames*nbrChannels)
          nbrSamples = nbrFrames*nbrChannels - start;

        if ((inType & ST_16BIT) != 0) {
          // SetFPos(fRef, fsFromStart, fileDataStart+start*sizeof(short));
          fRef.seek(fileDataStart+start*2);
          count= nbrSamples*2;

          byte[] ib = new byte[nbrSamples*2];
          fRef.read(ib);
          for (int i = 0 ; i < nbrSamples; ++i)
            outBuffer[startOffset+i] = (short) ((((short) ib[i*2]) << 8) | (short) ib[i*2+1]);

          if (fileType == FT_WAVE) {
            for (int i = 0; i < nbrSamples; ++i) {
              // Convert to big-endian
              outBuffer[startOffset+i] = LSBShortToLocal(outBuffer[startOffset+i]);
            }
          }
        }
        else
        {
          fRef.seek(fileDataStart+start);
          count= nbrSamples;
          byte[] ib = new byte[count];
          fRef.read(ib);

          // Convert 8-bit to 16 bit
          short signBit = (short) ((inType & ST_SIGNED) != 0? 0x80 : 0x00);
          for (int i = 0; i < nbrSamples; ++i)
          {
           short v = (short) (ib[i] ^ signBit);
           outBuffer[startOffset+i] = (short)  ((v | (v << 8)) ^ 0x8000);
          }
        }
      }
    }
    catch (IOException e)
    {
       System.out.println("Read error from " + fsSpec + ": " + e.toString());
       oe = -1;
    }
    catch (Exception e)
    {
       System.out.println("Error reading from " + fsSpec + ": " + e.toString());
       oe = -1;
    }
    return oe;
  }

  int StartStorage(int plannedSamples,double sampleRate, boolean listenFlag, int nbrChannels) // Start Buffered Storage
  {
    int oe = noErr;

    super.StartStorage(plannedSamples,sampleRate,listenFlag, nbrChannels);

    synchronized(this)
    {
      System.out.println("Start Storage");
      SetSampleRate(sampleRate);

      // Was using "planned samples here"

      CloseFile();

      // If No File Descriptor, Get One from User
      if (fsSpec.length() == 0) {
          return -1;
      }
      // Set memory buffer to correct size
      if (outbuf == null)
      {
        outbuf = new short[OutCacheSize];
        if (outbuf == null) {
          System.out.println("Out of Memory");
          return -1;
        }
        // DEBUG (no effect)
        // for (int i = 0; i < OutCacheSize; ++i) {
        //  outbuf[i]= 0x7FFF;
        // }
      }
      memPos = 0;
      nbrFrames = 0;

      // Open the File for Output
      try {
        // System.out.println("Opening output file: " + fsSpec);
        System.out.println("Opening file for write");
        fRef = new RandomAccessFile(fsSpec, "rw");
        fRef.setLength(0);
      }
      catch (IOException e)
      {
        System.out.println("Can't open for output: " + fsSpec + ": " + e.toString());
        return -1;
      }

      oe = WriteHeader();
      writing = true;
    }
    return oe;
  }

  int StoreSample(short sample)
  {
    int oe = noErr;
    try {
      synchronized(this)
      {
        if (nbrFrames+1 > memPos+OutCacheSize)
          FlushOutputBuffer();
        outbuf[(nbrFrames-memPos)] = sample;
        ++nbrFrames;
      }
    }
    catch (Exception e)
    {
        System.out.println("Can't store sample: " + e.toString());
        oe = -1;
    }
    return oe;
  }

  int StoreStereoSample(short sampleL, short sampleR)
  {
    int oe = noErr;
    try {
      synchronized(this)
      {
        if (nbrFrames*nbrChannels+2 > memPos+OutCacheSize)
          FlushOutputBuffer();
        outbuf[(nbrFrames*nbrChannels-memPos)] = sampleL;
        outbuf[(nbrFrames*nbrChannels+1-memPos)] = sampleR;
        ++nbrFrames;
      }
    }
    catch (Exception e)
    {
        System.out.println("Can't store sample: " + e.toString());
        oe = -1;
    }
    return oe;
  }

  int FlushOutputBuffer()
  {
    int oe = noErr;

    try {
      synchronized(this)
      {
        // System.out.println("Flushing buffers");
        // System.out.println("SSStoreFile: Flush");
        if (fRef == null) {
          System.out.println("File is closed");
          return -1;    // File is closed
        }
        int start = memPos;
        int sCount = nbrFrames*nbrChannels - start;
        if (sCount > 0) {
          if (PatchOwner.gVerbose > 2)
            System.out.println("Writing " + sCount + " samples");
          int bCount = sCount * 2;
          if (fileType == FT_WAVE) {
            for (int i = 0; i < sCount; ++i)
              outbuf[i] = LocalToLSBShort(outbuf[i]);
          }
          // SetFPos(fRef, fsFromStart, (long) (fileDataStart+start*sizeof(short)));
          fRef.seek(fileDataStart+start*2);

          byte[] oBuf = new byte[sCount*2];
          for (int i = 0; i < sCount; ++i) {
            oBuf[i*2] = (byte) (outbuf[i] >> 8);
            oBuf[i*2+1] = (byte) (outbuf[i] & 0x00FF);
          }
          fRef.write(oBuf);

          // for (int i = 0; i < sCount; ++i) {
          //   fRef.writeShort(outbuf[i]);
          // }
          // fflush(fRef);
        }
        memPos = nbrFrames*nbrChannels;
        
        // DEBUG (no effect)
        // for (int i = 0; i < OutCacheSize; ++i) {
        //  outbuf[i]= 0x7FFF;
        // }

      }
    }
    catch (IOException e)
    {
       System.out.println("Flush error from " + fsSpec + ": " + e.toString());
       oe = -1;
    }
    catch (Exception e)
    {
       System.out.println("Generic Flush error from " + fsSpec + ": " + e.toString());
       oe = -1;
    }
    return oe;
  }

  int StopStorage()  // Stop Buffered Storage (flush buffer)
  {
    int oe = noErr;
    System.out.println("StopStorage");
    try {
      synchronized(this) {
        if ((oe = FlushOutputBuffer()) != noErr)
          return oe;
        if ((oe = WriteHeader()) != noErr)
          return oe;
        if ((oe = CloseFile()) != noErr)
          return oe;
        writing = false;
        super.StopStorage();
      }
    }
    catch (Exception e)
    {
       System.out.println("Stop error " + e.toString());
       oe = -1;
    }
    return oe;
  }

  // Used for audio playback
  InputStream GetInputStream() throws IOException
  {
    return new BufferedInputStream(new FileInputStream(fsSpec));
  }


}
