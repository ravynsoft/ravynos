	.toc

	.globl	.g
	.csect	.g[PR]
.g:
	blr

	.globl	g
	.csect	g[DS]
g:	.long	.g,TOC[tc0],0
