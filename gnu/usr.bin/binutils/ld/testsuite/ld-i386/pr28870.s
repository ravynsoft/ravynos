	.text
	.globl _start
_start:
	addl	foo@GOT(%ebx), %eax
	cmpl	$0, foo@GOT(%ebx)
foo = 2
