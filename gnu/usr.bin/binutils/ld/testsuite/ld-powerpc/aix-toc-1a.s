	.macro  loadtoc
	.toc
	.tc	sym\@[TC], \@

	.csect	.f1[PR]
	.if     size == 32
	lwz     1,sym\@[TC](2)
	.else
	ld      1,sym\@[TC](2)
	.endif
	.endm

	.globl	.f1
	.csect	.f1[PR]
.f1:
	.rept	0x7ffc * 8 / size
	loadtoc
	.endr

	.globl	f1
	.csect	f1[DS]
f1:
	.long	.f1[PR],TOC[TC0],0
