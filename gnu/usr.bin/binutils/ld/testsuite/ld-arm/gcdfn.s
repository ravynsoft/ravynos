	.text
	.globl gcdfn
	.type gcdfn, %function
gcdfn:
	str	lr, [sp, #-4]!
	bl	hidfn(PLT)
	ldmfd	sp!, {pc}
	.size gcdfn, . - gcdfn
