	.section .text,"axG",%progbits,foo1,comdat
 .ifdef UNDERSCORE
	.globl _foo1
	.type _foo1, %function
_foo1:
 .else
	.globl foo1
	.type foo1, %function
foo1:
 .endif
	.byte 0
