# split_s390_z2.s: s390 specific test case for -fsplit-stack -
# zarch mode, conditional call, no add

	.text

	.global	fn1
	.type	fn1,@function
fn1:
	.cfi_startproc
	ear	%r1, %a0
	c	%r15, 0x20(%r1)
	larl	%r1, .L1
	jgl	__morestack
	.section .rodata
	.align 4
.L1:
	.long	0x100
	.long	0
	.long	.L2-.L1
	.previous
.L2:
	stm	%r13, %r15, 0x34(%r15)
	.cfi_offset	%r13, -0x2c
	.cfi_offset	%r14, -0x28
	.cfi_offset	%r15, -0x24
	ahi	%r15, -0x60
	.cfi_adjust_cfa_offset	0x60
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
