	.section .rodata.str,"aMS","progbits",1
.LC0:
	.asciz	"abc"
.LC1:
	.asciz	"c"
.LC2:

	.data
	.long	.LC0
	.long	.LC1
	.long	.LC1-.LC0
	.long	.LC2-.LC1
