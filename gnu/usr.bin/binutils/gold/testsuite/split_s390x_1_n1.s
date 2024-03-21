# split_s390x_1_n1.s: s390x specific test case for -fsplit-stack -
# no stack frame, load function address.

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	.cfi_startproc
	larl	%r2, fn2
	br	%r14
	.cfi_endproc
	.size	fn1,. - fn1

	.section	.note.GNU-stack,"",@progbits
	.section	.note.GNU-split-stack,"",@progbits
