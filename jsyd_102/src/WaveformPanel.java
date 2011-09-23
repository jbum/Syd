import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.*;
import javax.swing.*;

// Panel for editing a node...
public class WaveformPanel extends JPanel
{
  public static final long serialVersionUID = 1L;
  JLabel        label;
  GraphCanvas   itsCanvas;
  JSydApp       itsApp;

  // Graphing info

  public WaveformPanel(JSydApp itsApp, int pWidth, int pHeight)
  {
    this.itsApp = itsApp;
    BorderLayout layout = new BorderLayout();
    // SpringLayout layout = new SpringLayout();
    setLayout(layout); // try springlayout later...

    // setBackground(Color.lightGray);    // white background
    // setPreferredSize( new Dimension(pWidth, pHeight));
    label = new JLabel("Waveform");
    // label.setPreferredSize( new Dimension(100, 32));
    add(label,"North");
    // add(label);
    itsCanvas = new GraphCanvas(itsApp);
    // itsCanvas.setMinimumSize( new Dimension(200, 32));
    // itsCanvas.setPreferredSize( new Dimension(500, 100));
    // itsCanvas.setMaximumSize( new Dimension(1000, 600));
    // add(itsCanvas,"Center");
    add(itsCanvas);

    // Add canvas for drawing waveform
    // this.pack();
    itsCanvas.doFullRefresh();
  }

  public void InitWave(PatchOwner po)
  {
    itsCanvas.InitWave(po);
  }

  public void GraphSamples(PatchOwner po, int nbrSamples)
  {
    // System.out.println("Graph Samples: " + nbrSamples);
    itsCanvas.GraphSamples(po, nbrSamples);
  }

  public void SetGraphMode(int mode)
  {
    itsCanvas.graphMode = mode;
    itsCanvas.doFullRefresh();
  }

}
