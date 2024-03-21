	.syntax unified
	.text
	.global x
x:
	mov	r0, r1
	cmp	r0, #0
	it	ne
	moveq	r0, r1
	bx	lr
	movgt	r1, r2
