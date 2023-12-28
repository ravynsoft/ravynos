	.section .text.new_foo,"ax",%progbits
	.globl	new_foo
	.type	new_foo, %function
new_foo:
	.byte 0
	.symver new_foo,foo@@VERS_2.0
