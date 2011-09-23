// import ModList;

import java.io.*;
import java.util.*;
import java.awt.*;

public class SSModule implements SydConstants
{
  static int gID = 0;
  // static int gModAlloc = 0;
  static final int MaxInputs = 32;

  ModList 		parList;
  SSModule 		callingMod;
  String		  label;
  int			    nbrSupportedLinks;
  int			    moduleType;
  int			    id;


  Vector	inputs;
  LinkDesc[]		linkDesc;

  // used for stereo mode
  double        lastRightSample = 0;
  double        lastRightInput = 0;

  // UI Stuff
  Rectangle bounds;
  boolean   isSelected = false;
  // zaplink stuff
  SSModule  zapLink = null;
  int       nbrHardLinks;
  UnitDef   itsUnitDef;
  int       lastLinkType = 0;
  // String[]  itsParams;


  public SSModule(int itsType, ModList mList)
  {
	  // ++gModAlloc;
	  this.parList = mList;
	  inputs = new Vector();
	  linkDesc = new LinkDesc[MaxInputs+1];
	  do {
		  id = gID++;
	  } while (parList.GetModule(id) != null);
	  moduleType = itsType;
	  callingMod = null;
    DescribeLink(0, "Audio Signal", "sig");
	  for (int i = 1; i <= MaxInputs; ++i)
	  {
		  DescribeLink(i, "Undefined", "?");
	  }
	  nbrSupportedLinks = 1;
    ComputeName(parList.mods.size()+1);
  }

  public void ComputeName(int n)
  {
    String modName = parList.GetModuleName(moduleType);
    modName = modName.substring(0,1).toUpperCase() + modName.substring(1);
    label = "m" + modName + n;
  }

  public SSModule(SSModule mod) // Copy Constructor
  {
    this(mod.moduleType, mod.parList);
  }

  int NameToSignalType(String name)
  {
	  for (int i = 0; i < nbrSupportedLinks; ++i)
	  {
		  if (linkDesc[i] != null && linkDesc[i].varName.equals(name))
		  	return i;
	  }
	  return -1;
  }

  LinkDesc GetLinkDesc(int linkType)
  {
    if (linkType > nbrSupportedLinks)
      linkType = 0;
    return linkDesc[linkType];
  }

  void DescribeLink(int linkNbr, String desc, String varName)
  {
    linkDesc[linkNbr] = new LinkDesc(desc, varName);
    if (linkNbr >= nbrSupportedLinks)
      nbrSupportedLinks = linkNbr + 1;
  }

  boolean hasLinkTo(SSModule srcMod)
  {
    Enumeration e = inputs.elements();
    while (e.hasMoreElements())
    {
      ModInput inp = (ModInput) e.nextElement();
      if (inp.link == srcMod)
        return true;
    }
    return false;
  }

	void CopyAll(SSModule mod)
	{
		Copy(mod);
		// Copy Name & ID
    this.label = mod.label;
		this.id = mod.id;

		// Copy Links (generic)
    inputs.clear();
    Enumeration e = mod.inputs.elements();
    while (e.hasMoreElements())
    {
      ModInput  inp = (ModInput) e.nextElement();
      inputs.add(new ModInput(this,null, inp.inputType, -1, inp.fromLabel));
		}
	}

  boolean ContainsMod(int id)
  {
    if (this.id == id)
      return true;

    Enumeration e = inputs.elements();
    while (e.hasMoreElements())
    {
      ModInput inp = (ModInput) e.nextElement();
      if (inp != null &&
          inp.link != null &&
          inp.link.ContainsMod(id))
          return true;
    }
    return false;
  }


  void Copy(SSModule mod)
  {
  }

  void Save(PrintWriter ar) throws IOException
  {
    ar.println("MOD " +
               label + " " +
               parList.GetModuleName(moduleType) + " " +
               "(" + bounds.x + " " +
                     bounds.y + " " +
                     (bounds.x + bounds.width) + " " +
                     (bounds.y + bounds.height) +
                ")");
    if (inputs.size() > 0) {
      Enumeration e = inputs.elements();
      while (e.hasMoreElements())
      {
        ModInput  inp = (ModInput) e.nextElement();
        LinkDesc ld = GetLinkDesc(inp.inputType);
        SSModule  mod = inp.link; // parList.GetModule(inp.link.id);
        ar.println("IN " + mod.label + " -> " + ld.varName);

      }
    }
  }

  void Load(BufferedReader ar) throws IOException
  {
  }

  void setLabel(String label)
  {
    this.label = label;
  }

  double GenerateOutput(SSModule callingMod)
  {
    this.callingMod = callingMod;
    double retVal = MixInputs(-1, callingMod);
    lastRightSample = lastRightInput;
    return retVal;
  }

  double getRightSample()
  {
    return lastRightSample;
  }

  double getRightInput()
  {
    return lastRightInput;
  }

  // copied from SSOutput...
  double GenerateOutputTime(SSModule callingMod, double pTime)
  {
    double v;
    parList.InitTime(pTime);
    v = GenerateOutput(callingMod);
    return v;
  }

/*
  double GenerateStereoOutputTime(SSModule callingMod, double pTime)
  {
    double v;
    parList.InitTime(pTime);
    v = GenerateStereoOutput(callingMod);
    return v;
  }
*/

  // Used to allow a folder instrument to retrieve input signals to the folder
  // !!! needs to allow for boolean return so we can detect lack of folder sig
  double GetFolderSig(int n) throws Exception
  {
    if (callingMod != null &&  callingMod != this)
      return callingMod.GetFolderSig(n);
    else
     throw new Exception("Unknown Folder Sig");
  }

  boolean IsInScore()
  {
    if (callingMod != null &&  callingMod != this)
      return callingMod.IsInScore();
    else
      return false;
  }

  double GetNoteTime()
  {
    if (callingMod != null &&  callingMod != this)
      return callingMod.GetNoteTime();
    else
      return parList.itsOwner.gTime;
  }

  double GetKeyValue()
  {
    if (callingMod != null &&  callingMod != this)
      return callingMod.GetKeyValue();
    else
      return 0;
  }

  double GetIValue()
  {
    if (callingMod != null &&  callingMod != this)
      return callingMod.GetIValue();
    else
      return 0;
  }

  double GetMValue()
  {
    if (callingMod != null &&  callingMod != this)
      return callingMod.GetMValue();
    else
      return 1;
  }

  double GetAValue()
  {
    if (callingMod != null &&  callingMod != this)
      return callingMod.GetAValue();
    else
      return 1;
  }


  // Used to retreive parameters from the enclosing score module
  double GetInstParameter(int n)
  {
    // System.out.println("GetInstParam: " + this.getClass().getName());
    if (callingMod != null &&  callingMod != this)
      return callingMod.GetInstParameter(n);
    else
      return 0.0;
  }


  double MixInputs(int type, SSModule callingMod)
  {
    double v = 0.0, vR = 0.0;
    int i;

    this.callingMod = callingMod;

    Enumeration e = inputs.elements();
    while (e.hasMoreElements())
    {
      ModInput  inp = (ModInput) e.nextElement();
      if (type == -1 || inp.inputType == type) {
        if (inp.link == null)
          System.out.println("Missing Input: type=" + type + ", this.label=" + label);
        v += inp.link.GenerateOutput(this);
        vR += inp.link.getRightSample();
      }
    }
    lastRightInput = vR;
    return  v;
  }


  int CountInputs(int type)
  {
    int n = 0;

    if (type == -1)
      return inputs.size();

 //   if (PatchOwner.gVerbose > 1) {
 //     System.out.println("Counting inputs from vector of " + inputs.size());
 //   }

    Enumeration e = inputs.elements();

    while (e.hasMoreElements())
    {
       ModInput inp = (ModInput) e.nextElement();
        if (inp.inputType == type)
          n++;
    }
    return  n;
  }

  int CountInputTypes()
  {
    int   n=0,i;
    long  iFlags = 0;

    Enumeration e = inputs.elements();
    while (e.hasMoreElements())
    {
       ModInput inp = (ModInput) e.nextElement();
      if ((iFlags & (1L << inp.inputType)) == 0)
      {
        ++n;
        iFlags |= (1L << inp.inputType);
      }
    }
    return  n;
  }

  void  Reset(SSModule callingMod)
  {
    this.callingMod = callingMod;
  }

  void  CleanUp()
  {
  }

  int CompileExp(ExpRec exp)
  {
    int result;
    result = parList.itsOwner.expMgr.CompileExp(this, exp);
    if (result == 0 && (exp.cFlags & ExpMgr.EF_IsConstant) != 0)
      exp.eConst = parList.itsOwner.expMgr.EvalF(this, exp);
    else
      exp.eConst = 0;
    return result;
  }

  double ResetExp(ExpRec exp, SSModule callingMod)
  {
    this.callingMod = callingMod;
    exp.eConst = parList.itsOwner.expMgr.EvalF(this, exp);
    exp.cFlags |= ExpMgr.EF_NeedsSolving;
    return exp.eConst;
  }

  double SolveExp(ExpRec exp, SSModule callingMod)
  {
    this.callingMod = callingMod;

    // 12/3/98 - modified to always solve expressions on resets and
    //           on first sample
    if ((exp.cFlags & ExpMgr.EF_NeedsSolving) != 0)
    {
      exp.eConst = parList.itsOwner.expMgr.EvalF(this, exp);
      if ((exp.cFlags & (ExpMgr.EF_IsConstant | ExpMgr.EF_NoTime)) != 0)
        exp.cFlags &= ~ExpMgr.EF_NeedsSolving;
    }
    return exp.eConst;
  }

  void ClearExp(ExpRec exp)
  {
    exp.Init("", new char[ExpMgr.LenCompiledExp], new double[ExpMgr.MaxFloats], 0, 0.0);
  }


  void CopyExp(ExpRec src, ExpRec dst)
  {
    dst.Init(src.exp, src.cExp, src.fExp, src.cFlags, src.eConst);
  }

  ExpRec InitExp(String str) // Use this also in place of PrintfExp
  {
    ExpRec exp = new ExpRec(str.trim());
    CompileExp(exp);
    return exp;
  }

  ExpRec LoadExp(BufferedReader ar, String lab) throws IOException
  {
    String p = parList.GetNextInputLine(ar, lab);
    // System.out.println("Loading expression from " + lab + " : " + p);
    return InitExp(p.substring(lab.length()+1));
  }

  String[] getTokens(BufferedReader ar, String lab) throws IOException
  {
    String p = parList.GetNextInputLine(ar, lab);
    StringTokenizer st = new StringTokenizer(p);
    int nbrTokens = st.countTokens();
    String[] tokens = new String[nbrTokens];
    for (int i =0 ; i < nbrTokens; ++i)
      tokens[i] = st.nextToken();
    return tokens;
  }

  String getFirstToken(BufferedReader ar, String lab) throws IOException
  {
    String[] tokens = getTokens(ar, lab);
    return tokens[1];
  }

  String LoadString(BufferedReader ar, String lab) throws IOException
  {
    String p = parList.GetNextInputLine(ar, lab);
    return p.substring(lab.length()+1);
  }

  // UI Stuff
  void InitUnitNode(UnitDef itsUnitDef, int type, int x, int y)
  {
    this.itsUnitDef = itsUnitDef;
    // this.unitType = type; - use super.moduleType
    bounds = new Rectangle(x,y,SydPanel.NodeWidth,SydPanel.NodeWidth);
    /*
    itsParams = new String[itsUnitDef.nbrSupportedParams];
    for (int i = 0; i < itsParams.length; ++i)
    {
      itsParams[i] = itsUnitDef.paramDef[i].getDefault();
    }
    */
  }

  Rectangle GetDockRect()
  {
    return new Rectangle(bounds.x+bounds.width-SydPanel.PatchOutputLMargin,bounds.y,SydPanel.PatchOutputLMargin+SydPanel.PatchOutputRMargin,bounds.height);
  }

  static final int kDockMargin = 8;

  Point GetOutputDock(Point ref)
  {
    if ((ref.y < bounds.y + bounds.height ||
      Math.abs(ref.x - bounds.x+bounds.width) > Math.abs(ref.y - bounds.y + bounds.height)) &&
      !(bounds.x > ref.x)) {
      return new Point(bounds.x+bounds.width-kDockMargin, bounds.y+bounds.height/2);
    }
    else {
      return new Point(bounds.x+bounds.width/2, bounds.y+bounds.height-kDockMargin);
    }
  }

  Point GetOutputDock(SSModule dst)
  {
    if ((dst.bounds.y < bounds.y+bounds.height ||
      Math.abs(dst.bounds.x - bounds.x+bounds.width) >
      Math.abs(dst.bounds.y - bounds.y+bounds.height)) &&
      !(dst.bounds.x < bounds.x+bounds.width))
    {
      return new Point(bounds.x+bounds.width-kDockMargin, bounds.y+bounds.height/2);
    }
    else {
      return new Point(bounds.x+bounds.width/2, bounds.y+bounds.height-kDockMargin);
    }
  }

  Point GetInputDock(SSModule src)
  {
    if ((bounds.y < src.bounds.y+src.bounds.height ||
      Math.abs(bounds.x - src.bounds.x+src.bounds.width) >
      Math.abs(bounds.y - src.bounds.y+src.bounds.height)) &&
      !(bounds.x < src.bounds.x+src.bounds.width))
    {
      return new Point(bounds.x+kDockMargin, bounds.y+bounds.height/2);
    }
    else {
      return new Point(bounds.x+bounds.width/2, bounds.y+kDockMargin);
    }
  }


}

