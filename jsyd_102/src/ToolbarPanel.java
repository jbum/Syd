import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

public class ToolbarPanel extends JPanel implements  MouseListener
{
  public static final long serialVersionUID = 1L;

  static Image  tbImage;
  static final int  kNbrTools = 9;
  static final int tbWidth = 32;
  static final int tileWidth = 40;
  Vector items = new Vector();
  int   rightX = 0;

  ToolbarListener itsApp;

  {
   Toolkit toolkit = Toolkit.getDefaultToolkit();

    tbImage = toolkit.getImage(this.getClass().getResource("assets/toolbar_icons.png"));
    addMouseListener(this);
    setToolTipText("Toolbar"); // turn on tool tips

  }


  public ToolbarPanel(ToolbarListener itsApp)
  {
    this.itsApp = itsApp;
    setPreferredSize(new Dimension(tbWidth, (tbWidth+2)*kNbrTools));
  }

  public void add(ToolbarItem item)
  {
    item.bounds = new Rectangle(rightX, 0, tbWidth, tbWidth);
    rightX += tbWidth + 2;

    items.add(item);
    setPreferredSize(new Dimension((tbWidth+2)*items.size(), tbWidth));
  }

  public void addDivider()
  {
    rightX += 4;
  }

  public void paint(Graphics g)
  {
    g.setColor(new Color(0xEE,0xEE,0xEE)); // was 88,88,88
    g.fillRect (0, 0, getWidth(), getHeight());
    for (int i = 0; i < items.size(); ++i)
    {
      ToolbarItem ti = (ToolbarItem) items.elementAt(i);
      int iconNbr = ti.iconNbr + 2;

      int backX = ti.isDepressed? tileWidth : 0;
      int frontX = iconNbr*tileWidth;

      g.drawImage(tbImage, ti.bounds.x, ti.bounds.y, ti.bounds.x+ti.bounds.width, ti.bounds.y+ti.bounds.height, backX, 0, backX+tileWidth, tileWidth,this);
      g.drawImage(tbImage, ti.bounds.x, ti.bounds.y, ti.bounds.x+ti.bounds.width, ti.bounds.y+ti.bounds.height, frontX, 0, frontX+tileWidth, tileWidth,this);
    }
  }

  public void mouseEntered(MouseEvent e)
  {
  }

  public void mouseExited(MouseEvent e)
  {
  }

  public void mouseClicked(MouseEvent e)
  {
    Point mp = e.getPoint();
    for (int i = 0; i < items.size(); ++i)
    {
      ToolbarItem ti = (ToolbarItem) items.elementAt(i);
      if (ti.bounds.contains(mp))
      {
        itsApp.GetToolbarClick(ti.toolID);
        return;
      }
    }
  }

  public void mousePressed(MouseEvent e)
  {
  }

  public void mouseReleased(MouseEvent e)
  {
  }

  public String getToolTipText(MouseEvent e)
  {
    Point mp = e.getPoint();
    for (int i = 0; i < items.size(); ++i)
    {
      ToolbarItem ti = (ToolbarItem) items.elementAt(i);
      if (ti.bounds.contains(mp))
        return ti.name;
    }
    return "";
  }


}

