# Test bad SPLOOP instructions and operands.  Bad operands (generic errors).
.text
.globl f
f:
	spkernel 1,2,3
	spkernel .L1
	spkernelr .S1
	spkernelr 1
	sploop .D1
	sploop
	sploop 0
	sploop 15
	sploopd .S1
	sploopd
	sploopd 0
	sploopd 15
	sploopw .L1
	sploopw
	sploopw 0
	sploopw 15
	spmask .M1
	spmask X1
	spmask L1,L1,L1,L1,L1,L1,L1,L1,L1
	spmaskr .M1
	spmaskr X1
	spmaskr L1,L1,L1,L1,L1,L1,L1,L1,L1
