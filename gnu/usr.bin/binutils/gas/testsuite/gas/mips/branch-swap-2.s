	.set	micromips
	.text
foo:
	.rept	count
	ori	$2, $3, (. - foo) >> 2
	.endr
	addu	$2, $3, $4
	j	ext
