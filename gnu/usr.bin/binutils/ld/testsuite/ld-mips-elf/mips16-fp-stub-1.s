	.file	1 "mips-fp-stub-1.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	fp=32
	.module	oddspreg
	# Stub function to call float foo (float, float)
	.section	.mips16.call.fp.foo,"ax",@progbits
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	__call_stub_fp_foo
	.type	__call_stub_fp_foo, @function
__call_stub_fp_foo:
	.cfi_startproc
	.cfi_def_cfa 29,-4
	.cfi_escape 0x16,29,1,0x6d
	move	$18,$31
	mtc1	$4,$f12
	mtc1	$5,$f14
	jal	foo
	.cfi_register 31,18
	mfc1	$2,$f0
	jr	$18
	.cfi_endproc
	.size	__call_stub_fp_foo, .-__call_stub_fp_foo
	.end	__call_stub_fp_foo
	.text
	.align	2
	.globl	main
	.set	mips16
	.set	nomicromips
	.ent	main
	.type	main, @function
main:
	.frame	$17,24,$31		# vars= 8, regs= 3/0, args= 16, gp= 0
	.mask	0x80060000,-4
	.fmask	0x00000000,0
	save	40,$17,$18,$31
	addiu	$17,$sp,16
	lw	$3,.L3
	lw	$2,.L4
	move	$5,$3
	move	$4,$2
	jal	foo
	sw	$2,0($17)
	li	$2,0
	move	$sp,$17
	restore	24,$17,$18,$31
	j	$31
	.align	2
.L3:
	.word	1085485875
.L4:
	.word	1065353216
	.end	main
	.size	main, .-main
	.ident	"GCC: (Sourcery CodeBench Lite 2015.11-12 - Preview) 5.2.0"
