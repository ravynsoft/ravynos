# Motorola PowerPC BookE tests
#as: -mbooke
	.text

branch_target_1:	
	icbt	5, 8, 9	
	mfapidi	5, 6
	tlbivax	7, 8
	tlbsx	11, 12
	tlbwe
	tlbwe	0,0,0
	tlbwe	1,1,1

branch_target_2:	
	rfci
	wrtee	3
	wrteei	1
	mfdcrx	4, 5
	mfdcr	5, 234
	mtdcrx	6, 7
	mtdcr	432, 8
	msync
	dcba	9, 10
	mbar
	mbar	0
	mbar	1

	tlbsx	12, 13, 14
	tlbsx.	12, 13, 14

	mfsprg 0, 2
	mfsprg2 0
	mtsprg 2, 0
	mtsprg2 0
	mfsprg 0, 7
	mfsprg7 0
	mtsprg 7, 0
	mtsprg7 0

	dcbt 5,6
