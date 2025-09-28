	.text
.Ltext0:
.LFB0:
	/* locview.c:1 */
.LM1:
	/* view -0 */
	/* locview.c:2 */
.LM2:
	/* view 1 */
.LVL0:
	/* DEBUG i => 0 */
	/* locview.c:3 */
.LM3:
	/* view 2 */
	/* DEBUG j => 0x1 */
	/* locview.c:4 */
.LM4:
	/* view 3 */
	/* DEBUG i => 0x2 */
	/* locview.c:5 */
.LM5:
	/* view 4 */
	/* DEBUG j => 0x3 */
	/* locview.c:6 */
.LM6:
	/* view 5 */
	/* DEBUG k => 0x4 */
	/* DEBUG l => 0x4 */
	/* locview.c:7 */
.LM7:
	/* view 6 */
	/* DEBUG k => 0x5 */
	/* DEBUG l => 0x5 */
	/* locview.c:8 */
.LM8:
	/* view 7 */
	/* DEBUG k => 0x6 */
	/* DEBUG l => 0x6 */
	/* locview.c:9 */
.LM9:
	/* view 8 */
	.byte	0
.LFE0:
.Letext0:

	.section	.debug_info
.Ldebug_info0:
.LIbase:
	.4byte	.LIend - .LIstart	/* Length of Compilation Unit Info */
.LIstart:
	.2byte	0x5	/* DWARF version number */
	.byte	0x1	/* DW_UT_compile */
	.byte	0x4	/* Pointer Size (in bytes) */
	.4byte	.Ldebug_abbrev0	/* Offset Into Abbrev. Section */
.LIcu:
	.uleb128 0x2	/* (DIE (cu) DW_TAG_compile_unit) */
	.ascii "hand-crafted based on GCC output\0"
	.byte	0x1d	/* DW_AT_language */
	.ascii "locview.c\0"
	.ascii "/tmp\0"
	.4byte	0	/* DW_AT_low_pc */
.LIsubf:
	.uleb128 0x3	/* (DIE (subf) DW_TAG_subprogram) */
	.ascii "f\0"	/* DW_AT_name */
	.byte	0x1	/* DW_AT_decl_file (locview.c) */
	.byte	0x1	/* DW_AT_decl_line */
	.4byte	.LIint-.LIbase	/* DW_AT_type */
	.4byte	.LFB0	/* DW_AT_low_pc */
	.4byte	1 /* .LFE0-.LFB0 */	/* DW_AT_high_pc */
	.uleb128 0x1	/* DW_AT_frame_base */
	.byte	0x9c	/* DW_OP_call_frame_cfa */
			/* DW_AT_call_all_calls */
	.4byte	.LIint - .LIbase	/* DW_AT_sibling */
.LIvari:
	.uleb128 0x1	/* (DIE (vari) DW_TAG_variable) */
	.ascii "i\0"	/* DW_AT_name */
			/* DW_AT_decl_file (1, locview.c) */
	.byte	0x2	/* DW_AT_decl_line */
	.4byte	.LIint - .LIbase	/* DW_AT_type */
	.4byte	.LLST0	/* DW_AT_location */
	.4byte	.LVUS0	/* DW_AT_GNU_locviews */
.LIvarj:
	.uleb128 0x1	/* (DIE (varj) DW_TAG_variable) */
	.ascii "j\0"	/* DW_AT_name */
			/* DW_AT_decl_file (1, locview.c) */
	.byte	0x3	/* DW_AT_decl_line */
	.4byte	.LIint - .LIbase	/* DW_AT_type */
	.4byte	.LLST1	/* DW_AT_location */
	.4byte	.LVUS1	/* DW_AT_GNU_locviews */
.LIvark:
	.uleb128 0x5	/* (DIE (vark) DW_TAG_variable) */
	.ascii "k\0"	/* DW_AT_name */
			/* DW_AT_decl_file (1, locview.c) */
	.byte	0x6	/* DW_AT_decl_line */
	.4byte	.LIint - .LIbase	/* DW_AT_type */
	.4byte	.LVUS2	/* DW_AT_GNU_locviews */
	.4byte	.LLST2	/* DW_AT_location */
.LIvarl:
	.uleb128 0x6	/* (DIE (varl) DW_TAG_variable) */
	.ascii "l\0"	/* DW_AT_name */
			/* DW_AT_decl_file (1, locview.c) */
	.byte	0x6	/* DW_AT_decl_line */
	.4byte	.LIint - .LIbase	/* DW_AT_type */
	.4byte	.LLST3	/* DW_AT_location */
	.byte	0	/* end of children of DIE subf */

.LIint:
	.uleb128 0x4	/* (DIE (int) DW_TAG_base_type) */
	.byte	0x4	/* DW_AT_byte_size */
	.byte	0x5	/* DW_AT_encoding */
	.ascii "int\0"	/* DW_AT_name */
	.byte	0	/* end of children of DIE cu */
.LIend:
	.section	.debug_abbrev
.Ldebug_abbrev0:
.LAbrv1:
	.uleb128 0x1	/* (abbrev code) */
	.uleb128 0x34	/* (TAG: DW_TAG_variable) */
	.byte	0	/* DW_children_no */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0x8	/* (DW_FORM_string) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0x21	/* (DW_FORM_implicit_const) */
	.sleb128 1	/* (locview.c) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x49	/* (DW_AT_type) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.uleb128 0x2	/* (DW_AT_location) */
	.uleb128 0x17	/* (DW_FORM_sec_offset) */
	.uleb128 0x2137	/* (DW_AT_GNU_locviews) */
	.uleb128 0x17	/* (DW_FORM_sec_offset) */
	.byte	0
	.byte	0
.LAbrv2:
	.uleb128 0x2	/* (abbrev code) */
	.uleb128 0x11	/* (TAG: DW_TAG_compile_unit) */
	.byte	0x1	/* DW_children_yes */
	.uleb128 0x25	/* (DW_AT_producer) */
	.uleb128 0x8	/* (DW_FORM_string) */
	.uleb128 0x13	/* (DW_AT_language) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0x8	/* (DW_FORM_string) */
	.uleb128 0x1b	/* (DW_AT_comp_dir) */
	.uleb128 0x8	/* (DW_FORM_string) */
	.uleb128 0x11	/* (DW_AT_low_pc) */
	.uleb128 0x1	/* (DW_FORM_addr) */
	.byte	0
	.byte	0
.LAbrv3:
	.uleb128 0x3	/* (abbrev code) */
	.uleb128 0x2e	/* (TAG: DW_TAG_subprogram) */
	.byte	0x1	/* DW_children_yes */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0x8	/* (DW_FORM_string) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x49	/* (DW_AT_type) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.uleb128 0x11	/* (DW_AT_low_pc) */
	.uleb128 0x1	/* (DW_FORM_addr) */
	.uleb128 0x12	/* (DW_AT_high_pc) */
	.uleb128 0x6	/* (DW_FORM_data4) */
	.uleb128 0x40	/* (DW_AT_frame_base) */
	.uleb128 0x18	/* (DW_FORM_exprloc) */
	.uleb128 0x7a	/* (DW_AT_call_all_calls) */
	.uleb128 0x19	/* (DW_FORM_flag_present) */
	.uleb128 0x1	/* (DW_AT_sibling) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.byte	0
	.byte	0
.LAbrv4:
	.uleb128 0x4	/* (abbrev code) */
	.uleb128 0x24	/* (TAG: DW_TAG_base_type) */
	.byte	0	/* DW_children_no */
	.uleb128 0xb	/* (DW_AT_byte_size) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3e	/* (DW_AT_encoding) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0x8	/* (DW_FORM_string) */
	.byte	0
	.byte	0
.LAbrv5:
	.uleb128 0x5	/* (abbrev code) */
	.uleb128 0x34	/* (TAG: DW_TAG_variable) */
	.byte	0	/* DW_children_no */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0x8	/* (DW_FORM_string) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0x21	/* (DW_FORM_implicit_const) */
	.sleb128 1	/* (locview.c) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x49	/* (DW_AT_type) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.uleb128 0x2137	/* (DW_AT_GNU_locviews) */
	.uleb128 0x17	/* (DW_FORM_sec_offset) */
	.uleb128 0x2	/* (DW_AT_location) */
	.uleb128 0x17	/* (DW_FORM_sec_offset) */
	.byte	0
	.byte	0
.LAbrv6:
	.uleb128 0x6	/* (abbrev code) */
	.uleb128 0x34	/* (TAG: DW_TAG_variable) */
	.byte	0	/* DW_children_no */
	.uleb128 0x3	/* (DW_AT_name) */
	.uleb128 0x8	/* (DW_FORM_string) */
	.uleb128 0x3a	/* (DW_AT_decl_file) */
	.uleb128 0x21	/* (DW_FORM_implicit_const) */
	.sleb128 1	/* (locview.c) */
	.uleb128 0x3b	/* (DW_AT_decl_line) */
	.uleb128 0xb	/* (DW_FORM_data1) */
	.uleb128 0x49	/* (DW_AT_type) */
	.uleb128 0x13	/* (DW_FORM_ref4) */
	.uleb128 0x2	/* (DW_AT_location) */
	.uleb128 0x17	/* (DW_FORM_sec_offset) */
	.byte	0
	.byte	0
	.byte	0

	.section	.debug_loclists
	.4byte	.Ldebug_loc2-.Ldebug_loc1	/* Length of Location Lists */
.Ldebug_loc1:
	.2byte	0x5	/* DWARF version number */
	.byte	0x4	/* Address Size */
	.byte	0	/* Segment Size */
	.4byte	0	/* Offset Entry Count */
.Ldebug_loc0:
.LVUS0:
	.uleb128 0x2	/* View list begin (*.LVUS0) */
	.uleb128 0x4	/* View list end (*.LVUS0) */
	.uleb128 0x4	/* View list begin (*.LVUS0) */
	.uleb128 0	/* View list end (*.LVUS0) */
.LLST0:
	.byte	0x6	/* DW_LLE_base_address (*.LLST0) */
	.4byte	.LVL0	/* Base address (*.LLST0) */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST0) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list begin address (*.LLST0) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list end address (*.LLST0) */
	.uleb128 0x2	/* Location expression size */
	.byte	0x30	/* DW_OP_lit0 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST0) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list begin address (*.LLST0) */
	.uleb128 1 /* .LFE0-.LVL0 */	/* Location list end address (*.LLST0) */
	.uleb128 0x2	/* Location expression size */
	.byte	0x32	/* DW_OP_lit2 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0	/* DW_LLE_end_of_list (*.LLST0) */
.LLST1:
	.byte	0x6	/* DW_LLE_base_address (*.LLST1) */
	.4byte	.LVL0	/* Base address (*.LLST1) */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST1) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list begin address (*.LLST1) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list end address (*.LLST1) */
	.uleb128 0x2	/* Location expression size */
	.byte	0x31	/* DW_OP_lit1 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST1) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list begin address (*.LLST1) */
	.uleb128 1 /* .LFE0-.LVL0 */	/* Location list end address (*.LLST1) */
	.uleb128 0x2	/* Location expression size */
	.byte	0x33	/* DW_OP_lit3 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0	/* DW_LLE_end_of_list (*.LLST1) */
.LVUS1:
	.uleb128 0x3	/* View list begin (*.LVUS1) */
	.uleb128 0x5	/* View list end (*.LVUS1) */
	.uleb128 0x5	/* View list begin (*.LVUS1) */
	.uleb128 0	/* View list end (*.LVUS1) */
.LVUS2:
	.uleb128 0x6	/* View list begin (*.LVUS2) */
	.uleb128 0x7	/* View list end (*.LVUS2) */
	.uleb128 0x7	/* View list begin (*.LVUS2) */
	.uleb128 0x8	/* View list end (*.LVUS2) */
	.uleb128 0x8	/* View list begin (*.LVUS2) */
	.uleb128 0	/* View list end (*.LVUS2) */
.LLST2:
	.byte	0x6	/* DW_LLE_base_address (*.LLST2) */
	.4byte	.LVL0	/* Base address (*.LLST2) */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST2) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list begin address (*.LLST2) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list end address (*.LLST2) */
	.uleb128 0x2	/* Location expression size */
	.byte	0x34	/* DW_OP_lit4 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST2) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list begin address (*.LLST2) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list end address (*.LLST2) */
	.uleb128 0x2	/* Location expression size */
	.byte	0x35	/* DW_OP_lit5 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST2) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list begin address (*.LLST2) */
	.uleb128 1 /* .LFE0-.LVL0 */	/* Location list end address (*.LLST2) */
	.uleb128 0x2	/* Location expression size */
	.byte	0x36	/* DW_OP_lit6 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0	/* DW_LLE_end_of_list (*.LLST2) */
.LLST3:
	.byte	0x6	/* DW_LLE_base_address (*.LLST3) */
	.4byte	.LVL0	/* Base address (*.LLST3) */
	.byte	0x9	/* DW_LLE_view_pair (extension proposed for DWARF6) */
	.uleb128 0x6	/* View list begin (*.LLST3) */
	.uleb128 0x7	/* View list end (*.LVUS3) */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST3) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list begin address (*.LLST3) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list end address (*.LLST3) */
	.uleb128 0x2	/* Location expression size */
	.byte	0x34	/* DW_OP_lit4 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0x9	/* DW_LLE_view_pair */
	.uleb128 0x7	/* View list begin (*.LLST3) */
	.uleb128 0x8	/* View list end (*.LVUS3) */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST3) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list begin address (*.LLST3) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list end address (*.LLST3) */
	.uleb128 0x2	/* Location expression size */
	.byte	0x35	/* DW_OP_lit5 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0x9	/* DW_LLE_view_pair */
	.uleb128 0x8	/* View list begin (*.LLST3) */
	.uleb128 0x0	/* View list end (*.LVUS3) */
	.byte	0x4	/* DW_LLE_offset_pair (*.LLST3) */
	.uleb128 0 /* .LVL0-.LVL0 */	/* Location list begin address (*.LLST3) */
	.uleb128 1 /* .LFE0-.LVL0 */	/* Location list end address (*.LLST3) */
	.uleb128 0x2	/* Location expression size */
	.byte	0x36	/* DW_OP_lit6 */
	.byte	0x9f	/* DW_OP_stack_value */
	.byte	0	/* DW_LLE_end_of_list (*.LLST3) */
.Ldebug_loc2:
