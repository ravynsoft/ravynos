	.option nopic
	.text
	.align 1
	.globl _start
	.type _start, @function
_start:
	lui a0,%hi(foo)
	addi a0,a0,%lo(foo)
	.skip 0x7f8
foo:
	ret
	.size _start, .-_start
