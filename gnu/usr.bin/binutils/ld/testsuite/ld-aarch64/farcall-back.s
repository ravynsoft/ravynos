	.global _start
	.global _back
	.global bar1
	.global bar2
	.global bar3

# We will place the section .text at 0x1000.

	.text

	.type _start, @function
_start:
	b	bar1
	bl	bar1
	b	bar2
	bl	bar2
	b	bar3
	bl	bar3
	ret
	.space	0x1000
	.type _back, @function
_back:	ret

# We will place the section .foo at 0x8001000.

	.section .foo, "xa"
	.type bar1, @function
bar1:
	ret
	b	_start

	.space 0x1000
	.type bar2, @function
bar2:
	ret
	b	_start

	.space 0x1000
	.type bar3, @function
bar3:
	ret
	b	_back
