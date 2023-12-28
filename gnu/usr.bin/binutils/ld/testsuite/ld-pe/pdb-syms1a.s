.equ CV_SIGNATURE_C13, 4
.equ DEBUG_S_SYMBOLS, 0xf1

.equ T_UINT4, 0x0075

.equ LF_MODIFIER, 0x1001
.equ LF_PROCEDURE, 0x1008
.equ LF_ARGLIST, 0x1201
.equ LF_FIELDLIST, 0x1203
.equ LF_STRUCTURE, 0x1505
.equ LF_MEMBER, 0x150d

.equ S_END, 0x0006
.equ S_UDT, 0x1108
.equ S_GPROC32, 0x1110

.section ".debug$S", "rn"

.long CV_SIGNATURE_C13

.long DEBUG_S_SYMBOLS
.long .syms_end - .syms_start

.syms_start:

.gproc2:
.short .gproc2_end - .gproc2 - 2
.short S_GPROC32
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long 1 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x1002 /* type */
.secrel32 proc2
.secidx proc2
.byte 0 /* flags */
.asciz "proc2"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.gproc2_end:
.short .udt1 - .gproc2_end - 2
.short S_END

.udt1:
.short .syms_end - .udt1 - 2
.short S_UDT
.long 0x1004 /* struct bar */
.asciz "bar"

.syms_end:

.section ".debug$T", "rn"

.long CV_SIGNATURE_C13

/* Type 1000, const uint32_t */
.mod1:
.short .arglist1 - .mod1 - 2
.short LF_MODIFIER
.long T_UINT4
.short 1 /* const */
.p2align 2

/* Type 1001, arglist (uint32_t) */
.arglist1:
.short .proctype1 - .arglist1 - 2
.short LF_ARGLIST
.long 1 /* no. entries */
.long T_UINT4

# Type 1002, procedure (return type T_VOID, arglist 1001)
.proctype1:
.short .fieldlist1 - .proctype1 - 2
.short LF_PROCEDURE
.long T_VOID
.byte 0 /* calling convention */
.byte 0 /* attributes */
.short 1 /* no. parameters */
.long 0x1001

/* Type 1003, field list for struct bar */
.fieldlist1:
.short .struct1 - .fieldlist1 - 2
.short LF_FIELDLIST
.short LF_MEMBER
.short 3 /* public */
.long T_UINT4
.short 0 /* offset */
.asciz "num1"
.byte 0xf1 /* padding */

/* Type 1004, declaration of struct bar */
.struct1:
.short .types_end - .struct1 - 2
.short LF_STRUCTURE
.short 1 /* no. members */
.short 0 /* property */
.long 0x1003 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 4 /* size */
.asciz "bar" /* name */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.types_end:
