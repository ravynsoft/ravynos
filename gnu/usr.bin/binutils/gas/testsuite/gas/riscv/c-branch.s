	.text
target:
	beq	x8, x0, target
	beqz	x9, target
	bne	x8, x0, target
	bnez	x9, target
	j	target
	jal	target
	jalr	x6
	jr	x7
	ret
