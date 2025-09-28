	.file	1 "mips-fp-stub-2.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	fp=32
	.module	oddspreg
	.text
	.align	2
	.globl	foo
	# Stub function for foo (float, float)
	.section	.mips16.fn.foo,"ax",@progbits
	.align	2
	.set	nomips16
	.set	nomicromips
	.ent	__fn_stub_foo
	.type	__fn_stub_foo, @function
__fn_stub_foo:
	la	$25,foo
	mfc1	$4,$f12
	mfc1	$5,$f14
	jr	$25
	.end	__fn_stub_foo
	__fn_local_foo = foo
	.text
	.set	mips16
	.set	nomicromips
	.ent	foo
	.type	foo, @function
foo:
	.frame	$17,8,$31		# vars= 0, regs= 2/0, args= 0, gp= 0
	.mask	0x80020000,-4
	.fmask	0x00000000,0
	save	8,$17,$31
	move	$17,$sp
	sw	$4,8($17)
	sw	$5,12($17)
	lw	$2,8($17)
	move	$sp,$17
	restore	8,$17,$31
	j	$31
	.end	foo
	.size	foo, .-foo
	.ident	"GCC: (Sourcery CodeBench Lite 2015.11-12 - Preview) 5.2.0"
