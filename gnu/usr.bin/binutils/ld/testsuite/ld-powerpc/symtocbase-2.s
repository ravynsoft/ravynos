	.section .toc,"aw",@progbits
	.align 15
	.globl y
y:	.quad	.y,.y@tocbase,0
.LCi:	.quad	i
	.space	48 * 1024

	.data
	.p2align 3
.L2bases:
	.quad	.TOC.@tocbase
	.quad	.x@tocbase
	.quad	.y@tocbase

	.text
	.globl .y
	.hidden .y
.y:
	ld 9,.LCi@toc(2)
	blr
