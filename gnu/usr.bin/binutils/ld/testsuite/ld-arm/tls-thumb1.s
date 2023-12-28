	.arch armv4t
	.syntax unified
	.text
text:
	.arm
	ldr	r0,1f
2:	bl	+loc(tlscall)
	nop
	.p2align 2
1:	.word	loc(tlsdesc) + (. - 2b)

	.thumb
	ldr	r0,1f
2:	bl	+loc(tlscall)
	nop
	.p2align 2
1:	.word	loc(tlsdesc) + (. - 2b + 1)

	.section ".foo","ax"
foo:
	.arm
	ldr	r0,1f
2:	bl	+loc(tlscall)
	nop
	.p2align 2
1:	.word	loc(tlsdesc) + (. - 2b)

	.arm
	ldr	r0,1f
2:	bl	+glob(tlscall)
	nop
	.p2align 2
1:	.word	glob(tlsdesc) + (. - 2b)

	.thumb
	ldr	r0,1f
2:	bl	+loc(tlscall)
	nop
	.p2align 2
1:	.word	loc(tlsdesc) + (. - 2b + 1)

	.section .tdata,"awT"
loc:	.space	4
