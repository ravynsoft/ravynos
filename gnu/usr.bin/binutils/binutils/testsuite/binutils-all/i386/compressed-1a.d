#PROG: objcopy
#source: compressed-1.s
#as: --32 --compress-debug-sections --gdwarf-3
#objcopy:
#readelf: -w
#name: objcopy on compressed debug sections

Contents of the .[z]?debug_abbrev section:

  Number TAG \((0x)?0\)
   1      DW_TAG_compile_unit    \[has children\]
    DW_AT_producer     DW_FORM_strp
    DW_AT_language     DW_FORM_data1
    DW_AT_name         DW_FORM_strp
    DW_AT_comp_dir     DW_FORM_strp
    DW_AT_low_pc       DW_FORM_addr
    DW_AT_high_pc      DW_FORM_addr
    DW_AT_stmt_list    DW_FORM_data4
    DW_AT value: 0     DW_FORM value: 0
   2      DW_TAG_subprogram    \[no children\]
    DW_AT_external     DW_FORM_flag
    DW_AT_name         DW_FORM_strp
    DW_AT_decl_file    DW_FORM_data1
    DW_AT_decl_line    DW_FORM_data1
    DW_AT_low_pc       DW_FORM_addr
    DW_AT_high_pc      DW_FORM_addr
    DW_AT_frame_base   DW_FORM_block1
    DW_AT value: 0     DW_FORM value: 0

Contents of the .[z]?debug_info section:

  Compilation Unit @ offset (0x)?0:
   Length:        0x46 \(32-bit\)
   Version:       3
   Abbrev Offset: (0x)?0
   Pointer Size:  4
 <0><b>: Abbrev Number: 1 \(DW_TAG_compile_unit\)
    <c>   DW_AT_producer    : \(indirect string, offset: (0x)?0\): GNU C 4.4.4
    <10>   DW_AT_language    : 1	\(ANSI C\)
    <11>   DW_AT_name        : \(indirect string, offset: 0x18\): compressed-1.c
    <15>   DW_AT_comp_dir    : \(indirect string, offset: 0x16\): .
    <19>   DW_AT_low_pc      : (0x)?0
    <1d>   DW_AT_high_pc     : 0x1b
    <21>   DW_AT_stmt_list   : (0x)?0
 <1><25>: Abbrev Number: 2 \(DW_TAG_subprogram\)
    <26>   DW_AT_external    : 1
    <27>   DW_AT_name        : \(indirect string, offset: 0xc\): foo2
    <2b>   DW_AT_decl_file   : 1
    <2c>   DW_AT_decl_line   : 10
    <2d>   DW_AT_low_pc      : (0x)?0
    <31>   DW_AT_high_pc     : 0x2
    <35>   DW_AT_frame_base  : 1 byte block: 9c 	\(DW_OP_call_frame_cfa\)
 <1><37>: Abbrev Number: 2 \(DW_TAG_subprogram\)
    <38>   DW_AT_external    : 1
    <39>   DW_AT_name        : \(indirect string, offset: 0x11\): foo1
    <3d>   DW_AT_decl_file   : 1
    <3e>   DW_AT_decl_line   : 4
    <3f>   DW_AT_low_pc      : 0x10
    <43>   DW_AT_high_pc     : 0x1b
    <47>   DW_AT_frame_base  : 1 byte block: 9c 	\(DW_OP_call_frame_cfa\)
 <1><49>: Abbrev Number: 0

Raw dump of debug contents of section .[z]?debug_line:

  Offset:                      (0x)?0
  Length:                      64
  DWARF Version:               .
  Prologue Length:             37
  Minimum Instruction Length:  1
  Initial value of 'is_stmt':  1
  Line Base:                   -5
  Line Range:                  14
  Opcode Base:                 13

 Opcodes:
  Opcode 1 has 0 args
  Opcode 2 has 1 arg
  Opcode 3 has 1 arg
  Opcode 4 has 1 arg
  Opcode 5 has 1 arg
  Opcode 6 has 0 args
  Opcode 7 has 0 args
  Opcode 8 has 0 args
  Opcode 9 has 1 arg
  Opcode 10 has 0 args
  Opcode 11 has 0 args
  Opcode 12 has 1 arg

 The Directory Table is empty.

 The File Name Table \(offset 0x.*\):
  Entry	Dir	Time	Size	Name
  1	0	0	0	compressed-1.c

 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Advance Line by 10 to 11
  \[0x.*\]  Copy
  \[0x.*\]  Special opcode 6: advance Address by 0 to (0x)?0 and Line by 1 to 12 \(view 1\)
  \[0x.*\]  Advance Line by -7 to 5
  \[0x.*\]  Special opcode 229: advance Address by 16 to 0x10 and Line by 0 to 5
  \[0x.*\]  Special opcode 49: advance Address by 3 to 0x13 and Line by 2 to 7
  \[0x.*\]  Special opcode 46: advance Address by 3 to 0x16 and Line by -1 to 6
  \[0x.*\]  Advance PC by 5 to 0x1b
  \[0x.*\]  Extended opcode 1: End of Sequence


Contents of the .[z]?debug_pubnames section:

  Length:                              32
  Version:                             2
  Offset into .[z]?debug_info section:     (0x)?0
  Size of area in .[z]?debug_info section: 74

    Offset	Name
    25    	foo2
    37    	foo1

Contents of the .[z]?debug_aranges section:

  Length:                   28
  Version:                  2
  Offset into .[z]?debug_info:  (0x)?0
  Pointer Size:             4
  Segment Size:             0

    Address    Length
    00000000 0000001b ?
    00000000 00000000 ?

Contents of the .[z]?debug_str section:

  0x00000000 474e5520 4320342e 342e3400 666f6f32 GNU C 4.4.4.foo2
  0x00000010 00666f6f 31002e00 636f6d70 72657373 .foo1...compress
  0x00000020 65642d31 2e6300                     ed-1.c.

Contents of the .[z]?debug_frame section:

00000000 00000010 ffffffff CIE
  Version:               1
  Augmentation:          ""
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 8

  DW_CFA_def_cfa: r4 \(esp\) ofs 4
  DW_CFA_offset: r8 \(eip\) at cfa-4
  DW_CFA_nop
  DW_CFA_nop

00000014 0000000c 00000000 FDE cie=00000000 pc=00000000..00000002

00000024 00000014 00000000 FDE cie=00000000 pc=00000010..0000001b
  DW_CFA_advance_loc: 3 to 00000013
  DW_CFA_def_cfa_offset: 16
  DW_CFA_advance_loc: 3 to 00000016
  DW_CFA_def_cfa_offset: 4
  DW_CFA_nop
  DW_CFA_nop

