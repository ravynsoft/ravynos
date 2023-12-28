	.globl	foo
foo:

	.section	.tls_data
	.p2align	2
	.type	i,%object
	.size	i,4
i:
	.space	4
	.globl	__tls__i
	.section	.tls_vars
	.p2align	2
	.type	__tls__i,%object
	.size	__tls__i,12
__tls__i:
	.4byte	i
	.4byte	0
	.4byte	4

