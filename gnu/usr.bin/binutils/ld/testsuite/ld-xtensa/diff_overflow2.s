	.section .text.f1,"axG",@progbits,f1,comdat
	.literal_position
	.literal .L5, 0
	.align	4
f4:
	entry	a1, 32
.Lf4:
	l32r	a2, .L5
	l32r	a2, .L5
	nop
	nop
	retw

	.section .text
f5:
	entry	a1, 32
.Lf5:
	retw

	.section .debug_frame,"",@progbits
	.byte	.Lf4 - f4
	.byte	.Lf5 - f5
