/* Mach-O support for BFD.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

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

#ifndef _MACH_O_LOADER_H
#define _MACH_O_LOADER_H

/* Constants for header. */

typedef enum bfd_mach_o_mach_header_magic
{
  BFD_MACH_O_MH_MAGIC    = 0xfeedface,
  BFD_MACH_O_MH_CIGAM    = 0xcefaedfe,
  BFD_MACH_O_MH_MAGIC_64 = 0xfeedfacf,
  BFD_MACH_O_MH_CIGAM_64 = 0xcffaedfe
}
bfd_mach_o_mach_header_magic;

/* Capability bits in cpu type.  */
#define BFD_MACH_O_CPU_ARCH_MASK  0xff000000
#define BFD_MACH_O_CPU_ARCH_ABI64 0x01000000
#define BFD_MACH_O_CPU_IS64BIT    0x01000000

typedef enum bfd_mach_o_cpu_type
{
  BFD_MACH_O_CPU_TYPE_VAX = 1,
  BFD_MACH_O_CPU_TYPE_MC680x0 = 6,
  BFD_MACH_O_CPU_TYPE_I386 = 7,
  BFD_MACH_O_CPU_TYPE_MIPS = 8,
  BFD_MACH_O_CPU_TYPE_MC98000 = 10,
  BFD_MACH_O_CPU_TYPE_HPPA = 11,
  BFD_MACH_O_CPU_TYPE_ARM = 12,
  BFD_MACH_O_CPU_TYPE_MC88000 = 13,
  BFD_MACH_O_CPU_TYPE_SPARC = 14,
  BFD_MACH_O_CPU_TYPE_I860 = 15,
  BFD_MACH_O_CPU_TYPE_ALPHA = 16,
  BFD_MACH_O_CPU_TYPE_POWERPC = 18,
  BFD_MACH_O_CPU_TYPE_POWERPC_64 =
    (BFD_MACH_O_CPU_TYPE_POWERPC | BFD_MACH_O_CPU_IS64BIT),
  BFD_MACH_O_CPU_TYPE_X86_64 =
    (BFD_MACH_O_CPU_TYPE_I386 | BFD_MACH_O_CPU_IS64BIT),
  BFD_MACH_O_CPU_TYPE_ARM64 =
    (BFD_MACH_O_CPU_TYPE_ARM | BFD_MACH_O_CPU_IS64BIT)
}
bfd_mach_o_cpu_type;

/* Capability bits in cpu subtype.  */
#define BFD_MACH_O_CPU_SUBTYPE_MASK  0xff000000
#define BFD_MACH_O_CPU_SUBTYPE_LIB64 0x80000000

typedef enum bfd_mach_o_cpu_subtype
{
  /* i386.  */
  BFD_MACH_O_CPU_SUBTYPE_X86_ALL = 3,

  /* arm.  */
  BFD_MACH_O_CPU_SUBTYPE_ARM_ALL = 0,
  BFD_MACH_O_CPU_SUBTYPE_ARM_V4T = 5,
  BFD_MACH_O_CPU_SUBTYPE_ARM_V6 = 6,
  BFD_MACH_O_CPU_SUBTYPE_ARM_V5TEJ = 7,
  BFD_MACH_O_CPU_SUBTYPE_ARM_XSCALE = 8,
  BFD_MACH_O_CPU_SUBTYPE_ARM_V7 = 9,

  /* arm64.  */
  BFD_MACH_O_CPU_SUBTYPE_ARM64_ALL = 0,
  BFD_MACH_O_CPU_SUBTYPE_ARM64_V8 = 1
}
bfd_mach_o_cpu_subtype;

typedef enum bfd_mach_o_filetype
{
  BFD_MACH_O_MH_OBJECT      = 0x01,
  BFD_MACH_O_MH_EXECUTE     = 0x02,
  BFD_MACH_O_MH_FVMLIB      = 0x03,
  BFD_MACH_O_MH_CORE        = 0x04,
  BFD_MACH_O_MH_PRELOAD     = 0x05,
  BFD_MACH_O_MH_DYLIB       = 0x06,
  BFD_MACH_O_MH_DYLINKER    = 0x07,
  BFD_MACH_O_MH_BUNDLE      = 0x08,
  BFD_MACH_O_MH_DYLIB_STUB  = 0x09,
  BFD_MACH_O_MH_DSYM        = 0x0a,
  BFD_MACH_O_MH_KEXT_BUNDLE = 0x0b
}
bfd_mach_o_filetype;

typedef enum bfd_mach_o_header_flags
{
  BFD_MACH_O_MH_NOUNDEFS		= 0x0000001,
  BFD_MACH_O_MH_INCRLINK		= 0x0000002,
  BFD_MACH_O_MH_DYLDLINK		= 0x0000004,
  BFD_MACH_O_MH_BINDATLOAD		= 0x0000008,
  BFD_MACH_O_MH_PREBOUND		= 0x0000010,
  BFD_MACH_O_MH_SPLIT_SEGS		= 0x0000020,
  BFD_MACH_O_MH_LAZY_INIT		= 0x0000040,
  BFD_MACH_O_MH_TWOLEVEL		= 0x0000080,
  BFD_MACH_O_MH_FORCE_FLAT		= 0x0000100,
  BFD_MACH_O_MH_NOMULTIDEFS		= 0x0000200,
  BFD_MACH_O_MH_NOFIXPREBINDING		= 0x0000400,
  BFD_MACH_O_MH_PREBINDABLE		= 0x0000800,
  BFD_MACH_O_MH_ALLMODSBOUND		= 0x0001000,
  BFD_MACH_O_MH_SUBSECTIONS_VIA_SYMBOLS = 0x0002000,
  BFD_MACH_O_MH_CANONICAL		= 0x0004000,
  BFD_MACH_O_MH_WEAK_DEFINES		= 0x0008000,
  BFD_MACH_O_MH_BINDS_TO_WEAK		= 0x0010000,
  BFD_MACH_O_MH_ALLOW_STACK_EXECUTION	= 0x0020000,
  BFD_MACH_O_MH_ROOT_SAFE		= 0x0040000,
  BFD_MACH_O_MH_SETUID_SAFE		= 0x0080000,
  BFD_MACH_O_MH_NO_REEXPORTED_DYLIBS	= 0x0100000,
  BFD_MACH_O_MH_PIE			= 0x0200000,
  BFD_MACH_O_MH_DEAD_STRIPPABLE_DYLIB   = 0x0400000,
  BFD_MACH_O_MH_HAS_TLV_DESCRIPTORS     = 0x0800000,
  BFD_MACH_O_MH_NO_HEAP_EXECUTION       = 0x1000000,
  BFD_MACH_O_MH_APP_EXTENSION_SAFE      = 0x2000000
}
bfd_mach_o_header_flags;

/* Load command constants.  */
#define BFD_MACH_O_LC_REQ_DYLD 0x80000000

typedef enum bfd_mach_o_load_command_type
{
  BFD_MACH_O_LC_SEGMENT = 0x1,		/* File segment to be mapped.  */
  BFD_MACH_O_LC_SYMTAB = 0x2,		/* Link-edit stab symbol table info (obsolete).  */
  BFD_MACH_O_LC_SYMSEG = 0x3,		/* Link-edit gdb symbol table info.  */
  BFD_MACH_O_LC_THREAD = 0x4,		/* Thread.  */
  BFD_MACH_O_LC_UNIXTHREAD = 0x5,	/* UNIX thread (includes a stack).  */
  BFD_MACH_O_LC_LOADFVMLIB = 0x6,	/* Load a fixed VM shared library.  */
  BFD_MACH_O_LC_IDFVMLIB = 0x7,		/* Fixed VM shared library id.  */
  BFD_MACH_O_LC_IDENT = 0x8,		/* Object identification information (obsolete).  */
  BFD_MACH_O_LC_FVMFILE = 0x9,		/* Fixed VM file inclusion.  */
  BFD_MACH_O_LC_PREPAGE = 0xa,		/* Prepage command (internal use).  */
  BFD_MACH_O_LC_DYSYMTAB = 0xb,		/* Dynamic link-edit symbol table info.  */
  BFD_MACH_O_LC_LOAD_DYLIB = 0xc,	/* Load a dynamically linked shared library.  */
  BFD_MACH_O_LC_ID_DYLIB = 0xd,		/* Dynamically linked shared lib identification.  */
  BFD_MACH_O_LC_LOAD_DYLINKER = 0xe,	/* Load a dynamic linker.  */
  BFD_MACH_O_LC_ID_DYLINKER = 0xf,	/* Dynamic linker identification.  */
  BFD_MACH_O_LC_PREBOUND_DYLIB = 0x10,	/* Modules prebound for a dynamically.  */
  BFD_MACH_O_LC_ROUTINES = 0x11,	/* Image routines.  */
  BFD_MACH_O_LC_SUB_FRAMEWORK = 0x12,	/* Sub framework.  */
  BFD_MACH_O_LC_SUB_UMBRELLA = 0x13,	/* Sub umbrella.  */
  BFD_MACH_O_LC_SUB_CLIENT = 0x14,	/* Sub client.  */
  BFD_MACH_O_LC_SUB_LIBRARY = 0x15,   	/* Sub library.  */
  BFD_MACH_O_LC_TWOLEVEL_HINTS = 0x16,	/* Two-level namespace lookup hints.  */
  BFD_MACH_O_LC_PREBIND_CKSUM = 0x17, 	/* Prebind checksum.  */
  /* Load a dynamically linked shared library that is allowed to be
       missing (weak).  */
  BFD_MACH_O_LC_LOAD_WEAK_DYLIB = 0x18,
  BFD_MACH_O_LC_SEGMENT_64 = 0x19,		/* 64-bit segment of this file to be
						   mapped.  */
  BFD_MACH_O_LC_ROUTINES_64 = 0x1a,     	/* Address of the dyld init routine
						   in a dylib.  */
  BFD_MACH_O_LC_UUID = 0x1b,            	/* 128-bit UUID of the executable.  */
  BFD_MACH_O_LC_RPATH = 0x1c,			/* Run path addiions.  */
  BFD_MACH_O_LC_CODE_SIGNATURE = 0x1d,		/* Local of code signature.  */
  BFD_MACH_O_LC_SEGMENT_SPLIT_INFO = 0x1e,	/* Local of info to split seg.  */
  BFD_MACH_O_LC_REEXPORT_DYLIB = 0x1f,		/* Load and re-export lib.  */
  BFD_MACH_O_LC_LAZY_LOAD_DYLIB = 0x20,		/* Delay load of lib until use.  */
  BFD_MACH_O_LC_ENCRYPTION_INFO = 0x21,		/* Encrypted segment info.  */
  BFD_MACH_O_LC_DYLD_INFO = 0x22,		/* Compressed dyld information.  */
  BFD_MACH_O_LC_LOAD_UPWARD_DYLIB = 0x23,	/* Load upward dylib.  */
  BFD_MACH_O_LC_VERSION_MIN_MACOSX = 0x24,	/* Minimal macOS version.  */
  BFD_MACH_O_LC_VERSION_MIN_IPHONEOS = 0x25,	/* Minimal iOS version.  */
  BFD_MACH_O_LC_FUNCTION_STARTS = 0x26,  	/* Compressed table of func start.  */
  BFD_MACH_O_LC_DYLD_ENVIRONMENT = 0x27, 	/* Env variable string for dyld.  */
  BFD_MACH_O_LC_MAIN = 0x28,             	/* Entry point.  */
  BFD_MACH_O_LC_DATA_IN_CODE = 0x29,     	/* Table of non-instructions.  */
  BFD_MACH_O_LC_SOURCE_VERSION = 0x2a,   	/* Source version.  */
  BFD_MACH_O_LC_DYLIB_CODE_SIGN_DRS = 0x2b,	/* DRs from dylibs.  */
  BFD_MACH_O_LC_ENCRYPTION_INFO_64 = 0x2c,	/* Encrypted 64 bit seg info.  */
  BFD_MACH_O_LC_LINKER_OPTIONS = 0x2d,		/* Linker options.  */
  BFD_MACH_O_LC_LINKER_OPTIMIZATION_HINT = 0x2e,/* Optimization hints.  */
  BFD_MACH_O_LC_VERSION_MIN_TVOS = 0x2f,	/* Minimal tvOS version.  */
  BFD_MACH_O_LC_VERSION_MIN_WATCHOS = 0x30,	/* Minimal watchOS version.  */
  BFD_MACH_O_LC_NOTE = 0x31,			/* Region of arbitrary data.  */
  BFD_MACH_O_LC_BUILD_VERSION = 0x32,		/* Generic build version.  */
  BFD_MACH_O_LC_DYLD_EXPORTS_TRIE = 0x33,	/* Exports trie. */
  BFD_MACH_O_LC_DYLD_CHAINED_FIXUPS = 0x34,	/* Chained fixups. */
}
bfd_mach_o_load_command_type;

/* Section constants.  */
/* Constants for the type of a section.  */

typedef enum bfd_mach_o_section_type
{
  /* Regular section.  */
  BFD_MACH_O_S_REGULAR = 0x0,

  /* Zero fill on demand section.  */
  BFD_MACH_O_S_ZEROFILL = 0x1,

  /* Section with only literal C strings.  */
  BFD_MACH_O_S_CSTRING_LITERALS = 0x2,

  /* Section with only 4 byte literals.  */
  BFD_MACH_O_S_4BYTE_LITERALS = 0x3,

  /* Section with only 8 byte literals.  */
  BFD_MACH_O_S_8BYTE_LITERALS = 0x4,

  /* Section with only pointers to literals.  */
  BFD_MACH_O_S_LITERAL_POINTERS = 0x5,

  /* For the two types of symbol pointers sections and the symbol stubs
     section they have indirect symbol table entries.  For each of the
     entries in the section the indirect symbol table entries, in
     corresponding order in the indirect symbol table, start at the index
     stored in the reserved1 field of the section structure.  Since the
     indirect symbol table entries correspond to the entries in the
     section the number of indirect symbol table entries is inferred from
     the size of the section divided by the size of the entries in the
     section.  For symbol pointers sections the size of the entries in
     the section is 4 bytes and for symbol stubs sections the byte size
     of the stubs is stored in the reserved2 field of the section
     structure.  */

  /* Section with only non-lazy symbol pointers.  */
  BFD_MACH_O_S_NON_LAZY_SYMBOL_POINTERS = 0x6,

  /* Section with only lazy symbol pointers.  */
  BFD_MACH_O_S_LAZY_SYMBOL_POINTERS = 0x7,

  /* Section with only symbol stubs, byte size of stub in the reserved2
     field.  */
  BFD_MACH_O_S_SYMBOL_STUBS = 0x8,

  /* Section with only function pointers for initialization.  */
  BFD_MACH_O_S_MOD_INIT_FUNC_POINTERS = 0x9,

  /* Section with only function pointers for termination.  */
  BFD_MACH_O_S_MOD_FINI_FUNC_POINTERS = 0xa,

  /* Section contains symbols that are coalesced by the linkers.  */
  BFD_MACH_O_S_COALESCED = 0xb,

  /* Zero fill on demand section (possibly larger than 4 GB).  */
  BFD_MACH_O_S_GB_ZEROFILL = 0xc,

  /* Section with only pairs of function pointers for interposing.  */
  BFD_MACH_O_S_INTERPOSING = 0xd,

  /* Section with only 16 byte literals.  */
  BFD_MACH_O_S_16BYTE_LITERALS = 0xe,

  /* Section contains DTrace Object Format.  */
  BFD_MACH_O_S_DTRACE_DOF = 0xf,

  /* Section with only lazy symbol pointers to lazy loaded dylibs.  */
  BFD_MACH_O_S_LAZY_DYLIB_SYMBOL_POINTERS = 0x10
}
bfd_mach_o_section_type;

/* The flags field of a section structure is separated into two parts a section
   type and section attributes.  The section types are mutually exclusive (it
   can only have one type) but the section attributes are not (it may have more
   than one attribute).  */

#define BFD_MACH_O_SECTION_TYPE_MASK        0x000000ff

/* Constants for the section attributes part of the flags field of a section
   structure.  */
#define BFD_MACH_O_SECTION_ATTRIBUTES_MASK  0xffffff00
/* System setable attributes.  */
#define BFD_MACH_O_SECTION_ATTRIBUTES_SYS   0x00ffff00
/* User attributes.  */
#define BFD_MACH_O_SECTION_ATTRIBUTES_USR   0xff000000

typedef enum bfd_mach_o_section_attribute
{
  /* Section has no specified attibutes.  */
  BFD_MACH_O_S_ATTR_NONE              = 0,

  /* Section has local relocation entries.  */
  BFD_MACH_O_S_ATTR_LOC_RELOC         = 0x00000100,

  /* Section has external relocation entries.  */
  BFD_MACH_O_S_ATTR_EXT_RELOC         = 0x00000200,

  /* Section contains some machine instructions.  */
  BFD_MACH_O_S_ATTR_SOME_INSTRUCTIONS = 0x00000400,

  /* A debug section.  */
  BFD_MACH_O_S_ATTR_DEBUG             = 0x02000000,

  /* Used with i386 stubs.  */
  BFD_MACH_O_S_SELF_MODIFYING_CODE    = 0x04000000,

  /* Blocks are live if they reference live blocks.  */
  BFD_MACH_O_S_ATTR_LIVE_SUPPORT      = 0x08000000,

  /* No dead stripping.  */
  BFD_MACH_O_S_ATTR_NO_DEAD_STRIP     = 0x10000000,

  /* Section symbols can be stripped in files with MH_DYLDLINK flag.  */
  BFD_MACH_O_S_ATTR_STRIP_STATIC_SYMS = 0x20000000,

  /* Section contains coalesced symbols that are not to be in the TOC of an
     archive.  */
  BFD_MACH_O_S_ATTR_NO_TOC            = 0x40000000,

  /* Section contains only true machine instructions.  */
  BFD_MACH_O_S_ATTR_PURE_INSTRUCTIONS = 0x80000000
}
bfd_mach_o_section_attribute;

/* Symbol constants.  */

/* Symbol n_type values.  */
#define BFD_MACH_O_N_STAB  0xe0	/* If any of these bits set, a symbolic debugging entry.  */
#define BFD_MACH_O_N_PEXT  0x10	/* Private external symbol bit.  */
#define BFD_MACH_O_N_TYPE  0x0e	/* Mask for the type bits.  */
#define BFD_MACH_O_N_EXT   0x01	/* External symbol bit, set for external symbols.  */
#define BFD_MACH_O_N_UNDF  0x00	/* Undefined, n_sect == NO_SECT.  */
#define BFD_MACH_O_N_ABS   0x02	/* Absolute, n_sect == NO_SECT.  */
#define BFD_MACH_O_N_INDR  0x0a	/* Indirect.  */
#define BFD_MACH_O_N_PBUD  0x0c /* Prebound undefined (defined in a dylib).  */
#define BFD_MACH_O_N_SECT  0x0e	/* Defined in section number n_sect.  */

#define BFD_MACH_O_NO_SECT 0	/* Symbol not in any section of the image.  */

/* Symbol n_desc reference flags.  */
#define BFD_MACH_O_REFERENCE_MASK 				0x07
#define BFD_MACH_O_REFERENCE_FLAG_UNDEFINED_NON_LAZY		0x00
#define BFD_MACH_O_REFERENCE_FLAG_UNDEFINED_LAZY		0x01
#define BFD_MACH_O_REFERENCE_FLAG_DEFINED			0x02
#define BFD_MACH_O_REFERENCE_FLAG_PRIVATE_DEFINED		0x03
#define BFD_MACH_O_REFERENCE_FLAG_PRIVATE_UNDEFINED_NON_LAZY	0x04
#define BFD_MACH_O_REFERENCE_FLAG_PRIVATE_UNDEFINED_LAZY	0x05

#define BFD_MACH_O_REFERENCED_DYNAMICALLY			0x10
#define BFD_MACH_O_N_DESC_DISCARDED				0x20
#define BFD_MACH_O_N_NO_DEAD_STRIP				0x20
#define BFD_MACH_O_N_WEAK_REF					0x40
#define BFD_MACH_O_N_WEAK_DEF					0x80
#define BFD_MACH_O_N_REF_TO_WEAK				0x80

#define BFD_MACH_O_N_ARM_THUMB_DEF				0x08
#define BFD_MACH_O_N_SYMBOL_RESOLVER				0x100

#define BFD_MACH_O_INDIRECT_SYM_LOCAL			0x80000000
#define BFD_MACH_O_INDIRECT_SYM_ABS			0x40000000

/* Constants for dyld info rebase.  */
#define BFD_MACH_O_REBASE_OPCODE_MASK     0xf0
#define BFD_MACH_O_REBASE_IMMEDIATE_MASK  0x0f

/* The rebase opcodes.  */
#define BFD_MACH_O_REBASE_OPCODE_DONE                               0x00
#define BFD_MACH_O_REBASE_OPCODE_SET_TYPE_IMM                       0x10
#define BFD_MACH_O_REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB        0x20
#define BFD_MACH_O_REBASE_OPCODE_ADD_ADDR_ULEB                      0x30
#define BFD_MACH_O_REBASE_OPCODE_ADD_ADDR_IMM_SCALED                0x40
#define BFD_MACH_O_REBASE_OPCODE_DO_REBASE_IMM_TIMES                0x50
#define BFD_MACH_O_REBASE_OPCODE_DO_REBASE_ULEB_TIMES               0x60
#define BFD_MACH_O_REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB            0x70
#define BFD_MACH_O_REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB 0x80

/* The rebase type.  */
#define BFD_MACH_O_REBASE_TYPE_POINTER            1
#define BFD_MACH_O_REBASE_TYPE_TEXT_ABSOLUTE32    2
#define BFD_MACH_O_REBASE_TYPE_TEXT_PCREL32       3

/* Constants for dyld info bind.  */
#define BFD_MACH_O_BIND_OPCODE_MASK    0xf0
#define BFD_MACH_O_BIND_IMMEDIATE_MASK 0x0f

/* The bind opcodes.  */
#define BFD_MACH_O_BIND_OPCODE_DONE                   	      	 0x00
#define BFD_MACH_O_BIND_OPCODE_SET_DYLIB_ORDINAL_IMM  	      	 0x10
#define BFD_MACH_O_BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB 	      	 0x20
#define BFD_MACH_O_BIND_OPCODE_SET_DYLIB_SPECIAL_IMM  	      	 0x30
#define BFD_MACH_O_BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM 	 0x40
#define BFD_MACH_O_BIND_OPCODE_SET_TYPE_IMM                  	 0x50
#define BFD_MACH_O_BIND_OPCODE_SET_ADDEND_SLEB               	 0x60
#define BFD_MACH_O_BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB   	 0x70
#define BFD_MACH_O_BIND_OPCODE_ADD_ADDR_ULEB                 	 0x80
#define BFD_MACH_O_BIND_OPCODE_DO_BIND                       	 0x90
#define BFD_MACH_O_BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB         	 0xa0
#define BFD_MACH_O_BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED   	 0xb0
#define BFD_MACH_O_BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB 0xc0

/* The bind types.  */
#define BFD_MACH_O_BIND_TYPE_POINTER            1
#define BFD_MACH_O_BIND_TYPE_TEXT_ABSOLUTE32    2
#define BFD_MACH_O_BIND_TYPE_TEXT_PCREL32       3

/* The special dylib.  */
#define BFD_MACH_O_BIND_SPECIAL_DYLIB_SELF             0
#define BFD_MACH_O_BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE -1
#define BFD_MACH_O_BIND_SPECIAL_DYLIB_FLAT_LOOKUP     -2

/* Constants for dyld info export.  */
#define BFD_MACH_O_EXPORT_SYMBOL_FLAGS_KIND_MASK            0x03
#define BFD_MACH_O_EXPORT_SYMBOL_FLAGS_KIND_REGULAR         0x00
#define BFD_MACH_O_EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL    0x01
#define BFD_MACH_O_EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION      0x04
#define BFD_MACH_O_EXPORT_SYMBOL_FLAGS_REEXPORT             0x08
#define BFD_MACH_O_EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER    0x10

/* Constants for DATA_IN_CODE entries.  */
typedef enum bfd_mach_o_data_in_code_entry_kind
{
  BFD_MACH_O_DICE_KIND_DATA         = 0x0001, /* Data */
  BFD_MACH_O_DICE_JUMP_TABLES8      = 0x0002, /* 1 byte jump tables.  */
  BFD_MACH_O_DICE_JUMP_TABLES16     = 0x0003, /* 2 bytes.  */
  BFD_MACH_O_DICE_JUMP_TABLES32     = 0x0004, /* 4 bytes.  */
  BFD_MACH_O_DICE_ABS_JUMP_TABLES32 = 0x0005  /* Absolute jump table.  */
} bfd_mach_o_data_in_code_entry_kind;

/* Thread constants.  */

typedef enum bfd_mach_o_ppc_thread_flavour
{
  BFD_MACH_O_PPC_THREAD_STATE      = 1,
  BFD_MACH_O_PPC_FLOAT_STATE       = 2,
  BFD_MACH_O_PPC_EXCEPTION_STATE   = 3,
  BFD_MACH_O_PPC_VECTOR_STATE      = 4,
  BFD_MACH_O_PPC_THREAD_STATE64    = 5,
  BFD_MACH_O_PPC_EXCEPTION_STATE64 = 6,
  BFD_MACH_O_PPC_THREAD_STATE_NONE = 7
}
bfd_mach_o_ppc_thread_flavour;

/* Defined in <mach/i386/thread_status.h> */
typedef enum bfd_mach_o_i386_thread_flavour
{
  BFD_MACH_O_x86_THREAD_STATE32    = 1,
  BFD_MACH_O_x86_FLOAT_STATE32     = 2,
  BFD_MACH_O_x86_EXCEPTION_STATE32 = 3,
  BFD_MACH_O_x86_THREAD_STATE64    = 4,
  BFD_MACH_O_x86_FLOAT_STATE64     = 5,
  BFD_MACH_O_x86_EXCEPTION_STATE64 = 6,
  BFD_MACH_O_x86_THREAD_STATE      = 7,
  BFD_MACH_O_x86_FLOAT_STATE       = 8,
  BFD_MACH_O_x86_EXCEPTION_STATE   = 9,
  BFD_MACH_O_x86_DEBUG_STATE32     = 10,
  BFD_MACH_O_x86_DEBUG_STATE64     = 11,
  BFD_MACH_O_x86_DEBUG_STATE       = 12,
  BFD_MACH_O_x86_THREAD_STATE_NONE = 13
}
bfd_mach_o_i386_thread_flavour;

#endif /* _MACH_O_LOADER_H */
