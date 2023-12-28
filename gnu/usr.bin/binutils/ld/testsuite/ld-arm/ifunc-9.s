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
	ldr	r4,3f
	ldr	r4,4f
	ldr	r5,5f
1:
	.word	\name
2:
	.word	\name-.
3:
	.word	\name(GOTOFF)
4:
	.word	\name(GOT)
5:
	.word	\name(GOT_PREL)
	.endm

	.global	f3
	.hidden	f3

	define	f1
	# f2 provided by ifunc-3.so
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
