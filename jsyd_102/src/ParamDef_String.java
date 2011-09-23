import javax.swing.*;
import java.awt.*;
import java.lang.reflect.*;

// This is functionally the same as ParamDef_Expression...

public class ParamDef_String extends ParamDef
{
	String 	label;
  int     defLength = 32;
  JTextField  tFld = null;

  public ParamDef_String(String varName, String label, String defaultValue)
  {
    super(varName, ParamDef.PT_String, defaultValue);
    this.label = label;
  }

  public ParamDef_String(String varName, String label, String defaultValue, int defLength)
  {
    super(varName, ParamDef.PT_String, defaultValue);
    this.label = label;
    this.defLength = defLength;
  }

  JComponent getComponent()
  {
    JPanel  pnl = new JPanel();
    JLabel  lbl = new JLabel(label);
    tFld = new JTextField(defaultValue,defLength);
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
      fld.set(node, getCurValue());
  }

  void loadCurValueFromField(Field fld, SSModule node)  throws IllegalAccessException
  {
      setCurValue((String) fld.get(node));
  }


}