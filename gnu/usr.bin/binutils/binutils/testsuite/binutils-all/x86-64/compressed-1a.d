#PROG: objcopy
#source: compressed-1.s
#as: --64 --compress-debug-sections --gdwarf-3
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
   Length:        0x5e \(32-bit\)
   Version:       3
   Abbrev Offset: (0x)?0
   Pointer Size:  8
 <0><b>: Abbrev Number: 1 \(DW_TAG_compile_unit\)
    <c>   DW_AT_producer    : \(indirect string, offset: (0x)?0\): GNU C 4.4.4
    <10>   DW_AT_language    : 1	\(ANSI C\)
    <11>   DW_AT_name        : \(indirect string, offset: 0x18\): compressed-1.c
    <15>   DW_AT_comp_dir    : \(indirect string, offset: 0x16\): .
    <19>   DW_AT_low_pc      : (0x)?0
    <21>   DW_AT_high_pc     : 0x15
    <29>   DW_AT_stmt_list   : (0x)?0
 <1><2d>: Abbrev Number: 2 \(DW_TAG_subprogram\)
    <2e>   DW_AT_external    : 1
    <2f>   DW_AT_name        : \(indirect string, offset: 0xc\): foo2
    <33>   DW_AT_decl_file   : 1
    <34>   DW_AT_decl_line   : 10
    <35>   DW_AT_low_pc      : (0x)?0
    <3d>   DW_AT_high_pc     : 0x2
    <45>   DW_AT_frame_base  : 1 byte block: 9c 	\(DW_OP_call_frame_cfa\)
 <1><47>: Abbrev Number: 2 \(DW_TAG_subprogram\)
    <48>   DW_AT_external    : 1
    <49>   DW_AT_name        : \(indirect string, offset: 0x11\): foo1
    <4d>   DW_AT_decl_file   : 1
    <4e>   DW_AT_decl_line   : 4
    <4f>   DW_AT_low_pc      : 0x10
    <57>   DW_AT_high_pc     : 0x15
    <5f>   DW_AT_frame_base  : 1 byte block: 9c 	\(DW_OP_call_frame_cfa\)
 <1><61>: Abbrev Number: 0

Raw dump of debug contents of section .[z]?debug_line:

  Offset:                      (0x)?0
  Length:                      67
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
  \[0x.*\]  Special opcode 6: advance Address by 0 to 0x10 and Line by 1 to 6 \(view 1\)
  \[0x.*\]  Advance PC by 5 to 0x15
  \[0x.*\]  Extended opcode 1: End of Sequence


Contents of the .[z]?debug_pubnames section:

  Length:                              32
  Version:                             2
  Offset into .[z]?debug_info section:     (0x)?0
  Size of area in .[z]?debug_info section: 98

    Offset	Name
    2d    	foo2
    47    	foo1

Contents of the .[z]?debug_aranges section:

  Length:                   44
  Version:                  2
  Offset into .[z]?debug_info:  (0x)?0
  Pointer Size:             8
  Segment Size:             0

    Address            Length
    0000000000000000 0000000000000015 ?
    0000000000000000 0000000000000000 ?

Contents of the .[z]?debug_str section:

  0x00000000 474e5520 4320342e 342e3400 666f6f32 GNU C 4.4.4.foo2
  0x00000010 00666f6f 31002e00 636f6d70 72657373 .foo1...compress
  0x00000020 65642d31 2e6300                     ed-1.c.

Contents of the .[z]?debug_frame section:

0+ 0+14 0*ffffffff CIE
  Version:               1
  Augmentation:          ""
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: 16

  DW_CFA_def_cfa: r7 \(rsp\) ofs 8
  DW_CFA_offset: r16 \(rip\) at cfa-8
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+18 0+14 0+ FDE cie=0+ pc=0+..0+2

0+30 0+14 0+ FDE cie=0+ pc=0+10..0+15

