	.text
foo:
	lb	$0, 0x12345678($2)
	lbu	$0, 0x12345678($2)
	lh	$0, 0x12345678($2)
	lhu	$0, 0x12345678($2)
	lw	$0, 0x12345678($2)

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
