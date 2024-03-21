	.toc

	.macro	defabs,type,name,value
	\type	\name
	\name	= \value
	.endm

	.macro	deffun,type,name
	\type	\name
	.csect	\name\()[DS]
\name\():
	.long	\name\()[PR],TOC[TC0],0

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

	defabs	.globl,a1,0xf400
	deffun	.globl,a2
	defdata	.globl,a3,0x4400

	defabs	.globl,c1,0xf500
	deffun	.globl,c2
	defdata	.globl,c3,0x5500
