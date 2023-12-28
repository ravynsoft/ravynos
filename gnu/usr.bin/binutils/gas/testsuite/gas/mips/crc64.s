	.text
test_crc:
	crc32d	$4,$7,$4
	crc32cd	$4,$7,$4

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
