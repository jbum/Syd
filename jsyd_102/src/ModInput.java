public class ModInput
{
  SSModule  owner;
	SSModule	link;
	int			  inputType;
	int			  destID;
	String		fromLabel;

	public ModInput(SSModule owner, SSModule link, int inputType, int destID, String fromLabel)
	{
    this.owner = owner;
		this.link = link;
		this.inputType = inputType;
		this.destID = destID;
		this.fromLabel = fromLabel;
    if (PatchOwner.gVerbose > 2)
      System.out.println("Assigning label: " + fromLabel);
	}

  public ModInput(ModInput mi) // copy constructor
  {
    this(mi.owner, mi.link, mi.inputType, mi.destID, mi.fromLabel);
  }
}
