# split_i386_2.s: i386 specific, -fsplit-stack calling non-split

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	cmp	%gs:0x30,%esp
	jae	1f
	call	__morestack
	ret
1:
	call	fn3
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
	call	fn3
	ret

	.size	fn2,. - fn2

	.section	.note.GNU-stack,"",@progbits
	.section	.note.GNU-split-stack,"",@progbits
