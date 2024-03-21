	.global __tls_get_addr,__tls_get_addr_opt,gd,ld
	.global .__tls_get_addr,.__tls_get_addr_opt
	.type .__tls_get_addr,@function
	.type .__tls_get_addr_opt,@function

	.section ".opd","aw",@progbits
__tls_get_addr:
__tls_get_addr_opt:
	.align 3
	.quad	.__tls_get_addr
	.quad	.TOC.@tocbase
	.quad	0
	.size __tls_get_addr,24
	.size __tls_get_addr_opt,24

	.section ".tbss","awT",@nobits
	.align 3
gd:	.space 8

	.section ".tdata","awT",@progbits
	.align 2
ld:	.long 0xc0ffee

	.text
.__tls_get_addr:
.__tls_get_addr_opt:
	blr
	.size .__tls_get_addr,. - .__tls_get_addr
	.size .__tls_get_addr_opt,. - .__tls_get_addr_opt
