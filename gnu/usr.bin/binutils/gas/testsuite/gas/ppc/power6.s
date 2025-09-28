# PowerPC POWER6 AltiVec tests
#as: -mpower6
	.text
start:
	doze
	nap
	sleep
	rvwinkle
	prtyw	3,4
	prtyd	13,14
	mfcfar	10
	mtcfar	11
	cmpb	3,4,5
	mffgpr	6,7
	mftgpr	8,9
	lwzcix	10,11,12
	lfdpx	12,14,15
	dadd	16,17,18
	daddq	20,22,24
	dss	3
	dssall	
	dst	5,4,1
	dstt	8,7,0
	dstst	5,6,3
	dststt	4,5,2
	attn
	mtcr    3
	mtcrf   0xff,3
	mtcrf   0x81,3
	mtcrf   0x01,3
	mtcrf   0x02,3
	mtcrf   0x04,3
	mtcrf   0x08,3
	mtcrf   0x10,3
	mtcrf   0x20,3
	mtcrf   0x40,3
	mtcrf   0x80,3
	mfcr    3
	mfcr    3,0x01
	mfcr    3,0x02
	mfcr    3,0x04
	mfcr    3,0x08
	mfcr    3,0x10
	mfcr    3,0x20
	mfcr    3,0x40
	mfcr    3,0x80
	dcbz    1, 2
	dcbzl   3, 4
	dcbz    5, 6
	mtfsf   6,10
	mtfsf.  6,11
	mtfsf   6,10,0,0
	mtfsf.  6,11,0,0
	mtfsf   6,10,0,1
	mtfsf.  6,11,0,1
	mtfsf   6,10,1,0
	mtfsf.  6,11,1,0
	mtfsfi  6,0
	mtfsfi. 6,15
	mtfsfi  6,0,0
	mtfsfi. 6,15,0
	mtfsfi  6,0,1
	mtfsfi. 6,15,1
	cbcdtd  10,11
	cdtbcd  10,11
	addg6s  10,11,12
	ori	1,1,0
	.p2align 4,,15
	slbia
	slbia   0
	slbia   7
	tlbie   10
	tlbie   10,0
	tlbie   10,1
