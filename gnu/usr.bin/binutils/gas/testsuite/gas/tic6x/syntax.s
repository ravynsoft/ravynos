# Test variants of assembler syntax.
* Another comment.
.text
.nocmp
	.globl f ; comment here as well
  f:	nop;comment
	nop 5
 nop  4- 2
nop (1+2) ; 3
NoP 5
NOP 2
nop 4 @ nop 2
ABS .l1 a4 , a11
ABS .L2x A5, B13
add .S1 0xffffffff,a4,a11
