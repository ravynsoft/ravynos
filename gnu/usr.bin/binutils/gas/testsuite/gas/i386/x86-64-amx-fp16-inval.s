# Check illegal register usage in AMX-FP16 instructions

    .text
_start:
    tdpfp16ps %tmm1, %tmm1, %tmm0
    tdpfp16ps %tmm1, %tmm0, %tmm1
    tdpfp16ps %tmm0, %tmm1, %tmm1

    .intel_syntax noprefix
    tdpfp16ps tmm0, tmm1, tmm1
    tdpfp16ps tmm1, tmm0, tmm1
    tdpfp16ps tmm0, tmm1, tmm1
