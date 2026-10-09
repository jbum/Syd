/* SS Journal (misc notes)

Risset Bell Instrument Notes:

	Experimented with different drop off rates for envelope.
	(1-t)^2		(nicer, used for bell)
	(1/r)^t		(too fast)

	((fact^(1/len))^(t*len))*(1-t)
	
	This allows you to control rate of decay, from linear to exp:
	fact = 1 for linear, < 1 (and greater than 0) for exp.

	Question: Although the CSound version of risset patch is using a curve with very 
	quick dropoff, the resultant sound has a near linear partial envelope.  
	SS has to use a near linear envelope to achieve a similar sound.

	Examine simplified version of risset csound patch to see behavior
	of amplitude curve...
		
	Secondly, the SS sound has prominent phase cancellation nodes,
	which aren't present in the Risset sound.  This is caused by the frequency
	ratios of the 4 longer partials, which create beats of 1.0 hz and 1.7 hz, not sure
	why we aren't seeing these beats on Risset CSound file.
	
Hammer notes:
	Possibly bugged - still crashing occasionally.
	Possible tuning problems with table sine wave.
	Need to be able to control curve so it decays faster in the
	beginning, and then has a longer decay at end.

*/