import javax.swing.*;
import java.awt.*;
import java.lang.reflect.*;

public class ParamDef_Integer extends ParamDef
{
	String 	label;
  JTextField tFld;

  public ParamDef_Integer(String varName, String label, String defaultValue)
  {
    super(varName, ParamDef.PT_Integer, defaultValue);
    this.label = label;
  }

  JComponent getComponent()
  {
    JPanel  pnl = new JPanel();
    JLabel  lbl = new JLabel(label);
    tFld = new JTextField(defaultValue,20);
    pnl.add(lbl);
    lbl.setLabelFor(tFld);
    pnl.add(tFld);
    lbl.setPreferredSize(new Dimension(100,16));
    return pnl;
  }

  String getCurValue()
  {
    return tFld.getText();
  }

  void setCurValue(String val)
  {
    tFld.setText(val);
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