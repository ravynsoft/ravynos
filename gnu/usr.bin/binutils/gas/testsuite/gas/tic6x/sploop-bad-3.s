# Test bad SPLOOP instructions and operands.  SPKERNEL not after SPLOOP.
.text
.globl f
f:
	spkernel
	spkernel 0,0
	spkernelr
