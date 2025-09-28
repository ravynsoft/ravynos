# Test PA LSP operands checks
	.section ".text"

	.equ	rA,1
	.equ	rB,2
	.equ	rD,0           ;# ok
	.equ	rD_odd, 1      ;# GPR odd is illegal
	.equ	rS,0           ;# ok
	.equ	rS_odd, 1      ;# GPR odd is illegal
	.equ	UIMM_GT15, 16  ;# UIMM values >15 are illegal
	.equ	UIMM_2, 2      ;# ok
	.equ	UIMM_2_ILL, 3  ;# 3 is not a multiple of 2
	.equ	UIMM_2_ZERO, 0 ;# UIMM = 00000 is illegal if U=1
	.equ	UIMM_4, 4      ;# ok
	.equ	UIMM_4_ILL, 3  ;# 3 is not a multiple of 4
	.equ	UIMM_4_ZERO, 0 ;# UIMM = 00000 is illegal if U=1
	.equ	UIMM_8, 8      ;# ok
	.equ	UIMM_8_ILL, 7  ;# 7 is not a multiple of 8
	.equ	UIMM_8_ZERO, 0 ;# UIMM = 00000 is illegal if U=1
	.equ	offset, 0      ;# invalid offset

	zxtrw             rD, rA, rB, offset
	zvsrhiu           rD, rA, UIMM_GT15
	zvsrhis           rD, rA, UIMM_GT15
	zvslhi            rD, rA, UIMM_GT15
	zvrlhi            rD, rA, UIMM_GT15
	zvslhius          rD, rA, UIMM_GT15
	zvslhiss          rD, rA, UIMM_GT15
	zldd              rD_odd, UIMM_8(rA)
	zldd              rD, UIMM_8_ILL(rA)
	zldw              rD_odd, UIMM_8(rA)
	zldw              rD, UIMM_8_ILL(rA)
	zldh              rD_odd, UIMM_8(rA)
	zldh              rD, UIMM_8_ILL(rA)
	zlwgsfd           rD_odd, UIMM_4(rA)
	zlwgsfd           rD, UIMM_4_ILL(rA)
	zlwwosd           rD_odd, UIMM_4(rA)
	zlwwosd           rD, UIMM_4_ILL(rA)
	zlwhsplatwd       rD_odd, UIMM_4(rA)
	zlwhsplatwd       rD, UIMM_4_ILL(rA)
	zlwhsplatd        rD_odd, UIMM_4(rA)
	zlwhsplatd        rD, UIMM_4_ILL(rA)
	zlwhgwsfd         rD_odd, UIMM_4(rA)
	zlwhgwsfd         rD, UIMM_4_ILL(rA)
	zlwhed            rD_odd, UIMM_4(rA)
	zlwhed            rD, UIMM_4_ILL(rA)
	zlwhosd           rD_odd, UIMM_4(rA)
	zlwhosd           rD, UIMM_4_ILL(rA)
	zlwhoud           rD_odd, UIMM_4(rA)
	zlwh              rD, UIMM_4_ILL(rA)
	zlww              rD, UIMM_4_ILL(rA)
	zlhgwsf           rD, UIMM_2_ILL(rA)
	zlhhsplat         rD, UIMM_2_ILL(rA)
	zstdd             rS_odd, UIMM_8(rA)
	zstdd             rS, UIMM_8_ILL(rA)
	zstdw             rS_odd, UIMM_8(rA)
	zstdw             rS, UIMM_8_ILL(rA)
	zstdh             rS_odd, UIMM_8(rA)
	zstdh             rS, UIMM_8_ILL(rA)
	zstwhed           rS_odd, UIMM_4(rA)
	zstwhed           rS, UIMM_4_ILL(rA)
	zstwhod           rS_odd, UIMM_4(rA)
	zstwhod           rS, UIMM_4_ILL(rA)
	zlhhe             rD, UIMM_2_ILL(rA)
	zlhhos            rD, UIMM_2_ILL(rA)
	zlhhou            rD, UIMM_2_ILL(rA)
	zsthe             rS, UIMM_2_ILL(rA)
	zstho             rS, UIMM_2_ILL(rA)
	zstwh             rS, UIMM_4_ILL(rA)
	zstww             rS, UIMM_4_ILL(rA)
	zlddu             rD_odd, UIMM_8(rA)
	zlddu             rD, UIMM_8_ZERO(rA)
	zldwu             rD_odd, UIMM_8(rA)
	zldwu             rD, UIMM_8_ZERO(rA)
	zldhu             rD_odd, UIMM_8(rA)
	zldhu             rD, UIMM_8_ZERO(rA)
	zlwgsfdu          rD_odd, UIMM_4(rA)
	zlwgsfdu          rD, UIMM_4_ZERO(rA)
	zlwwosdu          rD_odd, UIMM_4(rA)
	zlwwosdu          rD, UIMM_4_ZERO(rA)
	zlwhsplatwdu      rD_odd, UIMM_4(rA)
	zlwhsplatwdu      rD, UIMM_4_ZERO(rA)
	zlwhsplatdu       rD_odd, UIMM_4(rA)
	zlwhsplatdu       rD, UIMM_4_ZERO(rA)
	zlwhgwsfdu        rD_odd, UIMM_4(rA)
	zlwhgwsfdu        rD, UIMM_4_ZERO(rA)
	zlwhedu           rD_odd, UIMM_4(rA)
	zlwhedu           rD, UIMM_4_ZERO(rA)
	zlwhosdu          rD_odd, UIMM_4(rA)
	zlwhosdu          rD, UIMM_4_ZERO(rA)
	zlwhoudu          rD_odd, UIMM_4(rA)
	zlwhoudu          rD, UIMM_4_ZERO(rA)
	zlwhu             rD, UIMM_4_ZERO(rA)
	zlwwu             rD, UIMM_4_ZERO(rA)
	zlhgwsfu          rD, UIMM_2_ZERO(rA)
	zlhhsplatu        rD, UIMM_2_ZERO(rA)
	zstddu            rS, UIMM_8_ZERO(rA)
	zstdwu            rS_odd, UIMM_8(rA)
	zstdwu            rS, UIMM_8_ZERO(rA)
	zstdhu            rS_odd, UIMM_8(rA)
	zstdhu            rS, UIMM_8_ZERO(rA)
	zstwhedu          rS_odd, UIMM_4(rA)
	zstwhedu          rS, UIMM_4_ZERO(rA)
	zstwhodu          rS_odd, UIMM_4(rA)
	zstwhodu          rS, UIMM_4_ZERO(rA)
	zlhheu            rD, UIMM_2_ZERO(rA)
	zlhhosu           rD, UIMM_2_ZERO(rA)
	zlhhouu           rD, UIMM_2_ZERO(rA)
	zstheu            rS, UIMM_2_ZERO(rA)
	zsthou            rS, UIMM_2_ZERO(rA)
	zstwhu            rS, UIMM_4_ZERO(rA)
	zstwwu            rS, UIMM_4_ZERO(rA)
