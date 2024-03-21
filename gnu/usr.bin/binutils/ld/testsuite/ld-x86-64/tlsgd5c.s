	.text
	.globl _start
_start:
	.byte	0x66
	leaq	foo@TLSGD(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
