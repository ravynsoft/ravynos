	.globl	_start
_start:
	ldr	r4,1f
	mov	pc,lr
1:
	.word	foo(GOT)
