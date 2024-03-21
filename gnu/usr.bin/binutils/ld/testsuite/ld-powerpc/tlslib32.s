	.global __tls_get_addr,__tls_get_addr_opt,gd,ld
	.type __tls_get_addr,@function
	.type __tls_get_addr_opt,@function

	.section ".tbss","awT",@nobits
	.align 2
gd:	.space 4

	.section ".tdata","awT",@progbits
	.align 2
ld:	.long 0xc0ffee

	.text
__tls_get_addr:
__tls_get_addr_opt:
	blr
