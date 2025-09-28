	.section .text
	.global _bar
	.type	_bar,@function
_bar:
	mov	.LC1,d0
	mov	.LC2,d1
	nop

	.section	.rodata.str1.1,"aMS",@progbits,1
.LC1:
	.rept	32768
	.byte	'a'
	.endr
	.byte	0
.LC2:
	.string	"abc\n"
