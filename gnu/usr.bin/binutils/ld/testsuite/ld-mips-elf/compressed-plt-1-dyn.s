# Create dummy DSO functions for everything that these tests call.

	.abicalls
	.option pic2

	.set	filter, -1

	.macro	test_one, name, mask
	.globl	\name
	.ent	\name
\name:
	jr	$31
	.end	\name
	.endm

	.include "compressed-plt-1.s"

	test_all
