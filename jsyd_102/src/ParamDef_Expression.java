import javax.swing.*;
import java.awt.*;
import java.lang.reflect.*;

public class ParamDef_Expression extends ParamDef
{
	String 	label;
  int     defLength = 32;
  JTextField tFld;

  public ParamDef_Expression(String varName, String label, String defaultValue)
  {
    super(varName, ParamDef.PT_Expression, defaultValue);
    this.label = label;
  }

  public ParamDef_Expression(String varName, String label, String defaultValue, int defLength)
  {
    super(varName, ParamDef.PT_Expression, defaultValue);
    this.label = label;
    this.defLength = defLength;
  }

  JComponent getComponent()
  {
    JPanel  pnl = new JPanel();
    JLabel  lbl = new JLabel(label);
    tFld = new JTextField(defaultValue,defLength);
//    pnl.setLayout(null);
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
      fld.set(node, node.InitExp(getCurValue()));
  }

  void loadCurValueFromField(Field fld, SSModule node)  throws IllegalAccessException
  {
    ExpRec expRec = (ExpRec) fld.get(node);
    setCurValue(expRec.exp);
  }

}