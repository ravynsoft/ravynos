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
	.type	.Lfunc1,%function
.Lfunc1:
	bx	lr
	.size	.Lfunc1,.-.Lfunc1

	.section	.text.2, "x"
	.align	11
	.space	2042

	.align	1
	.thumb
	.global	_test1
	.type	_test1,%function
_test1:
	add.w	r0, r0, 0
	b.w	.Lfunc1
	.size	_test1,.-_test1

	.align	8
	.thumb
	.type	.Lfunc2,%function
.Lfunc2:
	bx	lr
	.size	.Lfunc2,.-.Lfunc1

	.align	11
	.space	2042

	.align	1
	.thumb
	.global	_test2
	.type	_test2,%function
_test2:
	add.w	r0, r0, 0
	b.w	.Lfunc2
	.size	_test2,.-_test2


