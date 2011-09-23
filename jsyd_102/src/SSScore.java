import java.io.*;
import java.util.*;


public class SSScore extends SSModule implements SydConstants
{
  // int           nbrSections = 0; // can probably omit
  Vector sections = new Vector();
  ScoreSection curSection = null;
  int          sectionIndex = 0;
  double       noteTime = 0;
  NoteEvent    instParams;

  // int       nbrActiveNotes = 0,
  //          allocActiveNotes = 0; // can probably omit these

  Vector activeNotes = new Vector();
  boolean     scoreDone = true;
  double     curTempo = 60;

  public SSScore(int itsType, ModList mList)
  {
    super(itsType, mList);
    DescribeLink(0, "Instrument 1", "i1");
    DescribeLink(1, "Instrument 2", "i2");
    DescribeLink(2, "Instrument 3", "i3");
    DescribeLink(3, "Instrument 4", "i4");
    DescribeLink(4, "Instrument 5", "i5");
    DescribeLink(5, "Instrument 6", "i6");
    DescribeLink(6, "Instrument 7", "i7");
    DescribeLink(7, "Instrument 8", "i8");
  }

  void CleanUp()
  {
    TerminateActiveNotes();
    Enumeration e = sections.elements();
    while (e.hasMoreElements())
    {
      ScoreSection sp = (ScoreSection) e.nextElement();
      sp.notes.clear();
    }
    sections.clear();
    activeNotes.clear();

    super.CleanUp();
  }

  // Might be useful to display first, and give opp. to change it
  double GenerateOutput(SSModule callingMod)
  {
    double     pTime = parList.pTime;


    this.callingMod = callingMod;
    if (scoreDone)
      return 0.0;


    if (pTime >= curSection.sectionStart+curSection.sectionLength) {
      NextSection(pTime);
      if (scoreDone)
        return 0.0;
    }

    // given the time, determine which instruments are sounding
    while (curSection.noteNbr < curSection.notes.size() &&
        ((NoteEvent) curSection.notes.elementAt(curSection.noteNbr)).p2 + curSection.sectionStart <= pTime &&
        parList.itsOwner.windowState == WS_Synthesize)
    {
      ActivateNextNote(pTime);
    }

    if (parList.itsOwner.windowState != WS_Synthesize)
      return 0.0;

    // call each instrument's Generate Output with the note time
    // add the results
    double v = 0.0, vR = 0.0;
    int activeNoteNbr = 0;

    Enumeration e = activeNotes.elements();

    while (e.hasMoreElements() && parList.itsOwner.windowState == WS_Synthesize)
    {
      ActiveNote ap = (ActiveNote) e.nextElement();

      // Has Note Elapsed?  If so, kill it
      if (curSection.sectionStart + ap.noteInfo.p2 + ap.noteInfo.p3 <= pTime)
      {
        activeNotes.remove(ap);
      }
      else { // otherwise, play it, if the instrument has modules
        if (ap.instr != null && ap.instr.mods.size() >= 1) {
          noteTime = pTime - (curSection.sectionStart+ap.noteInfo.p2);
          this.instParams = ap.noteInfo;
          v += ((SSModule) ap.instr.mods.firstElement()).GenerateOutputTime(this,noteTime);
          vR += ((SSModule) ap.instr.mods.firstElement()).getRightSample();
        }
        ++activeNoteNbr;
      }
    }
    lastRightSample = vR;
    return v;
  }

  double GetInstParameter(int n)
  {
    // System.out.println("Score: GetInstParam " + n);
    if (instParams == null)
      return 0.0;

    switch (n) {
    case 0:  return noteTime;
    case 1:  return instParams.p1;
    case 2:  return instParams.p2;
    case 3:  return instParams.p3;
    default:
      if (n - 4 < instParams.op.length)
        return instParams.op[n-4];
      else
        return 0.0;
    }
  }

  boolean IsInScore()
  {
    return true;
  }

  // Does double duty: when generating notes, it's the time of each note.
  // When generating score, it's the ratio of note/maxnotes
  //
  double GetNoteTime()
  {
    return noteTime;
  }

  void ActivateNextNote(double pTime)
  {
    // Add note to active notes, and initialize it
    InitializeActiveNote((NoteEvent) curSection.notes.elementAt(curSection.noteNbr));

    // increment note ptr or set to null
    ++curSection.noteNbr;
  }

  void InitializeActiveNote(NoteEvent  np)
  {
    if (PatchOwner.gVerbose > 2)
      System.out.println("Initializing Active Note");
    int instNbr;

    if ((np.flags & NoteEvent.NEF_Tempo) != 0) {
      curTempo = np.p2;
    }
    else {
      ActiveNote  ap = new ActiveNote();
      activeNotes.add(ap);

      ap.noteInfo = np;
      ap.instr = null;

      instNbr = np.p1 -1;
      if (CountInputs(instNbr) == 0) {
        System.out.println("A note refers to a non-existent instrument (instNbr=" + instNbr + ")");
        if (PatchOwner.gVerbose > 1)
          System.exit(1);
      }
      else {

        Enumeration e = inputs.elements();
        while (e.hasMoreElements())
        {
          ModInput  inp = (ModInput) e.nextElement();

          if (instNbr == -1 || inp.inputType == instNbr) {
            // ??? We *could* support multiple instruments per note...
            // ??? But we're not...
            ap.instr = parList.CloneInstrument(inp.link);
            break;
          }
        }
        // Reset the Instrument
        if (ap.instr != null) {
          noteTime = 0;
          this.instParams = ap.noteInfo;
          if (PatchOwner.gVerbose > 1)
            System.out.println("Score: Reset Instruments");
          ap.instr.ResetInstruments(this);
        }
        else {
          System.out.println("A note refers to a non-existent instrument\n");
        }
      }
    }
  }

  void TerminateActiveNotes()
  {
    activeNotes.clear();
  }

  void EndScore()
  {
    scoreDone = true;
    CleanUp();
    curSection = null;
    sectionIndex = 0;
  }

  void NextSection(double pTime)
  {
    TerminateActiveNotes();
    ++sectionIndex;
    if (sectionIndex >= sections.size()) {
      EndScore();
      return;
    }
    curSection = (ScoreSection) sections.elementAt(sectionIndex);
    curSection.sectionStart = pTime;
    curSection.noteNbr = 0;
  }

  void AddSection()
  {
    ScoreSection sp = new ScoreSection();
    sections.add(sp);
    sp.tempoScale = 1.0;
    sp.tempoStart = 0.0;
    sp.timeStart = 0.0;
  }

  NoteEvent AddNoteEvent(ScoreSection sp, int flags, int inst,
                double start, double dur, int nbrOptParams)
  {
    int nbrExtraParams = 0;
    NoteEvent  np = new NoteEvent();
    double endNoteTime;
    sp.notes.add(np);
    if (nbrOptParams > 1)
      nbrExtraParams = nbrOptParams-1;
    endNoteTime= start+dur;
    if (endNoteTime > sp.sectionLength)
      sp.sectionLength = endNoteTime;
    np.flags = flags;
    np.p1 = inst;
    np.p2 = (start - sp.tempoStart) * sp.tempoScale + sp.timeStart;
    np.p3 = dur * sp.tempoScale;
    np.op = new double[nbrOptParams];
    return np;
  }


  // Load in score, sort it
  // !! Note this is the only part that is file dependent
  //   - We should also make some other score modules which
  //   - Overwrite this part....

  void Reset(SSModule callingMod)
  {
    try {
      super.Reset(callingMod);
      sections.clear();
      curTempo = 60.0;
      noteTime = 0.0;
    }
    catch (Exception e) {
      System.out.println(e.toString());
      e.printStackTrace();
      System.exit(1);
    }
  }

  void InitScore(SSModule callingMod)
  {
    if (PatchOwner.gVerbose > 1)
      System.out.println("InitScore: " + label);
    GenerateScore(callingMod);

    //
    // Sort Pass
    //
    SortScore();

    //
    // Resolve Ramps Pass
    //
    PostScoreProcessing();

    // Initialize for sound generation
    //
    activeNotes.clear();
    sectionIndex = 0;
    if (sections.size() > 0) {
      curSection = (ScoreSection) sections.firstElement();
      curSection.sectionStart = 0.0;
      curSection.noteNbr = 0;
      scoreDone = false;
    }
    else {
      EndScore();
    }
  }

  void SortScore()
  {
    // For each section;
    // Sort the notes in that section
    Enumeration e = sections.elements();
    while (e.hasMoreElements())
    {
      ScoreSection sp = (ScoreSection) e.nextElement();
      Collections.sort(sp.notes);
    }

  }

  // To be overridden
  void GenerateScore(SSModule callingMod)
  {
  }

  void PostScoreProcessing()
  {
  }

}
