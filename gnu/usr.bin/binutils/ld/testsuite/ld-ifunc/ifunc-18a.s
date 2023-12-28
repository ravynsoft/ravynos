	.section .data.rel,"aw",@progbits
	.globl foo_ptr
	.type	foo_ptr, @object
foo_ptr:
	.dc.a foo
