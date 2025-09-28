	.globl	foo
foo:

	.section	.tls_data,"a"
	.p2align	2
	
	.type	i,%object
	.size	i,4
i:
	.space	4
	
	.globl	j
	.type	j,%object
	.size	j,4
j:
	.space	4
	
	.section	.tls_vars,"a"
	.p2align	2
	.type	__tls__i,%object
	.size	__tls__i,12
__tls__i:
	.4byte	i
	.4byte	0
	.4byte	4
	
	.globl	__tls__j
	.type	__tls__j,%object
	.size	__tls__j,12
__tls__j:
	.4byte	j
	.4byte	0
	.4byte	4

