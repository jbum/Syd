import java.io.*;
import java.util.*;

public class SSCScore extends SSScore
{
  public String scoreFileSpec = "";

  SSCScore(ModList mList)
  {
    super(MT_CScore, mList);
  }

  static  double[] lastP = new double[16];

  double GrabParam(String p, int pNbr)
  {
    double v;
    if (p.equals("+") && pNbr == 2)
      v = lastP[2]+lastP[3];
    else if (p.equals("."))
      v = lastP[pNbr];
    else
      v = Double.parseDouble(p);
    lastP[pNbr] = v;
    return v;
  }

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
      System.out.println("Couldn't open " + scoreFileSpec + " sydspec=" + PatchOwner.sydSpec);
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
    System.out.println("Resetting");
    // Reset vars
    sections.clear();
    AddSection();
    ScoreSection sp = (ScoreSection) sections.firstElement();

    System.out.println("Initting");
    sp.sectionStart = 0.0;
    sp.tempoScale = 1.0;
    sp.tempoStart = 0.0;
    sp.timeStart = 0.0;

    String  p;

    try {

    while ((p = ar.readLine()) != null &&
           parList.itsOwner.windowState == WS_Synthesize)
    {
      int pcIdx = 0;
      int pLen = p.length();

      while (pcIdx < pLen && Character.isWhitespace(p.charAt(pcIdx)))
        ++pcIdx;

      if (pcIdx >= pLen || p.charAt(pcIdx) == ';' || p.charAt(pcIdx) == '#')
      {
        continue;
      }
      StringTokenizer st = new StringTokenizer(p.substring(pcIdx));
      String lab = st.nextToken();
      if (lab.equals("t")) // tempo
      {
        double p1 = GrabParam(st.nextToken(),1);
        double p2 = GrabParam(st.nextToken(),2);

        sp.timeStart = (p1 - sp.tempoStart) * sp.tempoScale + sp.timeStart;
        sp.tempoStart = p1;
        sp.tempoScale = 60 / p2;
      }
      else if (lab.equals("f")) // table  (f0 is used to create silence)
      {
        // !!! Add wave table event NEF_Function
      }
      else if (lab.equals("i")) // note event
      {
                    // !!!
                    // '<' is for ramping (mark 'begin ramp')
                    //  '>' treat as for ramping as well
        double p1 = GrabParam(st.nextToken(),1);
        double p2 = GrabParam(st.nextToken(),2);
        double p3 = GrabParam(st.nextToken(),3);
        double[] optP = new double[16];
        int     nbrOpts = 0;
        while (st.hasMoreTokens())
        {
           optP[nbrOpts++] = GrabParam(st.nextToken(),4+nbrOpts);
        }
        NoteEvent np = AddNoteEvent(sp,0,(int) p1,p2,p3,nbrOpts);
        System.arraycopy(optP,0,np.op,0,nbrOpts);
      }
      else if (lab.equals("s")) // section end
      {
        ScoreSection lastS = sp;
        AddSection();
        sp = (ScoreSection) sections.lastElement();

        sp.sectionStart = lastS.sectionStart+lastS.sectionLength;
        sp.tempoScale = 1.0;
        sp.tempoStart = 0.0;
        sp.timeStart = 0.0;
      }
      else if (p.charAt(pcIdx) == 'e') // end of score
      {
        break;
      }
      else {
        System.out.println("Invalid line in score: " + p);
        parList.itsOwner.AbortSynthesis();
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
    ar.println("CSCO " + scoreFileSpec);
  }

  void Load(BufferedReader ar) throws IOException
  {
    scoreFileSpec = getFirstToken(ar,"CSCO");
  }

  void Copy(SSModule ss)
  {
    SSCScore sc = (SSCScore) ss;
    super.Copy(ss);
    this.scoreFileSpec = sc.scoreFileSpec;
  }


}