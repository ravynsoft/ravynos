	.text
	.globl _start
_start:
	leal	foo@TLSLDM(%edi), %eax
	call	*___tls_get_addr@GOT(%edi)
	.globl foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
