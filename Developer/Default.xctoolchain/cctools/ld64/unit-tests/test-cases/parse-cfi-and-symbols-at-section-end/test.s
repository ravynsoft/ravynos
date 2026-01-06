	// File with minimal CFI starts. Linking this together
	// with constant.s lead to different results due to different
	// code paths when parsing symbols and CFI information.
	.section	__TEXT,__text,regular,pure_instructions
	.p2align 2

foo:
	.cfi_startproc
	.cfi_lsda 16, Lexception0
	ret
	.cfi_endproc

	.section	__TEXT,__gcc_except_tab
	.p2align	2
Lexception0:
	.byte	0
	.byte	0

.subsections_via_symbols
