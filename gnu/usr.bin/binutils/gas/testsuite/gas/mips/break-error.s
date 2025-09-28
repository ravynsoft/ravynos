# Source code used to test error diagnostics with the BREAK instruction.

	.text
foo:
	break	-1
	break	65536
	break	0x10000000000000000
	break	$3
	break	($3)
	break	0+$3
	break	(($3))
