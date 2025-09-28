# 64-bit size relocation in shared object
	.comm	xxx,40,32
	.data
	.p2align 2
	.quad	xxx@SIZE
	.quad	xxx@SIZE-30
	.quad	xxx@SIZE+30
	.quad	yyy@SIZE
	.quad	zzz@SIZE
	.globl	yyy
	.type	yyy, %object
	.size	yyy, 40
yyy:
	.zero	40
