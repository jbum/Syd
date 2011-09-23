import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.*;
import java.text.DecimalFormat;
import java.util.*;
import java.net.URL;
import java.lang.reflect.*;
import java.io.*;

public class SydPanel extends JPanel implements MouseListener, MouseMotionListener, ActionListener,
                                                PopupMenuListener, KeyListener,
                                                ComponentListener, SydConstants
{
  public static final long serialVersionUID = 1L;
  private static final int PWIDTH = 1024;   // preferred size of panel
  private static final int PHEIGHT = 800;
  public static final int UnitDefWidth = 32;
  public static final int NodeWidth = 40;

  // private Thread animator;            // for the animation
  private SynthThread itsSynthThread = null;
  private volatile boolean running = false;    // stops the animation

  private volatile boolean gameOver = false;   // for game termination

  // more variables, explained later
  //       :
  private Graphics dbg;
  private Image dbImage = null;
  private DecimalFormat df = new DecimalFormat("0.##");  // 2 dp
  private Font font;
  private FontMetrics metrics;

  private boolean isDragging = false;
  private boolean didDragging = false;
  private int doUpdate = 4;

  SSModule  curEditNode = null;


  MediaTracker     tracker;
  static Image     bgImage;
  static Image     stampImage;

  static final int ICON_OSC = 0;
  static final int ICON_FILTER = 1;
  static final int ICON_SCORE = 2;
  static final int ICON_OUTPUT = 3;

  UnitDef[]  unitDefs = new UnitDef[MT_NbrModules];

  static final int PatchOutputLMargin = (7+8);
  static final int PatchOutputRMargin = 4;

  // Vector unitNodes = new Vector();
  PatchOwner  itsPatch = new PatchOwner();
  ModList   unitNodes = itsPatch.mainInst;
  SSModule  dragNode = null;
  int       dragOX = 0;
  int       dragOY = 0;

  // Link stuff...
  boolean   linkDragging = false;
  Point     linkAnchor, linkPoint;
  SSModule  linkSource=null, linkTarget=null;
  int       linkType=0;
  JPopupMenu linkMenu = null;
  ModInput   linkMenuInput = null;

  // Zap-linking stuff
  Point     startPoint=null, lastPoint=null, curPoint=null;
  long      startPointTime=0, lastPointTime=0, curPointTime=0;
  double    dragVecAngle, dragVecLength, dragVecSpeed;
  boolean   isDirty = false;

  boolean   rectDragging = false;
  Point     rectAnchor, rectPoint;

  boolean   grabDragging = false;

  // Display offset
  Point     dOffset = new Point(0,0);
  boolean   useZapDragging = false;

  JSydApp   itsApp;

  static final int[] pOrder = {
    MT_Expression,MT_Noise,MT_Maraca,MT_HammerActuator,MT_Folder,
    MT_FTable,MT_Mixer,MT_Inverter,MT_Filter,
    MT_SampleAndHold,MT_CScore,MT_SkiniScore,MT_PInput,MT_Output,
    // front row
    MT_Oscillator,MT_Pluck,MT_HammerBank,MT_SampleFile,
    MT_Envelope,MT_Amplifier,MT_Smooth,MT_Butter,MT_Delay,
    MT_Threshhold,MT_RandScore,MT_FInput,MT_GAssign};


  void InitUnitDefs()
  {

    // Eventually do this with text file...
    unitDefs[MT_Output] = new UnitDef(MT_Output,"out", "Output", ICON_OUTPUT, 0);
      unitDefs[MT_Output].DescribeLink(0, "Output Signal", "sig",0,0,0xFF);
      String[] outputTypeStrs = {"Memory","AIFF File","WAV File"};
      String[] legalSoundFileSuffixes = {".aif",".wav",".wave",".aiff"};
      String[] channelStrs = {"Mono","Stereo"};
      unitDefs[MT_Output].AddParameter(new ParamDef_RadioGroup("outputType", "Output To", outputTypeStrs, "2"));
      unitDefs[MT_Output].AddParameter(new ParamDef_RadioGroup("isStereo", "Channels", channelStrs, "0"));
      unitDefs[MT_Output].AddParameter(new ParamDef_Filename("outFileSpec", "Audio File", "untitled.wav", true,legalSoundFileSuffixes));
      unitDefs[MT_Output].AddParameter(new ParamDef_Float("sampleDuration", "Duration", "2"));
      unitDefs[MT_Output].AddParameter(new ParamDef_Float("sampleRate", "Rate", "22050"),ParamDef.NOBREAK);
      // unitDefs[MT_Output].SetOutputFormat("OUTPUT <sampleDuration> <sampleRate> <outputType> 0 0 <outFileSpec>");

    unitDefs[MT_Oscillator] = new UnitDef(MT_Oscillator,"osc", "Oscillator", ICON_OSC, 1);
      unitDefs[MT_Oscillator].DescribeLink(0, "Amplitude Modulation", "am",0,0,0xFF);
      unitDefs[MT_Oscillator].DescribeLink(1, "Frequency Modulation", "fm",0,0xFF,0);
      unitDefs[MT_Oscillator].DescribeLink(2, "FM Gain", "fmgain",0xFF,0x88,0);
      unitDefs[MT_Oscillator].DescribeLink(3, "AM Gain", "amgain",0,0x88,0xFF);
      unitDefs[MT_Oscillator].AddParameter(new ParamDef_Expression("fExp", "Frequency", "440",16));
      unitDefs[MT_Oscillator].AddParameter(new ParamDef_Expression("aExp", "Amplitude", "1",16),ParamDef.NOBREAK);
      unitDefs[MT_Oscillator].AddParameter(new ParamDef_Expression("pExp", "Phase", "0"));
      String[] waveTypeStrs = {"Sine","Sawtooth","Square","Triangle","BL Square","Expression"};
      unitDefs[MT_Oscillator].AddParameter(new ParamDef_RadioGroup("waveType", "Waveform", waveTypeStrs, "0"));
      unitDefs[MT_Oscillator].AddParameter(new ParamDef_Expression("wExp", "Expression", ""));
      // unitDefs[MT_Oscillator].SetOutputFormat("OSCI <waveType>\nOSCF <fExp>\nOSCA <aExp>\nOSCP <pExp>\nWEXP <wExp>");

    unitDefs[MT_Envelope] = new UnitDef(MT_Envelope,"env",    "Envelope Generator", ICON_OSC, 2);
      unitDefs[MT_Envelope].DescribeLink(0, "Trigger Signal", "trig",0xFF,0,0);
      // !!! Consider building a table input for this...
      unitDefs[MT_Envelope].AddParameter(new ParamDef_Expression("attackTExp", "Attack Time", "0.1"));
      unitDefs[MT_Envelope].AddParameter(new ParamDef_Expression("attackLExp", "Attack Level", "1"),ParamDef.NOBREAK);
      unitDefs[MT_Envelope].AddParameter(new ParamDef_Expression("decayTExp", "Decay Time", "0.2"));
      unitDefs[MT_Envelope].AddParameter(new ParamDef_Expression("decayLExp", "Decay Level", "0.7"),ParamDef.NOBREAK);
      unitDefs[MT_Envelope].AddParameter(new ParamDef_Expression("sustainTExp", "Sustain Time", "0.6"));
      unitDefs[MT_Envelope].AddParameter(new ParamDef_Expression("sustainLExp", "Sustain Level", "0.6"),ParamDef.NOBREAK);
      unitDefs[MT_Envelope].AddParameter(new ParamDef_Expression("releaseTExp", "Release Time", "0.1"));
      unitDefs[MT_Envelope].AddParameter(new ParamDef_Expression("durExp", "Duration", "1.0"));
      unitDefs[MT_Envelope].AddParameter(new ParamDef_Checkbox("useTrigger", "Use Trigger", "0"));
      unitDefs[MT_Envelope].AddParameter(new ParamDef_Checkbox("interruptsOK", "Interrupts OK", "0"));
      // unitDefs[MT_Envelope].SetOutputFormat("ADSR <attackT> <attackL> <decayT> <decayL> <sustainT> <sustainL> <releaseT>\nADSRD <durExp>");

    unitDefs[MT_Mixer] = new UnitDef(MT_Mixer,"mix", "Mixer", ICON_FILTER, 3);
      unitDefs[MT_Mixer].DescribeLink(0, "Signal to Mix", "sig",0,0,0xFF);

    unitDefs[MT_Filter] = new UnitDef(MT_Filter,"filt",   "2nd-Order Filter",  ICON_FILTER, 4);
      unitDefs[MT_Filter].DescribeLink(0, "Signal to Filter", "sig",0,0,0xFF);
      unitDefs[MT_Filter].DescribeLink(1, "Control Signal", "ctl",  0xFF,0x11,0xee);
      unitDefs[MT_Filter].DescribeLink(2, "Alt Control Sig #1", "ctl1", 0xFF,0x22,0xdd);
      unitDefs[MT_Filter].DescribeLink(3, "Alt Control Sig #2", "ctl2", 0xFF,0x33,0xcc);
      unitDefs[MT_Filter].DescribeLink(4, "Alt Control Sig #3", "ctl3", 0xFF,0x44,0xbb);
      unitDefs[MT_Filter].DescribeLink(5, "Alt Control Sig #4", "ctl4", 0xFF,0x55,0xaa);
      unitDefs[MT_Filter].DescribeLink(6, "Alt Control Sig #5", "ctl5", 0xFF,0x66,0x99);
      unitDefs[MT_Filter].DescribeLink(7, "Alt Control Sig #6", "ctl6", 0xFF,0x77,0x88);
      unitDefs[MT_Filter].DescribeLink(8, "Alt Control Sig #7", "ctl7", 0xFF,0x88,0x77);
      unitDefs[MT_Filter].DescribeLink(9, "Alt Control Sig #8", "ctl8", 0xFF,0x99,0x66);
      unitDefs[MT_Filter].AddParameter(new ParamDef_Expression("a0Exp", "a0", "0.5"));
      unitDefs[MT_Filter].AddParameter(new ParamDef_Expression("a1Exp", "a1", "0.5"));
      unitDefs[MT_Filter].AddParameter(new ParamDef_Expression("a2Exp", "a2", "0"));
      unitDefs[MT_Filter].AddParameter(new ParamDef_Expression("b1Exp", "b1", "0"));
      unitDefs[MT_Filter].AddParameter(new ParamDef_Expression("b2Exp", "b2", "0"));
      // unitDefs[MT_Filter].SetOutputFormat("FLTA0 <a0Exp>\nFLTA1 <a1Exp>\nFLTA2 <A2Exp>\nFLTB1 <b1Exp>\nFLTB2 <b2Exp>");

    unitDefs[MT_Butter] = new UnitDef(MT_Butter,"butter",   "Butterworth Filter",  ICON_FILTER, 5);
      unitDefs[MT_Butter].DescribeLink(0, "Signal to Filter", "sig",0,0,0xFF);
      unitDefs[MT_Butter].DescribeLink(1, "Control Signal", "ctl",  0xFF,0x11,0xee);
      unitDefs[MT_Butter].DescribeLink(2, "Alt Control Sig #1", "ctl1", 0xFF,0x22,0xdd);
      unitDefs[MT_Butter].DescribeLink(3, "Alt Control Sig #2", "ctl2", 0xFF,0x33,0xcc);
      unitDefs[MT_Butter].DescribeLink(4, "Alt Control Sig #3", "ctl3", 0xFF,0x44,0xbb);
      unitDefs[MT_Butter].DescribeLink(5, "Alt Control Sig #4", "ctl4", 0xFF,0x55,0xaa);
      unitDefs[MT_Butter].DescribeLink(6, "Alt Control Sig #5", "ctl5", 0xFF,0x66,0x99);
      unitDefs[MT_Butter].DescribeLink(7, "Alt Control Sig #6", "ctl6", 0xFF,0x77,0x88);
      unitDefs[MT_Butter].DescribeLink(8, "Alt Control Sig #7", "ctl7", 0xFF,0x88,0x77);
      unitDefs[MT_Butter].DescribeLink(9, "Alt Control Sig #8", "ctl8", 0xFF,0x99,0x66);
      String[] butterTypeStrs = {"Low Pass","High Pass","Band Pass","Band Reject"};
      unitDefs[MT_Butter].AddParameter(new ParamDef_RadioGroup("filterType", "Filter Type", butterTypeStrs, "2"));
      unitDefs[MT_Butter].AddParameter(new ParamDef_Expression("freqExp", "Freq", "1200"));
      unitDefs[MT_Butter].AddParameter(new ParamDef_Expression("bwExp", "Band", "1200*0.1"));
      // unitDefs[MT_Butter].SetOutputFormat("BUTTER <filterType>\nBUTTERF <freqExp>\nBUTTERB <bwExp>");

    unitDefs[MT_Smooth] = new UnitDef(MT_Smooth,"smooth", "Smoother",  ICON_FILTER, 6);
      unitDefs[MT_Smooth].DescribeLink(0, "Signal to Smooth", "sig",0,0,0xFF);

    unitDefs[MT_Noise] = new UnitDef(MT_Noise,"noise", "Noise Generator",  ICON_OSC, 7);
      unitDefs[MT_Noise].DescribeLink(0, "INVALID", "???",0xFF,0xFF,0xFF);
      unitDefs[MT_Noise].AddParameter(new ParamDef_Integer("startSeed", "Seed", "1"));
      unitDefs[MT_Noise].AddParameter(new ParamDef_Checkbox("randomize", "Randomize", "1"));
      // unitDefs[MT_Noise].SetOutputFormat("RND <randomize> <startSeed>");

    unitDefs[MT_Delay] = new UnitDef(MT_Delay,"delay", "Reverb / Effects",  ICON_FILTER, 8);
      unitDefs[MT_Delay].DescribeLink(0, "Signal to Delay", "sig",0,0,0xFF);
      unitDefs[MT_Delay].DescribeLink(1, "Control Signal", "ctl",0xFF,0x88,0x88);
      unitDefs[MT_Delay].DescribeLink(2, "Control Signal 1", "ctl1",0xFF,0x88,0);
      unitDefs[MT_Delay].DescribeLink(3, "Control Signal 2", "ctl2",0xFF,0x88,0);
      unitDefs[MT_Delay].DescribeLink(4, "Control Signal 3", "ctl3",0xFF,0x88,0);
      unitDefs[MT_Delay].AddParameter(new ParamDef_Expression("delayExp", "Delay", "0.1"));
      unitDefs[MT_Delay].AddParameter(new ParamDef_Expression("a0Exp", "a0", "1.0"));
      unitDefs[MT_Delay].AddParameter(new ParamDef_Expression("a1Exp", "a1", "0.5"));
      unitDefs[MT_Delay].AddParameter(new ParamDef_Checkbox("flags", "Feedback", "1"));
      // unitDefs[MT_Delay].SetOutputFormat("DELF <flags>\nDELd <delayExp>\nDELa0 <a0Exp>\nDELa1 <a1Exp>");

    unitDefs[MT_Threshhold] = new UnitDef(MT_Threshhold,"thresh", "Threshhold Unit",  ICON_FILTER, 9);
      unitDefs[MT_Threshhold].DescribeLink(0, "Signal for Threshhold", "sig",0,0,0xFF);
      unitDefs[MT_Threshhold].AddParameter(new ParamDef_Expression("cutOffExp", "Cutoff", "0.5"));
      // unitDefs[MT_Threshhold].SetOutputFormat("THR <cutOffExp>");

    unitDefs[MT_SampleAndHold] = new UnitDef(MT_SampleAndHold,"shold", "Sample and Hold",  ICON_FILTER, 10);
      unitDefs[MT_SampleAndHold].DescribeLink(0, "Sampled Signal", "sig",0,0,0xFF);
      unitDefs[MT_SampleAndHold].DescribeLink(1, "Trigger Signal", "trig",0xFF,0x00,0x00);

    unitDefs[MT_Amplifier]  = new UnitDef(MT_Amplifier,"amp",   "Amplifier",  ICON_FILTER, 11);
      unitDefs[MT_Amplifier].DescribeLink(0, "Signal to Amplify", "sig",0,0,0xFF);
      // unitDefs[MT_Amplifier].DescribeLink(1, "Signal to Amplify (Right)", "sigR",0x77,0x77,0xFF);
      unitDefs[MT_Amplifier].DescribeLink(1, "Control Signal", "ctl",0xFF,0x88,0x88);
      unitDefs[MT_Amplifier].DescribeLink(2, "Control Signal 1", "ctl1",0xFF,0x88,0);
      unitDefs[MT_Amplifier].DescribeLink(3, "Control Signal 2", "ctl2",0xFF,0x88,0);
      unitDefs[MT_Amplifier].DescribeLink(4, "Control Signal 3", "ctl3",0xFF,0x88,0);
      unitDefs[MT_Amplifier].AddParameter(new ParamDef_Expression("scaleExp", "Amplify", "1.0"));
      unitDefs[MT_Amplifier].AddParameter(new ParamDef_Expression("offsetExp", "Offset", "0.0"));
      unitDefs[MT_Amplifier].AddParameter(new ParamDef_Expression("panExp", "Pan (-1 to 1)", "0.0"));
      // unitDefs[MT_Amplifier].SetOutputFormat("AMPS <scaleExp>\nAMPO <offsetExp>");

    unitDefs[MT_Inverter]    = new UnitDef(MT_Inverter,"inv",    "Inverter",  ICON_FILTER, 12);
      unitDefs[MT_Inverter].DescribeLink(0, "Signal to Invert", "sig",0,0,0xFF);

    unitDefs[MT_Expression]  = new UnitDef(MT_Expression,"exp",    "Expression",  ICON_OSC, 13);
      unitDefs[MT_Expression].DescribeLink(0, "Default Signal", "sig",  0x00,0x00,0xff);
      unitDefs[MT_Expression].DescribeLink(1, "Alt Signal #1", "sig1",  0x11,0x11,0xee);
      unitDefs[MT_Expression].DescribeLink(2, "Alt Signal #2", "sig2",  0x22,0x22,0xdd);
      unitDefs[MT_Expression].DescribeLink(3, "Alt Signal #3", "sig3",  0x33,0x33,0xcc);
      unitDefs[MT_Expression].DescribeLink(4, "Alt Signal #4", "sig4",  0x44,0x44,0xbb);
      unitDefs[MT_Expression].DescribeLink(5, "Alt Signal #5", "sig5",  0x55,0x55,0xaa);
      unitDefs[MT_Expression].DescribeLink(6, "Alt Signal #6", "sig6",  0x66,0x66,0x99);
      unitDefs[MT_Expression].DescribeLink(7, "Alt Signal #7", "sig7",  0x77,0x77,0x88);
      unitDefs[MT_Expression].DescribeLink(8, "Alt Signal #8", "sig8",  0x88,0x88,0x77);
      unitDefs[MT_Expression].DescribeLink(9, "Alt Signal #9", "sig9",  0x99,0x99,0x66);
      unitDefs[MT_Expression].DescribeLink(10, "Alt Signal #10", "sig10", 0xaa,0xaa,0x55);
      unitDefs[MT_Expression].DescribeLink(11, "Alt Signal #11", "sig11", 0xbb,0xbb,0x44);
      unitDefs[MT_Expression].DescribeLink(12, "Alt Signal #12", "sig12", 0xcc,0xcc,0x33);
      unitDefs[MT_Expression].DescribeLink(13, "Alt Signal #13", "sig13", 0xdd,0xdd,0x22);
      unitDefs[MT_Expression].DescribeLink(14, "Alt Signal #14", "sig14", 0xee,0xee,0x11);
      unitDefs[MT_Expression].DescribeLink(15, "Alt Signal #15", "sig15", 0xff,0xff,0x00);
      unitDefs[MT_Expression].AddParameter(new ParamDef_Expression("exp", "Expression", ""));
      // unitDefs[MT_Expression].SetOutputFormat("EXP <exp");

    unitDefs[MT_Folder]      = new UnitDef(MT_Folder,"folder", "Folder",  ICON_OSC, 14);
      String[] legalInstSuffixes = {".syd"};
      unitDefs[MT_Folder].DescribeLink(0, "Folder Input", "f0",       0x00,0x00,0xff);
      unitDefs[MT_Folder].DescribeLink(1, "Folder Input #1", "f1",    0x11,0x11,0xee);
      unitDefs[MT_Folder].DescribeLink(2, "Folder Input #2", "f2",    0x22,0x22,0xdd);
      unitDefs[MT_Folder].DescribeLink(3, "Folder Input #3", "f3",    0x33,0x33,0xcc);
      unitDefs[MT_Folder].DescribeLink(4, "Folder Input #4", "f4",    0x44,0x44,0xbb);
      unitDefs[MT_Folder].DescribeLink(5, "Folder Input #5", "f5",    0x55,0x55,0xaa);
      unitDefs[MT_Folder].DescribeLink(6, "Folder Input #6", "f6",    0x66,0x66,0x99);
      unitDefs[MT_Folder].DescribeLink(7, "Folder Input #7", "f7",    0x77,0x77,0x88);
      unitDefs[MT_Folder].DescribeLink(8, "Folder Input #8", "f8",    0x88,0x88,0x77);
      unitDefs[MT_Folder].DescribeLink(9, "Folder Input #9", "f9",    0x99,0x99,0x66);
      unitDefs[MT_Folder].DescribeLink(10, "Folder Input #10", "f10", 0xaa,0xaa,0x55);
      unitDefs[MT_Folder].DescribeLink(11, "Folder Input #11", "f11", 0xbb,0xbb,0x44);
      unitDefs[MT_Folder].DescribeLink(12, "Folder Input #12", "f12", 0xcc,0xcc,0x33);
      unitDefs[MT_Folder].DescribeLink(13, "Folder Input #13", "f13", 0xdd,0xdd,0x22);
      unitDefs[MT_Folder].DescribeLink(14, "Folder Input #14", "f14", 0xee,0xee,0x11);
      unitDefs[MT_Folder].DescribeLink(15, "Folder Input #15", "f15", 0xff,0xff,0x00);
      unitDefs[MT_Folder].AddParameter(new ParamDef_Filename("instFileSpec", "Patch File", "mypatch.syd", false,legalInstSuffixes));
      // unitDefs[MT_Folder].SetOutputFormat("CSCO <instFileSpec> 0 0"); // yes, it's CSCO - historical accident for folder instruments

    unitDefs[MT_RandScore]  = new UnitDef(MT_RandScore,"rscore",  "Random Score",  ICON_SCORE, 15);
      unitDefs[MT_RandScore].DescribeLink(0, "Instrument 1", "i1",0xFF,0x00,0xFF);
      unitDefs[MT_RandScore].DescribeLink(1, "Instrument 2", "i2",0xDD,0x00,0xDD);
      unitDefs[MT_RandScore].DescribeLink(2, "Instrument 3", "i3",0xBB,0x00,0xBB);
      unitDefs[MT_RandScore].DescribeLink(3, "Instrument 4", "i4",0x99,0x00,0x99);
      unitDefs[MT_RandScore].DescribeLink(4, "Instrument 5", "i5",0x77,0x00,0x77);
      unitDefs[MT_RandScore].DescribeLink(5, "Instrument 6", "i6",0x55,0x00,0x55);
      unitDefs[MT_RandScore].DescribeLink(6, "Instrument 7", "i7",0x33,0x00,0x33);
      unitDefs[MT_RandScore].DescribeLink(7, "Instrument 8", "i8",0x11,0x00,0x11);
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("nbrNotesExp", "# Events", "p3 + 1"));
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p1", "p1 (instrument#)", "1"));
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p2", "p2 (start time)", "?*(p3-1)"));
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p3", "p3 (duration)", "0.5+?*0.5"));
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p4", "p4", "0.25 + ?*0.25",10));
        unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p5", "p5", "55*2**(?*4.0)",10),ParamDef.NOBREAK);
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p6", "p6", "0",10));
        unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p7", "p7", "0",10),ParamDef.NOBREAK);
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p8", "p8", "0",10));
        unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p9", "p9", "0",10),ParamDef.NOBREAK);
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p10", "p10", "0",10));
        unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p11", "p11", "0",10),ParamDef.NOBREAK);
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p12", "p12", "0",10));
        unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p13", "p13", "0",10),ParamDef.NOBREAK);
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p14", "p14", "0",10));
        unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p15", "p15", "0",10),ParamDef.NOBREAK);
      unitDefs[MT_RandScore].AddParameter(new ParamDef_Expression("p16", "p16", "0",10));

     // !!! This one should be customized to compute nbr active expressions...
     // unitDefs[MT_RandScore].SetOutputFormat("RSCO <nbrNotesExp>\nRSCON 16\nRSCO1 p1\nRSCO2 p2\nRSCO3 p3\nRSCO4 p4\nRSCO5 p5\nRSCO6 p6\nRSCO7 p7\nRSCO8 p8\nRSCO9 p9\nRSCO10 p10\nRSCO11 p11\nRSCO12 p12\nRSCO13 p13\nRSCO14 p14\nRSCO15 p15\nRSCO16 p16\n");

    unitDefs[MT_CScore]     = new UnitDef(MT_CScore,"cscore", "CSound Score",  ICON_SCORE, 16);
      String[] legalCScoreSuffixes = {".sco"};
      unitDefs[MT_CScore].DescribeLink(0, "Instrument 1", "i1",0xFF,0x00,0xFF);
      unitDefs[MT_CScore].DescribeLink(1, "Instrument 2", "i2",0xDD,0x00,0xDD);
      unitDefs[MT_CScore].DescribeLink(2, "Instrument 3", "i3",0xBB,0x00,0xBB);
      unitDefs[MT_CScore].DescribeLink(3, "Instrument 4", "i4",0x99,0x00,0x99);
      unitDefs[MT_CScore].DescribeLink(4, "Instrument 5", "i5",0x77,0x00,0x77);
      unitDefs[MT_CScore].DescribeLink(5, "Instrument 6", "i6",0x55,0x00,0x55);
      unitDefs[MT_CScore].DescribeLink(6, "Instrument 7", "i7",0x33,0x00,0x33);
      unitDefs[MT_CScore].DescribeLink(7, "Instrument 8", "i8",0x11,0x00,0x11);
      unitDefs[MT_CScore].AddParameter(new ParamDef_Filename("scoreFileSpec", "Score File", "myscore.sco", false,legalCScoreSuffixes));
      // unitDefs[MT_CScore].SetOutputFormat("CSCO <scoreFileSpec> 0 0");

    unitDefs[MT_FInput]     = new UnitDef(MT_FInput,"finput", "Folder Input",  ICON_SCORE, 17);
      unitDefs[MT_FInput].DescribeLink(0, "INVALID", "???",0xFF,0xFF,0xFF);
      unitDefs[MT_FInput].AddParameter(new ParamDef_Integer("fNbr", "F# (0-n)", "0"));
      unitDefs[MT_FInput].AddParameter(new ParamDef_Expression("defExp", "default", ""));
      unitDefs[MT_FInput].AddParameter(new ParamDef_String("desc", "description", "input value"));
      // unitDefs[MT_FInput].SetOutputFormat("FIDEF <defExp>\nFINN <fNbr>\nFIND <desc>");

    unitDefs[MT_PInput]     = new UnitDef(MT_PInput,"pinput", "Score Parameter",  ICON_SCORE, 18);
      unitDefs[MT_PInput].DescribeLink(0, "INVALID", "???",0xFF,0xFF,0xFF);
      unitDefs[MT_PInput].AddParameter(new ParamDef_Integer("pNbr", "P# (0-n)", "0"));
      unitDefs[MT_PInput].AddParameter(new ParamDef_Expression("defExp", "default", ""));
      unitDefs[MT_PInput].AddParameter(new ParamDef_String("desc", "description", "input value"));
      // unitDefs[MT_PInput].SetOutputFormat("PIDEF <defExp>\nPINN <pNbr>\nPIND <desc>");

    unitDefs[MT_FTable]     = new UnitDef(MT_FTable,"ftable", "Function Table",  ICON_SCORE, 19);
      unitDefs[MT_FTable].DescribeLink(0, "Default Signal", "sig",  0x00,0x00,0xff);
      unitDefs[MT_FTable].DescribeLink(1, "Alt Signal #1", "sig1",  0x11,0x11,0xee);
      unitDefs[MT_FTable].DescribeLink(2, "Alt Signal #2", "sig2",  0x22,0x22,0xdd);
      unitDefs[MT_FTable].DescribeLink(3, "Alt Signal #3", "sig3",  0x33,0x33,0xcc);
      unitDefs[MT_FTable].AddParameter(new ParamDef_Integer("tabNbr", "ftab# (0-n)", "0"));
      unitDefs[MT_FTable].AddParameter(new ParamDef_Expression("tabExp", "function", "sin(t*2*pi)"));
      unitDefs[MT_FTable].AddParameter(new ParamDef_Integer("tabSize", "length", "100"));
      // unitDefs[MT_FTable].SetOutputFormat("FTAB <tabNbr> <tabSize>\nFTABE <tabExp>");

    unitDefs[MT_SampleFile] = new UnitDef(MT_SampleFile,"sample", "Sample File",  ICON_OSC, 20);
      unitDefs[MT_SampleFile].DescribeLink(0, "INVALID", "???",0xFF,0xFF,0xFF);
      unitDefs[MT_SampleFile].AddParameter(new ParamDef_Filename("sampleFileSpec", "Sample file", "mysample.wav", false,legalSoundFileSuffixes));
      unitDefs[MT_SampleFile].AddParameter(new ParamDef_Checkbox("flags", "interpolate", "0"));
      unitDefs[MT_SampleFile].AddParameter(new ParamDef_Expression("timeScaleExp", "time scale", "1.0",10));
      // unitDefs[MT_SampleFile].SetOutputFormat("FSAMP <sampleFileSpec> 0 0 <flags>\nFSAMPT <timeScaleExp>");

    unitDefs[MT_Pluck]      = new UnitDef(MT_Pluck,"pluck", "Karplus/Strong Plucked String Sound",  ICON_OSC, 21);
      unitDefs[MT_Pluck].DescribeLink(0, "INVALID", "???",0xFF,0xFF,0xFF);
      unitDefs[MT_Pluck].AddParameter(new ParamDef_Expression("freqExp", "frequency", "440",10));
      unitDefs[MT_Pluck].AddParameter(new ParamDef_Expression("durExp", "duration", "2",10));
      unitDefs[MT_Pluck].AddParameter(new ParamDef_Expression("ampExp", "amplitude", "1",10));
      unitDefs[MT_Pluck].AddParameter(new ParamDef_Expression("decayExp", "decay (dB)", "10",10));
      // unused
      // unitDefs[MT_Pluck].AddParameter(new ParamDef_Integer("variant", "variant", "0"));
      // unitDefs[MT_Pluck].SetOutputFormat("PLKI 0\nPLKF <freqExp>\nPLKD <durExp>\nPLKA <ampExp>\nPLKd <decayExp>");

    unitDefs[MT_Maraca]     = new UnitDef(MT_Maraca,"maraca", "Perry Cook's Maraca Simulation",  ICON_OSC, 22);
      unitDefs[MT_Maraca].DescribeLink(0, "INVALID", "???",0xFF,0xFF,0xFF);
      unitDefs[MT_Maraca].AddParameter(new ParamDef_Expression("resfreqExp", "resonance freq", "3200",10));
      unitDefs[MT_Maraca].AddParameter(new ParamDef_Expression("respoleExp", "resonance pole", "0.96",10),ParamDef.NOBREAK);
      unitDefs[MT_Maraca].AddParameter(new ParamDef_Expression("probExp", "probability", "1/16",10));
      unitDefs[MT_Maraca].AddParameter(new ParamDef_Expression("sysdecayExp", "system decay", "0.999",10));
      unitDefs[MT_Maraca].AddParameter(new ParamDef_Expression("snddecayExp", "sound decay", "0.95",10),ParamDef.NOBREAK);
      // unitDefs[MT_Maraca].SetOutputFormat("MARI\nMARRF <resfreqExp>\nMARRP <respoleExp>\nMARP <probExp>\nMARSysD <sysdecayExp>\nMARSndD <snddecayExp>");

    unitDefs[MT_HammerBank] = new UnitDef(MT_HammerBank,"hammerbank", "Hammer Bank",  ICON_OSC, 23);
      unitDefs[MT_HammerBank].DescribeLink(0, "Instrument 1", "i1",0xFF,0x00,0xFF);
      unitDefs[MT_HammerBank].DescribeLink(1, "Instrument 2", "i2",0xDD,0x00,0xDD);
      unitDefs[MT_HammerBank].DescribeLink(2, "Instrument 3", "i3",0xBB,0x00,0xBB);
      unitDefs[MT_HammerBank].DescribeLink(3, "Instrument 4", "i4",0x99,0x00,0x99);
      unitDefs[MT_HammerBank].DescribeLink(4, "Instrument 5", "i5",0x77,0x00,0x77);
      unitDefs[MT_HammerBank].DescribeLink(5, "Instrument 6", "i6",0x55,0x00,0x55);
      unitDefs[MT_HammerBank].DescribeLink(6, "Instrument 7", "i7",0x33,0x00,0x33);
      unitDefs[MT_HammerBank].DescribeLink(7, "Instrument 8", "i8",0x11,0x00,0x11);
      unitDefs[MT_HammerBank].DescribeLink(8, "Control Signal", "ctl",  0xFF,0x11,0xee);
      unitDefs[MT_HammerBank].DescribeLink(9, "Alt Control Sig #1", "ctl1", 0xFF,0x22,0xdd);
      unitDefs[MT_HammerBank].DescribeLink(10, "Alt Control Sig #2", "ctl2",  0xFF,0x33,0xcc);
      unitDefs[MT_HammerBank].DescribeLink(11, "Alt Control Sig #3", "ctl3",  0xFF,0x44,0xbb);
      unitDefs[MT_HammerBank].AddParameter(new ParamDef_Expression("bNbrExp", "bank #", "0",10));
      unitDefs[MT_HammerBank].AddParameter(new ParamDef_Expression("kFreqExp", "k->frequency", "cpsmidi(k)",10));
      unitDefs[MT_HammerBank].AddParameter(new ParamDef_Expression("kAmpExp", "k->amplitude", "1.0",10),ParamDef.NOBREAK);
      unitDefs[MT_HammerBank].AddParameter(new ParamDef_Expression("kAttackExp", "k->attack", "0.05",10));
      unitDefs[MT_HammerBank].AddParameter(new ParamDef_Expression("kDecayExp", "k->decay", "0.1",10),ParamDef.NOBREAK);
      unitDefs[MT_HammerBank].AddParameter(new ParamDef_Expression("sustainExp", "k->sustain", "1.0",10));
      unitDefs[MT_HammerBank].AddParameter(new ParamDef_Expression("waveformExp", "waveform", "sin(t*2*pi)",32),ParamDef.NOBREAK);
      // unitDefs[MT_HammerBank].SetOutputFormat("HAMB\nHAMBN <bNbrExp>\nHAMBF <kFreqExp>\nHAMBA <kAmpExp>\nHAMBAt <kAttackExp>\nHAMBD <kDecayExp>\nHAMBS <sustainExp>\nHAMBW <waveformExp>");

    unitDefs[MT_HammerActuator] = new UnitDef(MT_HammerActuator,"hammeract",  "Hammer Actuator",  ICON_SCORE, 24);
      unitDefs[MT_HammerActuator].DescribeLink(0, "Default Signal",     "sig",  0x00,0x00,0xff);
      unitDefs[MT_HammerActuator].DescribeLink(1, "Alt Signal #1",      "sig1", 0x11,0x11,0xee);
      unitDefs[MT_HammerActuator].DescribeLink(2, "Alt Signal #2",      "sig2", 0x22,0x22,0xdd);
      unitDefs[MT_HammerActuator].DescribeLink(3, "Alt Signal #3",      "sig3", 0x33,0x33,0xcc);
      unitDefs[MT_HammerActuator].DescribeLink(4, "Control Signal",     "ctl",  0xFF,0x11,0xee);
      unitDefs[MT_HammerActuator].DescribeLink(5, "Alt Control Sig #1", "ctl1", 0xFF,0x22,0xdd);
      unitDefs[MT_HammerActuator].DescribeLink(6, "Alt Control Sig #2", "ctl2", 0xFF,0x33,0xcc);
      unitDefs[MT_HammerActuator].DescribeLink(7, "Alt Control Sig #3", "ctl3", 0xFF,0x44,0xbb);
      unitDefs[MT_HammerActuator].AddParameter(new ParamDef_Expression("bNbrExp", "bank #", "0",10));
      unitDefs[MT_HammerActuator].AddParameter(new ParamDef_Expression("keyNbrExp", "key #", "p5",10),ParamDef.NOBREAK);
      unitDefs[MT_HammerActuator].AddParameter(new ParamDef_Expression("triggerExp", "trigger", "1",10));
      unitDefs[MT_HammerActuator].AddParameter(new ParamDef_Expression("velocityExp", "velocity", "p4/128",10),ParamDef.NOBREAK);
      unitDefs[MT_HammerActuator].AddParameter(new ParamDef_Expression("undampenExp", "undampen", "1",10));
      // unitDefs[MT_HammerActuator].SetOutputFormat("HAMA\nHAMAN <bNbrExp>\nHAMAK <keyNbrExp>\nHAMAT <triggerExp>\nHAMAV <velocityExp>\nHAMAU <undampenExp>");

    unitDefs[MT_GAssign]    = new UnitDef(MT_GAssign,"gassign", "Global Variable Assignment",  ICON_SCORE, 25);
      unitDefs[MT_GAssign].DescribeLink(0, "Default Signal",     "sig",  0x00,0x00,0xff);
      unitDefs[MT_GAssign].DescribeLink(1, "Alt Signal #1",      "sig1", 0x11,0x11,0xee);
      unitDefs[MT_GAssign].DescribeLink(2, "Alt Signal #2",      "sig2", 0x22,0x22,0xdd);
      unitDefs[MT_GAssign].DescribeLink(3, "Alt Signal #3",      "sig3", 0x33,0x33,0xcc);
      unitDefs[MT_GAssign].AddParameter(new ParamDef_Expression("gNbrExp", "G# (0-n)", "0"));
      unitDefs[MT_GAssign].AddParameter(new ParamDef_Expression("valExp", "value", "sig"));
      unitDefs[MT_GAssign].AddParameter(new ParamDef_String("desc", "description", "var"));
      // unitDefs[MT_GAssign].SetOutputFormat("GASSN <gNbrExp>\nGASSV <valExp>\nGASSD <desc>");

    unitDefs[MT_SkiniScore] = new UnitDef(MT_SkiniScore,"skiniscore", "Skini (STK) Score",  ICON_SCORE, 16);
      unitDefs[MT_SkiniScore].DescribeLink(0, "Instrument 1", "i1",0xFF,0x00,0xFF);
      unitDefs[MT_SkiniScore].DescribeLink(1, "Instrument 2", "i2",0xDD,0x00,0xDD);
      unitDefs[MT_SkiniScore].DescribeLink(2, "Instrument 3", "i3",0xBB,0x00,0xBB);
      unitDefs[MT_SkiniScore].DescribeLink(3, "Instrument 4", "i4",0x99,0x00,0x99);
      unitDefs[MT_SkiniScore].DescribeLink(4, "Instrument 5", "i5",0x77,0x00,0x77);
      unitDefs[MT_SkiniScore].DescribeLink(5, "Instrument 6", "i6",0x55,0x00,0x55);
      unitDefs[MT_SkiniScore].DescribeLink(6, "Instrument 7", "i7",0x33,0x00,0x33);
      unitDefs[MT_SkiniScore].DescribeLink(7, "Instrument 8", "i8",0x11,0x00,0x11);
      unitDefs[MT_SkiniScore].AddParameter(new ParamDef_Filename("scoreFileSpec", "Skini File", "myscore.skini", false));
      // unitDefs[MT_CScore].SetOutputFormat("CSCO <scoreFileSpec> 0 0");
  }

  public SydPanel(JSydApp itsApp )
  {
    this.itsApp = itsApp;
    setBackground(Color.white);    // white background
    setPreferredSize( new Dimension(PWIDTH, PHEIGHT));

    InitUnitDefs();

    addKeyListener(this);
    setFocusable(true);
    requestFocus();    // the JPanel now has focus, so receives key events
    // requestFocusInWindow();
    font = new Font("SansSerif", Font.BOLD, 24);

    // !!! Initialize Tools Icons Here...

    setToolTipText("Syd"); // turn on tool tips




    addMouseListener(this);
    addMouseMotionListener(this);
    addComponentListener(this);
    repaint();

  }  // end of SydPanel( )

  public void componentHidden(ComponentEvent ce)
  {
  }
  public void componentShown(ComponentEvent ce)
  {
  }
  public void componentMoved(ComponentEvent ce)
  {
  }
  public void componentResized(ComponentEvent ce)
  {
    System.out.println("Width = " + this.getWidth() + " Height = " + this.getHeight());
    dbImage = null;
    repaint();
  }

  public void addNotify( )
  /* Wait for the JPanel to be added to the
     JFrame/JApplet before starting. */
  {
    super.addNotify( );   // creates the peer
  }

  public void cleanUp()
  {
    running = false;
    // stop any threads
    if (itsPatch.windowState == WS_Synthesize)
      itsPatch.AbortSynthesis();
  }


  private void gameUpdate( )
  {
    if (gameOver)
      return;
  }

  static int blinkCtr = 0;

  public void RenderUnit(Graphics dbg, int unitNbr, boolean selected, Rectangle bounds,int dx,int dy)
  {
      UnitDef ud = unitDefs[unitNbr];
      if (ud == null)
        return;
      // dbg.drawImage(bgImages[ud.iconID], x, y, w, h, null);
      if (bounds.width == 64 && bounds.y < 64 && ((blinkCtr++) & 1) == 0)
        return;
      dbg.drawImage(bgImage,    bounds.x+dx, bounds.y+dy, bounds.x+bounds.width+dx, bounds.y+bounds.height+dy, 64*ud.iconID,64*(selected?1:0),  64*ud.iconID+64,64*(selected?1:0)+64,this);
      dbg.drawImage(stampImage, bounds.x+dx, bounds.y+dy, bounds.x+bounds.width+dx, bounds.y+bounds.height+dy, 64*ud.labelIdx,0,64*ud.labelIdx+64,64,this);
  }


  int curDetail = 2;
  static final int maxDisplace = 32;

  private void lightning(Graphics2D g2, int x1, int y1, int x2, int y2, double displace)
  {
     if (displace < curDetail) {
      //  var v = (x1 - gen1_mc._x)*255/(gen2_mc._x - gen1_mc._x);
      Line2D.Double ln = new Line2D.Double();
      ln.setLine(new Point(x1,y1), new Point(x2,y2));
      g2.draw(ln);
    }
    else {
      int mid_x = (x2+x1)/2;
      int mid_y = (y2+y1)/2;
      mid_x += (Math.random()-.5)*displace;
      mid_y += (Math.random()-.5)*displace;
      lightning(g2,x1,y1,mid_x,mid_y,displace/2);
      lightning(g2,x2,y2,mid_x,mid_y,displace/2);
    }
  }
  private void lightning(GeneralPath gp, int x1, int y1, int x2, int y2, double displace)
  {
     if (displace < curDetail) {
      //  var v = (x1 - gen1_mc._x)*255/(gen2_mc._x - gen1_mc._x);
      Line2D.Double ln = new Line2D.Double();
      gp.moveTo(x1,y1);
      gp.lineTo(x2,y2);
      // ln.setLine(new Point(x1,y1), new Point(x2,y2));
      // g2.draw(ln);
    }
    else {
      int mid_x = (x2+x1)/2;
      int mid_y = (y2+y1)/2;
      mid_x += (Math.random()-.5)*displace;
      mid_y += (Math.random()-.5)*displace;
      lightning(gp,x1,y1,mid_x,mid_y,displace/2);
      lightning(gp,x2,y2,mid_x,mid_y,displace/2);
    }
  }

  static AffineTransform shadowTx = new AffineTransform();
  static AffineTransform unShadowTx = new AffineTransform();

  {
    shadowTx.setToTranslation(1,1);
    unShadowTx.setToTranslation(-1,-1);
  }

  private void patchRender()
  {
    if (dbImage == null){
      // System.out.println("Making dbImage cWidth = " + this.getWidth());
      dbImage = createImage(this.getWidth(), getHeight());
      if (dbImage == null) {
        System.out.println("dbImage is null");
        return;
      }
      else
        dbg = dbImage.getGraphics();
    }
    Graphics2D g2 = (Graphics2D) dbg;

    // clear the background
    dbg.setColor(new Color(0xEE,0xEE,0xEE)); // was 88,88,88
    dbg.fillRect (0, 0, this.getWidth(), this.getHeight());
    dbg.setColor(Color.blue);
    dbg.setFont(font);

    // !!! Draw Toolbar here...


    // !!! Draw icons at top here...
    for (int i = 0; i < unitDefs.length; ++i) {
      int x = (i%14)*48 + 10 + (i >= 14? 24 : 0);
      int y = (i/14)*32 + 10;
      RenderUnit(dbg,pOrder[i],false,new Rectangle(x,y,UnitDefWidth,UnitDefWidth),0,0);
    }


    // Draw connections
    g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                        RenderingHints.VALUE_ANTIALIAS_ON);

    // Line2D.Double ln = new Line2D.Double();
    g2.setStroke(new BasicStroke(3));

    Enumeration e = unitNodes.mods.elements();
    while (e.hasMoreElements())
    {
      SSModule dstMod = (SSModule) e.nextElement();
      Enumeration ie = dstMod.inputs.elements();
      while (ie.hasMoreElements())
      {
        ModInput  inp = (ModInput) ie.nextElement();
        SSModule srcMod = inp.link;
        Point sp = srcMod.GetOutputDock(dstMod);
        Point dp = dstMod.GetInputDock(srcMod);

        Line2D.Double shadowLine = new Line2D.Double(sp.x+1+dOffset.x,sp.y+1+dOffset.y,dp.x+1+dOffset.x,dp.y+1+dOffset.y);
        g2.setColor(Color.BLACK);

        g2.draw(shadowLine);


        Line2D.Double mainLine = new Line2D.Double(sp.x+dOffset.x,sp.y+dOffset.y,dp.x+dOffset.x,dp.y+dOffset.y);
        LinkDesc ld = unitDefs[dstMod.moduleType].GetLinkDesc(inp.inputType);
        g2.setColor(ld.clr);
        g2.draw(mainLine);
      }
    }

    if (linkDragging)
    {
      // Do feedback...
      // ln.setLine(200,200,200+Math.cos(dragVecAngle)*dragVecLength,
      //                    200+Math.sin(dragVecAngle)*dragVecLength);
      // g2.draw(ln);

      // Show Dragging here...
      // ln.setLine(linkAnchor, linkPoint);
      // g2.setColor(new Color(255,0,0));
      // g2.draw(ln);
      if (Math.random() < 1.4) {
        double displace = 10+Math.pow(Math.random(),2)*maxDisplace;
        curDetail = 1 + ( (int) (Math.random()*2) );
        GeneralPath gp = new GeneralPath();
        lightning(gp,linkAnchor.x,linkAnchor.y,linkPoint.x,linkPoint.y, displace);
        g2.setStroke(new BasicStroke( 2+((int) (Math.random()*2))) );
        gp.transform(shadowTx);
        g2.setColor(Color.GRAY);
        g2.draw(gp);
        gp.transform(unShadowTx);
        g2.setColor(new Color(255,255,0+(int) (Math.random()*256)));
        g2.draw(gp);
      }
      repaint();
    }
    else if (rectDragging)
    {
      // System.out.println("Drawing rect " + rectAnchor.x + " -> " + rectPoint.x);
      g2.setColor(new Color(0,0,255));
      g2.setStroke(new BasicStroke(1));
      g2.drawRect(rectAnchor.x, rectAnchor.y, rectPoint.x - rectAnchor.x, rectPoint.y - rectAnchor.y);

    }
    else if (dragNode != null && dragNode.zapLink != null)
    {
         Point zapAnchor = dragNode.GetOutputDock(dragNode.zapLink);
         Point zapPoint = dragNode.zapLink.GetInputDock(dragNode);
          double displace = 10+Math.pow(Math.random(),2)*maxDisplace;
          curDetail = 1 + ( (int) (Math.random()*2) );
          GeneralPath gp = new GeneralPath();
          lightning(gp,zapAnchor.x,zapAnchor.y,zapPoint.x,zapPoint.y, displace);
          g2.setStroke(new BasicStroke( 2+((int) (Math.random()*2))) );
          gp.transform(shadowTx);
          g2.setColor(Color.GRAY);
          g2.draw(gp);
          gp.transform(unShadowTx);
          g2.setColor(new Color(255,255,0+(int) (Math.random()*256)));
          g2.draw(gp);
          repaint();
    }

    g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                        RenderingHints.VALUE_ANTIALIAS_OFF);
    // Draw modules
    e = unitNodes.mods.elements();
    while (e.hasMoreElements())
    {
      SSModule srcMod = (SSModule) e.nextElement();
      RenderUnit(dbg, srcMod.moduleType, srcMod.isSelected, srcMod.bounds,dOffset.x,dOffset.y);
    }


  }


  void LoadAssets()
  {
    // Load image assets
    if (bgImage == null) {
      Toolkit toolkit = Toolkit.getDefaultToolkit();
      tracker = new MediaTracker(this);

      bgImage = toolkit.getImage(this.getClass().getResource("assets/iconbacks.png")); // "assets/iconbacks.png");
      tracker.addImage(stampImage, 0);
      stampImage = toolkit.getImage(this.getClass().getResource("assets/unitlabels.png")); // "assets/unitlabels.png");
      tracker.addImage(stampImage, 1);
    }
  }

  int ClickedOnTool(MouseEvent e)
  {
    return -1;  // !!!
  }

  boolean MouseOverTool(MouseEvent e)
  {
    return ClickedOnTool(e) != -1;
  }

  boolean ProcessToolClick(MouseEvent e)
  {
    int toolID = ClickedOnTool(e);
    if (toolID == -1)
      return false;
    // Instantiate tool
    return true;
  }
  boolean ProcessToolUnclick(MouseEvent e)
  {
    int toolID = ClickedOnTool(e);
    if (toolID == -1)
      return false;
    // Instantiate tool
    return true;
  }

  int ClickedOnUnitDef(MouseEvent e)
  {
    // System.out.println("Checking unitdef");
    int mx = e.getX();
    int my = e.getY();
    for (int i = 0; i < unitDefs.length; ++i) {
      int x = (i%14)*48 + 10 + (i >= 14? 24 : 0);
      int y = (i/14)*32 + 10;
      if (mx >= x && my >= y &&
          mx < x+32 && my < y+32)
      {
        return pOrder[i];
      }
    }
    return -1;
  }

  boolean MouseOverUnitDef(MouseEvent e)
  {
    return ClickedOnUnitDef(e) != -1;
  }

  boolean ProcessUnitDefClick(MouseEvent e)
  {
    int unitID = ClickedOnUnitDef(e);
    int modifiers = e.getModifiersEx();
    if (unitID == -1)
      return false;
    // Instantiate unit and start dragging it
    SSModule node = unitNodes.AddModule(unitID);
    node.InitUnitNode(unitDefs[unitID], unitID, (e.getX()-NodeWidth/2)-dOffset.x, (e.getY()-NodeWidth/2)-dOffset.y);
    node.isSelected = true;
    isDirty = true;

    /*
    Class nc = node.getClass();
    System.out.println("Field list for " + nc.getName());
    Field[] fs = nc.getFields();
    for (int i = 0; i < fs.length; ++i)
    {
      System.out.println("Field " + i + " name = " + fs[i].getName());
      if (fs[i].getName().equals("fExp")) {
        try {
          ExpRec eRec = (ExpRec) fs[i].get(node);
           System.out.println("fExp = " + eRec.exp);
        }
        catch (IllegalAccessException iae) {
           System.out.println(iae.toString());

        }
      }
    }
    */

    // unitNodes.mods.add(node);
    // System.out.println(modifiers);
    DeselectNodes(); // !! deal with shift correctly...
    dragNode = node;
    dragOX = e.getX();
    dragOY = e.getY();
    dragNode.isSelected = true;
    didDragging = true;
    ActivateNodeEditor(dragNode);
    return true;
  }

  boolean ProcessUnitDefUnclick(MouseEvent e)
  {
    int unitID = ClickedOnUnitDef(e);
    int modifiers = e.getModifiersEx();
    if (unitID == -1)
      return false;
    return true;
  }


  SSModule ClickedOnUnitNode(MouseEvent evt)
  {
    Point mp = evt.getPoint();

    Enumeration e = unitNodes.mods.elements();
    while (e.hasMoreElements())
    {
      SSModule node = (SSModule) e.nextElement();
      if (node.bounds.contains(mp))
      {
         return node;
      }
    }
    return null;
  }

  boolean MouseOverUnitNode(MouseEvent e)
  {
    return ClickedOnUnitNode(e) != null;
  }

  boolean PtInLine(Point mp, Point sp, Point dp)
  {
      Line2D.Double ln = new Line2D.Double(sp,dp);
      return ln.intersects(mp.getX()-3,mp.getY()-3,6,6);
  }

  ModInput  ClickedOnUnitLink(MouseEvent evt)
  {
    Point mp = evt.getPoint();

    Enumeration e = unitNodes.mods.elements();
    while (e.hasMoreElements())
    {
      SSModule dstMod = (SSModule) e.nextElement();
      Enumeration ie = dstMod.inputs.elements();
      while (ie.hasMoreElements())
      {
        ModInput  inp = (ModInput) ie.nextElement();
        SSModule srcMod = inp.link;
        Point sp = srcMod.GetOutputDock(dstMod);
        Point dp = dstMod.GetInputDock(srcMod);
        if (PtInLine(mp,sp,dp))
        {
          return inp; // unitDefs[dstMod.unitType].GetLinkDesc(inp.inputType);
        }
      }
    }
    return null;
  }

  boolean MouseOverUnitLink(MouseEvent e)
  {
    return ClickedOnUnitLink(e) != null;
  }

  void DeselectNodes()
  {
    Enumeration e = unitNodes.mods.elements();
    while (e.hasMoreElements())
    {
      SSModule node = (SSModule) e.nextElement();
      node.isSelected = false;
    }
  }

  void startLinkDrag(SSModule node, Point p, int inpLinkType)
  {
      linkDragging = true;
      linkPoint = p;
      linkAnchor = node.GetOutputDock(linkPoint);
      linkSource = node;
      linkTarget = null;
      linkType = inpLinkType;
      DeselectNodes();
      ActivateNodeEditor(null);
  }

  boolean ProcessUnitNodeClick(MouseEvent e)
  {
    // deal with shift/cntrl etc.
    SSModule node = ClickedOnUnitNode(e);
    int modifiers = e.getModifiersEx();
    if (node == null)
      return false;
    Point mp = e.getPoint();

    if (node.GetDockRect().contains(mp))
    {
      // Begin patch drag
      startLinkDrag(node,  new Point(e.getX(), e.getY()), -1);
    }
    else {
      // Begin module drag
      dragNode = node;
      dragOX = e.getX();
      dragOY = e.getY();

      dragNode.zapLink = null;
      dragNode.nbrHardLinks = CountOutgoingLinks(dragNode);
      // System.out.println(dragNode.nbrHardLinks + " outgoing links");
    }
    return true;
  }

  boolean ProcessUnitNodeUnclick(MouseEvent e)
  {
    // deal with shift/cntrl etc.
    // System.out.println("Unclick");
    SSModule node = ClickedOnUnitNode(e);
    int modifiers = e.getModifiersEx();
    if (node == null)
      return false;
    // System.out.println("node = " + node + " dragNode = " + dragNode);
    if (node == dragNode) {
      if ((modifiers & (MouseEvent.SHIFT_DOWN_MASK | MouseEvent.CTRL_DOWN_MASK)) != 0)
        dragNode.isSelected = !dragNode.isSelected;
      else {
        DeselectNodes(); // !! deal with shift correctly...
        dragNode.isSelected = true;
        ActivateNodeEditor(dragNode);
      }
    }
    return true;
  }

  // Popup Menu Stuff
  public void popupMenuCanceled(PopupMenuEvent e)
  {
      linkDragging = false;
      isDragging = false;
      linkMenuInput = null;
      linkMenu = null;
  }
  public void popupMenuWillBecomeInvisible(PopupMenuEvent e)
  {
  }
  public void popupMenuWillBecomeVisible(PopupMenuEvent e)
  {
  }

  public void actionPerformed( ActionEvent event )
  {
      // Add action handling code here
      // System.out.println( "action: " + event.getActionCommand() + ",param=" + event.paramString() );
      if (event.getActionCommand().equals("Delete"))
      {
        linkMenuInput.owner.inputs.remove(linkMenuInput);
        isDirty = true;
        repaint();
      }
      else {  // it's a parameter name...
        // extract out the string
        String p =  event.getActionCommand();
        String varName = p.substring(p.indexOf("[")+1, p.indexOf("]"));
        // System.out.println("Parameter: " + varName);
        // look it up in unitdef for owner of linkMenuInput
        UnitDef ud = unitDefs[linkMenuInput.owner.moduleType];
        for (int i = 0; i < ud.nbrSupportedLinks; ++i) {
          if (ud.linkDesc[i].varName.equals(varName))
          {
            // set linkMenuInput to correct input type
            linkMenuInput.inputType = i;
            linkMenuInput.owner.lastLinkType = i;
            break;
          }
        }
        repaint();
      }
      linkDragging = false;
      isDragging = false;
      linkMenuInput = null;
      linkMenu = null;
  }

  boolean HandlePopUp(MouseEvent e, ModInput inp)
  {
    int modifiers = e.getModifiersEx();
    if ((modifiers & (MouseEvent.BUTTON2_DOWN_MASK|MouseEvent.BUTTON3_DOWN_MASK|MouseEvent.CTRL_DOWN_MASK)) != 0 ||
        e.isPopupTrigger())
    {
       // patch reassignment menu...
       linkMenuInput = inp;
       linkMenu = new JPopupMenu();
       JMenuItem delItem = new JMenuItem("Delete");
       linkMenu.add(delItem);
       delItem.addActionListener(this);
       linkMenu.add("-");
       // For each supported link for the link type
       UnitDef udef = unitDefs[inp.owner.moduleType];
       for (int i = 0; i < udef.nbrSupportedLinks; ++i) {
         LinkDesc ld = udef.linkDesc[i];
         JMenuItem inpItem = new JMenuItem(ld.desc + " [" + ld.varName + "]");
         linkMenu.add(inpItem);
         inpItem.addActionListener(this);
       }
       linkMenu.setInvoker(this);
       linkMenu.show(this, e.getX(), e.getY());
       linkMenu.addPopupMenuListener(this);
       return true;
    }
    return false;
  }

  boolean ProcessUnitLinkClick(MouseEvent e)
  {
    // deal with shift/cntrl etc.
    ModInput inp = ClickedOnUnitLink(e);
    if (inp == null)
      return false;
    int modifiers = e.getModifiersEx();
    Point mp = e.getPoint();

    if (HandlePopUp(e,inp))
      return true;
    else {
      // repatch
      inp.owner.inputs.remove(inp);
      startLinkDrag(inp.link,  new Point(e.getX(), e.getY()), inp.inputType);
      isDirty = true;
    }
    return true;
  }

  boolean ProcessUnitLinkUnclick(MouseEvent e)
  {
    return false;
  }

  boolean ProcessDesktopClick(MouseEvent e)
  {
    ActivateNodeEditor(null);
    rectAnchor = e.getPoint();
    rectPoint = rectAnchor;
    rectDragging = true;
    return false;
  }

  boolean ProcessDesktopUnclick(MouseEvent e)
  {
    DeselectNodes();
    rectDragging = false;
    return true;
  }

  public String getToolTipText(MouseEvent e)
  {
    if (MouseOverTool(e))
      return "Tool";
    else if (MouseOverUnitDef(e))
    {
      int unitID = ClickedOnUnitDef(e);
      return unitDefs[unitID].desc;
    }
    else if (MouseOverUnitNode(e))
    {
      SSModule node = ClickedOnUnitNode(e);
      return unitDefs[node.moduleType].desc;
    }
    else if (MouseOverUnitLink(e))
    {
      ModInput inp = ClickedOnUnitLink(e);
      SSModule dstMod = inp.owner;
      LinkDesc ld = unitDefs[dstMod.moduleType].GetLinkDesc(inp.inputType);
      return ld.desc + " [" + ld.varName + "]";
    }
    else
      return null;
  }

  public void MySetCursor(int i)
  {
    itsApp.MySetCursor(this, i);
  }

  public void SetProperCursor(MouseEvent e)
  {
    int modifiers = e.getModifiers();
    if (isDragging)
      return;
    else if (itsApp.currentTool == JSydApp.TB_GRABSCROLL ||
              (modifiers & MouseEvent.ALT_MASK) != 0)
      MySetCursor(JSydApp.CURS_handopen);
    else if (MouseOverTool(e))
      MySetCursor(JSydApp.CURS_handfinger);
    else if (MouseOverUnitDef(e))
      MySetCursor(JSydApp.CURS_handfinger);
    else if (MouseOverUnitNode(e))
    {
      SSModule node = ClickedOnUnitNode(e);
      if (node.GetDockRect().contains(e.getPoint()))
         MySetCursor(JSydApp.CURS_patchcord);
       else
        MySetCursor(JSydApp.CURS_handopen);
    }
    else if (MouseOverUnitLink(e))
       MySetCursor(JSydApp.CURS_patchcord);
    else
      this.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
  }

  // MouseMotionListener
  public void mouseMoved(MouseEvent e)
  {
    SetProperCursor(e);
    computeDragInfo(e);
  }

  public void mouseDragged(MouseEvent evt)
  {
    if (grabDragging) {
      dOffset.x -= lastPoint.x - evt.getX();
      dOffset.y -= lastPoint.y - evt.getY();
      lastPoint = evt.getPoint();
      repaint();
    }
    else {
      computeDragInfo(evt);
      int modifiers = evt.getModifiersEx();
      if (linkDragging) // Patching from one module to another
      {
        setCursor(Cursor.getPredefinedCursor(Cursor.CROSSHAIR_CURSOR));
        linkPoint = evt.getPoint();
        if (linkTarget != null && !linkTarget.bounds.contains(linkPoint))
        {
          linkTarget.isSelected = false;
          linkTarget = null;
        }
        linkTarget = ClickedOnUnitNode(evt);
        if (linkTarget == linkSource)
          linkTarget = null;
        if (linkTarget != null) {
          linkTarget.isSelected = true;
          ActivateNodeEditor(linkTarget);
        }
        linkAnchor = linkSource.GetOutputDock(linkPoint);
        repaint();
      }
      else if (rectDragging) {
        rectPoint = evt.getPoint();
        DeselectNodes();
        Enumeration e = unitNodes.mods.elements();
        while (e.hasMoreElements())
        {
          SSModule node = (SSModule) e.nextElement();
          if (node.bounds.intersects(new Rectangle(rectAnchor.x, rectAnchor.y, rectPoint.x - rectAnchor.x, rectPoint.y - rectAnchor.y)))
            node.isSelected = true;
        }
        didDragging = true;
        // !!! handle selection of modules within rectangle rectAnchor -> rectPoint
      }
      else if (dragNode != null) {
        int dx = evt.getX() - dragOX;
        int dy = evt.getY() - dragOY;
        if (dx != 0 || dy != 0) {
          if (!didDragging) {
             MySetCursor(JSydApp.CURS_handclosed);

            didDragging = true;
            // if drag node was not selected, select it
            if (dragNode.isSelected == false) {
              if ((modifiers & (MouseEvent.SHIFT_DOWN_MASK | MouseEvent.CTRL_DOWN_MASK)) == 0)
              {
                DeselectNodes(); // !! deal with shift correctly...
                ActivateNodeEditor(dragNode);
              }
              dragNode.isSelected = true;
            }
          }
        }
        // For each selected node, move by dx,dy
        int nbrSelected =0;
        Enumeration e = unitNodes.mods.elements();
        while (e.hasMoreElements())
        {
          SSModule node = (SSModule) e.nextElement();
          if (node.isSelected) {
            node.bounds.x += dx;
            node.bounds.y += dy;
            ++nbrSelected;
            isDirty = true;
          }
        }
        dragOX = evt.getX();
        dragOY = evt.getY();
        if (nbrSelected == 1)
          ZapLinkage();
        repaint();
      }
    }
  }


  public void mousePressed(MouseEvent e)
  {
    int modifiers = e.getModifiers();

    if (itsApp.currentTool == JSydApp.TB_GRABSCROLL ||
        (modifiers & MouseEvent.ALT_MASK) != 0)
    {
      grabDragging = true;
      lastPoint = e.getPoint();
    }
    else {
      // !!!
      // System.out.println("Checking click");
      boolean processedClick =
            ProcessToolClick(e) ||
            ProcessUnitDefClick(e) ||
            ProcessUnitNodeClick(e) ||
            ProcessUnitLinkClick(e) ||
            ProcessDesktopClick(e);
      didDragging = false;
      if (processedClick)
       isDragging = true;
    }
    repaint();
    requestFocusInWindow();
  }

  public void mouseReleased(MouseEvent e)
  {
    if (grabDragging) {
      Enumeration en = unitNodes.mods.elements();
      while (en.hasMoreElements())
      {
        SSModule node = (SSModule) en.nextElement();
        node.bounds.x += dOffset.x;
        node.bounds.y += dOffset.y;
      }
      isDirty = true;
      grabDragging = false;
      dOffset.x = 0;
      dOffset.y = 0;
      return;
    }

    if (!didDragging)
    {
      // process selection clicks
      boolean processed = ProcessToolUnclick(e) ||
                          ProcessUnitDefUnclick(e) ||
                          ProcessUnitNodeUnclick(e) ||
                          ProcessUnitLinkUnclick(e) ||
                          ProcessDesktopUnclick(e);
    }
    else {
      // System.out.println(e.getX() + "/" + e.getY());
    }
    setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));

    if (dragNode != null && dragNode.bounds.y < 64) {
      System.out.println("Deleting selection");
      DeleteSelection();
      dragNode = null;
    }
    else if (dragNode != null && dragNode.zapLink != null)
    {
      linkTarget = dragNode.zapLink;
      linkSource = dragNode;
      linkDragging = true;
      linkType = linkTarget.lastLinkType;
    }

    if (linkDragging) {
      linkDragging = false;
      if (linkTarget != null) {
         DeselectNodes(); // !! deal with shift correctly...
         linkTarget.isSelected = false;
         ActivateNodeEditor(null);
         if (linkTarget != linkSource && !linkTarget.hasLinkTo(linkSource)) {
           // Create link...
           if (linkType == -1)
              linkType = linkTarget.lastLinkType;
           linkTarget.inputs.add(new ModInput(linkTarget, linkSource, linkType, linkSource.id, linkSource.label));
           linkTarget.lastLinkType = linkType;
           // linkTarget.addInput(linkSource, linkType);
           isDirty = true;
         }
      }
    }
    if (rectDragging) {
      rectDragging = false;
    }

    dragNode = null;
    isDragging = false;
    didDragging = false;
    SetProperCursor(e);
    repaint();
  }

  public void mouseEntered(MouseEvent e)
  {
  }

  public void mouseExited(MouseEvent e)
  {
  }

  public void mouseClicked(MouseEvent e)
  {
  }

  static final int kMinReanchorLength = 16;
  static final double kMinPopSpeed = 2.0;
  static final double kMinPopAngle = 0.25; // about 15 degrees
  static final double kMinZapSpeed = 0.5;
  static final double kMinJoinAngle = 1.57; // 45 degrees
  static final int kMinJoinXConstraint = 80;
  static final int kMinJoinYConstraint = 40;

  void  computeDragInfo(MouseEvent e)
  {
    curPointTime = System.currentTimeMillis();
    curPoint = e.getPoint();
    if (curPointTime - startPointTime > 1000 ||
        (startPointTime > 0 && startPoint.distance(curPoint) > kMinReanchorLength) ||
        startPoint == null
        )
    {
      if (lastPointTime == 0 || lastPoint == null || curPointTime - lastPointTime > 1000)
      {
        startPoint = curPoint;
        startPointTime = curPointTime;
      }
      else {
        startPoint = lastPoint;
        startPointTime = lastPointTime;
      }
    }
    lastPoint = curPoint;
    lastPointTime = curPointTime;
    dragVecAngle = Math.atan2(curPoint.y-startPoint.y, curPoint.x-startPoint.x);
    dragVecLength = curPoint.distance(startPoint);
    if (startPoint.equals(curPoint) || curPointTime == startPointTime)
      dragVecSpeed = 0;
    else {
      dragVecSpeed = dragVecLength/(curPointTime-startPointTime);
    }
  }

  public String ShowZapStats()
  {
    return startPoint.x + "," + startPoint.y + " --> " + curPoint.x + "," + curPoint.y;
  }

  // Perform zap-link on dragnode, using vecAngle, vecLength
  public void ZapLinkage()
  {
    try {
      if (dragNode.nbrHardLinks == 0) {
        // Find best candidate for zap-linking, if any
        Enumeration e = unitNodes.mods.elements();
        Point dragPt = dragNode.bounds.getLocation();
        SSModule  candNode = null;
        while (e.hasMoreElements())
        {
          SSModule dstMod = (SSModule) e.nextElement();
          if (dstMod == dragNode)
            continue;
          if (dstMod.bounds.x <= dragNode.bounds.x)
            continue;
          double modAngle = Math.atan2(dstMod.bounds.y-dragNode.bounds.y,dstMod.bounds.x-dragNode.bounds.x);
          // !!! take into account modules which don't like being linked...
          if (dstMod == dragNode.zapLink)
          { // once we're connected, allow some leeway...
            if (candNode == null ||
                dragPt.distance(dstMod.bounds.getLocation()) < dragPt.distance(candNode.bounds.getLocation()) )
            {
                candNode = dstMod;
            }
          }
          // if not connected, require vector and distance constraints...
          else if (Math.abs(modAngle-dragVecAngle) < kMinJoinAngle &&
                   Math.abs(dstMod.bounds.y - dragPt.y) < kMinJoinYConstraint &&
                   Math.abs(dstMod.bounds.x - dragPt.x) < kMinJoinXConstraint &&
              // dragVecSpeed > kMinZapSpeed &&
               (candNode == null ||
                dragPt.distance(dstMod.bounds.getLocation()) < dragPt.distance(candNode.bounds.getLocation()) )
              )
          {
            candNode = dstMod;
          }
        }
        if (candNode != null)
        {
          // Clear zap-link if we are trying to pull it off...
          if (dragVecSpeed > kMinPopSpeed &&
              Math.abs(Math.atan2(dragNode.bounds.y-candNode.bounds.y,dragNode.bounds.x-candNode.bounds.x)-dragVecAngle) < kMinPopAngle)
           candNode = null;
        }

        // Zap-link to best candidate
        if (candNode != null && useZapDragging) {
          dragNode.zapLink = candNode;
        }
        else {
          dragNode.zapLink = null;
        }
      }
      else if (dragNode.nbrHardLinks == 1) {
        // Check if we want to disconnect...
        if (dragVecSpeed > kMinPopSpeed)
        {
            ModInput  link = GetFirstOutgoingLink(dragNode);
          SSModule  linkedNode = link.owner; // GetFirstOutgoingLink(dragNode);
          Point sp = dragNode.GetOutputDock(linkedNode);
          Point dp = linkedNode.GetInputDock(dragNode);
          double targetAngle = Math.atan2(sp.y-dp.y,sp.x-dp.x);
          if (Math.abs(targetAngle-dragVecAngle) < kMinPopAngle) // with about 30 degrees
          {
            // System.out.println("POP: " + dragVecSpeed + " " + ShowZapStats());
            linkedNode.inputs.remove(link);
            dragNode.nbrHardLinks = 0;
          }
        }
      }
    }
    catch (Exception e) {
      System.out.println("Zap problem: " + e.toString());
    }
  }

  public void paint(Graphics g)
  {
    LoadAssets();
    // System.out.println("paint");
    patchRender();
    if ((dbImage != null))
      g.drawImage(dbImage, 0, 0, this);
  }

  ModInput  GetFirstOutgoingLink(SSModule srcMod)
  {
    Enumeration e = unitNodes.mods.elements();
    while (e.hasMoreElements())
    {
      SSModule dstMod = (SSModule) e.nextElement();
      Enumeration ie = dstMod.inputs.elements();
      while (ie.hasMoreElements())
      {
        ModInput  inp = (ModInput) ie.nextElement();
        if (inp.link == srcMod)
          return inp;
      }
    }
    return null;
  }

  public int CountOutgoingLinks(SSModule srcMod)
  {
    int sum = 0;
    Enumeration e = unitNodes.mods.elements();
    while (e.hasMoreElements())
    {
      SSModule dstMod = (SSModule) e.nextElement();
      Enumeration ie = dstMod.inputs.elements();
      while (ie.hasMoreElements())
      {
        ModInput  inp = (ModInput) ie.nextElement();
        if (inp.link == srcMod)
          ++sum;
      }
    }
    return sum;
  }

  public void DeleteSelection()
  {
    Enumeration e = unitNodes.mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      if (mod.isSelected && mod.moduleType != MT_Output)
      {
        unitNodes.DeleteLinks(mod);
        mod.CleanUp();
        unitNodes.mods.remove(mod);
        isDirty = true;
      }
    }
   ActivateNodeEditor(null);
   repaint();

  }

  public void keyTyped(KeyEvent e) {
    // System.out.println("KEY TYPED");
    int keyCode = e.getKeyChar();
    if (keyCode == KeyEvent.VK_BACK_SPACE || keyCode == KeyEvent.VK_DELETE)
    {
      // System.out.println("DELETE");
      if (!isDragging && !linkDragging)
        DeleteSelection();
    }
  }

  public void keyPressed(KeyEvent e) {
    // System.out.println("KEY PRESSED");
  }

  public void keyReleased(KeyEvent e) {
    // System.out.println("KEY RELEASED");
  }
  // more methods, explained later...


  void DoSynthesize()
  {
      System.out.println("Synthesize");
      if (curEditNode != null)
        unitDefs[curEditNode.moduleType].SaveParamsToNode(curEditNode);
      itsSynthThread = new SynthThread(itsApp, itsPatch);
  }

  void DoWrite(String filename)
  {
      try {
        PrintWriter ar = new PrintWriter(new FileOutputStream(filename));
        itsPatch.mainInst.Save(ar);
        ar.close();
        isDirty = false;

      }
      catch (Exception e)
      {
        System.out.println("Uncaught exception during write: " + e.toString());
        e.printStackTrace();
      }
  }


  void ActivateNodeEditor(SSModule  node)
  {
    if (curEditNode != node)
    {
      if (curEditNode != null)
        unitDefs[curEditNode.moduleType].SaveParamsToNode(curEditNode);
      // Get appropriate panel from unitDefs and populate it\
      // Attach it to appropriate parent component.
      curEditNode = node;
      if (curEditNode != null) {
        itsApp.SwitchPPane(unitDefs[node.moduleType].itsPanel);
        unitDefs[curEditNode.moduleType].LoadParamsFromNode(curEditNode);
      }
      else {
        itsApp.SwitchPPane(itsApp.wavePanel);
      }
    }
  }

}  // end of SydPanel class