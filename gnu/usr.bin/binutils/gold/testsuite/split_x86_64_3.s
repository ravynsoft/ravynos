# split_x86_64_3.s: x86_64 specific, adjustment failure

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	push	%rbp
	mov	%rsp,%rbp
	cmp	%fs:0x70,%rsp
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
