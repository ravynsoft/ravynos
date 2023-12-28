	.text
	.globl _start
	.type _start, %function
_start:
	str	lr, [sp, #-4]!
	bl	hidfn(PLT)
	ldmfd	sp!, {pc}
	.size _start, . - _start
