# Test C674x SPLOOP instructions.  The present tests are placeholders
# to verify encoding that may not be valid when the full set of checks
# for invalid input are implemented and may need changing to valid
# code at that point.
.text
.nocmp
.globl f
f:
	spmask
	spmask l1
	spmask L2
	spmask s1
	spmask S2
	spmask D1
	spmask d2
	spmask M1
	spmask m2
	spmask D1,L1
	spmask L1,D1
	spmask L1,S1,D1,M1,M2,D2,S2,L2
	spmask M1
||^	mv .L1 a0,a1
||	mv .D2 b0,b1
||^	mv .S1 a2,a3
	spmaskr
	spmaskr l1
	spmaskr L2
	spmaskr s1
	spmaskr S2
	spmaskr D1
	spmaskr d2
	spmaskr M1
	spmaskr m2
	spmaskr D1,L1
	spmaskr L1,D1
	spmaskr L1,S1,D1,M1,M2,D2,S2,L2
	spmaskr M1
||^	mv .L1 a0,a1
||	mv .D2 b0,b1
||^	mv .S1 a2,a3
	[a0] sploop 1
	nop
	spkernelr
	[b0] sploopd 1
	nop
	spkernel
	[!a0] sploopw 1
	nop
	spkernel
	sploop 1
	nop
	spkernel 0,0
	sploop 1
	nop
	spkernel 63,0
	sploop 2
	nop
	spkernel 31,0
	sploop 2
	nop
	spkernel 31,1
	sploop 3
	nop
	spkernel 15,2
	sploop 4
	nop
	spkernel 15,3
	sploop 5
	nop
	spkernel 7,4
	sploop 8
	nop
	spkernel 7,7
	sploop 9
	nop
	spkernel 3,8
	sploop 14
	nop
	spkernel 3,13
	sploop 1
	nop
	spkernel 8,0
	sploop 2
	nop
	spkernel 6,0
