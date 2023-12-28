# PA EFS 1.0 and 1.1 instructions
# CMPE200GCC-62
	.section ".text"

	.equ	rA,1
	.equ	rB,2
	.equ	rD,0

;# EFS 1.0 instructions in accordance with EFP2_rev.1.4_spec
	efscfsi   rD, rB
	efsctsi   rD, rB
	efdcfsi   rD, rB
	efdctsi   rD, rB

;# EFS 1.1 instructions in accordance with EFP2_rev.1.4_spec
	efsmadd   rD, rA, rB
	efsmsub   rD, rA, rB
	efsnmadd  rD, rA, rB
	efsnmsub  rD, rA, rB
	efdmadd   rD, rA, rB
	efdmsub   rD, rA, rB
	efdnmadd  rD, rA, rB
	efdnmsub  rD, rA, rB

;# moved EFS opcodes in accordance with EFP2_rev.1.4_spec
	efdcfuid  rD, rB
	efdcfsid  rD, rB
	efdctuidz rD, rB
	efdctsidz rD, rB
