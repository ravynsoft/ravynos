# Test bad SPLOOP instructions and operands.  Duplicate masking.
.text
.globl f
f:
	spmask L1,L1
	spmaskr D1,D1
	spmask L1
||^	mv .L1 a1,a2
	spmaskr S1
||^	mv .S1 a1,a2
