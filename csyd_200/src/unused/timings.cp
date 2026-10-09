/* TImings


WORK:
	Time for shepsco
                    Before:	692.15
                    3 tabs:	643.60 (after 3 ftables added)

	Time for shep1
   					Original: 14.85
   					CosTab  : 14.10 (1 ftable)
   					2 Tabs  : 13.35
   					3 Tabs  : 12.28

	Time for shep4:
  				   Original:  86.10
  				   3 tabs:    71.77

			desc		shep1		shep4		shepsco (prelude)
			====		======		=====		========
						14.52		37.45		281.20 (prelude)

HOME:
			desc		shep1		shep4		shepsco
			====		======		=====		========
			before								1051.05
			3 tabs		14.53		47.77
			pow2		14.50
			] express	13.08		40.92
			pTime opt	16.00 		52.97
			??? (do indirect references cost more than stack pushes?)

			compiler opt 13.93		43.33
			minor opts	 13.15		41.05		903.50 (1/2 g)
					
*/