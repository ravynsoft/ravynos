# jal relocs against undefined weak symbols should not be treated as
# overflowing

	.globl	start
	.type	start, @function
	.weak	foo
start:
	jal	foo
