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
	movl	bar@GOT(%ecx), %eax
	adcl	bar@GOT(%ecx), %eax
	addl	bar@GOT(%ecx), %ebx
	andl	bar@GOT(%ecx), %ecx
	cmpl	bar@GOT(%ecx), %edx
	orl	bar@GOT(%ecx), %edi
	sbbl	bar@GOT(%ecx), %esi
	subl	bar@GOT(%ecx), %ebp
	xorl	bar@GOT(%ecx), %esp
	testl	%ecx, bar@GOT(%ecx)
	movl	bar@GOT, %eax
	adcl	bar@GOT, %eax
	addl	bar@GOT, %ebx
	andl	bar@GOT, %ecx
	cmpl	bar@GOT, %edx
	orl	bar@GOT, %edi
	sbbl	bar@GOT, %esi
	subl	bar@GOT, %ebp
	xorl	bar@GOT, %esp
	testl	%ecx, bar@GOT
	movl	foo@GOT(%ecx), %eax
	adcl	foo@GOT(%ecx), %eax
	addl	foo@GOT(%ecx), %ebx
	andl	foo@GOT(%ecx), %ecx
	cmpl	foo@GOT(%ecx), %edx
	orl	foo@GOT(%ecx), %edi
	sbbl	foo@GOT(%ecx), %esi
	subl	foo@GOT(%ecx), %ebp
	xorl	foo@GOT(%ecx), %esp
	testl	%ecx, foo@GOT(%ecx)
	movl	foo@GOT, %eax
	adcl	foo@GOT, %eax
	addl	foo@GOT, %ebx
	andl	foo@GOT, %ecx
	cmpl	foo@GOT, %edx
	orl	foo@GOT, %edi
	sbbl	foo@GOT, %esi
	subl	foo@GOT, %ebp
	xorl	foo@GOT, %esp
	testl	%ecx, foo@GOT
	.size	_start, .-_start
