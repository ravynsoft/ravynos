	divs.b  d0, d1, #98
	divs.b  d1, d2, d3
	divs.w  d2, d3, d4
	divs.l  d3, d7, #9842
	divu.b  d4, d1, (32,s)
	divu.w  d5, d3, [34,x]
	divu.l  d6, d7, (s+)
	divu.lp d7, [12,y], (7,d1)
