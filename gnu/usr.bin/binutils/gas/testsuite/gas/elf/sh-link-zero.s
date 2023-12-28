	.section .meta1,"awo",%progbits,0
.LC1:
	.dc.b 0

	.section .meta2,"awo",%progbits,foo
	.dc.b 1

	.section foo, "aw"
	.dc.b 2

	.section .gc_root,"a",%progbits
	.dc.a .LC1
