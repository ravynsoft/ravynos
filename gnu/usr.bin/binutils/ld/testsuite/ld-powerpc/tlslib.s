	.global __tls_get_addr,__tls_get_addr_opt,gd,ld
	.type __tls_get_addr,@function
	.type __tls_get_addr_opt,@function

	.section ".opd","aw",@progbits
__tls_get_addr:
__tls_get_addr_opt:
	.align 3
	.quad	.L.__tls_get_addr
	.quad	.TOC.@tocbase
	.quad	0

	.section ".tbss","awT",@nobits
	.align 3
gd:	.space 8

	.section ".tdata","awT",@progbits
	.align 2
ld:	.long 0xc0ffee

	.text
.L.__tls_get_addr:
	blr
	.size __tls_get_addr,. - .L.__tls_get_addr
