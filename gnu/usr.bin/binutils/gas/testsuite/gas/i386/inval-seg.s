	.text
# All the following should be illegal
	movl	%ds,(%eax)
	movl	(%eax),%ds

	.intel_syntax noprefix
	mov	eax, es:foo:[eax]
	mov	eax, es:fs:foo:[eax]
	mov	eax, fs:foo:bar:[eax]
	mov	eax, fs:foo:gs:[eax]
	mov	eax, bar:gs:[eax]
