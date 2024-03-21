# Source file used to test the pseudo instructions.

foo:
	mov	r1, r2
	nop
	call	0x400
	call	r10
	ret
	wbc	r12.w1, 12
	wbs	r12, r1
