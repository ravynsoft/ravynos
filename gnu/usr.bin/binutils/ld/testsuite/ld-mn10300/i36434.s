	.section .text
	.global _start
	.type	_start,@function
_start:
	mov	.LC2,d1
	nop

	.section	.rodata.str1.1,"aMS",@progbits,1
.LC2:
	.string	"\n"
