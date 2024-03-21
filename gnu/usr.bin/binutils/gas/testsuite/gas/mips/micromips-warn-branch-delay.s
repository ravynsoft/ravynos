# Source file used to test microMIPS fixed-size branch delay slots.

	.text
	.set	noreorder
	.set	noat
foo:
	jalrs	$2
	 and	$2,$3,$4
	jalrs	$2
	 swr	$2,0($3)
	jalrs	$2
	 swl	$2,0($3)
	jalrs	$2
	 mul	$16,$18,$19
	jalrs	$2
	 sltu	$17,$31,$0
	jalrs	$2
	 add	$17,$0,$17
	jalrs	$2
	 sub	$17,$17,$13
