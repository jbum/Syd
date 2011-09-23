import java.awt.Color;


public class LinkDesc
{
	String desc;
	String varName;
  Color  clr;
	public LinkDesc(String desc, String varName)
	{
		this.desc = desc;
		this.varName = varName;
    this.clr = new Color(255,0,0);
	}
  public LinkDesc(String desc, String varName, Color clr)
  {
    this.desc = desc;
    this.varName = varName;
    this.clr = clr;
  }
}
