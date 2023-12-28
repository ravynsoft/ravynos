	.syntax unified
	.arch armv6t2

	.macro	define,name,type
	.type	\name,%gnu_indirect_function
	\type
\name:
	mov	pc,lr
	.size	\name,.-\name
	.endm

	.macro	define2,name
	define	aa\name,.arm
	define	at\name,.arm
	define	ab\name,.arm
	define	ta\name,.thumb_func
	define	tt\name,.thumb_func
	define	tb\name,.thumb_func
	.endm

	.macro	test_relocs,name,width
	bl	\name(PLT)
	b\width \name
	beq\width \name
	ldr	r4,1f
	ldr	r4,2f
1:
	.word	\name(GOT)
2:
	.word	\name(GOT_PREL)
	.endm

	.macro	test_relocs2,name,type,width
	test_relocs a\type\name,\width
	test_relocs t\type\name,\width
	test_relocs ab\name,\width
	test_relocs tb\name,\width
	.endm

	.macro	diff,name
	.word	\name
	.word	\name-.
	.endm

	.macro	alldirs,doit,name
	\doit	aa\name
	\doit	at\name
	\doit	ab\name
	\doit	ta\name
	\doit	tt\name
	\doit	tb\name
	.endm

	define2	f1
	define2	f3

	alldirs	.globl,f3
	alldirs .hidden,f3

	.globl	arm
	.type	arm,%function
	.arm
arm:
	test_relocs foo
	test_relocs2 f1,a,
	test_relocs2 f3,a,
	.size	arm,.-arm

	.globl	_thumb
	.type	_thumb,%function
	.thumb_func
_thumb:
	test_relocs foo
	test_relocs2 f1,t,.w
	test_relocs2 f3,t,.w
	.size	_thumb,.-_thumb

	.data
foo:
	.word	0x11223344
	alldirs	diff,f1
	alldirs	diff,f3
