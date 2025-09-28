# 32-bit size relocation in shared object
	.comm	xxx,40,32
	.data
	.p2align 2
	.long	xxx@SIZE
	.long	xxx@SIZE-30
	.long	xxx@SIZE+30
	.long	yyy@SIZE
	.long	zzz@SIZE
	.globl	yyy
	.type	yyy, %object
	.size	yyy, 40
yyy:
	.zero	40
