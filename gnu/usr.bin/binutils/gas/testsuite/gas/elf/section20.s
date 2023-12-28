	.section .text,"axG",%progbits,foo,comdat
foo:
	.section __patchable_function_entries,"awoG",%progbits,foo,foo,comdat
	.dc.a	.LPFE1
	.section .text,"axG",%progbits,foo,comdat
.LPFE1:
	.byte 0
	.section __patchable_function_entries,"awoG",%progbits,bar,bar,comdat,unique,4
	.dc.a	.LPFE2
	.section .text,"axG",%progbits,bar,comdat,unique,24
bar:
.LPFE2:
	.byte 0
