L1:	brset   d2, #2, *+5
	brset   d3, #3, *+23
	brset   d3, #2, *-15
	brset   d2, #8, *-61
	brset   d2, #8, *-767
	brset   d6, #13, *-767
L2:	brclr   d2, #2, *+5
	brclr   d3, #3, *+23
	brclr   d3, #2, *-832
	brclr   d2, #8, *-61
	brclr   d2, #8, *-767

