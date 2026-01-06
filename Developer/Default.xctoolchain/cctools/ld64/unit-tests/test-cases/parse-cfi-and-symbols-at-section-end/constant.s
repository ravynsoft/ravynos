	// File with an empty `dummy` symbol at the end of TEXT section
	// and minimal debug information. Debug information will make ld
	// generate BNSYM and related stabs when linking with `-r`.
	.file	1 "/missing-bnsym" "constant.c"

	.section	__TEXT,__text,regular,pure_instructions
	.p2align 2

// Dummy label right at the end of section, it has no contents
dummy:

	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 // @.str
	.asciz	"bar"

	.section	__DATA,__data
	.p2align	3
_foostr:
	.quad	l_.str

	.section	__DWARF,__debug_abbrev,regular,debug
Lsection_abbrev:
	.byte	1                               // Abbreviation Code
	.byte	17                              // DW_TAG_compile_unit
	.byte	1                               // DW_CHILDREN_yes
	.byte	37                              // DW_AT_producer
	.byte	14                              // DW_FORM_strp
	.byte	19                              // DW_AT_language
	.byte	5                               // DW_FORM_data2
	.byte	3                               // DW_AT_name
	.byte	14                              // DW_FORM_strp
	.byte	16                              // DW_AT_stmt_list
	.byte	23                              // DW_FORM_sec_offset
	.byte	27                              // DW_AT_comp_dir
	.byte	14                              // DW_FORM_strp
	.byte	0                               // EOM(1)
	.byte	0                               // EOM(2)
	.section	__DWARF,__debug_info,regular,debug
Lsection_info:
Lcu_begin0:
.set Lset0, Ldebug_info_end0-Ldebug_info_start0 // Length of Unit
	.long	Lset0
Ldebug_info_start0:
	.short	4                               // DWARF version number
.set Lset1, Lsection_abbrev-Lsection_abbrev // Offset Into Abbrev. Section
	.long	Lset1
	.byte	8                               // Address Size (in bytes)
	.byte	1                               // Abbrev [1] 0xb:0x42 DW_TAG_compile_unit
	.long	0                               // DW_AT_producer
	.short	0                             // DW_AT_language
	.long	0                               // DW_AT_name
	.long	0                               // DW_AT_LLVM_sysroot
	.long	0                               // DW_AT_APPLE_sdk
	.long	0
	.long	0                               // DW_AT_comp_dir
	.byte	0                               // End Of Children Mark
Ldebug_info_end0:
	.section	__DWARF,__debug_str,regular,debug
Linfo_string:
	.asciz	"Apple" // string offset=0
.subsections_via_symbols
