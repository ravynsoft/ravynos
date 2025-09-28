	.text
	.globl _start
_start:
	mov	foo@GOTTPOFF(%rip), %eax
	add	foo@GOTTPOFF(%rip), %eax
	mov	foo@GOTTPOFF(%rip), %r8d
	add	foo@GOTTPOFF(%rip), %r8d
	mov	foo@GOTTPOFF(%rip), %r12d
	add	foo@GOTTPOFF(%rip), %r12d
	.globl	foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
