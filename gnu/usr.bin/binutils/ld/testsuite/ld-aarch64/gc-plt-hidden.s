	.text
	.globl hidfn
	.hidden hidfn
	.type hidfn, %function
hidfn:
	and x0, x0, x0
	.size hidfn, . - hidfn
