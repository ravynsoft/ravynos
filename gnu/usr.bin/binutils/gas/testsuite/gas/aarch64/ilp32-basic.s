	.cpu generic
	.text
	.align	2
	.global	foo
	.type	foo, %function
foo:
	adrp	x4, ptrs
	add	x3, x4, :lo12:ptrs
	str	w0, [x4,#:lo12:ptrs]
	str	w1, [x3,4]
	str	w2, [x3,8]
	adrp	x4, :got:ptrs
	ldr	x3, [x4,#:got_lo12:ptrs]
	mov	w0, w4
	ret
	ldr	x3, [x4,#:gotpage_lo14:ptrs]
	.size	foo, .-foo
	.comm	ptrs,12,8
