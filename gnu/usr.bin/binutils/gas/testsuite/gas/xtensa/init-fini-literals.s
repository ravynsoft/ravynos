	.section	.init,"ax",@progbits
	.literal_position
	.literal .LC0, init@PLT
	.literal_position
	.literal .LC1, 1
	.align	4

	l32r	a2, .LC0
	l32r	a2, .LC1

	.section	.fini,"ax",@progbits
	.literal_position
	.literal .LC2, fini@PLT
	.literal_position
	.literal .LC3, 1
	.align	4

	l32r	a2, .LC2
	l32r	a2, .LC3
