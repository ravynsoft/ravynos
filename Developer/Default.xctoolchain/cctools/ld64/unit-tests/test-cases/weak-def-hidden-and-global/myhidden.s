	.data
	.globl	_myweak
	.private_extern _myweak
	.weak_definition _myweak
_myweak:
	.long	 0
	

	.text
	.align 2
#if __ARM_ARCH_7A__
	.code	16
	.thumb_func	_test
#endif

	.globl	_test
_test:
#if __x86_64__
	nop
	movl	_myweak(%rip), %eax
	ret
#elif __i386__
	call	L1
L1:	popl	%eax
	movl	_myweak-L1(%eax), %eax
	ret
#elif __arm__

#if __ARM_ARCH_7A__
	movw	r0, :lower16:(_myweak-(L4+4))
	movt	r0, :upper16:(_myweak-(L4+4))
L4:	add	r0, pc
	ldr	r0, [r0]
	bx	lr
#else
	ldr	r0, L2
L3:	ldr	r0, [pc, r0]
	bx	lr
	.align	2
L2:	.long	_myweak-(L3+8)
#endif


#endif

