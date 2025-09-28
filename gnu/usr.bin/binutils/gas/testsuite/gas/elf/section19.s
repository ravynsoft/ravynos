	.section .text,"ax",%progbits,unique,20
foo:
	.section __patchable_function_entries,"awo",%progbits,foo,unique,2
	.dc.a	.LPFE1
	.section .text,"ax",%progbits,unique,20
.LPFE1:
	.byte 0
	.section __patchable_function_entries,"awo",%progbits,bar,unique,102
	.dc.a	.LPFE2
	.section .text,"ax",%progbits,unique,2
bar:
.LPFE2:
	.byte 0
