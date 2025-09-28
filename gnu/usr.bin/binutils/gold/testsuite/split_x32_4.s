# split_x32_4.s: x32 specific, permitted adjustment failure

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	push	%rbp
	mov	%esp,%ebp
	cmp	%fs:0x40,%esp
	jae	1f
	callq	__morestack
	retq
1:
	callq	fn3
	leaveq
	retq

	.size	fn1,. - fn1

	.section	.note.GNU-stack,"",@progbits
	.section	.note.GNU-split-stack,"",@progbits
	.section	.note.GNU-no-split-stack,"",@progbits
