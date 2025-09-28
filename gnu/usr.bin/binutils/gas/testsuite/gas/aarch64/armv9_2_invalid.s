	.arch	armv9.2-a
	cpyfp	[x0]!, [x1]!, x30!
	cpyfm	[x0]!, [x1]!, x30!
	cpyfe	[x0]!, [x1]!, x30!
1:
	bc.eq	1b
