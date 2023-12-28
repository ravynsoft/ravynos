	qmuls.b d0, d1, #98
	qmuls.b d1, d2, d3
	qmuls.w d2, d3, d4
	qmuls.l d3, d7, #9842
	qmulu.b d4, d1, (32,s)
	qmulu.w d5, d3, [34,x]
	qmulu.l d6, d7, (s+)
	qmulu.lp d7, [12,y], (7,d1)
