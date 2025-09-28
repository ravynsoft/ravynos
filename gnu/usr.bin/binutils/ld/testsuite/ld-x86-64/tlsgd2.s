	.text
	.globl _start
_start:
	leaq	foo@TLSGD(%rip), %rdi
	call	__tls_get_addr
