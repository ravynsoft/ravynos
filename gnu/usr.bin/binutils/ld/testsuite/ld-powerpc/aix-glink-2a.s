	.toc

	.macro	defabs,type,name,value
	\type	\name
	\name	= \value
	.endm

	.macro	deffun,type,name,fn
	\type	\name
	.csect	\name\()[DS]
\name\():
	.if	size == 32
	.long	.\name\()[PR],TOC[TC0],0
	.else
	.llong	.\name\()[PR],TOC[TC0],0
	.endif

	.globl	.\name
	.csect	.\name\()[PR]
.\name\():
	nop
	.endm

	.macro	defdata,type,name,contents
	\type	\name
	.csect	\name\()[RW]
\name\():
	.long	\contents
	.endm

	defabs	.globl,a1,0xf100
	deffun	.globl,a2
	defdata	.globl,a3,0x1100

	defabs	.globl,b1,0xf200
	deffun	.globl,b2
	defdata	.globl,b3,0x2200

	defabs	.weak,c1,0xf300
	deffun	.weak,c2
	defdata	.weak,c3,0x3300
