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
	ldr	r4,3f
	ldr	r4,4f
	ldr	r5,5f
	movw	r4,#:lower16:\name
	movt	r4,#:upper16:\name
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

	define	f1,.arm
	define	f2,.thumb_func
	define	f3,.arm
	define	f4,.thumb_func

	.globl	f5
	.globl	f6
	.globl	f7
	.globl	f8

	define	f5,.arm
	define	f6,.thumb_func
	define	f7,.arm
	define	f8,.thumb_func

	.globl	_start
	.type	_start,%function
	.arm
_start:
	test_relocs foo
	test_relocs f1,
	test_relocs f2,
	test_relocs f5,
	test_relocs f6,
	.size	_start,.-_start

	.globl	_thumb
	.type	_thumb,%function
	.thumb_func
_thumb:
	test_relocs foo
	test_relocs f3,.w
	test_relocs f4,.w
	test_relocs f7,.w
	test_relocs f8,.w
	.size	_thumb,.-_thumb

	.data
foo:
	.word	0x11223344
