#source: dw2-compress-3.s
#as: --compress-debug-sections --elf-stt-common=no --gdwarf-3
#readelf: -w
#name: DWARF2 debugging information 3 w/o STT_COMMON

Contents of the .debug_info section:

  Compilation Unit @ offset (0x)?0:
   Length:        0x32 \(32-bit\)
   Version:       4
   Abbrev Offset: (0x)?0
   Pointer Size:  4
 <0><b>: Abbrev Number: 1 \(DW_TAG_compile_unit\)
    <c>   DW_AT_producer    : \(indirect string, offset: 0x2\): GNU C 4.8.3
    <10>   DW_AT_language    : 1	\(ANSI C\)
    <11>   DW_AT_name        : \(indirect string, offset: 0xe\): dw2-compress-3.c
    <15>   DW_AT_comp_dir    : \(indirect string, offset: (0x)?0\): .
    <19>   DW_AT_stmt_list   : (0x)?0
 <1><1d>: Abbrev Number: 2 \(DW_TAG_variable\)
    <1e>   DW_AT_name        : foo
    <22>   DW_AT_decl_file   : 1
    <23>   DW_AT_decl_line   : 1
    <24>   DW_AT_type        : <0x2e>
    <28>   DW_AT_external    : 1
    <28>   DW_AT_location    : 5 byte block: 3 4 0 0 0 	\(DW_OP_addr: 4\)
 <1><2e>: Abbrev Number: 3 \(DW_TAG_base_type\)
    <2f>   DW_AT_byte_size   : 4
    <30>   DW_AT_encoding    : 5	\(signed\)
    <31>   DW_AT_name        : int
 <1><35>: Abbrev Number: 0

Contents of the .debug_abbrev section:

  Number TAG \((0x)?0\)
   1      DW_TAG_compile_unit    \[has children\]
    DW_AT_producer     DW_FORM_strp
    DW_AT_language     DW_FORM_data1
    DW_AT_name         DW_FORM_strp
    DW_AT_comp_dir     DW_FORM_strp
    DW_AT_stmt_list    DW_FORM_sec_offset
    DW_AT value: 0     DW_FORM value: 0
   2      DW_TAG_variable    \[no children\]
    DW_AT_name         DW_FORM_string
    DW_AT_decl_file    DW_FORM_data1
    DW_AT_decl_line    DW_FORM_data1
    DW_AT_type         DW_FORM_ref4
    DW_AT_external     DW_FORM_flag_present
    DW_AT_location     DW_FORM_exprloc
    DW_AT value: 0     DW_FORM value: 0
   3      DW_TAG_base_type    \[no children\]
    DW_AT_byte_size    DW_FORM_data1
    DW_AT_encoding     DW_FORM_data1
    DW_AT_name         DW_FORM_string
    DW_AT value: 0     DW_FORM value: 0

Contents of the .debug_aranges section:

  Length:                   20
  Version:                  2
  Offset into .debug_info:  (0x)?0
  Pointer Size:             4
  Segment Size:             0

    Address    Length
    00000000 00000000 ?

Raw dump of debug contents of section .debug_line:

  Offset:                      (0x)?0
  Length:                      45
  DWARF Version:               3
  Prologue Length:             39
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

 The File Name Table \(offset 0x1c\):
  Entry	Dir	Time	Size	Name
  1	0	0	0	dw2-compress-3.c

 No Line Number Statements.
Contents of the .debug_str section:

  0x00000000 2e00474e 55204320 342e382e 33006477 ..GNU C 4.8.3.dw
  0x00000010 322d636f 6d707265 73732d33 2e6300   2-compress-3.c.

