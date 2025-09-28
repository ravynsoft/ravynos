# split_i386_4.s: i386 specific, permitted adjustment failure

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	push	%ebp
	mov	%esp,%ebp
	cmp	%gs:0x30,%esp
	jae	1f
	call	__morestack
	ret
1:
	call	fn3
	leave
	ret

	.size	fn1,. - fn1

	.section	.note.GNU-stack,"",@progbits
	.section	.note.GNU-split-stack,"",@progbits
	.section	.note.GNU-no-split-stack,"",@progbits
