.equ CV_SIGNATURE_C13, 4

.equ T_LONG, 0x0012

.equ LF_MODIFIER, 0x1001

.section ".debug$T", "rn"

.long CV_SIGNATURE_C13

/* Type 1000, volatile long */
.mod1:
.short .mod2 - .mod1 - 2
.short LF_MODIFIER
.long T_LONG
.short 2 /* volatile */
.p2align 2

/* Type 1001, const long */
.mod2:
.short .types_end - .mod2 - 2
.short LF_MODIFIER
.long T_LONG
.short 1 /* const */
.p2align 2

.types_end:
