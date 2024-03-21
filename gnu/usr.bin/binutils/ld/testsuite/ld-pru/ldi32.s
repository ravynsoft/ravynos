# Test the ldi32 relocation

	.text
	.global _start
_start:
	ldi32	r16, long_symbol
