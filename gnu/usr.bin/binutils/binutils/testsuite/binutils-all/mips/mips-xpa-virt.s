	.set	mips64r5
	.set	xpa
	.set	virt

	.text
foo:
	mfc0	$2, $1
	mfhc0	$2, $1
	mfgc0	$2, $1
	mfhgc0	$2, $1

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
        .align  4, 0
        .space  16
