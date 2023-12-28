	.text
bar:
	retl
	nop

	.align	1024
	.section ".wdispcall"
foo:
	nop
	call	bar
	or	%r1, %g0, %o7
	nop
	call	bar
	nop
