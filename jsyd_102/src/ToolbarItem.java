import java.awt.*;

public class ToolbarItem
{
	String 	name; 	// shows up in tooltip
	int	toolID;	// returned when clicked on
	int	iconNbr; // used for rendering
	boolean isDepressed;
  Rectangle bounds;

	public ToolbarItem(String name, int toolID, int iconNbr, boolean isDepressed)
	{
    this.name = name;
    this.toolID = toolID;
    this.iconNbr = iconNbr;
    this.isDepressed = isDepressed;
	}
}