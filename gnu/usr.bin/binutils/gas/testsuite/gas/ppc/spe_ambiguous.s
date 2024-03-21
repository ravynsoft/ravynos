# PA SPE instructions
	.section ".text"
	.equ	rA,1
	.equ	rB,2
	.equ	rD,0
	.equ	rS,0
	.equ	rT,0
	.equ	UIMM, 31
	.equ	UIMM_2, 2
	.equ	UIMM_4, 4
	.equ	UIMM_8, 8
	.equ	SIMM, -16
	.equ	crD, 0
	.equ	crS, 0

	evsubfw         rS, rA, rB
	evsubw          rS, rB, rA
	evsubifw        rS, UIMM, rB
	evsubiw         rS, rB, UIMM
	evnor           rS, rA, rA
	evnot           rS, rA
