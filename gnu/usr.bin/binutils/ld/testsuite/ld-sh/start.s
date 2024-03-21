	.section .text
	.global	start
	.global	_start
start:
_start:

	mov.l	stack_k,r15

	! call the mainline	
L1:	
	mov.l	main_k,r0
	.uses	L1
	jsr	@r0
	nop

	.align 2
stack_k:
	.long	_stack	
main_k:
	.long	_main

	.global _trap
_trap:	
	trapa #34
	rts
	nop

	.section .stack,"aw",%nobits
	.balign 4096
	.space 4096
_stack:
