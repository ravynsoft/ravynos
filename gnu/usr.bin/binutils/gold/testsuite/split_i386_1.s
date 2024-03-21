# split_i386_1.s: i386 specific test case for -fsplit-stack.

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	cmp	%gs:0x30,%esp
	jae	1f
	call	__morestack
	ret
1:
	call	fn2
	ret

	.size	fn1,. - fn1

	.global	fn2
	.type	fn2,@function
fn2:
	lea	-0x200(%esp),%ecx
	cmp	%gs:0x30,%ecx
	jae	1f
	call	__morestack
	ret
1:
	call	fn1
	ret

	.size	fn2,. - fn2

	.section	.note.GNU-stack,"",@progbits
	.section	.note.GNU-split-stack,"",@progbits
