.equ CV_SIGNATURE_C13, 4
.equ DEBUG_S_SYMBOLS, 0xf1

.equ T_VOID, 0x0003
.equ T_UINT4, 0x0075

.equ LF_MODIFIER, 0x1001
.equ LF_PROCEDURE, 0x1008
.equ LF_ARGLIST, 0x1201
.equ LF_FUNC_ID, 0x1601
.equ LF_BUILDINFO, 0x1603
.equ LF_STRING_ID, 0x1605

.equ S_END, 0x0006
.equ S_FRAMEPROC, 0x1012
.equ S_OBJNAME, 0x1101
.equ S_THUNK32, 0x1102
.equ S_BLOCK32, 0x1103
.equ S_LABEL32, 0x1105
.equ S_REGISTER, 0x1106
.equ S_BPREL32, 0x110b
.equ S_GPROC32, 0x1110
.equ S_REGREL32, 0x1111
.equ S_UNAMESPACE, 0x1124
.equ S_FRAMECOOKIE, 0x113a
.equ S_COMPILE3, 0x113c
.equ S_LOCAL, 0x113e
.equ S_DEFRANGE_REGISTER, 0x1141
.equ S_DEFRANGE_FRAMEPOINTER_REL, 0x1142
.equ S_DEFRANGE_SUBFIELD_REGISTER, 0x1143
.equ S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE, 0x1144
.equ S_DEFRANGE_REGISTER_REL, 0x1145
.equ S_BUILDINFO, 0x114c
.equ S_INLINESITE, 0x114d
.equ S_INLINESITE_END, 0x114e
.equ S_HEAPALLOCSITE, 0x115e

.equ CV_AMD64_RAX, 328
.equ CV_CFL_AMD64, 0xd0

.section ".debug$S", "rn"

.long CV_SIGNATURE_C13

.long DEBUG_S_SYMBOLS
.long .syms_end - .syms_start

.syms_start:

.objname1:
.short .compile1 - .objname1 - 2
.short S_OBJNAME
.long 0 /* signature */
.asciz "syms3.o"

.compile1:
.short .unamespace1 - .compile1 - 2
.short S_COMPILE3
.long 0 /* flags */
.short CV_CFL_AMD64 /* target processor */
.short 0 /* frontend major */
.short 0 /* frontend minor */
.short 0 /* frontend build */
.short 0 /* frontend qfe */
.short 0 /* backend major */
.short 0 /* backend minor */
.short 0 /* backend build */
.short 0 /* backend qfe */
.asciz "GNU AS"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.unamespace1:
.short .sbuildinfo1 - .unamespace1 - 2
.short S_UNAMESPACE
.asciz "std"

.sbuildinfo1:
.short .gproc1 - .sbuildinfo1 - 2
.short S_BUILDINFO
.long 0x1007 /* type */

.gproc1:
.short .frameproc1 - .gproc1 - 2
.short S_GPROC32
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next symbol */
.long .proc1_end - proc1 /* length */
.long 0 /* debug start offset */
.long 0 /* debug end offset */
.long 0x1001 /* type */
.secrel32 proc1
.secidx proc1
.byte 0 /* flags */
.asciz "proc1"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.frameproc1:
.short .framecookie1 - .frameproc1 - 2
.short S_FRAMEPROC
.long 0 /* frame size */
.long 0 /* frame padding */
.long 0 /* padding offset */
.long 0 /* size of callee-save registers */
.long 0 /* offset of exception handler */
.short 0 /* section of exception handler */
.long 0 /* flags */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.framecookie1:
.short .bprel1 - .framecookie1 - 2
.short S_FRAMECOOKIE
.long 8 /* frame-relative offset */
.short CV_AMD64_RAX /* register */
.long 0 /* cookie type (CV_COOKIETYPE_COPY) */
.byte 0 /* flags */
.byte 0xf1 /* padding */

.bprel1:
.short .reg1 - .bprel1 - 2
.short S_BPREL32
.long 4 /* BP-relative offset */
.long 0x1008 /* type */
.asciz "foo"

.reg1:
.short .regrel1 - .reg1 - 2
.short S_REGISTER
.long 0x1008 /* type */
.short CV_AMD64_RAX
.asciz "bar"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.regrel1:
.short .local1 - .regrel1 - 2
.short S_REGREL32
.long 4 /* offset */
.long 0x1008 /* type */
.short CV_AMD64_RAX
.asciz "baz"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.local1:
.short .defrange1 - .local1 - 2
.short S_LOCAL
.long 0x1008 /* type */
.short 0 /* flags */
.asciz "local1"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.defrange1:
.short .local2 - .defrange1 - 2
.short S_DEFRANGE_REGISTER_REL
.short CV_AMD64_RAX
.short 0 /* offset parent */
.long 0 /* offset register */
.secrel32 .block1 /* offset */
.secidx .block1 /* section */
.short .block1_end - .block1 /* length */
.short .gap1 - .block1 /* gap 1 offset */
.short .gap1_end - .gap1 /* gap 1 length */

.local2:
.short .defrange2 - .local2 - 2
.short S_LOCAL
.long 0x1008 /* type */
.short 0 /* flags */
.asciz "local2"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.defrange2:
.short .local3 - .defrange2 - 2
.short S_DEFRANGE_FRAMEPOINTER_REL
.long 4 /* frame pointer offset */
.secrel32 .block1 /* offset */
.secidx .block1 /* section */
.short .block1_end - .block1 /* length */
.short .gap1 - .block1 /* gap 1 offset */
.short .gap1_end - .gap1 /* gap 1 length */

.local3:
.short .defrange3 - .local3 - 2
.short S_LOCAL
.long 0x1008 /* type */
.short 0 /* flags */
.asciz "local3"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.defrange3:
.short .local4 - .defrange3 - 2
.short S_DEFRANGE_SUBFIELD_REGISTER
.short CV_AMD64_RAX
.short 0 /* attributes */
.long 4 /* offset in parent variable */
.secrel32 .block1 /* offset */
.secidx .block1 /* section */
.short .block1_end - .block1 /* length */
.short .gap1 - .block1 /* gap 1 offset */
.short .gap1_end - .gap1 /* gap 1 length */

.local4:
.short .defrange4 - .local4 - 2
.short S_LOCAL
.long 0x1008 /* type */
.short 0 /* flags */
.asciz "local4"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.defrange4:
.short .local5 - .defrange4 - 2
.short S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE
.long 4 /* frame pointer offset */

.local5:
.short .defrange5 - .local5 - 2
.short S_LOCAL
.long 0x1008 /* type */
.short 0 /* flags */
.asciz "local5"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.defrange5:
.short .inlinesite1 - .defrange5 - 2
.short S_DEFRANGE_REGISTER
.short CV_AMD64_RAX
.short 0 /* attributes */
.secrel32 .block1 /* offset */
.secidx .block1 /* section */
.short .block1_end - .block1 /* length */
.short .gap1 - .block1 /* gap 1 offset */
.short .gap1_end - .gap1 /* gap 1 length */

.inlinesite1:
.short .inlinesite1end - .inlinesite1 - 2
.short S_INLINESITE
.long 0 /* parent */
.long 0 /* end */
.long 0x1009 /* inlinee (inline_func) */

.inlinesite1end:
.short .sblock1 - .inlinesite1end - 2
.short S_INLINESITE_END

.sblock1:
.short .label1 - .sblock1 - 2
.short S_BLOCK32
.long 0 /* parent (filled in by linker) */
.long 0 /* end (filled in by linker) */
.long .block1_end - .block1 /* length */
.secrel32 .block1
.secidx .block1
.byte 0 /* name */
.byte 0xf1 /* padding */

.label1:
.short .sblock1_end - .label1 - 2
.short S_LABEL32
.secrel32 label
.secidx label
.byte 0 /* flags */
.asciz "label"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

.sblock1_end:
.short .thunk1 - .sblock1_end - 2
.short S_END

.thunk1:
.short .thunk1_end - .thunk1 - 2
.short S_THUNK32
.long 0 /* parent */
.long 0 /* end */
.long 0 /* next */
.secrel32 thunk
.secidx thunk
.short .thunk_end - thunk
.byte 0 /* THUNK_ORDINAL value */
.asciz "thunk"
.byte 0xf1 /* padding */

.thunk1_end:
.short .heapallocsite1 - .thunk1_end - 2
.short S_END

.heapallocsite1:
.short .gproc1_end - .heapallocsite1 - 2
.short S_HEAPALLOCSITE
.secrel32 .gap1_end
.secidx .gap1_end
.short .block1_end - .gap1_end
.long 0x1008 /* type */

.gproc1_end:
.short .syms_end - .gproc1_end - 2
.short S_END

.syms_end:

.section ".debug$T", "rn"

.long CV_SIGNATURE_C13

/* Type 1000, arglist (uint32_t) */
.arglist1:
.short .proctype1 - .arglist1 - 2
.short LF_ARGLIST
.long 1 /* no. entries */
.long T_UINT4

/* Type 1001, procedure (return type T_VOID, arglist 1000) */
.proctype1:
.short .string1 - .proctype1 - 2
.short LF_PROCEDURE
.long T_VOID
.byte 0 /* calling convention */
.byte 0 /* attributes */
.short 1 /* no. parameters */
.long 0x1000

/* Type 1002, string "/tmp" (build directory) */
.string1:
.short .string2 - .string1 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "/tmp"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1003, string "gcc" (compiler) */
.string2:
.short .string3 - .string2 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "gcc"

/* Type 1004, string "tmp.c" (source file) */
.string3:
.short .string4 - .string3 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "tmp.c"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1005, string "tmp.pdb" (PDB file) */
.string4:
.short .string5 - .string4 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "tmp.pdb"

/* Type 1006, string "-gcodeview" (command arguments) */
.string5:
.short .buildinfo1 - .string5 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "-gcodeview"
.byte 0xf1 /* padding */

/* Type 1007, build info */
.buildinfo1:
.short .mod1 - .buildinfo1 - 2
.short LF_BUILDINFO
.short 5 /* count */
.long 0x1002 /* build directory */
.long 0x1003 /* compiler */
.long 0x1004 /* source file */
.long 0x1005 /* PDB file */
.long 0x1006 /* command arguments */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1008, const uint32_t */
.mod1:
.short .funcid1 - .mod1 - 2
.short LF_MODIFIER
.long T_UINT4
.short 1 /* const */
.p2align 2

/* Type 1009, func ID for inline_func */
.funcid1:
.short .types_end - .funcid1 - 2
.short LF_FUNC_ID
.long 0 /* parent scope */
.long 0x1001 /* type */
.asciz "inline_func"

.types_end:

.text

.global proc1
proc1:
  .byte 0
.block1:
  .byte 0
label:
  .byte 0
.gap1:
  .byte 0
.gap1_end:
  .byte 0
.block1_end:
  .byte 0
.proc1_end:

thunk:
  .byte 0
.thunk_end:
