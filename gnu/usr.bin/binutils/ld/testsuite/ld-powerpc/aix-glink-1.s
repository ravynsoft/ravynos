	.toc
	.globl	.f1
	.csect	.f1[PR]
.f1:
	blr

	.globl	.f2
	.csect	.f2[PR]
.f2:
	bl	.f3

	.globl	.f3
	.csect	.f3[PR]
.f3:
	blr

	.globl	foo
	.csect	foo[RW]
foo:
	.long	f1
	.long	.ext
