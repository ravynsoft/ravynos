	.global __tls_get_addr,__tls_get_addr_opt,gd,ld
	.type __tls_get_addr,@function
	.type __tls_get_addr_opt,@function

	.text
__tls_get_addr:
__tls_get_addr_opt:
	blr
	.size __tls_get_addr,. - __tls_get_addr
	.size __tls_get_addr_opt,. - __tls_get_addr_opt

	.section ".tbss","awT",@nobits
	.p2align 2
gd:	.space 4

	.section ".tdata","awT",@progbits
	.p2align 2
ld:	.long 0xc0ffee
