	.text
	.globl _start
_start:
	leal	foo@TLSGD(%edx), %eax
	call	*___tls_get_addr@GOT(%ecx)
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
