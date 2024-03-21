# Create a GOT reference for every function under test.

	.abicalls
	.option pic2

	.include "compressed-plt-1.s"

	.macro	test_one, name, types
	lw	$2,%got(\name)($gp)
	.endm

	.if	micromips
	.set	micromips
	.endif

	.section .text.d, "ax", @progbits
	.globl	testgot
	.ent	testgot
	.set	noreorder
testgot:
	test_all
	.end	testgot
