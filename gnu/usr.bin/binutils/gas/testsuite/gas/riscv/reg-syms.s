	.text
start:
	and	x8, x8, x1
	j	x2
	jal	x3, x4
	lui	x5, %hi(x6)
	lw	x7, %lo(x8)(x9)
	sll	x1, x1, x10
	sra	x8, x8, x11
	srl	x8, x8, x12
