	.toc

	.extern	.g
	.globl	.f
	.csect	.f[PR]
.f:
	bl	.g

	.globl	f
	.csect	f[DS]
f:	.long	.f,TOC[tc0],0
