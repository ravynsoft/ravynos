# Source code used to test the LUI instruction with some expressions.

	.text
foo:
0:
	lui	$2, 0
	lui	$2, 65535
1:
	lui	$2, 1b - 0b
bar:
	lui	$2, 2f - 1b
2:
	lui	$2, bar - foo
	lui	$2, baz - bar
baz:
	lui	$2, bar
	lui	$2, ext
3:
	lui	$2, 3b
	lui	$2, 4f
4:
	lui	$2, min + 1
	lui	$2, max - 1
	.eqv	min, -1
	.eqv	max, 65536

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
