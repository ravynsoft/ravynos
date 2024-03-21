	.text
	.globl	_start
	.type	_start, %function
_start:
	.section __patchable_function_entries,"awo",%progbits,_start
	.dc.a	.LPFE1
	.text
.LPFE1:
	.byte 0
