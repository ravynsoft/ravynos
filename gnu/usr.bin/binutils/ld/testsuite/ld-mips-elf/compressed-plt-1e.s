# Create a .word reference for every function under test.

	.abicalls
	.option pic2

	.include "compressed-plt-1.s"

	.macro	test_one, name, types
	.word	\name
	.endm

	.data
	test_all
