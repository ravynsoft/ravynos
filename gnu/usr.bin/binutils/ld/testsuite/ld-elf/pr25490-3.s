	.section	.text.bar,"ax",%progbits
	.globl	bar
	.type	bar, %function
bar:
	.section __patchable_function_entries,"awo",%progbits,bar
	.dc.a	.LPFE1
	.section	.text.bar,"ax",%progbits
.LPFE1:
	.byte 0
	.section	.text._start,"ax",%progbits
	.globl	_start
	.type	_start, %function
_start:
	.section __patchable_function_entries,"awo",%progbits,_start
	.dc.a	.LPFE2
	.section	.text._start,"ax",%progbits
.LPFE2:
	.byte 0
