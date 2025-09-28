	.text
	.globl _start
_start:
	.byte 0
	.section .note,"",%note
	.dc.a .foo

	.section .foo,"a"
	.dc.a	0
	.section .bar,"ao",%progbits,.foo
	.dc.a	0
	.section .zed,"ao",%progbits,.foo
	.dc.a	0
