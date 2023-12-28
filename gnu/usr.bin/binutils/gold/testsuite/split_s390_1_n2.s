# split_s390_1_n2.s: s390 specific test case for -fsplit-stack -
# no stack frame, short sibcall (will fail)

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	.cfi_startproc
	j	fn2
	.cfi_endproc
	.size	fn1,. - fn1

	.section	.note.GNU-stack,"",@progbits
	.section	.note.GNU-split-stack,"",@progbits
