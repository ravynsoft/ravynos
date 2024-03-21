.equ CV_SIGNATURE_C13, 4

.equ T_CHAR, 0x0010
.equ T_LONG, 0x0012
.equ T_ULONG, 0x0022
.equ T_INT4, 0x0074
.equ T_UINT4, 0x0075
.equ T_UQUAD, 0x0023

.equ LF_VTSHAPE, 0x000a
.equ LF_MODIFIER, 0x1001
.equ LF_POINTER, 0x1002
.equ LF_PROCEDURE, 0x1008
.equ LF_MFUNCTION, 0x1009
.equ LF_ARGLIST, 0x1201
.equ LF_FIELDLIST, 0x1203
.equ LF_BITFIELD, 0x1205
.equ LF_METHODLIST, 0x1206
.equ LF_BCLASS, 0x1400
.equ LF_VBCLASS, 0x1401
.equ LF_INDEX, 0x1404
.equ LF_VFUNCTAB, 0x1409
.equ LF_ENUMERATE, 0x1502
.equ LF_ARRAY, 0x1503
.equ LF_STRUCTURE, 0x1505
.equ LF_UNION, 0x1506
.equ LF_ENUM, 0x1507
.equ LF_MEMBER, 0x150d
.equ LF_STMEMBER, 0x150e
.equ LF_METHOD, 0x150f
.equ LF_NESTTYPE, 0x1510
.equ LF_ONEMETHOD, 0x1511
.equ LF_VFTABLE, 0x151d

.equ LF_USHORT, 0x8002
.equ LF_LONG, 0x8003
.equ LF_ULONG, 0x8004
.equ LF_UQUADWORD, 0x800a

.equ CV_PTR_NEAR32, 0xa
.equ CV_PTR_64, 0xc

.section ".debug$T", "rn"

.long CV_SIGNATURE_C13

/* Type 1000, const long */
.mod1:
.short .mod2 - .mod1 - 2
.short LF_MODIFIER
.long T_LONG
.short 1 /* const */
.p2align 2

/* Type 1001, volatile unsigned long */
.mod2:
.short .mod3 - .mod2 - 2
.short LF_MODIFIER
.long T_ULONG
.short 2 /* volatile */
.p2align 2

/* Type 1002, const volatile int */
.mod3:
.short .ptr1 - .mod3 - 2
.short LF_MODIFIER
.long T_INT4
.short 3 /* const volatile */
.p2align 2

/* Type 1003, const long * (64-bit pointer) */
.ptr1:
.short .ptr2 - .ptr1 - 2
.short LF_POINTER
.long 0x1000
.long (8 << 13) | CV_PTR_64

/* Type 1004, volatile unsigned long * (32-bit pointer) */
.ptr2:
.short .arglist1 - .ptr2 - 2
.short LF_POINTER
.long 0x1001
.long (4 << 13) | CV_PTR_NEAR32

/* Type 1005, arg list of types 1000, 1001, 1002 */
.arglist1:
.short .proc1 - .arglist1 - 2
.short LF_ARGLIST
.long 3 /* no. entries */
.long 0x1000
.long 0x1001
.long 0x1002

/* Type 1006, procedure, return type 1001, arg list 1005 */
.proc1:
.short .arr1 - .proc1 - 2
.short LF_PROCEDURE
.long 0x1001
.byte 0 /* calling convention */
.byte 0 /* attributes */
.short 3 /* no. parameters */
.long 0x1005

/* Type 1007, array[3] of const long * */
.arr1:
.short .bitfield1 - .arr1 - 2
.short LF_ARRAY
.long 0x1003 /* element type */
.long T_INT4 /* index type */
.short 24 /* length in bytes */
.byte 0 /* name */
.byte 0xf1 /* padding */

/* Type 1008, bitfield of uint32_t, position 0, length 1 */
.bitfield1:
.short .bitfield2 - .bitfield1 - 2
.short LF_BITFIELD
.long T_UINT4
.byte 1
.byte 0
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1009, bitfield of uint32_t, position 1, length 31 */
.bitfield2:
.short .fieldlist1 - .bitfield2 - 2
.short LF_BITFIELD
.long T_UINT4
.byte 31
.byte 1
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 100a, field list (1008 as num1, 1009 as num2) */
.fieldlist1:
.short .struct1 - .fieldlist1 - 2
.short LF_FIELDLIST
.short LF_MEMBER
.short 3 /* public */
.long 0x1008
.short 0 /* offset */
.asciz "num1"
.byte 0xf1 /* padding */
.short LF_MEMBER
.short 3 /* public */
.long 0x1009
.short 0 /* offset */
.asciz "num2"
.byte 0xf1 /* padding */

/* Type 100b, anonymous struct, field list 100a */
.struct1:
.short .struct2 - .struct1 - 2
.short LF_STRUCTURE
.short 2 /* no. members */
.short 0 /* property */
.long 0x100a /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 4 /* size */
.asciz "<unnamed-tag>"

/* Type 100c, forward declaration of struct foo */
.struct2:
.short .ptr3 - .struct2 - 2
.short LF_STRUCTURE
.short 0 /* no. members */
.short 0x280 /* property (has unique name, forward declaration) */
.long 0 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 0 /* size */
.asciz "foo" /* name */
.asciz "bar" /* unique name */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 100d, pointer to 100c */
.ptr3:
.short .arglist2 - .ptr3 - 2
.short LF_POINTER
.long 0x100c
.long (8 << 13) | CV_PTR_64

/* Type 100e, empty arg list */
.arglist2:
.short .mfunc1 - .arglist2 - 2
.short LF_ARGLIST
.long 0 /* no. entries */

/* Type 100f, member function of 100c, return type 1001 */
.mfunc1:
.short .mfunc2 - .mfunc1 - 2
.short LF_MFUNCTION
.long 0x1001
.long 0x100c
.long 0x100d /* type of "this" pointer */
.byte 0 /* calling convention */
.byte 0 /* attributes */
.short 0 /* no. parameters */
.long 0x100e /* arg list */
.long 0 /* "this" adjustment */

/* Type 1010, member function of 100c, return type 1001, arg list 1005 */
.mfunc2:
.short .methodlist1 - .mfunc2 - 2
.short LF_MFUNCTION
.long 0x1001
.long 0x100c
.long 0x100d /* type of "this" pointer */
.byte 0 /* calling convention */
.byte 0 /* attributes */
.short 3 /* no. parameters */
.long 0x1005 /* arg list */
.long 0 /* "this" adjustment */

/* Type 1011, method list for both member functions 100f and 1010 */
.methodlist1:
.short .fieldlist2 - .methodlist1 - 2
.short LF_METHODLIST
.short 0 /* attributes */
.short 0 /* padding */
.long 0x100f
.short 0 /* attributes */
.short 0 /* padding */
.long 0x1010

/* Type 1012, field list (uint32_t as num1) */
.fieldlist2:
.short .struct3 - .fieldlist2 - 2
.short LF_FIELDLIST
.short LF_MEMBER
.short 3 /* public */
.long T_UINT4
.short 0 /* offset */
.asciz "num"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */
.short LF_ONEMETHOD
.short 0 /* method attribute */
.long 0x100f /* method type */
.asciz "method"
.byte 0xf1
.short LF_METHOD
.short 2 /* no. overloads */
.long 0x1011 /* method list */
.asciz "method2"

/* Type 1013, struct foo, field list 1012 */
.struct3:
.short .fieldlist3 - .struct3 - 2
.short LF_STRUCTURE
.short 2 /* no. members */
.short 0x200 /* property (has unique name) */
.long 0x1012 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 4 /* size */
.asciz "foo" /* name */
.asciz "bar" /* unique name */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1014, field list (uint32_t as num1, char as num2) */
.fieldlist3:
.short .union1 - .fieldlist3 - 2
.short LF_FIELDLIST
.short LF_MEMBER
.short 3 /* public */
.long T_UINT4
.short 0 /* offset */
.asciz "num1"
.byte 0xf1 /* padding */
.short LF_MEMBER
.short 3 /* public */
.long T_CHAR
.short 0 /* offset */
.asciz "num2"
.byte 0xf1 /* padding */

/* Type 1015, anonymous union (field list 1014) */
.union1:
.short .union2 - .union1 - 2
.short LF_UNION
.short 2 /* no. members */
.short 0 /* property */
.long 0x1014
.short 4 /* size */
.asciz "<unnamed-tag>"

/* Type 1016, forward declaration of union baz */
.union2:
.short .union3 - .union2 - 2
.short LF_UNION
.short 0 /* no. members */
.short 0x280 /* property (has unique name, forward declaration) */
.long 0 /* field list */
.short 0 /* size */
.asciz "baz"
.asciz "qux"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1017, union baz (field list 1014) */
.union3:
.short .fieldlist4 - .union3 - 2
.short LF_UNION
.short 2 /* no. members */
.short 0x200 /* property (has unique name, forward declaration) */
.long 0x1014 /* field list */
.short 4 /* size */
.asciz "baz"
.asciz "qux"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1018, field list for enum (red = 0, green = 1, blue = -1, yellow = 0x8000, purple = 0x100000000) */
.fieldlist4:
.short .enum1 - .fieldlist4 - 2
.short LF_FIELDLIST
.short LF_ENUMERATE
.short 3 /* public */
.short 0 /* value */
.asciz "red"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */
.short LF_ENUMERATE
.short 3 /* public */
.short 1 /* value */
.asciz "green"
.short LF_ENUMERATE
.short 3 /* public */
.short LF_LONG
.long 0xffffffff /* value */
.asciz "blue"
.byte 0xf1 /* padding */
.short LF_ENUMERATE
.short 3 /* public */
.short LF_USHORT
.short 0x8000 /* value */
.asciz "yellow"
.byte 0xf1 /* padding */
.short LF_ENUMERATE
.short 3 /* public */
.short LF_UQUADWORD
.quad 0x100000000 /* value */
.asciz "purple"
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1019, forward reference to enum */
.enum1:
.short .enum2 - .enum1 - 2
.short LF_ENUM
.short 0 /* no. elements */
.short 0x280 /* property (has unique name, forward ref) */
.long T_UQUAD /* underlying type */
.long 0 /* field list */
.asciz "colour"
.asciz "colour2"
.byte 0xf1 /* padding */

/* Type 101a, enum (field list 1018) */
.enum2:
.short .fieldlist5 - .enum2 - 2
.short LF_ENUM
.short 5 /* no. elements */
.short 0x200 /* property (has unique name) */
.long T_UQUAD /* underlying type */
.long 0x1018 /* field list */
.asciz "colour"
.asciz "colour2"
.byte 0xf1 /* padding */

/* Type 101b, field list referencing other field list 1018 */
.fieldlist5:
.short .vtshape1 - .fieldlist5 - 2
.short LF_FIELDLIST
.short LF_INDEX
.short 0 /* padding */
.long 0x1018

/* Type 101c, virtual function table shape */
.vtshape1:
.short .ptr4 - .vtshape1 - 2
.short LF_VTSHAPE
.short 1 /* no. descriptors */
.byte 0 /* descriptor (CV_VTS_near) */
.byte 0xf1 /* padding */

/* Type 101d, pointer to 101c */
.ptr4:
.short .fieldlist6 - .ptr4 - 2
.short LF_POINTER
.long 0x101c
.long (8 << 13) | CV_PTR_64

/* Type 101e, fieldlist for enum */
.fieldlist6:
.short .enum3 - .fieldlist6 - 2
.short LF_FIELDLIST
.short LF_ENUMERATE
.short 3 /* public */
.short 0 /* value */
.asciz "a"

/* Type 101f, nested enum */
.enum3:
.short .fieldlist7 - .enum3 - 2
.short LF_ENUM
.short 1 /* no. elements */
.short 0x8 /* property (is nested) */
.long T_UINT4 /* underlying type */
.long 0x101e /* field list */
.asciz "quux::nested_enum"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1020, field list for struct quux */
.fieldlist7:
.short .struct4 - .fieldlist7 - 2
.short LF_FIELDLIST
.short LF_BCLASS
.short 0 /* attributes */
.long 0x1013 /* base class */
.short 4 /* offset within class */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */
.short LF_VFUNCTAB
.short 0 /* padding */
.long 0x101d /* pointer to vtshape */
.short LF_VBCLASS
.short 0 /* attribute */
.long 0x1013 /* type index of direct virtual base class */
.long 0x101d /* type index of virtual base pointer */
.short 0 /* virtual base pointer offset */
.short 0 /* virtual base offset from vbtable */
.short LF_STMEMBER
.short 0 /* attribute */
.long 0x1001 /* volatile unsigned long */
.asciz "static_member"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */
.short LF_NESTTYPE
.short 0 /* padding */
.long 0x101f /* enum type */
.asciz "nested_enum"

/* Type 1021, struct quux, field list 1020 */
.struct4:
.short .arr2 - .struct4 - 2
.short LF_STRUCTURE
.short 1 /* no. members */
.short 0 /* property */
.long 0x1020 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 4 /* size */
.asciz "quux" /* name */
.byte 0xf1 /* padding */

/* Type 1022, array[60000] of char */
.arr2:
.short .fieldlist8 - .arr2 - 2
.short LF_ARRAY
.long T_CHAR /* element type */
.long T_INT4 /* index type */
.short LF_USHORT
.short 60000 /* size in bytes */
.byte 0 /* name */
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1023, field list for struct longstruct */
.fieldlist8:
.short .struct5 - .fieldlist8 - 2
.short LF_FIELDLIST
.short LF_MEMBER
.short 3 /* public */
.long 0x1022
.short 0 /* offset */
.asciz "a"
.short LF_MEMBER
.short 3 /* public */
.long 0x1022
.short LF_USHORT
.short 60000 /* offset */
.asciz "b"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */
.short LF_MEMBER
.short 3 /* public */
.long 0x1022
.short LF_ULONG
.long 120000 /* offset */
.asciz "c"

/* Type 1024, struct longstruct */
.struct5:
.short .fieldlist9 - .struct5 - 2
.short LF_STRUCTURE
.short 3 /* no. members */
.short 0 /* property */
.long 0x1023 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short LF_ULONG
.long 180000 /* size */
.asciz "longstruct" /* name */
.byte 0xf3 /* padding */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1025, field list for union longunion */
.fieldlist9:
.short .union4 - .fieldlist9 - 2
.short LF_FIELDLIST
.short LF_MEMBER
.short 3 /* public */
.long 0x1022
.short 0 /* offset */
.asciz "a"
.short LF_MEMBER
.short 3 /* public */
.long 0x1022
.short 0 /* offset */
.asciz "b"

/* Type 1026, union longunion (field list 1025) */
.union4:
.short .fieldlist10 - .union4 - 2
.short LF_UNION
.short 2 /* no. members */
.short 0 /* property */
.long 0x1025 /* field list */
.short LF_USHORT
.short 60000 /* size */
.asciz "longunion"
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */

/* Type 1027, field list with base class longstruct */
.fieldlist10:
.short .fieldlist11 - .fieldlist10 - 2
.short LF_FIELDLIST
.short LF_BCLASS
.short 0 /* attributes */
.long 0x1024 /* base class */
.short LF_ULONG
.long 120000 /* offset within class */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */
.short LF_MEMBER
.short 3 /* public */
.long 0x1022
.short 0 /* offset */
.asciz "d"

/* Type 1028, field list with virtual base class longstruct */
.fieldlist11:
.short .struct6 - .fieldlist11 - 2
.short LF_FIELDLIST
.short LF_VBCLASS
.short 0 /* attributes */
.long 0x1024 /* type index of direct virtual base class */
.long 0 /* type index of virtual base pointer */
.short LF_USHORT
.short 60000 /* virtual base pointer offset */
.short LF_ULONG
.long 120000 /* virtual base offset from vbtable */
.byte 0xf2 /* padding */
.byte 0xf1 /* padding */
.short LF_MEMBER
.short 3 /* public */
.long 0x1022
.short 0 /* offset */
.asciz "d"

/* Type 1029, forward declaration of struct IUnknown */
.struct6:
.short .vftable1 - .struct6 - 2
.short LF_STRUCTURE
.short 0 /* no. members */
.short 0x80 /* property (forward declaration) */
.long 0 /* field list */
.long 0 /* type derived from */
.long 0 /* type of vshape table */
.short 0 /* size */
.asciz "IUnknown" /* name */
.byte 0xf1 /* padding */

/* Type 102a, virtual function table */
.vftable1:
.short .types_end - .vftable1 - 2
.short LF_VFTABLE
.long 0x1029 /* type */
.long 0 /* base vftable */
.long 0 /* offset */
.long .vftable1_names_end - .vftable1_names /* length of names array */
.vftable1_names:
.asciz "IUnknown"
.asciz "QueryInterface"
.asciz "AddRef"
.asciz "Release"
.vftable1_names_end:
.byte 0xf1 /* padding */

.types_end:
