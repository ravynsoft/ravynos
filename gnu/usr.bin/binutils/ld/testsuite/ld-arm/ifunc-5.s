	.macro	define,name
	.type	\name,%gnu_indirect_function
\name:
	mov	pc,lr
	.size	\name,.-\name
	.endm

	.macro	test_relocs,name
	bl	\name
	ldr	r4,1f
	ldr	r4,2f
1:
	.word	\name(GOT)
2:
	.word	\name(GOT_PREL)
	.endm

	.global	f2

	.global	f3
	.hidden	f3

	define	f1
	define	f2
	define	f3

	.globl	_start
_start:
	test_relocs foo
	test_relocs f1
	test_relocs f2
	test_relocs f3
	.size	_start,.-_start

	.data
foo:
	.word	0x11223344
	.word	__irel_start
	.word	__irel_end
