	.syntax unified
	.text
	.globl	Strong
Strong:	
	adrl	r0,Strong
	adr	r0,Strong
	.globl	Weak
	.weak	Weak
Weak:	adrl	r0,Weak
	adr	r0,Weak
