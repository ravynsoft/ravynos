.equ CV_SIGNATURE_C13, 4

.equ T_VOID, 0x0003
.equ T_INT4, 0x0074

.equ LF_PROCEDURE, 0x1008
.equ LF_MFUNCTION, 0x1009
.equ LF_POINTER, 0x1002
.equ LF_ARGLIST, 0x1201
.equ LF_FIELDLIST, 0x1203
.equ LF_CLASS, 0x1504
.equ LF_ONEMETHOD, 0x1511
.equ LF_FUNC_ID, 0x1601
.equ LF_MFUNC_ID, 0x1602
.equ LF_BUILDINFO, 0x1603
.equ LF_SUBSTR_LIST, 0x1604
.equ LF_STRING_ID, 0x1605

.equ CV_PTR_64, 0xc

.section ".debug$T", "rn"

.long CV_SIGNATURE_C13

/* Type 1000, string "test" */
.string1:
.short .string2 - .string1 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "test"
.byte 0xf3
.byte 0xf2
.byte 0xf1

/* Type 1001, string "foo" */
.string2:
.short .types_end - .string2 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "foo"

.types_end:
