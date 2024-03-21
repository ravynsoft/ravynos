	.text
	.globl _start
_start:
	leaq	foo@TLSGD(%rip), %rdi
	call	__tls_get_addr
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
