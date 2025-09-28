	.globl	_start
	.type	_start,%function
_start:
	ldr	r1,1f
	ldr	r1,2f
1:
	.word	foo(GOT)
2:
	.word	_start(GOT)
	.size	_start,.-_start
