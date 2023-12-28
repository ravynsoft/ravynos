# PA SPE2 instructions
	.section ".text"

	.equ	rA,1
	.equ	rB,2
	.equ	rD,0
	.equ	rS,0
	.equ	UIMM_ILL, 32
	.equ	UIMM_1_ZERO, 0
	.equ	UIMM_1_ILL, 32
	.equ	UIMM_2_ILL, 1
	.equ	UIMM_4_ILL, 3
	.equ	UIMM_8_ILL, 7
	.equ	UIMM_GT7,   8
	.equ	UIMM_GT15,  16
	.equ	nnn_ILL,     8
	.equ	bbb_ILL,     8
	.equ	dd,          3
	.equ	dd_ILL,      4
	.equ	Ddd,         7
	.equ	Ddd_ILL,     8
	.equ	hh,          3
	.equ	hh_ILL,      4
	.equ	mask_ILL,   16
	.equ	offset_ILL0, 0
	.equ	offset_ILL,  8


	evaddib               rD, rB, UIMM_ILL
	evaddih               rD, rB, UIMM_ILL
	evsubifh              rD, UIMM_ILL, rB
	evsubifb              rD, UIMM_ILL, rB
	evinsb                rD, rA, Ddd, bbb_ILL
	evxtrb                rD, rA, Ddd, bbb_ILL
	evsplath              rD, rA, hh_ILL
	evsplatb              rD, rA, bbb_ILL
	evinsh                rD, rA, dd_ILL, hh
	evclrbe               rD, rA, mask_ILL
	evclrbo               rD, rA, mask_ILL
	evclrh                rD, rA, mask_ILL
	evxtrh                rD, rA, dd_ILL, hh
	evxtrh                rD, rA, dd, hh_ILL
	evxtrd                rD, rA, rB, offset_ILL0
	evxtrd                rD, rA, rB, offset_ILL
	evsrbiu               rD, rA, UIMM_GT7
	evsrbis               rD, rA, UIMM_GT7
	evslbi                rD, rA, UIMM_GT7
	evrlbi                rD, rA, UIMM_GT7
	evsrhiu               rD, rA, UIMM_GT15
	evsrhis               rD, rA, UIMM_GT15
	evslhi                rD, rA, UIMM_GT15
	evrlhi                rD, rA, UIMM_GT15
	evsroiu               rD, rA, nnn_ILL
	evsrois               rD, rA, nnn_ILL
	evsloi                rD, rA, nnn_ILL
	evldb                 rD, UIMM_8_ILL (rA)
	evlhhsplath           rD, UIMM_2_ILL (rA)
	evlwbsplatw           rD, UIMM_4_ILL (rA)
	evlwhsplatw           rD, UIMM_4_ILL (rA)
	evlbbsplatb           rD, UIMM_1_ILL (rA)
	evstdb                rS, UIMM_8_ILL (rA)
	evlwbe                rD, UIMM_4_ILL (rA)
	evlwbou               rD, UIMM_4_ILL (rA)
	evlwbos               rD, UIMM_4_ILL (rA)
	evstwbe               rS, UIMM_4_ILL (rA)
	evstwbo               rS, UIMM_4_ILL (rA)
	evstwb                rS, UIMM_4_ILL (rA)
	evsthb                rS, UIMM_2_ILL (rA)
	evlddu                rD, UIMM_8_ILL (rA)
	evldwu                rD, UIMM_8_ILL (rA)
	evldhu                rD, UIMM_8_ILL (rA)
	evldbu                rD, UIMM_8_ILL (rA)
	evlhhesplatu          rD, UIMM_2_ILL (rA)
	evlhhsplathu          rD, UIMM_2_ILL (rA)
	evlhhousplatu         rD, UIMM_2_ILL (rA)
	evlhhossplatu         rD, UIMM_2_ILL (rA)
	evlwheu               rD, UIMM_4_ILL (rA)
	evlwbsplatwu          rD, UIMM_4_ILL (rA)
	evlwhouu              rD, UIMM_4_ILL (rA)
	evlwhosu              rD, UIMM_4_ILL (rA)
	evlwwsplatu           rD, UIMM_4_ILL (rA)
	evlwhsplatwu          rD, UIMM_4_ILL (rA)
	evlwhsplatu           rD, UIMM_4_ILL (rA)
	evlbbsplatbu          rD, UIMM_1_ZERO (rA)
	evstddu               rS, UIMM_8_ILL (rA)
	evstdwu               rS, UIMM_8_ILL (rA)
	evstdhu               rS, UIMM_8_ILL (rA)
	evstdbu               rS, UIMM_8_ILL (rA)
	evlwbeu               rD, UIMM_4_ILL (rA)
	evlwbouu              rD, UIMM_4_ILL (rA)
	evlwbosu              rD, UIMM_4_ILL (rA)
	evstwheu              rS, UIMM_4_ILL (rA)
	evstwbeu              rS, UIMM_4_ILL (rA)
	evstwhou              rS, UIMM_4_ILL (rA)
	evstwbou              rS, UIMM_4_ILL (rA)
	evstwweu              rS, UIMM_4_ILL (rA)
	evstwbu               rS, UIMM_4_ILL (rA)
	evstwwou              rS, UIMM_4_ILL (rA)
	evsthbu               rS, UIMM_2_ILL (rA)
