	.section        __sancov_cntrs,"aG",%progbits,foo1,comdat
	.long 0
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
	.long 0
	.section        __sancov_cntrs,"aG",%progbits,foo2,comdat
	.long 1
	.section .text,"axG",%progbits,foo2,comdat
 .ifdef UNDERSCORE
	.globl _foo2
	.type _foo2, %function
_foo2:
	.long 1
 .else
	.globl foo2
	.type foo2, %function
foo2:
 .endif
