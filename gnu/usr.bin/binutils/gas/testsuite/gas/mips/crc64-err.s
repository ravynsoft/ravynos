	.text
test_crc:
	crc32d	$4,$4,$4
	crc32d	$5,$4,$4
	crc32d	$4,$5,$4
	crc32d	$4,$4,$5
	crc32d	$4,$5,$6
	crc32cd	$4,$4,$4
	crc32cd	$5,$4,$4
	crc32cd	$4,$5,$4
	crc32cd	$4,$4,$5
	crc32cd	$4,$5,$6
