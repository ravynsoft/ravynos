	.text
foo:
	lwp	$0, 0x12345678($2)
	ldp	$0, 0x12345678($2)

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
