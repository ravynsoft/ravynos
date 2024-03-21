	.section .text.foo,"ax",%progbits
	.globl	foo
	.type	foo, %function
foo:
	.byte 0
	.section .text.bar,"ax",%progbits
	.type	bar, %function
	.globl	bar
bar:
	.byte 0
