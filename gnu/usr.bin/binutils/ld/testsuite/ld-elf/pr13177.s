	.section .text.foo,"ax",%progbits
	.globl	foo
	.type	foo, %function
foo:
	.byte 0
	.section .data.opt_out,"aw",%progbits
	.type	opt_out, %object
opt_out:
	.dc.a bar
