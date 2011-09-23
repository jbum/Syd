import javax.swing.*;
import javax.swing.text.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import  sun.audio.*;    //import the sun.audio package
import  java.io.*;

public class JSydApp extends JFrame implements WindowListener, ActionListener, ToolbarListener, SydConstants
{
    static final String versionStr = "1.0.3";
    static final int TB_OPEN = 0;
    static final int TB_SAVE = 1;
    static final int TB_SYNTHESIZE = 2;
    static final int TB_PLAY = 3;
    static final int TB_STOP = 4;
    static final int TB_MAGPLUS = 5;
    static final int TB_MAGMINUS = 6;
    static final int TB_SELECT = 7;
    static final int TB_GRABSCROLL = 8;

    static final int CURS_handfinger = 0;
    static final int CURS_handopen = 1;
    static final int CURS_handclosed = 2;
    static final int CURS_patchcord = 3;
    static final int CURS_zoomin = 4;
    static final int CURS_zoomout = 5;
    static final int kNbrCursors = 6;

    static Image[]   cursImages;
    static Cursor[]  cursors = new Cursor[kNbrCursors];


    static String[] cursNames = {"curs_handfinger","curs_handopen","curs_handclosed",
                                        "curs_patchcord", "curs_zoomin", "curs_zoomout"};
    static int[][]  cursHotspots = {{10,4},{12,11},{12,11},{20,2},{16,7},{16,7}};


    public static final long serialVersionUID = 1L;
    SydPanel       sp;
    // ParameterPanel genPanel;
    WaveformPanel  wavePanel;
    ToolbarPanel   tbPanel;

    JSplitPane    spp;
    public static JSydApp gApp;

    static  double      gDuration = 0;
    static  double      gSampleRate = 0;
    static  String      gPatchFilename = null;
    static  String      gOutputFilename = "";
    static  String      sydSpec = "";
    String  fileDir = null;
    String  fileSpec = null;
    int     currentTool = TB_SELECT;
    static Vector itsWindows = new Vector();


    {
      PatchOwner.gOutputFilename = gOutputFilename;
      PatchOwner.gDuration = gDuration;
      PatchOwner.gSampleRate = gSampleRate;
      PatchOwner.sydSpec = sydSpec;
      System.setProperty("apple.laf.useScreenMenuBar", "true");
      // Load Cursors
      Toolkit toolkit = Toolkit.getDefaultToolkit();
      cursImages = new Image[kNbrCursors];
      for (int i = 0; i < kNbrCursors; ++i) {
        try {
          String cursImageName = "assets/" + cursNames[i] + ".png";
          cursImages[i] = toolkit.getImage(this.getClass().getResource(cursImageName));
        }
        catch (Exception e) {
          System.out.println("Cursor loading problem with #" + i + "  " + e.toString());
        }
      }
    }

     public JSydApp(String myDir, String myFile)
     {
        setSize(800,600);
        Container c = getContentPane();
        c.setLayout( new BorderLayout() );
        SetupMenus();


        sp = new SydPanel(this);    // ms --> nanosecs

        // Do this if a filename was specified on command line...
        if (myFile != null)
        {
          System.out.println("Opening " + myFile);
          fileSpec = myFile;
          fileDir = myDir;
          sydSpec = myDir+myFile;
          this.setTitle(sydSpec);
          PatchOwner.sydSpec = sydSpec; // !! make sydSpec local...
          System.out.println("Set sydspec to " + PatchOwner.sydSpec);
          sp.itsPatch.mainInst.OpenSpec(sydSpec);
        }
        else {
        // otherwise, do this...
          setTitle("untitled.syd");
          SSModule  node = sp.unitNodes.GetOutputModule();
          node.InitUnitNode(sp.unitDefs[MT_Output], MT_Output, 600, 200);
        }

        // initialize empty doc

        ParameterPanel genPanel = new ParameterPanel(800, 200, "JSyd version " + versionStr);
        wavePanel = new WaveformPanel(this, 800, 200);
        tbPanel = new ToolbarPanel(this);
        SetupToolbar(tbPanel);

        // temp panel for top panel of splitter
        JPanel cPanel = new JPanel();
        cPanel.setLayout(new BorderLayout());
        cPanel.add(tbPanel,"North");
        cPanel.add(sp, "Center");


        spp = new JSplitPane(JSplitPane.VERTICAL_SPLIT,  cPanel, genPanel);
        spp.setOneTouchExpandable(true);
        spp.setDividerLocation(400);

//Provide minimum sizes for the two components in the split pane
        Dimension minimumSize = new Dimension(800, 100);
        sp.setMinimumSize(new Dimension(800, 100));
        // genPanel.setMinimumSize(new Dimension(800, 50));
        wavePanel.setMinimumSize(new Dimension(800, 50));

//        c.add(sp, "Center");
//        c.add(pp, "South");
        c.add(spp, "Center");
//        c.add(pp, "South");

        addWindowListener( this );
        setResizable(true);
        setVisible(true);
        JSydApp.itsWindows.add(this);
        setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
        repaint();
     }



  public void windowActivated(WindowEvent e)
  {
    gApp = this;
  }

  public void windowDeactivated(WindowEvent e)  {  }


  public void windowDeiconified(WindowEvent e)  {   }

  public void windowIconified(WindowEvent e)  {  }

  public void windowClosing(WindowEvent e)
  {
    doclose();
  }

  public void windowClosed(WindowEvent e)  {}
  public void windowOpened(WindowEvent e) {}


  public static void SyntaxExit()
  {
    System.out.println("JSydApp [options] [patchfile]\n");
    System.out.println("Options:\n");
    System.out.println("    -v #           verbosity level");
    System.out.println("    -x             score can override duration");
    System.exit(1);
  }

  public static void main(String args[])
  {
    if (args != null) {
       for (int i = 0; i < args.length; ++i)
       {
          if (args[i].startsWith("-"))
          {
            if (args[i].equals("-v")) {
              PatchOwner.gVerbose = Integer.parseInt(args[++i]);
            }
            else if (args[i].equals("-x")) {
              PatchOwner.gDurationOverride = true;
            }
            else {
              SyntaxExit();
            }
          }
          else if (gPatchFilename == null)
          {
            gPatchFilename = args[i];
          }
          else {
            SyntaxExit();
          }
       }
    }


    gApp = new JSydApp(null,gPatchFilename);    // ms --> nanosecs
  }

  public void SwitchPPane(JPanel pp)
  {
    int div = spp.getDividerLocation();
    spp.setBottomComponent(pp);
    spp.setDividerLocation(div);
  }

  JCheckBoxMenuItem autoPatchOff;
  JCheckBoxMenuItem autoPatchOn;

  public void SetupMenus()
  {
    JMenuBar menuBar = new JMenuBar();
    this.setJMenuBar(menuBar);

    //Build the first menu.
    JMenu filemenu = new JMenu("File");
    filemenu.setMnemonic('F');
    menuBar.add(filemenu);

    int keyMask = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();

    AddMenuItem(filemenu, new JMenuItem("New",KeyEvent.VK_N),
                                KeyStroke.getKeyStroke(KeyEvent.VK_N, keyMask));
    AddMenuItem(filemenu, new JMenuItem("Open...",KeyEvent.VK_O),
                                KeyStroke.getKeyStroke(KeyEvent.VK_O, keyMask));
    AddMenuItem(filemenu, new JMenuItem("Close"),null);
    filemenu.addSeparator(); // (new JMenuItem("-"));

    AddMenuItem(filemenu, new JMenuItem("Save",KeyEvent.VK_S),
                                KeyStroke.getKeyStroke(KeyEvent.VK_S, keyMask));
    AddMenuItem(filemenu, new JMenuItem("Save As..."),null);
    filemenu.addSeparator(); // (new JMenuItem("-"));
    AddMenuItem(filemenu, new JMenuItem("Exit"),
                                KeyStroke.getKeyStroke(KeyEvent.VK_Q, keyMask));

    JMenu editmenu = new JMenu("Edit");
    editmenu.setMnemonic('E');
    menuBar.add(editmenu);

//    AddMenuItem(editmenu, DefaultEditorKit.CutAction);
//    AddMenuItem(editmenu, DefaultEditorKit.CopyAction);
//    AddMenuItem(editmenu, DefaultEditorKit.PasteAction);


    AddMenuItem(editmenu, new JMenuItem("Undo",KeyEvent.VK_Z),
                                KeyStroke.getKeyStroke(KeyEvent.VK_Z, keyMask));
    editmenu.addSeparator(); // (new JMenuItem("-"));
    AddMenuItem(editmenu, new JMenuItem("Cut",KeyEvent.VK_X),
                                KeyStroke.getKeyStroke(KeyEvent.VK_X, keyMask));
    AddMenuItem(editmenu, new JMenuItem("Copy",KeyEvent.VK_C),
                                KeyStroke.getKeyStroke(KeyEvent.VK_C, keyMask));
    AddMenuItem(editmenu, new JMenuItem("Paste",KeyEvent.VK_V),
                                KeyStroke.getKeyStroke(KeyEvent.VK_V, keyMask));

    JMenu synthmenu = new JMenu("Synth");
    synthmenu.setMnemonic('S');
    menuBar.add(synthmenu);
    AddMenuItem(synthmenu, new JMenuItem("Synthesize"),null);
    AddMenuItem(synthmenu, new JMenuItem("Play"),null);
    AddMenuItem(synthmenu, new JMenuItem("Abort Synth"),null);

    JMenu patchmenu = new JMenu("Patch Options");
    patchmenu.setMnemonic('P');
    menuBar.add(patchmenu);
    autoPatchOn = (JCheckBoxMenuItem) AddMenuItem(patchmenu, new JCheckBoxMenuItem("Auto Connect On", false),null);
    autoPatchOff = (JCheckBoxMenuItem) AddMenuItem(patchmenu, new JCheckBoxMenuItem("Auto Connect Off", true),null);
  }

  public JMenuItem AddMenuItem(JMenu menu, JMenuItem item, KeyStroke ks)
  {
    if (ks != null)
      item.setAccelerator(ks);
    menu.add(item);
    item.addActionListener(this);
    return item;
  }

  public boolean saveAs()
  {
    FileDialog fd = new FileDialog(this, "Patch Filename", FileDialog.SAVE);
    fd.setFile(fileSpec == null? "untitled.syd" : fileSpec);
    if (fileDir != null)
      fd.setDirectory(fileDir);
    fd.setVisible(true);
    if (fd.getFile() != null) {
      fileDir = fd.getDirectory();
      fileSpec = fd.getFile();
      setTitle(fileSpec);
      sp.DoWrite(fileDir+fileSpec);
      return true;
    }
    else
      return false;
  }

  public void open()
  {
    FileDialog fd = new FileDialog(this, "Patch Filename", FileDialog.LOAD);
    fd.setFile(fileSpec == null? "untitled.syd" : fileSpec);
    if (fileDir != null)
      fd.setDirectory(fileDir);
    fd.setVisible(true);
    if (fd.getFile() != null) {
      // if the current app is empty, just load into this one...
      if (sp.itsPatch.mainInst.mods.size() == 1)
      {
        fileDir = fd.getDirectory();
        fileSpec = fd.getFile();
        sp.itsPatch.mainInst.OpenSpec(fileDir+fileSpec);
        setTitle(fileDir+fileSpec);
        sydSpec = fileDir+fileSpec;
        PatchOwner.sydSpec = sydSpec;
      }
      else {
        gApp = new JSydApp(fd.getDirectory(),fd.getFile());
      }
    }
  }


  AudioStream as = null;

  void doplay()
  {
    // ONLY ALLOW THIS IF WE HAVE SYNTHESIZED THE SOUND...
    if (sp.itsPatch.storageMod == null ||
        sp.itsPatch.storageMod.nbrFrames <= 0)
         return;

    //** add this into your application code as appropriate
    // Open an input stream  to the audio file.
    try {
      SSOutput outM = (SSOutput) sp.unitNodes.GetOutputModule();
      InputStream in = sp.itsPatch.storageMod.GetInputStream();
      // InputStream in = new BufferedInputStream(new FileInputStream(outM.outFileSpec));
      if (as != null)
        AudioPlayer.player.stop(as);
      as = new AudioStream(in);
      AudioPlayer.player.start(as);
      wavePanel.itsCanvas.setStartPlayback(System.currentTimeMillis());
      sp.ActivateNodeEditor(null);
    }
    catch (Exception e) {
      System.out.println("Exection playing audio: " + e.toString());
    }
    // Similarly, to stop the audio.
    // AudioPlayer.player.stop(as);
  }

  void dostop()
  {
    AudioPlayer.player.stop(as);
    sp.itsPatch.AbortSynthesis();
    wavePanel.itsCanvas.stopPlayback();
  }


  void doclose()
  {
    if (sp.isDirty) {
      int n = JOptionPane.showConfirmDialog(this,
        "Save changes to " + (fileSpec == null? "untitled patch" : fileSpec) + "?",
        "JSyd",
        JOptionPane.YES_NO_CANCEL_OPTION);
        if (n == 2)
          return;
        if (n == 0) {
          if (!dosave())
            return;
        }
    }
    // System.out.println("Closing!");
    if (as != null)
      AudioPlayer.player.stop(as);
    sp.cleanUp();
    JSydApp.itsWindows.remove(this);
    this.dispose();
    if (JSydApp.itsWindows.size() == 0) {
      System.exit(0);
    }
  }

    // return false if cancel
  public boolean dosave()
  {
    if (sp.curEditNode != null)
      sp.unitDefs[sp.curEditNode.moduleType].SaveParamsToNode(sp.curEditNode);

    if (fileSpec == null)
    {
      if (!saveAs())
        return false;
    }
    else
      sp.DoWrite(fileDir+fileSpec);
   return true;
  }

  public void doexit()
  {
    Vector wClone = (Vector) (JSydApp.itsWindows.clone());
    Enumeration e = wClone.elements();
    while (e.hasMoreElements())
    {
      JSydApp iApp = (JSydApp) e.nextElement();
      iApp.doclose();
    }
    System.exit(0);
  }

  public void actionPerformed( ActionEvent event )
  {
      // Add action handling code here
      // System.out.println( "action: " + event.getActionCommand() + ",param=" + event.paramString() );
      // System.out.println("App Action: " + event.getActionCommand());
      if (event.getActionCommand().equals("New"))
      {
        gApp = new JSydApp(null,null);
      }
      else if (event.getActionCommand().equals("Open..."))
      {
        open();
        repaint();
      }
      else if (event.getActionCommand().equals("Close"))
      {
        doclose();
      }
      else if (event.getActionCommand().equals("Save"))
      {
        dosave();
      }
      else if (event.getActionCommand().equals("Save As..."))
      {
        saveAs();
      }
      else if (event.getActionCommand().equals("Exit"))
      {
        doexit();
      }
      else if (event.getActionCommand().equals("Synthesize"))
      {
        sp.DoSynthesize();
      }
      else if (event.getActionCommand().equals("Play"))
      {
        doplay();
      }
      else if (event.getActionCommand().equals("Stop"))
      {
        dostop();
      }
      else if (event.getActionCommand().equals("Auto Connect On"))
      {
        sp.useZapDragging = true;
        autoPatchOn.setState(true);
        autoPatchOff.setState(false);
      }
      else if (event.getActionCommand().equals("Auto Connect Off"))
      {
        sp.useZapDragging = false;
        autoPatchOn.setState(false);
        autoPatchOff.setState(true);
      }
  }


  public void SetupToolbar(ToolbarPanel tp)
  {
      // Setup Icon Indices, Tooltips for toolbar.
      // tp.add(new ToolbarItem("Open", TB_OPEN, 0, false));
      // tp.add(new ToolbarItem("Save", TB_SAVE, 1, false));
      tp.add(new ToolbarItem("Synthesize Sound", TB_SYNTHESIZE, 2, false));
      tp.add(new ToolbarItem("Play Sound", TB_PLAY, 3, false));
      tp.add(new ToolbarItem("Stop", TB_STOP, 4, false));
      // tp.add(new ToolbarItem("Zoom In", TB_MAGPLUS, 5, false));
      // tp.add(new ToolbarItem("Zoom Out", TB_MAGMINUS, 6, false));
      // tp.addDivider();
      // tp.add(new ToolbarItem("Select", TB_SELECT, 7, true));
      // tp.add(new ToolbarItem("Grab Scroll", TB_GRABSCROLL, 8, false));
  }

  public void GetToolbarClick(int id)
  {
    switch (id) {
    case TB_OPEN:
      open();
      break;
    case TB_SAVE:
      dosave();
      break;
    case TB_SYNTHESIZE:
      sp.DoSynthesize();
      break;
    case TB_PLAY:
      doplay();
      break;
    case TB_STOP:
      dostop();
      break;
    case TB_MAGPLUS:
      break;
    case TB_MAGMINUS:
      break;
    case TB_SELECT:
      currentTool = TB_SELECT;
      ((ToolbarItem) tbPanel.items.elementAt(TB_SELECT)).isDepressed = true;
      ((ToolbarItem) tbPanel.items.elementAt(TB_GRABSCROLL)).isDepressed = false;
      tbPanel.repaint();
      break;
    case TB_GRABSCROLL:
      currentTool = TB_GRABSCROLL;
      ((ToolbarItem) tbPanel.items.elementAt(TB_SELECT)).isDepressed = false;
      ((ToolbarItem) tbPanel.items.elementAt(TB_GRABSCROLL)).isDepressed = true;
      tbPanel.repaint();
      break;
    }
    repaint();
  }

  void MySetCursor(Component cmp, int i)
  {
   if (cursors[i] == null) {
      try {
        Toolkit toolkit = Toolkit.getDefaultToolkit();
        Dimension d = toolkit.getBestCursorSize(24,24);
        // System.out.println("Cursor Size: " + d.width + ", " + d.height);
        cursors[i] = toolkit.createCustomCursor(cursImages[i], new Point(cursHotspots[i][0],cursHotspots[i][1]), cursNames[i]);
      }
      catch (Exception e) {
        System.out.println("Cursor problem");
        return;
      }
    }
    try {
      if (cursors[i] != null) {
        cmp.setCursor(cursors[i]);
     }
    }
    catch (Exception e) {
      System.out.println("CursorSet problem");
      return;
    }
  }

}
