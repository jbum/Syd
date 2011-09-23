import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.Timer;
import java.util.TimerTask;
//import javax.swing.event.*;
//import java.awt.event.*;
//import java.awt.geom.*;

public class GraphCanvas  extends JPanel implements ComponentListener, MouseListener, MouseMotionListener
{
  public static final long serialVersionUID = 1L;
  Image         dbImage = null;
  boolean       needsRefresh = false;

  int           nbrSamples = 0;
  double        minPt = 0, maxPt = 1; // Normalized zooming info
  double        sampleRate = 22000;

  PatchOwner    patchOwner = null;
  int           graphMode = 0;      // 1 = graph with lines

  boolean rectDragging = false;
  boolean checkForClick = false;
  Point rectAnchor, rectPoint;
  long  startPlayback = 0;
  boolean isPlayingBack;

  boolean grabDragging = false;
  JSydApp itsApp;

  public GraphCanvas(JSydApp itsApp)
  {
    this.itsApp = itsApp;
    addComponentListener(this);
    addMouseListener(this);
    addMouseMotionListener(this);
  }

  public void setStartPlayback(long millis)
  {
    startPlayback = millis;
    isPlayingBack = true;
    repaint();
  }

  public void stopPlayback()
  {
    isPlayingBack = false;
    repaint();
  }

  public void drawWaveform(Graphics g)
  {
    Graphics2D g2 = (Graphics2D) g;
    int cy = getHeight()/2;
    int cyR = 0;
    int    startPos = (int) (nbrSamples *minPt);
    int    endPos = (int) (nbrSamples *maxPt);
    int    sampleWidth = endPos - startPos;
    double xScale = getWidth() / (double) (endPos - startPos);
    double yScale = cy / (double) 0x07FFF;
    int   skip;
    g.setColor(Color.white); // was 88,88,88
    g.fillRect (0, 0, getWidth(), getHeight());
    g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                      RenderingHints.VALUE_ANTIALIAS_ON);

    if (patchOwner != null &&
       patchOwner.storageMod != null &&
       nbrSamples > 0)
    {
      SSStorage sm = patchOwner.storageMod;
      // System.out.println("Graphing " + nbrSamples + ", channels = " + sm.nbrChannels);
      if (sampleWidth < getWidth()*4)
        skip = 1;
      else if (sampleWidth < getWidth()*10)
        skip = 4;
      else
        skip = (sampleWidth/getWidth())/10;

      g.setColor(Color.black); // was 88,88,88

      int lastY = 0;
      int lastX = 0;

      int lastYR = 0;
      int lastXR = 0;

      if (sm.nbrChannels == 1)
      {
        for (int i =0 ; i < sampleWidth; i += skip) {
          int  x = (int)(i*xScale);
          int  yd;
          if (startPos+i < sm.nbrFrames)
            yd = (int) (sm.GetSample(startPos+i) * yScale);
          else
            yd = 0;
          if (i > 0)
           g.drawLine(lastX,lastY, x,cy-yd);
          if (xScale >= 1)
           // g.fillRect(x,cy-yd-1,1,3);
           g.fillOval(x-1,cy-yd-1,3,3);
          lastY = cy-yd;
          lastX = x;
        }
      }
      else { // STEREO PLOT
        cy = getHeight()/4;
        yScale =  (getHeight()/4) / (double) 0x07FFF;
        cyR = (getHeight()*3)/4;

        for (int i =0 ; i < sampleWidth; i += skip)
        {
          int  x = (int)(i*xScale);
          int  yd;
          int  sampleIdx = (startPos+i)*2;
          if (sampleIdx < sm.nbrFrames*sm.nbrChannels)
            yd = (int) (sm.GetSample(sampleIdx) * yScale);
          else
            yd = 0;
          if (i > 0)
           g.drawLine(lastX,lastY, x,cy-yd);
          if (xScale >= 1)
           // g.fillRect(x,cy-yd-1,1,3);
           g.fillOval(x-1,cy-yd-1,3,3);
          lastY = cy-yd;
          lastX = x;

          sampleIdx = (startPos+i)*2+1;
          if (sampleIdx < sm.nbrFrames*sm.nbrChannels)
            yd = (int) (sm.GetSample(sampleIdx) * yScale);
          else
            yd = 0;
          if (i > 0)
           g.drawLine(lastXR,lastYR, x,cyR-yd);
          if (xScale >= 1)
           // g.fillRect(x,cy-yd-1,1,3);
           g.fillOval(x-1,cyR-yd-1,3,3);
          lastYR = cyR-yd;
          lastXR = x;
        }
        g.setXORMode(Color.white);
        g.setColor(Color.green);
        g.drawLine(0,cyR,getWidth(),cyR);
        g.setPaintMode();
      }
    }
    g.setXORMode(Color.white);
    g.setColor(Color.green);
    g.drawLine(0,cy,getWidth(),cy);
    g.setPaintMode();
  }

  public void paint(Graphics g)
  {
    // Draw WaveForm
    if (needsRefresh || dbImage == null)
    {
      needsRefresh = false;
      if (dbImage == null){
        // System.out.println("Making dbImage cWidth = " + this.getWidth());
        dbImage = createImage(this.getWidth(), getHeight());
        if (dbImage == null) {
          System.out.println("dbImage is null");
          return;
        }
      }
      drawWaveform(dbImage.getGraphics());
    }
    g.drawImage(dbImage, 0, 0, this);
    if (rectDragging)
    {
      Graphics2D g2 = (Graphics2D) g;
      // System.out.println("Drawing rect " + rectAnchor.x + " -> " + rectPoint.x);
      // Switch to inverting a range of the waveform...
      g2.setColor(new Color(0,0,255));
      g2.setStroke(new BasicStroke(1));
      g2.setXORMode(Color.white);
      int x1 = rectAnchor.x;
      int x2 = rectPoint.x;
      if (x1 > x2) {
        int x3 = x1;
        x1 = x2;
        x2 = x3;
      }
      g2.fillRect(x1, 0, x2 - x1, getHeight());
      g2.setPaintMode();
    }
    if (isPlayingBack)
    {
      double curPt = (System.currentTimeMillis()-startPlayback)/(nbrSamples*1000/sampleRate);
      // System.out.println("curPt = " + curPt);
      if (curPt >= minPt && curPt <= maxPt)
      {
        int x = (int) ((curPt-minPt)*getWidth()/(maxPt-minPt));
        Graphics2D g2 = (Graphics2D) g;
        g2.setColor(new Color(0,0,255));
        g2.setStroke(new BasicStroke(1));
        g2.setXORMode(Color.white);
        g2.drawLine(x, 0, x, getHeight());
        g2.setPaintMode();
      }
      if (curPt > 1)
        isPlayingBack = false;
      if (curPt <= maxPt)
      {
        if (curPt < minPt)
           ScheduleFutureUpdate((long) ((minPt-curPt)*(nbrSamples*1000/sampleRate)));
        else
           // repaint();
           ScheduleFutureUpdate((long) 30);
      }
      else {
        // System.out.println("Curpt: " + curPt);
      }
    }
  }

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
    dbImage = null;
    repaint();
  }

  public void MySetCursor(int id)
  {
    itsApp.MySetCursor(this, id);
  }

  public void SetProperCursor(InputEvent e)
  {
    if (itsApp.currentTool == JSydApp.TB_SELECT)
    {
      int modifiers = e.getModifiers();
      // System.out.println("Clicked Mods = " + modifiers);
      if ((modifiers & InputEvent.ALT_MASK) != 0)
        MySetCursor(JSydApp.CURS_handopen);
      else if ((modifiers & (InputEvent.CTRL_MASK | InputEvent.BUTTON2_MASK | InputEvent.BUTTON3_MASK)) != 0)
        MySetCursor(JSydApp.CURS_zoomout);
      else
        MySetCursor(JSydApp.CURS_zoomin);
    }
    else if (itsApp.currentTool == JSydApp.TB_GRABSCROLL) {
      MySetCursor(JSydApp.CURS_handopen);
    }
  }

  // MouseMotionListener
  public void mouseMoved(MouseEvent e)
  {
    SetProperCursor(e);
  }

  public void mouseDragged(MouseEvent e)
  {
    if (rectDragging)
    {
      rectPoint = e.getPoint();
      repaint();
    }
    if (grabDragging) {
      double deltaN = (rectPoint.x - e.getX()) / (double) getWidth(); // normalized delta
      rectPoint = e.getPoint();
      // change minPt, maxPt
      double newMinPt = minPt + (maxPt - minPt)*deltaN;
      double newMaxPt = newMinPt + (maxPt - minPt);
      minPt = newMinPt;
      maxPt = newMaxPt;
      if (minPt < 0) {
        maxPt += -minPt;
        minPt = 0;
      }
      else if (maxPt > 1) {
        minPt -= maxPt-1;
        maxPt = 1;
      }
      if (minPt < 0)
        minPt = 0;
      if (maxPt > 1)
        maxPt = 1;
      doFullRefresh();
    }
  }

  // MouseListener
  public void mouseEntered(MouseEvent e)
  {
  }

  public void mouseExited(MouseEvent e)
  {
  }

  long lastClick = 0;

  public void mouseClicked(MouseEvent e)
  {
    if (checkForClick) {
      //  handle click  handling here...
      int modifiers = e.getModifiers();
      // System.out.println("Clicked Mods = " + modifiers);
      if ((modifiers & (MouseEvent.CTRL_MASK | MouseEvent.BUTTON2_MASK | MouseEvent.BUTTON3_MASK)) != 0)
      {
        // zoom out
        if (System.currentTimeMillis() - lastClick < 300) {
          minPt = 0;
          maxPt = 1;
        }
        else {
          double r1 = e.getX()/(double)getWidth();
          double newMinPt = minPt + (maxPt - minPt)*(r1 - .625);
          double newMaxPt = minPt + (maxPt - minPt)*(r1 + .625);
          minPt = newMinPt;
          maxPt = newMaxPt;
          if (minPt < 0) {
            maxPt += -minPt;
            minPt = 0;
          }
          else if (maxPt > 1) {
            minPt -= maxPt-1;
            maxPt = 1;
          }
          if (minPt < 0)
            minPt = 0;
          if (maxPt > 1)
            maxPt = 1;
        }
        needsRefresh = true;
      }
      else {
        // zoom in
        double r1 = e.getX()/(double)getWidth();
        double newMinPt = minPt + (maxPt - minPt)*(r1 - .375);
        double newMaxPt = minPt + (maxPt - minPt)*(r1 + .375);
        minPt = newMinPt;
        maxPt = newMaxPt;
        needsRefresh = true;
        if (minPt < 0)
          minPt = 0;
        if (maxPt > 1)
          maxPt = 1;
      }
      lastClick = System.currentTimeMillis();
}
    // System.out.println("Graph Click");
  }

  long  pressTimeMillis = 0;

  public void mousePressed(MouseEvent e)
  {
    int modifiers = e.getModifiers();
    SetProperCursor(e);
    boolean isGrab = (modifiers & MouseEvent.ALT_MASK) != 0;
    if (itsApp.currentTool == JSydApp.TB_SELECT && !isGrab) {
      // System.out.println("Pressed Mods = " + modifiers);
      rectAnchor = e.getPoint();
      rectPoint = rectAnchor;
      rectDragging = true;
      checkForClick = false;
      pressTimeMillis = System.currentTimeMillis();
    }
    else if (isGrab || itsApp.currentTool == JSydApp.TB_GRABSCROLL) {
      grabDragging = true;
      rectPoint = e.getPoint();
    }
  }

  public void mouseReleased(MouseEvent e)
  {
    int modifiers = e.getModifiers();
    SetProperCursor(e);
    if (rectDragging) {
      rectDragging = false;
      if (rectAnchor.equals(rectPoint) ||
          (modifiers & (MouseEvent.CTRL_MASK | MouseEvent.BUTTON2_MASK | MouseEvent.BUTTON3_MASK)) != 0 ||
          (Math.abs(rectPoint.x - rectAnchor.x) < 20 && System.currentTimeMillis() - pressTimeMillis < 500))
      {
        checkForClick = true;
      }
      else {
        // do zoom in based on rectangle...
        double r1 = rectAnchor.getX()/(double)getWidth();
        double r2 = rectPoint.getX()/(double)getWidth();
        if (r1 > r2) {
          // swap 'em
          double r3 = r1;
          r1 = r2;
          r2 = r3;
        }
        double newMinPt = minPt + (maxPt - minPt)*r1;
        double newMaxPt = minPt + (maxPt - minPt)*r2;
        minPt = newMinPt;
        maxPt = newMaxPt;
        needsRefresh = true;
      }
      repaint();
    }
  }

  public void doFullRefresh()
  {
    needsRefresh = true;
    repaint();
  }

  public void InitWave(PatchOwner po)
  {
    this.patchOwner = po;
    nbrSamples = 0;
    sampleRate = ((SSOutput) po.mainInst.GetOutputModule()).sampleRate;
    minPt = 0;
    maxPt = 1;
    doFullRefresh();
  }

  public void GraphSamples(PatchOwner po, int nbrSamples)
  {
    this.patchOwner = po;
    this.nbrSamples = nbrSamples;
    if (PatchOwner.gVerbose > 1)
      System.out.println("Graphing Samples: " + nbrSamples);
    sampleRate = ((SSOutput) po.mainInst.GetOutputModule()).sampleRate;
    doFullRefresh();
  }

  Timer timer;

  public void ScheduleFutureUpdate(long millis) {
        timer = new Timer();
        timer.schedule(new RepaintTask(), millis);
  }

  class RepaintTask extends TimerTask {
      public void run() {
        Timer t = timer;
        repaint();
        t.cancel(); //Terminate the timer thread
      }
  }

}