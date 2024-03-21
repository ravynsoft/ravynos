	.file	1 "dwarfreloc1.c"
	.comm   i1,4,4
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_info
.Ldebug_info0:
	.long	.Ldebug_info_end - .Ldebug_info_start
.Ldebug_info_start:
	.value	0x2
	.long	.Ldebug_abbrev0
	.byte	0x8

	.uleb128 0x1		/* DW_TAG_compile_unit */
	.long	.LASF0		/* DW_AT_producer */
	.byte	0x1		/* DW_AT_language */
	.long	.LASF1		/* DW_AT_name */
	.long	.LASF2		/* DW_AT_comp_dir */
	.long	.Ldebug_line0	/* DW_AT_stmt_list */

	.uleb128 0x2		/* DW_TAG_variable */
	.string	"i1"		/* DW_AT_name */
	.byte	0x1		/* DW_AT_decl_file */
	.byte	0x1		/* DW_AT_decl_line */
	.long	.dwarfreloc1.0.2 - .Ldebug_info0 /* DW_AT_type (DW_FORM_ref4) */
	.byte	0x1		/* DW_AT_external */
	.byte	0x9		/* DW_AT_location: length */
	.byte	0x3		/* DW_AT_location: DW_OP_addr */
	.quad	i1		/* DW_AT_location: DW_OP_addr: address */

	/* DWARF3 Page 224 (236/267)
	<prefix>.<file-designator>.<gid-number>.<die-number>  */
	.globl .dwarfreloc1.0.2
.dwarfreloc1.0.2:
	.uleb128 0x3		/* DW_TAG_base_type */
	.byte	0x4		/* DW_AT_byte_size */
	.byte	0x5		/* DW_AT_encoding */
	.string	"int"		/* DW_AT_name */

	.byte	0x0
.Ldebug_info_end:

	.section	.debug_abbrev
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11

	.byte	0x1
	.uleb128 0x25	/* DW_AT_producer */
	.uleb128 0xe	/* DW_FORM_strp */
	.uleb128 0x13	/* DW_AT_language */
	.uleb128 0xb	/* DW_FORM_data1 */
	.uleb128 0x3	/* DW_AT_name */
	.uleb128 0xe	/* DW_FORM_strp */
	.uleb128 0x1b	/* DW_AT_comp_dir */
	.uleb128 0xe	/* DW_FORM_strp */
	.uleb128 0x10	/* DW_AT_stmt_list */
	.uleb128 0x6	/* DW_FORM_data4 */
	.byte	0x0
	.byte	0x0

	.uleb128 0x2
	.uleb128 0x34	/* DW_TAG_variable */
	.byte	0x0
	.uleb128 0x3	/* DW_AT_name */
	.uleb128 0x8	/* DW_FORM_string */
	.uleb128 0x3a	/* DW_AT_decl_file */
	.uleb128 0xb	/* DW_FORM_data1 */
	.uleb128 0x3b	/* DW_AT_decl_line */
	.uleb128 0xb	/* DW_FORM_data1 */
	.uleb128 0x49	/* DW_AT_type */
	.uleb128 0x13	/* DW_FORM_ref4 */
	.uleb128 0x3f	/* DW_AT_external */
	.uleb128 0xc	/* DW_FORM_flag */
	.uleb128 0x2	/* DW_AT_location */
	.uleb128 0xa	/* DW_FORM_block1 */
	.byte	0x0
	.byte	0x0

	.uleb128 0x3
	.uleb128 0x24	/* DW_TAG_base_type */
	.byte	0x0
	.uleb128 0xb	/* DW_AT_byte_size */
	.uleb128 0xb	/* DW_FORM_data1 */
	.uleb128 0x3e	/* DW_AT_encoding */
	.uleb128 0xb	/* DW_FORM_data1 */
	.uleb128 0x3	/* DW_AT_name */
	.uleb128 0x8	/* DW_FORM_string */
	.byte	0x0
	.byte	0x0

	.byte	0x0

	.section	.debug_str,"MS",@progbits,1
.LASF1:
	.string	"dwarfreloc1.c"
.LASF0:
	.string	"GNU C 4.3.1 20080801 (Red Hat 4.3.1-6)"
.LASF2:
	.string	"/"
	.ident	"GCC: (GNU) 4.3.1 20080801 (Red Hat 4.3.1-6)"
	.section	.note.GNU-stack,"",@progbits
