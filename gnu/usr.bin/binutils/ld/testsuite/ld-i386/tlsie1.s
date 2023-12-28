	.text
	.globl _start
_start:
	addl	foo@gotntpoff(%ebx), %ecx
	addl	foo@indntpoff, %ecx
	movl	foo@gotntpoff(%ebx), %eax
	movl	foo@gotntpoff(%ebx), %ecx
	movl	foo@indntpoff, %eax
	movl	foo@indntpoff, %ecx
	.globl foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
