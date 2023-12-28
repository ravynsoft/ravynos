# Test for illegal pc-relative relocations
	.text
	.section .init0, "x"
	.global _start
_start:
	loop L1, r0
L1:
