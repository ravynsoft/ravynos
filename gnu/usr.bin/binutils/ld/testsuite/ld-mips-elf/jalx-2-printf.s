	.file	1 "jalx-2-printf.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 1
	.abicalls
	.text
	.align	2
	.globl	printf
	.set	nomips16
	.set	micromips
	.ent	printf
	.type	printf, @function
printf:
	.frame	$fp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro

	addiu	$sp,$sp,-8
	sw	$fp,4($sp)
	move	$fp,$sp
	sw	$5,12($fp)
	sw	$6,16($fp)
	sw	$7,20($fp)
	sw	$4,8($fp)
	move	$2,$0
	move	$sp,$fp
	lw	$fp,4($sp)
	jraddiusp	8
	.set	macro
	.set	reorder
	.end	printf
	.size	printf, .-printf
	.ident	"GCC: (Sourcery G++ Lite 4.4-999999 - Preview) 4.4.1"
