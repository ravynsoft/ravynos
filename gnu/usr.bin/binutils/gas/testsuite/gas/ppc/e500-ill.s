# Motorola PowerPC e500 illegal instructions
	.text
start:
	eciwx  3,4,5
	ecowx  3,4,5
	mfapidi 5, 6
	mfdcr   5, 234
	mtdcr   432, 8
	tlbia
	tlbie  3
