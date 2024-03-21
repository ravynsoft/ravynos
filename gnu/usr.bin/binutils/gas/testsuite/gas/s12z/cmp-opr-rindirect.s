;;; Indirect register offset
	cmp d2, [d1,y]
	cmp d3, [d1,y]
	cmp d4, [d0,y]
	cmp d5, [d0,y]
	cmp d0, [d6,x]
	cmp d1, [d6,x]
	cmp d6, [d6,x]
	cmp d7, [d7,x]
