	.syntax unified
	.arch armv6t2
	.text
text:
	.arm
	ldr	r0,1f
2:	blx	loc(tlscall)
	nop
	.p2align 2
1:	.word	loc(tlsdesc) + (. - 2b)

	.thumb
	ldr	r0,1f
2:	blx	loc(tlscall)
	nop
	.p2align 2
1:	.word	loc(tlsdesc) + (. - 2b + 1)

	.section ".foo","ax"
foo:
	.arm
	ldr	r0,1f
2:	blx	loc(tlscall)
	nop
	.p2align 2
1:	.word	loc(tlsdesc) + (. - 2b)

	.arm
	ldr	r0,1f
2:	blx	glob(tlscall)
	nop
	.p2align 2
1:	.word	glob(tlsdesc) + (. - 2b)

	.thumb
	ldr	r0,1f
2:	blx	loc(tlscall)
	nop
	.p2align 2
1:	.word	loc(tlsdesc) + (. - 2b + 1)

	.section .tdata,"awT"
	.type	loc, %object
loc:	.space	4

	@ glob1 and glob2 used by tls-longplt
	.type	glob1, %object
	.globl	glob1
glob1:	.space	4
	.type	glob2, %object
	.globl	glob2
glob2:	.space	4
