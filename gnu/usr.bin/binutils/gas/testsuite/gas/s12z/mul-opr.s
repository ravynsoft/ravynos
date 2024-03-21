	muls.b d0, d1, #-1
	muls.w d3, d7, (-s)
	muls.l d3, d7, (+x)
	mulu.b d4, d1, (32,s)
	mulu.w d5, d3, [34,x]
	mulu.l d6, d7, (s+)
