.thumb
.syntax unified

T:

clrm {} @ Rejects empty list
clrm {sp} @ Rejects SP in list
clrm {pc} @ Reject PC in list

vscclrm {} @ Rejects empty list
vscclrm {s0} @ Rejects list without VPR
vscclrm {s1, d1, VPR} @ Reject mix of single and double-precision VFP registers
vscclrm {s1-d1, VPR} @ Likewise when using a range

vldr APSR, [r2] @ Rejects incorrect system register
vldr FPSCR, [r2, #2] @ Rejects invalid immediate for offset variant
vldr FPSCR, [r2, #2]! @ Likewise for pre-index variant
vldr FPSCR, [r2], #2 @ Likewise for post-index variant

vstr APSR, [r2] @ Rejects incorrect system register
vstr FPSCR, [r2, #2] @ Rejects invalid immediate for offset variant
vstr FPSCR, [r2, #2]! @ Likewise for pre-index variant
vstr FPSCR, [r2], #2 @ Likewise for post-index variant
