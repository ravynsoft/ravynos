	.syntax unified
	.arch armv6t2

	.macro	define,name,type
	.type	\name,%gnu_indirect_function
	\type
\name:
	mov	pc,lr
	.size	\name,.-\name
	.endm

	.macro	test_relocs,name
	ldr	r4,1f
1:
	.word	\name
	.endm

	.global	f2
	.global	f2t

	.global	f3
	.hidden	f3
	.global	f3t
	.hidden	f3t

	define	f1,.arm
	define	f2,.arm
	define	f3,.arm

	define	f1t,.thumb_func
	define	f2t,.thumb_func
	define	f3t,.thumb_func

	.globl	_start
_start:
	test_relocs foo
	test_relocs f1
	test_relocs f2
	test_relocs f3
	test_relocs f1t
	test_relocs f2t
	test_relocs f3t
	.size	_start,.-_start

	.data
foo:
	.word	0x11223344
	.word	__irel_start
	.word	__irel_end
