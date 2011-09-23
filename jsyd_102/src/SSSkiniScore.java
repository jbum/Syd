import java.io.*;
import java.util.*;

public class SSSkiniScore extends SSScore
{
  String scoreFileSpec = "";

  SSSkiniScore(ModList mList)
  {
    super(MT_SkiniScore, mList);
  }

  static  double[] lastP = new double[16];
  static final char[] nbase = {0,2,3,5,7,8,10};
  static int octave = 4;

  // !! FIX THIS
  double GrabParam(String p, int pNbr)
  {
    double v;
    char  firstChar = p.toLowerCase().charAt(0);

    if (firstChar == 'm') {
      v = (double) Integer.parseInt(p.substring(1));
    }
    else if (firstChar >= 'a' && firstChar <= 'g')
    {
      int nval = nbase[firstChar-'a'];
      int pIdx = 1;
      if (p.charAt(pIdx) == '#')
      {
        ++nval;
        ++pIdx;
      }
      else if (p.charAt(pIdx) == 'b') {
        --nval;
        ++pIdx;
      }
      if (Character.isDigit(p.charAt(pIdx)))
        octave = p.charAt(pIdx) - '0';
      v = 21 + nval + 12*octave;
    }
    else {
      v = Double.parseDouble(p);
    }
    lastP[pNbr] = v;
    return v;
  }

  // !! FIX THIS
  void GenerateScore(SSModule callingMod)
  {
    // unsigned char *buffer;
    // ScoreSectionPtr sp;
    // NoteEventPtr  np;
    // char    *p;

    // Open the file
    BufferedReader ar = null;
    try {
      ar = new BufferedReader(new FileReader(scoreFileSpec));
    }
    catch (IOException e)
    {
      // !!! Try to open using path in global sydSpec
      int     li = PatchOwner.sydSpec.lastIndexOf('/');
      if (li == -1)
        li = PatchOwner.sydSpec.lastIndexOf('\\');
      if (li >= 0) {
        scoreFileSpec = PatchOwner.sydSpec.substring(0,li+1) + scoreFileSpec;
        System.out.println("Can't open score, trying " + scoreFileSpec);
        try {
          ar = new BufferedReader(new FileReader(scoreFileSpec));
        }
        catch (IOException e2) {
          System.out.println("Error opening score " + scoreFileSpec + ": " + e2.toString());
          parList.itsOwner.AbortSynthesis();
          return;
        }
      }
      else {
        System.out.println("Error opening score (retry path?): " + scoreFileSpec + ": " + e.toString());
        parList.itsOwner.AbortSynthesis();
        return;
      }
    }

    if (ar == null) {
      System.out.println("Can't open score file");
      parList.itsOwner.AbortSynthesis();
      return;
    }

    // Reset vars
    sections.clear();
    AddSection();
    ScoreSection sp = (ScoreSection) sections.firstElement();

    sp.sectionStart = 0.0;
    sp.tempoScale = 1.0;
    sp.tempoStart = 0.0;
    sp.timeStart = 0.0;

    double ampMult = 1;


    String  p;

    try {

    while ((p = ar.readLine()) != null &&
           parList.itsOwner.windowState == WS_Synthesize)
    {
      int pcIdx = 0;
      int pLen = p.length();

      while (pcIdx < pLen && Character.isWhitespace(p.charAt(pcIdx)))
        ++pcIdx;

      if (pcIdx >= pLen || p.charAt(pcIdx) == ';' || p.charAt(pcIdx) == '#' ||
          p.substring(pcIdx,pcIdx+2).equals("//"))
      {
        continue;
      }
      StringTokenizer st = new StringTokenizer(p.substring(pcIdx));
      String lab = st.nextToken();
      if (lab.equals("AmpMult"))
      {
        ampMult = GrabParam(st.nextToken(),1);
      }
      else if (lab.equals("BPM"))
      {
        double p1 = GrabParam(st.nextToken(),1);
        sp.timeStart = 0;
        sp.tempoStart = 0;
        sp.tempoScale = 60 / p1;
      }
      else if (lab.equals("Note")) // note event
      {
        double[] optP = new double[16];
        double p1 = GrabParam(st.nextToken(),1);
        double p2 = GrabParam(st.nextToken(),2);
        int     nbrOpts = 0;
        while (st.hasMoreTokens())
        {
           optP[nbrOpts++] = GrabParam(st.nextToken(),2+nbrOpts);
        }
        optP[0] = optP[0]*128*ampMult/1000; // Repair Amplitude
        NoteEvent np = AddNoteEvent(sp,0,1,p1,p2,nbrOpts);
        System.arraycopy(optP,0,np.op,0,nbrOpts);
      }
      else if (lab.equals("Synth") || lab.equals("Inst")) // section end
      {
      }
      else {
        System.out.println("Skini: Unrecognized line in score: " + p);
        // parList.itsOwner.AbortSynthesis();
      }
    }
    ar.close();
    }
    catch (IOException e)
    {
        System.out.println("Exception reading C Score: " + e.toString());
        parList.itsOwner.AbortSynthesis();
    }

    if (PatchOwner.gDurationOverride) {
      // System.out.println("Override duration?");
      double scoreDuration = (sp.sectionStart + sp.sectionLength) * sp.tempoScale;
      SSOutput outMod = (SSOutput) (parList.itsOwner.outMod);
      if (outMod != null && scoreDuration > outMod.sampleDuration)
      {
        System.out.println("Overriding duration " + outMod.sampleDuration + " -> " + scoreDuration);
        outMod.sampleDuration = scoreDuration;
        parList.itsOwner.mainInst.sampleDuration = scoreDuration;
      }
    }
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    ar.println("SKINISCO " + scoreFileSpec);
  }

  void Load(BufferedReader ar) throws IOException
  {
    scoreFileSpec = getFirstToken(ar,"SKINISCO");
  }

  void Copy(SSModule ss)
  {
    SSSkiniScore sc = (SSSkiniScore) ss;
    super.Copy(ss);
    this.scoreFileSpec = sc.scoreFileSpec;
  }


}