# Motorola PowerPC BookE tests
#as: -mbooke
	.csect .text[PR]
	.csect main[DS]
main:
	.csect .text[PR]
.main:

	tlbre   1, 2, 7
	tlbwe   5, 30, 3
	icbt	5, 8, 9	
	mfapidi	5, 6
	tlbivax	7, 8
	tlbsx	11, 12
	rfci
	wrtee	3
	wrteei	1
	mfdcrx	4, 5
	mfdcr	5, 234
	mtdcrx	6, 7
	mtdcr	432, 8
	msync
	dcba	9, 10
	mbar	0
