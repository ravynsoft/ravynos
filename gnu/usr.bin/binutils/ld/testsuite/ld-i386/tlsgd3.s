	.text
	.globl _start
_start:
	leal	foo@TLSGD(%ecx), %eax
	call	*___tls_get_addr@GOT(%ecx)
	leal	foo@TLSGD(%edx), %eax
	call	*___tls_get_addr@GOT(%edx)
	nop
	.globl foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
