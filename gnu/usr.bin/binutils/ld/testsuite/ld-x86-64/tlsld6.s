	.text
	.globl _start
_start:
	leaq    foo@TLSLD(%rip), %rdi
	call    *__tls_get_addr@GOTPCREL(%rip)
	.globl foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
