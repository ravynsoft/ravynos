	.syntax unified
	.arch armv6t2
	.text
text:
	.arm
	ldr	r0,1f
2:	blx	udefw(tlscall)
	nop
	.p2align 2
1:	.word	udefw(tlsdesc) + (. - 2b)

	.thumb
	ldr	r0,1f
2:	blx	udefw(tlscall)
	nop
	.p2align 2
1:	.word	udefw(tlsdesc) + (. - 2b + 1)

	.section ".foo","ax"
foo:
	.arm
	ldr	r0,1f
2:	blx	glob1(tlscall)
	nop
	.p2align 2
1:	.word	glob1(tlsdesc) + (. - 2b)

	.arm
	ldr	r0,1f
2:	blx	udefw(tlscall)
	nop
	.p2align 2
1:	.word	udefw(tlsdesc) + (. - 2b)

	.thumb
	ldr	r0,1f
2:	blx	udefw(tlscall)
	nop
	.p2align 2
1:	.word	udefw(tlsdesc) + (. - 2b + 1)

	.section .tdata,"awT"
	@ glob used by tls-longplt-lib
	.type	glob, %object
	.globl	glob
glob:	.space	4
	.globl  udefw
	.weak	udefw
