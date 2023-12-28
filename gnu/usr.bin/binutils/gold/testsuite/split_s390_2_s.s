# split_s390_2_s.s: s390 specific, -fsplit-stack calling -fsplit-stack

	.text

	.global	fn2
	.type	fn2,@function
fn2:
	br	%r14

	.size	fn2,. - fn2

	.section	.note.GNU-stack,"",@progbits
	.section	.note.GNU-split-stack,"",@progbits
