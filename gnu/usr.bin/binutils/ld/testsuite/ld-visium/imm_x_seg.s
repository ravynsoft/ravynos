	.section seg1
foo:
	nop
	moviq	r6,bar-foo

	.section seg2
bar:
	.end
