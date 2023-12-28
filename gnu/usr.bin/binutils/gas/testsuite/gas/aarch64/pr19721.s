	.text

	mov	x7, x17
	mov	x7, x17, lsl 25
	orr	x7, xzr, x17, lsl 25
        orr     x0, xzr, x0, lsr #0	// shift == 01
        orr     x0, xzr, x0, lsl #1	// imm6 == 000001
        orr     x16, x30, x0		// Rn == 11110
