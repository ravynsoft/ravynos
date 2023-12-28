	.text
foo:
	j	0x0000000
	j	0xaaaaaa2
	j	0x5555554
	j	0xffffff6
	jal	0x0000008
	jal	0xaaaaaaa
	jal	0x555555c
	jal	0xffffffe
	jals	0x0000002
	jals	0xaaaaaa6
	jals	0x555555a
	jals	0xffffffe

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	8
