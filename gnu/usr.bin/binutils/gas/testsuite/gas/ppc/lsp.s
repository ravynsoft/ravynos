# PA LSP instructions
# CMPE200GCC-62
	.section ".text"

	.equ	rA,1
	.equ	rB,2
	.equ	rD,0
	.equ	rS,0
	.equ 	UIMM, 15 ;#UIMM values >15 are illegal
	.equ 	UIMM_2, 4
	.equ 	UIMM_4, 8
	.equ 	UIMM_8, 16
	.equ	SIMM, -16
	.equ	crD, 0
	.equ	offset, 1

	zvaddih            rD, rA, UIMM
	zvsubifh           rD, rA, UIMM
	zvaddh             rD, rA, rB
	zvsubfh            rD, rA, rB
	zvaddsubfh         rD, rA, rB
	zvsubfaddh         rD, rA, rB
	zvaddhx            rD, rA, rB
	zvsubfhx           rD, rA, rB
	zvaddsubfhx        rD, rA, rB
	zvsubfaddhx        rD, rA, rB
	zaddwus            rD, rA, rB
	zsubfwus           rD, rA, rB
	zaddwss            rD, rA, rB
	zsubfwss           rD, rA, rB
	zvaddhus           rD, rA, rB
	zvsubfhus          rD, rA, rB
	zvaddhss           rD, rA, rB
	zvsubfhss          rD, rA, rB
	zvaddsubfhss       rD, rA, rB
	zvsubfaddhss       rD, rA, rB
	zvaddhxss          rD, rA, rB
	zvsubfhxss         rD, rA, rB
	zvaddsubfhxss      rD, rA, rB
	zvsubfaddhxss      rD, rA, rB
	zaddheuw           rD, rA, rB
	zsubfheuw          rD, rA, rB
	zaddhesw           rD, rA, rB
	zsubfhesw          rD, rA, rB
	zaddhouw           rD, rA, rB
	zsubfhouw          rD, rA, rB
	zaddhosw           rD, rA, rB
	zsubfhosw          rD, rA, rB
	zvmergehih         rD, rA, rB
	zvmergeloh         rD, rA, rB
	zvmergehiloh       rD, rA, rB
	zvmergelohih       rD, rA, rB
	zvcmpgthu          crD, rA, rB
	zvcmpgths          crD, rA, rB
	zvcmplthu          crD, rA, rB
	zvcmplths          crD, rA, rB
	zvcmpeqh           crD, rA, rB
	zpkswgshfrs        rD, rA, rB
	zpkswgswfrs        rD, rA, rB
	zvpkshgwshfrs      rD, rA, rB
	zvpkswshfrs        rD, rA, rB
	zvpkswuhs          rD, rA, rB
	zvpkswshs          rD, rA, rB
	zvpkuwuhs          rD, rA, rB
	zvsplatih          rD, SIMM
	zvsplatfih         rD, SIMM
	zcntlsw            rD, rA
	zvcntlzh           rD, rA
	zvcntlsh           rD, rA
	znegws             rD, rA
	zvnegh             rD, rA
	zvneghs            rD, rA
	zvnegho            rD, rA
	zvneghos           rD, rA
	zrndwh             rD, rA
	zrndwhss           rD, rA
	zvabsh             rD, rA
	zvabshs            rD, rA
	zabsw              rD, rA
	zabsws             rD, rA
	zsatswuw           rD, rA
	zsatuwsw           rD, rA
	zsatswuh           rD, rA
	zsatswsh           rD, rA
	zvsatshuh          rD, rA
	zvsatuhsh          rD, rA
	zsatuwuh           rD, rA
	zsatuwsh           rD, rA
	zsatsduw           rD, rA, rB
	zsatsdsw           rD, rA, rB
	zsatuduw           rD, rA, rB
	zvselh             rD, rA, rB
	zxtrw              rD, rA, rB, offset
	zbrminc            rD, rA, rB
	zcircinc           rD, rA, rB
	zdivwsf            rD, rA, rB
	zvsrhu             rD, rA, rB
	zvsrhs             rD, rA, rB
	zvsrhiu            rD, rA, UIMM
	zvsrhis            rD, rA, UIMM
	zvslh              rD, rA, rB
	zvrlh              rD, rA, rB
	zvslhi             rD, rA, UIMM
	zvrlhi             rD, rA, UIMM
	zvslhus            rD, rA, rB
	zvslhss            rD, rA, rB
	zvslhius           rD, rA, UIMM
	zvslhiss           rD, rA, UIMM
	zslwus             rD, rA, rB
	zslwss             rD, rA, rB
	zslwius            rD, rA, UIMM
	zslwiss            rD, rA, UIMM
	zaddwgui           rD, rA, rB
	zsubfwgui          rD, rA, rB
	zaddd              rD, rA, rB
	zsubfd             rD, rA, rB
	zvaddsubfw         rD, rA, rB
	zvsubfaddw         rD, rA, rB
	zvaddw             rD, rA, rB
	zvsubfw            rD, rA, rB
	zaddwgsi           rD, rA, rB
	zsubfwgsi          rD, rA, rB
	zadddss            rD, rA, rB
	zsubfdss           rD, rA, rB
	zvaddsubfwss       rD, rA, rB
	zvsubfaddwss       rD, rA, rB
	zvaddwss           rD, rA, rB
	zvsubfwss          rD, rA, rB
	zaddwgsf           rD, rA, rB
	zsubfwgsf          rD, rA, rB
	zadddus            rD, rA, rB
	zsubfdus           rD, rA, rB
	zvaddwus           rD, rA, rB
	zvsubfwus          rD, rA, rB
	zvunpkhgwsf        rD, rA
	zvunpkhsf          rD, rA
	zvunpkhui          rD, rA
	zvunpkhsi          rD, rA
	zunpkwgsf          rD, rA
	zvdotphgwasmf      rD, rA, rB
	zvdotphgwasmfr     rD, rA, rB
	zvdotphgwasmfaa    rD, rA, rB
	zvdotphgwasmfraa   rD, rA, rB
	zvdotphgwasmfan    rD, rA, rB
	zvdotphgwasmfran   rD, rA, rB
	zvmhulgwsmf        rD, rA, rB
	zvmhulgwsmfr       rD, rA, rB
	zvmhulgwsmfaa      rD, rA, rB
	zvmhulgwsmfraa     rD, rA, rB
	zvmhulgwsmfan      rD, rA, rB
	zvmhulgwsmfran     rD, rA, rB
	zvmhulgwsmfanp     rD, rA, rB
	zvmhulgwsmfranp    rD, rA, rB
	zmhegwsmf          rD, rA, rB
	zmhegwsmfr         rD, rA, rB
	zmhegwsmfaa        rD, rA, rB
	zmhegwsmfraa       rD, rA, rB
	zmhegwsmfan        rD, rA, rB
	zmhegwsmfran       rD, rA, rB
	zvdotphxgwasmf     rD, rA, rB
	zvdotphxgwasmfr    rD, rA, rB
	zvdotphxgwasmfaa   rD, rA, rB
	zvdotphxgwasmfraa  rD, rA, rB
	zvdotphxgwasmfan   rD, rA, rB
	zvdotphxgwasmfran  rD, rA, rB
	zvmhllgwsmf        rD, rA, rB
	zvmhllgwsmfr       rD, rA, rB
	zvmhllgwsmfaa      rD, rA, rB
	zvmhllgwsmfraa     rD, rA, rB
	zvmhllgwsmfan      rD, rA, rB
	zvmhllgwsmfran     rD, rA, rB
	zvmhllgwsmfanp     rD, rA, rB
	zvmhllgwsmfranp    rD, rA, rB
	zmheogwsmf         rD, rA, rB
	zmheogwsmfr        rD, rA, rB
	zmheogwsmfaa       rD, rA, rB
	zmheogwsmfraa      rD, rA, rB
	zmheogwsmfan       rD, rA, rB
	zmheogwsmfran      rD, rA, rB
	zvdotphgwssmf      rD, rA, rB
	zvdotphgwssmfr     rD, rA, rB
	zvdotphgwssmfaa    rD, rA, rB
	zvdotphgwssmfraa   rD, rA, rB
	zvdotphgwssmfan    rD, rA, rB
	zvdotphgwssmfran   rD, rA, rB
	zvmhuugwsmf        rD, rA, rB
	zvmhuugwsmfr       rD, rA, rB
	zvmhuugwsmfaa      rD, rA, rB
	zvmhuugwsmfraa     rD, rA, rB
	zvmhuugwsmfan      rD, rA, rB
	zvmhuugwsmfran     rD, rA, rB
	zvmhuugwsmfanp     rD, rA, rB
	zvmhuugwsmfranp    rD, rA, rB
	zmhogwsmf          rD, rA, rB
	zmhogwsmfr         rD, rA, rB
	zmhogwsmfaa        rD, rA, rB
	zmhogwsmfraa       rD, rA, rB
	zmhogwsmfan        rD, rA, rB
	zmhogwsmfran       rD, rA, rB
	zvmhxlgwsmf        rD, rA, rB
	zvmhxlgwsmfr       rD, rA, rB
	zvmhxlgwsmfaa      rD, rA, rB
	zvmhxlgwsmfraa     rD, rA, rB
	zvmhxlgwsmfan      rD, rA, rB
	zvmhxlgwsmfran     rD, rA, rB
	zvmhxlgwsmfanp     rD, rA, rB
	zvmhxlgwsmfranp    rD, rA, rB
	zmhegui            rD, rA, rB
	zvdotphgaui        rD, rA, rB
	zmheguiaa          rD, rA, rB
	zvdotphgauiaa      rD, rA, rB
	zmheguian          rD, rA, rB
	zvdotphgauian      rD, rA, rB
	zmhegsi            rD, rA, rB
	zvdotphgasi        rD, rA, rB
	zmhegsiaa          rD, rA, rB
	zvdotphgasiaa      rD, rA, rB
	zmhegsian          rD, rA, rB
	zvdotphgasian      rD, rA, rB
	zmhegsui           rD, rA, rB
	zvdotphgasui       rD, rA, rB
	zmhegsuiaa         rD, rA, rB
	zvdotphgasuiaa     rD, rA, rB
	zmhegsuian         rD, rA, rB
	zvdotphgasuian     rD, rA, rB
	zmhegsmf           rD, rA, rB
	zvdotphgasmf       rD, rA, rB
	zmhegsmfaa         rD, rA, rB
	zvdotphgasmfaa     rD, rA, rB
	zmhegsmfan         rD, rA, rB
	zvdotphgasmfan     rD, rA, rB
	zmheogui           rD, rA, rB
	zvdotphxgaui       rD, rA, rB
	zmheoguiaa         rD, rA, rB
	zvdotphxgauiaa     rD, rA, rB
	zmheoguian         rD, rA, rB
	zvdotphxgauian     rD, rA, rB
	zmheogsi           rD, rA, rB
	zvdotphxgasi       rD, rA, rB
	zmheogsiaa         rD, rA, rB
	zvdotphxgasiaa     rD, rA, rB
	zmheogsian         rD, rA, rB
	zvdotphxgasian     rD, rA, rB
	zmheogsui          rD, rA, rB
	zvdotphxgasui      rD, rA, rB
	zmheogsuiaa        rD, rA, rB
	zvdotphxgasuiaa    rD, rA, rB
	zmheogsuian        rD, rA, rB
	zvdotphxgasuian    rD, rA, rB
	zmheogsmf          rD, rA, rB
	zvdotphxgasmf      rD, rA, rB
	zmheogsmfaa        rD, rA, rB
	zvdotphxgasmfaa    rD, rA, rB
	zmheogsmfan        rD, rA, rB
	zvdotphxgasmfan    rD, rA, rB
	zmhogui            rD, rA, rB
	zvdotphgsui        rD, rA, rB
	zmhoguiaa          rD, rA, rB
	zvdotphgsuiaa      rD, rA, rB
	zmhoguian          rD, rA, rB
	zvdotphgsuian      rD, rA, rB
	zmhogsi            rD, rA, rB
	zvdotphgssi        rD, rA, rB
	zmhogsiaa          rD, rA, rB
	zvdotphgssiaa      rD, rA, rB
	zmhogsian          rD, rA, rB
	zvdotphgssian      rD, rA, rB
	zmhogsui           rD, rA, rB
	zvdotphgssui       rD, rA, rB
	zmhogsuiaa         rD, rA, rB
	zvdotphgssuiaa     rD, rA, rB
	zmhogsuian         rD, rA, rB
	zvdotphgssuian     rD, rA, rB
	zmhogsmf           rD, rA, rB
	zvdotphgssmf       rD, rA, rB
	zmhogsmfaa         rD, rA, rB
	zvdotphgssmfaa     rD, rA, rB
	zmhogsmfan         rD, rA, rB
	zvdotphgssmfan     rD, rA, rB
	zmwgui             rD, rA, rB
	zmwguiaa           rD, rA, rB
	zmwguiaas          rD, rA, rB
	zmwguian           rD, rA, rB
	zmwguians          rD, rA, rB
	zmwgsi             rD, rA, rB
	zmwgsiaa           rD, rA, rB
	zmwgsiaas          rD, rA, rB
	zmwgsian           rD, rA, rB
	zmwgsians          rD, rA, rB
	zmwgsui            rD, rA, rB
	zmwgsuiaa          rD, rA, rB
	zmwgsuiaas         rD, rA, rB
	zmwgsuian          rD, rA, rB
	zmwgsuians         rD, rA, rB
	zmwgsmf            rD, rA, rB
	zmwgsmfr           rD, rA, rB
	zmwgsmfaa          rD, rA, rB
	zmwgsmfraa         rD, rA, rB
	zmwgsmfan          rD, rA, rB
	zmwgsmfran         rD, rA, rB
	zvmhului           rD, rA, rB
	zvmhuluiaa         rD, rA, rB
	zvmhuluiaas        rD, rA, rB
	zvmhuluian         rD, rA, rB
	zvmhuluians        rD, rA, rB
	zvmhuluianp        rD, rA, rB
	zvmhuluianps       rD, rA, rB
	zvmhulsi           rD, rA, rB
	zvmhulsiaa         rD, rA, rB
	zvmhulsiaas        rD, rA, rB
	zvmhulsian         rD, rA, rB
	zvmhulsians        rD, rA, rB
	zvmhulsianp        rD, rA, rB
	zvmhulsianps       rD, rA, rB
	zvmhulsui          rD, rA, rB
	zvmhulsuiaa        rD, rA, rB
	zvmhulsuiaas       rD, rA, rB
	zvmhulsuian        rD, rA, rB
	zvmhulsuians       rD, rA, rB
	zvmhulsuianp       rD, rA, rB
	zvmhulsuianps      rD, rA, rB
	zvmhulsf           rD, rA, rB
	zvmhulsfr          rD, rA, rB
	zvmhulsfaas        rD, rA, rB
	zvmhulsfraas       rD, rA, rB
	zvmhulsfans        rD, rA, rB
	zvmhulsfrans       rD, rA, rB
	zvmhulsfanps       rD, rA, rB
	zvmhulsfranps      rD, rA, rB
	zvmhllui           rD, rA, rB
	zvmhlluiaa         rD, rA, rB
	zvmhlluiaas        rD, rA, rB
	zvmhlluian         rD, rA, rB
	zvmhlluians        rD, rA, rB
	zvmhlluianp        rD, rA, rB
	zvmhlluianps       rD, rA, rB
	zvmhllsi           rD, rA, rB
	zvmhllsiaa         rD, rA, rB
	zvmhllsiaas        rD, rA, rB
	zvmhllsian         rD, rA, rB
	zvmhllsians        rD, rA, rB
	zvmhllsianp        rD, rA, rB
	zvmhllsianps       rD, rA, rB
	zvmhllsui          rD, rA, rB
	zvmhllsuiaa        rD, rA, rB
	zvmhllsuiaas       rD, rA, rB
	zvmhllsuian        rD, rA, rB
	zvmhllsuians       rD, rA, rB
	zvmhllsuianp       rD, rA, rB
	zvmhllsuianps      rD, rA, rB
	zvmhllsf           rD, rA, rB
	zvmhllsfr          rD, rA, rB
	zvmhllsfaas        rD, rA, rB
	zvmhllsfraas       rD, rA, rB
	zvmhllsfans        rD, rA, rB
	zvmhllsfrans       rD, rA, rB
	zvmhllsfanps       rD, rA, rB
	zvmhllsfranps      rD, rA, rB
	zvmhuuui           rD, rA, rB
	zvmhuuuiaa         rD, rA, rB
	zvmhuuuiaas        rD, rA, rB
	zvmhuuuian         rD, rA, rB
	zvmhuuuians        rD, rA, rB
	zvmhuuuianp        rD, rA, rB
	zvmhuuuianps       rD, rA, rB
	zvmhuusi           rD, rA, rB
	zvmhuusiaa         rD, rA, rB
	zvmhuusiaas        rD, rA, rB
	zvmhuusian         rD, rA, rB
	zvmhuusians        rD, rA, rB
	zvmhuusianp        rD, rA, rB
	zvmhuusianps       rD, rA, rB
	zvmhuusui          rD, rA, rB
	zvmhuusuiaa        rD, rA, rB
	zvmhuusuiaas       rD, rA, rB
	zvmhuusuian        rD, rA, rB
	zvmhuusuians       rD, rA, rB
	zvmhuusuianp       rD, rA, rB
	zvmhuusuianps      rD, rA, rB
	zvmhuusf           rD, rA, rB
	zvmhuusfr          rD, rA, rB
	zvmhuusfaas        rD, rA, rB
	zvmhuusfraas       rD, rA, rB
	zvmhuusfans        rD, rA, rB
	zvmhuusfrans       rD, rA, rB
	zvmhuusfanps       rD, rA, rB
	zvmhuusfranps      rD, rA, rB
	zvmhxlui           rD, rA, rB
	zvmhxluiaa         rD, rA, rB
	zvmhxluiaas        rD, rA, rB
	zvmhxluian         rD, rA, rB
	zvmhxluians        rD, rA, rB
	zvmhxluianp        rD, rA, rB
	zvmhxluianps       rD, rA, rB
	zvmhxlsi           rD, rA, rB
	zvmhxlsiaa         rD, rA, rB
	zvmhxlsiaas        rD, rA, rB
	zvmhxlsian         rD, rA, rB
	zvmhxlsians        rD, rA, rB
	zvmhxlsianp        rD, rA, rB
	zvmhxlsianps       rD, rA, rB
	zvmhxlsui          rD, rA, rB
	zvmhxlsuiaa        rD, rA, rB
	zvmhxlsuiaas       rD, rA, rB
	zvmhxlsuian        rD, rA, rB
	zvmhxlsuians       rD, rA, rB
	zvmhxlsuianp       rD, rA, rB
	zvmhxlsuianps      rD, rA, rB
	zvmhxlsf           rD, rA, rB
	zvmhxlsfr          rD, rA, rB
	zvmhxlsfaas        rD, rA, rB
	zvmhxlsfraas       rD, rA, rB
	zvmhxlsfans        rD, rA, rB
	zvmhxlsfrans       rD, rA, rB
	zvmhxlsfanps       rD, rA, rB
	zvmhxlsfranps      rD, rA, rB
	zmheui             rD, rA, rB
	zmheuiaa           rD, rA, rB
	zmheuiaas          rD, rA, rB
	zmheuian           rD, rA, rB
	zmheuians          rD, rA, rB
	zmhesi             rD, rA, rB
	zmhesiaa           rD, rA, rB
	zmhesiaas          rD, rA, rB
	zmhesian           rD, rA, rB
	zmhesians          rD, rA, rB
	zmhesui            rD, rA, rB
	zmhesuiaa          rD, rA, rB
	zmhesuiaas         rD, rA, rB
	zmhesuian          rD, rA, rB
	zmhesuians         rD, rA, rB
	zmhesf             rD, rA, rB
	zmhesfr            rD, rA, rB
	zmhesfaas          rD, rA, rB
	zmhesfraas         rD, rA, rB
	zmhesfans          rD, rA, rB
	zmhesfrans         rD, rA, rB
	zmheoui            rD, rA, rB
	zmheouiaa          rD, rA, rB
	zmheouiaas         rD, rA, rB
	zmheouian          rD, rA, rB
	zmheouians         rD, rA, rB
	zmheosi            rD, rA, rB
	zmheosiaa          rD, rA, rB
	zmheosiaas         rD, rA, rB
	zmheosian          rD, rA, rB
	zmheosians         rD, rA, rB
	zmheosui           rD, rA, rB
	zmheosuiaa         rD, rA, rB
	zmheosuiaas        rD, rA, rB
	zmheosuian         rD, rA, rB
	zmheosuians        rD, rA, rB
	zmheosf            rD, rA, rB
	zmheosfr           rD, rA, rB
	zmheosfaas         rD, rA, rB
	zmheosfraas        rD, rA, rB
	zmheosfans         rD, rA, rB
	zmheosfrans        rD, rA, rB
	zmhoui             rD, rA, rB
	zmhouiaa           rD, rA, rB
	zmhouiaas          rD, rA, rB
	zmhouian           rD, rA, rB
	zmhouians          rD, rA, rB
	zmhosi             rD, rA, rB
	zmhosiaa           rD, rA, rB
	zmhosiaas          rD, rA, rB
	zmhosian           rD, rA, rB
	zmhosians          rD, rA, rB
	zmhosui            rD, rA, rB
	zmhosuiaa          rD, rA, rB
	zmhosuiaas         rD, rA, rB
	zmhosuian          rD, rA, rB
	zmhosuians         rD, rA, rB
	zmhosf             rD, rA, rB
	zmhosfr            rD, rA, rB
	zmhosfaas          rD, rA, rB
	zmhosfraas         rD, rA, rB
	zmhosfans          rD, rA, rB
	zmhosfrans         rD, rA, rB
	zvmhuih            rD, rA, rB
	zvmhuihs           rD, rA, rB
	zvmhuiaah          rD, rA, rB
	zvmhuiaahs         rD, rA, rB
	zvmhuianh          rD, rA, rB
	zvmhuianhs         rD, rA, rB
	zvmhsihs           rD, rA, rB
	zvmhsiaahs         rD, rA, rB
	zvmhsianhs         rD, rA, rB
	zvmhsuihs          rD, rA, rB
	zvmhsuiaahs        rD, rA, rB
	zvmhsuianhs        rD, rA, rB
	zvmhsfh            rD, rA, rB
	zvmhsfrh           rD, rA, rB
	zvmhsfaahs         rD, rA, rB
	zvmhsfraahs        rD, rA, rB
	zvmhsfanhs         rD, rA, rB
	zvmhsfranhs        rD, rA, rB
	zvdotphaui         rD, rA, rB
	zvdotphauis        rD, rA, rB
	zvdotphauiaa       rD, rA, rB
	zvdotphauiaas      rD, rA, rB
	zvdotphauian       rD, rA, rB
	zvdotphauians      rD, rA, rB
	zvdotphasi         rD, rA, rB
	zvdotphasis        rD, rA, rB
	zvdotphasiaa       rD, rA, rB
	zvdotphasiaas      rD, rA, rB
	zvdotphasian       rD, rA, rB
	zvdotphasians      rD, rA, rB
	zvdotphasui        rD, rA, rB
	zvdotphasuis       rD, rA, rB
	zvdotphasuiaa      rD, rA, rB
	zvdotphasuiaas     rD, rA, rB
	zvdotphasuian      rD, rA, rB
	zvdotphasuians     rD, rA, rB
	zvdotphasfs        rD, rA, rB
	zvdotphasfrs       rD, rA, rB
	zvdotphasfaas      rD, rA, rB
	zvdotphasfraas     rD, rA, rB
	zvdotphasfans      rD, rA, rB
	zvdotphasfrans     rD, rA, rB
	zvdotphxaui        rD, rA, rB
	zvdotphxauis       rD, rA, rB
	zvdotphxauiaa      rD, rA, rB
	zvdotphxauiaas     rD, rA, rB
	zvdotphxauian      rD, rA, rB
	zvdotphxauians     rD, rA, rB
	zvdotphxasi        rD, rA, rB
	zvdotphxasis       rD, rA, rB
	zvdotphxasiaa      rD, rA, rB
	zvdotphxasiaas     rD, rA, rB
	zvdotphxasian      rD, rA, rB
	zvdotphxasians     rD, rA, rB
	zvdotphxasui       rD, rA, rB
	zvdotphxasuis      rD, rA, rB
	zvdotphxasuiaa     rD, rA, rB
	zvdotphxasuiaas    rD, rA, rB
	zvdotphxasuian     rD, rA, rB
	zvdotphxasuians    rD, rA, rB
	zvdotphxasfs       rD, rA, rB
	zvdotphxasfrs      rD, rA, rB
	zvdotphxasfaas     rD, rA, rB
	zvdotphxasfraas    rD, rA, rB
	zvdotphxasfans     rD, rA, rB
	zvdotphxasfrans    rD, rA, rB
	zvdotphsui         rD, rA, rB
	zvdotphsuis        rD, rA, rB
	zvdotphsuiaa       rD, rA, rB
	zvdotphsuiaas      rD, rA, rB
	zvdotphsuian       rD, rA, rB
	zvdotphsuians      rD, rA, rB
	zvdotphssi         rD, rA, rB
	zvdotphssis        rD, rA, rB
	zvdotphssiaa       rD, rA, rB
	zvdotphssiaas      rD, rA, rB
	zvdotphssian       rD, rA, rB
	zvdotphssians      rD, rA, rB
	zvdotphssui        rD, rA, rB
	zvdotphssuis       rD, rA, rB
	zvdotphssuiaa      rD, rA, rB
	zvdotphssuiaas     rD, rA, rB
	zvdotphssuian      rD, rA, rB
	zvdotphssuians     rD, rA, rB
	zvdotphssfs        rD, rA, rB
	zvdotphssfrs       rD, rA, rB
	zvdotphssfaas      rD, rA, rB
	zvdotphssfraas     rD, rA, rB
	zvdotphssfans      rD, rA, rB
	zvdotphssfrans     rD, rA, rB
	zmwluis            rD, rA, rB
	zmwluiaa           rD, rA, rB
	zmwluiaas          rD, rA, rB
	zmwluian           rD, rA, rB
	zmwluians          rD, rA, rB
	zmwlsis            rD, rA, rB
	zmwlsiaas          rD, rA, rB
	zmwlsians          rD, rA, rB
	zmwlsuis           rD, rA, rB
	zmwlsuiaas         rD, rA, rB
	zmwlsuians         rD, rA, rB
	zmwsf              rD, rA, rB
	zmwsfr             rD, rA, rB
	zmwsfaas           rD, rA, rB
	zmwsfraas          rD, rA, rB
	zmwsfans           rD, rA, rB
	zmwsfrans          rD, rA, rB
	zlddx              rD, rA, rB
	zldd               rD, UIMM_8(rA)
	zldwx              rD, rA, rB
	zldw               rD, UIMM_8(rA)
	zldhx              rD, rA, rB
	zldh               rD, UIMM_8(rA)
	zlwgsfdx           rD, rA, rB
	zlwgsfd            rD, UIMM_4(rA)
	zlwwosdx           rD, rA, rB
	zlwwosd            rD, UIMM_4(rA)
	zlwhsplatwdx       rD, rA, rB
	zlwhsplatwd        rD, UIMM_4(rA)
	zlwhsplatdx        rD, rA, rB
	zlwhsplatd         rD, UIMM_4(rA)
	zlwhgwsfdx         rD, rA, rB
	zlwhgwsfd          rD, UIMM_4(rA)
	zlwhedx            rD, rA, rB
	zlwhed             rD, UIMM_4(rA)
	zlwhosdx           rD, rA, rB
	zlwhosd            rD, UIMM_4(rA)
	zlwhoudx           rD, rA, rB
	zlwhoud            rD, UIMM_4(rA)
	zlwhx              rD, rA, rB
	zlwh               rD, UIMM_4(rA)
	zlwwx              rD, rA, rB
	zlww               rD, UIMM_4(rA)
	zlhgwsfx           rD, rA, rB
	zlhgwsf            rD, UIMM_2(rA)
	zlhhsplatx         rD, rA, rB
	zlhhsplat          rD, UIMM_2(rA)
	zstddx             rS, rA, rB
	zstdd              rS, UIMM_8(rA)
	zstdwx             rS, rA, rB
	zstdw              rS, UIMM_8(rA)
	zstdhx             rS, rA, rB
	zstdh              rS, UIMM_8(rA)
	zstwhedx           rS, rA, rB
	zstwhed            rS, UIMM_4(rA)
	zstwhodx           rS, rA, rB
	zstwhod            rS, UIMM_4(rA)
	zlhhex             rS, rA, rB
	zlhhe              rD, UIMM_2(rA)
	zlhhosx            rS, rA, rB
	zlhhos             rD, UIMM_2(rA)
	zlhhoux            rS, rA, rB
	zlhhou             rD, UIMM_2(rA)
	zsthex             rS, rA, rB
	zsthe              rS, UIMM_2(rA)
	zsthox             rS, rA, rB
	zstho              rS, UIMM_2(rA)
	zstwhx             rS, rA, rB
	zstwh              rS, UIMM_4(rA)
	zstwwx             rS, rA, rB
	zstww              rS, UIMM_4(rA)
	zlddmx             rD, rA, rB
	zlddu              rD, UIMM_8(rA)
	zldwmx             rD, rA, rB
	zldwu              rD, UIMM_8(rA)
	zldhmx             rD, rA, rB
	zldhu              rD, UIMM_8(rA)
	zlwgsfdmx          rD, rA, rB
	zlwgsfdu           rD, UIMM_4(rA)
	zlwwosdmx          rD, rA, rB
	zlwwosdu           rD, UIMM_4(rA)
	zlwhsplatwdmx      rD, rA, rB
	zlwhsplatwdu       rD, UIMM_4(rA)
	zlwhsplatdmx       rD, rA, rB
	zlwhsplatdu        rD, UIMM_4(rA)
	zlwhgwsfdmx        rD, rA, rB
	zlwhgwsfdu         rD, UIMM_4(rA)
	zlwhedmx           rD, rA, rB
	zlwhedu            rD, UIMM_4(rA)
	zlwhosdmx          rD, rA, rB
	zlwhosdu           rD, UIMM_4(rA)
	zlwhoudmx          rD, rA, rB
	zlwhoudu           rD, UIMM_4(rA)
	zlwhmx             rD, rA, rB
	zlwhu              rD, UIMM_4(rA)
	zlwwmx             rD, rA, rB
	zlwwu              rD, UIMM_4(rA)
	zlhgwsfmx          rD, rA, rB
	zlhgwsfu           rD, UIMM_2(rA)
	zlhhsplatmx        rD, rA, rB
	zlhhsplatu         rD, UIMM_2(rA)
	zstddmx            rS, rA, rB
	zstddu             rS, UIMM_8(rA)
	zstdwmx            rS, rA, rB
	zstdwu             rS, UIMM_8(rA)
	zstdhmx            rS, rA, rB
	zstdhu             rS, UIMM_8(rA)
	zstwhedmx          rS, rA, rB
	zstwhedu           rS, UIMM_4(rA)
	zstwhodmx          rD, rA, rB
	zstwhodu           rS, UIMM_4(rA)
	zlhhemx            rD, rA, rB
	zlhheu             rD, UIMM_2(rA)
	zlhhosmx           rD, rA, rB
	zlhhosu            rD, UIMM_2(rA)
	zlhhoumx           rD, rA, rB
	zlhhouu            rD, UIMM_2(rA)
	zsthemx            rS, rA, rB
	zstheu             rS, UIMM_2(rA)
	zsthomx            rS, rA, rB
	zsthou             rS, UIMM_2(rA)
	zstwhmx            rS, rA, rB
	zstwhu             rS, UIMM_4(rA)
	zstwwmx            rS, rA, rB
	zstwwu             rS, UIMM_4(rA)
