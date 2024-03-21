	.text
	.globl	foo
	.type	foo, @function
foo:
	call bar@PLT
	.size	foo, .-foo

	.globl	gap
	.type	gap, @function
gap:
	jmp .L0
	.space 0x40000000, 0x90
.L0:
	jmp .L2
	.space 0x3fdfff14, 0x90
.L2:
	.size	gap, .-gap
	.section	.note.GNU-stack,"",@progbits
