.equ CV_SIGNATURE_C13, 4

.equ T_LONG, 0x0012
.equ T_INT4, 0x0074

.equ LF_MODIFIER, 0x1001
.equ LF_FIELDLIST, 0x1203
.equ LF_STRUCTURE, 0x1505
.equ LF_MEMBER, 0x150d
.equ LF_STRING_ID, 0x1605
.equ LF_UDT_SRC_LINE, 0x1606

.section ".debug$T", "rn"

.long CV_SIGNATURE_C13

/* Type 1000, const long */
.mod1:
.short .fieldlist1 - .mod1 - 2
.short LF_MODIFIER
.long T_LONG
.short 1 /* const */
.short 0 /* padding */

/* Type 1001, fieldlist for struct foo */
.fieldlist1:
.short .struct1 - .fieldlist1 - 2
.short LF_FIELDLIST
.short LF_MEMBER
.short 3 /* public */
.long T_INT4
.short 0 /* offset */
.asciz "num"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1002, struct foo */
.struct1:
.short .string1 - .struct1 - 2
.short LF_STRUCTURE
.short 1 /* no. members */
.short 0 /* property */
.long 0x1001 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 4 /* size */
.asciz "foo" /* name */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1003, string "foo" */
.string1:
.short .udtsrcline1 - .string1 - 2
.short LF_STRING_ID
.long 0 /* sub-string */
.asciz "foo.h"
.byte 0xf2
.byte 0xf1

/* Type 1004, UDT source line for type 1002 */
.udtsrcline1:
.short .types_end - .udtsrcline1 - 2
.short LF_UDT_SRC_LINE
.long 0x1002
.long 0x1003 /* source file string */
.long 42 /* line no. */

.types_end:
