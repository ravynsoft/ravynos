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

/* Type 1000, string "foo" */
.string1:
.short .string2 - .string1 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "foo"

/* Type 1001, string "bar" */
.string2:
.short .substrlist1 - .string2 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "bar"

/* Type 1002, substr list of "foo" and "bar" */
.substrlist1:
.short .string3 - .substrlist1 - 2
.short LF_SUBSTR_LIST
.long 2 /* count */
.long 0x1000
.long 0x1001

/* Type 1003, string "baz" referencing substr list 1002 */
.string3:
.short .string4 - .string3 - 2
.short LF_STRING_ID
.long 0x1002
.asciz "baz"

/* Type 1004, string "/tmp" (build directory) */
.string4:
.short .string5 - .string4 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "/tmp"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1005, string "gcc" (compiler) */
.string5:
.short .string6 - .string5 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "gcc"

/* Type 1006, string "tmp.c" (source file) */
.string6:
.short .string7 - .string6 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "tmp.c"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1007, string "tmp.pdb" (PDB file) */
.string7:
.short .string8 - .string7 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "tmp.pdb"

/* Type 1008, string "-gcodeview" (command arguments) */
.string8:
.short .buildinfo1 - .string8 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "-gcodeview"
.byte 0xf1 /* padding */

/* The 1009, build info */
.buildinfo1:
.short .string9 - .buildinfo1 - 2
.short LF_BUILDINFO
.short 5 /* count */
.long 0x1004 /* build directory */
.long 0x1005 /* compiler */
.long 0x1006 /* source file */
.long 0x1007 /* PDB file */
.long 0x1008 /* command arguments */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 100a, string "namespace" */
.string9:
.short .arglist1 - .string9 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "namespace"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 100b, arg list of type T_INT4 */
.arglist1:
.short .proc1 - .arglist1 - 2
.short LF_ARGLIST
.long 1 /* no. entries */
.long T_INT4

/* Type 100c, procedure, return type T_VOID, arg list 100b */
.proc1:
.short .func1 - .proc1 - 2
.short LF_PROCEDURE
.long T_VOID
.byte 0 /* calling convention */
.byte 0 /* attributes */
.short 1 /* no. parameters */
.long 0x100b

/* Type 100d, function "func1" */
.func1:
.short .func2 - .func1 - 2
.short LF_FUNC_ID
.long 0 /* parent scope */
.long 0x100c /* type */
.asciz "func1"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 100e, function "func2" within scope "namespace" */
.func2:
.short .class1 - .func2 - 2
.short LF_FUNC_ID
.long 0x100a /* parent scope */
.long 0x100c /* type */
.asciz "func2"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 100f, forward declaration of class foo */
.class1:
.short .ptr1 - .class1 - 2
.short LF_CLASS
.short 0 /* no. members */
.short 0x80 /* property (has unique name, forward declaration) */
.long 0 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 0 /* size */
.asciz "foo" /* name */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1010, pointer to 100f */
.ptr1:
.short .mfunction1 - .ptr1 - 2
.short LF_POINTER
.long 0x100f
.long (8 << 13) | CV_PTR_64

/* Type 1011, member function of 100f, return type void, arg list 100b */
.mfunction1:
.short .fieldlist1 - .mfunction1 - 2
.short LF_MFUNCTION
.long T_VOID
.long 0x100f
.long 0x1010 /* type of "this" pointer */
.byte 0 /* calling convention */
.byte 0 /* attributes */
.short 1 /* no. parameters */
.long 0x100b /* arg list */
.long 0 /* "this" adjustment */

/* Type 1012, field list for class foo */
.fieldlist1:
.short .class2 - .fieldlist1 - 2
.short LF_FIELDLIST
.short LF_ONEMETHOD
.short 0 /* method attribute */
.long 0x1010 /* method type */
.asciz "method"
.byte 0xf1

/* Type 1013, actual declaration of class foo */
.class2:
.short .mfunc1 - .class2 - 2
.short LF_CLASS
.short 0 /* no. members */
.short 0 /* property */
.long 0x1012 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 0 /* size */
.asciz "foo" /* name */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1014, function "method" within class "foo" */
.mfunc1:
.short .types_end - .mfunc1 - 2
.short LF_MFUNC_ID
.long 0x100f /* parent class */
.long 0x1011 /* function type */
.asciz "method"
.byte 0xf1 /* padding */

.types_end:
