	.data
	.p2align 16
x:	.skip	10000

	.text
	.globl	_start
_start:

	.set	i, 0
.rept 100
	l.movhi	r3, ha(x+i)
	l.sw	lo(x+i)(r3), r0
	.set	i, i+1000
.endr
