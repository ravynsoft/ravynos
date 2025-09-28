	.text
	lvlx %v0, %r1, %r2
	lvlx %v0, 0, %r2
	lvlxl %v0, %r1, %r2
	lvlxl %v0, 0, %r2
	lvrx %v0, %r1, %r2
	lvrx %v0, 0, %r2
	lvrxl %v0, %r1, %r2
	lvrxl %v0, 0, %r2

	stvlx %v0, %r1, %r2
	stvlx %v0, 0, %r2
	stvlxl %v0, %r1, %r2
	stvlxl %v0, 0, %r2
	stvrx %v0, %r1, %r2
	stvrx %v0, 0, %r2
	stvrxl %v0, %r1, %r2
	stvrxl %v0, 0, %r2

	ldbrx %r0, 0, %r1
	ldbrx %r0, %r1, %r2

	stdbrx %r0, 0, %r1
	stdbrx %r0, %r1, %r2

	dss	3
	dssall	
	dst	5,4,1
	dstt	8,7,0
	dstst	5,6,3
	dststt	4,5,2
