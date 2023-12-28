	.data
	.type	bar, @object
bar:
	.byte	1
	.size	bar, .-bar
	.globl	foo
	.type	foo, @object
foo:
	.byte	1
	.size	foo, .-foo
	.text
	.globl	_start
	.type	_start, @function
_start:
	adcl	bar@GOTPCREL(%rip), %eax
	addl	bar@GOTPCREL(%rip), %ebx
	andl	bar@GOTPCREL(%rip), %ecx
	cmpl	bar@GOTPCREL(%rip), %edx
	orl	bar@GOTPCREL(%rip), %esi
	sbbl	bar@GOTPCREL(%rip), %edi
	subl	bar@GOTPCREL(%rip), %ebp
	xorl	bar@GOTPCREL(%rip), %r8d
	testl	%r15d, bar@GOTPCREL(%rip)
	adcq	bar@GOTPCREL(%rip), %rax
	addq	bar@GOTPCREL(%rip), %rbx
	andq	bar@GOTPCREL(%rip), %rcx
	cmpq	bar@GOTPCREL(%rip), %rdx
	orq	bar@GOTPCREL(%rip), %rdi
	sbbq	bar@GOTPCREL(%rip), %rsi
	subq	bar@GOTPCREL(%rip), %rbp
	xorq	bar@GOTPCREL(%rip), %r8
	testq	%r15, bar@GOTPCREL(%rip)
	adcl	foo@GOTPCREL(%rip), %eax
	addl	foo@GOTPCREL(%rip), %ebx
	andl	foo@GOTPCREL(%rip), %ecx
	cmpl	foo@GOTPCREL(%rip), %edx
	orl	foo@GOTPCREL(%rip), %esi
	sbbl	foo@GOTPCREL(%rip), %edi
	subl	foo@GOTPCREL(%rip), %ebp
	xorl	foo@GOTPCREL(%rip), %r8d
	testl	%r15d, foo@GOTPCREL(%rip)
	adcq	foo@GOTPCREL(%rip), %rax
	addq	foo@GOTPCREL(%rip), %rbx
	andq	foo@GOTPCREL(%rip), %rcx
	cmpq	foo@GOTPCREL(%rip), %rdx
	orq	foo@GOTPCREL(%rip), %rdi
	sbbq	foo@GOTPCREL(%rip), %rsi
	subq	foo@GOTPCREL(%rip), %rbp
	xorq	foo@GOTPCREL(%rip), %r8
	testq	%r15, foo@GOTPCREL(%rip)
	.size	_start, .-_start
