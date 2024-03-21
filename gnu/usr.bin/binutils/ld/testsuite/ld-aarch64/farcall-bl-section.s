	.global _start

# We will place the section .text at 0x1000.

	.text

_start:
	bl bar
	bl bar2
	ret

# We will place the section .foo at 0x8001000.

	.section .foo, "xa"
	.type bar, @function
bar:
	ret
	.type bar2, @function
bar2:
	ret
