
	bne .label
	.fill 0x3FFC, 1, 0
.label:
	.fill 0x4000, 1, 0
	bne .label
