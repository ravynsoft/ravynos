	.section	__TEXT,__text,regular,pure_instructions
  .global _foo
	.p2align 2
_foo:
	.zero 4

	.section	__DATA_CONST,__data

	.zero 1

_arr:
	.zero 7
	.quad	_foo

.subsections_via_symbols
