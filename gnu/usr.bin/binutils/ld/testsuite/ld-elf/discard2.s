	.globl here
	.data
here:
	.long	1
	.globl there
	.section	.discard,"aw",%progbits
	.align 4
there:
	.long	2
	.section	.debug_info,"",%progbits
	.long	here
	.long	there
