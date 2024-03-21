	.text
	.align	2
	.globl	__def_start
	.ent	__def_start
	.type	__def_start, @function
__def_start:
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

	.end	__def_start
$Lfe1:
	.size	__def_start,$Lfe1-__def_start
	.align	2
	.globl	main
	.ent	main
	.type	main, @function
main:
	.frame	r19,32,r15		# vars= 0, regs= 1, args= 24
	.mask	0x00088000
	addik	r1,r1,-32
	swi	r15,r1,0
	swi	r19,r1,28
	addk	r19,r1,r0
	brlid	r15,test_start
	nop		# Unfilled delay slot

	brlid	r15,test_start_strong
	nop		# Unfilled delay slot

	addk	r3,r0,r0
	lwi	r15,r1,0
	addk	r1,r19,r0
	lwi	r19,r1,28
	addik	r1,r1,32
	rtsd	r15,8
	nop		# Unfilled delay slot

	.end	main
$Lfe2:
	.size	main,$Lfe2-main
	.weakext	test_start
	test_start = __def_start
	.globl	test_start_strong
	test_start_strong = __def_start
