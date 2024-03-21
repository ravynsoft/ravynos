	.text
	.p2align 4
	.globl	_start
	.type	_start, @function
_start:
	ret
	.size	_start, .-_start
	.globl	ptr
	.section	.rodata
	.align 8
	.type	ptr, @object
	.size	ptr, 8
ptr:
	.quad	foo
	.section	.note.GNU-stack,"",@progbits
