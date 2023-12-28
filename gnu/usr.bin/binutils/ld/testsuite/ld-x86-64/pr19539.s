	.text
	.global _start
_start:
	.dc.a 0
	.section .prefix,"a",%progbits
	.dc.a foo
