# Test for pc-relative relocations
	.text
	.section .init0, "x"
	.global _start
_start:
	qba ext_label
	qba ext_label + 16
