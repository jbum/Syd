public class NoteEvent implements Comparable
{
  static final int NEF_Tempo = 1;
	int		flags;		  // might be tempo or wave table event
	int		p1;
	double		p2,p3;	// start, duration
	double[]	op;	    // P4 thru N

  public int compareTo(Object o)
  {
    NoteEvent that = (NoteEvent) o;
    double val = this.p2 - that.p2;
    if (val < 0)
      return -1;
    else if (val > 0)
      return 1;
    else
      return 0;
  }

}
