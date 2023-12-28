# Source code used to test error diagnostics with the LUI instruction.

	.text
foo:
	lui	$2, -1
	lui	$2, 65536
	lui	$2, 0x10000000000000000
	lui	$2, $3
	lui	$2, ($3)
	lui	$2, 0+$3
	lui	$2, (($3))
