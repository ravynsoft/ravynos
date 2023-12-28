;;; Direct register offset
	cmp d2, (d1,s)
	cmp d3, (d2,x)
	cmp d4, (d3,x)
	cmp d5, (d3,y)
	cmp d0, (d4,x)
	cmp d1, (d5,x)
	cmp d6, (d6,x)
	cmp d7, (d7,x)
