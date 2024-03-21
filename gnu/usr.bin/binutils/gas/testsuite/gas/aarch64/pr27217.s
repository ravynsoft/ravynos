	.arch armv8-a
	.file	"set.c"
	.text
	// Start of user assembly
	.set bar, 0x12345678
	// End of user assembly
	.align	2
	.p2align 4,,11
	.global	foo
	.type	foo, %function
foo:
	adrp	x0, bar
	add	x0, x0, :lo12:bar
	ret
	.size	foo, .-foo
	.ident	"GCC: (GNU) 10.2.1 20201030 (RTEMS 6, RSB "
