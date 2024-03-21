	.text
foo:
	j	0x0000000
	j	0xaaaaaa4
	j	0x5555558
	j	0xffffffc
	jal	0x0000000
	jal	0xaaaaaa4
	jal	0x5555558
	jal	0xffffffc

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	8
