	.section .text.f0,"axG",@progbits,f0,comdat
	.literal_position
	.literal .L0, 0
	.align 4
f0:
	entry	a1, 32
	l32r	a2, .L0
	retw

	.section .text
	.literal_position
	.global _start
_start:
	entry	a1, 32
	retw

	.section .text.f1,"axG",@progbits,f1,comdat
	.literal_position
	.literal .L1, 0
	.literal .L2, 0
	.align 4
	.global	f1
f1:
	entry	a1, 32
	l32r	a2, .L1
	l32r	a3, .L2
	retw
