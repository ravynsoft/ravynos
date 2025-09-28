# split_s390_2_ns.s: s390 specific, -fsplit-stack calling non-split

	.text

	.global	fn2
	.type	fn2,@function
fn2:
	br	%r14

	.size	fn2,. - fn2

	.section	.note.GNU-stack,"",@progbits
