# Source code used to test error diagnostics with the ADDIU instruction.

	.text
foo:
	addiu	$2, -32769
	addiu	$2, 65536
	addiu	$2, 0x10000000000000000
	addiu	$2, $3
	addiu	$2, ($3)
	addiu	$2, 0+$3
	addiu	$2, (($3))
