	.text

	.ent test_mfix_r5900
test_mfix_r5900:
	# Test the short loop fix with 3 loop instructions.
	li $3, 300
short_loop3:
	addi $3, -1
	addi $4, -1
	# A NOP will be inserted in the branch delay slot.
	bne $3, $0, short_loop3

	# Test the short loop fix with 6 loop instructions.
	li $3, 300
short_loop6:
	addi $3, -1
	addi $4, -1
	addi $5, -1
	addi $6, -1
	addi $7, -1
	# A NOP will be inserted in the branch delay slot.
	bne $3, $0, short_loop6

	# Test the short loop fix with 7 loop instructions.
	li $3, 300
short_loop7:
	addi $3, -1
	addi $4, -1
	addi $5, -1
	addi $6, -1
	addi $7, -1
	addi $8, -1
	# The short loop fix does not apply for loops with
	# more than 6 instructions.
	bne $3, $0, short_loop7

	li $4, 3

	.space	8
	.end test_mfix_r5900
