	.syntax unified
	.text
	.thumb

	.globl	Weak
	.weak	Weak
	.thumb_func
	.type	Weak, %function
Weak:
	b	Strong
	.size	Weak, .-Weak
	
	.globl	Strong
	.type	Strong, %function
Strong:
	b	Random
	b	Weak
	.size	Strong, .-Strong
