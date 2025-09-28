# split_x86_64_1.s: x86_64 specific test case for -fsplit-stack.

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	cmp	%fs:0x70,%rsp
	jae	1f
	callq	__morestack
	retq
1:
	callq	fn2
	retq

	.size	fn1,. - fn1

	.global	fn2
	.type	fn2,@function
fn2:
	lea	-0x200(%rsp),%r10
	cmp	%fs:0x70,%r10
	jae	1f
	callq	__morestack
	retq
1:
	callq	fn1
	retq

	.size	fn2,. - fn2

	.section	.note.GNU-stack,"",@progbits
	.section	.note.GNU-split-stack,"",@progbits
