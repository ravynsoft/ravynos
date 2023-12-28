# split_s390x_1_a2.s: s390x specific, permitted adjustment failure

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	.cfi_startproc
	stmg	%r13, %r15, 0x68(%r15)
	.cfi_offset	%r13, -0x38
	.cfi_offset	%r14, -0x30
	.cfi_offset	%r15, -0x28
	aghi	%r15, -0xa0
	.cfi_adjust_cfa_offset	0xa0
	brasl	%r14, __morestack
	brasl	%r14, fn2
	lmg	%r13, %r15, 0x108(%r15)
	.cfi_restore	%r13
	.cfi_restore	%r14
	.cfi_restore	%r15
	.cfi_adjust_cfa_offset	-0xa0
	br	%r14
	.cfi_endproc
	.size	fn1,. - fn1

	.section	.note.GNU-stack,"",@progbits
	.section	.note.GNU-split-stack,"",@progbits
	.section	.note.GNU-no-split-stack,"",@progbits
