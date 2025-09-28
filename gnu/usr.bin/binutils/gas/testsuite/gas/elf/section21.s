	.section __patchable_function_entries,"awo",%progbits,bar
	.dc.a	.LPFE1
	.text
.LPFE1:
	.byte 0
	.section __patchable_function_entries,"awo",%progbits,foo
	.dc.a	.LPFE2
	.text
.LPFE2:
	.dc.a foo
	.section __patchable_function_entries,"awo",%progbits,1foo
	.dc.a	.LPFE3
	.text
.LPFE3:
	.byte 0
