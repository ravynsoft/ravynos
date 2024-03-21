	.macro	loadtoc,sym
	.if	size == 32
	lwz	1,\sym(2)
	.else
	ld	1,\sym(2)
	.endif
	.endm

	.toc
LC01:	.tc	indirect1[TC],indirect1[RW]
LC02:	.tc	block[TC],block[RW]

	.csect	.unused_local[PR]
.unused_local:
	bl	.unused_global

	.globl	.init_function
	.csect	.init_function[PR]
.init_function:
	loadtoc	LC01

	.globl	.fini_function
	.csect	.fini_function[PR]
.fini_function:
	loadtoc	LC02

	.globl	.unused_global
	.csect	.unused_global[PR]
.unused_global:
	bl	.unused_local

	.globl	.exported_global
	.csect	.exported_global[PR]
.exported_global:
	bl	.indirect2

	.globl	.indirect1
	.csect	.indirect1[PR]
.indirect1:
	lwz	8,4(8)

	.csect	.indirect2[PR]
.indirect2:
	lwz	8,8(8)

	.globl	.indirect3
	.csect	.indirect3[PR]
.indirect3:
	lwz	8,12(8)

	.globl	block
	.csect	block[RW]
block:
	.long	indirect3
	.long	0x11223344
