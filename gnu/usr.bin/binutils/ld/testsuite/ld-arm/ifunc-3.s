	.macro	define,name
	.text
	.type	\name,%gnu_indirect_function
\name:
	mov	pc,lr
	.size	\name,.-\name
	.endm

	.macro	test_relocs,name
	bl	\name(PLT)
	ldr	r4,1f
	ldr	r4,2f
1:
	.word	\name(GOT)
2:
	.word	\name(GOT_PREL)

	.data
	.word	\name
	.word	\name - .
	.text
	.endm

	.globl	f2

	.globl	f3
	.hidden	f3

	.globl	f4
	.protected f4

	define	f1
	define	f2
	define	f3
	define	f4

	.data
foo:
	.word	0x11223344

	.text
	.globl	arm
arm:
	test_relocs foo
	test_relocs f1
	test_relocs f2
	test_relocs f3
	test_relocs f4
	.size	arm,.-arm
