	.section .data,"aw",%progbits,unique,0x100000000
	.byte 0
	.section .bss,"aw",%nobits,unique,foo
	.byte 0
	.section .text,"ax",%progbits,unique,1,foo
	.byte 0
