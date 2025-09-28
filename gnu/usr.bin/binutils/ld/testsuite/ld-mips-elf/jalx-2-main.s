	.file	1 "jalx-2-main.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 1
	.abicalls
	.option	pic0
	.text
	.align	2
	.globl	internal_function
	.set	nomips16
	.set	micromips
	.ent	internal_function
	.type	internal_function, @function
internal_function:
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
	jraddiusp	8
	.set	macro
	.set	reorder
	.end	internal_function
	.size	internal_function, .-internal_function
	.rdata
	.align	2
$LC0:
	.ascii	"hello world\012\000"
	.text
	.align	2
	.globl	main
	.set	nomips16
	.set	micromips
	.ent	main
	.type	main, @function
main:
	.frame	$fp,32,$31		# vars= 0, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro

	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$fp,24($sp)
	move	$fp,$sp
	sw	$4,32($fp)
	sw	$5,36($fp)
	lui	$2,%hi($LC0)
	addiu	$4,$2,%lo($LC0)
	jal	printf
	nop

	jal	internal_function
	nop

	jal	external_function
	nop

	move	$sp,$fp
	lw	$31,28($sp)
	lw	$fp,24($sp)
	jraddiusp	32
	.set	macro
	.set	reorder
	.end	main
	.size	main, .-main
	.ident	"GCC: (Sourcery G++ Lite 4.4-999999 - Preview) 4.4.1"
