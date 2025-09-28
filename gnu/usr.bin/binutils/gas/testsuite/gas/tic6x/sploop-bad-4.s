# Test bad SPLOOP instructions and operands.  SPLOOP after SPLOOP.
.text
.globl f
f:
	sploop 1
	sploop 1
	nop
	spkernel
	sploop 1
	sploopd 1
	spkernel
	sploop 1
	sploopw 1
	spkernel
