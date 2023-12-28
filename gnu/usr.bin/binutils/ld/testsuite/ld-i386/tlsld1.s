	.text
	.globl _start
_start:
	leal	foo@TLSLDM(%ebx), %eax
	call	___tls_get_addr
	.globl foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
