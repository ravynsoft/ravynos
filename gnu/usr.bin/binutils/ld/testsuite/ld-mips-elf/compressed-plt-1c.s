# Define a function with all direct (%lo) references.

	.abicalls
	.option	pic0

	.include "compressed-plt-1.s"

	.macro	test_one, name, types
	.if	(\types) & LO
	li	$2,%lo(\name)
	.endif
	.endm

	.if	micromips
	.set	micromips
	.endif

	.section .text.c, "ax", @progbits
	.globl	testlo
	.ent	testlo
	.set	noreorder
testlo:
	test_all
	.end	testlo
