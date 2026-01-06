	.section	__TEXT,__text,regular,pure_instructions
	.section	__DATA_CONST,__dummy
	.p2align 14
// dummy data of a page size to make sure __DATA_CONST,__data is 16k aligned
.dummy:
	.zero 16384

	.section	__DATA_CONST,__data
	.p2align 0

// place _arr right before the end of the page, so there's a pointer fixup at a
// page boundary location _arr+8
  .zero 16384 - 12

	.global _arr
_arr:
	.zero 8
	.quad	_main

.subsections_via_symbols
