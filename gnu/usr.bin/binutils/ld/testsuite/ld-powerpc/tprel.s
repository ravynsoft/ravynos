	.section ".tbss","awT",@nobits
	.p2align 2
wot:	.space 4

	.text
	.global _start
_start:
	addis 3,REG,wot@tprel@ha
	lwz 3,wot@tprel@l(3)
	blr
