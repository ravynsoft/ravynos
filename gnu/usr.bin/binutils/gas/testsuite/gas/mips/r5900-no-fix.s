	.text

	.ent test_no_mfix_r5900
test_no_mfix_r5900:
	# Test that the short loop fix with 3 loop instructions
	# is not applied with `-mno-fix-r5900'.
	li $3, 300
short_loop_no_mfix_r5900:
	addi $3, -1
	addi $4, -1
	# A NOP will not be inserted in the branch delay slot.
	bne $3, $0, short_loop_no_mfix_r5900

	li $4, 3

	.space	8
	.end test_no_mfix_r5900
