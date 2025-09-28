# Test for illegal pc-relative relocations
	.text
	.section .init0, "x"
	.global _start
_start:
foo:
# Negative loop termination point
	nop
	loop foo, r20
