	.text
	.arm

foo:
	ldr	r0,1f
2:	blx	loc(tlscall)
	nop

	.p2align 2
1:	.word	loc(tlsdesc) + (. - 2b)

	.section .tdata,"awT"
loc:
	.space	4
