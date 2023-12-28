	.globl bar
	.data
bar:
	.long	1
	.section	.discard,"aw",%progbits
	.align 4
there:
	.long	2
	.section	.debug_info,"",%progbits
	.long	bar
	.long	there
