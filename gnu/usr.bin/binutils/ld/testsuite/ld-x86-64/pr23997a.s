	.text
	.p2align 3
	.globl foo_p
foo_p:
	.long foo@plt
	.section	.note.GNU-stack,"",@progbits
