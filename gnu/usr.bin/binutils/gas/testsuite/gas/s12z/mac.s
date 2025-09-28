	macs.b  d0, d1, #98
	macs.b  d1, d2, d3
	macs.w  d2, d3, d4
	macs.l  d3, d7, #9842
	macu.b  d4, d1, (32,s)
	macu.w  d5, d3, [34,x]
	macu.l  d6, d7, (s+)
	macu.lp d7, [12,y], (7,d1)
