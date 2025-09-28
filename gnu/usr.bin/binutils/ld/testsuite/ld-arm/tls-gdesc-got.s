
	.arm
foo:
	ldr	r0,1f
2:	bl	loc1(tlscall)
	nop
1:	.word	loc1(tlsdesc) + (. - 2b)

	ldr	r0,1f
2:	bl	loc2(tlscall)
	nop
1:	.word	loc2(tlsdesc) + (. - 2b)

	ldr	r0,1f
2:	bl	glob1(tlscall)
	nop
1:	.word	glob1(tlsdesc) + (. - 2b)

	ldr	r0,1f
2:	bl	glob2(tlscall)
	nop
1:	.word	glob2(tlsdesc) + (. - 2b)

	ldr	r0,1f
2:	bl	ext1(tlscall)
	nop
1:	.word	ext1(tlsdesc) + (. - 2b)

	ldr	r0,1f
2:	bl	ext2(tlscall)
	nop
1:	.word	ext2(tlsdesc) + (. - 2b)

	.section	.tdata,"awT",%progbits
	.space	8
	.type	loc1, %object
loc1:	.space	4
	.type	loc2, %object
loc2:	.space	4
	.globl	glob1
	.type	glob1, %object
glob1:	.space	4
	.globl	glob2
	.type	glob2, %object
glob2:	.space	4
