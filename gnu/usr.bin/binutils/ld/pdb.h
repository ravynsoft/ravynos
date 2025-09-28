/* pdb.h - header file for generating PDB CodeView debugging files.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* Header files referred to below can be found in Microsoft's PDB
   repository: https://github.com/microsoft/microsoft-pdb.  */

#ifndef PDB_H
#define PDB_H

#include "sysdep.h"
#include "bfd.h"
#include <stdbool.h>
#include <stddef.h>

#define LF_VTSHAPE			0x000a
#define LF_MODIFIER			0x1001
#define LF_POINTER			0x1002
#define LF_PROCEDURE			0x1008
#define LF_MFUNCTION			0x1009
#define LF_ARGLIST			0x1201
#define LF_FIELDLIST			0x1203
#define LF_BITFIELD			0x1205
#define LF_METHODLIST			0x1206
#define LF_BCLASS			0x1400
#define LF_VBCLASS			0x1401
#define LF_IVBCLASS			0x1402
#define LF_INDEX			0x1404
#define LF_VFUNCTAB			0x1409
#define LF_ENUMERATE			0x1502
#define LF_ARRAY			0x1503
#define LF_CLASS			0x1504
#define LF_STRUCTURE			0x1505
#define LF_UNION			0x1506
#define LF_ENUM				0x1507
#define LF_MEMBER			0x150d
#define LF_STMEMBER			0x150e
#define LF_METHOD			0x150f
#define LF_NESTTYPE			0x1510
#define LF_ONEMETHOD			0x1511
#define LF_VFTABLE			0x151d
#define LF_FUNC_ID			0x1601
#define LF_MFUNC_ID			0x1602
#define LF_BUILDINFO			0x1603
#define LF_SUBSTR_LIST			0x1604
#define LF_STRING_ID			0x1605
#define LF_UDT_SRC_LINE			0x1606
#define LF_UDT_MOD_SRC_LINE		0x1607

#define LF_CHAR				0x8000
#define LF_SHORT			0x8001
#define LF_USHORT			0x8002
#define LF_LONG				0x8003
#define LF_ULONG			0x8004
#define LF_QUADWORD			0x8009
#define LF_UQUADWORD			0x800a

#define S_END				0x0006
#define S_FRAMEPROC			0x1012
#define S_OBJNAME			0x1101
#define S_THUNK32			0x1102
#define S_BLOCK32			0x1103
#define S_LABEL32			0x1105
#define S_REGISTER			0x1106
#define S_CONSTANT			0x1107
#define S_UDT				0x1108
#define S_BPREL32			0x110b
#define S_LDATA32			0x110c
#define S_GDATA32 			0x110d
#define S_PUB32				0x110e
#define S_LPROC32			0x110f
#define S_GPROC32			0x1110
#define S_REGREL32			0x1111
#define S_LTHREAD32			0x1112
#define S_GTHREAD32			0x1113
#define S_UNAMESPACE			0x1124
#define S_PROCREF			0x1125
#define S_LPROCREF			0x1127
#define S_FRAMECOOKIE			0x113a
#define S_COMPILE3			0x113c
#define S_ENVBLOCK			0x113d
#define S_LOCAL				0x113e
#define S_DEFRANGE_REGISTER		0x1141
#define S_DEFRANGE_FRAMEPOINTER_REL	0x1142
#define S_DEFRANGE_SUBFIELD_REGISTER	0x1143
#define S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE	0x1144
#define S_DEFRANGE_REGISTER_REL		0x1145
#define S_LPROC32_ID			0x1146
#define S_GPROC32_ID			0x1147
#define S_BUILDINFO			0x114c
#define S_INLINESITE			0x114d
#define S_INLINESITE_END		0x114e
#define S_PROC_ID_END			0x114f
#define S_HEAPALLOCSITE			0x115e

/* PDBStream70 in pdb1.h */
struct pdb_stream_70
{
  uint32_t version;
  uint32_t signature;
  uint32_t age;
  uint8_t guid[16];
};

#define PDB_STREAM_VERSION_VC70		20000404
#define PDB_STREAM_VERSION_VC140	20140508

/* HDR in tpi.h */
struct pdb_tpi_stream_header
{
  uint32_t version;
  uint32_t header_size;
  uint32_t type_index_begin;
  uint32_t type_index_end;
  uint32_t type_record_bytes;
  uint16_t hash_stream_index;
  uint16_t hash_aux_stream_index;
  uint32_t hash_key_size;
  uint32_t num_hash_buckets;
  uint32_t hash_value_buffer_offset;
  uint32_t hash_value_buffer_length;
  uint32_t index_offset_buffer_offset;
  uint32_t index_offset_buffer_length;
  uint32_t hash_adj_buffer_offset;
  uint32_t hash_adj_buffer_length;
};

#define TPI_STREAM_VERSION_80		20040203

#define TPI_FIRST_INDEX			0x1000
#define NUM_TPI_HASH_BUCKETS		0x3ffff

#define NUM_GLOBALS_HASH_BUCKETS	4096

/* NewDBIHdr in dbi.h */
struct pdb_dbi_stream_header
{
  uint32_t version_signature;
  uint32_t version_header;
  uint32_t age;
  uint16_t global_stream_index;
  uint16_t build_number;
  uint16_t public_stream_index;
  uint16_t pdb_dll_version;
  uint16_t sym_record_stream;
  uint16_t pdb_dll_rbld;
  uint32_t mod_info_size;
  uint32_t section_contribution_size;
  uint32_t section_map_size;
  uint32_t source_info_size;
  uint32_t type_server_map_size;
  uint32_t mfc_type_server_index;
  uint32_t optional_dbg_header_size;
  uint32_t ec_substream_size;
  uint16_t flags;
  uint16_t machine;
  uint32_t padding;
};

#define DBI_STREAM_VERSION_70		19990903

/* PSGSIHDR in gsi.h */
struct publics_header
{
  uint32_t sym_hash_size;
  uint32_t addr_map_size;
  uint32_t num_thunks;
  uint32_t thunks_size;
  uint32_t thunk_table;
  uint32_t thunk_table_offset;
  uint32_t num_sects;
};

/* GSIHashHdr in gsi.h */
struct globals_hash_header
{
  uint32_t signature;
  uint32_t version;
  uint32_t entries_size;
  uint32_t buckets_size;
};

/* HRFile in gsi.h */
struct hash_record
{
  uint32_t offset;
  uint32_t reference;
};

#define GLOBALS_HASH_SIGNATURE		0xffffffff
#define GLOBALS_HASH_VERSION_70		0xf12f091a

/* PUBSYM32 in cvinfo.h */
struct pubsym
{
  uint16_t record_length;
  uint16_t record_type;
  uint32_t flags;
  uint32_t offset;
  uint16_t section;
  /* followed by null-terminated string */
} ATTRIBUTE_PACKED;

/* see bitset CV_PUBSYMFLAGS in cvinfo.h */
#define PUBSYM_FUNCTION			0x2

struct optional_dbg_header
{
  uint16_t fpo_stream;
  uint16_t exception_stream;
  uint16_t fixup_stream;
  uint16_t omap_to_src_stream;
  uint16_t omap_from_src_stream;
  uint16_t section_header_stream;
  uint16_t token_map_stream;
  uint16_t xdata_stream;
  uint16_t pdata_stream;
  uint16_t new_fpo_stream;
  uint16_t orig_section_header_stream;
};

#define CV_SIGNATURE_C13		4

#define DEBUG_S_SYMBOLS			0xf1
#define DEBUG_S_LINES			0xf2
#define DEBUG_S_STRINGTABLE		0xf3
#define DEBUG_S_FILECHKSMS		0xf4

#define STRING_TABLE_SIGNATURE		0xeffeeffe
#define STRING_TABLE_VERSION		1

/* VHdr in nmt.h */
struct string_table_header
{
  uint32_t signature;
  uint32_t version;
};

#define SECTION_CONTRIB_VERSION_60	0xf12eba2d

/* SC in dbicommon.h */
struct section_contribution
{
  uint16_t section;
  uint16_t padding1;
  uint32_t offset;
  uint32_t size;
  uint32_t characteristics;
  uint16_t module_index;
  uint16_t padding2;
  uint32_t data_crc;
  uint32_t reloc_crc;
};

/* MODI_60_Persist in dbi.h */
struct module_info
{
  uint32_t unused1;
  struct section_contribution sc;
  uint16_t flags;
  uint16_t module_sym_stream;
  uint32_t sym_byte_size;
  uint32_t c11_byte_size;
  uint32_t c13_byte_size;
  uint16_t source_file_count;
  uint16_t padding;
  uint32_t unused2;
  uint32_t source_file_name_index;
  uint32_t pdb_file_path_name_index;
};

/* filedata in dumpsym7.cpp */
struct file_checksum
{
  uint32_t file_id;
  uint8_t checksum_length;
  uint8_t checksum_type;
} ATTRIBUTE_PACKED;

/* lfModifier in cvinfo.h */
struct lf_modifier
{
  uint16_t size;
  uint16_t kind;
  uint32_t base_type;
  uint16_t modifier;
  uint16_t padding;
} ATTRIBUTE_PACKED;

/* lfPointer in cvinfo.h */
struct lf_pointer
{
  uint16_t size;
  uint16_t kind;
  uint32_t base_type;
  uint32_t attributes;
} ATTRIBUTE_PACKED;

/* lfArgList in cvinfo.h (used for both LF_ARGLIST and LF_SUBSTR_LIST) */
struct lf_arglist
{
  uint16_t size;
  uint16_t kind;
  uint32_t num_entries;
  uint32_t args[];
} ATTRIBUTE_PACKED;

/* lfProc in cvinfo.h */
struct lf_procedure
{
  uint16_t size;
  uint16_t kind;
  uint32_t return_type;
  uint8_t calling_convention;
  uint8_t attributes;
  uint16_t num_parameters;
  uint32_t arglist;
} ATTRIBUTE_PACKED;

/* lfMFunc in cvinfo.h */
struct lf_mfunction
{
  uint16_t size;
  uint16_t kind;
  uint32_t return_type;
  uint32_t containing_class_type;
  uint32_t this_type;
  uint8_t calling_convention;
  uint8_t attributes;
  uint16_t num_parameters;
  uint32_t arglist;
  int32_t this_adjustment;
} ATTRIBUTE_PACKED;

/* lfArray in cvinfo.h */
struct lf_array
{
  uint16_t size;
  uint16_t kind;
  uint32_t element_type;
  uint32_t index_type;
  uint16_t length_in_bytes;
  char name[];
} ATTRIBUTE_PACKED;

/* lfBitfield in cvinfo.h */
struct lf_bitfield
{
  uint16_t size;
  uint16_t kind;
  uint32_t base_type;
  uint8_t length;
  uint8_t position;
} ATTRIBUTE_PACKED;

/* lfMember in cvinfo.h */
struct lf_member
{
  uint16_t kind;
  uint16_t attributes;
  uint32_t type;
  uint16_t offset;
  char name[];
} ATTRIBUTE_PACKED;

/* from bitfield structure CV_prop_t in cvinfo.h */
#define CV_PROP_FORWARD_REF	0x80
#define CV_PROP_SCOPED		0x100
#define CV_PROP_HAS_UNIQUE_NAME	0x200

/* lfClass in cvinfo.h */
struct lf_class
{
  uint16_t size;
  uint16_t kind;
  uint16_t num_members;
  uint16_t properties;
  uint32_t field_list;
  uint32_t derived_from;
  uint32_t vshape;
  uint16_t length;
  char name[];
} ATTRIBUTE_PACKED;

/* lfUnion in cvinfo.h */
struct lf_union
{
  uint16_t size;
  uint16_t kind;
  uint16_t num_members;
  uint16_t properties;
  uint32_t field_list;
  uint16_t length;
  char name[];
} ATTRIBUTE_PACKED;

/* lfEnumerate in cvinfo.h */
struct lf_enumerate
{
  uint16_t kind;
  uint16_t attributes;
  uint16_t value;
  /* then actual value if value >= 0x8000 */
  char name[];
} ATTRIBUTE_PACKED;

/* lfEnum in cvinfo.h */
struct lf_enum
{
  uint16_t size;
  uint16_t kind;
  uint16_t num_elements;
  uint16_t properties;
  uint32_t underlying_type;
  uint32_t field_list;
  char name[];
} ATTRIBUTE_PACKED;

/* lfIndex in cvinfo.h */
struct lf_index
{
  uint16_t kind;
  uint16_t padding;
  uint32_t index;
} ATTRIBUTE_PACKED;

/* lfOneMethod in cvinfo.h */
struct lf_onemethod
{
  uint16_t kind;
  uint16_t method_attribute;
  uint32_t method_type;
  char name[];
} ATTRIBUTE_PACKED;

/* mlMethod in cvinfo.h */
struct lf_methodlist_entry
{
  uint16_t method_attribute;
  uint16_t padding;
  uint32_t method_type;
} ATTRIBUTE_PACKED;

/* lfMethodList in cvinfo.h */
struct lf_methodlist
{
  uint16_t size;
  uint16_t kind;
  struct lf_methodlist_entry entries[];
} ATTRIBUTE_PACKED;

/* lfMethod in cvinfo.h */
struct lf_method
{
  uint16_t kind;
  uint16_t count;
  uint32_t method_list;
  char name[];
} ATTRIBUTE_PACKED;

/* lfBClass in cvinfo.h */
struct lf_bclass
{
  uint16_t kind;
  uint16_t attributes;
  uint32_t base_class_type;
  uint16_t offset;
} ATTRIBUTE_PACKED;

/* lfVFuncTab in cvinfo.h */
struct lf_vfunctab
{
  uint16_t kind;
  uint16_t padding;
  uint32_t type;
} ATTRIBUTE_PACKED;

/* lfVBClass in cvinfo.h */
struct lf_vbclass
{
  uint16_t kind;
  uint16_t attributes;
  uint32_t base_class_type;
  uint32_t virtual_base_pointer_type;
  uint16_t virtual_base_pointer_offset;
  uint16_t virtual_base_vbtable_offset;
} ATTRIBUTE_PACKED;

/* lfSTMember in cvinfo.h */
struct lf_static_member
{
  uint16_t kind;
  uint16_t attributes;
  uint32_t type;
  char name[];
} ATTRIBUTE_PACKED;

/* lfNestType in cvinfo.h */
struct lf_nest_type
{
  uint16_t kind;
  uint16_t padding;
  uint32_t type;
  char name[];
} ATTRIBUTE_PACKED;

/* lfStringId in cvinfo.h */
struct lf_string_id
{
  uint16_t size;
  uint16_t kind;
  uint32_t substring;
  char string[];
} ATTRIBUTE_PACKED;

/* lfBuildInfo in cvinfo.h */
struct lf_build_info
{
  uint16_t size;
  uint16_t kind;
  uint16_t count;
  uint32_t strings[];
} ATTRIBUTE_PACKED;

/* lfFuncId in cvinfo.h */
struct lf_func_id
{
  uint16_t size;
  uint16_t kind;
  uint32_t parent_scope;
  uint32_t function_type;
  char name[];
} ATTRIBUTE_PACKED;

/* lfMFuncId in cvinfo.h */
struct lf_mfunc_id
{
  uint16_t size;
  uint16_t kind;
  uint32_t parent_type;
  uint32_t function_type;
  char name[];
} ATTRIBUTE_PACKED;

/* lfUdtSrcLine in cvinfo.h */
struct lf_udt_src_line
{
  uint16_t size;
  uint16_t kind;
  uint32_t type;
  uint32_t source_file_type;
  uint32_t line_no;
} ATTRIBUTE_PACKED;

/* lfUdtModSrcLine in cvinfo.h */
struct lf_udt_mod_src_line
{
  uint16_t size;
  uint16_t kind;
  uint32_t type;
  uint32_t source_file_string;
  uint32_t line_no;
  uint16_t module_no;
} ATTRIBUTE_PACKED;

/* lfVftable in cvinfo.h */
struct lf_vftable
{
  uint16_t size;
  uint16_t kind;
  uint32_t type;
  uint32_t base_vftable;
  uint32_t offset;
  uint32_t names_len;
  char names[];
} ATTRIBUTE_PACKED;

/* DATASYM32 in cvinfo.h */
struct datasym
{
  uint16_t size;
  uint16_t kind;
  uint32_t type;
  uint32_t offset;
  uint16_t section;
  char name[];
} ATTRIBUTE_PACKED;

/* PROCSYM32 in cvinfo.h */
struct procsym
{
  uint16_t size;
  uint16_t kind;
  uint32_t parent;
  uint32_t end;
  uint32_t next;
  uint32_t proc_len;
  uint32_t debug_start;
  uint32_t debug_end;
  uint32_t type;
  uint32_t offset;
  uint16_t section;
  uint8_t flags;
  char name[];
} ATTRIBUTE_PACKED;

/* REFSYM2 in cvinfo.h */
struct refsym
{
  uint16_t size;
  uint16_t kind;
  uint32_t sum_name;
  uint32_t symbol_offset;
  uint16_t mod;
  char name[];
} ATTRIBUTE_PACKED;

/* UDTSYM in cvinfo.h */
struct udtsym
{
  uint16_t size;
  uint16_t kind;
  uint32_t type;
  char name[];
} ATTRIBUTE_PACKED;

/* CONSTSYM in cvinfo.h */
struct constsym
{
  uint16_t size;
  uint16_t kind;
  uint32_t type;
  uint16_t value;
  /* then actual value if value >= 0x8000 */
  char name[];
} ATTRIBUTE_PACKED;

/* BUILDINFOSYM in cvinfo.h */
struct buildinfosym
{
  uint16_t size;
  uint16_t kind;
  uint32_t type;
} ATTRIBUTE_PACKED;

/* BLOCKSYM32 in cvinfo.h */
struct blocksym
{
  uint16_t size;
  uint16_t kind;
  uint32_t parent;
  uint32_t end;
  uint32_t len;
  uint32_t offset;
  uint16_t section;
  char name[];
} ATTRIBUTE_PACKED;

/* BPRELSYM32 in cvinfo.h */
struct bprelsym
{
  uint16_t size;
  uint16_t kind;
  uint32_t bp_offset;
  uint32_t type;
  char name[];
} ATTRIBUTE_PACKED;

/* REGSYM in cvinfo.h */
struct regsym
{
  uint16_t size;
  uint16_t kind;
  uint32_t type;
  uint16_t reg;
  char name[];
} ATTRIBUTE_PACKED;

/* REGREL32 in cvinfo.h */
struct regrel
{
  uint16_t size;
  uint16_t kind;
  uint32_t offset;
  uint32_t type;
  uint16_t reg;
  char name[];
} ATTRIBUTE_PACKED;

/* LOCALSYM in cvinfo.h */
struct localsym
{
  uint16_t size;
  uint16_t kind;
  uint32_t type;
  uint16_t flags;
  char name[];
} ATTRIBUTE_PACKED;

/* CV_LVAR_ADDR_RANGE in cvinfo.h */
struct lvar_addr_range
{
  uint32_t offset;
  uint16_t section;
  uint16_t length;
} ATTRIBUTE_PACKED;

/* CV_LVAR_ADDR_GAP in cvinfo.h */
struct lvar_addr_gap {
  uint16_t offset;
  uint16_t length;
} ATTRIBUTE_PACKED;

/* DEFRANGESYMREGISTERREL in cvinfo.h */
struct defrange_register_rel
{
  uint16_t size;
  uint16_t kind;
  uint16_t reg;
  uint16_t offset_parent;
  uint32_t offset_register;
  struct lvar_addr_range range;
  struct lvar_addr_gap gaps[];
} ATTRIBUTE_PACKED;

/* DEFRANGESYMFRAMEPOINTERREL in cvinfo.h */
struct defrange_framepointer_rel
{
  uint16_t size;
  uint16_t kind;
  uint32_t offset;
  struct lvar_addr_range range;
  struct lvar_addr_gap gaps[];
} ATTRIBUTE_PACKED;

/* DEFRANGESYMSUBFIELDREGISTER in cvinfo.h */
struct defrange_subfield_register
{
  uint16_t size;
  uint16_t kind;
  uint16_t reg;
  uint16_t attributes;
  uint32_t offset_parent;
  struct lvar_addr_range range;
  struct lvar_addr_gap gaps[];
} ATTRIBUTE_PACKED;

/* DEFRANGESYMREGISTER in cvinfo.h */
struct defrange_register
{
  uint16_t size;
  uint16_t kind;
  uint16_t reg;
  uint16_t attributes;
  struct lvar_addr_range range;
  struct lvar_addr_gap gaps[];
} ATTRIBUTE_PACKED;

/* INLINESITESYM in cvinfo.h */
struct inline_site
{
  uint16_t size;
  uint16_t kind;
  uint32_t parent;
  uint32_t end;
  uint32_t inlinee;
  uint8_t binary_annotations[];
} ATTRIBUTE_PACKED;

/* THUNKSYM32 in cvinfo.h */
struct thunk
{
  uint16_t size;
  uint16_t kind;
  uint32_t parent;
  uint32_t end;
  uint32_t next;
  uint32_t offset;
  uint16_t section;
  uint16_t length;
  uint8_t thunk_type;
  char name[];
} ATTRIBUTE_PACKED;

/* HEAPALLOCSITE in cvinfo.h */
struct heap_alloc_site
{
  uint16_t size;
  uint16_t kind;
  uint32_t offset;
  uint16_t section;
  uint16_t length;
  uint32_t type;
} ATTRIBUTE_PACKED;

/* OBJNAMESYM in cvinfo.h */
struct objname
{
  uint16_t size;
  uint16_t kind;
  uint32_t signature;
  char name[];
} ATTRIBUTE_PACKED;

#define CV_CFL_80386			0x03
#define CV_CFL_X64			0xD0
#define CV_CFL_ARM64			0xF6

#define CV_CFL_LINK			0x07

/* COMPILESYM3 in cvinfo.h */
struct compile3
{
  uint16_t size;
  uint16_t kind;
  uint32_t flags;
  uint16_t machine;
  uint16_t frontend_major;
  uint16_t frontend_minor;
  uint16_t frontend_build;
  uint16_t frontend_qfe;
  uint16_t backend_major;
  uint16_t backend_minor;
  uint16_t backend_build;
  uint16_t backend_qfe;
  char compiler[];
} ATTRIBUTE_PACKED;

/* ENVBLOCKSYM in cvinfo.h */
struct envblock
{
  uint16_t size;
  uint16_t kind;
  uint8_t flags;
  char strings[];
} ATTRIBUTE_PACKED;

extern bool create_pdb_file (bfd *, const char *, const unsigned char *);

#endif
