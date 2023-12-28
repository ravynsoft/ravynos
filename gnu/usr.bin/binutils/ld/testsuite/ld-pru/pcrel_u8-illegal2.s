# Test for illegal pc-relative relocations
	.text
	.section .init0, "x"
	.global _start
_start:
L0:
	loop L0, r0
