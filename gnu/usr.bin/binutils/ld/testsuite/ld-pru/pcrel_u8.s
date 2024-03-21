# Test for pc-relative relocations
	.text
	.section .init0, "x"
	.global _start
_start:
	loop end_loop, 5
	nop
	nop
	nop
