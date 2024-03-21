	.text
	.globl __tls_get_addr
	.globl ___tls_get_addr
	.type	__tls_get_addr,%function
	.set ___tls_get_addr, __tls_get_addr
__tls_get_addr:
	.byte 0
	.size	__tls_get_addr, .-__tls_get_addr
