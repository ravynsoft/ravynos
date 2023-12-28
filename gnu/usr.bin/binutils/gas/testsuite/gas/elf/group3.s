	.section .foo,"axG",%progbits,foo,comdat
	.globl	foo
	.type	foo,%function
foo:
	.byte	1
	.section .bar,"aG",%progbits,foo,comdat
	.dc.a	foo
