import java.io.*;
import java.util.*;

public class SSRandScore extends SSScore
{
  static final int MaxParams = 16;
  // statics
  public ExpRec    nbrNotesExp;
  public ExpRec  p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16;
  // Exp = new ExpRec[MaxParams]; // this will need work..

  // Dynamics
  double noteIndex;
  int    maxNotes = 0;

  public SSRandScore(ModList mList)
  {
    super(MT_RandScore, mList);
    nbrNotesExp = InitExp("p3 + 1");
    p1 = InitExp("1");                   // instrument
    p2 = InitExp("?*(p3-1)");            // start time
    p3 = InitExp("0.5+?*0.5");           // duration
    p4 = InitExp("0.25 + ?*0.25");       // amplitude (suggestion)
    p5 = InitExp("55*2**(?*4.0)");       // frequency (suggestion)
    p6 = InitExp("0");
    p7 = InitExp("0");
    p8 = InitExp("0");
    p9 = InitExp("0");
    p10 = InitExp("0");
    p11 = InitExp("0");
    p12 = InitExp("0");
    p13 = InitExp("0");
    p14 = InitExp("0");
    p15 = InitExp("0");
    p16 = InitExp("0");
//    for (int i = 5; i < MaxParams; ++i)
//      pExp[i] = InitExp("0");        // extra parameters

  }

  void AddRandomNoteToScore(ScoreSection sp, int inst,
                double start, double dur,
                double[] p)
  {
    NoteEvent  np;
    int nbrExtras;

    nbrExtras = CountActiveExpressions() - 3;

    np = AddNoteEvent(sp, 0, inst, start, dur, nbrExtras);
    for (int i = 0; i < nbrExtras; ++i)
      np.op[i] = p[i];
  }

  // Called from Reset
  void GenerateScore(SSModule callingMod)
  {
    // ssfloat  t;
    int   nbrInstruments = CountInputTypes();
    double[] p = new double[MaxParams-3];

    // Initialize Expressions
    ResetExp(nbrNotesExp, callingMod);
    ResetExp(p1, callingMod);
    ResetExp(p2, callingMod);
    ResetExp(p3, callingMod);
    ResetExp(p4, callingMod);
    ResetExp(p5, callingMod);
    ResetExp(p6, callingMod);
    ResetExp(p7, callingMod);
    ResetExp(p8, callingMod);
    ResetExp(p9, callingMod);
    ResetExp(p10, callingMod);
    ResetExp(p11, callingMod);
    ResetExp(p12, callingMod);
    ResetExp(p13, callingMod);
    ResetExp(p14, callingMod);
    ResetExp(p15, callingMod);
    ResetExp(p16, callingMod);

//    for (int i = 0; i < MaxParams; ++i)
//      ResetExp(pExp[i], callingMod);

    sections.clear();
    AddSection();

    ScoreSection sp = (ScoreSection) sections.firstElement();
    sp.sectionStart = 0.0;
    sp.tempoScale = 1.0;
    sp.tempoStart = 0.0;
    sp.timeStart = 0.0;

    // Possible variables:
    //    Event Density (average events per second)
    //      min dur, max dur
    //      min freq, max freq
    //                  !!! may need to pass time here...
    maxNotes = (int) SolveExp(nbrNotesExp, callingMod);
    double totalDur = callingMod.GetInstParameter(3);

    for (int n = 0; n < maxNotes; ++n) {

      noteTime = n; // Allows retrieval via "N"
      noteIndex = n;  // Allows retrieval via "I"

      int inst = (int) SolveExp(p1, callingMod);
      double start = SolveExp(p2, callingMod);
      double dur = SolveExp(p3, callingMod);

      if (start+dur > totalDur)
        dur = totalDur - start;

      p[0] = SolveExp(p4, callingMod);
      p[1] = SolveExp(p5, callingMod);
      p[2] = SolveExp(p6, callingMod);
      p[3] = SolveExp(p7, callingMod);
      p[4] = SolveExp(p8, callingMod);
      p[5] = SolveExp(p9, callingMod);
      p[6] = SolveExp(p10, callingMod);
      p[7] = SolveExp(p11, callingMod);
      p[8] = SolveExp(p12, callingMod);
      p[9] = SolveExp(p13, callingMod);
      p[10] = SolveExp(p14, callingMod);
      p[11] = SolveExp(p15, callingMod);
      p[12] = SolveExp(p16, callingMod);

//      for (int i = 4; i < MaxParams; ++i)
//        p[i-4] = SolveExp(pExp[i-1], callingMod);

      AddRandomNoteToScore(sp, inst,start,dur,p);
    }
  }

  double GetIValue()
  {
    return noteIndex;
  }

  double GetMValue()
  {
    return (double) maxNotes;
  }

  int CountActiveExpressions()
  {
    if (!p16.exp.equals("0"))
      return 16;
    if (!p15.exp.equals("0"))
      return 15;
    if (!p14.exp.equals("0"))
      return 14;
    if (!p13.exp.equals("0"))
      return 13;
    if (!p12.exp.equals("0"))
      return 12;
    if (!p11.exp.equals("0"))
      return 11;
    if (!p10.exp.equals("0"))
      return 10;
    if (!p9.exp.equals("0"))
      return 9;
    if (!p8.exp.equals("0"))
      return 8;
    if (!p7.exp.equals("0"))
      return 7;
    if (!p6.exp.equals("0"))
      return 6;
    if (!p4.exp.equals("0"))
      return 5;
    if (!p3.exp.equals("0"))
      return 4;
    if (!p3.exp.equals("0"))
      return 3;
    if (!p2.exp.equals("0"))
      return 2;
    if (!p1.exp.equals("0"))
      return 1;
//    for (int i = MaxParams-1; i >= 0; --i) {
//      if (!pExp[i].exp.equals("0"))
//        return i+1;
//    }
    return 0;
  }

  void Save(PrintWriter ar) throws IOException
  {
    super.Save(ar);
    int nbrActiveExpressions = CountActiveExpressions();
    ar.println("RSCO " + nbrActiveExpressions);
    ar.println("RSCON " + nbrNotesExp.exp);
    if (1 <= nbrActiveExpressions)
      ar.println("RSCO1 " + p1.exp);
    if (2 <= nbrActiveExpressions)
      ar.println("RSCO2 " + p2.exp);
    if (3 <= nbrActiveExpressions)
      ar.println("RSCO3 " + p3.exp);
    if (4 <= nbrActiveExpressions)
      ar.println("RSCO4 " + p4.exp);
    if (5 <= nbrActiveExpressions)
      ar.println("RSCO5 " + p5.exp);
    if (6 <= nbrActiveExpressions)
      ar.println("RSCO6 " + p6.exp);
    if (7 <= nbrActiveExpressions)
      ar.println("RSCO7 " + p7.exp);
    if (8 <= nbrActiveExpressions)
      ar.println("RSCO8 " + p8.exp);
    if (9 <= nbrActiveExpressions)
      ar.println("RSCO9 " + p9.exp);
    if (10 <= nbrActiveExpressions)
      ar.println("RSCO10 " + p10.exp);
    if (11 <= nbrActiveExpressions)
      ar.println("RSCO11 " + p11.exp);
    if (12 <= nbrActiveExpressions)
      ar.println("RSCO12 " + p12.exp);
    if (13 <= nbrActiveExpressions)
      ar.println("RSCO13 " + p13.exp);
    if (14 <= nbrActiveExpressions)
      ar.println("RSCO14 " + p14.exp);
    if (15 <= nbrActiveExpressions)
      ar.println("RSCO15 " + p15.exp);
    if (16 <= nbrActiveExpressions)
      ar.println("RSCO16 " + p16.exp);
  }


  void Load(BufferedReader ar) throws IOException
  {
    int   nbrActiveExpressions;

    String p = parList.GetNextInputLine(ar,"RSCO");
    if (p.charAt(4) == '#') {  // Old Style
      nbrNotesExp = InitExp(p.substring(6));
      nbrActiveExpressions = 8;
    }
    else {
      StringTokenizer st = new StringTokenizer(p);
      st.nextToken();              // RSCO
      nbrActiveExpressions = Integer.parseInt(st.nextToken());
      nbrNotesExp = LoadExp(ar,"RSCON");
    }
    if (1 <= nbrActiveExpressions)
      p1 = LoadExp(ar,"RSCO1");
    else
      p1 = InitExp("0");
    if (2 <= nbrActiveExpressions)
      p2 = LoadExp(ar,"RSCO2");
    else
      p2 = InitExp("0");
    if (3 <= nbrActiveExpressions)
      p3 = LoadExp(ar,"RSCO3");
    else
      p3 = InitExp("0");
    if (4 <= nbrActiveExpressions)
      p4 = LoadExp(ar,"RSCO4");
    else
      p4 = InitExp("0");
    if (5 <= nbrActiveExpressions)
      p5 = LoadExp(ar,"RSCO5");
    else
      p5 = InitExp("0");
    if (6 <= nbrActiveExpressions)
      p6 = LoadExp(ar,"RSCO6");
    else
      p6 = InitExp("0");
    if (7 <= nbrActiveExpressions)
      p7 = LoadExp(ar,"RSCO7");
    else
      p7 = InitExp("0");
    if (8 <= nbrActiveExpressions)
      p8 = LoadExp(ar,"RSCO8");
    else
      p8 = InitExp("0");
    if (9 <= nbrActiveExpressions)
      p9 = LoadExp(ar,"RSCO9");
    else
      p9 = InitExp("0");
    if (10 <= nbrActiveExpressions)
      p10 = LoadExp(ar,"RSCO10");
    else
      p10 = InitExp("0");
    if (11 <= nbrActiveExpressions)
      p11 = LoadExp(ar,"RSCO11");
    else
      p11 = InitExp("0");
    if (12 <= nbrActiveExpressions)
      p12 = LoadExp(ar,"RSCO12");
    else
      p12 = InitExp("0");
    if (13 <= nbrActiveExpressions)
      p13 = LoadExp(ar,"RSCO13");
    else
      p13 = InitExp("0");
    if (14 <= nbrActiveExpressions)
      p14 = LoadExp(ar,"RSCO14");
    else
      p14 = InitExp("0");
    if (15 <= nbrActiveExpressions)
      p15 = LoadExp(ar,"RSCO15");
    else
      p15 = InitExp("0");
    if (16 <= nbrActiveExpressions)
      p16 = LoadExp(ar,"RSCO16");
    else
      p16 = InitExp("0");

/*    for (int i = 0; i < nbrActiveExpressions; ++i) {
      pExp[i] = LoadExp(ar,"RSCO"+(i+1));
    }
    for (int i = nbrActiveExpressions; i < MaxParams; ++i) {
      pExp[i] = InitExp("0");
    }
 */
  }



  void Copy(SSModule ss)
  {
    SSRandScore sr = (SSRandScore) ss;
    super.Copy(ss);
    CopyExp(sr.nbrNotesExp, nbrNotesExp);
    CopyExp(sr.p1, p1);
    CopyExp(sr.p2, p2);
    CopyExp(sr.p3, p3);
    CopyExp(sr.p4, p4);
    CopyExp(sr.p5, p5);
    CopyExp(sr.p6, p6);
    CopyExp(sr.p7, p7);
    CopyExp(sr.p8, p8);
    CopyExp(sr.p9, p9);
    CopyExp(sr.p10, p10);
    CopyExp(sr.p11, p11);
    CopyExp(sr.p12, p12);
    CopyExp(sr.p13, p13);
    CopyExp(sr.p14, p14);
    CopyExp(sr.p15, p15);
    CopyExp(sr.p16, p16);
//    for (int i = 0; i < MaxParams; ++i)
//      CopyExp(sr.pExp[i], pExp[i]);
  }


}
