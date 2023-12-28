	.text
	.globl _start
_start:
	leaq	foo@TLSGD(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
