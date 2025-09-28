# Test bad SPLOOP instructions and operands.  SPKERNEL operands out of
# range.
.text
.globl f
f:
	sploop 1
	spkernel -1,0
	spkernel 0,-1
	spkernel 0,1
	spkernel 64,0
	spkernel
	sploop 2
	spkernel -1,0
	spkernel 0,-1
	spkernel 0,2
	spkernel 32,0
	spkernel
	sploop 3
	spkernel -1,0
	spkernel 0,-1
	spkernel 0,3
	spkernel 16,0
	spkernel
	sploop 4
	spkernel -1,0
	spkernel 0,-1
	spkernel 0,4
	spkernel 16,0
	spkernel
	sploop 5
	spkernel -1,0
	spkernel 0,-1
	spkernel 0,5
	spkernel 8,0
	spkernel
	sploop 9
	spkernel -1,0
	spkernel 0,-1
	spkernel 0,9
	spkernel 4,0
	spkernel
