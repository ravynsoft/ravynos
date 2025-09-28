	.data
foo:
	.long 0
	.text
movl	foo@GOT(%ecx), %eax
movl	bar@GOT(%ecx), %eax
	bar = 0xfffffff0
