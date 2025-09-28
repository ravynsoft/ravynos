# Test bad SPLOOP instructions and operands.  Bad use of ||^.
.text
.globl f
f:
	nop
||^	mv .L1 a2,a3
	spmask
||^	nop
