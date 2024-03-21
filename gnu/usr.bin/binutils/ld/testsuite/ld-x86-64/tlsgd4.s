	.text
	.globl _start
_start:
	leaq	foo@TLSGD(%rip), %rdi
	.word	0x6666
	rex64
	call	__tls_get_addr
	.globl foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
