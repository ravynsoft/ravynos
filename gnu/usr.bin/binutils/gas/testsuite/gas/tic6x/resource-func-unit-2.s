# Test parallel instructions on same functional unit, with section switching.
.section .text.f1,"ax",%progbits
.globl f1
f1:
	add .L1 a1,a2,a3
	nop
	|| [b1] add .L1 a1,a2,a3
.section .text.f2,"ax",%progbits
.globl f2
f2:
	add .L2 b1,b2,b3
	nop
	|| [b1] add .L2 b1,b2,b3
	|| nop
.section .text.f1,"ax",%progbits
	|| nop
	|| [!b1] add .L1 a4,a5,a6
.section .text.f2,"ax",%progbits
	|| [!b1] add .L2 b4,b5,b6
