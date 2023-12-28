	.section .text,"axG",%progbits,bar,comdat
	.globl	bar
	.type	bar, %function
bar:
	.section __patchable_function_entries,"awo",%progbits,bar
	.dc.a	.LPFE1
	.section .text,"axG",%progbits,bar,comdat
.LPFE1:
	.byte 0
	.section .text,"axG",%progbits,foo,comdat
	.globl	foo
	.type	foo, %function
foo:
	.section __patchable_function_entries,"awo",%progbits,foo,unique,0
	.dc.a	.LPFE2
	.section .text,"axG",%progbits,foo,comdat
.LPFE2:
	.byte 0
	.section .text,"axG",%progbits,_start,comdat,unique,1
	.globl	_start
	.type	_start, %function
_start:
	.section __patchable_function_entries,"awoG",%progbits,_start,_start,comdat,unique,3
	.dc.a	.LPFE3
	.section .text,"axG",%progbits,_start,comdat,unique,1
.LPFE3:
	.byte 0
	.section .note,"",%note
	.dc.a	foo
	.dc.a	bar
