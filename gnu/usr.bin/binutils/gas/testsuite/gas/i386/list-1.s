	.section .rodata
.LC0:
	.string "haha %x\n"
	.text
.globl hex
	.type hex, @function
hex:
	leave
	ret
