	.file	1 "jalx-2-ex.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 1
	.abicalls
	.option	pic0
	.text
	.align	2
	.globl	external_function
	.set	nomips16
	.set	nomicromips
	.ent	external_function
	.type	external_function, @function
external_function:
	.frame	$fp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro

	addiu	$sp,$sp,-8
	sw	$fp,4($sp)
	move	$fp,$sp
	move	$sp,$fp
	lw	$fp,4($sp)
	addiu	$sp,$sp,8
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	external_function
	.size	external_function, .-external_function
	.ident	"GCC: (Sourcery G++ Lite 4.4-999999 - Preview) 4.4.1"
