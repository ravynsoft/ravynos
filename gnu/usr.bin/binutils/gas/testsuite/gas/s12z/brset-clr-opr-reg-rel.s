	brclr.w (34,y), d2, *+3034
L1:	brset.b (-y),   d0, *+434
	brset.l (+x),   d3, *+2134
	brclr.l [34,x], d4, L1

