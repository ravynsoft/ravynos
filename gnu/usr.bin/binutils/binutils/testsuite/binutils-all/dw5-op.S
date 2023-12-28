/* Copyright (C) 2017-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

       .file   "main.c"
       .text
.Letext0:
       .section        .debug_info,"",%progbits
.Ldebug_info0:
       .4byte  0x4e    /* Length of Compilation Unit Info */
       .2byte  0x5     /* DWARF version number */
       .byte   0x1     /* DW_UT_compile */
       .byte   0x8     /* Pointer Size (in bytes) */
       .4byte  .Ldebug_abbrev0 /* Offset Into Abbrev. Section */
       .uleb128 0x3    /* (DIE (0xc) DW_TAG_compile_unit) */
       .4byte  .LASF21 /* DW_AT_producer: "GNU C11 7.0.1 20170218 (experimental) -mtune=generic -march=x86-64 -gdwarf-5 -O2" */
       .byte   0x1d    /* DW_AT_language */
       .4byte  .LASF0  /* DW_AT_name: "main.c" */
       .4byte  .LASF1  /* DW_AT_comp_dir: "" */
       .4byte  .LLRL2  /* DW_AT_ranges */
       .8byte  0       /* DW_AT_low_pc */
       .4byte  .Ldebug_line0   /* DW_AT_stmt_list */
       .uleb128 0x1    /* (DIE (0x2a) DW_TAG_base_type) */
       .byte   0x4     /* DW_AT_byte_size */
       .byte   0x5     /* DW_AT_encoding */
       .4byte  .LASF2  /* DW_AT_name: "short int" */
       .uleb128 0x2    /* (DIE (0x31) DW_TAG_variable) */
       .4byte  .LASF16 /* DW_AT_name: "xvar" */
                       /* DW_AT_decl_file (1, main.c) */
       .byte   0x2     /* DW_AT_decl_line */
       .4byte  0x2a    /* DW_AT_type */
                       /* DW_AT_external */
       .uleb128 0x9    /* DW_AT_location */
       .byte   0x3     /* DW_OP_addr */
       .8byte  0x1234
       .uleb128 0x2    /* (DIE (0x45) DW_TAG_variable) */
       .4byte  .LASF17 /* DW_AT_name: "yvar" */
                       /* DW_AT_decl_file (1, main.c) */
       .byte   0x3     /* DW_AT_decl_line */
       .4byte  0x2a    /* DW_AT_type */
                       /* DW_AT_external */
       .uleb128 0x2    /* DW_AT_location */
       .byte   0xa1    /* DW_OP_addrx */
       .byte   0x0
       .section        .debug_abbrev,"",%progbits
.Ldebug_abbrev0:
       .uleb128 0x1    /* (abbrev code) */
       .uleb128 0x24   /* (TAG: DW_TAG_base_type) */
       .byte   0       /* DW_children_no */
       .uleb128 0xb    /* (DW_AT_byte_size) */
       .uleb128 0xb    /* (DW_FORM_data1) */
       .uleb128 0x3e   /* (DW_AT_encoding) */
       .uleb128 0xb    /* (DW_FORM_data1) */
       .uleb128 0x3    /* (DW_AT_name) */
       .uleb128 0xe    /* (DW_FORM_strp) */
       .byte   0
       .byte   0
       .uleb128 0x2    /* (abbrev code) */
       .uleb128 0x34   /* (TAG: DW_TAG_variable) */
       .byte   0       /* DW_children_no */
       .uleb128 0x3    /* (DW_AT_name) */
       .uleb128 0xe    /* (DW_FORM_strp) */
       .uleb128 0x3a   /* (DW_AT_decl_file) */
       .uleb128 0x21   /* (DW_FORM_implicit_const) */
       .sleb128 1      /* (main.c) */
       .uleb128 0x3b   /* (DW_AT_decl_line) */
       .uleb128 0xb    /* (DW_FORM_data1) */
       .uleb128 0x49   /* (DW_AT_type) */
       .uleb128 0x13   /* (DW_FORM_ref4) */
       .uleb128 0x3f   /* (DW_AT_external) */
       .uleb128 0x19   /* (DW_FORM_flag_present) */
       .uleb128 0x2    /* (DW_AT_location) */
       .uleb128 0x18   /* (DW_FORM_exprloc) */
       .byte   0
       .byte   0
       .uleb128 0x3    /* (abbrev code) */
       .uleb128 0x11   /* (TAG: DW_TAG_compile_unit) */
       .byte   0x1     /* DW_children_yes */
       .uleb128 0x25   /* (DW_AT_producer) */
       .uleb128 0xe    /* (DW_FORM_strp) */
       .uleb128 0x13   /* (DW_AT_language) */
       .uleb128 0xb    /* (DW_FORM_data1) */
       .uleb128 0x3    /* (DW_AT_name) */
       .uleb128 0x1f   /* (DW_FORM_line_strp) */
       .uleb128 0x1b   /* (DW_AT_comp_dir) */
       .uleb128 0x1f   /* (DW_FORM_line_strp) */
       .uleb128 0x55   /* (DW_AT_ranges) */
       .uleb128 0x17   /* (DW_FORM_sec_offset) */
       .uleb128 0x11   /* (DW_AT_low_pc) */
       .uleb128 0x1    /* (DW_FORM_addr) */
       .uleb128 0x10   /* (DW_AT_stmt_list) */
       .uleb128 0x17   /* (DW_FORM_sec_offset) */
       .byte   0
       .byte   0
       .byte   0
       .section        .debug_line,"",%progbits
.Ldebug_line0:
       .4byte  .LELT0-.LSLT0   /* Length of Source Line Info */
.LSLT0:
       .2byte  0x5     /* DWARF Version */
       .byte   0x8     /* Address Size */
       .byte   0       /* Segment Size */
       .4byte  .LASF0  /* File Entry: 0: "main.c" */
       .byte   0
.LELT0:
       .section        .debug_str,"MS",%progbits,1
.LASF22:
       .asciz  "main"
.LASF16:
       .asciz  "xvar"
.LASF21:
       .asciz  "GNU C11 7.0.1 20170218 (experimental) -mtune=generic -march=x86-64 -gdwarf-5 -O2"
.LASF17:
       .asciz  "yvar"
.LASF7:
       .asciz  "short int"
       .section        .debug_line_str,"MS",%progbits,1
.LASF1:
       .asciz  ""
.LASF25:
       .asciz  ""
.LASF0:
       .asciz  "main.c"
