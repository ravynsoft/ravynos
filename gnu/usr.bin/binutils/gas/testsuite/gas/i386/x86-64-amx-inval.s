# Check illegal SIBMEM and register size used in AMX instructions

    .text
_start:
    tileloadd (%rip), %tmm1
    tileloaddt1 (%rip), %tmm1
    tilestored %tmm1, (%rip)
    tdpbssd %xmm1, %xmm2, %xmm3
    vaddps %tmm1, %tmm2, %tmm3
    tdpbssd %tmm1, %tmm1, %tmm0
    tdpbssd %tmm1, %tmm0, %tmm1
    tdpbssd %tmm0, %tmm1, %tmm1
    tdpbf16ps %tmm1, %tmm1, %tmm0
    tdpbf16ps %tmm1, %tmm0, %tmm1
    tdpbf16ps %tmm0, %tmm1, %tmm1

    .intel_syntax noprefix
    tileloadd tmm1, [rip]
    tileloaddt1 tmm1, [rip]
    tilestored [rip], tmm1
    tdpbssd xmm3, xmm2, xmm1
    vaddps %tmm1, %tmm2, %tmm3
    tdpbssd tmm0, tmm1, tmm1
    tdpbssd tmm1, tmm0, tmm1
    tdpbssd tmm1, tmm1, tmm0
    tdpbf16ps tmm0, tmm1, tmm1
    tdpbf16ps tmm1, tmm0, tmm1
    tdpbf16ps tmm0, tmm1, tmm1
