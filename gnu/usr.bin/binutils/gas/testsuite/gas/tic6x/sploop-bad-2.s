# Test bad SPLOOP instructions and operands.  Instructions not first
# in execute packet.
.text
.globl f
f:
	nop
||	sploop 1
	nop
	spkernel
	nop
||	sploopd 1
	nop
	spkernel
	nop
||	sploopw 1
	nop
	spkernel
	sploop 1
	nop
||	spkernel
	sploop 1
	nop
||	spkernel 0,0
	sploop 1
	nop
||	spkernelr
	nop
||	spmask
	nop
||	spmaskr
