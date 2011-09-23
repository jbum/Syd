import javax.swing.*;
import java.awt.*;
import java.lang.reflect.*;

public class ParamDef_Checkbox extends ParamDef
{
  String     label;
  boolean    defaultSelection;
  JCheckBox  cbox;

  public ParamDef_Checkbox(String varName, String label, String defaultValue)
  {
    super(varName, ParamDef.PT_Checkbox, defaultValue);
    this.label = label;
    this.defaultSelection = Integer.parseInt(defaultValue) != 0;
  }

  JComponent getComponent()
  {
    JPanel  pnl = new JPanel();
    JLabel  lbl = new JLabel("");
    pnl.add(lbl);

    cbox = new JCheckBox(label, defaultSelection);
    pnl.add(cbox);

    // pnl.add(tFld);
    lbl.setPreferredSize(new Dimension(100,16));
    return pnl;
  }

  String getCurValue()
  {
    return cbox.isSelected()? "1" : "0";
  }

  void setCurValue(String val)
  {
    cbox.setSelected(Integer.parseInt(val) == 0? false : true);
  }

  void saveCurValueToField(Field fld, SSModule node) throws IllegalAccessException
  {
      fld.setInt(node, cbox.isSelected()? 1 : 0);
  }

  void loadCurValueFromField(Field fld, SSModule node)  throws IllegalAccessException
  {
    boolean isSelected = fld.getInt(node) != 0;
    setCurValue(isSelected? "1" : "0");
  }


}