	.section ".tbss","awT",@nobits
	.p2align 2
wot:	.space 4

	.text
	.global _start
_start:
	lis 3,wot@tprel@ha
	addi 3,3,wot@tprel@l
	blr
