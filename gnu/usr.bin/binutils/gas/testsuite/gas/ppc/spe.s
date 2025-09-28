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

	evaddw          rS, rA, rB
	evaddiw         rS, rB, UIMM
	evsubfw         rS, rA, rB
	evsubw          rS, rB, rA
	evsubifw        rS, UIMM, rB
	evsubiw         rS, rB, UIMM
	evabs           rS, rA
	evneg           rS, rA
	evextsb         rS, rA
	evextsh         rS, rA
	evrndw          rS, rA
	evcntlzw        rS, rA
	evcntlsw        rS, rA
	brinc           rS, rA, rB
	evand           rS, rA, rB
	evandc          rS, rA, rB
	evxor           rS, rA, rB
	evmr            rS, rA
	evor            rS, rA, rA
	evor            rS, rA, rB
	evnor           rS, rA, rB
	evnot           rS, rA
	evnor           rS, rA, rA
	eveqv           rS, rA, rB
	evorc           rS, rA, rB
	evnand          rS, rA, rB
	evsrwu          rS, rA, rB
	evsrws          rS, rA, rB
	evsrwiu         rS, rA, UIMM
	evsrwis         rS, rA, UIMM
	evslw           rS, rA, rB
	evslwi          rS, rA, UIMM
	evrlw           rS, rA, rB
	evsplati        rS, SIMM
	evrlwi          rS, rA, UIMM
	evsplatfi       rS, SIMM
	evmergehi       rS, rA, rB
	evmergelo       rS, rA, rB
	evmergehilo     rS, rA, rB
	evmergelohi     rS, rA, rB
	evcmpgtu        crD, rA, rB
	evcmpgts        crD, rA, rB
	evcmpltu        crD, rA, rB
	evcmplts        crD, rA, rB
	evcmpeq         crD, rA, rB
	evsel           rS, rA, rB, crS
	evfsadd         rS, rA, rB
	evfssub         rS, rA, rB
	evfsmadd        rS, rA, rB
	evfsmsub        rS, rA, rB
	evfsabs         rS, rA
	evfsnabs        rS, rA
	evfsneg         rS, rA
	evfsmul         rS, rA, rB
	evfsdiv         rS, rA, rB
	evfsnmadd       rS, rA, rB
	evfsnmsub       rS, rA, rB
	evfscmpgt       crD, rA, rB
	evfscmplt       crD, rA, rB
	evfscmpeq       crD, rA, rB
	evfscfui        rS, rB
	evfscfsi        rS, rB
	evfscfuf        rS, rB
	evfscfsf        rS, rB
	evfsctui        rS, rB
	evfsctsi        rS, rB
	evfsctuf        rS, rB
	evfsctsf        rS, rB
	evfsctuiz       rS, rB
	evfsctsiz       rS, rB
	evfststgt       crD, rA, rB
	evfststlt       crD, rA, rB
	evfststeq       crD, rA, rB
	evlddx          rS, rA, rB
	evldd           rS, UIMM_8(rA)
	evldwx          rS, rA, rB
	evldw           rS, UIMM_8(rA)
	evldhx          rS, rA, rB
	evldh           rS, UIMM_8(rA)
	evlhhesplatx    rS, rA, rB
	evlhhesplat     rS, UIMM_2(rA)
	evlhhousplatx   rS, rA, rB
	evlhhousplat    rS, UIMM_2(rA)
	evlhhossplatx   rS, rA, rB
	evlhhossplat    rS, UIMM_2(rA)
	evlwhex         rS, rA, rB
	evlwhe          rS, UIMM_4(rA)
	evlwhoux        rS, rA, rB
	evlwhou         rS, UIMM_4(rA)
	evlwhosx        rS, rA, rB
	evlwhos         rS, UIMM_4(rA)
	evlwwsplatx     rS, rA, rB
	evlwwsplat      rS, UIMM_4(rA)
	evlwhsplatx     rS, rA, rB
	evlwhsplat      rS, UIMM_4(rA)
	evstddx         rS, rA, rB
	evstdd          rS, UIMM_8(rA)
	evstdwx         rS, rA, rB
	evstdw          rS, UIMM_8(rA)
	evstdhx         rS, rA, rB
	evstdh          rS, UIMM_8(rA)
	evstwhex        rS, rA, rB
	evstwhe         rS, UIMM_4(rA)
	evstwhox        rS, rA, rB
	evstwho         rS, UIMM_4(rA)
	evstwwex        rS, rA, rB
	evstwwe         rS, UIMM_4(rA)
	evstwwox        rS, rA, rB
	evstwwo         rS, UIMM_4(rA)
	evmhessf        rS, rA, rB
	evmhossf        rS, rA, rB
	evmheumi        rS, rA, rB
	evmhesmi        rS, rA, rB
	evmhesmf        rS, rA, rB
	evmhoumi        rS, rA, rB
	evmhosmi        rS, rA, rB
	evmhosmf        rS, rA, rB
	evmhessfa       rS, rA, rB
	evmhossfa       rS, rA, rB
	evmheumia       rS, rA, rB
	evmhesmia       rS, rA, rB
	evmhesmfa       rS, rA, rB
	evmhoumia       rS, rA, rB
	evmhosmia       rS, rA, rB
	evmhosmfa       rS, rA, rB
	evmwlssf        rD, rA, rB
	evmwhssf        rS, rA, rB
	evmwlumi        rS, rA, rB
	evmwlsmf        rD, rA, rB
	evmwhumi        rS, rA, rB
	evmwhsmi        rS, rA, rB
	evmwhsmf        rS, rA, rB
	evmwssf         rS, rA, rB
	evmwumi         rS, rA, rB
	evmwsmi         rS, rA, rB
	evmwsmf         rS, rA, rB
	evmwlssfa       rD, rA, rB
	evmwhssfa       rS, rA, rB
	evmwlumia       rS, rA, rB
	evmwlsmfa       rD, rA, rB
	evmwhumia       rS, rA, rB
	evmwhsmia       rS, rA, rB
	evmwhsmfa       rS, rA, rB
	evmwssfa        rS, rA, rB
	evmwumia        rS, rA, rB
	evmwsmia        rS, rA, rB
	evmwsmfa        rS, rA, rB
	evaddusiaaw     rS, rA
	evaddssiaaw     rS, rA
	evsubfusiaaw    rS, rA
	evsubfssiaaw    rS, rA
	evmra           rS, rA
	evdivws         rS, rA, rB
	evdivwu         rS, rA, rB
	evaddumiaaw     rS, rA
	evaddsmiaaw     rS, rA
	evsubfumiaaw    rS, rA
	evsubfsmiaaw    rS, rA
	evmheusiaaw     rS, rA, rB
	evmhessiaaw     rS, rA, rB
	evmhessfaaw     rS, rA, rB
	evmhousiaaw     rS, rA, rB
	evmhossiaaw     rS, rA, rB
	evmhossfaaw     rS, rA, rB
	evmheumiaaw     rS, rA, rB
	evmhesmiaaw     rS, rA, rB
	evmhesmfaaw     rS, rA, rB
	evmhoumiaaw     rS, rA, rB
	evmhosmiaaw     rS, rA, rB
	evmhosmfaaw     rS, rA, rB
	evmhegumiaa     rS, rA, rB
	evmhegsmiaa     rS, rA, rB
	evmhegsmfaa     rS, rA, rB
	evmhogumiaa     rS, rA, rB
	evmhogsmiaa     rS, rA, rB
	evmhogsmfaa     rS, rA, rB
	evmwlusiaaw     rS, rA, rB
	evmwlssiaaw     rS, rA, rB
	evmwlssfaaw     rD, rA, rB
	evmwhusiaa      rD, rA, rB
	evmwhssmaa      rD, rA, rB
	evmwhssfaa      rD, rA, rB
	evmwlumiaaw     rS, rA, rB
	evmwlsmiaaw     rS, rA, rB
	evmwlsmfaaw     rD, rA, rB
	evmwhumiaa      rD, rA, rB
	evmwhsmiaa      rD, rA, rB
	evmwhsmfaa      rD, rA, rB
	evmwssfaa       rS, rA, rB
	evmwumiaa       rS, rA, rB
	evmwsmiaa       rS, rA, rB
	evmwsmfaa       rS, rA, rB
	evmwhgumiaa     rD, rA, rB
	evmwhgsmiaa     rD, rA, rB
	evmwhgssfaa     rD, rA, rB
	evmwhgsmfaa     rD, rA, rB
	evmheusianw     rS, rA, rB
	evmhessianw     rS, rA, rB
	evmhessfanw     rS, rA, rB
	evmhousianw     rS, rA, rB
	evmhossianw     rS, rA, rB
	evmhossfanw     rS, rA, rB
	evmheumianw     rS, rA, rB
	evmhesmianw     rS, rA, rB
	evmhesmfanw     rS, rA, rB
	evmhoumianw     rS, rA, rB
	evmhosmianw     rS, rA, rB
	evmhosmfanw     rS, rA, rB
	evmhegumian     rS, rA, rB
	evmhegsmian     rS, rA, rB
	evmhegsmfan     rS, rA, rB
	evmhogumian     rS, rA, rB
	evmhogsmian     rS, rA, rB
	evmhogsmfan     rS, rA, rB
	evmwlusianw     rS, rA, rB
	evmwlssianw     rS, rA, rB
	evmwlssfanw     rD, rA, rB
	evmwhusian      rD, rA, rB
	evmwhssian      rD, rA, rB
	evmwhssfan      rD, rA, rB
	evmwlumianw     rS, rA, rB
	evmwlsmianw     rS, rA, rB
	evmwlsmfanw     rD, rA, rB
	evmwhumian      rD, rA, rB
	evmwhsmian      rD, rA, rB
	evmwhsmfan      rD, rA, rB
	evmwssfan       rS, rA, rB
	evmwumian       rS, rA, rB
	evmwsmian       rS, rA, rB
	evmwsmfan       rS, rA, rB
	evmwhgumian     rD, rA, rB
	evmwhgsmian     rD, rA, rB
	evmwhgssfan     rD, rA, rB
	evmwhgsmfan     rD, rA, rB
	evlddepx        rT, rA, rB
	evstddepx       rT, rA, rB

;#SPE mapped by macro
	evsadd          rS, rA, rB
	evssub          rS, rA, rB
	evsabs          rS, rA
	evsnabs         rS, rA
	evsneg          rS, rA
	evsmul          rS, rA, rB
	evsdiv          rS, rA, rB
	evscmpgt        crD, rA, rB
	evsgmplt        crD, rA, rB
	evsgmpeq        crD, rA, rB
	evscfui         rS, rB
	evscfsi         rS, rB
	evscfuf         rS, rB
	evscfsf         rS, rB
	evsctui         rS, rB
	evsctsi         rS, rB
	evsctuf         rS, rB
	evsctsf         rS, rB
	evsctuiz        rS, rB
	evsctsiz        rS, rB
	evststgt        crD, rA, rB
	evststlt        crD, rA, rB
	evststeq        crD, rA, rB
