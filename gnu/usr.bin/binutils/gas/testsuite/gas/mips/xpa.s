	.text
	.set	noat
	.set	noreorder
	.set	nomacro
test_xpa:

	mfhc0	$2, $1
	mfhc0	$2, $16
	mfhc0	$2, $0, 2
	mfhc0	$2, $0, 7

	mthc0	$2, $1
	mthc0	$2, $16
	mthc0	$2, $0, 2
	mthc0	$2, $0, 7
	
	mfhgc0	$2, $1
	mfhgc0	$2, $16
	mfhgc0	$2, $0, 2
	mfhgc0	$2, $0, 7

	mthgc0	$2, $1
	mthgc0	$2, $16
	mthgc0	$2, $0, 2
	mthgc0	$2, $0, 7

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  2
	.space  8
