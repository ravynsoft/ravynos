/*
  Copyright (C) 2000,2001,2003,2004,2005,2006 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307,
  USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


#ifndef __DWARF_H
#define __DWARF_H
#ifdef __cplusplus
extern "C" {
#endif

/*
        dwarf.h   DWARF  debugging information values
        $Revision: 1.41 $    $Date: 2006/04/17 00:09:56 $

        The comment "DWARF3" appears where there are
        new entries from DWARF3 as of 2004, "DWARF3f"
        where there are new entries as of the November 2005
        public review document and other comments apply
        where extension entries appear.

        Extensions part of DWARF4 are marked DWARF4.

        A few extension names have omitted the 'vendor id'
        (See chapter 7, "Vendor Extensibility"). Please
        always use a 'vendor id' string in extension names.

        Vendors should use a vendor string in names and
        whereever possible avoid duplicating values used by
        other vendor extensions

*/


#define DW_TAG_array_type               0x01
#define DW_TAG_class_type               0x02
#define DW_TAG_entry_point              0x03
#define DW_TAG_enumeration_type         0x04
#define DW_TAG_formal_parameter         0x05
#define DW_TAG_imported_declaration     0x08
#define DW_TAG_label                    0x0a
#define DW_TAG_lexical_block            0x0b
#define DW_TAG_member                   0x0d
#define DW_TAG_pointer_type             0x0f
#define DW_TAG_reference_type           0x10
#define DW_TAG_compile_unit             0x11
#define DW_TAG_string_type              0x12
#define DW_TAG_structure_type           0x13
#define DW_TAG_subroutine_type          0x15
#define DW_TAG_typedef                  0x16
#define DW_TAG_union_type               0x17
#define DW_TAG_unspecified_parameters   0x18
#define DW_TAG_variant                  0x19
#define DW_TAG_common_block             0x1a
#define DW_TAG_common_inclusion         0x1b
#define DW_TAG_inheritance              0x1c
#define DW_TAG_inlined_subroutine       0x1d
#define DW_TAG_module                   0x1e
#define DW_TAG_ptr_to_member_type       0x1f
#define DW_TAG_set_type                 0x20
#define DW_TAG_subrange_type            0x21
#define DW_TAG_with_stmt                0x22
#define DW_TAG_access_declaration       0x23
#define DW_TAG_base_type                0x24
#define DW_TAG_catch_block              0x25
#define DW_TAG_const_type               0x26
#define DW_TAG_constant                 0x27
#define DW_TAG_enumerator               0x28
#define DW_TAG_file_type                0x29
#define DW_TAG_friend                   0x2a
#define DW_TAG_namelist                 0x2b
        /* Early releases of this header had the following
           misspelled with a trailing 's' */
#define DW_TAG_namelist_item            0x2c /* DWARF3/2 spelling */
#define DW_TAG_namelist_items           0x2c /* SGI misspelling/typo */
#define DW_TAG_packed_type              0x2d
#define DW_TAG_subprogram               0x2e
        /* The DWARF2 document had two spellings of the following
           two TAGs, DWARF3 specifies the longer spelling. */
#define DW_TAG_template_type_parameter  0x2f /* DWARF3/2 spelling*/
#define DW_TAG_template_type_param      0x2f /* DWARF2   spelling*/
#define DW_TAG_template_value_parameter 0x30 /* DWARF3/2 spelling*/
#define DW_TAG_template_value_param     0x30 /* DWARF2   spelling*/
#define DW_TAG_thrown_type              0x31
#define DW_TAG_try_block                0x32
#define DW_TAG_variant_part             0x33
#define DW_TAG_variable                 0x34
#define DW_TAG_volatile_type            0x35
#define DW_TAG_dwarf_procedure          0x36  /* DWARF3 */
#define DW_TAG_restrict_type            0x37  /* DWARF3 */
#define DW_TAG_interface_type           0x38  /* DWARF3 */
#define DW_TAG_namespace                0x39  /* DWARF3 */
#define DW_TAG_imported_module          0x3a  /* DWARF3 */
#define DW_TAG_unspecified_type         0x3b  /* DWARF3 */
#define DW_TAG_partial_unit             0x3c  /* DWARF3 */
#define DW_TAG_imported_unit            0x3d  /* DWARF3 */
				 /* Do not use DW_TAG_mutable_type */
#define DW_TAG_mutable_type 0x3e /* Withdrawn from DWARF3 by DWARF3f. */
#define DW_TAG_condition                0x3f  /* DWARF3f */
#define DW_TAG_shared_type              0x40  /* DWARF3f */
#define DW_TAG_type_unit                0x41  /* DWARF4 */
#define DW_TAG_rvalue_reference_type    0x42  /* DWARF4 */
#define DW_TAG_lo_user                  0x4080

#define DW_TAG_MIPS_loop                0x4081

/* HP extensions: ftp://ftp.hp.com/pub/lang/tools/WDB/wdb-4.0.tar.gz  */
#define DW_TAG_HP_array_descriptor      0x4090 /* HP */

/* GNU extensions.  The first 3 missing the GNU_. */
#define DW_TAG_format_label             0x4101 /* GNU. Fortran. */
#define DW_TAG_function_template        0x4102 /* GNU. For C++ */
#define DW_TAG_class_template           0x4103 /* GNU. For C++ */
#define DW_TAG_GNU_BINCL                0x4104 /* GNU */
#define DW_TAG_GNU_EINCL                0x4105 /* GNU */

/* Apple extensions */
#define DW_TAG_APPLE_property           0x4200
#define DW_TAG_APPLE_ptrauth_type       0x4300

/* ALTIUM extensions */
	/* DSP-C/Starcore __circ qualifier */
#define DW_TAG_ALTIUM_circ_type         0x5101 /* ALTIUM */
	/* Starcore __mwa_circ qualifier */ 
#define DW_TAG_ALTIUM_mwa_circ_type     0x5102 /* ALTIUM */
	/* Starcore __rev_carry qualifier */
#define DW_TAG_ALTIUM_rev_carry_type    0x5103 /* ALTIUM */
	/* M16 __rom qualifier */
#define DW_TAG_ALTIUM_rom               0x5111 /* ALTIUM */

/* The following 3 are extensions to support UPC */
#define DW_TAG_upc_shared_type          0x8765 /* UPC */
#define DW_TAG_upc_strict_type          0x8766 /* UPC */
#define DW_TAG_upc_relaxed_type         0x8767 /* UPC */

/* PGI (STMicroelectronics) extensions. */
#define DW_TAG_PGI_kanji_type           0xa000 /* PGI */
#define DW_TAG_PGI_interface_block      0xa020 /* PGI */

#define DW_TAG_hi_user                  0xffff

#define DW_children_no                  0
#define DW_children_yes                 1


#define DW_FORM_addr                    0x01
#define DW_FORM_block2                  0x03
#define DW_FORM_block4                  0x04
#define DW_FORM_data2                   0x05
#define DW_FORM_data4                   0x06
#define DW_FORM_data8                   0x07
#define DW_FORM_string                  0x08
#define DW_FORM_block                   0x09
#define DW_FORM_block1                  0x0a
#define DW_FORM_data1                   0x0b
#define DW_FORM_flag                    0x0c
#define DW_FORM_sdata                   0x0d
#define DW_FORM_strp                    0x0e
#define DW_FORM_udata                   0x0f
#define DW_FORM_ref_addr                0x10
#define DW_FORM_ref1                    0x11
#define DW_FORM_ref2                    0x12
#define DW_FORM_ref4                    0x13
#define DW_FORM_ref8                    0x14
#define DW_FORM_ref_udata               0x15
#define DW_FORM_indirect                0x16
#define DW_FORM_sec_offset              0x17 /* DWARF4 */
#define DW_FORM_exprloc                 0x18 /* DWARF4 */
#define DW_FORM_flag_present            0x19 /* DWARF4 */
  /* 0x1a thru 0x1f were left unused accidentally. Reserved for future use. */
#define DW_FORM_ref_sig8                0x20 /* DWARF4 */

#define DW_AT_sibling                           0x01
#define DW_AT_location                          0x02
#define DW_AT_name                              0x03
#define DW_AT_ordering                          0x09
#define DW_AT_subscr_data                       0x0a
#define DW_AT_byte_size                         0x0b
#define DW_AT_bit_offset                        0x0c
#define DW_AT_bit_size                          0x0d
#define DW_AT_element_list                      0x0f
#define DW_AT_stmt_list                         0x10
#define DW_AT_low_pc                            0x11
#define DW_AT_high_pc                           0x12
#define DW_AT_language                          0x13
#define DW_AT_member                            0x14
#define DW_AT_discr                             0x15
#define DW_AT_discr_value                       0x16
#define DW_AT_visibility                        0x17
#define DW_AT_import                            0x18
#define DW_AT_string_length                     0x19
#define DW_AT_common_reference                  0x1a
#define DW_AT_comp_dir                          0x1b
#define DW_AT_const_value                       0x1c
#define DW_AT_containing_type                   0x1d
#define DW_AT_default_value                     0x1e
#define DW_AT_inline                            0x20
#define DW_AT_is_optional                       0x21
#define DW_AT_lower_bound                       0x22
#define DW_AT_producer                          0x25
#define DW_AT_prototyped                        0x27
#define DW_AT_return_addr                       0x2a
#define DW_AT_start_scope                       0x2c
#define DW_AT_bit_stride                        0x2e /* DWARF3 name */
#define DW_AT_stride_size                       0x2e /* DWARF2 name */
#define DW_AT_upper_bound                       0x2f
#define DW_AT_abstract_origin                   0x31
#define DW_AT_accessibility                     0x32
#define DW_AT_address_class                     0x33
#define DW_AT_artificial                        0x34
#define DW_AT_base_types                        0x35
#define DW_AT_calling_convention                0x36
#define DW_AT_count                             0x37
#define DW_AT_data_member_location              0x38
#define DW_AT_decl_column                       0x39
#define DW_AT_decl_file                         0x3a
#define DW_AT_decl_line                         0x3b
#define DW_AT_declaration                       0x3c
#define DW_AT_discr_list                        0x3d
#define DW_AT_encoding                          0x3e
#define DW_AT_external                          0x3f
#define DW_AT_frame_base                        0x40
#define DW_AT_friend                            0x41
#define DW_AT_identifier_case                   0x42
#define DW_AT_macro_info                        0x43
#define DW_AT_namelist_item                     0x44
#define DW_AT_priority                          0x45
#define DW_AT_segment                           0x46
#define DW_AT_specification                     0x47
#define DW_AT_static_link                       0x48
#define DW_AT_type                              0x49
#define DW_AT_use_location                      0x4a
#define DW_AT_variable_parameter                0x4b
#define DW_AT_virtuality                        0x4c
#define DW_AT_vtable_elem_location              0x4d
#define DW_AT_allocated                         0x4e /* DWARF3 */
#define DW_AT_associated                        0x4f /* DWARF3 */
#define DW_AT_data_location                     0x50 /* DWARF3 */
#define DW_AT_byte_stride                       0x51 /* DWARF3f */
#define DW_AT_stride                            0x51 /* DWARF3 (do not use) */
#define DW_AT_entry_pc                          0x52 /* DWARF3 */
#define DW_AT_use_UTF8                          0x53 /* DWARF3 */
#define DW_AT_extension                         0x54 /* DWARF3 */
#define DW_AT_ranges                            0x55 /* DWARF3 */
#define DW_AT_trampoline                        0x56 /* DWARF3 */
#define DW_AT_call_column                       0x57 /* DWARF3 */
#define DW_AT_call_file                         0x58 /* DWARF3 */
#define DW_AT_call_line                         0x59 /* DWARF3 */
#define DW_AT_description                       0x5a /* DWARF3 */
#define DW_AT_binary_scale                      0x5b /* DWARF3f */
#define DW_AT_decimal_scale                     0x5c /* DWARF3f */
#define DW_AT_small                             0x5d /* DWARF3f */
#define DW_AT_decimal_sign                      0x5e /* DWARF3f */
#define DW_AT_digit_count                       0x5f /* DWARF3f */
#define DW_AT_picture_string                    0x60 /* DWARF3f */
#define DW_AT_mutable                           0x61 /* DWARF3f */
#define DW_AT_threads_scaled                    0x62 /* DWARF3f */
#define DW_AT_explicit                          0x63 /* DWARF3f */
#define DW_AT_object_pointer                    0x64 /* DWARF3f */
#define DW_AT_endianity                         0x65 /* DWARF3f */
#define DW_AT_elemental                         0x66 /* DWARF3f */
#define DW_AT_pure                              0x67 /* DWARF3f */
#define DW_AT_recursive                         0x68 /* DWARF3f */
#define DW_AT_signature                         0x69 /* DWARF4 */
#define DW_AT_main_subprogram                   0x6a /* DWARF4 */
#define DW_AT_data_bit_offset                   0x6b /* DWARF4 */
#define DW_AT_const_expr                        0x6c /* DWARF4 */
#define DW_AT_enum_class                        0x6d /* DWARF4 */
#define DW_AT_linkage_name                      0x6e /* DWARF4 */

/* HP extensions. */
#define DW_AT_HP_block_index                    0x2000  /* HP */

/* Follows extension so dwarfdump prints the most-likely-useful name. */
#define DW_AT_lo_user                           0x2000

#define DW_AT_MIPS_fde                          0x2001 /* MIPS/SGI */
#define DW_AT_MIPS_loop_begin                   0x2002 /* MIPS/SGI */
#define DW_AT_MIPS_tail_loop_begin              0x2003 /* MIPS/SGI */
#define DW_AT_MIPS_epilog_begin                 0x2004 /* MIPS/SGI */
#define DW_AT_MIPS_loop_unroll_factor           0x2005 /* MIPS/SGI */
#define DW_AT_MIPS_software_pipeline_depth      0x2006 /* MIPS/SGI */
#define DW_AT_MIPS_linkage_name                 0x2007 /* MIPS/SGI */
#define DW_AT_MIPS_stride                       0x2008 /* MIPS/SGI */
#define DW_AT_MIPS_abstract_name                0x2009 /* MIPS/SGI */
#define DW_AT_MIPS_clone_origin                 0x200a /* MIPS/SGI */
#define DW_AT_MIPS_has_inlines                  0x200b /* MIPS/SGI */
#define DW_AT_MIPS_stride_byte                  0x200c /* MIPS/SGI */
#define DW_AT_MIPS_stride_elem                  0x200d /* MIPS/SGI */
#define DW_AT_MIPS_ptr_dopetype                 0x200e /* MIPS/SGI */
#define DW_AT_MIPS_allocatable_dopetype         0x200f /* MIPS/SGI */
#define DW_AT_MIPS_assumed_shape_dopetype       0x2010 /* MIPS/SGI */
#define DW_AT_MIPS_assumed_size                 0x2011 /* MIPS/SGI */

/* HP extensions. */
#define DW_AT_HP_unmodifiable                   0x2001 /* conflict: MIPS */
#define DW_AT_HP_actuals_stmt_list              0x2010 /* conflict: MIPS */
#define DW_AT_HP_proc_per_section               0x2011 /* conflict: MIPS */
#define DW_AT_HP_raw_data_ptr                   0x2012 /* HP */
#define DW_AT_HP_pass_by_reference              0x2013 /* HP */
#define DW_AT_HP_opt_level                      0x2014 /* HP */
#define DW_AT_HP_prof_version_id                0x2015 /* HP */
#define DW_AT_HP_opt_flags                      0x2016 /* HP */
#define DW_AT_HP_cold_region_low_pc             0x2017 /* HP */
#define DW_AT_HP_cold_region_high_pc            0x2018 /* HP */
#define DW_AT_HP_all_variables_modifiable       0x2019 /* HP */
#define DW_AT_HP_linkage_name                   0x201a /* HP */
#define DW_AT_HP_prof_flags                     0x201b /* HP */


/* GNU extensions. */
#define DW_AT_sf_names                          0x2101 /* GNU */
#define DW_AT_src_info                          0x2102 /* GNU */
#define DW_AT_mac_info                          0x2103 /* GNU */
#define DW_AT_src_coords                        0x2104 /* GNU */
#define DW_AT_body_begin                        0x2105 /* GNU */
#define DW_AT_body_end                          0x2106 /* GNU */
#define DW_AT_GNU_vector                        0x2107 /* GNU */

/* VMS extensions. */
#define DW_AT_VMS_rtnbeg_pd_address             0x2201 /* VMS */

/* ALTIUM extension: ALTIUM Compliant location lists (flag) */
#define DW_AT_ALTIUM_loclist    0x2300          /* ALTIUM  */

/* PGI (STMicroelectronics) extensions. */
#define DW_AT_PGI_lbase                         0x3a00 /* PGI */
#define DW_AT_PGI_soffset                       0x3a01 /* PGI */
#define DW_AT_PGI_lstride                       0x3a02 /* PGI */


/* UPC extension */
#define DW_AT_upc_threads_scaled                0x3210 /* UPC */

/* Apple extension */
#define DW_AT_APPLE_ptrauth_key                       0x3e04
#define DW_AT_APPLE_ptrauth_address_discriminated     0x3e05
#define DW_AT_APPLE_ptrauth_extra_discriminator       0x3e06

#define DW_AT_hi_user                           0x3fff


#define DW_OP_addr                      0x03
#define DW_OP_deref                     0x06
#define DW_OP_const1u                   0x08
#define DW_OP_const1s                   0x09
#define DW_OP_const2u                   0x0a
#define DW_OP_const2s                   0x0b
#define DW_OP_const4u                   0x0c
#define DW_OP_const4s                   0x0d
#define DW_OP_const8u                   0x0e
#define DW_OP_const8s                   0x0f
#define DW_OP_constu                    0x10
#define DW_OP_consts                    0x11
#define DW_OP_dup                       0x12
#define DW_OP_drop                      0x13
#define DW_OP_over                      0x14
#define DW_OP_pick                      0x15
#define DW_OP_swap                      0x16
#define DW_OP_rot                       0x17
#define DW_OP_xderef                    0x18
#define DW_OP_abs                       0x19
#define DW_OP_and                       0x1a
#define DW_OP_div                       0x1b
#define DW_OP_minus                     0x1c
#define DW_OP_mod                       0x1d
#define DW_OP_mul                       0x1e
#define DW_OP_neg                       0x1f
#define DW_OP_not                       0x20
#define DW_OP_or                        0x21
#define DW_OP_plus                      0x22
#define DW_OP_plus_uconst               0x23
#define DW_OP_shl                       0x24
#define DW_OP_shr                       0x25
#define DW_OP_shra                      0x26
#define DW_OP_xor                       0x27
#define DW_OP_bra                       0x28
#define DW_OP_eq                        0x29
#define DW_OP_ge                        0x2a
#define DW_OP_gt                        0x2b
#define DW_OP_le                        0x2c
#define DW_OP_lt                        0x2d
#define DW_OP_ne                        0x2e
#define DW_OP_skip                      0x2f
#define DW_OP_lit0                      0x30
#define DW_OP_lit1                      0x31
#define DW_OP_lit2                      0x32
#define DW_OP_lit3                      0x33
#define DW_OP_lit4                      0x34
#define DW_OP_lit5                      0x35
#define DW_OP_lit6                      0x36
#define DW_OP_lit7                      0x37
#define DW_OP_lit8                      0x38
#define DW_OP_lit9                      0x39
#define DW_OP_lit10                     0x3a
#define DW_OP_lit11                     0x3b
#define DW_OP_lit12                     0x3c
#define DW_OP_lit13                     0x3d
#define DW_OP_lit14                     0x3e
#define DW_OP_lit15                     0x3f
#define DW_OP_lit16                     0x40
#define DW_OP_lit17                     0x41
#define DW_OP_lit18                     0x42
#define DW_OP_lit19                     0x43
#define DW_OP_lit20                     0x44
#define DW_OP_lit21                     0x45
#define DW_OP_lit22                     0x46
#define DW_OP_lit23                     0x47
#define DW_OP_lit24                     0x48
#define DW_OP_lit25                     0x49
#define DW_OP_lit26                     0x4a
#define DW_OP_lit27                     0x4b
#define DW_OP_lit28                     0x4c
#define DW_OP_lit29                     0x4d
#define DW_OP_lit30                     0x4e
#define DW_OP_lit31                     0x4f
#define DW_OP_reg0                      0x50
#define DW_OP_reg1                      0x51
#define DW_OP_reg2                      0x52
#define DW_OP_reg3                      0x53
#define DW_OP_reg4                      0x54
#define DW_OP_reg5                      0x55
#define DW_OP_reg6                      0x56
#define DW_OP_reg7                      0x57
#define DW_OP_reg8                      0x58
#define DW_OP_reg9                      0x59
#define DW_OP_reg10                     0x5a
#define DW_OP_reg11                     0x5b
#define DW_OP_reg12                     0x5c
#define DW_OP_reg13                     0x5d
#define DW_OP_reg14                     0x5e
#define DW_OP_reg15                     0x5f
#define DW_OP_reg16                     0x60
#define DW_OP_reg17                     0x61
#define DW_OP_reg18                     0x62
#define DW_OP_reg19                     0x63
#define DW_OP_reg20                     0x64
#define DW_OP_reg21                     0x65
#define DW_OP_reg22                     0x66
#define DW_OP_reg23                     0x67
#define DW_OP_reg24                     0x68
#define DW_OP_reg25                     0x69
#define DW_OP_reg26                     0x6a
#define DW_OP_reg27                     0x6b
#define DW_OP_reg28                     0x6c
#define DW_OP_reg29                     0x6d
#define DW_OP_reg30                     0x6e
#define DW_OP_reg31                     0x6f
#define DW_OP_breg0                     0x70
#define DW_OP_breg1                     0x71
#define DW_OP_breg2                     0x72
#define DW_OP_breg3                     0x73
#define DW_OP_breg4                     0x74
#define DW_OP_breg5                     0x75
#define DW_OP_breg6                     0x76
#define DW_OP_breg7                     0x77
#define DW_OP_breg8                     0x78
#define DW_OP_breg9                     0x79
#define DW_OP_breg10                    0x7a
#define DW_OP_breg11                    0x7b
#define DW_OP_breg12                    0x7c
#define DW_OP_breg13                    0x7d
#define DW_OP_breg14                    0x7e
#define DW_OP_breg15                    0x7f
#define DW_OP_breg16                    0x80
#define DW_OP_breg17                    0x81
#define DW_OP_breg18                    0x82
#define DW_OP_breg19                    0x83
#define DW_OP_breg20                    0x84
#define DW_OP_breg21                    0x85
#define DW_OP_breg22                    0x86
#define DW_OP_breg23                    0x87
#define DW_OP_breg24                    0x88
#define DW_OP_breg25                    0x89
#define DW_OP_breg26                    0x8a
#define DW_OP_breg27                    0x8b
#define DW_OP_breg28                    0x8c
#define DW_OP_breg29                    0x8d
#define DW_OP_breg30                    0x8e
#define DW_OP_breg31                    0x8f
#define DW_OP_regx                      0x90
#define DW_OP_fbreg                     0x91
#define DW_OP_bregx                     0x92
#define DW_OP_piece                     0x93
#define DW_OP_deref_size                0x94
#define DW_OP_xderef_size               0x95
#define DW_OP_nop                       0x96
#define DW_OP_push_object_address       0x97 /* DWARF3 */
#define DW_OP_call2                     0x98 /* DWARF3 */
#define DW_OP_call4                     0x99 /* DWARF3 */
#define DW_OP_call_ref                  0x9a /* DWARF3 */
#define DW_OP_form_tls_address          0x9b /* DWARF3f */
#define DW_OP_call_frame_cfa            0x9c /* DWARF3f */
#define DW_OP_bit_piece                 0x9d /* DWARF3f */
#define DW_OP_implicit_value            0x9e /* DWARF4 */
#define DW_OP_stack_value               0x9f /* DWARF4 */

    /* GNU extensions. */
#define DW_OP_GNU_push_tls_address      0xe0 /* GNU */

/* Follows extension so dwarfdump prints the most-likely-useful name. */
#define DW_OP_lo_user                   0xe0

    /* HP extensions. */
#define DW_OP_HP_unknown                0xe0 /* HP conflict: GNU */
#define DW_OP_HP_is_value               0xe1 /* HP */
#define DW_OP_HP_fltconst4              0xe2 /* HP */
#define DW_OP_HP_fltconst8              0xe3 /* HP */
#define DW_OP_HP_mod_range              0xe4 /* HP */
#define DW_OP_HP_unmod_range            0xe5 /* HP */
#define DW_OP_HP_tls                    0xe6 /* HP */

#define DW_OP_hi_user                   0xff

#define DW_ATE_address                  0x1
#define DW_ATE_boolean                  0x2
#define DW_ATE_complex_float            0x3
#define DW_ATE_float                    0x4
#define DW_ATE_signed                   0x5
#define DW_ATE_signed_char              0x6
#define DW_ATE_unsigned                 0x7
#define DW_ATE_unsigned_char            0x8
#define DW_ATE_imaginary_float          0x9  /* DWARF3 */
#define DW_ATE_packed_decimal           0xa  /* DWARF3f */
#define DW_ATE_numeric_string           0xb  /* DWARF3f */
#define DW_ATE_edited                   0xc  /* DWARF3f */
#define DW_ATE_signed_fixed             0xd  /* DWARF3f */
#define DW_ATE_unsigned_fixed           0xe  /* DWARF3f */
#define DW_ATE_decimal_float            0xf  /* DWARF3f */


/* ALTIUM extensions. x80, x81 */
#define DW_ATE_ALTIUM_fract           0x80 /* ALTIUM __fract type */

/* Follows extension so dwarfdump prints the most-likely-useful name. */
#define DW_ATE_lo_user                  0x80

/* Shown here to help dwarfdump build script. */
#define DW_ATE_ALTIUM_accum           0x81 /* ALTIUM __accum type */

/* HP Floating point extensions. */
#define DW_ATE_HP_float80             0x80 /* (80 bit). HP */


#define DW_ATE_HP_complex_float80     0x81 /* Complex (80 bit). HP  */
#define DW_ATE_HP_float128            0x82 /* (128 bit). HP */
#define DW_ATE_HP_complex_float128    0x83 /* Complex (128 bit). HP */
#define DW_ATE_HP_floathpintel        0x84 /* (82 bit IA64). HP */
#define DW_ATE_HP_imaginary_float80   0x85 /* HP */
#define DW_ATE_HP_imaginary_float128  0x86 /* HP */

#define DW_ATE_hi_user                  0xff


/* Decimal Sign codes. */
#define DW_DS_unsigned                  0x01 /* DWARF3f */
#define DW_DS_leading_overpunch         0x02 /* DWARF3f */
#define DW_DS_trailing_overpunch        0x03 /* DWARF3f */
#define DW_DS_leading_separate          0x04 /* DWARF3f */

#define DW_DS_trailing_separate         0x05 /* DWARF3f */

/* Endian code name. */
#define DW_END_default                  0x00 /* DWARF3f */
#define DW_END_big                      0x01 /* DWARF3f */
#define DW_END_little                   0x02 /* DWARF3f */

#define DW_END_lo_user                  0x40 /* DWARF3f */
#define DW_END_hi_user                  0xff /* DWARF3f */


/* Accessibility code name. */
#define DW_ACCESS_public                0x01
#define DW_ACCESS_protected             0x02
#define DW_ACCESS_private               0x03

/* Visibility code name. */
#define DW_VIS_local                    0x01
#define DW_VIS_exported                 0x02
#define DW_VIS_qualified                0x03

/* Virtuality code name. */
#define DW_VIRTUALITY_none              0x00
#define DW_VIRTUALITY_virtual           0x01
#define DW_VIRTUALITY_pure_virtual      0x02

#define DW_LANG_C89                     0x0001
#define DW_LANG_C                       0x0002
#define DW_LANG_Ada83                   0x0003
#define DW_LANG_C_plus_plus             0x0004
#define DW_LANG_C_plus_plus_03          0x0019 /* DWARF5 */
#define DW_LANG_C_plus_plus_11          0x001a /* DWARF5 */
#define DW_LANG_C_plus_plus_14          0x0021 /* DWARF5 */
#define DW_LANG_Cobol74                 0x0005
#define DW_LANG_Cobol85                 0x0006
#define DW_LANG_Fortran77               0x0007
#define DW_LANG_Fortran90               0x0008
#define DW_LANG_Pascal83                0x0009
#define DW_LANG_Modula2                 0x000a
#define DW_LANG_Java                    0x000b /* DWARF3 */
#define DW_LANG_C99                     0x000c /* DWARF3 */
#define DW_LANG_Ada95                   0x000d /* DWARF3 */
#define DW_LANG_Fortran95               0x000e /* DWARF3 */
#define DW_LANG_PLI                     0x000f /* DWARF3 */
#define DW_LANG_ObjC                    0x0010 /* DWARF3f */
#define DW_LANG_ObjC_plus_plus          0x0011 /* DWARF3f */
#define DW_LANG_UPC                     0x0012 /* DWARF3f */
#define DW_LANG_D                       0x0013 /* DWARF3f */
#define DW_LANG_Python                  0x0014 /* DWARF4 */
#define DW_LANG_lo_user                 0x8000
#define DW_LANG_Mips_Assembler          0x8001 /* MIPS   */
#define DW_LANG_Upc                     0x8765 /* UPC, use
                                        DW_LANG_UPC instead. */
/* ALTIUM extension */
#define DW_LANG_ALTIUM_Assembler        0x9101  /* ALTIUM */

#define DW_LANG_hi_user                 0xffff

/* Identifier case name. */
#define DW_ID_case_sensitive            0x00
#define DW_ID_up_case                   0x01
#define DW_ID_down_case                 0x02
#define DW_ID_case_insensitive          0x03

/* Calling Convention Name. */
#define DW_CC_normal                    0x01
#define DW_CC_program                   0x02
#define DW_CC_nocall                    0x03
#define DW_CC_lo_user                   0x40

/* ALTIUM extensions. */
/* Function is an interrupt handler, return address on system stack. */
#define DW_CC_ALTIUM_interrupt          0x65  /* ALTIUM*/

/* Near function model, return address on system stack. */
#define DW_CC_ALTIUM_near_system_stack  0x66  /*ALTIUM */

/* Near function model, return address on user stack. */
#define DW_CC_ALTIUM_near_user_stack    0x67  /* ALTIUM */  

/* Huge function model, return address on user stack.  */
#define DW_CC_ALTIUM_huge_user_stack    0x68  /* ALTIUM */    


#define DW_CC_hi_user                   0xff

/* Inline Code Name. */
#define DW_INL_not_inlined              0x00
#define DW_INL_inlined                  0x01
#define DW_INL_declared_not_inlined     0x02
#define DW_INL_declared_inlined         0x03

/* Ordering Name. */
#define DW_ORD_row_major                0x00
#define DW_ORD_col_major                0x01

/* Discriminant Descriptor Name. */
#define DW_DSC_label                    0x00
#define DW_DSC_range                    0x01

/* Line number standard opcode name. */
#define DW_LNS_copy                     0x01
#define DW_LNS_advance_pc               0x02
#define DW_LNS_advance_line             0x03
#define DW_LNS_set_file                 0x04
#define DW_LNS_set_column               0x05
#define DW_LNS_negate_stmt              0x06
#define DW_LNS_set_basic_block          0x07
#define DW_LNS_const_add_pc             0x08
#define DW_LNS_fixed_advance_pc         0x09
#define DW_LNS_set_prologue_end         0x0a /* DWARF3 */
#define DW_LNS_set_epilogue_begin       0x0b /* DWARF3 */
#define DW_LNS_set_isa                  0x0c /* DWARF3 */

/* Line number extended opcode name. */
#define DW_LNE_end_sequence             0x01
#define DW_LNE_set_address              0x02
#define DW_LNE_define_file              0x03
#define DW_LNE_set_discriminator        0x04  /* DWARF4 */

/* HP extensions. */
#define DW_LNE_HP_negate_is_UV_update       0x11 /* 17 HP */
#define DW_LNE_HP_push_context              0x12 /* 18 HP */
#define DW_LNE_HP_pop_context               0x13 /* 19 HP */
#define DW_LNE_HP_set_file_line_column      0x14 /* 20 HP */
#define DW_LNE_HP_set_routine_name          0x15 /* 21 HP */
#define DW_LNE_HP_set_sequence              0x16 /* 22 HP */
#define DW_LNE_HP_negate_post_semantics     0x17 /* 23 HP */
#define DW_LNE_HP_negate_function_exit      0x18 /* 24 HP */
#define DW_LNE_HP_negate_front_end_logical  0x19 /* 25 HP */
#define DW_LNE_HP_define_proc               0x20 /* 32 HP */

#define DW_LNE_lo_user                  0x80 /* DWARF3 */
#define DW_LNE_hi_user                  0xff /* DWARF3 */

/* Macro information. */
#define DW_MACINFO_define               0x01
#define DW_MACINFO_undef                0x02
#define DW_MACINFO_start_file           0x03
#define DW_MACINFO_end_file             0x04
#define DW_MACINFO_vendor_ext           0xff

#define DW_CFA_advance_loc        0x40
#define DW_CFA_offset             0x80
#define DW_CFA_restore            0xc0
#define DW_CFA_extended           0

#define DW_CFA_nop              0x00
#define DW_CFA_set_loc          0x01
#define DW_CFA_advance_loc1     0x02
#define DW_CFA_advance_loc2     0x03
#define DW_CFA_advance_loc4     0x04
#define DW_CFA_offset_extended  0x05
#define DW_CFA_restore_extended 0x06
#define DW_CFA_undefined        0x07
#define DW_CFA_same_value       0x08
#define DW_CFA_register         0x09
#define DW_CFA_remember_state   0x0a
#define DW_CFA_restore_state    0x0b
#define DW_CFA_def_cfa          0x0c
#define DW_CFA_def_cfa_register 0x0d
#define DW_CFA_def_cfa_offset   0x0e
#define DW_CFA_def_cfa_expression 0x0f     /* DWARF3 */
#define DW_CFA_expression       0x10       /* DWARF3 */
#define DW_CFA_cfa_offset_extended_sf 0x11 /* DWARF3 */
#define DW_CFA_def_cfa_sf       0x12       /* DWARF3 */
#define DW_CFA_def_cfa_offset_sf 0x13      /* DWARF3 */
#define DW_CFA_val_offset        0x14      /* DWARF3f */
#define DW_CFA_val_offset_sf     0x15      /* DWARF3f */
#define DW_CFA_val_expression    0x16      /* DWARF3f */

#define DW_CFA_lo_user           0x1c
#define DW_CFA_low_user          0x1c  /* Incorrect spelling, do not use. */

/* SGI/MIPS extension. */
#define DW_CFA_MIPS_advance_loc8 0x1d   /* MIPS */

/* GNU extensions. */
#define DW_CFA_GNU_window_save   0x2d  /* GNU */
#define DW_CFA_GNU_args_size     0x2e /* GNU  */
#define DW_CFA_GNU_negative_offset_extended  0x2f /* GNU */

#define DW_CFA_high_user         0x3f

/* GNU exception header encoding.  See the Generic
   Elf Specification of the Linux Standard Base (LSB). 
   http://refspecs.freestandards.org/LSB_3.0.0/LSB-Core-generic/LSB-Core-generic/dwarfext.html
   The upper 4 bits indicate how the value is to be applied. 
   The lower 4 bits indicate the format of the data.
*/
#define DW_EH_PE_absptr	  0x00  /* GNU */
#define DW_EH_PE_uleb128  0x01  /* GNU */
#define DW_EH_PE_udata2	  0x02  /* GNU */
#define DW_EH_PE_udata4	  0x03  /* GNU */
#define DW_EH_PE_udata8	  0x04  /* GNU */
#define DW_EH_PE_sleb128  0x09  /* GNU */
#define DW_EH_PE_sdata2   0x0A  /* GNU */
#define DW_EH_PE_sdata4	  0x0B  /* GNU */
#define DW_EH_PE_sdata8	  0x0C  /* GNU */

#define DW_EH_PE_pcrel	  0x10  /* GNU */
#define DW_EH_PE_textrel  0x20  /* GNU */
#define DW_EH_PE_datarel  0x30  /* GNU */
#define DW_EH_PE_funcrel  0x40  /* GNU */
#define DW_EH_PE_aligned  0x50  /* GNU */

#define DW_EH_PE_omit     0xff  /* GNU.  Means  no value present. */


/* Mapping from machine registers and pseudo-regs into the .debug_frame table.
   DW_FRAME entries are machine specific. These describe
   MIPS/SGI R3000, R4K, R4400.
   And (simultaneously) a mapping from hardware register number to
   the number used in the table to identify that register.

   The CFA (Canonical Frame Address) described in DWARF is called
   the Virtual Frame Pointer on MIPS/SGI machines.

                             Rule describes:
*/
/* Column used for CFA. Assumes reg 0 never appears as
   a register in DWARF info.  */
#define DW_FRAME_CFA_COL 0  

#define DW_FRAME_REG1   1  /* integer reg 1 */
#define DW_FRAME_REG2   2  /* integer reg 2 */
#define DW_FRAME_REG3   3  /* integer reg 3 */
#define DW_FRAME_REG4   4  /* integer reg 4 */
#define DW_FRAME_REG5   5  /* integer reg 5 */
#define DW_FRAME_REG6   6  /* integer reg 6 */
#define DW_FRAME_REG7   7  /* integer reg 7 */
#define DW_FRAME_REG8   8  /* integer reg 8 */
#define DW_FRAME_REG9   9  /* integer reg 9 */
#define DW_FRAME_REG10  10 /* integer reg 10 */
#define DW_FRAME_REG11  11 /* integer reg 11 */
#define DW_FRAME_REG12  12 /* integer reg 12 */
#define DW_FRAME_REG13  13 /* integer reg 13 */
#define DW_FRAME_REG14  14 /* integer reg 14 */
#define DW_FRAME_REG15  15 /* integer reg 15 */
#define DW_FRAME_REG16  16 /* integer reg 16 */
#define DW_FRAME_REG17  17 /* integer reg 17 */
#define DW_FRAME_REG18  18 /* integer reg 18 */
#define DW_FRAME_REG19  19 /* integer reg 19 */
#define DW_FRAME_REG20  20 /* integer reg 20 */
#define DW_FRAME_REG21  21 /* integer reg 21 */
#define DW_FRAME_REG22  22 /* integer reg 22 */
#define DW_FRAME_REG23  23 /* integer reg 23 */
#define DW_FRAME_REG24  24 /* integer reg 24 */
#define DW_FRAME_REG25  25 /* integer reg 25 */
#define DW_FRAME_REG26  26 /* integer reg 26 */
#define DW_FRAME_REG27  27 /* integer reg 27 */
#define DW_FRAME_REG28  28 /* integer reg 28 */
#define DW_FRAME_REG29  29 /* integer reg 29 */
#define DW_FRAME_REG30  30 /* integer reg 30 */
#define DW_FRAME_REG31  31 /* integer reg 31, aka ra */

        /* MIPS1, 2 have only some of these 64-bit registers.
        ** MIPS1  save/restore takes 2 instructions per 64-bit reg, and
        ** in that case, the register is considered stored after the second
        ** swc1.
        */
#define DW_FRAME_FREG0  32 /* 64-bit floating point reg 0 */
#define DW_FRAME_FREG1  33 /* 64-bit floating point reg 1 */
#define DW_FRAME_FREG2  34 /* 64-bit floating point reg 2 */
#define DW_FRAME_FREG3  35 /* 64-bit floating point reg 3 */
#define DW_FRAME_FREG4  36 /* 64-bit floating point reg 4 */
#define DW_FRAME_FREG5  37 /* 64-bit floating point reg 5 */
#define DW_FRAME_FREG6  38 /* 64-bit floating point reg 6 */
#define DW_FRAME_FREG7  39 /* 64-bit floating point reg 7 */
#define DW_FRAME_FREG8  40 /* 64-bit floating point reg 8 */
#define DW_FRAME_FREG9  41 /* 64-bit floating point reg 9 */
#define DW_FRAME_FREG10 42 /* 64-bit floating point reg 10 */
#define DW_FRAME_FREG11 43 /* 64-bit floating point reg 11 */
#define DW_FRAME_FREG12 44 /* 64-bit floating point reg 12 */
#define DW_FRAME_FREG13 45 /* 64-bit floating point reg 13 */
#define DW_FRAME_FREG14 46 /* 64-bit floating point reg 14 */
#define DW_FRAME_FREG15 47 /* 64-bit floating point reg 15 */
#define DW_FRAME_FREG16 48 /* 64-bit floating point reg 16 */
#define DW_FRAME_FREG17 49 /* 64-bit floating point reg 17 */
#define DW_FRAME_FREG18 50 /* 64-bit floating point reg 18 */
#define DW_FRAME_FREG19 51 /* 64-bit floating point reg 19 */
#define DW_FRAME_FREG20 52 /* 64-bit floating point reg 20 */
#define DW_FRAME_FREG21 53 /* 64-bit floating point reg 21 */
#define DW_FRAME_FREG22 54 /* 64-bit floating point reg 22 */
#define DW_FRAME_FREG23 55 /* 64-bit floating point reg 23 */
#define DW_FRAME_FREG24 56 /* 64-bit floating point reg 24 */
#define DW_FRAME_FREG25 57 /* 64-bit floating point reg 25 */
#define DW_FRAME_FREG26 58 /* 64-bit floating point reg 26 */
#define DW_FRAME_FREG27 59 /* 64-bit floating point reg 27 */
#define DW_FRAME_FREG28 60 /* 64-bit floating point reg 28 */
#define DW_FRAME_FREG29 61 /* 64-bit floating point reg 29 */
#define DW_FRAME_FREG30 62 /* 64-bit floating point reg 30 */
#define DW_FRAME_FREG31 63 /* 64-bit floating point reg 31 */

/*  ***IMPORTANT NOTE, TARGET DEPENDENCY ****
    The following 4 #defines are dependent on 
    the target cpu(s) that you apply libdwarf to.
    Ensure that DW_FRAME_UNDEFINED_VAL  and DW_FRAME_SAME_VAL
    do not conflict with the range [0-DW_FRAME_STATIC_LINK].
    The value 63 works for MIPS cpus at least up to the R16000.

    For a cpu with more than 63 real registers
    DW_FRAME_HIGHEST_NORMAL_REGISTER
    must be increased for things to work properly!
    Also ensure that DW_FRAME_UNDEFINED_VAL DW_FRAME_SAME_VAL
    are not in the range [0-DW_FRAME_STATIC_LINK]

    Having DW_FRAME_HIGHEST_NORMAL_REGISTER be higher than
    is strictly needed is safe.

*/

#ifndef DW_FRAME_HIGHEST_NORMAL_REGISTER
#define DW_FRAME_HIGHEST_NORMAL_REGISTER 63
#endif
/* This is the number of columns in the Frame Table. 
   This constant should
   be kept in sync with DW_REG_TABLE_SIZE defined in libdwarf.h 
   It must also be large enough to be beyond the highest 
   compiler-defined-register (meaning DW_FRAME_RA_COL DW_FRAME_STATIC_LINK
   in the MIPS/IRIX case */
#ifndef DW_FRAME_LAST_REG_NUM
#define DW_FRAME_LAST_REG_NUM   (DW_FRAME_HIGHEST_NORMAL_REGISTER + 3)
#endif


/* Column recording ra (return addrress from a function call). 
   This is common to many architectures, but as a 'simple register'
   is not necessarily adequate for all architectures.
   For MIPS/IRIX this register number is actually recorded on disk
   in the .debug_frame section.
   */
#define DW_FRAME_RA_COL  (DW_FRAME_HIGHEST_NORMAL_REGISTER + 1)

/* Column recording static link applicable to up-level      
   addressing, as in IRIX mp code, pascal, etc.
   This is common to many architectures but
   is not necessarily adequate for all architectures.
   For MIPS/IRIX this register number is actually recorded on disk
   in the .debug_frame section.
*/
#define DW_FRAME_STATIC_LINK (DW_FRAME_HIGHEST_NORMAL_REGISTER + 2)



/*
  DW_FRAME_UNDEFINED_VAL and  DW_FRAME_SAME_VAL  are
  never on disk, just generated by libdwarf. See libdwarf.h
  for their values.
*/



#define DW_CHILDREN_no               0x00
#define DW_CHILDREN_yes              0x01

#define DW_ADDR_none            0

#ifdef __cplusplus
}
#endif
#endif /* __DWARF_H */
