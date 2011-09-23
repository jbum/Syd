import java.util.*;
import java.io.*;
import java.awt.*;

public class ModList implements SydConstants
{
  static final int MaxModules = 256;
  static final int MaxTimes = 16;

	double	        sampleDuration;
	double	        sampleRate;
  double          pTime;
  Vector          mods; // Consider using a vector so we can grow it more easily...
  PatchOwner      itsOwner;
  int             timeStackCtr;
  double[]        pTimes;

	static Vector mDesc;

	{
		mDesc = new Vector();
		mDesc.add(new ModuleDesc(MT_Output,			"out",		"Output"));
		mDesc.add(new ModuleDesc(MT_Oscillator,		"osc",		"Oscillator"));
		mDesc.add(new ModuleDesc(MT_Envelope,		"env",		"Envelope Generator"));
		mDesc.add(new ModuleDesc(MT_Mixer,			"mix",		"Mixer"));
		mDesc.add(new ModuleDesc(MT_Filter,			"filt",		"2nd-Order Filter"));
		mDesc.add(new ModuleDesc(MT_Butter,			"butter", 	"Butterworth Filter"));
		mDesc.add(new ModuleDesc(MT_Smooth,			"smooth",	"Smoother"));
		mDesc.add(new ModuleDesc(MT_Noise,			"noise",	"Noise Generator"));
		mDesc.add(new ModuleDesc(MT_Delay,			"delay",	"Reverb / Effects"));
		mDesc.add(new ModuleDesc(MT_Threshhold,		"thresh",	"Threshhold Unit"));
		mDesc.add(new ModuleDesc(MT_SampleAndHold,	"shold",	"Sample and Hold"));
		mDesc.add(new ModuleDesc(MT_Amplifier,		"amp",		"Amplifier"));
		mDesc.add(new ModuleDesc(MT_Inverter,		"inv",		"Inverter"));
		mDesc.add(new ModuleDesc(MT_Expression,  	"exp",		"Expression"));
		mDesc.add(new ModuleDesc(MT_Folder,  		"folder",	"Folder"));
		mDesc.add(new ModuleDesc(MT_RandScore,  	"rscore",	"Random Score"));
		mDesc.add(new ModuleDesc(MT_CScore,  		"cscore",	"CSound Score"));
		mDesc.add(new ModuleDesc(MT_FInput,			"finput",	"Folder Input"));
		mDesc.add(new ModuleDesc(MT_PInput,			"pinput",	"Score Parameter"));
		mDesc.add(new ModuleDesc(MT_FTable,			"ftable",	"Function Table"));
		mDesc.add(new ModuleDesc(MT_SampleFile,		"sample",	"Sample File"));
		mDesc.add(new ModuleDesc(MT_Pluck,			"pluck",	"Karplus/Strong Plucked String Sound"));
		mDesc.add(new ModuleDesc(MT_Maraca,			"maraca",	"Perry Cook's Maraca Simulation"));
		mDesc.add(new ModuleDesc(MT_HammerBank,		"hammerbank", "Hammer Bank"));
		mDesc.add(new ModuleDesc(MT_HammerActuator, "hammeract",  "Hammer Actuator"));
		mDesc.add(new ModuleDesc(MT_GAssign,		"gassign",	"Global Variable Assignment"));
		mDesc.add(new ModuleDesc(MT_SkiniScore,      "skiniscore", "Skini (STK) Score" ));
	}


	public ModList()
	{
		sampleDuration = 2.0;
		sampleRate = 22050;
    pTime = 0.0;
		itsOwner = null;
		mods = new Vector();
    pTimes = new double[MaxTimes];
    timeStackCtr = 0;
	}

	public ModList(PatchOwner itsOwner)
	{
		this();
		this.itsOwner = itsOwner;
	}

	public ModList(PatchOwner itsOwner, String fileSpec)
	{
		this(itsOwner);
		OpenSpec(fileSpec);
	}

  ModList CloneInstrument(SSModule rootModule)
  {
    int n;
    ModList   nList;
    SSModule  nn;

    nList = new ModList(itsOwner);
    if (nList == null)
      return null;

    nList.AddModule(rootModule.moduleType);


    nn = (SSModule) nList.mods.lastElement();
    nn.CopyAll(rootModule);  // Includes name & links

    int nbrModules = mods.size();

    for (n = 0; n < nbrModules; ++n) {
      SSModule  mod = (SSModule) mods.elementAt(n);
      // If module is in subtree
      if (rootModule.ContainsMod(mod.id) &&
        !nList.ContainsNamedMod(mod.label)) {
        nList.AddModule(mod.moduleType);
        nn = (SSModule) nList.mods.lastElement();
        nn.CopyAll(mod);
      }
    }

    // Resolve Links
    nList.ResolveModuleLinks();

    return nList;
  }

	boolean OpenSpec(String fsSpec)
	{
		BufferedReader readFile = null;
		try {
			readFile = new BufferedReader(new FileReader(fsSpec));
		}
		catch (IOException e)
		{
      System.out.println("Error opening patch: " + fsSpec + ": " + e.toString());
      // Try to open using path in global sydSpec
      int     li = PatchOwner.sydSpec.lastIndexOf('/');
      if (li == -1)
        li = PatchOwner.sydSpec.lastIndexOf('\\');
      if (li >= 0) {
        fsSpec = PatchOwner.sydSpec.substring(0,li+1) + fsSpec;
        try {
          readFile = new BufferedReader(new FileReader(fsSpec));
        }
        catch (IOException e2) {
          System.out.println("Error opening patch " + fsSpec + ": " + e2.toString());
          return true;
        }
      }
      else {
        return true;
      }

		}
		try {
			OpenArchive(readFile);
		}
		catch (Exception e)
		{
			System.out.println("Problem reading file " + fsSpec + ": " + e.toString());
      e.printStackTrace();
      System.exit(1);
		}
		return false;
	}


  boolean Save(PrintWriter ar) throws IOException
  {
    ar.println("MODS " + mods.size());

    // Recompute names to avoid collisions
    Enumeration e = mods.elements();
    int n = 0;
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      ++n;
      mod.ComputeName(n);
    }
    e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      mod.Save(ar);
    }
    return true;
  }

	boolean OpenArchive(BufferedReader ar) throws IOException
	{
		if (ar == null)
			return true;

		String p = GetNextInputLine(ar, "MODS");
		if (p != null) {
			// Parse MODS %d into n
			StringTokenizer st = new StringTokenizer(p);
			st.nextToken(); // skip MODS
      mods.clear();
			int n = Integer.parseInt(st.nextToken());
			for (int i = 0; i < n; ++i)
				LoadModule(ar);
      System.out.println("Loaded " + n + " modules");
      System.out.println("Mods size = " + mods.size());
		}
    int nbrModules = mods.size();
    for (int i = 1; i < nbrModules; ++i)
    {
      SSModule  mod = (SSModule) mods.elementAt(i);
      if (mod.moduleType == MT_Output) {
        mods.removeElementAt(i);
        mods.insertElementAt(mod,0);
      }
    }

    // Resolve Links
    ResolveModuleLinks();

    ar.close();

		return false;
	}

  void ResolveModuleLinks()
  {
    int nbrModules = mods.size();
    // System.out.println("Modlist: Resolve module links");

    for (int n = 0; n < nbrModules; ++n) {
      SSModule modn = (SSModule) mods.elementAt(n);
      int nbrInputs = modn.inputs.size();
      for (int i = 0; i < nbrInputs; ++i) {
        ModInput  inpi = (ModInput) modn.inputs.elementAt(i);
        // System.out.println("Mod " + n + " input " + i + " " + inpi.fromLabel + " -> " + inpi.destID + " inp " + inpi);
        if (inpi.destID == -1)
          inpi.destID = GetModuleID(inpi.fromLabel);
        inpi.link = GetModule(inpi.destID);
        // System.out.println("Resolved link to mod " + n + " to " + inpi.link);
      }
    }
  }

  public SSModule GetModule(int id)
  {
    if (mods == null)
      return null;
    Enumeration e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      if (mod.id == id)
        return mod;
    }
    return null;
  }

  int GetModuleID(String label)
  {
    if (mods == null)
      return -1;
    Enumeration e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      if (mod.label.equals(label))
        return mod.id;
    }
    return -1;
  }

  boolean ContainsNamedMod(String label)
  {
    if (mods == null)
      return false;
    Enumeration e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      if (mod.label.equals(label))
        return true;
    }
    return false;
  }

  SSModule AddModule(int n)
  {
    SSModule  nn = null;
    // System.out.println("Got Module Type " + n);
    switch (n) {
    case MT_Output:   nn = new SSOutput(this);          break;
    case MT_Oscillator: nn = new SSOscillator(this);    break;
    case MT_Amplifier:  nn = new SSAmplifier(this);     break;
    case MT_Envelope: nn = new SSADSR(this);            break;
    case MT_Inverter: nn = new SSInverter(this);        break;
    case MT_Noise:    nn = new SSNoise(this);           break;
    case MT_Threshhold: nn = new SSThreshhold(this);    break;
    case MT_SampleAndHold: nn = new SSSampleAndHold(this);  break;
    case MT_Delay:    nn = new SSDelay(this);           break;
    case MT_Filter:   nn = new SSFilter(this);          break;
    case MT_Butter:   nn = new SSButter(this);          break;
    case MT_Smooth:   nn = new SSSmooth(this);          break;
    case MT_Expression: nn = new SSExpression(this);    break;
    case MT_Mixer:    nn = new SSMixer(this);           break;
    case MT_RandScore:  nn = new SSRandScore(this);     break;
    case MT_CScore:   nn = new SSCScore(this);      break;
    case MT_Folder:   nn = new SSFolder(this);      break;
    case MT_FInput:   nn = new SSFInput(this);      break;
    case MT_PInput:   nn = new SSPInput(this);      break;
    case MT_FTable:   nn = new SSFTable(this);      break;
    case MT_SampleFile: nn = new SSSampleFile(this);    break;
    case MT_Pluck:    nn = new SSPluck(this);       break;
    case MT_Maraca:   nn = new SSMaraca(this);      break;
    case MT_HammerBank: nn = new SSHammerBank(this);    break;
    case MT_HammerActuator: nn = new SSHammerActuator(this);break;
    case MT_GAssign:  nn = new SSGAssign(this);     break;
    case MT_SkiniScore:   nn = new SSSkiniScore(this);        break;
    }
    if (nn != null)
      mods.add(nn);
    else
      System.out.println("AddModule: Couldn't add module, type=" + n);
   return nn;
  }

  void LoadModule(BufferedReader ar) throws IOException
  {
    String p = GetNextInputLine(ar, "MOD");
    StringTokenizer st;
    if (p == null)
      return;

    // Remove parens MOD name name (x1 y1 x2 y2)
    int p1 = p.indexOf('(');
    p = p.substring(0,p1) + p.substring(p1+1);
    int p2 = p.indexOf(')');
    p = p.substring(0,p2);

    // Parse MODS %d into n
    st = new StringTokenizer(p);
    st.nextToken(); // skip MOD
    String modLabel = st.nextToken();
    String modTypeName = st.nextToken();
    int x1 = Integer.parseInt(st.nextToken());
    int y1 = Integer.parseInt(st.nextToken());
    int x2 = Integer.parseInt(st.nextToken());
    int y2 = Integer.parseInt(st.nextToken());
    // System.out.println(x1+","+y1+" --> " + x2+","+y2);
    int moduleType = NameToModuleType(modTypeName);
    int id = mods.size();
    SSModule nn = AddModule(moduleType);
    nn.setLabel(modLabel);
    nn.bounds = new Rectangle(x1,y1,SydPanel.NodeWidth,SydPanel.NodeWidth);
    // else
    //  nn.bounds = new Rectangle(x1,y1,x2-x1,y2-y1);


    nn.id = id;

    // parse inputs here... INPUT from -> input
    while ((p = GetNextInputLine(ar, "IN")) != null)
    {
      st = new StringTokenizer(p);
      st.nextToken(); // skip IN
      String fromLabel = st.nextToken();
      st.nextToken();  // skip ->
      String inputName = st.nextToken();
      int inputType = nn.NameToSignalType(inputName);
      if (inputType < 0) {
        System.out.println("Modlist: Could not find signal type: " + inputName);
        inputType = 0;
      }
      if (PatchOwner.gVerbose > 1)
        System.out.println("Modlist: Adding input type " + inputType + ", label = " + fromLabel);
      nn.inputs.add(new ModInput(nn,null, inputType, -1, fromLabel));
      ModInput inpi = (ModInput) nn.inputs.elementAt(nn.inputs.size()-1);
     //  System.out.println(inpi + ": " + inpi.fromLabel);
    }
    nn.Load(ar);
  }


  void ResetInstruments(SSModule callingMod)
  {
    int n;

    if (PatchOwner.gVerbose > 1)
      System.out.println("Reset Instruments");

    InitTime(0);

    // Reset Modules
    Enumeration e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      if (PatchOwner.gVerbose > 1)
        System.out.println("Resetting " + mod.label);
      mod.Reset(callingMod);
    }

    // Fill FTables (which may depend on reset modules)
    e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      if (mod.moduleType == MT_FTable) {
        ((SSFTable) mod).FillTable(callingMod);
      }
    }

    // Generate Scores (which may depend on ftables, etc.)
    e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      if (mod.moduleType == MT_RandScore ||
          mod.moduleType == MT_CScore ||
          mod.moduleType == MT_SkiniScore)
       {
          ((SSScore) mod).InitScore(callingMod);
       }
    }
  }

  void CleanUpInstruments()
  {
    Enumeration e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      mod.CleanUp();
    }
  }

  void DisposeModuleList()
  {
    Enumeration e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      mod.CleanUp();
    }
    mods = new Vector();
  }

	static String	lastString = null;

	String GetNextInputLine(BufferedReader ar, String pat) throws IOException
	{
		while (true)
		{
			if (lastString == null) {
				lastString = ar.readLine();
        // System.out.println("LastString: read " + lastString);
        if (lastString == null)
          return null;
        if (PatchOwner.gVerbose > 1) {
          System.out.println("Read: " + lastString);
        }
      }
			if (lastString.startsWith("#") ||
				lastString.startsWith(";") ||
				lastString.length() == 0)
			{
				lastString = null;
				continue;
			}
			if (lastString.startsWith(pat))
			{
        // System.out.println("LastString: starts with " + pat);
				String p = lastString;
				lastString = null;
				return p;
			}
			return null;
		}
	}

  String GetModuleName(int n)
  {
    ModuleDesc md = (ModuleDesc) mDesc.elementAt(n);
    return md.name;
  }

  String GetModuleDesc(int n)
  {
    ModuleDesc md = (ModuleDesc) mDesc.elementAt(n);
    return md.desc;
  }

  int NameToModuleType(String modTypeName)
  {
    Enumeration e = mDesc.elements();
    int i = 0;
    while (e.hasMoreElements())
    {
      ModuleDesc  md = (ModuleDesc) e.nextElement();
      if (md.name.equals(modTypeName))
        return i;
      ++i;
    }
    return -1;
  }

  SSModule GetOutputModule()
  {
    Enumeration e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mod = (SSModule) e.nextElement();
      if (mod.moduleType == MT_Output)
        return mod;
    }
    return null;
  }

  void DeleteLinks(SSModule ss)
  {
    Enumeration e = mods.elements();
    while (e.hasMoreElements())
    {
      SSModule mp = (SSModule) e.nextElement();
      int nbrInputs = mp.inputs.size();
      for (int i = 0; i < nbrInputs; ++i)
      {
        ModInput mi = (ModInput) mp.inputs.elementAt(i);
        if (mi.link == ss) {
          mp.inputs.removeElementAt(i);
          --i;
          --nbrInputs;
        }
      }
    }
  }

  void InitTime(double timeVal)
  {
    timeStackCtr = 0;
    pTime = timeVal;
  }

  void PushTime(double timeVal)
  {
    pTimes[timeStackCtr++] = pTime;
    pTime = timeVal;
  }

  void PopTime()
  {
    pTime = pTimes[--timeStackCtr];
  }


} // end of class

