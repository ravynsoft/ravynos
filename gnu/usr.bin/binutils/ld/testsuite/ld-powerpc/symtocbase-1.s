	.section .toc,"aw",@progbits
	.align 15
	.globl x
x:	.quad	.x,.x@tocbase,0
.LCi:	.quad	i
	.space	48 * 1024

	.data
	.p2align 2
	.globl i
i:	.long	0
.L1bases:
	.quad	.TOC.@tocbase
	.quad	.x@tocbase
	.quad	.y@tocbase

	.text
	.globl .x
	.hidden .x
.x:
	ld 9,.LCi@toc(2)
        blr
