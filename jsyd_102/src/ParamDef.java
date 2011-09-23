// A description of a parameter (such as Frequency or Amplitude)
// that can be edited.
// These are attached to instances of the UnitDef class.
import java.awt.*;
import javax.swing.*;
import java.lang.reflect.*;

public abstract class ParamDef
{

  static final int NOBREAK = 1; // flags

  String  varName;
	int	    paramType;
  String  defaultValue;

  static final int PT_Integer   = 0; // An expression
	static final int PT_Float     = 1; // A floating point number
  static final int PT_String    = 2; // An expression
  static final int PT_Expression = 3; // An expression
  static final int PT_Filename   = 4; // A filespec
  static final int PT_Checkbox   = 5; // A filespec
  static final int PT_RadioGroup = 6; // A group of radio buttons
  static final int PT_PulldownGroup = 7; // A pulldown menu selection

  public ParamDef(String varName, int paramType, String defaultValue)
  {
    this.varName = varName;
    this.paramType = paramType;
    this.defaultValue = defaultValue;
  }

  String getDefault()
  {
    return defaultValue;
  }

  void saveCurValueToNode(SSModule node)
  {
    Class nc = node.getClass();
    Field[] fs = nc.getFields();
    for (int i = 0; i < fs.length; ++i)
    {
      if (fs[i].getName().equals(varName))
      {
        try {
          saveCurValueToField(fs[i], node);
        }
        catch (IllegalAccessException iae) {
           System.out.println(iae.toString());
        }
        return;
      }
    }
  }

  void loadCurValueFromNode(SSModule node)
  {
    Class nc = node.getClass();
    Field[] fs = nc.getFields();
    for (int i = 0; i < fs.length; ++i)
    {
      if (fs[i].getName().equals(varName))
      {
        try {
          loadCurValueFromField(fs[i], node);
        }
        catch (IllegalAccessException iae) {
           System.out.println(iae.toString());
        }
        return;
      }
    }
  }


  abstract String getCurValue();
  abstract void setCurValue(String val);
  abstract void saveCurValueToField(Field fld, SSModule node) throws IllegalAccessException;
  abstract void loadCurValueFromField(Field fld, SSModule node) throws IllegalAccessException;

  abstract JComponent getComponent();
}