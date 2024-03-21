	.section __weak, __weak, coalesced

	.private_extern a
	.weak_definition a
a:	.space 1

	.globl b
	.weak_definition b
b:	.space 1
