	.text
_start:
	mov	foo@gottpoff(%rip), %rax
	mov	foo@gottpoff(%rip), %r12
	add	foo@gottpoff(%rip), %eax
	add	foo@gottpoff(%rip), %r12d
	lea	foo@tlsdesc(%rip), %eax
	lea	foo@tlsdesc(%rip), %r12d
	.globl	foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
