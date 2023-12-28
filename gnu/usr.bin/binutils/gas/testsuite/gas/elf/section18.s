	.text
foo:
	.section __patchable_function_entries,"awo",%progbits,foo
	.dc.a	.LPFE1
	.text
.LPFE1:
	.byte 0
	.section __patchable_function_entries,"awo",%progbits,bar
	.dc.a	.LPFE2
	.text
bar:
.LPFE2:
	.byte 0
