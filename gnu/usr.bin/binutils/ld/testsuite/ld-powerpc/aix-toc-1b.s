	.macro  loadtoc
	.toc
	.tc	asym\@[TC], \@ | 0x10000

	.csect	.f2[PR]
	.if     size == 32
	lwz     1,asym\@[TC](2)
	.else
	ld      1,asym\@[TC](2)
	.endif
	.endm

	.globl	.f2
	.csect	.f2[PR]
.f2:
	.rept	0x7ffc * 8 / size
	loadtoc
	.endr

	.globl	f2
	.csect	f2[DS]
f2:
	.long	.f2[PR],TOC[TC0],0
