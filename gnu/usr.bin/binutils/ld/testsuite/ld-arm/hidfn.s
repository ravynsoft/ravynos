	.text
	.globl hidfn
	.hidden	hidfn
	.type hidfn, %function
hidfn:
	ldmfd	sp!, {pc}
	.size hidfn, . - hidfn
