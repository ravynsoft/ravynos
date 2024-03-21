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

#ifndef _MACH_O_EXTERNAL_H
#define _MACH_O_EXTERNAL_H

struct mach_o_header_external
{
  unsigned char magic[4];	/* Magic number.  */
  unsigned char cputype[4];	/* CPU that this object is for.  */
  unsigned char cpusubtype[4];	/* CPU subtype.  */
  unsigned char filetype[4];	/* Type of file.  */
  unsigned char ncmds[4];	/* Number of load commands.  */
  unsigned char sizeofcmds[4];	/* Total size of load commands.  */
  unsigned char flags[4];	/* Flags.  */
  unsigned char reserved[4];	/* Reserved (on 64-bit version only).  */
};

#define BFD_MACH_O_HEADER_SIZE 28
#define BFD_MACH_O_HEADER_64_SIZE 32

/* 32-bit section header.  */

struct mach_o_section_32_external
{
  unsigned char sectname[16];   /* Section name.  */
  unsigned char segname[16];    /* Segment that the section belongs to.  */
  unsigned char addr[4];        /* Address of this section in memory.  */
  unsigned char size[4];        /* Size in bytes of this section.  */
  unsigned char offset[4];      /* File offset of this section.  */
  unsigned char align[4];       /* log2 of this section's alignment.  */
  unsigned char reloff[4];      /* File offset of this section's relocs.  */
  unsigned char nreloc[4];      /* Number of relocs for this section.  */
  unsigned char flags[4];       /* Section flags/attributes.  */
  unsigned char reserved1[4];
  unsigned char reserved2[4];
};
#define BFD_MACH_O_SECTION_SIZE 68

/* 64-bit section header.  */

struct mach_o_section_64_external
{
  unsigned char sectname[16];   /* Section name.  */
  unsigned char segname[16];    /* Segment that the section belongs to.  */
  unsigned char addr[8];        /* Address of this section in memory.  */
  unsigned char size[8];        /* Size in bytes of this section.  */
  unsigned char offset[4];      /* File offset of this section.  */
  unsigned char align[4];       /* log2 of this section's alignment.  */
  unsigned char reloff[4];      /* File offset of this section's relocs.  */
  unsigned char nreloc[4];      /* Number of relocs for this section.  */
  unsigned char flags[4];       /* Section flags/attributes.  */
  unsigned char reserved1[4];
  unsigned char reserved2[4];
  unsigned char reserved3[4];
};
#define BFD_MACH_O_SECTION_64_SIZE 80

struct mach_o_load_command_external
{
  unsigned char cmd[4];         /* The type of load command.  */
  unsigned char cmdsize[4];     /* Size in bytes of entire command.  */
};
#define BFD_MACH_O_LC_SIZE 8

struct mach_o_segment_command_32_external
{
  unsigned char segname[16];    /* Name of this segment.  */
  unsigned char vmaddr[4];      /* Virtual memory address of this segment.  */
  unsigned char vmsize[4];      /* Size there, in bytes.  */
  unsigned char fileoff[4];     /* Offset in bytes of the data to be mapped.  */
  unsigned char filesize[4];    /* Size in bytes on disk.  */
  unsigned char maxprot[4];     /* Maximum permitted vm protection.  */
  unsigned char initprot[4];    /* Initial vm protection.  */
  unsigned char nsects[4];      /* Number of sections in this segment.  */
  unsigned char flags[4];       /* Flags that affect the loading.  */
};
#define BFD_MACH_O_LC_SEGMENT_SIZE 56 /* Include the header.  */

struct mach_o_segment_command_64_external
{
  unsigned char segname[16];    /* Name of this segment.  */
  unsigned char vmaddr[8];      /* Virtual memory address of this segment.  */
  unsigned char vmsize[8];      /* Size there, in bytes.  */
  unsigned char fileoff[8];     /* Offset in bytes of the data to be mapped.  */
  unsigned char filesize[8];    /* Size in bytes on disk.  */
  unsigned char maxprot[4];     /* Maximum permitted vm protection.  */
  unsigned char initprot[4];    /* Initial vm protection.  */
  unsigned char nsects[4];      /* Number of sections in this segment.  */
  unsigned char flags[4];       /* Flags that affect the loading.  */
};
#define BFD_MACH_O_LC_SEGMENT_64_SIZE 72 /* Include the header.  */

struct mach_o_reloc_info_external
{
  unsigned char r_address[4];
  unsigned char r_symbolnum[4];
};
#define BFD_MACH_O_RELENT_SIZE 8

/* Relocations are based on 'address' being a section offset and an assumption
   that sections are never more than 2^24-1 bytes in size.  Relocation data
   also carry information on type/size/PC-relative/extern and whether scattered
   or not [stored in the MSB of the r_address].  */

#define BFD_MACH_O_SR_SCATTERED		0x80000000

/* For a non-scattered reloc, the relocation info is found in r_symbolnum.
   Bytes 1 to 3 contain the symbol number (0xffffff, in a non-scattered PAIR).
   Byte 4 contains the relocation info - but with differing bit-positions
   dependent on target endian-ness - as below.  */

#define BFD_MACH_O_LE_PCREL		0x01
#define BFD_MACH_O_LE_LENGTH_SHIFT	1
#define BFD_MACH_O_LE_EXTERN		0x08
#define BFD_MACH_O_LE_TYPE_SHIFT	4

#define BFD_MACH_O_BE_PCREL		0x80
#define BFD_MACH_O_BE_LENGTH_SHIFT	5
#define BFD_MACH_O_BE_EXTERN		0x10
#define BFD_MACH_O_BE_TYPE_SHIFT	0

/* The field sizes are the same for both BE and LE.  */
#define BFD_MACH_O_LENGTH_MASK		0x03
#define BFD_MACH_O_TYPE_MASK		0x0f

/* For a scattered reloc entry the info is contained in r_address.  There
   is no need to discriminate on target endian-ness, since the design was
   arranged to produce the same layout on both.  Scattered relocations are
   only used for local items, therefore there is no 'extern' field.  */

#define BFD_MACH_O_SR_PCREL		0x40000000
#define BFD_MACH_O_GET_SR_LENGTH(s)	(((s) >> 28) & 0x3)
#define BFD_MACH_O_GET_SR_TYPE(s)	(((s) >> 24) & 0x0f)
#define BFD_MACH_O_GET_SR_ADDRESS(s)	((s) & 0x00ffffff)
#define BFD_MACH_O_SET_SR_LENGTH(l)	(((l) & 0x3) << 28)
#define BFD_MACH_O_SET_SR_TYPE(t)	(((t) & 0xf) << 24)
#define BFD_MACH_O_SET_SR_ADDRESS(s)	((s) & 0x00ffffff)

struct mach_o_symtab_command_external
{
  unsigned char symoff[4];	/* File offset of the symbol table.  */
  unsigned char nsyms[4];	/* Number of symbols.  */
  unsigned char stroff[4];	/* File offset of the string table.  */
  unsigned char strsize[4];	/* String table size.  */
};

struct mach_o_nlist_external
{
  unsigned char n_strx[4];
  unsigned char n_type[1];
  unsigned char n_sect[1];
  unsigned char n_desc[2];
  unsigned char n_value[4];
};
#define BFD_MACH_O_NLIST_SIZE 12

struct mach_o_nlist_64_external
{
  unsigned char n_strx[4];
  unsigned char n_type[1];
  unsigned char n_sect[1];
  unsigned char n_desc[2];
  unsigned char n_value[8];
};
#define BFD_MACH_O_NLIST_64_SIZE 16

struct mach_o_thread_command_external
{
  unsigned char flavour[4];
  unsigned char count[4];
};

/* For commands that just have a string or a path.  */
struct mach_o_str_command_external
{
  unsigned char str[4];
};

struct mach_o_dylib_command_external
{
  unsigned char name[4];
  unsigned char timestamp[4];
  unsigned char current_version[4];
  unsigned char compatibility_version[4];
};

struct mach_o_dysymtab_command_external
{
  unsigned char ilocalsym[4];	/* Index of.  */
  unsigned char nlocalsym[4];	/* Number of.  */
  unsigned char iextdefsym[4];
  unsigned char nextdefsym[4];
  unsigned char iundefsym[4];
  unsigned char nundefsym[4];
  unsigned char tocoff[4];
  unsigned char ntoc[4];
  unsigned char modtaboff[4];
  unsigned char nmodtab[4];
  unsigned char extrefsymoff[4];
  unsigned char nextrefsyms[4];
  unsigned char indirectsymoff[4];
  unsigned char nindirectsyms[4];
  unsigned char extreloff[4];
  unsigned char nextrel[4];
  unsigned char locreloff[4];
  unsigned char nlocrel[4];
};

struct mach_o_dylib_module_external
{
  unsigned char module_name[4];
  unsigned char iextdefsym[4];
  unsigned char nextdefsym[4];
  unsigned char irefsym[4];
  unsigned char nrefsym[4];
  unsigned char ilocalsym[4];
  unsigned char nlocalsym[4];
  unsigned char iextrel[4];
  unsigned char nextrel[4];
  unsigned char iinit_iterm[4];
  unsigned char ninit_nterm[4];
  unsigned char objc_module_info_addr[4];
  unsigned char objc_module_info_size[4];
};
#define BFD_MACH_O_DYLIB_MODULE_SIZE 52

struct mach_o_dylib_module_64_external
{
  unsigned char module_name[4];
  unsigned char iextdefsym[4];
  unsigned char nextdefsym[4];
  unsigned char irefsym[4];
  unsigned char nrefsym[4];
  unsigned char ilocalsym[4];
  unsigned char nlocalsym[4];
  unsigned char iextrel[4];
  unsigned char nextrel[4];
  unsigned char iinit_iterm[4];
  unsigned char ninit_nterm[4];
  unsigned char objc_module_info_size[4];
  unsigned char objc_module_info_addr[8];
};
#define BFD_MACH_O_DYLIB_MODULE_64_SIZE 56

struct mach_o_dylib_table_of_contents_external
{
  unsigned char symbol_index[4];
  unsigned char module_index[4];
};
#define BFD_MACH_O_TABLE_OF_CONTENT_SIZE 8

struct mach_o_linkedit_data_command_external
{
  unsigned char dataoff[4];
  unsigned char datasize[4];
};

struct mach_o_dyld_info_command_external
{
  unsigned char rebase_off[4];
  unsigned char rebase_size[4];
  unsigned char bind_off[4];
  unsigned char bind_size[4];
  unsigned char weak_bind_off[4];
  unsigned char weak_bind_size[4];
  unsigned char lazy_bind_off[4];
  unsigned char lazy_bind_size[4];
  unsigned char export_off[4];
  unsigned char export_size[4];
};

struct mach_o_prebound_dylib_command_external
{
  unsigned char name[4];
  unsigned char nmodules[4];
  unsigned char linked_modules[4];
};

struct mach_o_prebind_cksum_command_external
{
  unsigned char cksum[4];
};

struct mach_o_twolevel_hints_command_external
{
  unsigned char offset[4];
  unsigned char nhints[4];
};

struct mach_o_version_min_command_external
{
  unsigned char version[4];
  unsigned char sdk[4];
};

struct mach_o_encryption_info_command_external
{
  unsigned char cryptoff[4];	/* File offset of the encrypted area.  */
  unsigned char cryptsize[4];	/* Size of the encrypted area.  */
  unsigned char cryptid[4];	/* Encryption method.  */
};

struct mach_o_encryption_info_64_command_external
{
  unsigned char cryptoff[4];	/* File offset of the encrypted area.  */
  unsigned char cryptsize[4];	/* Size of the encrypted area.  */
  unsigned char cryptid[4];	/* Encryption method.  */
  unsigned char pad[4];		/* Pad to make struct size a multiple of 8.  */
};

struct mach_o_fvmlib_command_external
{
  unsigned char name[4];	/* Offset of the name.  */
  unsigned char minor_version[4];
  unsigned char header_addr[4];
};

struct mach_o_entry_point_command_external
{
  unsigned char entryoff[8];	/* File offset of the entry point.  */
  unsigned char stacksize[8];   /* Initial stack size, if no null.  */
};

struct mach_o_source_version_command_external
{
  unsigned char version[8];	/* Version A.B.C.D.E, with 10 bits for B-E,
				   and 24 bits for A.  */
};

struct mach_o_note_command_external
{
  unsigned char data_owner[16]; /* Owner name for this note.  */
  unsigned char offset[8];      /* File offset of the note.  */
  unsigned char size[8];        /* Length of the note.  */
};

struct mach_o_build_version_command_external
{
  unsigned char platform[4];    /* Target platform.  */
  unsigned char minos[4];       /* X.Y.Z is encoded in nibbles xxxx.yy.zz.  */
  unsigned char sdk[4];         /* X.Y.Z is encoded in nibbles xxxx.yy.zz.  */
  unsigned char ntools[4];      /* Number of tool entries following this.  */
};

/* The LD_DATA_IN_CODE command use a linkedit_data_command that points to
   a table of entries.  */

struct mach_o_data_in_code_entry_external
{
  unsigned char offset[4];	/* Offset from the mach_header.  */
  unsigned char length[2];	/* Number of bytes.  */
  unsigned char kind[2];	/* Kind.  See BFD_MACH_O_DICE_ values.  */
};

struct mach_o_linker_option_command_external
{
  unsigned char count[4];	/* Number of strings.  */
  /* COUNT null terminated UTF-8 strings, with 0 at the end for padding.  */
};

struct mach_o_fat_header_external
{
  unsigned char magic[4];
  unsigned char nfat_arch[4];	/* Number of components.  */
};

struct mach_o_fat_arch_external
{
  unsigned char cputype[4];
  unsigned char cpusubtype[4];
  unsigned char offset[4];	/* File offset of the member.  */
  unsigned char size[4];	/* Size of the member.  */
  unsigned char align[4];	/* Power of 2.  */
};

struct mach_o_dyld_cache_header_external
{
  unsigned char magic[16];

  unsigned char mapping_offset[4];
  unsigned char mapping_count[4];
  unsigned char images_offset[4];
  unsigned char images_count[4];

  unsigned char dyld_base_address[8];

  unsigned char code_signature_offset[8];
  unsigned char code_signature_size[8];

  unsigned char slide_info_offset[8];
  unsigned char slide_info_size[8];

  unsigned char local_symbols_offset[8];
  unsigned char local_symbols_size[8];
};

struct mach_o_dyld_cache_mapping_info_external
{
  unsigned char address[8];
  unsigned char size[8];
  unsigned char file_offset[8];
  unsigned char max_prot[4];
  unsigned char init_prot[4];
};

struct mach_o_dyld_cache_image_info_external
{
  unsigned char address[8];
  unsigned char mtime[8];
  unsigned char inode[8];
  unsigned char path_file_offset[4];
  unsigned char pad[4];
};


#endif /* _MACH_O_EXTERNAL_H */
