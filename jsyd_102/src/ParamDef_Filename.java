import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import java.io.*;

public class ParamDef_Filename extends ParamDef implements ActionListener, FilenameFilter
{
	String 	label;
  boolean isOutSpec;  // is output filename spec (otherwise is input - use "Get")
  JTextField tFld;
  String[]    legalSuffixes = null;

  public ParamDef_Filename(String varName, String label,   String  defaultValue, boolean isOutSpec, String[] legalSuffixes)
  {
    super(varName, ParamDef.PT_Filename, defaultValue);
    this.label = label;
    this.isOutSpec = isOutSpec;
    this.legalSuffixes = legalSuffixes;
  }

  public ParamDef_Filename(String varName, String label,   String  defaultValue, boolean isOutSpec)
  {
    this(varName, label, defaultValue, isOutSpec, null);
  }

  JComponent getComponent()
  {
    JPanel  pnl = new JPanel();
    JLabel  lbl = new JLabel(label);
    JButton btn = new JButton("Select...");
    tFld = new JTextField(defaultValue,48);
    pnl.add(lbl);
    lbl.setLabelFor(tFld);
    pnl.add(tFld);
    pnl.add(btn);
    lbl.setPreferredSize(new Dimension(100,16));
    btn.addActionListener(this);
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

  public boolean accept(File dir, String name)
  {
    System.out.println("Accept: " + name);
    if (legalSuffixes == null)
      return true;
    for (int i = 0; i < legalSuffixes.length; ++i)
      if (name.toLowerCase().endsWith(legalSuffixes[i]))
        return true;
    System.out.println("FALSE");
    return false;
  }

  public void actionPerformed( ActionEvent event )
  {
    String  prompt = label + " to " + (isOutSpec? "Save" : "Load");
    FileDialog fd = new FileDialog(JSydApp.gApp, prompt, isOutSpec? FileDialog.SAVE : FileDialog.LOAD);
    fd.setFilenameFilter(this);
    fd.setFile(tFld.getText());
    fd.setVisible(true);
    // Set file filter here...
    if (fd.getFile() != null) {
      tFld.setText(fd.getDirectory()+fd.getFile());
    }
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