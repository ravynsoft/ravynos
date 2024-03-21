	bclr    d0, #3
	bset    d1, #4
	btgl    d2, #5
	bclr    d3, d5
	bset    d4, d6
	btgl    d5, d7
	bclr.b  (34,x), #2
	bclr.w  (s+), #12
	bclr.l  (56,s), d7
	bset.b  [34,x], #5
	bset.l  (-s), #29
	bset.w  (156,x), d7
	btgl.b  [34,x], #5
	btgl.w  (-s), #15
	btgl.l  (15,p), d7

