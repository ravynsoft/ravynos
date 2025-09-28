# 32-bit size relocation against hidden symbol in shared object
	.comm	xxx,40,32
	.data
	.p2align 2
	.long	xxx@SIZE
