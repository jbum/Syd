import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.*;
import javax.swing.*;

// Panel for editing a node...
public class ParameterPanel extends JPanel
{
  public static final long serialVersionUID = 1L;
  SpringLayout  layout;
  JLabel        label;
  JComponent    lastCmp;
  int           lastYOffset;

  public ParameterPanel(int pWidth, int pHeight, String name)
  {
    layout = new SpringLayout();
    setLayout(layout); // try springlayout later...

    // setBackground(Color.lightGray);    // white background
    setPreferredSize( new Dimension(pWidth, pHeight));
    label = new JLabel(name);
    add(label);
    layout.putConstraint(SpringLayout.WEST, label,
                         8,
                          SpringLayout.WEST, this);
    lastCmp = label;
    lastYOffset = 8;
    repaint();
  }

/*  public void paint(Graphics g)
  {
     g.setColor(clr); // was 88,88,88
     g.fillRect (0, 0, getWidth(), getHeight());

  }
*/
}
