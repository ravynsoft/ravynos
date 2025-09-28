// Test instructions with opertional operand or other optional element.

.text
	dcps1	#15
	dcps1	#0
	dcps1
	dcps2	#31
	dcps2	#0
	dcps2
	dcps3	#63
	dcps3	#0
	dcps3

	ret	x7
	ret	x30
	ret

	clrex	#0
	clrex	#9
	clrex	#15
	clrex

	sys	#0, c0, c0, #0

	// Optional leading # for symbolic load/store offsets.
	adr	x0, sym
	ldr	x1,[x0,:lo12:sym]
	ldr	x1,[x0,#:lo12:sym]
	str	x1,[x0,:lo12:sym]
	str	x1,[x0,#:lo12:sym]
