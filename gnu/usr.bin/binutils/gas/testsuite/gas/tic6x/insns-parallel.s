# Test parallel instructions.
.text
.nocmp
.globl f
f:
	nop 5
||	nop
	nop 4
||	nop 4
||	nop 4
