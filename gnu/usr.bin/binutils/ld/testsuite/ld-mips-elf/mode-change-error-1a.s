	.option	pic0
	.text
	.align	4
	.globl	main
	.set	nomips16
	.ent	main
	.type	main, @function
main:
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	
	j	doit
	nop

	j	doit
	nop

	.end	main
	.size	main, .-main
