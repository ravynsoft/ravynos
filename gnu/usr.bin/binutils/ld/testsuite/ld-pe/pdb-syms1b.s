.equ CV_SIGNATURE_C13, 4
.equ DEBUG_S_SYMBOLS, 0xf1

.equ T_VOID, 0x0003
.equ T_UQUAD, 0x0023
.equ T_UINT4, 0x0075

.equ LF_MODIFIER, 0x1001
.equ LF_POINTER, 0x1002
.equ LF_PROCEDURE, 0x1008
.equ LF_MFUNCTION, 0x1009
.equ LF_ARGLIST, 0x1201
.equ LF_FIELDLIST, 0x1203
.equ LF_CLASS, 0x1504
.equ LF_STRUCTURE, 0x1505
.equ LF_MEMBER, 0x150d
.equ LF_ONEMETHOD, 0x1511
.equ LF_FUNC_ID, 0x1601
.equ LF_MFUNC_ID, 0x1602

.equ LF_UQUADWORD, 0x800a

.equ S_END, 0x0006
.equ S_CONSTANT, 0x1107
.equ S_UDT, 0x1108
.equ S_LDATA32, 0x110c
.equ S_GDATA32, 0x110d
.equ S_LPROC32, 0x110f
.equ S_GPROC32, 0x1110
.equ S_LTHREAD32, 0x1112
.equ S_GTHREAD32, 0x1113
.equ S_LPROC32_ID, 0x1146
.equ S_GPROC32_ID, 0x1147
.equ S_PROC_ID_END, 0x114f

.equ CV_PTR_64, 0xc

.section ".debug$S", "rn"

.long CV_SIGNATURE_C13

.long DEBUG_S_SYMBOLS
.long .syms_end - .syms_start

.syms_start:

.ldata1:
.short .ldata1a - .ldata1 - 2
.short S_LDATA32
.long 0x1000 /* const uint32_t */
.secrel32 lvar1
.secidx lvar1
.asciz "lvar1"

.ldata1a: /* duplicate with same address */
.short .ldata1b - .ldata1a - 2
.short S_LDATA32
.long 0x1000 /* const uint32_t */
.secrel32 lvar1
.secidx lvar1
.asciz "lvar1"

.ldata1b: /* duplicate with different address */
.short .ldata2 - .ldata1b - 2
.short S_LDATA32
.long 0x1000 /* const uint32_t */
.secrel32 lvar1a
.secidx lvar1a
.asciz "lvar1"

.ldata2:
.short .gdata1 - .ldata2 - 2
.short S_LDATA32
.long 0x1000 /* const uint32_t */
.secrel32 lvar2
.secidx lvar2
.asciz "lvar2"

.gdata1:
.short .gdata2 - .gdata1 - 2
.short S_GDATA32
.long 0x1000 /* const uint32_t */
.secrel32 gvar1
.secidx gvar1
.asciz "gvar1"

.gdata2:
.short .gproc1 - .gdata2 - 2
.short S_GDATA32
.long 0x1000 /* const uint32_t */
.secrel32 gvar2
.secidx gvar2
.asciz "gvar2"

.gproc1:
.short .gproc1_end - .gproc1 - 2
.short S_GPROC32
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc1_end - proc1 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x1002 /* type */
.secrel32 proc1
.secidx proc1
.byte 0 /* flags */
.asciz "proc1"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.gproc1_end:
.short .gproc2 - .gproc1_end - 2
.short S_END

.gproc2:
.short .udt1 - .gproc2 - 2
.short S_GPROC32
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc2_end - proc2 /* length */
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

.udt1:
.short .ldata3 - .udt1 - 2
.short S_UDT
.long 0x1011 /* struct bar */
.asciz "bar"

.ldata3:
.short .lthread1 - .ldata3 - 2
.short S_LDATA32
.long 0x1000 /* const uint32_t */
.secrel32 lvar3
.secidx lvar3
.asciz "lvar3"

.lthread1:
.short .gproc2_end - .lthread1 - 2
.short S_LTHREAD32
.long 0x1000 /* const uint32_t */
.secrel32 lvar4
.secidx lvar4
.asciz "lvar4"

.gproc2_end:
.short .gproc3 - .gproc2_end - 2
.short S_END

.gproc3:
.short .gproc3_end - .gproc3 - 2
.short S_LPROC32
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc3_end - proc3 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x1002 /* type */
.secrel32 proc3
.secidx proc3
.byte 0 /* flags */
.asciz "proc3"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.gproc3_end:
.short .gproc4 - .gproc3_end - 2
.short S_END

.gproc4:
.short .gproc4_end - .gproc4 - 2
.short S_LPROC32
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc4_end - proc4 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x1002 /* type */
.secrel32 proc4
.secidx proc4
.byte 0 /* flags */
.asciz "proc4"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.gproc4_end:
.short .gproc5 - .gproc4_end - 2
.short S_END

.gproc5:
.short .gproc5_end - .gproc5 - 2
.short S_GPROC32_ID
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc5_end - proc5 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x1003 /* func ID */
.secrel32 proc5
.secidx proc5
.byte 0 /* flags */
.asciz "proc5"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.gproc5_end:
.short .gproc6 - .gproc5_end - 2
.short S_PROC_ID_END

.gproc6:
.short .gproc6_end - .gproc6 - 2
.short S_GPROC32_ID
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc6_end - proc6 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x1004 /* func ID */
.secrel32 proc6
.secidx proc6
.byte 0 /* flags */
.asciz "proc6"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.gproc6_end:
.short .gproc7 - .gproc6_end - 2
.short S_PROC_ID_END

.gproc7:
.short .gproc7_end - .gproc7 - 2
.short S_GPROC32_ID
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc7_end - proc7 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x100a /* func ID */
.secrel32 proc7
.secidx proc7
.byte 0 /* flags */
.asciz "foo::method"
.byte 0xf1 /* padding */

.gproc7_end:
.short .gproc8 - .gproc7_end - 2
.short S_PROC_ID_END

.gproc8:
.short .gproc8_end - .gproc8 - 2
.short S_GPROC32_ID
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc8_end - proc8 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x100b /* func ID */
.secrel32 proc8
.secidx proc8
.byte 0 /* flags */
.asciz "foo::method2"

.gproc8_end:
.short .gproc9 - .gproc8_end - 2
.short S_PROC_ID_END

.gproc9:
.short .gproc9_end - .gproc9 - 2
.short S_LPROC32_ID
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc9_end - proc9 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x100c /* func ID */
.secrel32 proc9
.secidx proc9
.byte 0 /* flags */
.asciz "proc9"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.gproc9_end:
.short .gproc10 - .gproc9_end - 2
.short S_PROC_ID_END

.gproc10:
.short .gproc10_end - .gproc10 - 2
.short S_GPROC32_ID
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc10_end - proc10 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x100d /* func ID */
.secrel32 proc10
.secidx proc10
.byte 0 /* flags */
.asciz "proc10"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.gproc10_end:
.short .gproc11 - .gproc10_end - 2
.short S_PROC_ID_END

.gproc11:
.short .gproc11_end - .gproc11 - 2
.short S_LPROC32_ID
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc11_end - proc11 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x100e /* func ID */
.secrel32 proc11
.secidx proc11
.byte 0 /* flags */
.asciz "foo::method3"

.gproc11_end:
.short .gproc12 - .gproc11_end - 2
.short S_PROC_ID_END

.gproc12:
.short .gproc12_end - .gproc12 - 2
.short S_LPROC32_ID
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc12_end - proc12 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x100f /* func ID */
.secrel32 proc12
.secidx proc12
.byte 0 /* flags */
.asciz "foo::method4"

.gproc12_end:
.short .udt2 - .gproc12_end - 2
.short S_PROC_ID_END

.udt2:
.short .constant1 - .udt2 - 2
.short S_UDT
.long 0x1009 /* class foo */
.asciz "foo"

.constant1:
.short .constant2 - .constant1 - 2
.short S_CONSTANT
.long T_UINT4
.short 42
.asciz "answer"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.constant2:
.short .lthread2 - .constant2 - 2
.short S_CONSTANT
.long T_UQUAD
.short LF_UQUADWORD
.quad 0x0123456789abcdef
.asciz "answer2"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.lthread2:
.short .lthread3 - .lthread2 - 2
.short S_LTHREAD32
.long 0x1000 /* const uint32_t */
.secrel32 lvar5
.secidx lvar5
.asciz "lvar5"

.lthread3:
.short .gthread1 - .lthread3 - 2
.short S_LTHREAD32
.long 0x1000 /* const uint32_t */
.secrel32 lvar6
.secidx lvar6
.asciz "lvar6"

.gthread1:
.short .gthread2 - .gthread1 - 2
.short S_GTHREAD32
.long 0x1000 /* const uint32_t */
.secrel32 gvar3
.secidx gvar3
.asciz "gvar3"

.gthread2:
.short .syms_end - .gthread2 - 2
.short S_GTHREAD32
.long 0x1000 /* const uint32_t */
.secrel32 gvar4
.secidx gvar4
.asciz "gvar4"

.p2align 2
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

/* Type 1002, procedure (return type T_VOID, arglist 1001) */
.proctype1:
.short .funcid1 - .proctype1 - 2
.short LF_PROCEDURE
.long T_VOID
.byte 0 /* calling convention */
.byte 0 /* attributes */
.short 1 /* no. parameters */
.long 0x1001

/* Type 1003, func ID for proc5 */
.funcid1:
.short .funcid2 - .funcid1 - 2
.short LF_FUNC_ID
.long 0 /* parent scope */
.long 0x1002 /* type */
.asciz "proc5"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1004, func ID for proc6 */
.funcid2:
.short .class1 - .funcid2 - 2
.short LF_FUNC_ID
.long 0 /* parent scope */
.long 0x1002 /* type */
.asciz "proc6"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1005, forward declaration of class foo */
.class1:
.short .ptr1 - .class1 - 2
.short LF_CLASS
.short 0 /* no. members */
.short 0x80 /* property (forward declaration) */
.long 0 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 0 /* size */
.asciz "foo" /* name */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1006, pointer to 1005 */
.ptr1:
.short .mfunction1 - .ptr1 - 2
.short LF_POINTER
.long 0x1005
.long (8 << 13) | CV_PTR_64

/* Type 1007, member function of 1005, return type void, arg list 1001 */
.mfunction1:
.short .fieldlist1 - .mfunction1 - 2
.short LF_MFUNCTION
.long T_VOID
.long 0x1005
.long 0x1006 /* type of "this" pointer */
.byte 0 /* calling convention */
.byte 0 /* attributes */
.short 1 /* no. parameters */
.long 0x1001 /* arg list */
.long 0 /* "this" adjustment */

/* Type 1008, field list for class foo */
.fieldlist1:
.short .class2 - .fieldlist1 - 2
.short LF_FIELDLIST
.short LF_ONEMETHOD
.short 0 /* method attribute */
.long 0x1007 /* method type */
.asciz "method"
.byte 0xf1 /* padding */
.short LF_ONEMETHOD
.short 0 /* method attribute */
.long 0x1007 /* method type */
.asciz "method2"
.short LF_ONEMETHOD
.short 0 /* method attribute */
.long 0x1007 /* method type */
.asciz "method3"
.short LF_ONEMETHOD
.short 0 /* method attribute */
.long 0x1007 /* method type */
.asciz "method4"

/* Type 1009, actual declaration of class foo */
.class2:
.short .mfunc1 - .class2 - 2
.short LF_CLASS
.short 0 /* no. members */
.short 0 /* property */
.long 0x1008 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 0 /* size */
.asciz "foo" /* name */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 100a, function "method" within class "foo" */
.mfunc1:
.short .mfunc2 - .mfunc1 - 2
.short LF_MFUNC_ID
.long 0x1009 /* parent class */
.long 0x1002 /* function type */
.asciz "method"
.byte 0xf1 /* padding */

/* Type 100b, function "method2" within class "foo" */
.mfunc2:
.short .funcid3 - .mfunc2 - 2
.short LF_MFUNC_ID
.long 0x1009 /* parent class */
.long 0x1002 /* function type */
.asciz "method2"

/* Type 100c, func ID for proc9 */
.funcid3:
.short .funcid4 - .funcid3 - 2
.short LF_FUNC_ID
.long 0 /* parent scope */
.long 0x1002 /* type */
.asciz "proc9"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 100d, func ID for proc10 */
.funcid4:
.short .mfunc3 - .funcid4 - 2
.short LF_FUNC_ID
.long 0 /* parent scope */
.long 0x1002 /* type */
.asciz "proc10"
.byte 0xf1 /* padding */

/* Type 100e, function "method3" within class "foo" */
.mfunc3:
.short .mfunc4 - .mfunc3 - 2
.short LF_MFUNC_ID
.long 0x1009 /* parent class */
.long 0x1002 /* function type */
.asciz "method3"

/* Type 100f, function "method4" within class "foo" */
.mfunc4:
.short .fieldlist2 - .mfunc4 - 2
.short LF_MFUNC_ID
.long 0x1009 /* parent class */
.long 0x1002 /* function type */
.asciz "method4"

/* Type 1010, field list for struct bar */
.fieldlist2:
.short .struct1 - .fieldlist2 - 2
.short LF_FIELDLIST
.short LF_MEMBER
.short 3 /* public */
.long T_UINT4
.short 0 /* offset */
.asciz "num1"
.byte 0xf1 /* padding */

/* Type 1011, declaration of struct bar */
.struct1:
.short .types_end - .struct1 - 2
.short LF_STRUCTURE
.short 1 /* no. members */
.short 0 /* property */
.long 0x1010 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 4 /* size */
.asciz "bar" /* name */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.types_end:

.data

.long 0x12345678
.long 0x12345678

lvar1:
.long 42

lvar1a:
.long 0x12345678

lvar3:
.long 84

lvar4:
.long 85

.global gvar1
gvar1:
.long 43

.global gvar3
gvar3:
.long 41

lvar5:
.long 86

.text

.global main
main:
    .short 0
    .secrel32 .data

.global proc2
proc2:
    .byte 0
.proc2_end:

.global proc4
proc4:
    .byte 0
.proc4_end:

.global proc6
proc6:
    .byte 0
.proc6_end:

.global proc8
proc8:
    .byte 0
.proc8_end:

.global proc10
proc10:
    .byte 0
.proc10_end:

.global proc12
proc12:
    .byte 0
.proc12_end:

.section "gcsect"

lvar2:
.long 84

.global gvar2
gvar2:
.long 85

.global gvar4
gvar4:
.long 86

.global proc1
proc1:
    .byte 0
.proc1_end:

.global proc3
proc3:
    .byte 0
.proc3_end:

.global proc5
proc5:
    .byte 0
.proc5_end:

.global proc7
proc7:
    .byte 0
.proc7_end:

.global proc9
proc9:
    .byte 0
.proc9_end:

.global proc11
proc11:
    .byte 0
.proc11_end:

lvar6:
.long 86
