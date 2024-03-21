	.text
	.globl	_start
	.type	_start, @function
_start:
	lea	foo@TLSDESC(%rip), %eax
	call	*foo@TLSCALL(%eax)
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	30
	.section	.note.GNU-stack,"",@progbits
