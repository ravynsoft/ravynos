	.global _start
	.global bar

# We will place the section .text at 0x1000.

	.text

_start:
	bl bar
	ret
