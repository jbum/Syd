import javax.swing.*;
import java.awt.*;
import java.lang.reflect.*;

public class ParamDef_Float extends ParamDef
{
	String 	label;
  JTextField tFld;

 public ParamDef_Float(String varName, String label, String defaultValue)
  {
    super(varName, ParamDef.PT_Float, defaultValue);
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
      fld.setDouble(node, Double.parseDouble(getCurValue()));
  }

  void loadCurValueFromField(Field fld, SSModule node)  throws IllegalAccessException
  {
    double  v = fld.getDouble(node);
    setCurValue(Double.toString(v));
  }


}