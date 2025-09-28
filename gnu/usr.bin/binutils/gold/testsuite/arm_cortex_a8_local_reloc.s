	.syntax	unified
	.cpu	cortex-a8

	.section	.text.0, "x"
	.align	12
	
_start:
	.type	_start,%function
	bx	lr
	.size	_start,.-_start

	.section	.text.1, "x"
	.align	11
	.thumb
	.type	.Lfunc,%function
.Lfunc:
	bx	lr
	.size	.Lfunc,.-.Lfunc

	.section	.text.2, "x"
	.align	11
	.space	2042

	.align	1
	.thumb
	.global	_test
	.type	_test,%function
_test:
	add.w	r0, r0, 0
	b.w	.Lfunc
	.size	_test,.-_test
