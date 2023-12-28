.thumb
.syntax unified

T:

clrm {r0, r2} @ Accepts list without APSR
clrm {APSR} @ Accepts APSR alone
clrm {r3, APSR} @ Accepts core register and APSR together
clrmeq {r4} @ Accepts conditional execution

vscclrm {VPR} @ Accepts list with only VPR
vscclrm {s30, VPR} @ Accept single-precision VFP register and VPR together
vscclrm {d14, VPR} @ Likewise for double-precision VFP register
vscclrm {s1-s4, VPR} @ Accept range of single-precision VFP registers
		     @ and VPR together
vscclrm {d1-d4, VPR} @ Likewise for double-precision VFP registers
vscclrm {s0-s31, VPR} @ Accept all single-precision VFP registers and VPR
		      @ together
vscclrm {d0-d15, VPR} @ Likewise for double-precision VFP registers
vscclrmne {s3, VPR} @ Accepts conditional execution

vldr FPSCR, [r2] @ Accepts offset variant without immediate
vldr FPSCR, [r2, #8] @ Likewise but with immediate without sign
vldr FPSCR, [r2, #+8] @ Likewise but with positive sign
vldr FPSCR, [r2, #-8] @ Likewise but with negative sign
vldr FPSCR, [r2, #8]! @ Accepts pre-index variant with immediate without sign
vldr FPSCR, [r2, #+8]! @ Likewise but with positive sign
vldr FPSCR, [r2, #-8]! @ Likewise but with negative sign
vldr FPSCR, [r2], #8 @ Accepts post-index variant with immediate without sign
vldr FPSCR, [r2], #+8 @ Likewise but with positive sign
vldr FPSCR, [r2], #-8 @ Likewise but with negative sign
vldr FPSCR_nzcvqc, [r3] @ Accepts FPSCR_nzcvqc system register
vldr VPR, [r3] @ Accepts VPR system register
vldr P0,  [r3] @ Accepts P0 system register
vldr FPCXTNS, [r3] @ Accepts FPCXTNS system register
vldr FPCXTS, [r3] @ Accepts FPCXTS system register
vldrge FPCXTS, [r3] @ Accepts conditional execution

vstr FPSCR, [r2] @ Accepts offset variant without immediate
vstr FPSCR, [r2, #8] @ Likewise but with immediate without sign
vstr FPSCR, [r2, #+8] @ Likewise but with positive sign
vstr FPSCR, [r2, #-8] @ Likewise but with negative sign
vstr FPSCR, [r2, #8]! @ Accepts pre-index variant with immediate without sign
vstr FPSCR, [r2, #+8]! @ Likewise but with positive sign
vstr FPSCR, [r2, #-8]! @ Likewise but with negative sign
vstr FPSCR, [r2], #8 @ Accepts post-index variant with immediate without sign
vstr FPSCR, [r2], #+8 @ Likewise but with positive sign
vstr FPSCR, [r2], #-8 @ Likewise but with negative sign
vstr FPSCR_nzcvqc, [r3] @ Accepts FPSCR_nzcvqc system register
vstr VPR, [r3] @ Accepts VPR system register
vstr P0,  [r3] @ Accepts P0 system register
vstr FPCXTNS, [r3] @ Accepts FPCXTNS system register
vstr FPCXTS, [r3] @ Accepts FPCXTS system register
vstrge FPCXTS, [r3] @ Accepts conditional execution
