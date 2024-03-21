	.macro	loadtoc,sym
	.if	size == 32
	lwz	1,\sym(2)
	.else
	ld	1,\sym(2)
	.endif
	.endm

	.toc
LC01:	.tc	stuff[TC],stuff[RW]

	.globl	foo1
	.csect	foo1[pr]
foo1:
	.align	8
	nop
	loadtoc	LC01

	.globl	foo2
	.csect	foo2[pr]
foo2:
	nop
	loadtoc	LC01
	.ref	foo4 , foo6

	.globl	foo3
	.csect	foo3[pr]
foo3:
	nop
	loadtoc	LC01

	.globl	foo4
	.csect	foo4[pr]
foo4:
	nop
	loadtoc	LC01

	.globl	foo5
	.csect	foo5[pr]
foo5:
	nop
	loadtoc	LC01
	.ref	foo3

	.globl	foo6
	.csect	foo6[pr]
foo6:
	nop
	loadtoc	LC01

	.csect	foo1[pr]
	blr
	.ref	foo2

	.csect	stuff[rw]
stuff:
	.long	1
