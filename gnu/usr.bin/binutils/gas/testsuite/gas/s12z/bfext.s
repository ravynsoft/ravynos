	bfext   d0, d1,       d2
	bfext   d1, d2,    #8:23
	bfext.b d2, (+x),     d3
	bfext.w d3, [123,x],  d4
	bfext.p d4, (d0, s),  d5
	bfext.l d5, (45,d0),  d2
	bfext.b (45,d0),  d6, d3
	bfext.w [45,x],   d7, d2
	bfext.b d0, (45,d1), #13:2
	bfext.w (45,d1), d7, #13:3
	bfext.p [451,x],   d7, d2

	bfins   d0, d1,       d2
	bfins   d1, d2,    #8:23
	bfins.b d2, (+x),     d3
	bfins.w d3, [123,x],  d4
	bfins.p d4, (d0, s),  d5
	bfins.l d5, (45,d0),  d2
	bfins.b (45,d0),  d6, d3
	bfins.w [45,x],   d7, d2
	bfins.b d0, (45,d1), #13:2
	bfins.w (45,d1), d7, #13:3
	bfins.p [451,x],   d7, d2
