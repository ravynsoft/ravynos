# split_s390_1_a1.s: s390 specific, adjustment failure

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	.cfi_startproc
	stm	%r13, %r15, 0x34(%r15)
	.cfi_offset	%r13, -0x2c
	.cfi_offset	%r14, -0x28
	.cfi_offset	%r15, -0x24
	ahi	%r15, -0x60
	.cfi_adjust_cfa_offset	0x60
	brasl	%r14, __morestack
	brasl	%r14, fn2
	lm	%r13, %r15, 0x94(%r15)
	.cfi_restore	%r13
	.cfi_restore	%r14
	.cfi_restore	%r15
	.cfi_adjust_cfa_offset	-0x60
	br	%r14
	.cfi_endproc
	.size	fn1,. - fn1

	.section	.note.GNU-stack,"",@progbits
	.section	.note.GNU-split-stack,"",@progbits
