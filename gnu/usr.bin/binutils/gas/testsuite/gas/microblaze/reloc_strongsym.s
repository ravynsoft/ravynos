	.section ".testsection"
	.align	2
	.globl	test_start
	.ent	test_start
	.type	test_start, @function
test_start:
	.frame	r19,8,r15		# vars= 0, regs= 1, args= 0
	.mask	0x00080000
	addik	r1,r1,-8
	swi	r19,r1,4
	addk	r19,r1,r0
	addk	r1,r19,r0
	lwi	r19,r1,4
	addik	r1,r1,8
	rtsd	r15,8
	nop		# Unfilled delay slot

	.end	test_start
$Lfe1:
	.size	test_start,$Lfe1-test_start
