// Used to provide unit descriptions for user interface
// will eventually be initialized from text file.
//
// Info in here should tell us everything we need to know to construct
// patches, and load/save patch data with this unit.
import java.awt.*;
import javax.swing.*;

public class UnitDef
{
  static final int MaxInputs = 32;
  static final int MaxParams = 32;

	String	    name;
	String	    desc;
  int         iconID;
  int         labelIdx;
  int         itsType;
  LinkDesc[]  linkDesc = new LinkDesc[MaxInputs+1]; // list of incoming links, their names, descriptions and colors
  int         nbrSupportedLinks = 0;
  ParamDef[]  paramDef = new ParamDef[MaxParams+1];
  int         nbrSupportedParams = 0;
  // String      outputFormat = "";
  ParameterPanel  itsPanel;

  public UnitDef(int itsType, String name, String desc, int iconID, int labelIdx)
  {
    this.itsType = itsType;
    this.name = name;
    this.desc = desc;
    this.iconID = iconID;
    this.labelIdx  = labelIdx;

    DescribeLink(0, "Audio Signal", "sig", 0,0,0);
    for (int i = 1; i <= MaxInputs; ++i)
    {
      DescribeLink(i, "Undefined", "?", 255,255,255);
    }
    nbrSupportedLinks = 1;
    itsPanel = new ParameterPanel(800,100,desc);
  }

  int NameToSignalType(String name)
  {
    for (int i = 0; i < nbrSupportedLinks; ++i)
    {
      if (linkDesc[i] != null && linkDesc[i].varName.equals(name))
        return i;
    }
    return -1;
  }

  LinkDesc GetLinkDesc(int linkType)
  {
    if (linkType > nbrSupportedLinks)
      linkType = 0;
    return linkDesc[linkType];
  }

  void DescribeLink(int linkNbr, String desc, String varName, int r, int g, int b)
  {
    // System.out.println("link " + desc + " :" + r + ","+g+","+b);
    linkDesc[linkNbr] = new LinkDesc(desc, varName, new Color(r,g,b));
    if (linkNbr >= nbrSupportedLinks)
      nbrSupportedLinks = linkNbr + 1;
  }

  void AddParameter(ParamDef pd)
  {
    AddParameter(pd, 0);
  }

  void AddParameter(ParamDef pd, int flags)
  {
    // !! Add input element to itsPanel
    paramDef[nbrSupportedParams++] = pd;
    JComponent cmp = pd.getComponent();
    itsPanel.add(cmp);

    if ((flags & ParamDef.NOBREAK) != 0)
    {
      itsPanel.layout.putConstraint(SpringLayout.WEST, cmp,
                                   32,
                                    SpringLayout.EAST, itsPanel.lastCmp);
      itsPanel.layout.putConstraint(SpringLayout.NORTH, cmp,
                                    0,
                                    SpringLayout.NORTH, itsPanel.lastCmp);
    }
    else {
      itsPanel.layout.putConstraint(SpringLayout.WEST, cmp,
                                   8,
                                    SpringLayout.WEST, itsPanel);
      itsPanel.layout.putConstraint(SpringLayout.NORTH, cmp,
                                    itsPanel.lastYOffset,
                                    SpringLayout.SOUTH, itsPanel.lastCmp);
    }
    itsPanel.lastCmp = cmp;
    itsPanel.lastYOffset = 0;
  }

  void SaveParamsToNode(SSModule node)
  {
    // For each paramDef, save String version of value to node.itsParams
    for (int i = 0; i < nbrSupportedParams; ++i) {
      // node.itsParams[i] = paramDef[i].getCurValue(); // this may not be necessary...

      // save parameter to node...
      paramDef[i].saveCurValueToNode(node);

    }
  }

  void LoadParamsFromNode(SSModule node)
  {
    for (int i = 0; i < nbrSupportedParams; ++i) {
      // paramDef[i].setCurValue(node.itsParams[i]);
      // save parameter to node...

      // not sure about this one...
      // ????
      paramDef[i].loadCurValueFromNode(node);
    }
  }

//  void SetOutputFormat(String fmt)
//  {
//    outputFormat = fmt;
//  }

  void SaveToFile(SSModule node)
  {
  }

}