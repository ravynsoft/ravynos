	.text
	.globl	_start
	.type	_start, %function
_start:
	.section	__patchable_function_entries,"aw",%progbits
	.dc.a	.LPFE1
	.text
.LPFE1:
	.byte 0
