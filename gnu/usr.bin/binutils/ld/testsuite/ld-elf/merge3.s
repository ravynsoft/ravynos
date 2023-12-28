	.section .rodata.str1.8,"aMS",%progbits,1
	.p2align 3
.LC0:
	.asciz	"abcdefg"
	.p2align 3
.LC1:
	.asciz	"defg"
	.p2align 3
.LC2:
	.asciz	"01234567abcdefg"

	.data
	.long	.LC0
	.long	.LC1
	.long	.LC2
