	.syntax	unified
	.globl	foo
	.type	foo,%function
	.thumb_func
foo:
	nop			@ 0x00
	movw	r0,#0		@ 0x02
	movw	r0,#0		@ 0x06
	movw	r0,#0		@ 0x0a
	bl	bar(PLT)	@ 0x0e
