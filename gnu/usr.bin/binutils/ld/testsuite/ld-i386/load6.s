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
	adcl	bar@GOT(%ecx), %eax
	addl	bar@GOT(%ecx), %ebx
	andl	bar@GOT(%ecx), %ecx
	cmpl	bar@GOT(%ecx), %edx
	orl	bar@GOT(%ecx), %edi
	sbbl	bar@GOT(%ecx), %esi
	subl	bar@GOT(%ecx), %ebp
	xorl	bar@GOT(%ecx), %esp
	testl	%ecx, bar@GOT(%ecx)
	adcl	foo@GOT(%ecx), %eax
	addl	foo@GOT(%ecx), %ebx
	andl	foo@GOT(%ecx), %ecx
	cmpl	foo@GOT(%ecx), %edx
	orl	foo@GOT(%ecx), %edi
	sbbl	foo@GOT(%ecx), %esi
	subl	foo@GOT(%ecx), %ebp
	xorl	foo@GOT(%ecx), %esp
	testl	%ecx, foo@GOT(%ecx)
	.size	_start, .-_start
