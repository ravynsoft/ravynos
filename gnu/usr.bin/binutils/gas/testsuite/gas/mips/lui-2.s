# Source code used to test error diagnostics with the LUI instruction.
# These need to be separate from lui-1.s as they are reported at a later
# stage in assembly.

	.text
foo:
	lui	$2, bar - foo
	lui	$2, baz - bar
	lui	$2, foo - baz
	lui	$2, bar / baz
