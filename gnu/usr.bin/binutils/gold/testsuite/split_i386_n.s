# split_i386_n.s: i386 specific, -fsplit-stack calling non-split

	.text

	.global	fn3
	.type	fn3,@function
fn3:
	ret

	.size	fn3,. - fn3

	.section	.note.GNU-stack,"",@progbits
