	.toc

	.macro	deffun,type,name
	.globl	.\name
	.csect	.\name\()[PR]
.\name\():
	nop
	.endm

	deffun	.globl,b1
	deffun	.globl,b2
	deffun	.globl,b3
