	.syntax unified
	.arch armv6t2

	.macro	define,name,type
	.type	\name,%gnu_indirect_function
	\type
\name:
	mov	pc,lr
	.size	\name,.-\name
	.endm

	.macro	test_relocs,name,width
	bl\width \name
	b\width \name
	beq\width \name
	ldr	r4,1f
	ldr	r4,2f
1:
	.word	\name(GOT)
2:
	.word	\name(GOT_PREL)
	.endm

	.global	f1
	.global	f2

	.global	f3
	.global	f4
	.hidden	f3
	.hidden	f4

	define	f1,.arm
	define	f2,.thumb_func
	define	f3,.arm
	define	f4,.thumb_func

	.globl	_start
	.type	_start,%function
	.arm
_start:
	test_relocs foo
	test_relocs f1,
	test_relocs f2,
	.size	_start,.-_start

	.globl	_thumb
	.type	_thumb,%function
	.thumb_func
_thumb:
	test_relocs foo
	test_relocs f3,.w
	test_relocs f4,.w
	.size	_thumb,.-_thumb

	.data
foo:
	.word	0x11223344
	.word	__irel_start
	.word	__irel_end
