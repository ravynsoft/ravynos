	.text
test_crc:
	crc32b	$4,$7,$4
	crc32h	$4,$7,$4
	crc32w	$4,$7,$4
	crc32cb	$4,$7,$4
	crc32ch	$4,$7,$4
	crc32cw	$4,$7,$4

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
