# Check illegal register usage in AMX-COMPLEX instructions

    .text
_start:
    tcmmimfp16ps %tmm1, %tmm1, %tmm0
    tcmmimfp16ps %tmm1, %tmm0, %tmm1
    tcmmimfp16ps %tmm0, %tmm1, %tmm1

    .intel_syntax noprefix
    tcmmimfp16ps tmm0, tmm1, tmm1
    tcmmimfp16ps tmm1, tmm0, tmm1
    tcmmimfp16ps tmm0, tmm1, tmm1
