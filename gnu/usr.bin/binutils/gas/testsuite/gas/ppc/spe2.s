# PA SPE2 instructions
# Testcase for CMPE200GCC-5, CMPE200GCC-62

	.section ".text"

	.equ	rA,1
	.equ	rB,2
	.equ	rD,0
	.equ	rS,0
	.equ	UIMM, 31
	.equ	UIMM_LT8, 7
	.equ	UIMM_LT16, 15
	.equ	UIMM_1, 1
	.equ	UIMM_2, 2
	.equ	UIMM_4, 4
	.equ	UIMM_8, 8
	.equ	SIMM, -16
	.equ	crD,    0
	.equ	nnn,    7
	.equ	bbb,    7
	.equ	dd,     3
	.equ	Ddd,    7
	.equ	hh,     3
	.equ	mask,  15
	.equ	offset, 7

	evdotpwcssi           rD, rA, rB
	evdotpwcsmi           rD, rA, rB
	evdotpwcssfr          rD, rA, rB
	evdotpwcssf           rD, rA, rB
	evdotpwgasmf          rD, rA, rB
	evdotpwxgasmf         rD, rA, rB
	evdotpwgasmfr         rD, rA, rB
	evdotpwxgasmfr        rD, rA, rB
	evdotpwgssmf          rD, rA, rB
	evdotpwxgssmf         rD, rA, rB
	evdotpwgssmfr         rD, rA, rB
	evdotpwxgssmfr        rD, rA, rB
	evdotpwcssiaaw3       rD, rA, rB
	evdotpwcsmiaaw3       rD, rA, rB
	evdotpwcssfraaw3      rD, rA, rB
	evdotpwcssfaaw3       rD, rA, rB
	evdotpwgasmfaa3       rD, rA, rB
	evdotpwxgasmfaa3      rD, rA, rB
	evdotpwgasmfraa3      rD, rA, rB
	evdotpwxgasmfraa3     rD, rA, rB
	evdotpwgssmfaa3       rD, rA, rB
	evdotpwxgssmfaa3      rD, rA, rB
	evdotpwgssmfraa3      rD, rA, rB
	evdotpwxgssmfraa3     rD, rA, rB
	evdotpwcssia          rD, rA, rB
	evdotpwcsmia          rD, rA, rB
	evdotpwcssfra         rD, rA, rB
	evdotpwcssfa          rD, rA, rB
	evdotpwgasmfa         rD, rA, rB
	evdotpwxgasmfa        rD, rA, rB
	evdotpwgasmfra        rD, rA, rB
	evdotpwxgasmfra       rD, rA, rB
	evdotpwgssmfa         rD, rA, rB
	evdotpwxgssmfa        rD, rA, rB
	evdotpwgssmfra        rD, rA, rB
	evdotpwxgssmfra       rD, rA, rB
	evdotpwcssiaaw        rD, rA, rB
	evdotpwcsmiaaw        rD, rA, rB
	evdotpwcssfraaw       rD, rA, rB
	evdotpwcssfaaw        rD, rA, rB
	evdotpwgasmfaa        rD, rA, rB
	evdotpwxgasmfaa       rD, rA, rB
	evdotpwgasmfraa       rD, rA, rB
	evdotpwxgasmfraa      rD, rA, rB
	evdotpwgssmfaa        rD, rA, rB
	evdotpwxgssmfaa       rD, rA, rB
	evdotpwgssmfraa       rD, rA, rB
	evdotpwxgssmfraa      rD, rA, rB
	evdotphihcssi         rD, rA, rB
	evdotplohcssi         rD, rA, rB
	evdotphihcssf         rD, rA, rB
	evdotplohcssf         rD, rA, rB
	evdotphihcsmi         rD, rA, rB
	evdotplohcsmi         rD, rA, rB
	evdotphihcssfr        rD, rA, rB
	evdotplohcssfr        rD, rA, rB
	evdotphihcssiaaw3     rD, rA, rB
	evdotplohcssiaaw3     rD, rA, rB
	evdotphihcssfaaw3     rD, rA, rB
	evdotplohcssfaaw3     rD, rA, rB
	evdotphihcsmiaaw3     rD, rA, rB
	evdotplohcsmiaaw3     rD, rA, rB
	evdotphihcssfraaw3    rD, rA, rB
	evdotplohcssfraaw3    rD, rA, rB
	evdotphihcssia        rD, rA, rB
	evdotplohcssia        rD, rA, rB
	evdotphihcssfa        rD, rA, rB
	evdotplohcssfa        rD, rA, rB
	evdotphihcsmia        rD, rA, rB
	evdotplohcsmia        rD, rA, rB
	evdotphihcssfra       rD, rA, rB
	evdotplohcssfra       rD, rA, rB
	evdotphihcssiaaw      rD, rA, rB
	evdotplohcssiaaw      rD, rA, rB
	evdotphihcssfaaw      rD, rA, rB
	evdotplohcssfaaw      rD, rA, rB
	evdotphihcsmiaaw      rD, rA, rB
	evdotplohcsmiaaw      rD, rA, rB
	evdotphihcssfraaw     rD, rA, rB
	evdotplohcssfraaw     rD, rA, rB
	evdotphausi           rD, rA, rB
	evdotphassi           rD, rA, rB
	evdotphasusi          rD, rA, rB
	evdotphassf           rD, rA, rB
	evdotphsssf           rD, rA, rB
	evdotphaumi           rD, rA, rB
	evdotphasmi           rD, rA, rB
	evdotphasumi          rD, rA, rB
	evdotphassfr          rD, rA, rB
	evdotphssmi           rD, rA, rB
	evdotphsssfr          rD, rA, rB
	evdotphausiaaw3       rD, rA, rB
	evdotphassiaaw3       rD, rA, rB
	evdotphasusiaaw3      rD, rA, rB
	evdotphassfaaw3       rD, rA, rB
	evdotphsssiaaw3       rD, rA, rB
	evdotphsssfaaw3       rD, rA, rB
	evdotphaumiaaw3       rD, rA, rB
	evdotphasmiaaw3       rD, rA, rB
	evdotphasumiaaw3      rD, rA, rB
	evdotphassfraaw3      rD, rA, rB
	evdotphssmiaaw3       rD, rA, rB
	evdotphsssfraaw3      rD, rA, rB
	evdotphausia          rD, rA, rB
	evdotphassia          rD, rA, rB
	evdotphasusia         rD, rA, rB
	evdotphassfa          rD, rA, rB
	evdotphsssfa          rD, rA, rB
	evdotphaumia          rD, rA, rB
	evdotphasmia          rD, rA, rB
	evdotphasumia         rD, rA, rB
	evdotphassfra         rD, rA, rB
	evdotphssmia          rD, rA, rB
	evdotphsssfra         rD, rA, rB
	evdotphausiaaw        rD, rA, rB
	evdotphassiaaw        rD, rA, rB
	evdotphasusiaaw       rD, rA, rB
	evdotphassfaaw        rD, rA, rB
	evdotphsssiaaw        rD, rA, rB
	evdotphsssfaaw        rD, rA, rB
	evdotphaumiaaw        rD, rA, rB
	evdotphasmiaaw        rD, rA, rB
	evdotphasumiaaw       rD, rA, rB
	evdotphassfraaw       rD, rA, rB
	evdotphssmiaaw        rD, rA, rB
	evdotphsssfraaw       rD, rA, rB
	evdotp4hgaumi         rD, rA, rB
	evdotp4hgasmi         rD, rA, rB
	evdotp4hgasumi        rD, rA, rB
	evdotp4hgasmf         rD, rA, rB
	evdotp4hgssmi         rD, rA, rB
	evdotp4hgssmf         rD, rA, rB
	evdotp4hxgasmi        rD, rA, rB
	evdotp4hxgasmf        rD, rA, rB
	evdotpbaumi           rD, rA, rB
	evdotpbasmi           rD, rA, rB
	evdotpbasumi          rD, rA, rB
	evdotp4hxgssmi        rD, rA, rB
	evdotp4hxgssmf        rD, rA, rB
	evdotp4hgaumiaa3      rD, rA, rB
	evdotp4hgasmiaa3      rD, rA, rB
	evdotp4hgasumiaa3     rD, rA, rB
	evdotp4hgasmfaa3      rD, rA, rB
	evdotp4hgssmiaa3      rD, rA, rB
	evdotp4hgssmfaa3      rD, rA, rB
	evdotp4hxgasmiaa3     rD, rA, rB
	evdotp4hxgasmfaa3     rD, rA, rB
	evdotpbaumiaaw3       rD, rA, rB
	evdotpbasmiaaw3       rD, rA, rB
	evdotpbasumiaaw3      rD, rA, rB
	evdotp4hxgssmiaa3     rD, rA, rB
	evdotp4hxgssmfaa3     rD, rA, rB
	evdotp4hgaumia        rD, rA, rB
	evdotp4hgasmia        rD, rA, rB
	evdotp4hgasumia       rD, rA, rB
	evdotp4hgasmfa        rD, rA, rB
	evdotp4hgssmia        rD, rA, rB
	evdotp4hgssmfa        rD, rA, rB
	evdotp4hxgasmia       rD, rA, rB
	evdotp4hxgasmfa       rD, rA, rB
	evdotpbaumia          rD, rA, rB
	evdotpbasmia          rD, rA, rB
	evdotpbasumia         rD, rA, rB
	evdotp4hxgssmia       rD, rA, rB
	evdotp4hxgssmfa       rD, rA, rB
	evdotp4hgaumiaa       rD, rA, rB
	evdotp4hgasmiaa       rD, rA, rB
	evdotp4hgasumiaa      rD, rA, rB
	evdotp4hgasmfaa       rD, rA, rB
	evdotp4hgssmiaa       rD, rA, rB
	evdotp4hgssmfaa       rD, rA, rB
	evdotp4hxgasmiaa      rD, rA, rB
	evdotp4hxgasmfaa      rD, rA, rB
	evdotpbaumiaaw        rD, rA, rB
	evdotpbasmiaaw        rD, rA, rB
	evdotpbasumiaaw       rD, rA, rB
	evdotp4hxgssmiaa      rD, rA, rB
	evdotp4hxgssmfaa      rD, rA, rB
	evdotpwausi           rD, rA, rB
	evdotpwassi           rD, rA, rB
	evdotpwasusi          rD, rA, rB
	evdotpwaumi           rD, rA, rB
	evdotpwasmi           rD, rA, rB
	evdotpwasumi          rD, rA, rB
	evdotpwssmi           rD, rA, rB
	evdotpwausiaa3        rD, rA, rB
	evdotpwassiaa3        rD, rA, rB
	evdotpwasusiaa3       rD, rA, rB
	evdotpwsssiaa3        rD, rA, rB
	evdotpwaumiaa3        rD, rA, rB
	evdotpwasmiaa3        rD, rA, rB
	evdotpwasumiaa3       rD, rA, rB
	evdotpwssmiaa3        rD, rA, rB
	evdotpwausia          rD, rA, rB
	evdotpwassia          rD, rA, rB
	evdotpwasusia         rD, rA, rB
	evdotpwaumia          rD, rA, rB
	evdotpwasmia          rD, rA, rB
	evdotpwasumia         rD, rA, rB
	evdotpwssmia          rD, rA, rB
	evdotpwausiaa         rD, rA, rB
	evdotpwassiaa         rD, rA, rB
	evdotpwasusiaa        rD, rA, rB
	evdotpwsssiaa         rD, rA, rB
	evdotpwaumiaa         rD, rA, rB
	evdotpwasmiaa         rD, rA, rB
	evdotpwasumiaa        rD, rA, rB
	evdotpwssmiaa         rD, rA, rB
	evaddib               rD, rB, UIMM
	evaddih               rD, rB, UIMM
	evsubifh              rD, UIMM, rB
	evsubifb              rD, UIMM, rB
	evabsb                rD, rA
	evabsh                rD, rA
	evabsd                rD, rA
	evabss                rD, rA
	evabsbs               rD, rA
	evabshs               rD, rA
	evabsds               rD, rA
	evnegwo               rD, rA
	evnegb                rD, rA
	evnegbo               rD, rA
	evnegh                rD, rA
	evnegho               rD, rA
	evnegd                rD, rA
	evnegs                rD, rA
	evnegwos              rD, rA
	evnegbs               rD, rA
	evnegbos              rD, rA
	evneghs               rD, rA
	evneghos              rD, rA
	evnegds               rD, rA
	evextzb               rD, rA
	evextsbh              rD, rA
	evextsw               rD, rA
	evrndwh               rD, rA
	evrndhb               rD, rA
	evrnddw               rD, rA
	evrndwhus             rD, rA
	evrndwhss             rD, rA
	evrndhbus             rD, rA
	evrndhbss             rD, rA
	evrnddwus             rD, rA
	evrnddwss             rD, rA
	evrndwnh              rD, rA
	evrndhnb              rD, rA
	evrnddnw              rD, rA
	evrndwnhus            rD, rA
	evrndwnhss            rD, rA
	evrndhnbus            rD, rA
	evrndhnbss            rD, rA
	evrnddnwus            rD, rA
	evrnddnwss            rD, rA
	evcntlzh              rD, rA
	evcntlsh              rD, rA
	evpopcntb             rD, rA
	circinc               rD, rA, rB
	evunpkhibui           rD, rA
	evunpkhibsi           rD, rA
	evunpkhihui           rD, rA
	evunpkhihsi           rD, rA
	evunpklobui           rD, rA
	evunpklobsi           rD, rA
	evunpklohui           rD, rA
	evunpklohsi           rD, rA
	evunpklohf            rD, rA
	evunpkhihf            rD, rA
	evunpklowgsf          rD, rA
	evunpkhiwgsf          rD, rA
	evsatsduw             rD, rA
	evsatsdsw             rD, rA
	evsatshub             rD, rA
	evsatshsb             rD, rA
	evsatuwuh             rD, rA
	evsatswsh             rD, rA
	evsatswuh             rD, rA
	evsatuhub             rD, rA
	evsatuduw             rD, rA
	evsatuwsw             rD, rA
	evsatshuh             rD, rA
	evsatuhsh             rD, rA
	evsatswuw             rD, rA
	evsatswgsdf           rD, rA
	evsatsbub             rD, rA
	evsatubsb             rD, rA
	evmaxhpuw             rD, rA
	evmaxhpsw             rD, rA
	evmaxbpuh             rD, rA
	evmaxbpsh             rD, rA
	evmaxwpud             rD, rA
	evmaxwpsd             rD, rA
	evminhpuw             rD, rA
	evminhpsw             rD, rA
	evminbpuh             rD, rA
	evminbpsh             rD, rA
	evminwpud             rD, rA
	evminwpsd             rD, rA
	evmaxmagws            rD, rA, rB
	evsl                  rD, rA, rB
	evsli                 rD, rA, UIMM
	evsplatie             rD, SIMM
	evsplatib             rD, SIMM
	evsplatibe            rD, SIMM
	evsplatih             rD, SIMM
	evsplatihe            rD, SIMM
	evsplatid             rD, SIMM
	evsplatia             rD, SIMM
	evsplatiea            rD, SIMM
	evsplatiba            rD, SIMM
	evsplatibea           rD, SIMM
	evsplatiha            rD, SIMM
	evsplatihea           rD, SIMM
	evsplatida            rD, SIMM
	evsplatfio            rD, SIMM
	evsplatfib            rD, SIMM
	evsplatfibo           rD, SIMM
	evsplatfih            rD, SIMM
	evsplatfiho           rD, SIMM
	evsplatfid            rD, SIMM
	evsplatfia            rD, SIMM
	evsplatfioa           rD, SIMM
	evsplatfiba           rD, SIMM
	evsplatfiboa          rD, SIMM
	evsplatfiha           rD, SIMM
	evsplatfihoa          rD, SIMM
	evsplatfida           rD, SIMM
	evcmpgtdu             crD, rA, rB
	evcmpgtds             crD, rA, rB
	evcmpltdu             crD, rA, rB
	evcmpltds             crD, rA, rB
	evcmpeqd              crD, rA, rB
	evswapbhilo           rD, rA, rB
	evswapblohi           rD, rA, rB
	evswaphhilo           rD, rA, rB
	evswaphlohi           rD, rA, rB
	evswaphe              rD, rA, rB
	evswaphhi             rD, rA, rB
	evswaphlo             rD, rA, rB
	evswapho              rD, rA, rB
	evinsb                rD, rA, Ddd, bbb
	evxtrb                rD, rA, Ddd, bbb
	evsplath              rD, rA, hh
	evsplatb              rD, rA, bbb
	evinsh                rD, rA, dd, hh
	evclrbe               rD, rA, mask
	evclrbo               rD, rA, mask
	evclrh                rD, rA, mask
	evxtrh                rD, rA, dd, hh
	evselbitm0            rD, rA, rB
	evselbitm1            rD, rA, rB
	evselbit              rD, rA, rB
	evperm                rD, rA, rB
	evperm2               rD, rA, rB
	evperm3               rD, rA, rB
	evxtrd                rD, rA, rB, offset
	evsrbu                rD, rA, rB
	evsrbs                rD, rA, rB
	evsrbiu               rD, rA, UIMM_LT8
	evsrbis               rD, rA, UIMM_LT8
	evslb                 rD, rA, rB
	evrlb                 rD, rA, rB
	evslbi                rD, rA, UIMM_LT8
	evrlbi                rD, rA, UIMM_LT8
	evsrhu                rD, rA, rB
	evsrhs                rD, rA, rB
	evsrhiu               rD, rA, UIMM_LT16
	evsrhis               rD, rA, UIMM_LT16
	evslh                 rD, rA, rB
	evrlh                 rD, rA, rB
	evslhi                rD, rA, UIMM_LT16
	evrlhi                rD, rA, UIMM_LT16
	evsru                 rD, rA, rB
	evsrs                 rD, rA, rB
	evsriu                rD, rA, UIMM
	evsris                rD, rA, UIMM
	evlvsl                rD, rA, rB
	evlvsr                rD, rA, rB
	evsroiu               rD, rA, nnn
	evsrois               rD, rA, nnn
	evsloi                rD, rA, nnn
	evfssqrt              rD, rA
	evfscfh               rD, rB
	evfscth               rD, rB
	evfsmax               rD, rA, rB
	evfsmin               rD, rA, rB
	evfsaddsub            rD, rA, rB
	evfssubadd            rD, rA, rB
	evfssum               rD, rA, rB
	evfsdiff              rD, rA, rB
	evfssumdiff           rD, rA, rB
	evfsdiffsum           rD, rA, rB
	evfsaddx              rD, rA, rB
	evfssubx              rD, rA, rB
	evfsaddsubx           rD, rA, rB
	evfssubaddx           rD, rA, rB
	evfsmulx              rD, rA, rB
	evfsmule              rD, rA, rB
	evfsmulo              rD, rA, rB
	evldbx                rD, rA, rB
	evldb                 rD, UIMM_8 (rA)
	evlhhsplathx          rD, rA, rB
	evlhhsplath           rD, UIMM_2 (rA)
	evlwbsplatwx          rD, rA, rB
	evlwbsplatw           rD, UIMM_4 (rA)
	evlwhsplatwx          rD, rA, rB
	evlwhsplatw           rD, UIMM_4 (rA)
	evlbbsplatbx          rD, rA, rB
	evlbbsplatb           rD, UIMM_1 (rA)
	evstdbx               rS, rA, rB
	evstdb                rS, UIMM_8 (rA)
	evlwbex               rD, rA, rB
	evlwbe                rD, UIMM_4 (rA)
	evlwboux              rD, rA, rB
	evlwbou               rD, UIMM_4 (rA)
	evlwbosx              rD, rA, rB
	evlwbos               rD, UIMM_4 (rA)
	evstwbex              rS, rA, rB
	evstwbe               rS, UIMM_4 (rA)
	evstwbox              rS, rA, rB
	evstwbo               rS, UIMM_4 (rA)
	evstwbx               rS, rA, rB
	evstwb                rS, UIMM_4 (rA)
	evsthbx               rS, rA, rB
	evsthb                rS, UIMM_2 (rA)
	evlddmx               rD, rA, rB
	evlddu                rD, UIMM_8 (rA)
	evldwmx               rD, rA, rB
	evldwu                rD, UIMM_8 (rA)
	evldhmx               rD, rA, rB
	evldhu                rD, UIMM_8 (rA)
	evldbmx               rD, rA, rB
	evldbu                rD, UIMM_8 (rA)
	evlhhesplatmx         rD, rA, rB
	evlhhesplatu          rD, UIMM_2 (rA)
	evlhhsplathmx         rD, rA, rB
	evlhhsplathu          rD, UIMM_2 (rA)
	evlhhousplatmx        rD, rA, rB
	evlhhousplatu         rD, UIMM_2 (rA)
	evlhhossplatmx        rD, rA, rB
	evlhhossplatu         rD, UIMM_2 (rA)
	evlwhemx              rD, rA, rB
	evlwheu               rD, UIMM_4 (rA)
	evlwbsplatwmx         rD, rA, rB
	evlwbsplatwu          rD, UIMM_4 (rA)
	evlwhoumx             rD, rA, rB
	evlwhouu              rD, UIMM_4 (rA)
	evlwhosmx             rD, rA, rB
	evlwhosu              rD, UIMM_4 (rA)
	evlwwsplatmx          rD, rA, rB
	evlwwsplatu           rD, UIMM_4 (rA)
	evlwhsplatwmx         rD, rA, rB
	evlwhsplatwu          rD, UIMM_4 (rA)
	evlwhsplatmx          rD, rA, rB
	evlwhsplatu           rD, UIMM_4 (rA)
	evlbbsplatbmx         rD, rA, rB
	evlbbsplatbu          rD, UIMM_1 (rA)
	evstddmx              rS, rA, rB
	evstddu               rS, UIMM_8 (rA)
	evstdwmx              rS, rA, rB
	evstdwu               rS, UIMM_8 (rA)
	evstdhmx              rS, rA, rB
	evstdhu               rS, UIMM_8 (rA)
	evstdbmx              rS, rA, rB
	evstdbu               rS, UIMM_8 (rA)
	evlwbemx              rD, rA, rB
	evlwbeu               rD, UIMM_4 (rA)
	evlwboumx             rD, rA, rB
	evlwbouu              rD, UIMM_4 (rA)
	evlwbosmx             rD, rA, rB
	evlwbosu              rD, UIMM_4 (rA)
	evstwhemx             rS, rA, rB
	evstwheu              rS, UIMM_4 (rA)
	evstwbemx             rS, rA, rB
	evstwbeu              rS, UIMM_4 (rA)
	evstwhomx             rS, rA, rB
	evstwhou              rS, UIMM_4 (rA)
	evstwbomx             rS, rA, rB
	evstwbou              rS, UIMM_4 (rA)
	evstwwemx             rS, rA, rB
	evstwweu              rS, UIMM_4 (rA)
	evstwbmx              rS, rA, rB
	evstwbu               rS, UIMM_4 (rA)
	evstwwomx             rS, rA, rB
	evstwwou              rS, UIMM_4 (rA)
	evsthbmx              rS, rA, rB
	evsthbu               rS, UIMM_2 (rA)
	evmhusi               rD, rA, rB
	evmhssi               rD, rA, rB
	evmhsusi              rD, rA, rB
	evmhssf               rD, rA, rB
	evmhumi               rD, rA, rB
	evmhssfr              rD, rA, rB
	evmhesumi             rD, rA, rB
	evmhosumi             rD, rA, rB
	evmbeumi              rD, rA, rB
	evmbesmi              rD, rA, rB
	evmbesumi             rD, rA, rB
	evmboumi              rD, rA, rB
	evmbosmi              rD, rA, rB
	evmbosumi             rD, rA, rB
	evmhesumia            rD, rA, rB
	evmhosumia            rD, rA, rB
	evmbeumia             rD, rA, rB
	evmbesmia             rD, rA, rB
	evmbesumia            rD, rA, rB
	evmboumia             rD, rA, rB
	evmbosmia             rD, rA, rB
	evmbosumia            rD, rA, rB
	evmwusiw              rD, rA, rB
	evmwssiw              rD, rA, rB
	evmwhssfr             rD, rA, rB
	evmwehgsmfr           rD, rA, rB
	evmwehgsmf            rD, rA, rB
	evmwohgsmfr           rD, rA, rB
	evmwohgsmf            rD, rA, rB
	evmwhssfra            rD, rA, rB
	evmwehgsmfra          rD, rA, rB
	evmwehgsmfa           rD, rA, rB
	evmwohgsmfra          rD, rA, rB
	evmwohgsmfa           rD, rA, rB
	evaddusiaa            rD, rA
	evaddssiaa            rD, rA
	evsubfusiaa           rD, rA
	evsubfssiaa           rD, rA
	evaddsmiaa            rD, rA
	evsubfsmiaa           rD, rA
	evaddh                rD, rA, rB
	evaddhss              rD, rA, rB
	evsubfh               rD, rA, rB
	evsubfhss             rD, rA, rB
	evaddhx               rD, rA, rB
	evaddhxss             rD, rA, rB
	evsubfhx              rD, rA, rB
	evsubfhxss            rD, rA, rB
	evaddd                rD, rA, rB
	evadddss              rD, rA, rB
	evsubfd               rD, rA, rB
	evsubfdss             rD, rA, rB
	evaddb                rD, rA, rB
	evaddbss              rD, rA, rB
	evsubfb               rD, rA, rB
	evsubfbss             rD, rA, rB
	evaddsubfh            rD, rA, rB
	evaddsubfhss          rD, rA, rB
	evsubfaddh            rD, rA, rB
	evsubfaddhss          rD, rA, rB
	evaddsubfhx           rD, rA, rB
	evaddsubfhxss         rD, rA, rB
	evsubfaddhx           rD, rA, rB
	evsubfaddhxss         rD, rA, rB
	evadddus              rD, rA, rB
	evaddbus              rD, rA, rB
	evsubfdus             rD, rA, rB
	evsubfbus             rD, rA, rB
	evaddwus              rD, rA, rB
	evaddwxus             rD, rA, rB
	evsubfwus             rD, rA, rB
	evsubfwxus            rD, rA, rB
	evadd2subf2h          rD, rA, rB
	evadd2subf2hss        rD, rA, rB
	evsubf2add2h          rD, rA, rB
	evsubf2add2hss        rD, rA, rB
	evaddhus              rD, rA, rB
	evaddhxus             rD, rA, rB
	evsubfhus             rD, rA, rB
	evsubfhxus            rD, rA, rB
	evaddwss              rD, rA, rB
	evsubfwss             rD, rA, rB
	evaddwx               rD, rA, rB
	evaddwxss             rD, rA, rB
	evsubfwx              rD, rA, rB
	evsubfwxss            rD, rA, rB
	evaddsubfw            rD, rA, rB
	evaddsubfwss          rD, rA, rB
	evsubfaddw            rD, rA, rB
	evsubfaddwss          rD, rA, rB
	evaddsubfwx           rD, rA, rB
	evaddsubfwxss         rD, rA, rB
	evsubfaddwx           rD, rA, rB
	evsubfaddwxss         rD, rA, rB
	evmar                 rD
	evsumwu               rD, rA
	evsumws               rD, rA
	evsum4bu              rD, rA
	evsum4bs              rD, rA
	evsum2hu              rD, rA
	evsum2hs              rD, rA
	evdiff2his            rD, rA
	evsum2his             rD, rA
	evsumwua              rD, rA
	evsumwsa              rD, rA
	evsum4bua             rD, rA
	evsum4bsa             rD, rA
	evsum2hua             rD, rA
	evsum2hsa             rD, rA
	evdiff2hisa           rD, rA
	evsum2hisa            rD, rA
	evsumwuaa             rD, rA
	evsumwsaa             rD, rA
	evsum4buaaw           rD, rA
	evsum4bsaaw           rD, rA
	evsum2huaaw           rD, rA
	evsum2hsaaw           rD, rA
	evdiff2hisaaw         rD, rA
	evsum2hisaaw          rD, rA
	evdivwsf              rD, rA, rB
	evdivwuf              rD, rA, rB
	evdivs                rD, rA, rB
	evdivu                rD, rA, rB
	evaddwegsi            rD, rA, rB
	evaddwegsf            rD, rA, rB
	evsubfwegsi           rD, rA, rB
	evsubfwegsf           rD, rA, rB
	evaddwogsi            rD, rA, rB
	evaddwogsf            rD, rA, rB
	evsubfwogsi           rD, rA, rB
	evsubfwogsf           rD, rA, rB
	evaddhhiuw            rD, rA, rB
	evaddhhisw            rD, rA, rB
	evsubfhhiuw           rD, rA, rB
	evsubfhhisw           rD, rA, rB
	evaddhlouw            rD, rA, rB
	evaddhlosw            rD, rA, rB
	evsubfhlouw           rD, rA, rB
	evsubfhlosw           rD, rA, rB
	evmhesusiaaw          rD, rA, rB
	evmhosusiaaw          rD, rA, rB
	evmhesumiaaw          rD, rA, rB
	evmhosumiaaw          rD, rA, rB
	evmbeusiaah           rD, rA, rB
	evmbessiaah           rD, rA, rB
	evmbesusiaah          rD, rA, rB
	evmbousiaah           rD, rA, rB
	evmbossiaah           rD, rA, rB
	evmbosusiaah          rD, rA, rB
	evmbeumiaah           rD, rA, rB
	evmbesmiaah           rD, rA, rB
	evmbesumiaah          rD, rA, rB
	evmboumiaah           rD, rA, rB
	evmbosmiaah           rD, rA, rB
	evmbosumiaah          rD, rA, rB
	evmwlusiaaw3          rD, rA, rB
	evmwlssiaaw3          rD, rA, rB
	evmwhssfraaw3         rD, rA, rB
	evmwhssfaaw3          rD, rA, rB
	evmwhssfraaw          rD, rA, rB
	evmwhssfaaw           rD, rA, rB
	evmwlumiaaw3          rD, rA, rB
	evmwlsmiaaw3          rD, rA, rB
	evmwusiaa             rD, rA, rB
	evmwssiaa             rD, rA, rB
	evmwehgsmfraa         rD, rA, rB
	evmwehgsmfaa          rD, rA, rB
	evmwohgsmfraa         rD, rA, rB
	evmwohgsmfaa          rD, rA, rB
	evmhesusianw          rD, rA, rB
	evmhosusianw          rD, rA, rB
	evmhesumianw          rD, rA, rB
	evmhosumianw          rD, rA, rB
	evmbeusianh           rD, rA, rB
	evmbessianh           rD, rA, rB
	evmbesusianh          rD, rA, rB
	evmbousianh           rD, rA, rB
	evmbossianh           rD, rA, rB
	evmbosusianh          rD, rA, rB
	evmbeumianh           rD, rA, rB
	evmbesmianh           rD, rA, rB
	evmbesumianh          rD, rA, rB
	evmboumianh           rD, rA, rB
	evmbosmianh           rD, rA, rB
	evmbosumianh          rD, rA, rB
	evmwlusianw3          rD, rA, rB
	evmwlssianw3          rD, rA, rB
	evmwhssfranw3         rD, rA, rB
	evmwhssfanw3          rD, rA, rB
	evmwhssfranw          rD, rA, rB
	evmwhssfanw           rD, rA, rB
	evmwlumianw3          rD, rA, rB
	evmwlsmianw3          rD, rA, rB
	evmwusian             rD, rA, rB
	evmwssian             rD, rA, rB
	evmwehgsmfran         rD, rA, rB
	evmwehgsmfan          rD, rA, rB
	evmwohgsmfran         rD, rA, rB
	evmwohgsmfan          rD, rA, rB
	evseteqb              rD, rA, rB
	evseteqb.             rD, rA, rB
	evseteqh              rD, rA, rB
	evseteqh.             rD, rA, rB
	evseteqw              rD, rA, rB
	evseteqw.             rD, rA, rB
	evsetgthu             rD, rA, rB
	evsetgthu.            rD, rA, rB
	evsetgths             rD, rA, rB
	evsetgths.            rD, rA, rB
	evsetgtwu             rD, rA, rB
	evsetgtwu.            rD, rA, rB
	evsetgtws             rD, rA, rB
	evsetgtws.            rD, rA, rB
	evsetgtbu             rD, rA, rB
	evsetgtbu.            rD, rA, rB
	evsetgtbs             rD, rA, rB
	evsetgtbs.            rD, rA, rB
	evsetltbu             rD, rA, rB
	evsetltbu.            rD, rA, rB
	evsetltbs             rD, rA, rB
	evsetltbs.            rD, rA, rB
	evsetlthu             rD, rA, rB
	evsetlthu.            rD, rA, rB
	evsetlths             rD, rA, rB
	evsetlths.            rD, rA, rB
	evsetltwu             rD, rA, rB
	evsetltwu.            rD, rA, rB
	evsetltws             rD, rA, rB
	evsetltws.            rD, rA, rB
	evsaduw               rD, rA, rB
	evsadsw               rD, rA, rB
	evsad4ub              rD, rA, rB
	evsad4sb              rD, rA, rB
	evsad2uh              rD, rA, rB
	evsad2sh              rD, rA, rB
	evsaduwa              rD, rA, rB
	evsadswa              rD, rA, rB
	evsad4uba             rD, rA, rB
	evsad4sba             rD, rA, rB
	evsad2uha             rD, rA, rB
	evsad2sha             rD, rA, rB
	evabsdifuw            rD, rA, rB
	evabsdifsw            rD, rA, rB
	evabsdifub            rD, rA, rB
	evabsdifsb            rD, rA, rB
	evabsdifuh            rD, rA, rB
	evabsdifsh            rD, rA, rB
	evsaduwaa             rD, rA, rB
	evsadswaa             rD, rA, rB
	evsad4ubaaw           rD, rA, rB
	evsad4sbaaw           rD, rA, rB
	evsad2uhaaw           rD, rA, rB
	evsad2shaaw           rD, rA, rB
	evpkshubs             rD, rA, rB
	evpkshsbs             rD, rA, rB
	evpkswuhs             rD, rA, rB
	evpkswshs             rD, rA, rB
	evpkuhubs             rD, rA, rB
	evpkuwuhs             rD, rA, rB
	evpkswshilvs          rD, rA, rB
	evpkswgshefrs         rD, rA, rB
	evpkswshfrs           rD, rA, rB
	evpkswshilvfrs        rD, rA, rB
	evpksdswfrs           rD, rA, rB
	evpksdshefrs          rD, rA, rB
	evpkuduws             rD, rA, rB
	evpksdsws             rD, rA, rB
	evpkswgswfrs          rD, rA, rB
	evilveh               rD, rA, rB
	evilveoh              rD, rA, rB
	evilvhih              rD, rA, rB
	evilvhiloh            rD, rA, rB
	evilvloh              rD, rA, rB
	evilvlohih            rD, rA, rB
	evilvoeh              rD, rA, rB
	evilvoh               rD, rA, rB
	evdlveb               rD, rA, rB
	evdlveh               rD, rA, rB
	evdlveob              rD, rA, rB
	evdlveoh              rD, rA, rB
	evdlvob               rD, rA, rB
	evdlvoh               rD, rA, rB
	evdlvoeb              rD, rA, rB
	evdlvoeh              rD, rA, rB
	evmaxbu               rD, rA, rB
	evmaxbs               rD, rA, rB
	evmaxhu               rD, rA, rB
	evmaxhs               rD, rA, rB
	evmaxwu               rD, rA, rB
	evmaxws               rD, rA, rB
	evmaxdu               rD, rA, rB
	evmaxds               rD, rA, rB
	evminbu               rD, rA, rB
	evminbs               rD, rA, rB
	evminhu               rD, rA, rB
	evminhs               rD, rA, rB
	evminwu               rD, rA, rB
	evminws               rD, rA, rB
	evmindu               rD, rA, rB
	evminds               rD, rA, rB
	evavgwu               rD, rA, rB
	evavgws               rD, rA, rB
	evavgbu               rD, rA, rB
	evavgbs               rD, rA, rB
	evavghu               rD, rA, rB
	evavghs               rD, rA, rB
	evavgdu               rD, rA, rB
	evavgds               rD, rA, rB
	evavgwur              rD, rA, rB
	evavgwsr              rD, rA, rB
	evavgbur              rD, rA, rB
	evavgbsr              rD, rA, rB
	evavghur              rD, rA, rB
	evavghsr              rD, rA, rB
	evavgdur              rD, rA, rB
	evavgdsr              rD, rA, rB

;#SPE2 mapped by macro
	evdotphsssi           rD, rA, rB
	evdotphsssia          rD, rA, rB
	evdotpwsssi           rD, rA, rB
	evdotpwsssia          rD, rA, rB
