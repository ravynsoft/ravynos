# split_x32_n.s: x32 specific, -fsplit-stack calling non-split

	.text

	.global	fn3
	.type	fn3,@function
fn3:
	retq

	.size	fn3,. - fn3

	.section	.note.GNU-stack,"",@progbits
