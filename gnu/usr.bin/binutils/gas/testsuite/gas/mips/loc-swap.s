# Source file to test DWARF-2 location information with branch swapping.

	.file	1 "loc-swap.s"
	.text
foo:
	.loc	1 7
	move	$4, $16
	.loc	1 9
	jr	$4

	.loc	1 12
	move	$31, $16
	.loc	1 14
	jr	$4

	.loc	1 17
	move	$4, $16
	.loc	1 19
	jr	$31

	.loc	1 22
	move	$31, $16
	.loc	1 24
	jr	$31

	.loc	1 27
	move	$4, $16
	.loc	1 29
	jalr	$4

	.loc	1 32
	move	$31, $16
	.loc	1 34
	jalr	$4

	.loc	1 37
	move	$4, $16
	.loc	1 39
	jal	bar

	.loc	1 42
	move	$31, $16
	.loc	1 44
	jal	bar

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
