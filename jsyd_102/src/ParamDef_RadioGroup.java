import javax.swing.*;
import java.awt.*;
import java.lang.reflect.*;

public class ParamDef_RadioGroup extends ParamDef
{
  String    groupLabel;
	String[] 	radioLabels;
  int       defaultSelection;
  JRadioButton[]  btns;

  public ParamDef_RadioGroup(String varName, String groupLabel, String[] radioLabels, String defaultSelection)
  {
    super(varName, ParamDef.PT_RadioGroup, defaultSelection);
    this.groupLabel = groupLabel;
    this.radioLabels = radioLabels;
    this.defaultSelection = Integer.parseInt(defaultSelection);
    btns = new JRadioButton[radioLabels.length];
  }

  JComponent getComponent()
  {
    JPanel  pnl = new JPanel();
    JLabel  lbl = new JLabel(groupLabel);
    pnl.add(lbl);

    ButtonGroup group = new ButtonGroup();
    for (int i = 0; i < radioLabels.length; ++i)
    {
      btns[i] = new JRadioButton(radioLabels[i]);
      btns[i].setSelected(i == defaultSelection);
      pnl.add(btns[i]);
      group.add(btns[i]);
    }
    // pnl.add(group);

    // pnl.add(tFld);
    lbl.setPreferredSize(new Dimension(100,16));
    return pnl;
  }


  String getCurValue()
  {
    for (int i = 0; i < radioLabels.length; ++i)
    {
      if (btns[i].isSelected())
        return Integer.toString(i);
    }
    return "0";
  }

  void setCurValue(String val)
  {
    btns[Integer.parseInt(val)].setSelected(true);
  }

  void saveCurValueToField(Field fld, SSModule node) throws IllegalAccessException
  {
      fld.setInt(node, Integer.parseInt(getCurValue()));
  }

  void loadCurValueFromField(Field fld, SSModule node)  throws IllegalAccessException
  {
    int  v = fld.getInt(node);
    setCurValue(Integer.toString(v));
  }



}