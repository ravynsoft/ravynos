# Test too many instructions in execute packet, with section switching.
.nocmp
.section .text.f1,"ax",%progbits
.globl f1
f1:
	nop 5
.section .text.f2,"ax",%progbits
.globl f2
f2:
	nop 4
.section .text.f1,"ax",%progbits
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
.section .text.f2,"ax",%progbits
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
||	nop
.section .text.f1,"ax",%progbits
||	nop
.section .text.f2,"ax",%progbits
||	nop
