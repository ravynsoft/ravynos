# Test maximum encoding range of bl

	.global _start
	.global bar

# We will place the section .text at 0x1000.

	.text

_start:
	bl bar
	ret

# We will place the section .foo at 0x8000ffc

	.section .foo, "xa"
	.type bar, @function
bar:
	ret
