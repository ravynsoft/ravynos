	.section .foo2,"aw",%progbits
	.byte 1
	.section .foo1,"w",%progbits
	.string "This is a test."
	.section .foo,"aw",%progbits
	.space 16
