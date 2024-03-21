	.text
	.globl _start
_start:
	leaq	foo@TLSGD(%rip), %rdi
	.word	0x6666
	rex64
	call	__tls_get_addr
