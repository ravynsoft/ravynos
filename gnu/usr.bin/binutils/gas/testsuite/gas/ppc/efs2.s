# PA EFS2 instructions in accordance with EFP2_rev.1.4_spec
# CMPE200GCC-62
	.section ".text"

	.equ	rA,1
	.equ	rB,2
	.equ	rD,0

	efsmax     rD, rA, rB
	efsmin     rD, rA, rB
	efdmax     rD, rA, rB
	efdmin     rD, rA, rB
	efssqrt    rD, rA
	efscfh     rD, rB
	efscth     rD, rB
	efdsqrt    rD, rA
	efdcfh     rD, rB
	efdcth     rD, rB
