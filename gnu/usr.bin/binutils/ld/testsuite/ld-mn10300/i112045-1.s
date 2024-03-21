	.text
	.global _start
_start:
	add     0x1000 - L01, A0
	nop
	nop
L01:
	add     L01 + 0x123, A0
	nop
	nop
L02:
	add     L02 - L01, A0
	nop
	nop
