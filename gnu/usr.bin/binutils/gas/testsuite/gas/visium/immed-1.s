!	Tests for complex immediate expressions.
	.text
bar:
	moviq	r1, 0
	subi	r1, (. - bar + 8)
	addi	r1, bar
	moviq	r4, (bar - . -8) & 0xffff
	.space 4096
	moviq	r6, (. - bar - 8) & 0xff
	moviq	r7, (bar - . -8) & 0xff
