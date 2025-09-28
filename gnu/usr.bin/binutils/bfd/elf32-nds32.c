/* NDS32-specific support for 32-bit ELF.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Andes Technology Corporation.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */


#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "libiberty.h"
#include "elf/nds32.h"
#include "opcode/nds32.h"
#include "elf32-nds32.h"
#include "opcode/cgen.h"
#include "../opcodes/nds32-opc.h"

/* All users of this file have bfd_octets_per_byte (abfd, sec) == 1.  */
#define OCTETS_PER_BYTE(ABFD, SEC) 1

/* Relocation HOWTO functions.  */
static bfd_reloc_status_type nds32_elf_ignore_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nds32_elf_9_pcrel_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type nds32_elf_hi20_reloc
  (bfd *, arelent *, asymbol *, void *,
   asection *, bfd *, char **);
static bfd_reloc_status_type nds32_elf_lo12_reloc
  (bfd *, arelent *, asymbol *, void *,
   asection *, bfd *, char **);
static bfd_reloc_status_type nds32_elf_generic_reloc
  (bfd *, arelent *, asymbol *, void *,
   asection *, bfd *, char **);
static bfd_reloc_status_type nds32_elf_sda15_reloc
  (bfd *, arelent *, asymbol *, void *,
   asection *, bfd *, char **);

/* Helper functions for HOWTO.  */
static bfd_reloc_status_type nds32_elf_do_9_pcrel_reloc
  (bfd *, reloc_howto_type *, asection *, bfd_byte *, bfd_vma,
   asection *, bfd_vma, bfd_vma);

/* Nds32 helper functions.  */
static bfd_vma calculate_memory_address
  (bfd *, Elf_Internal_Rela *, Elf_Internal_Sym *, Elf_Internal_Shdr *);
static int nds32_get_section_contents (bfd *, asection *,
				       bfd_byte **, bool);
static int nds32_get_local_syms (bfd *, asection *ATTRIBUTE_UNUSED,
				 Elf_Internal_Sym **);
static bool  nds32_relax_fp_as_gp
  (struct bfd_link_info *link_info, bfd *abfd, asection *sec,
   Elf_Internal_Rela *internal_relocs, Elf_Internal_Rela *irelend,
   Elf_Internal_Sym *isymbuf);
static bool nds32_fag_remove_unused_fpbase
  (bfd *abfd, asection *sec, Elf_Internal_Rela *internal_relocs,
   Elf_Internal_Rela *irelend);

enum
{
  MACH_V1 = bfd_mach_n1h,
  MACH_V2 = bfd_mach_n1h_v2,
  MACH_V3 = bfd_mach_n1h_v3,
  MACH_V3M = bfd_mach_n1h_v3m
};

#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER "/usr/lib/ld.so.1"

#define NDS32_GUARD_SEC_P(flags) ((flags) & SEC_ALLOC \
				  && (flags) & SEC_LOAD \
				  && (flags) & SEC_READONLY)

/* The nop opcode we use.  */
#define NDS32_NOP32 0x40000009
#define NDS32_NOP16 0x9200

/* The size in bytes of an entry in the procedure linkage table.  */
#define PLT_ENTRY_SIZE 24
#define PLT_HEADER_SIZE 24

/* The first entry in a procedure linkage table are reserved,
   and the initial contents are unimportant (we zero them out).
   Subsequent entries look like this.  */
#define PLT0_ENTRY_WORD0  0x46f00000		/* sethi   r15, HI20(.got+4)	  */
#define PLT0_ENTRY_WORD1  0x58f78000		/* ori	   r15, r25, LO12(.got+4) */
#define PLT0_ENTRY_WORD2  0x05178000		/* lwi	   r17, [r15+0]		  */
#define PLT0_ENTRY_WORD3  0x04f78001		/* lwi	   r15, [r15+4]		  */
#define PLT0_ENTRY_WORD4  0x4a003c00		/* jr	   r15			  */

/* $ta is change to $r15 (from $r25).  */
#define PLT0_PIC_ENTRY_WORD0  0x46f00000	/* sethi   r15, HI20(got[1]@GOT)  */
#define PLT0_PIC_ENTRY_WORD1  0x58f78000	/* ori	   r15, r15, LO12(got[1]@GOT) */
#define PLT0_PIC_ENTRY_WORD2  0x40f7f400	/* add	   r15, gp, r15		  */
#define PLT0_PIC_ENTRY_WORD3  0x05178000	/* lwi	   r17, [r15+0]		  */
#define PLT0_PIC_ENTRY_WORD4  0x04f78001	/* lwi	   r15, [r15+4]		  */
#define PLT0_PIC_ENTRY_WORD5  0x4a003c00	/* jr	   r15			  */

#define PLT_ENTRY_WORD0	 0x46f00000		/* sethi   r15, HI20(&got[n+3])	     */
#define PLT_ENTRY_WORD1	 0x04f78000		/* lwi	   r15, r15, LO12(&got[n+3]) */
#define PLT_ENTRY_WORD2	 0x4a003c00		/* jr	   r15			     */
#define PLT_ENTRY_WORD3	 0x45000000		/* movi	   r16, sizeof(RELA) * n     */
#define PLT_ENTRY_WORD4	 0x48000000		/* j	  .plt0.		     */

#define PLT_PIC_ENTRY_WORD0  0x46f00000		/* sethi  r15, HI20(got[n+3]@GOT)    */
#define PLT_PIC_ENTRY_WORD1  0x58f78000		/* ori	  r15, r15,    LO12(got[n+3]@GOT) */
#define PLT_PIC_ENTRY_WORD2  0x38febc02		/* lw	  r15, [gp+r15]		     */
#define PLT_PIC_ENTRY_WORD3  0x4a003c00		/* jr	  r15			     */
#define PLT_PIC_ENTRY_WORD4  0x45000000		/* movi	  r16, sizeof(RELA) * n	     */
#define PLT_PIC_ENTRY_WORD5  0x48000000		/* j	  .plt0			     */

/* These are macros used to get the relocation accurate value.  */
#define ACCURATE_8BIT_S1	(0x100)
#define ACCURATE_U9BIT_S1	(0x400)
#define ACCURATE_12BIT_S1	(0x2000)
#define ACCURATE_14BIT_S1	(0x4000)
#define ACCURATE_19BIT		(0x40000)

/* These are macros used to get the relocation conservative value.  */
#define CONSERVATIVE_8BIT_S1	(0x100 - 4)
#define CONSERVATIVE_14BIT_S1	(0x4000 - 4)
#define CONSERVATIVE_16BIT_S1	(0x10000 - 4)
#define CONSERVATIVE_24BIT_S1	(0x1000000 - 4)
/* These must be more conservative because the address may be in
   different segment.  */
#define CONSERVATIVE_15BIT	(0x4000 - 0x1000)
#define CONSERVATIVE_15BIT_S1	(0x8000 - 0x1000)
#define CONSERVATIVE_15BIT_S2	(0x10000 - 0x1000)
#define CONSERVATIVE_19BIT	(0x40000 - 0x1000)
#define CONSERVATIVE_20BIT	(0x80000 - 0x1000)

/* Size of small data/bss sections, used to calculate SDA_BASE.  */
static long got_size = 0;
static int is_SDA_BASE_set = 0;

/* Convert ELF-VER in eflags to string for debugging purpose.  */
static const char *const nds32_elfver_strtab[] =
{
  "ELF-1.2",
  "ELF-1.3",
  "ELF-1.4",
};

/* The nds32 linker needs to keep track of the number of relocs that it
   decides to copy in check_relocs for each symbol.  This is so that
   it can discard PC relative relocs if it doesn't need them when
   linking with -Bsymbolic.  We store the information in a field
   extending the regular ELF linker hash table.  */

/* This structure keeps track of the number of PC relative relocs we
   have copied for a given symbol.  */

struct elf_nds32_pcrel_relocs_copied
{
  /* Next section.  */
  struct elf_nds32_pcrel_relocs_copied *next;
  /* A section in dynobj.  */
  asection *section;
  /* Number of relocs copied in this section.  */
  bfd_size_type count;
};

enum elf_nds32_tls_type
{
  GOT_UNKNOWN = (0),
  GOT_NORMAL = (1 << 0),
  GOT_TLS_LE = (1 << 1),
  GOT_TLS_IE = (1 << 2),
  GOT_TLS_IEGP = (1 << 3),
  GOT_TLS_LD = (1 << 4),
  GOT_TLS_GD = (1 << 5),
  GOT_TLS_DESC = (1 << 6),
};

/* Nds32 ELF linker hash entry.  */

struct elf_nds32_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* For checking relocation type.  */
  enum elf_nds32_tls_type tls_type;

  int offset_to_gp;
};

/* Get the nds32 ELF linker hash table from a link_info structure.  */

#define FP_BASE_NAME "_FP_BASE_"
static int check_start_export_sym = 0;

/* The offset for executable tls relaxation.  */
#define TP_OFFSET 0x0

typedef struct
{
  int min_id;
  int max_id;
  int count;
  int bias;
  int init;
} elf32_nds32_relax_group_t;

struct elf_nds32_obj_tdata
{
  struct elf_obj_tdata root;

  /* tls_type for each local got entry.  */
  char *local_got_tls_type;

  /* GOTPLT entries for TLS descriptors.  */
  bfd_vma *local_tlsdesc_gotent;

  /* for R_NDS32_RELAX_GROUP handling.  */
  elf32_nds32_relax_group_t relax_group;

  unsigned int hdr_size;
  int* offset_to_gp;
};

#define elf_nds32_tdata(bfd) \
  ((struct elf_nds32_obj_tdata *) (bfd)->tdata.any)

#define elf32_nds32_local_got_tls_type(bfd) \
  (elf_nds32_tdata (bfd)->local_got_tls_type)

#define elf32_nds32_local_gp_offset(bfd) \
  (elf_nds32_tdata (bfd)->offset_to_gp)

#define elf32_nds32_hash_entry(ent) ((struct elf_nds32_link_hash_entry *)(ent))

#define elf32_nds32_relax_group_ptr(bfd) \
  &(elf_nds32_tdata (bfd)->relax_group)

static bool
nds32_elf_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_nds32_obj_tdata),
				  NDS32_ELF_DATA);
}

/* Relocations used for relocation.  */
/* Define HOWTO2 (for relocation) and HOWTO3 (for relaxation) to
   initialize array nds32_elf_howto_table in any order. The benefit
   is that we can add any new relocations with any numbers and don't
   need to fill the gap by lots of EMPTY_HOWTO. */
#define HOWTO2(C, R, S, B, P, BI, O, SF, NAME, INPLACE, MASKSRC, MASKDST, PC) \
  [C] = HOWTO(C, R, S, B, P, BI, O, SF, NAME, INPLACE, MASKSRC, MASKDST, PC)

static reloc_howto_type nds32_elf_howto_table[] =
{
  /* This reloc does nothing.  */
  HOWTO2 (R_NDS32_NONE,		/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_NONE",	/* name  */
	 false,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0,			/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* A 16 bit absolute relocation.  */
  HOWTO2 (R_NDS32_16,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size  */
	 16,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 nds32_elf_generic_reloc,/* special_function  */
	 "R_NDS32_16",		/* name  */
	 false,			/* partial_inplace  */
	 0xffff,		/* src_mask  */
	 0xffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* A 32 bit absolute relocation.  */
  HOWTO2 (R_NDS32_32,		/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 nds32_elf_generic_reloc,/* special_function  */
	 "R_NDS32_32",		/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* A 20 bit address.  */
  HOWTO2 (R_NDS32_20,		/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_unsigned,/* complain_on_overflow  */
	 nds32_elf_generic_reloc,/* special_function  */
	 "R_NDS32_20",		/* name  */
	 false,			/* partial_inplace  */
	 0xfffff,		/* src_mask  */
	 0xfffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* An PC Relative 9-bit relocation, shifted by 2.
     This reloc is complicated because relocations are relative to pc & -4.
     i.e. branches in the right insn slot use the address of the left insn
     slot for pc.  */
  /* It's not clear whether this should have partial_inplace set or not.
     Branch relaxing in the assembler can store the addend in the insn,
     and if bfd_install_relocation gets called the addend may get added
     again.  */
  HOWTO2 (R_NDS32_9_PCREL,	/* type  */
	 1,			/* rightshift  */
	 2,			/* size  */
	 8,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 nds32_elf_9_pcrel_reloc,/* special_function  */
	 "R_NDS32_9_PCREL",	/* name  */
	 false,			/* partial_inplace  */
	 0xff,			/* src_mask  */
	 0xff,			/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* A relative 15 bit relocation, right shifted by 1.  */
  HOWTO2 (R_NDS32_15_PCREL,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 14,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_15_PCREL",	/* name  */
	 false,			/* partial_inplace  */
	 0x3fff,		/* src_mask  */
	 0x3fff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* A relative 17 bit relocation, right shifted by 1.  */
  HOWTO2 (R_NDS32_17_PCREL,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 16,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_17_PCREL",	/* name  */
	 false,			/* partial_inplace  */
	 0xffff,		/* src_mask  */
	 0xffff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* A relative 25 bit relocation, right shifted by 1.  */
  /* It's not clear whether this should have partial_inplace set or not.
     Branch relaxing in the assembler can store the addend in the insn,
     and if bfd_install_relocation gets called the addend may get added
     again.  */
  HOWTO2 (R_NDS32_25_PCREL,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 24,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_25_PCREL",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffff,		/* src_mask  */
	 0xffffff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* High 20 bits of address when lower 12 is or'd in.  */
  HOWTO2 (R_NDS32_HI20,		/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_hi20_reloc,	/* special_function  */
	 "R_NDS32_HI20",	/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Lower 12 bits of address.  */
  HOWTO2 (R_NDS32_LO12S3,	/* type  */
	 3,			/* rightshift  */
	 4,			/* size  */
	 9,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_lo12_reloc,	/* special_function  */
	 "R_NDS32_LO12S3",	/* name  */
	 false,			/* partial_inplace  */
	 0x000001ff,		/* src_mask  */
	 0x000001ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Lower 12 bits of address.  */
  HOWTO2 (R_NDS32_LO12S2,	/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 10,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_lo12_reloc,	/* special_function  */
	 "R_NDS32_LO12S2",	/* name  */
	 false,			/* partial_inplace  */
	 0x000003ff,		/* src_mask  */
	 0x000003ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Lower 12 bits of address.  */
  HOWTO2 (R_NDS32_LO12S1,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 11,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_lo12_reloc,	/* special_function  */
	 "R_NDS32_LO12S1",	/* name  */
	 false,			/* partial_inplace  */
	 0x000007ff,		/* src_mask  */
	 0x000007ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Lower 12 bits of address.  */
  HOWTO2 (R_NDS32_LO12S0,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_lo12_reloc,	/* special_function  */
	 "R_NDS32_LO12S0",	/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Small data area 15 bits offset.  */
  HOWTO2 (R_NDS32_SDA15S3,	/* type  */
	 3,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 nds32_elf_sda15_reloc,	/* special_function  */
	 "R_NDS32_SDA15S3",	/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Small data area 15 bits offset.  */
  HOWTO2 (R_NDS32_SDA15S2,	/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 nds32_elf_sda15_reloc,	/* special_function  */
	 "R_NDS32_SDA15S2",	/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Small data area 15 bits offset.  */
  HOWTO2 (R_NDS32_SDA15S1,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 nds32_elf_sda15_reloc,	/* special_function  */
	 "R_NDS32_SDA15S1",	/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Small data area 15 bits offset.  */
  HOWTO2 (R_NDS32_SDA15S0,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 nds32_elf_sda15_reloc,	/* special_function  */
	 "R_NDS32_SDA15S0",	/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* GNU extension to record C++ vtable hierarchy  */
  HOWTO2 (R_NDS32_GNU_VTINHERIT,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 0,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 NULL,			/* special_function  */
	 "R_NDS32_GNU_VTINHERIT",/* name  */
	 false,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0,			/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* GNU extension to record C++ vtable member usage  */
  HOWTO2 (R_NDS32_GNU_VTENTRY,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 0,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 _bfd_elf_rel_vtable_reloc_fn,/* special_function  */
	 "R_NDS32_GNU_VTENTRY",	/* name  */
	 false,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0,			/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* A 16 bit absolute relocation.  */
  HOWTO2 (R_NDS32_16_RELA,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size  */
	 16,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_16_RELA",	/* name  */
	 false,			/* partial_inplace  */
	 0xffff,		/* src_mask  */
	 0xffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* A 32 bit absolute relocation.  */
  HOWTO2 (R_NDS32_32_RELA,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_32_RELA",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* A 20 bit address.  */
  HOWTO2 (R_NDS32_20_RELA,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_20_RELA",	/* name  */
	 false,			/* partial_inplace  */
	 0xfffff,		/* src_mask  */
	 0xfffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_9_PCREL_RELA,	/* type  */
	 1,			/* rightshift  */
	 2,			/* size  */
	 8,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_9_PCREL_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0xff,			/* src_mask  */
	 0xff,			/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* A relative 15 bit relocation, right shifted by 1.  */
  HOWTO2 (R_NDS32_15_PCREL_RELA,/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 14,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_15_PCREL_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x3fff,		/* src_mask  */
	 0x3fff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* A relative 17 bit relocation, right shifted by 1.  */
  HOWTO2 (R_NDS32_17_PCREL_RELA,/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 16,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_17_PCREL_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0xffff,		/* src_mask  */
	 0xffff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* A relative 25 bit relocation, right shifted by 2.  */
  HOWTO2 (R_NDS32_25_PCREL_RELA,/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 24,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_25_PCREL_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0xffffff,		/* src_mask  */
	 0xffffff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* High 20 bits of address when lower 16 is or'd in.  */
  HOWTO2 (R_NDS32_HI20_RELA,	/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_HI20_RELA",	/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Lower 12 bits of address.  */
  HOWTO2 (R_NDS32_LO12S3_RELA,	/* type  */
	 3,			/* rightshift  */
	 4,			/* size  */
	 9,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_LO12S3_RELA",	/* name  */
	 false,			/* partial_inplace  */
	 0x000001ff,		/* src_mask  */
	 0x000001ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Lower 12 bits of address.  */
  HOWTO2 (R_NDS32_LO12S2_RELA,	/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 10,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_LO12S2_RELA",	/* name  */
	 false,			/* partial_inplace  */
	 0x000003ff,		/* src_mask  */
	 0x000003ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Lower 12 bits of address.  */
  HOWTO2 (R_NDS32_LO12S1_RELA,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 11,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_LO12S1_RELA",	/* name  */
	 false,			/* partial_inplace  */
	 0x000007ff,		/* src_mask  */
	 0x000007ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Lower 12 bits of address.  */
  HOWTO2 (R_NDS32_LO12S0_RELA,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_LO12S0_RELA",	/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Small data area 15 bits offset.  */
  HOWTO2 (R_NDS32_SDA15S3_RELA,	/* type  */
	 3,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA15S3_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Small data area 15 bits offset.  */
  HOWTO2 (R_NDS32_SDA15S2_RELA,	/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA15S2_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_SDA15S1_RELA,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA15S1_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_SDA15S0_RELA,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA15S0_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* GNU extension to record C++ vtable hierarchy  */
  HOWTO2 (R_NDS32_RELA_GNU_VTINHERIT,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 0,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 NULL,			/* special_function  */
	 "R_NDS32_RELA_GNU_VTINHERIT",/* name  */
	 false,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0,			/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* GNU extension to record C++ vtable member usage  */
  HOWTO2 (R_NDS32_RELA_GNU_VTENTRY,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 0,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 _bfd_elf_rel_vtable_reloc_fn,/* special_function  */
	 "R_NDS32_RELA_GNU_VTENTRY",/* name  */
	 false,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0,			/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Like R_NDS32_20, but referring to the GOT table entry for
     the symbol.  */
  HOWTO2 (R_NDS32_GOT20,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOT20",	/* name  */
	 false,			/* partial_inplace  */
	 0xfffff,		/* src_mask  */
	 0xfffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Like R_NDS32_PCREL, but referring to the procedure linkage table
     entry for the symbol.  */
  HOWTO2 (R_NDS32_25_PLTREL,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 24,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_25_PLTREL",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffff,		/* src_mask  */
	 0xffffff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* This is used only by the dynamic linker.  The symbol should exist
     both in the object being run and in some shared library.  The
     dynamic linker copies the data addressed by the symbol from the
     shared library into the object, because the object being
     run has to have the data at some particular address.  */
  HOWTO2 (R_NDS32_COPY,		/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_COPY",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Like R_NDS32_20, but used when setting global offset table
     entries.  */
  HOWTO2 (R_NDS32_GLOB_DAT,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GLOB_DAT",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Marks a procedure linkage table entry for a symbol.  */
  HOWTO2 (R_NDS32_JMP_SLOT,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_JMP_SLOT",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Used only by the dynamic linker.  When the object is run, this
     longword is set to the load address of the object, plus the
     addend.  */
  HOWTO2 (R_NDS32_RELATIVE,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_RELATIVE",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_GOTOFF,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOTOFF",	/* name  */
	 false,			/* partial_inplace  */
	 0xfffff,		/* src_mask  */
	 0xfffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* An PC Relative 20-bit relocation used when setting PIC offset
     table register.  */
  HOWTO2 (R_NDS32_GOTPC20,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOTPC20",	/* name  */
	 false,			/* partial_inplace  */
	 0xfffff,		/* src_mask  */
	 0xfffff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* Like R_NDS32_HI20, but referring to the GOT table entry for
     the symbol.  */
  HOWTO2 (R_NDS32_GOT_HI20,	/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOT_HI20",	/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_GOT_LO12,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOT_LO12",	/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* An PC Relative relocation used when setting PIC offset table register.
     Like R_NDS32_HI20, but referring to the GOT table entry for
     the symbol.  */
  HOWTO2 (R_NDS32_GOTPC_HI20,	/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOTPC_HI20",	/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  HOWTO2 (R_NDS32_GOTPC_LO12,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOTPC_LO12",	/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  HOWTO2 (R_NDS32_GOTOFF_HI20,	/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOTOFF_HI20",	/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_GOTOFF_LO12,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOTOFF_LO12",	/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Alignment hint for relaxable instruction.  This is used with
     R_NDS32_LABEL as a pair.  Relax this instruction from 4 bytes to 2
     in order to make next label aligned on word boundary.  */
  HOWTO2 (R_NDS32_INSN16,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_INSN16",	/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Alignment hint for label.  */
  HOWTO2 (R_NDS32_LABEL,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LABEL",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for unconditional call sequence  */
  HOWTO2 (R_NDS32_LONGCALL1,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGCALL1",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for conditional call sequence.  */
  HOWTO2 (R_NDS32_LONGCALL2,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGCALL2",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for conditional call sequence.  */
  HOWTO2 (R_NDS32_LONGCALL3,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGCALL3",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for unconditional branch sequence.  */
  HOWTO2 (R_NDS32_LONGJUMP1,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGJUMP1",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for conditional branch sequence.  */
  HOWTO2 (R_NDS32_LONGJUMP2,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGJUMP2",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for conditional branch sequence.  */
  HOWTO2 (R_NDS32_LONGJUMP3,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGJUMP3",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for load/store sequence.   */
  HOWTO2 (R_NDS32_LOADSTORE,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LOADSTORE",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for load/store sequence.  */
  HOWTO2 (R_NDS32_9_FIXED_RELA,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size  */
	 16,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_9_FIXED_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x000000ff,		/* src_mask  */
	 0x000000ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for load/store sequence.  */
  HOWTO2 (R_NDS32_15_FIXED_RELA,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_15_FIXED_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x00003fff,		/* src_mask  */
	 0x00003fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for load/store sequence.  */
  HOWTO2 (R_NDS32_17_FIXED_RELA,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_17_FIXED_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x0000ffff,		/* src_mask  */
	 0x0000ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for load/store sequence.  */
  HOWTO2 (R_NDS32_25_FIXED_RELA,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_25_FIXED_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x00ffffff,		/* src_mask  */
	 0x00ffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* High 20 bits of PLT symbol offset relative to PC.  */
  HOWTO2 (R_NDS32_PLTREL_HI20,	/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_PLTREL_HI20",	/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Low 12 bits of PLT symbol offset relative to PC.  */
  HOWTO2 (R_NDS32_PLTREL_LO12,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_PLTREL_LO12",	/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* High 20 bits of PLT symbol offset relative to GOT (GP).  */
  HOWTO2 (R_NDS32_PLT_GOTREL_HI20,	/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_PLT_GOTREL_HI20",/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Low 12 bits of PLT symbol offset relative to GOT (GP).  */
  HOWTO2 (R_NDS32_PLT_GOTREL_LO12,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_PLT_GOTREL_LO12",/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Small data area 12 bits offset.  */
  HOWTO2 (R_NDS32_SDA12S2_DP_RELA,/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA12S2_DP_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Small data area 12 bits offset.  */
  HOWTO2 (R_NDS32_SDA12S2_SP_RELA,/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA12S2_SP_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */
  /* Lower 12 bits of address.  */

  HOWTO2 (R_NDS32_LO12S2_DP_RELA,	/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 10,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_LO12S2_DP_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x000003ff,		/* src_mask  */
	 0x000003ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Lower 12 bits of address.  */
  HOWTO2 (R_NDS32_LO12S2_SP_RELA,/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 10,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_LO12S2_SP_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x000003ff,		/* src_mask  */
	 0x000003ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Lower 12 bits of address.  Special identity for or case.  */
  HOWTO2 (R_NDS32_LO12S0_ORI_RELA,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_LO12S0_ORI_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Small data area 19 bits offset.  */
  HOWTO2 (R_NDS32_SDA16S3_RELA,	/* type  */
	 3,			/* rightshift  */
	 4,			/* size  */
	 16,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA16S3_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x0000ffff,		/* src_mask  */
	 0x0000ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Small data area 15 bits offset.  */
  HOWTO2 (R_NDS32_SDA17S2_RELA,	/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 17,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA17S2_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x0001ffff,		/* src_mask  */
	 0x0001ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_SDA18S1_RELA,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 18,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA18S1_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x0003ffff,		/* src_mask  */
	 0x0003ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_SDA19S0_RELA,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 19,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA19S0_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x0007ffff,		/* src_mask  */
	 0x0007ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */
  HOWTO2 (R_NDS32_DWARF2_OP1_RELA,/* type  */
	 0,			/* rightshift  */
	 1,			/* size  */
	 8,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_DWARF2_OP1_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0xff,			/* src_mask  */
	 0xff,			/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_DWARF2_OP2_RELA,/* type  */
	 0,			/* rightshift  */
	 2,			/* size  */
	 16,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_DWARF2_OP2_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0xffff,		/* src_mask  */
	 0xffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_DWARF2_LEB_RELA,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_DWARF2_LEB_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_UPDATE_TA_RELA,/* type  */
	 0,			/* rightshift  */
	 2,			/* size  */
	 16,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_UPDATE_TA_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0xffff,		/* src_mask  */
	 0xffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Like R_NDS32_PCREL, but referring to the procedure linkage table
     entry for the symbol.  */
  HOWTO2 (R_NDS32_9_PLTREL,	/* type  */
	 1,			/* rightshift  */
	 2,			/* size  */
	 8,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_9_PLTREL",	/* name  */
	 false,			/* partial_inplace  */
	 0xff,			/* src_mask  */
	 0xff,			/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* Low 20 bits of PLT symbol offset relative to GOT (GP).  */
  HOWTO2 (R_NDS32_PLT_GOTREL_LO20,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_PLT_GOTREL_LO20",/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* low 15 bits of PLT symbol offset relative to GOT (GP)  */
  HOWTO2 (R_NDS32_PLT_GOTREL_LO15,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_PLT_GOTREL_LO15",/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Low 19 bits of PLT symbol offset relative to GOT (GP).  */
  HOWTO2 (R_NDS32_PLT_GOTREL_LO19,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 19,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_PLT_GOTREL_LO19",/* name  */
	 false,			/* partial_inplace  */
	 0x0007ffff,		/* src_mask  */
	 0x0007ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_GOT_LO15,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOT_LO15",	/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_GOT_LO19,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 19,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOT_LO19",	/* name  */
	 false,			/* partial_inplace  */
	 0x0007ffff,		/* src_mask  */
	 0x0007ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_GOTOFF_LO15,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOTOFF_LO15",	/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_GOTOFF_LO19,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 19,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOTOFF_LO19",	/* name  */
	 false,			/* partial_inplace  */
	 0x0007ffff,		/* src_mask  */
	 0x0007ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* GOT 15 bits offset.  */
  HOWTO2 (R_NDS32_GOT15S2_RELA,	/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOT15S2_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x00007fff,		/* src_mask  */
	 0x00007fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* GOT 17 bits offset.  */
  HOWTO2 (R_NDS32_GOT17S2_RELA,	/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 17,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_GOT17S2_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x0001ffff,		/* src_mask  */
	 0x0001ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* A 5 bit address.  */
  HOWTO2 (R_NDS32_5_RELA,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size  */
	 5,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_5_RELA",	/* name  */
	 false,			/* partial_inplace  */
	 0x1f,			/* src_mask  */
	 0x1f,			/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_10_UPCREL_RELA,/* type  */
	 1,			/* rightshift  */
	 2,			/* size  */
	 9,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_unsigned,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_10_UPCREL_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x1ff,			/* src_mask  */
	 0x1ff,			/* dst_mask  */
	 true),			/* pcrel_offset  */

  HOWTO2 (R_NDS32_SDA_FP7U2_RELA,/* type  */
	 2,			/* rightshift  */
	 2,			/* size  */
	 7,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_unsigned,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_SDA_FP7U2_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x0000007f,		/* src_mask  */
	 0x0000007f,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_WORD_9_PCREL_RELA,/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 8,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_WORD_9_PCREL_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0xff,			/* src_mask  */
	 0xff,			/* dst_mask  */
	 true),			/* pcrel_offset  */

  HOWTO2 (R_NDS32_25_ABS_RELA,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 24,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_25_ABS_RELA",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffff,		/* src_mask  */
	 0xffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* A relative 17 bit relocation for ifc, right shifted by 1.  */
  HOWTO2 (R_NDS32_17IFC_PCREL_RELA,/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 16,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_17IFC_PCREL_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0xffff,		/* src_mask  */
	 0xffff,		/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* A relative unsigned 10 bit relocation for ifc, right shifted by 1.  */
  HOWTO2 (R_NDS32_10IFCU_PCREL_RELA,/* type  */
	 1,			/* rightshift  */
	 2,			/* size  */
	 9,			/* bitsize  */
	 true,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_unsigned,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_10IFCU_PCREL_RELA",/* name  */
	 false,			/* partial_inplace  */
	 0x1ff,			/* src_mask  */
	 0x1ff,			/* dst_mask  */
	 true),			/* pcrel_offset  */

  /* Like R_NDS32_HI20, but referring to the TLS LE entry for the symbol.  */
  HOWTO2 (R_NDS32_TLS_LE_HI20,	/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_LE_HI20",	/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_TLS_LE_LO12,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_LE_LO12",	/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Like R_NDS32_HI20, but referring to the TLS IE entry for the symbol.  */
  HOWTO2 (R_NDS32_TLS_IE_HI20,	/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_IE_HI20",	/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_TLS_IE_LO12S2,/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 10,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_IE_LO12S2",/* name  */
	 false,			/* partial_inplace  */
	 0x000003ff,		/* src_mask  */
	 0x000003ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS LE TP offset relocation  */
  HOWTO2 (R_NDS32_TLS_TPOFF,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_TPOFF",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* A 20 bit address.  */
  HOWTO2 (R_NDS32_TLS_LE_20,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_LE_20",	/* name  */
	 false,			/* partial_inplace  */
	 0xfffff,		/* src_mask  */
	 0xfffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_TLS_LE_15S0,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_LE_15S0",	/* name  */
	 false,			/* partial_inplace  */
	 0x7fff,		/* src_mask  */
	 0x7fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_TLS_LE_15S1,	/* type  */
	 1,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_LE_15S1",	/* name  */
	 false,			/* partial_inplace  */
	 0x7fff,		/* src_mask  */
	 0x7fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_TLS_LE_15S2,	/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 15,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_LE_15S2",	/* name  */
	 false,			/* partial_inplace  */
	 0x7fff,		/* src_mask  */
	 0x7fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for unconditional call sequence  */
  HOWTO2 (R_NDS32_LONGCALL4,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGCALL4",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for conditional call sequence.  */
  HOWTO2 (R_NDS32_LONGCALL5,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGCALL5",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for conditional call sequence.  */
  HOWTO2 (R_NDS32_LONGCALL6,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGCALL6",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for unconditional branch sequence.  */
  HOWTO2 (R_NDS32_LONGJUMP4,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGJUMP4",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for conditional branch sequence.  */
  HOWTO2 (R_NDS32_LONGJUMP5,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGJUMP5",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for conditional branch sequence.  */
  HOWTO2 (R_NDS32_LONGJUMP6,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGJUMP6",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Relax hint for conditional branch sequence.  */
  HOWTO2 (R_NDS32_LONGJUMP7,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LONGJUMP7",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  EMPTY_HOWTO (114),

  HOWTO2 (R_NDS32_TLS_IE_LO12,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_IE_LO12",	/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* Like R_NDS32_HI20, but referring to the TLS IE (PIE)
     entry for the symbol.  */
  HOWTO2 (R_NDS32_TLS_IEGP_HI20,/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_IEGP_HI20",/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_TLS_IEGP_LO12,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_IEGP_LO12",/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO2 (R_NDS32_TLS_IEGP_LO12S2,/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 10,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_IEGP_LO12S2",/* name  */
	 false,			/* partial_inplace  */
	 0x000003ff,		/* src_mask  */
	 0x000003ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS description relocation  */
  HOWTO2 (R_NDS32_TLS_DESC,	/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_hi20_reloc,	/* special_function  */
	 "R_NDS32_TLS_DESC_HI20",/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS GD/LD description offset high part.  */
  HOWTO2 (R_NDS32_TLS_DESC_HI20,/* type  */
	 12,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_hi20_reloc,	/* special_function  */
	 "R_NDS32_TLS_DESC_HI20",/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS GD/LD description offset low part.  */
  HOWTO2 (R_NDS32_TLS_DESC_LO12,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 12,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_lo12_reloc,	/* special_function  */
	 "R_NDS32_TLS_DESC_LO12",/* name  */
	 false,			/* partial_inplace  */
	 0x00000fff,		/* src_mask  */
	 0x00000fff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS GD/LD description offset set (movi).  */
  HOWTO2 (R_NDS32_TLS_DESC_20,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 20,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_DESC_20",	/* name  */
	 false,			/* partial_inplace  */
	 0x000fffff,		/* src_mask  */
	 0x000fffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS GD/LD description offset set (lwi.gp).  */
  HOWTO2 (R_NDS32_TLS_DESC_SDA17S2,/* type  */
	 2,			/* rightshift  */
	 4,			/* size  */
	 17,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed,/* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_NDS32_TLS_DESC_SDA17S2",/* name  */
	 false,			/* partial_inplace  */
	 0x0001ffff,		/* src_mask  */
	 0x0001ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */
};

/* Relocations used for relaxation.  */
#define HOWTO3(C, R, S, B, P, BI, O, SF, NAME, INPLACE, MASKSRC, MASKDST, PC) \
  [C-R_NDS32_RELAX_ENTRY] = HOWTO(C, R, S, B, P, BI, O, SF, NAME, INPLACE, MASKSRC, MASKDST, PC)

static reloc_howto_type nds32_elf_relax_howto_table[] = {
  HOWTO3 (R_NDS32_RELAX_ENTRY,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_RELAX_ENTRY",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_GOT_SUFF,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_GOT_SUFF",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_GOTOFF_SUFF,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_GOTOFF_SUFF",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_PLT_GOT_SUFF,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_PLT_GOT_SUFF",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_MULCALL_SUFF,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_MULCALL_SUFF",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_PTR,		/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_PTR",		/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_PTR_COUNT,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_PTR_COUNT",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_PTR_RESOLVED,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_PTR_RESOLVED",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_PLTBLOCK,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_PLTBLOCK",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_RELAX_REGION_BEGIN,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_RELAX_REGION_BEGIN",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_RELAX_REGION_END,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_RELAX_REGION_END",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_MINUEND,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_MINUEND",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_SUBTRAHEND,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_SUBTRAHEND",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_DIFF8,	/* type  */
	 0,			/* rightshift  */
	 1,			/* size  */
	 8,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_DIFF8",	/* name  */
	 false,			/* partial_inplace  */
	 0x000000ff,		/* src_mask  */
	 0x000000ff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_DIFF16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size  */
	 16,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_DIFF16",	/* name  */
	 false,			/* partial_inplace  */
	 0x0000ffff,		/* src_mask  */
	 0x0000ffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_DIFF32,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_DIFF32",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_DIFF_ULEB128,	/* type  */
	 0,			/* rightshift  */
	 1,			/* size  */
	 0,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_DIFF_ULEB128",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_DATA,		/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_DATA",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_TRAN,		/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_TRAN",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_TLS_LE_ADD,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_TLS_LE_ADD",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_TLS_LE_LS,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_TLS_LE_LS",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_EMPTY,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_EMPTY",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS GD/LD description address base addition.  */
  HOWTO3 (R_NDS32_TLS_DESC_ADD,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_TLS_DESC_ADD",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS GD/LD description function load.  */
  HOWTO3 (R_NDS32_TLS_DESC_FUNC,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_TLS_DESC_FUNC",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS DESC resolve function call.  */
  HOWTO3 (R_NDS32_TLS_DESC_CALL,/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_TLS_DESC_CALL",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS DESC variable access.  */
  HOWTO3 (R_NDS32_TLS_DESC_MEM,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_TLS_DESC_MEM",/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS GD/LD description mark (@tlsdec).  */
  HOWTO3 (R_NDS32_RELAX_REMOVE,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_REMOVE",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* TLS GD/LD description mark (@tlsdec).  */
  HOWTO3 (R_NDS32_RELAX_GROUP,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_GROUP",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  HOWTO3 (R_NDS32_TLS_IEGP_LW,	/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_TLS_IEGP_LW",	/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),		/* pcrel_offset  */

  /* LA and FLSI relaxation.  */
  HOWTO3 (R_NDS32_LSI,		/* type  */
	 0,			/* rightshift  */
	 4,			/* size  */
	 32,			/* bitsize  */
	 false,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont,/* complain_on_overflow  */
	 nds32_elf_ignore_reloc,/* special_function  */
	 "R_NDS32_LSI",		/* name  */
	 false,			/* partial_inplace  */
	 0xffffffff,		/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 false),
};

static unsigned long dl_tlsdesc_lazy_trampoline[] =
{
  0x46200000,			/* sethi $r2,#0x0      */
  0x58210000,			/* ori $r2,$r2,#0x0    */
  0x40217400,			/* add $r2,$r2,$gp     */
  0x04210000,			/* lwi $r2,[$r2+#0x0]  */
  0x46300000,			/* sethi $r3,#0x0      */
  0x58318000,			/* ori $r3,$r3,#0x0    */
  0x4031f400,			/* add $r3,$r3,$gp     */
  0x4a000800,			/* jr $r2              */
};

static void
nds32_put_trampoline (void *contents, const unsigned long *template,
		      unsigned count)
{
  unsigned ix;

  for (ix = 0; ix != count; ix++)
    {
      unsigned long insn = template[ix];
      bfd_putb32 (insn, (char *) contents + ix * 4);
    }
}

/* nds32_insertion_sort sorts an array with nmemb elements of size size.
   This prototype is the same as qsort ().  */

static void
nds32_insertion_sort (void *base, size_t nmemb, size_t size,
		      int (*compar) (const void *lhs, const void *rhs))
{
  char *ptr = (char *) base;
  int i, j;
  char tmp[sizeof (Elf_Internal_Rela)];

  BFD_ASSERT (size <= sizeof (tmp));

  /* If i is less than j, i is inserted before j.

     |---- j ----- i --------------|
      \		 / \		  /
	 sorted		unsorted
   */

  for (i = 1; i < (int) nmemb; i++)
    {
      for (j = (i - 1); j >= 0; j--)
	if (compar (ptr + i * size, ptr + j * size) >= 0)
	  break;

      j++;

      if (i == j)
	continue; /* i is in order.  */

      memcpy (tmp, ptr + i * size, size);
      memmove (ptr + (j + 1) * size, ptr + j * size, (i - j) * size);
      memcpy (ptr + j * size, tmp, size);
    }
}

/* Sort relocation by r_offset.

   We didn't use qsort () in stdlib, because quick-sort is not a stable sorting
   algorithm.  Relocations at the same r_offset must keep their order.
   For example, RELAX_ENTRY must be the very first relocation entry.

   Currently, this function implements insertion-sort.

   FIXME: If we already sort them in assembler, why bother sort them
	  here again?  */

static int
compar_reloc (const void *lhs, const void *rhs)
{
  const Elf_Internal_Rela *l = (const Elf_Internal_Rela *) lhs;
  const Elf_Internal_Rela *r = (const Elf_Internal_Rela *) rhs;

  if (l->r_offset > r->r_offset)
    return 1;
  else if (l->r_offset == r->r_offset)
    return 0;
  else
    return -1;
}

/* Functions listed below are only used for old relocs.
     nds32_elf_9_pcrel_reloc
     nds32_elf_do_9_pcrel_reloc
     nds32_elf_hi20_reloc
     nds32_elf_relocate_hi20
     nds32_elf_lo12_reloc
     nds32_elf_sda15_reloc
     nds32_elf_generic_reloc.  */

/* Handle the R_NDS32_9_PCREL & R_NDS32_9_PCREL_RELA reloc.  */

static bfd_reloc_status_type
nds32_elf_9_pcrel_reloc (bfd *       abfd,
			 arelent *   reloc_entry,
			 asymbol *   symbol,
			 void *      data,
			 asection *  input_section,
			 bfd *       output_bfd,
			 char **     error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != (bfd *) NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    {
      /* FIXME: See bfd_perform_relocation.  Is this right?  */
      return bfd_reloc_continue;
    }

  return nds32_elf_do_9_pcrel_reloc (abfd, reloc_entry->howto,
				     input_section,
				     data, reloc_entry->address,
				     symbol->section,
				     (symbol->value
				      + symbol->section->output_section->vma
				      + symbol->section->output_offset),
				     reloc_entry->addend);
}

/* Utility to actually perform an R_NDS32_9_PCREL reloc.  */
#define N_ONES(n) (((((bfd_vma) 1 << ((n) - 1)) - 1) << 1) | 1)

static bfd_reloc_status_type
nds32_elf_do_9_pcrel_reloc (bfd *               abfd,
			    reloc_howto_type *  howto,
			    asection *          input_section,
			    bfd_byte *          data,
			    bfd_vma             offset,
			    asection *          symbol_section ATTRIBUTE_UNUSED,
			    bfd_vma             symbol_value,
			    bfd_vma             addend)
{
  bfd_signed_vma relocation;
  unsigned short x;
  bfd_reloc_status_type status;

  /* Sanity check the address (offset in section).  */
  if (offset > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  relocation = symbol_value + addend;
  /* Make it pc relative.  */
  relocation -= (input_section->output_section->vma
		 + input_section->output_offset);
  /* These jumps mask off the lower two bits of the current address
     before doing pcrel calculations.  */
  relocation -= (offset & -(bfd_vma) 2);

  if (relocation < -ACCURATE_8BIT_S1 || relocation >= ACCURATE_8BIT_S1)
    status = bfd_reloc_overflow;
  else
    status = bfd_reloc_ok;

  x = bfd_getb16 (data + offset);

  relocation >>= howto->rightshift;
  relocation <<= howto->bitpos;
  x = (x & ~howto->dst_mask)
      | (((x & howto->src_mask) + relocation) & howto->dst_mask);

  bfd_putb16 ((bfd_vma) x, data + offset);

  return status;
}

/* Handle the R_NDS32_HI20_[SU]LO relocs.
   HI20_SLO is for the add3 and load/store with displacement instructions.
   HI20 is for the or3 instruction.
   For R_NDS32_HI20_SLO, the lower 16 bits are sign extended when added to
   the high 16 bytes so if the lower 16 bits are negative (bit 15 == 1) then
   we must add one to the high 16 bytes (which will get subtracted off when
   the low 16 bits are added).
   These relocs have to be done in combination with an R_NDS32_LO12 reloc
   because there is a carry from the LO12 to the HI20.  Here we just save
   the information we need; we do the actual relocation when we see the LO12.
   This code is copied from the elf32-mips.c.  We also support an arbitrary
   number of HI20 relocs to be associated with a single LO12 reloc.  The
   assembler sorts the relocs to ensure each HI20 immediately precedes its
   LO12.  However if there are multiple copies, the assembler may not find
   the real LO12 so it picks the first one it finds.  */

struct nds32_hi20
{
  struct nds32_hi20 *next;
  bfd_byte *addr;
  bfd_vma addend;
};

static struct nds32_hi20 *nds32_hi20_list;

static bfd_reloc_status_type
nds32_elf_hi20_reloc (bfd *abfd ATTRIBUTE_UNUSED,
		      arelent *reloc_entry,
		      asymbol *symbol,
		      void *data,
		      asection *input_section,
		      bfd *output_bfd,
		      char **error_message ATTRIBUTE_UNUSED)
{
  bfd_reloc_status_type ret;
  bfd_vma relocation;
  struct nds32_hi20 *n;

  /* This part is from bfd_elf_generic_reloc.
     If we're relocating, and this an external symbol, we don't want
     to change anything.  */
  if (output_bfd != (bfd *) NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0 && reloc_entry->addend == 0)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  /* Sanity check the address (offset in section).  */
  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  ret = bfd_reloc_ok;
  if (bfd_is_und_section (symbol->section) && output_bfd == (bfd *) NULL)
    ret = bfd_reloc_undefined;

  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  relocation += symbol->section->output_section->vma;
  relocation += symbol->section->output_offset;
  relocation += reloc_entry->addend;

  /* Save the information, and let LO12 do the actual relocation.  */
  n = (struct nds32_hi20 *) bfd_malloc ((bfd_size_type) sizeof *n);
  if (n == NULL)
    return bfd_reloc_outofrange;

  n->addr = (bfd_byte *) data + reloc_entry->address;
  n->addend = relocation;
  n->next = nds32_hi20_list;
  nds32_hi20_list = n;

  if (output_bfd != (bfd *) NULL)
    reloc_entry->address += input_section->output_offset;

  return ret;
}

/* Handle an NDS32 ELF HI20 reloc.  */

static void
nds32_elf_relocate_hi20 (bfd *input_bfd ATTRIBUTE_UNUSED,
			 int type ATTRIBUTE_UNUSED,
			 Elf_Internal_Rela *relhi,
			 Elf_Internal_Rela *rello,
			 bfd_byte *contents,
			 bfd_vma addend)
{
  unsigned long insn;
  bfd_vma addlo;

  insn = bfd_getb32 (contents + relhi->r_offset);

  addlo = bfd_getb32 (contents + rello->r_offset);
  addlo &= 0xfff;

  addend += ((insn & 0xfffff) << 20) + addlo;

  insn = (insn & 0xfff00000) | ((addend >> 12) & 0xfffff);
  bfd_putb32 (insn, contents + relhi->r_offset);
}

/* Do an R_NDS32_LO12 relocation.  This is a straightforward 12 bit
   inplace relocation; this function exists in order to do the
   R_NDS32_HI20_[SU]LO relocation described above.  */

static bfd_reloc_status_type
nds32_elf_lo12_reloc (bfd *input_bfd, arelent *reloc_entry, asymbol *symbol,
		      void *data, asection *input_section, bfd *output_bfd,
		      char **error_message)
{
  /* This part is from bfd_elf_generic_reloc.
     If we're relocating, and this an external symbol, we don't want
     to change anything.  */
  if (output_bfd != NULL && (symbol->flags & BSF_SECTION_SYM) == 0
      && reloc_entry->addend == 0)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (nds32_hi20_list != NULL)
    {
      struct nds32_hi20 *l;

      l = nds32_hi20_list;
      while (l != NULL)
	{
	  unsigned long insn;
	  unsigned long val;
	  unsigned long vallo;
	  struct nds32_hi20 *next;

	  /* Do the HI20 relocation.  Note that we actually don't need
	     to know anything about the LO12 itself, except where to
	     find the low 12 bits of the addend needed by the LO12.  */
	  insn = bfd_getb32 (l->addr);
	  vallo = bfd_getb32 ((bfd_byte *) data + reloc_entry->address);
	  vallo &= 0xfff;
	  switch (reloc_entry->howto->type)
	    {
	    case R_NDS32_LO12S3:
	      vallo <<= 3;
	      break;

	    case R_NDS32_LO12S2:
	      vallo <<= 2;
	      break;

	    case R_NDS32_LO12S1:
	      vallo <<= 1;
	      break;

	    case R_NDS32_LO12S0:
	      vallo <<= 0;
	      break;
	    }

	  val = ((insn & 0xfffff) << 12) + vallo;
	  val += l->addend;

	  insn = (insn & ~(bfd_vma) 0xfffff) | ((val >> 12) & 0xfffff);
	  bfd_putb32 ((bfd_vma) insn, l->addr);

	  next = l->next;
	  free (l);
	  l = next;
	}

      nds32_hi20_list = NULL;
    }

  /* Now do the LO12 reloc in the usual way.
     ??? It would be nice to call bfd_elf_generic_reloc here,
     but we have partial_inplace set.  bfd_elf_generic_reloc will
     pass the handling back to bfd_install_relocation which will install
     a section relative addend which is wrong.  */
  return nds32_elf_generic_reloc (input_bfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);
}

/* Do generic partial_inplace relocation.
   This is a local replacement for bfd_elf_generic_reloc.  */

static bfd_reloc_status_type
nds32_elf_generic_reloc (bfd *input_bfd, arelent *reloc_entry,
			 asymbol *symbol, void *data, asection *input_section,
			 bfd *output_bfd, char **error_message ATTRIBUTE_UNUSED)
{
  bfd_reloc_status_type ret;
  bfd_vma relocation;
  bfd_byte *inplace_address;

  /* This part is from bfd_elf_generic_reloc.
     If we're relocating, and this an external symbol, we don't want
     to change anything.  */
  if (output_bfd != NULL && (symbol->flags & BSF_SECTION_SYM) == 0
      && reloc_entry->addend == 0)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  /* Now do the reloc in the usual way.
     ??? It would be nice to call bfd_elf_generic_reloc here,
     but we have partial_inplace set.  bfd_elf_generic_reloc will
     pass the handling back to bfd_install_relocation which will install
     a section relative addend which is wrong.  */

  /* Sanity check the address (offset in section).  */
  if (reloc_entry->address > bfd_get_section_limit (input_bfd, input_section))
    return bfd_reloc_outofrange;

  ret = bfd_reloc_ok;
  if (bfd_is_und_section (symbol->section) && output_bfd == (bfd *) NULL)
    ret = bfd_reloc_undefined;

  if (bfd_is_com_section (symbol->section) || output_bfd != (bfd *) NULL)
    relocation = 0;
  else
    relocation = symbol->value;

  /* Only do this for a final link.  */
  if (output_bfd == (bfd *) NULL)
    {
      relocation += symbol->section->output_section->vma;
      relocation += symbol->section->output_offset;
    }

  relocation += reloc_entry->addend;
  switch (reloc_entry->howto->type)
    {
    case R_NDS32_LO12S3:
      relocation >>= 3;
      break;

    case R_NDS32_LO12S2:
      relocation >>= 2;
      break;

    case R_NDS32_LO12S1:
      relocation >>= 1;
      break;

    case R_NDS32_LO12S0:
    default:
      relocation >>= 0;
      break;
    }

  inplace_address = (bfd_byte *) data + reloc_entry->address;

#define DOIT(x)						\
  x = ((x & ~reloc_entry->howto->dst_mask) |		\
  (((x & reloc_entry->howto->src_mask) +  relocation) &	\
  reloc_entry->howto->dst_mask))

  switch (bfd_get_reloc_size (reloc_entry->howto))
    {
    case 2:
      {
	short x = bfd_getb16 (inplace_address);

	DOIT (x);
	bfd_putb16 ((bfd_vma) x, inplace_address);
      }
      break;
    case 4:
      {
	unsigned long x = bfd_getb32 (inplace_address);

	DOIT (x);
	bfd_putb32 ((bfd_vma) x, inplace_address);
      }
      break;
    default:
      BFD_ASSERT (0);
    }

  if (output_bfd != (bfd *) NULL)
    reloc_entry->address += input_section->output_offset;

  return ret;
}

/* Handle the R_NDS32_SDA15 reloc.
   This reloc is used to compute the address of objects in the small data area
   and to perform loads and stores from that area.
   The lower 15 bits are sign extended and added to the register specified
   in the instruction, which is assumed to point to _SDA_BASE_.

   Since the lower 15 bits offset is left-shifted 0, 1 or 2 bits depending on
   the access size, this must be taken care of.  */

static bfd_reloc_status_type
nds32_elf_sda15_reloc (bfd *abfd ATTRIBUTE_UNUSED, arelent *reloc_entry,
		       asymbol *symbol, void *data ATTRIBUTE_UNUSED,
		       asection *input_section, bfd *output_bfd,
		       char **error_message ATTRIBUTE_UNUSED)
{
  /* This part is from bfd_elf_generic_reloc.  */
  if (output_bfd != (bfd *) NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    {
      /* FIXME: See bfd_perform_relocation.  Is this right?  */
      return bfd_reloc_continue;
    }

  /* FIXME: not sure what to do here yet.  But then again, the linker
     may never call us.  */
  abort ();
}

/* nds32_elf_ignore_reloc is the special function for
   relocation types which don't need to be relocated
   like relaxation relocation types.
   This function simply return bfd_reloc_ok when it is
   invoked.  */

static bfd_reloc_status_type
nds32_elf_ignore_reloc (bfd *abfd ATTRIBUTE_UNUSED, arelent *reloc_entry,
			asymbol *symbol ATTRIBUTE_UNUSED,
			void *data ATTRIBUTE_UNUSED, asection *input_section,
			bfd *output_bfd, char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;

  return bfd_reloc_ok;
}


/* Map BFD reloc types to NDS32 ELF reloc types.  */

struct nds32_reloc_map_entry
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct nds32_reloc_map_entry nds32_reloc_map[] =
{
  {BFD_RELOC_NONE, R_NDS32_NONE},
  {BFD_RELOC_16, R_NDS32_16_RELA},
  {BFD_RELOC_32, R_NDS32_32_RELA},
  {BFD_RELOC_VTABLE_INHERIT, R_NDS32_RELA_GNU_VTINHERIT},
  {BFD_RELOC_VTABLE_ENTRY, R_NDS32_RELA_GNU_VTENTRY},

  {BFD_RELOC_NDS32_20, R_NDS32_20_RELA},
  {BFD_RELOC_NDS32_9_PCREL, R_NDS32_9_PCREL_RELA},
  {BFD_RELOC_NDS32_WORD_9_PCREL, R_NDS32_WORD_9_PCREL_RELA},
  {BFD_RELOC_NDS32_15_PCREL, R_NDS32_15_PCREL_RELA},
  {BFD_RELOC_NDS32_17_PCREL, R_NDS32_17_PCREL_RELA},
  {BFD_RELOC_NDS32_25_PCREL, R_NDS32_25_PCREL_RELA},
  {BFD_RELOC_NDS32_HI20, R_NDS32_HI20_RELA},
  {BFD_RELOC_NDS32_LO12S3, R_NDS32_LO12S3_RELA},
  {BFD_RELOC_NDS32_LO12S2, R_NDS32_LO12S2_RELA},
  {BFD_RELOC_NDS32_LO12S1, R_NDS32_LO12S1_RELA},
  {BFD_RELOC_NDS32_LO12S0, R_NDS32_LO12S0_RELA},
  {BFD_RELOC_NDS32_LO12S0_ORI, R_NDS32_LO12S0_ORI_RELA},
  {BFD_RELOC_NDS32_SDA15S3, R_NDS32_SDA15S3_RELA},
  {BFD_RELOC_NDS32_SDA15S2, R_NDS32_SDA15S2_RELA},
  {BFD_RELOC_NDS32_SDA15S1, R_NDS32_SDA15S1_RELA},
  {BFD_RELOC_NDS32_SDA15S0, R_NDS32_SDA15S0_RELA},
  {BFD_RELOC_NDS32_SDA16S3, R_NDS32_SDA16S3_RELA},
  {BFD_RELOC_NDS32_SDA17S2, R_NDS32_SDA17S2_RELA},
  {BFD_RELOC_NDS32_SDA18S1, R_NDS32_SDA18S1_RELA},
  {BFD_RELOC_NDS32_SDA19S0, R_NDS32_SDA19S0_RELA},
  {BFD_RELOC_NDS32_GOT20, R_NDS32_GOT20},
  {BFD_RELOC_NDS32_9_PLTREL, R_NDS32_9_PLTREL},
  {BFD_RELOC_NDS32_25_PLTREL, R_NDS32_25_PLTREL},
  {BFD_RELOC_NDS32_COPY, R_NDS32_COPY},
  {BFD_RELOC_NDS32_GLOB_DAT, R_NDS32_GLOB_DAT},
  {BFD_RELOC_NDS32_JMP_SLOT, R_NDS32_JMP_SLOT},
  {BFD_RELOC_NDS32_RELATIVE, R_NDS32_RELATIVE},
  {BFD_RELOC_NDS32_GOTOFF, R_NDS32_GOTOFF},
  {BFD_RELOC_NDS32_GOTOFF_HI20, R_NDS32_GOTOFF_HI20},
  {BFD_RELOC_NDS32_GOTOFF_LO12, R_NDS32_GOTOFF_LO12},
  {BFD_RELOC_NDS32_GOTPC20, R_NDS32_GOTPC20},
  {BFD_RELOC_NDS32_GOT_HI20, R_NDS32_GOT_HI20},
  {BFD_RELOC_NDS32_GOT_LO12, R_NDS32_GOT_LO12},
  {BFD_RELOC_NDS32_GOTPC_HI20, R_NDS32_GOTPC_HI20},
  {BFD_RELOC_NDS32_GOTPC_LO12, R_NDS32_GOTPC_LO12},
  {BFD_RELOC_NDS32_INSN16, R_NDS32_INSN16},
  {BFD_RELOC_NDS32_LABEL, R_NDS32_LABEL},
  {BFD_RELOC_NDS32_LONGCALL1, R_NDS32_LONGCALL1},
  {BFD_RELOC_NDS32_LONGCALL2, R_NDS32_LONGCALL2},
  {BFD_RELOC_NDS32_LONGCALL3, R_NDS32_LONGCALL3},
  {BFD_RELOC_NDS32_LONGJUMP1, R_NDS32_LONGJUMP1},
  {BFD_RELOC_NDS32_LONGJUMP2, R_NDS32_LONGJUMP2},
  {BFD_RELOC_NDS32_LONGJUMP3, R_NDS32_LONGJUMP3},
  {BFD_RELOC_NDS32_LOADSTORE, R_NDS32_LOADSTORE},
  {BFD_RELOC_NDS32_9_FIXED, R_NDS32_9_FIXED_RELA},
  {BFD_RELOC_NDS32_15_FIXED, R_NDS32_15_FIXED_RELA},
  {BFD_RELOC_NDS32_17_FIXED, R_NDS32_17_FIXED_RELA},
  {BFD_RELOC_NDS32_25_FIXED, R_NDS32_25_FIXED_RELA},
  {BFD_RELOC_NDS32_LONGCALL4, R_NDS32_LONGCALL4},
  {BFD_RELOC_NDS32_LONGCALL5, R_NDS32_LONGCALL5},
  {BFD_RELOC_NDS32_LONGCALL6, R_NDS32_LONGCALL6},
  {BFD_RELOC_NDS32_LONGJUMP4, R_NDS32_LONGJUMP4},
  {BFD_RELOC_NDS32_LONGJUMP5, R_NDS32_LONGJUMP5},
  {BFD_RELOC_NDS32_LONGJUMP6, R_NDS32_LONGJUMP6},
  {BFD_RELOC_NDS32_LONGJUMP7, R_NDS32_LONGJUMP7},
  {BFD_RELOC_NDS32_PLTREL_HI20, R_NDS32_PLTREL_HI20},
  {BFD_RELOC_NDS32_PLTREL_LO12, R_NDS32_PLTREL_LO12},
  {BFD_RELOC_NDS32_PLT_GOTREL_HI20, R_NDS32_PLT_GOTREL_HI20},
  {BFD_RELOC_NDS32_PLT_GOTREL_LO12, R_NDS32_PLT_GOTREL_LO12},
  {BFD_RELOC_NDS32_SDA12S2_DP, R_NDS32_SDA12S2_DP_RELA},
  {BFD_RELOC_NDS32_SDA12S2_SP, R_NDS32_SDA12S2_SP_RELA},
  {BFD_RELOC_NDS32_LO12S2_DP, R_NDS32_LO12S2_DP_RELA},
  {BFD_RELOC_NDS32_LO12S2_SP, R_NDS32_LO12S2_SP_RELA},
  {BFD_RELOC_NDS32_DWARF2_OP1, R_NDS32_DWARF2_OP1_RELA},
  {BFD_RELOC_NDS32_DWARF2_OP2, R_NDS32_DWARF2_OP2_RELA},
  {BFD_RELOC_NDS32_DWARF2_LEB, R_NDS32_DWARF2_LEB_RELA},
  {BFD_RELOC_NDS32_UPDATE_TA, R_NDS32_UPDATE_TA_RELA},
  {BFD_RELOC_NDS32_PLT_GOTREL_LO20, R_NDS32_PLT_GOTREL_LO20},
  {BFD_RELOC_NDS32_PLT_GOTREL_LO15, R_NDS32_PLT_GOTREL_LO15},
  {BFD_RELOC_NDS32_PLT_GOTREL_LO19, R_NDS32_PLT_GOTREL_LO19},
  {BFD_RELOC_NDS32_GOT_LO15, R_NDS32_GOT_LO15},
  {BFD_RELOC_NDS32_GOT_LO19, R_NDS32_GOT_LO19},
  {BFD_RELOC_NDS32_GOTOFF_LO15, R_NDS32_GOTOFF_LO15},
  {BFD_RELOC_NDS32_GOTOFF_LO19, R_NDS32_GOTOFF_LO19},
  {BFD_RELOC_NDS32_GOT15S2, R_NDS32_GOT15S2_RELA},
  {BFD_RELOC_NDS32_GOT17S2, R_NDS32_GOT17S2_RELA},
  {BFD_RELOC_NDS32_5, R_NDS32_5_RELA},
  {BFD_RELOC_NDS32_10_UPCREL, R_NDS32_10_UPCREL_RELA},
  {BFD_RELOC_NDS32_SDA_FP7U2_RELA, R_NDS32_SDA_FP7U2_RELA},
  {BFD_RELOC_NDS32_RELAX_ENTRY, R_NDS32_RELAX_ENTRY},
  {BFD_RELOC_NDS32_GOT_SUFF, R_NDS32_GOT_SUFF},
  {BFD_RELOC_NDS32_GOTOFF_SUFF, R_NDS32_GOTOFF_SUFF},
  {BFD_RELOC_NDS32_PLT_GOT_SUFF, R_NDS32_PLT_GOT_SUFF},
  {BFD_RELOC_NDS32_MULCALL_SUFF, R_NDS32_MULCALL_SUFF},
  {BFD_RELOC_NDS32_PTR, R_NDS32_PTR},
  {BFD_RELOC_NDS32_PTR_COUNT, R_NDS32_PTR_COUNT},
  {BFD_RELOC_NDS32_PTR_RESOLVED, R_NDS32_PTR_RESOLVED},
  {BFD_RELOC_NDS32_PLTBLOCK, R_NDS32_PLTBLOCK},
  {BFD_RELOC_NDS32_RELAX_REGION_BEGIN, R_NDS32_RELAX_REGION_BEGIN},
  {BFD_RELOC_NDS32_RELAX_REGION_END, R_NDS32_RELAX_REGION_END},
  {BFD_RELOC_NDS32_MINUEND, R_NDS32_MINUEND},
  {BFD_RELOC_NDS32_SUBTRAHEND, R_NDS32_SUBTRAHEND},
  {BFD_RELOC_NDS32_DIFF8, R_NDS32_DIFF8},
  {BFD_RELOC_NDS32_DIFF16, R_NDS32_DIFF16},
  {BFD_RELOC_NDS32_DIFF32, R_NDS32_DIFF32},
  {BFD_RELOC_NDS32_DIFF_ULEB128, R_NDS32_DIFF_ULEB128},
  {BFD_RELOC_NDS32_EMPTY, R_NDS32_EMPTY},
  {BFD_RELOC_NDS32_25_ABS, R_NDS32_25_ABS_RELA},
  {BFD_RELOC_NDS32_DATA, R_NDS32_DATA},
  {BFD_RELOC_NDS32_TRAN, R_NDS32_TRAN},
  {BFD_RELOC_NDS32_17IFC_PCREL, R_NDS32_17IFC_PCREL_RELA},
  {BFD_RELOC_NDS32_10IFCU_PCREL, R_NDS32_10IFCU_PCREL_RELA},
  /* Not sure.  */
  {BFD_RELOC_NDS32_TPOFF, R_NDS32_TLS_TPOFF},
  /* Missing: BFD_RELOC_NDS32_GOTTPOFF.  */
  {BFD_RELOC_NDS32_TLS_LE_HI20, R_NDS32_TLS_LE_HI20},
  {BFD_RELOC_NDS32_TLS_LE_LO12, R_NDS32_TLS_LE_LO12},
  {BFD_RELOC_NDS32_TLS_LE_20, R_NDS32_TLS_LE_20},
  {BFD_RELOC_NDS32_TLS_LE_15S0, R_NDS32_TLS_LE_15S0},
  {BFD_RELOC_NDS32_TLS_LE_15S1, R_NDS32_TLS_LE_15S1},
  {BFD_RELOC_NDS32_TLS_LE_15S2, R_NDS32_TLS_LE_15S2},
  {BFD_RELOC_NDS32_TLS_LE_ADD, R_NDS32_TLS_LE_ADD},
  {BFD_RELOC_NDS32_TLS_LE_LS, R_NDS32_TLS_LE_LS},
  {BFD_RELOC_NDS32_TLS_IE_HI20, R_NDS32_TLS_IE_HI20},
  {BFD_RELOC_NDS32_TLS_IE_LO12, R_NDS32_TLS_IE_LO12},
  {BFD_RELOC_NDS32_TLS_IE_LO12S2, R_NDS32_TLS_IE_LO12S2},
  {BFD_RELOC_NDS32_TLS_IEGP_HI20, R_NDS32_TLS_IEGP_HI20},
  {BFD_RELOC_NDS32_TLS_IEGP_LO12, R_NDS32_TLS_IEGP_LO12},
  {BFD_RELOC_NDS32_TLS_IEGP_LO12S2, R_NDS32_TLS_IEGP_LO12S2},
  {BFD_RELOC_NDS32_TLS_IEGP_LW, R_NDS32_TLS_IEGP_LW},
  {BFD_RELOC_NDS32_TLS_DESC, R_NDS32_TLS_DESC},
  {BFD_RELOC_NDS32_TLS_DESC_HI20, R_NDS32_TLS_DESC_HI20},
  {BFD_RELOC_NDS32_TLS_DESC_LO12, R_NDS32_TLS_DESC_LO12},
  {BFD_RELOC_NDS32_TLS_DESC_20, R_NDS32_TLS_DESC_20},
  {BFD_RELOC_NDS32_TLS_DESC_SDA17S2, R_NDS32_TLS_DESC_SDA17S2},
  {BFD_RELOC_NDS32_TLS_DESC_ADD, R_NDS32_TLS_DESC_ADD},
  {BFD_RELOC_NDS32_TLS_DESC_FUNC, R_NDS32_TLS_DESC_FUNC},
  {BFD_RELOC_NDS32_TLS_DESC_CALL, R_NDS32_TLS_DESC_CALL},
  {BFD_RELOC_NDS32_TLS_DESC_MEM, R_NDS32_TLS_DESC_MEM},
  {BFD_RELOC_NDS32_REMOVE, R_NDS32_RELAX_REMOVE},
  {BFD_RELOC_NDS32_GROUP, R_NDS32_RELAX_GROUP},
  {BFD_RELOC_NDS32_LSI, R_NDS32_LSI},
};

/* Patch tag.  */

static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (nds32_elf_howto_table); i++)
    if (nds32_elf_howto_table[i].name != NULL
	&& strcasecmp (nds32_elf_howto_table[i].name, r_name) == 0)
      return &nds32_elf_howto_table[i];

  for (i = 0; i < ARRAY_SIZE (nds32_elf_relax_howto_table); i++)
    if (nds32_elf_relax_howto_table[i].name != NULL
	&& strcasecmp (nds32_elf_relax_howto_table[i].name, r_name) == 0)
      return &nds32_elf_relax_howto_table[i];

  return NULL;
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_table_lookup (unsigned int code)
{
  if (code < R_NDS32_RELAX_ENTRY)
    {
      if (code < ARRAY_SIZE (nds32_elf_howto_table))
	return &nds32_elf_howto_table[code];
    }
  else
    {
      if (code - R_NDS32_RELAX_ENTRY < ARRAY_SIZE (nds32_elf_relax_howto_table))
	return &nds32_elf_relax_howto_table[code - R_NDS32_RELAX_ENTRY];
    }
  return NULL;
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (nds32_reloc_map); i++)
    {
      if (nds32_reloc_map[i].bfd_reloc_val == code)
	return bfd_elf32_bfd_reloc_type_table_lookup
	  (nds32_reloc_map[i].elf_reloc_val);
    }

  return NULL;
}

/* Set the howto pointer for an NDS32 ELF reloc.  */

static bool
nds32_info_to_howto_rel (bfd *abfd, arelent *cache_ptr,
			 Elf_Internal_Rela *dst)
{
  unsigned int r_type = ELF32_R_TYPE (dst->r_info);

  cache_ptr->howto = NULL;
  if (r_type <= R_NDS32_GNU_VTENTRY)
    cache_ptr->howto = bfd_elf32_bfd_reloc_type_table_lookup (r_type);
  if (cache_ptr->howto == NULL || cache_ptr->howto->name == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  return true;
}

static bool
nds32_info_to_howto (bfd *abfd, arelent *cache_ptr,
		     Elf_Internal_Rela *dst)
{
  unsigned int r_type = ELF32_R_TYPE (dst->r_info);

  cache_ptr->howto = NULL;
  if (r_type == R_NDS32_NONE
      || r_type > R_NDS32_GNU_VTENTRY)
    cache_ptr->howto = bfd_elf32_bfd_reloc_type_table_lookup (r_type);
  if (cache_ptr->howto == NULL || cache_ptr->howto->name == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  return true;
}

/* Support for core dump NOTE sections.
   Reference to include/linux/elfcore.h in Linux.  */

static bool
nds32_elf_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  switch (note->descsz)
    {
    case 0x114:
      /* Linux/NDS32 32-bit, ABI1.  */

      /* pr_cursig */
      elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

      /* pr_pid */
      elf_tdata (abfd)->core->pid = bfd_get_32 (abfd, note->descdata + 24);

      /* pr_reg */
      offset = 72;
      size = 200;
      break;

    case 0xfc:
      /* Linux/NDS32 32-bit.  */

      /* pr_cursig */
      elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

      /* pr_pid */
      elf_tdata (abfd)->core->pid = bfd_get_32 (abfd, note->descdata + 24);

      /* pr_reg */
      offset = 72;
      size = 176;
      break;

    default:
      return false;
    }

  /* Make a ".reg" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
nds32_elf_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    case 124:
      /* Linux/NDS32.  */

      /* __kernel_uid_t, __kernel_gid_t are short on NDS32 platform.  */
      elf_tdata (abfd)->core->program =
	_bfd_elfcore_strndup (abfd, note->descdata + 28, 16);
      elf_tdata (abfd)->core->command =
	_bfd_elfcore_strndup (abfd, note->descdata + 44, 80);
      break;

    default:
      return false;
    }

  /* Note that for some reason, a spurious space is tacked
     onto the end of the args in some (at least one anyway)
     implementations, so strip it off if it exists.  */
  {
    char *command = elf_tdata (abfd)->core->command;
    int n = strlen (command);

    if (0 < n && command[n - 1] == ' ')
      command[n - 1] = '\0';
  }

  return true;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We must handle the special NDS32 section numbers here.
   We also keep watching for whether we need to create the sdata special
   linker sections.  */

static bool
nds32_elf_add_symbol_hook (bfd *abfd,
			   struct bfd_link_info *info ATTRIBUTE_UNUSED,
			   Elf_Internal_Sym *sym,
			   const char **namep ATTRIBUTE_UNUSED,
			   flagword *flagsp ATTRIBUTE_UNUSED,
			   asection **secp, bfd_vma *valp)
{
  switch (sym->st_shndx)
    {
    case SHN_COMMON:
      /* Common symbols less than the GP size are automatically
	 treated as SHN_MIPS_SCOMMON symbols.  */
      if (sym->st_size > elf_gp_size (abfd)
	  || ELF_ST_TYPE (sym->st_info) == STT_TLS)
	break;

      /* st_value is the alignment constraint.
	 That might be its actual size if it is an array or structure.  */
      switch (sym->st_value)
	{
	case 1:
	  *secp = bfd_make_section_old_way (abfd, ".scommon_b");
	  break;
	case 2:
	  *secp = bfd_make_section_old_way (abfd, ".scommon_h");
	  break;
	case 4:
	  *secp = bfd_make_section_old_way (abfd, ".scommon_w");
	  break;
	case 8:
	  *secp = bfd_make_section_old_way (abfd, ".scommon_d");
	  break;
	default:
	  return true;
	}

      (*secp)->flags |= SEC_IS_COMMON | SEC_SMALL_DATA;
      *valp = sym->st_size;
      break;
    }

  return true;
}

/* This function can figure out the best location for a base register to access
   data relative to this base register
   INPUT:
   sda_d0: size of first DOUBLE WORD data section
   sda_w0: size of first WORD data section
   sda_h0: size of first HALF WORD data section
   sda_b : size of BYTE data section
   sda_hi: size of second HALF WORD data section
   sda_w1: size of second WORD data section
   sda_d1: size of second DOUBLE WORD data section
   OUTPUT:
   offset (always positive) from the beginning of sda_d0 if OK
   a negative error value if fail
   NOTE:
   these 7 sections have to be located back to back if exist
   a pass in 0 value for non-existing section   */

/* Due to the interpretation of simm15 field of load/store depending on
   data accessing size, the organization of base register relative data shall
   like the following figure
   -------------------------------------------
   |  DOUBLE WORD sized data (range +/- 128K)
   -------------------------------------------
   |  WORD sized data (range +/- 64K)
   -------------------------------------------
   |  HALF WORD sized data (range +/- 32K)
   -------------------------------------------
   |  BYTE sized data (range +/- 16K)
   -------------------------------------------
   |  HALF WORD sized data (range +/- 32K)
   -------------------------------------------
   |  WORD sized data (range +/- 64K)
   -------------------------------------------
   |  DOUBLE WORD sized data (range +/- 128K)
   -------------------------------------------
   Its base register shall be set to access these data freely.  */

/* We have to figure out the SDA_BASE value, so that we can adjust the
   symbol value correctly.  We look up the symbol _SDA_BASE_ in the output
   BFD.  If we can't find it, we're stuck.  We cache it in the ELF
   target data.  We don't need to adjust the symbol value for an
   external symbol if we are producing relocatable output.  */

static asection *sda_rela_sec = NULL;

#define SDA_SECTION_NUM 10

static bfd_reloc_status_type
nds32_elf_final_sda_base (bfd *output_bfd,
			  struct bfd_link_info *info,
			  bfd_vma *psb,
			  bool add_symbol)
{
  int relax_fp_as_gp;
  struct elf_nds32_link_hash_table *table;
  struct bfd_link_hash_entry *h, *h2;
  long unsigned int total = 0;
  asection *first = NULL, *final = NULL, *temp;
  bfd_vma sda_base = 0;

  h = bfd_link_hash_lookup (info->hash, "_SDA_BASE_", false, false, true);
  if (!h || (h->type != bfd_link_hash_defined
	     && h->type != bfd_link_hash_defweak))
    {
      /* The first section must be 4-byte aligned to promise _SDA_BASE_ being
	 4 byte-aligned.  Therefore, it has to set the first section ".data"
	 4 byte-aligned.  */
      static const char sec_name[SDA_SECTION_NUM][10] =
	{
	  ".data", ".got", ".sdata_d", ".sdata_w", ".sdata_h", ".sdata_b",
	  ".sbss_b", ".sbss_h", ".sbss_w", ".sbss_d"
	};
      size_t i = 0;

      if (output_bfd->sections == NULL)
	{
	  *psb = elf_gp (output_bfd);
	  return bfd_reloc_ok;
	}

      /* Get the first and final section.  */
      while (i < ARRAY_SIZE (sec_name))
	{
	  temp = bfd_get_section_by_name (output_bfd, sec_name[i]);
	  if (temp && !first && (temp->size != 0 || temp->rawsize != 0))
	    first = temp;
	  if (temp && (temp->size != 0 || temp->rawsize != 0))
	    final = temp;

	  /* Summarize the sections in order to check if joining .bss.  */
	  if (temp && temp->size != 0)
	    total += temp->size;
	  else if (temp && temp->rawsize != 0)
	    total += temp->rawsize;

	  i++;
	}

      /* Check .bss size.  */
      temp = bfd_get_section_by_name (output_bfd, ".bss");
      if (temp)
	{
	  if (temp->size != 0)
	    total += temp->size;
	  else if (temp->rawsize != 0)
	    total += temp->rawsize;

	  if (total < 0x80000)
	    {
	      if (!first && (temp->size != 0 || temp->rawsize != 0))
		first = temp;
	      if ((temp->size != 0 || temp->rawsize != 0))
		final = temp;
	    }
	}

      if (first && final)
	{
	  /* The middle of data region.  */
	  sda_base = final->vma / 2 + final->rawsize / 2 + first->vma / 2;

	  /* Find the section sda_base located.  */
	  i = 0;
	  while (i < ARRAY_SIZE (sec_name))
	    {
	      final = bfd_get_section_by_name (output_bfd, sec_name[i]);
	      if (final && (final->size != 0 || final->rawsize != 0)
		  && sda_base >= final->vma)
		{
		  first = final;
		  i++;
		}
	      else
		break;
	    }
	}
      else
	{
	  /* If there is not any default data section in output bfd, try to find
	     the first data section.  If no data section be found, just simplily
	     choose the first output section.  */
	  temp = output_bfd->sections;
	  while (temp)
	    {
	      if (temp->flags & SEC_ALLOC
		  && (((temp->flags & SEC_DATA)
		       && ((temp->flags & SEC_READONLY) == 0))
		      || (temp->flags & SEC_LOAD) == 0)
		  && (temp->size != 0 || temp->rawsize != 0))
		{
		  if (!first)
		    first = temp;
		  final = temp;
		}
	      temp = temp->next;
	    }

	  /* There is no data or bss section.  */
	  if (!first || (first->size == 0 && first->rawsize == 0))
	    {
	      first = output_bfd->sections;
	      while (first && first->size == 0 && first->rawsize == 0)
		first = first->next;
	    }

	  /* There is no concrete section.  */
	  if (!first)
	    {
	      *psb = elf_gp (output_bfd);
	      return bfd_reloc_ok;
	    }

	  if (final && (final->vma + final->rawsize - first->vma) <= 0x4000)
	    sda_base = final->vma / 2 + final->rawsize / 2 + first->vma / 2;
	  else
	    sda_base = first->vma + 0x2000;
	}

      sda_base -= first->vma;
      sda_base = sda_base & (~7);

      if (!_bfd_generic_link_add_one_symbol
	  (info, output_bfd, "_SDA_BASE_", BSF_GLOBAL | BSF_WEAK, first,
	   (bfd_vma) sda_base, (const char *) NULL, false,
	   get_elf_backend_data (output_bfd)->collect, &h))
	return false;

      sda_rela_sec = first;
    }

  /* Set _FP_BASE_ to _SDA_BASE_.  */
  table = nds32_elf_hash_table (info);
  relax_fp_as_gp = table->relax_fp_as_gp;
  h2 = bfd_link_hash_lookup (info->hash, FP_BASE_NAME, false, false, false);
  /* _SDA_BASE_ is difined in linker script.  */
  if (!first)
    {
      first = h->u.def.section;
      sda_base = h->u.def.value;
    }

  if (relax_fp_as_gp && h2
      && (h2->type == bfd_link_hash_undefweak
	  || h2->type == bfd_link_hash_undefined))
    {
      /* Define a weak FP_BASE_NAME here to prevent the undefined symbol.
	 And set FP equal to SDA_BASE to do relaxation for
	 la $fp, _FP_BASE_.  */
      if (!_bfd_generic_link_add_one_symbol
	  (info, output_bfd, FP_BASE_NAME, BSF_GLOBAL | BSF_WEAK,
	   first, sda_base, (const char *) NULL,
	   false, get_elf_backend_data (output_bfd)->collect, &h2))
	return false;
    }

  if (add_symbol)
    {
      if (h)
	{
	  /* Now set gp.  */
	  elf_gp (output_bfd) = (h->u.def.value
				 + h->u.def.section->output_section->vma
				 + h->u.def.section->output_offset);
	}
      else
	{
	  _bfd_error_handler (_("error: can't find symbol: %s"), "_SDA_BASE_");
	  return bfd_reloc_dangerous;
	}
    }

  *psb = h->u.def.value
    + h->u.def.section->output_section->vma
    + h->u.def.section->output_offset;
  return bfd_reloc_ok;
}


/* Return size of a PLT entry.  */
#define elf_nds32_sizeof_plt(info) PLT_ENTRY_SIZE

/* Create an entry in an nds32 ELF linker hash table.  */

static struct bfd_hash_entry *
nds32_elf_link_hash_newfunc (struct bfd_hash_entry *entry,
			     struct bfd_hash_table *table,
			     const char *string)
{
  struct elf_nds32_link_hash_entry *ret;

  ret = (struct elf_nds32_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = (struct elf_nds32_link_hash_entry *)
       bfd_hash_allocate (table, sizeof (struct elf_nds32_link_hash_entry));

  if (ret == NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = (struct elf_nds32_link_hash_entry *)
    _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret, table, string);

  if (ret != NULL)
    {
      struct elf_nds32_link_hash_entry *eh;

      eh = (struct elf_nds32_link_hash_entry *) ret;
      eh->tls_type = GOT_UNKNOWN;
      eh->offset_to_gp = 0;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Create an nds32 ELF linker hash table.  */

static struct bfd_link_hash_table *
nds32_elf_link_hash_table_create (bfd *abfd)
{
  struct elf_nds32_link_hash_table *ret;

  size_t amt = sizeof (struct elf_nds32_link_hash_table);

  ret = (struct elf_nds32_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  /* Patch tag.  */
  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      nds32_elf_link_hash_newfunc,
				      sizeof (struct elf_nds32_link_hash_entry),
				      NDS32_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->sym_ld_script = NULL;

  return &ret->root.root;
}

/* Create .got, .gotplt, and .rela.got sections in DYNOBJ, and set up
   shortcuts to them in our hash table.  */

static bool
create_got_section (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf_link_hash_table *ehtab;

  if (!_bfd_elf_create_got_section (dynobj, info))
    return false;

  ehtab = elf_hash_table (info);
  ehtab->sgot = bfd_get_section_by_name (dynobj, ".got");
  ehtab->sgotplt = bfd_get_section_by_name (dynobj, ".got.plt");
  if (!ehtab->sgot || !ehtab->sgotplt)
    abort ();

  /* _bfd_elf_create_got_section will create it for us.  */
  ehtab->srelgot = bfd_get_section_by_name (dynobj, ".rela.got");
  if (ehtab->srelgot == NULL
      || !bfd_set_section_flags (ehtab->srelgot,
				 (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
				  | SEC_IN_MEMORY | SEC_LINKER_CREATED
				  | SEC_READONLY))
      || !bfd_set_section_alignment (ehtab->srelgot, 2))
    return false;

  return true;
}

/* Create dynamic sections when linking against a dynamic object.  */

static bool
nds32_elf_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_link_hash_table *ehtab;
  struct elf_nds32_link_hash_table *htab;
  flagword flags, pltflags;
  register asection *s;
  const struct elf_backend_data *bed;
  int ptralign = 2;		/* 32-bit  */
  const char *secname;
  char *relname;
  flagword secflags;
  asection *sec;

  bed = get_elf_backend_data (abfd);
  ehtab = elf_hash_table (info);
  htab = nds32_elf_hash_table (info);

  /* We need to create .plt, .rel[a].plt, .got, .got.plt, .dynbss, and
     .rel[a].bss sections.  */

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED);

  pltflags = flags;
  pltflags |= SEC_CODE;
  if (bed->plt_not_loaded)
    pltflags &= ~(SEC_LOAD | SEC_HAS_CONTENTS);
  if (bed->plt_readonly)
    pltflags |= SEC_READONLY;

  s = bfd_make_section (abfd, ".plt");
  ehtab->splt = s;
  if (s == NULL
      || !bfd_set_section_flags (s, pltflags)
      || !bfd_set_section_alignment (s, bed->plt_alignment))
    return false;

  if (bed->want_plt_sym)
    {
      /* Define the symbol _PROCEDURE_LINKAGE_TABLE_ at the start of the
	 .plt section.  */
      struct bfd_link_hash_entry *bh = NULL;
      struct elf_link_hash_entry *h;

      if (!(_bfd_generic_link_add_one_symbol
	    (info, abfd, "_PROCEDURE_LINKAGE_TABLE_", BSF_GLOBAL, s,
	     (bfd_vma) 0, (const char *) NULL, false,
	     get_elf_backend_data (abfd)->collect, &bh)))
	return false;

      h = (struct elf_link_hash_entry *) bh;
      h->def_regular = 1;
      h->type = STT_OBJECT;

      if (bfd_link_pic (info) && !bfd_elf_link_record_dynamic_symbol (info, h))
	return false;
    }

  s = bfd_make_section (abfd,
			bed->default_use_rela_p ? ".rela.plt" : ".rel.plt");
  ehtab->srelplt = s;
  if (s == NULL
      || !bfd_set_section_flags (s, flags | SEC_READONLY)
      || !bfd_set_section_alignment (s, ptralign))
    return false;

  if (ehtab->sgot == NULL && !create_got_section (abfd, info))
    return false;

  for (sec = abfd->sections; sec; sec = sec->next)
    {
      secflags = bfd_section_flags (sec);
      if ((secflags & (SEC_DATA | SEC_LINKER_CREATED))
	  || ((secflags & SEC_HAS_CONTENTS) != SEC_HAS_CONTENTS))
	continue;
      secname = bfd_section_name (sec);
      relname = (char *) bfd_malloc ((bfd_size_type) strlen (secname) + 6);
      strcpy (relname, ".rela");
      strcat (relname, secname);
      if (bfd_get_section_by_name (abfd, secname))
	continue;
      s = bfd_make_section (abfd, relname);
      if (s == NULL
	  || !bfd_set_section_flags (s, flags | SEC_READONLY)
	  || !bfd_set_section_alignment (s, ptralign))
	return false;
    }

  if (bed->want_dynbss)
    {
      /* The .dynbss section is a place to put symbols which are defined
	 by dynamic objects, are referenced by regular objects, and are
	 not functions.  We must allocate space for them in the process
	 image and use a R_*_COPY reloc to tell the dynamic linker to
	 initialize them at run time.  The linker script puts the .dynbss
	 section into the .bss section of the final image.  */
      s = bfd_make_section (abfd, ".dynbss");
      htab->root.sdynbss = s;
      if (s == NULL
	  || !bfd_set_section_flags (s, SEC_ALLOC | SEC_LINKER_CREATED))
	return false;
      /* The .rel[a].bss section holds copy relocs.  This section is not
	 normally needed.  We need to create it here, though, so that the
	 linker will map it to an output section.  We can't just create it
	 only if we need it, because we will not know whether we need it
	 until we have seen all the input files, and the first time the
	 main linker code calls BFD after examining all the input files
	 (size_dynamic_sections) the input sections have already been
	 mapped to the output sections.  If the section turns out not to
	 be needed, we can discard it later.  We will never need this
	 section when generating a shared object, since they do not use
	 copy relocs.  */
      if (!bfd_link_pic (info))
	{
	  s = bfd_make_section (abfd, (bed->default_use_rela_p
				       ? ".rela.bss" : ".rel.bss"));
	  htab->root.srelbss = s;
	  if (s == NULL
	      || !bfd_set_section_flags (s, flags | SEC_READONLY)
	      || !bfd_set_section_alignment (s, ptralign))
	    return false;
	}
    }

  return true;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */
static void
nds32_elf_copy_indirect_symbol (struct bfd_link_info *info,
				struct elf_link_hash_entry *dir,
				struct elf_link_hash_entry *ind)
{
  struct elf_nds32_link_hash_entry *edir, *eind;

  edir = (struct elf_nds32_link_hash_entry *) dir;
  eind = (struct elf_nds32_link_hash_entry *) ind;

  if (ind->root.type == bfd_link_hash_indirect)
    {
      if (dir->got.refcount <= 0)
	{
	  edir->tls_type = eind->tls_type;
	  eind->tls_type = GOT_UNKNOWN;
	}
    }

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
nds32_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				 struct elf_link_hash_entry *h)
{
  struct elf_nds32_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  unsigned int power_of_two;

  dynobj = elf_hash_table (info)->dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
	      && (h->needs_plt
		  || h->is_weakalias
		  || (h->def_dynamic && h->ref_regular && !h->def_regular)));


  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC || h->needs_plt)
    {
      if (!bfd_link_pic (info)
	  && !h->def_dynamic
	  && !h->ref_dynamic
	  && h->root.type != bfd_link_hash_undefweak
	  && h->root.type != bfd_link_hash_undefined)
	{
	  /* This case can occur if we saw a PLT reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object.  In such a case, we don't actually need to build
	     a procedure linkage table, and we can just do a PCREL
	     reloc instead.  */
	  h->plt.offset = (bfd_vma) - 1;
	  h->needs_plt = 0;
	}

      return true;
    }
  else
    h->plt.offset = (bfd_vma) - 1;

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (h);
      BFD_ASSERT (def->root.type == bfd_link_hash_defined);
      h->root.u.def.section = def->root.u.def.section;
      h->root.u.def.value = def->root.u.def.value;
      return true;
    }

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (bfd_link_pic (info))
    return true;

  /* If there are no references to this symbol that do not use the
     GOT, we don't need to generate a copy reloc.  */
  if (!h->non_got_ref)
    return true;

  /* If -z nocopyreloc was given, we won't generate them either.  */
  if (0 && info->nocopyreloc)
    {
      h->non_got_ref = 0;
      return true;
    }

  /* If we don't find any dynamic relocs in read-only sections, then
     we'll be keeping the dynamic relocs and avoiding the copy reloc.  */
  if (!_bfd_elf_readonly_dynrelocs (h))
    {
      h->non_got_ref = 0;
      return true;
    }

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */

  htab = nds32_elf_hash_table (info);
  s = htab->root.sdynbss;
  BFD_ASSERT (s != NULL);

  /* We must generate a R_NDS32_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      asection *srel;

      srel = htab->root.srelbss;
      BFD_ASSERT (srel != NULL);
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  /* We need to figure out the alignment required for this symbol.  I
     have no idea how ELF linkers handle this.  */
  power_of_two = bfd_log2 (h->size);
  if (power_of_two > 3)
    power_of_two = 3;

  /* Apply the required alignment.  */
  s->size = BFD_ALIGN (s->size, (bfd_size_type) (1 << power_of_two));
  if (power_of_two > bfd_section_alignment (s))
    {
      if (!bfd_set_section_alignment (s, power_of_two))
	return false;
    }

  /* Define the symbol as being at this point in the section.  */
  h->root.u.def.section = s;
  h->root.u.def.value = s->size;

  /* Increment the section size to make room for the symbol.  */
  s->size += h->size;

  return true;
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct elf_link_hash_table *ehtab;
  struct elf_nds32_link_hash_table *htab;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  /* When warning symbols are created, they **replace** the "real"
     entry in the hash table, thus we never get to see the real
     symbol in a hash traversal. So look at it now.  */
  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  info = (struct bfd_link_info *) inf;
  ehtab = elf_hash_table (info);
  htab = nds32_elf_hash_table (info);
  if (htab == NULL)
    return false;

  if ((htab->root.dynamic_sections_created || h->type == STT_GNU_IFUNC)
      && h->plt.refcount > 0
      && !(bfd_link_pie (info) && h->def_regular))
    {
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1 && !h->forced_local)
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, bfd_link_pic (info), h))
	{
	  asection *s = ehtab->splt;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (s->size == 0)
	    s->size += PLT_ENTRY_SIZE;

	  h->plt.offset = s->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (!bfd_link_pic (info) && !h->def_regular)
	    {
	      h->root.u.def.section = s;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry.  */
	  s->size += PLT_ENTRY_SIZE;

	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  ehtab->sgotplt->size += 4;

	  /* We also need to make an entry in the .rel.plt section.  */
	  ehtab->srelplt->size += sizeof (Elf32_External_Rela);
	  if (htab->tls_desc_trampoline)
	    htab->next_tls_desc_index++;
	}
      else
	{
	  h->plt.offset = (bfd_vma) - 1;
	  h->needs_plt = 0;
	}
    }
  else
    {
      h->plt.offset = (bfd_vma) - 1;
      h->needs_plt = 0;
    }

  if (h->got.refcount > 0)
    {
      asection *sgot;
      bool dyn;
      int tls_type = elf32_nds32_hash_entry (h)->tls_type;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1 && !h->forced_local)
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      sgot = elf_hash_table (info)->sgot;
      h->got.offset = sgot->size;

      if (tls_type == GOT_UNKNOWN)
	abort ();

      /* Non-TLS symbols, and TLS_IE need one GOT slot.  */
      if (tls_type & (GOT_NORMAL | GOT_TLS_IE | GOT_TLS_IEGP))
	sgot->size += 4;
      else
	{
	  /* TLS_DESC, TLS_GD, and TLS_LD need 2 consecutive GOT slots.  */
	  if (tls_type & GOT_TLS_DESC)
	    sgot->size += 8;
	}

      dyn = htab->root.dynamic_sections_created;

      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h))
	{
	  if (tls_type == GOT_TLS_DESC && htab->tls_desc_trampoline)
	    {
	      /* TLS_DESC with trampoline needs a relocation slot
		 within .rela.plt.  */
	      htab->num_tls_desc++;
	      ehtab->srelplt->size += sizeof (Elf32_External_Rela);
	      htab->tls_trampoline = -1;
	    }
	  else
	    {
	      /* other relocations, including TLS_DESC without trampoline, need
		 a relocation slot within .rela.got.  */
	      ehtab->srelgot->size += sizeof (Elf32_External_Rela);
	    }
	}
    }
  else
    h->got.offset = (bfd_vma)-1;

  if (h->dyn_relocs == NULL)
    return true;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for pc-relative relocs that have become local due to symbol
     visibility changes.  */

  if (bfd_link_pic (info))
    {
      if (h->def_regular && (h->forced_local || info->symbolic))
	{
	  struct elf_dyn_relocs **pp;

	  for (pp = &h->dyn_relocs; (p = *pp) != NULL;)
	    {
	      p->count -= p->pc_count;
	      p->pc_count = 0;
	      if (p->count == 0)
		*pp = p->next;
	      else
		pp = &p->next;
	    }
	}
    }
  else
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not dynamic.  */

      if (!h->non_got_ref
	  && ((h->def_dynamic
	       && !h->def_regular)
	      || (htab->root.dynamic_sections_created
		  && (h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined))))
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (h->dynindx == -1 && !h->forced_local)
	    {
	      if (!bfd_elf_link_record_dynamic_symbol (info, h))
		return false;
	    }

	  /* If that succeeded, we know we'll be keeping all the
	     relocs.  */
	  if (h->dynindx != -1)
	    goto keep;
	}

      h->dyn_relocs = NULL;

    keep:;
    }

  /* Finally, allocate space.  */
  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;
      sreloc->size += p->count * sizeof (Elf32_External_Rela);
    }

  return true;
}

/* Add relocation REL to the end of relocation section SRELOC.  */

static void
elf32_nds32_add_dynreloc (bfd *output_bfd,
			  struct bfd_link_info *info ATTRIBUTE_UNUSED,
			  asection *sreloc, Elf_Internal_Rela *rel)
{
  bfd_byte *loc;
  if (sreloc == NULL)
    abort ();

  loc = sreloc->contents;
  loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
  if (sreloc->reloc_count * sizeof (Elf32_External_Rela) > sreloc->size)
    abort ();

  bfd_elf32_swap_reloca_out (output_bfd, rel, loc);
}

/* Set the sizes of the dynamic sections.  */

static bool
nds32_elf_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				 struct bfd_link_info *info)
{
  struct elf_nds32_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bool relocs;
  bfd *ibfd;

  htab = nds32_elf_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = elf_hash_table (info)->dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_section_by_name (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *sgot;
      char *local_tls_type;
      unsigned long symndx;
      bfd_vma *local_tlsdesc_gotent;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = ((struct elf_dyn_relocs *)
		    elf_section_data (s)->local_dynrel);
	       p != NULL; p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (p->count != 0)
		{
		  asection *sreloc = elf_section_data (p->sec)->sreloc;
		  sreloc->size += p->count * sizeof (Elf32_External_Rela);
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
	continue;

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      sgot = elf_hash_table (info)->sgot;
      local_tls_type = elf32_nds32_local_got_tls_type (ibfd);
      local_tlsdesc_gotent = elf32_nds32_local_tlsdesc_gotent (ibfd);
      for (symndx = 0; local_got < end_local_got;
	   ++local_got, ++local_tls_type, ++local_tlsdesc_gotent, ++symndx)
	{
	  if (*local_got > 0)
	    {
	      int num_of_got_entry_needed = 0;
	      *local_got = sgot->size;
	      *local_tlsdesc_gotent = sgot->size;

	      /* TLS_NORMAL, and TLS_IE need one slot in .got.  */
	      if (*local_tls_type & (GOT_NORMAL | GOT_TLS_IE | GOT_TLS_IEGP))
		num_of_got_entry_needed = 1;
	      /* TLS_GD, TLS_LD, and TLS_DESC need an 8-byte structure in the GOT.  */
	      else if (*local_tls_type & GOT_TLS_DESC)
		num_of_got_entry_needed = 2;

	      sgot->size += (num_of_got_entry_needed << 2);

	      /* non-relax-able TLS_DESCs need a slot in .rela.plt.
		 others need a slot in .rela.got.  */
	      if (*local_tls_type == GOT_TLS_DESC)
		{
		  if (bfd_link_pic (info))
		    {
		      if (htab->tls_desc_trampoline)
			{
			  htab->num_tls_desc++;
			  htab->root.srelplt->size += sizeof (Elf32_External_Rela);
			  htab->tls_trampoline = -1;
			}
		      else
			htab->root.srelgot->size += sizeof (Elf32_External_Rela);
		    }
		  else
		    {
		      /* TLS_DESC -> TLS_LE  */
		    }
		}
	      else
		{
		  htab->root.srelgot->size += sizeof (Elf32_External_Rela);
		}
	    }
	  else
	    {
	      *local_got = (bfd_vma) -1;
	      *local_tlsdesc_gotent = (bfd_vma) -1;
	    }
	}
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->root, allocate_dynrelocs, (void *) info);

  /* For every jump slot reserved in the sgotplt, reloc_count is
     incremented.  However, when we reserve space for TLS descriptors,
     it's not incremented, so in order to compute the space reserved
     for them, it suffices to multiply the reloc count by the jump
     slot size.  */
  if (htab->tls_desc_trampoline && htab->root.srelplt)
    htab->sgotplt_jump_table_size = elf32_nds32_compute_jump_table_size (htab);

  if (htab->tls_trampoline)
    {
      htab->tls_trampoline = htab->root.splt->size;

      /* If we're not using lazy TLS relocations, don't generate the
	 PLT and GOT entries they require.  */
      if ((info->flags & DF_BIND_NOW))
	htab->root.tlsdesc_plt = 0;
      else
	{
	  htab->root.tlsdesc_got = htab->root.sgot->size;
	  htab->root.sgot->size += 4;

	  htab->root.tlsdesc_plt = htab->root.splt->size;
	  htab->root.splt->size += 4 * ARRAY_SIZE (dl_tlsdesc_lazy_trampoline);
	}
    }

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->root.splt)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	  ;
	}
      else if (s == elf_hash_table (info)->sgot)
	{
	  got_size += s->size;
	}
      else if (s == elf_hash_table (info)->sgotplt)
	{
	  got_size += s->size;
	}
      else if (startswith (bfd_section_name (s), ".rela"))
	{
	  if (s->size != 0 && s != elf_hash_table (info)->srelplt)
	    relocs = true;

	  /* We use the reloc_count field as a counter if we need
	     to copy relocs into the output file.  */
	  s->reloc_count = 0;
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0)
	{
	  /* If we don't need this section, strip it from the
	     output file.  This is mostly to handle .rela.bss and
	     .rela.plt.  We must create both sections in
	     create_dynamic_sections, because they must be created
	     before the linker maps input sections to output
	     sections.  The linker does that before
	     adjust_dynamic_symbol is called, and it is that
	     function which decides whether anything needs to go
	     into these sections.  */
	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      /* Allocate memory for the section contents.  We use bfd_zalloc
	 here in case unused entries are not reclaimed before the
	 section's contents are written out.  This should not happen,
	 but this way if it does, we get a R_NDS32_NONE reloc instead
	 of garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs);
}

static bfd_reloc_status_type
nds32_relocate_contents (reloc_howto_type *howto, bfd *input_bfd,
			 bfd_vma relocation, bfd_byte *location)
{
  int size;
  bfd_vma x = 0;
  bfd_reloc_status_type flag;
  unsigned int rightshift = howto->rightshift;
  unsigned int bitpos = howto->bitpos;

  if (howto->negate)
    relocation = -relocation;

  /* Get the value we are going to relocate.  */
  size = bfd_get_reloc_size (howto);
  switch (size)
    {
    default:
      abort ();
      break;
    case 0:
      return bfd_reloc_ok;
    case 2:
      x = bfd_getb16 (location);
      break;
    case 4:
      x = bfd_getb32 (location);
      break;
    }

  /* Check for overflow.  FIXME: We may drop bits during the addition
     which we don't check for.  We must either check at every single
     operation, which would be tedious, or we must do the computations
     in a type larger than bfd_vma, which would be inefficient.  */
  flag = bfd_reloc_ok;
  if (howto->complain_on_overflow != complain_overflow_dont)
    {
      bfd_vma addrmask, fieldmask, signmask, ss;
      bfd_vma a, b, sum;

      /* Get the values to be added together.  For signed and unsigned
	 relocations, we assume that all values should be truncated to
	 the size of an address.  For bitfields, all the bits matter.
	 See also bfd_check_overflow.  */
      fieldmask = N_ONES (howto->bitsize);
      signmask = ~fieldmask;
      addrmask = N_ONES (bfd_arch_bits_per_address (input_bfd)) | fieldmask;
      a = (relocation & addrmask) >> rightshift;
      b = (x & howto->src_mask & addrmask) >> bitpos;

      switch (howto->complain_on_overflow)
	{
	case complain_overflow_signed:
	  /* If any sign bits are set, all sign bits must be set.
	     That is, A must be a valid negative address after
	     shifting.  */
	  signmask = ~(fieldmask >> 1);
	  /* Fall through.  */

	case complain_overflow_bitfield:
	  /* Much like the signed check, but for a field one bit
	     wider.  We allow a bitfield to represent numbers in the
	     range -2**n to 2**n-1, where n is the number of bits in the
	     field.  Note that when bfd_vma is 32 bits, a 32-bit reloc
	     can't overflow, which is exactly what we want.  */
	  ss = a & signmask;
	  if (ss != 0 && ss != ((addrmask >> rightshift) & signmask))
	    flag = bfd_reloc_overflow;

	  /* We only need this next bit of code if the sign bit of B
	     is below the sign bit of A.  This would only happen if
	     SRC_MASK had fewer bits than BITSIZE.  Note that if
	     SRC_MASK has more bits than BITSIZE, we can get into
	     trouble; we would need to verify that B is in range, as
	     we do for A above.  */
	  ss = ((~howto->src_mask) >> 1) & howto->src_mask;
	  ss >>= bitpos;

	  /* Set all the bits above the sign bit.  */
	  b = (b ^ ss) - ss;

	  /* Now we can do the addition.  */
	  sum = a + b;

	  /* See if the result has the correct sign.  Bits above the
	     sign bit are junk now; ignore them.  If the sum is
	     positive, make sure we did not have all negative inputs;
	     if the sum is negative, make sure we did not have all
	     positive inputs.  The test below looks only at the sign
	     bits, and it really just
	     SIGN (A) == SIGN (B) && SIGN (A) != SIGN (SUM)

	     We mask with addrmask here to explicitly allow an address
	     wrap-around.  The Linux kernel relies on it, and it is
	     the only way to write assembler code which can run when
	     loaded at a location 0x80000000 away from the location at
	     which it is linked.  */
	  if (((~(a ^ b)) & (a ^ sum)) & signmask & addrmask)
	    flag = bfd_reloc_overflow;

	  break;

	case complain_overflow_unsigned:
	  /* Checking for an unsigned overflow is relatively easy:
	     trim the addresses and add, and trim the result as well.
	     Overflow is normally indicated when the result does not
	     fit in the field.  However, we also need to consider the
	     case when, e.g., fieldmask is 0x7fffffff or smaller, an
	     input is 0x80000000, and bfd_vma is only 32 bits; then we
	     will get sum == 0, but there is an overflow, since the
	     inputs did not fit in the field.  Instead of doing a
	     separate test, we can check for this by or-ing in the
	     operands when testing for the sum overflowing its final
	     field.  */
	  sum = (a + b) & addrmask;
	  if ((a | b | sum) & signmask)
	    flag = bfd_reloc_overflow;
	  break;

	default:
	  abort ();
	}
    }

  /* Put RELOCATION in the right bits.  */
  relocation >>= (bfd_vma) rightshift;
  relocation <<= (bfd_vma) bitpos;

  /* Add RELOCATION to the right bits of X.  */
  /* FIXME : 090616
     Because the relaxation may generate duplicate relocation at one address,
     an addition to immediate in the instruction may cause the relocation added
     several times.
     This bug should be fixed in assembler, but a check is also needed here.  */
  if (howto->partial_inplace)
    x = ((x & ~howto->dst_mask)
	 | (((x & howto->src_mask) + relocation) & howto->dst_mask));
  else
    x = ((x & ~howto->dst_mask) | ((relocation) & howto->dst_mask));


  /* Put the relocated value back in the object file.  */
  switch (size)
    {
    default:
    case 0:
    case 1:
    case 8:
      abort ();
      break;
    case 2:
      bfd_putb16 (x, location);
      break;
    case 4:
      bfd_putb32 (x, location);
      break;
    }

  return flag;
}

static bfd_reloc_status_type
nds32_elf_final_link_relocate (reloc_howto_type *howto, bfd *input_bfd,
			       asection *input_section, bfd_byte *contents,
			       bfd_vma address, bfd_vma value, bfd_vma addend)
{
  bfd_vma relocation;

  /* Sanity check the address.  */
  if (address > bfd_get_section_limit (input_bfd, input_section))
    return bfd_reloc_outofrange;

  /* This function assumes that we are dealing with a basic relocation
     against a symbol.  We want to compute the value of the symbol to
     relocate to.  This is just VALUE, the value of the symbol, plus
     ADDEND, any addend associated with the reloc.  */
  relocation = value + addend;

  /* If the relocation is PC relative, we want to set RELOCATION to
     the distance between the symbol (currently in RELOCATION) and the
     location we are relocating.  If pcrel_offset is FALSE we do not
     need to subtract out the offset of the location within the
     section (which is just ADDRESS).  */
  if (howto->pc_relative)
    {
      relocation -= (input_section->output_section->vma
		     + input_section->output_offset);
      if (howto->pcrel_offset)
	relocation -= address;
    }

  return nds32_relocate_contents (howto, input_bfd, relocation,
				  contents + address);
}

static int
nds32_elf_output_symbol_hook (struct bfd_link_info *info,
			      const char *name,
			      Elf_Internal_Sym *elfsym ATTRIBUTE_UNUSED,
			      asection *input_sec,
			      struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  const char *source;
  FILE *sym_ld_script = NULL;
  struct elf_nds32_link_hash_table *table;

  table = nds32_elf_hash_table (info);
  sym_ld_script = table->sym_ld_script;
  if (!sym_ld_script)
    return true;

  if (!h || !name || *name == '\0')
    return true;

  if (input_sec->flags & SEC_EXCLUDE)
    return true;

  if (!check_start_export_sym)
    {
      fprintf (sym_ld_script, "SECTIONS\n{\n");
      check_start_export_sym = 1;
    }

  if (h->root.type == bfd_link_hash_defined
      || h->root.type == bfd_link_hash_defweak)
    {
      if (!h->root.u.def.section->output_section)
	return true;

      if (bfd_is_const_section (input_sec))
	source = input_sec->name;
      else
	source = bfd_get_filename (input_sec->owner);

      fprintf (sym_ld_script, "\t%s = 0x%08lx;\t /* %s */\n",
	       h->root.root.string,
	       (long) (h->root.u.def.value
		+ h->root.u.def.section->output_section->vma
		+ h->root.u.def.section->output_offset), source);
    }

  return true;
}

/* Relocate an NDS32/D ELF section.
   There is some attempt to make this function usable for many architectures,
   both for RELA and REL type relocs, if only to serve as a learning tool.

   The RELOCATE_SECTION function is called by the new ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjust the section contents as
   necessary, and (if using Rela relocs and generating a
   relocatable output file) adjusting the reloc addend as
   necessary.

   This function does not have to worry about setting the reloc
   address or the reloc symbol index.

   LOCAL_SYMS is a pointer to the swapped in local symbols.

   LOCAL_SECTIONS is an array giving the section in the input file
   corresponding to the st_shndx field of each local symbol.

   The global hash table entry for the global symbols can be found
   via elf_sym_hashes (input_bfd).

   When generating relocatable output, this function must handle
   STB_LOCAL/STT_SECTION symbols specially.  The output symbol is
   going to be the section symbol corresponding to the output
   section, which means that the addend must be adjusted
   accordingly.  */

/* Return the base VMA address which should be subtracted from real addresses
   when resolving @dtpoff relocation.
   This is PT_TLS segment p_vaddr.  */

/* Return the relocation value for @tpoff relocation
   if STT_TLS virtual address is ADDRESS.  */

/* Return the relocation value for @gottpoff relocation
   if STT_TLS virtual address is ADDRESS.  */

static bfd_vma
gottpoff (struct bfd_link_info *info, bfd_vma address)
{
  bfd_vma tp_base;
  bfd_vma tp_offset;

  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;

  tp_base = elf_hash_table (info)->tls_sec->vma;
  tp_offset = address - tp_base;

  return tp_offset;
}

static bool
patch_tls_desc_to_ie (bfd_byte *contents, Elf_Internal_Rela *rel, bfd *ibfd)
{
  /* TLS_GD/TLS_LD model #1
     46 00 00 00 sethi $r0,#0x0
     58 00 00 00 ori $r0,$r0,#0x0
     40 00 74 00 add $r0,$r0,$gp
     04 10 00 00 lwi $r1,[$r0+#0x0]
     4b e0 04 01 jral $lp,$r1  */

  /* TLS_GD/TLS_LD model #2
     46 00 00 00 sethi $r0,#0x0
     58 00 00 00 ori $r0,$r0,#0x0
     38 10 74 02 lw $r1,[$r0+($gp<<#0x0)]
     40 00 74 00 add $r0,$r0,$gp
     4b e0 04 01 jral $lp,$r1  */

  /* TLS_IE model (non-PIC)
     46 00 00 00 sethi $r0,#0x0
     04 00 00 00 lwi $r0,[$r0+#0x0]
     38 00 64 02 lw $r0,[$r0+($r25<<#0x0)]  */

  /* TLS_IE model (PIC)
     46 00 00 00 sethi $r0,#0x0
     58 00 00 00 ori $r0,$r0,#0x0
     38 00 74 02 lw $r0,[$r0+($gp<<#0x0)]
     38 00 64 02 lw $r0,[$r0+($r25<<#0x0)]  */

  /* TLS_GD_TO_IE model
     46 00 00 00 sethi $r0,#0x0
     58 00 00 00 ori $r0,$r0,#0x0
     40 00 74 00 add $r0,$rM,$gp
     04 00 00 01 lwi $r0,[$r0+#0x4]
     40 00 64 00 add $r0,$r0,$r25  */

  bool rz = false;

  typedef struct
    {
      uint32_t opcode;
      uint32_t mask;
    } pat_t;

  uint32_t patch[3] =
    {
      0x40007400, /* add $r0,$rM,$gp     */
      0x04000001, /* lwi $r0,[$r0+#0x4]  */
      0x40006400, /* add $r0,$r0,$r25    */
    };

  pat_t mode0[3] =
    {
	{ 0x40000000, 0xfe0003ff },
	{ 0x04000000, 0xfe000000 },
	{ 0x4be00001, 0xffff83ff },
    };

  pat_t mode1[3] =
    {
	{ 0x38007402, 0xfe007fff },
	{ 0x40007400, 0xfe007fff },
	{ 0x4be00001, 0xffff83ff },
    };

  unsigned char *p = contents + rel->r_offset;

  uint32_t insn;
  uint32_t regidx = 0;
  insn = bfd_getb32 (p);
  if (INSN_SETHI == (0xfe0fffffu & insn))
    {
      regidx = 0x1f & (insn >> 20);
      p += 4;
    }

  insn = bfd_getb32 (p);
  if (INSN_ORI == (0xfe007fffu & insn))
    {
      regidx = 0x1f & (insn >> 20);
      p += 4;
    }

  if (patch[2] == bfd_getb32 (p + 8)) /* Character instruction.  */
    {
      /* already patched?  */
      if ((patch[0] == (0xfff07fffu & bfd_getb32 (p + 0))) &&
	  (patch[1] == bfd_getb32 (p + 4)))
	rz = true;
    }
  else if (mode0[0].opcode == (mode0[0].mask & bfd_getb32 (p + 0)))
    {
      if ((mode0[1].opcode == (mode0[1].mask & bfd_getb32 (p + 4))) &&
	  (mode0[2].opcode == (mode0[2].mask & bfd_getb32 (p + 8))))
	{
	  bfd_putb32 (patch[0] | (regidx << 15), p + 0);
	  bfd_putb32 (patch[1], p + 4);
	  bfd_putb32 (patch[2], p + 8);
	  rz = true;
	}
    }
  else if (mode1[0].opcode == (mode1[0].mask & bfd_getb32 (p + 0)))
    {
      if ((mode1[1].opcode == (mode1[1].mask & bfd_getb32 (p + 4))) &&
	  (mode1[2].opcode == (mode1[2].mask & bfd_getb32 (p + 8))))
	{
	  bfd_putb32 (patch[0] | (regidx << 15), p + 0);
	  bfd_putb32 (patch[1], p + 4);
	  bfd_putb32 (patch[2], p + 8);
	  rz = true;
	}
    }

  if (!rz)
    {
      printf ("%s: %s @ 0x%08x\n", __func__, bfd_get_filename (ibfd),
	      (int) rel->r_offset);
      BFD_ASSERT(0); /* Unsupported pattern.  */
    }

  return rz;
}

static enum elf_nds32_tls_type
get_tls_type (enum elf_nds32_reloc_type r_type, struct elf_link_hash_entry *h);

static unsigned int
ones32 (register unsigned int x)
{
  /* 32-bit recursive reduction using SWAR...
     but first step is mapping 2-bit values
     into sum of 2 1-bit values in sneaky way.  */
  x -= ((x >> 1) & 0x55555555);
  x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
  x = (((x >> 4) + x) & 0x0f0f0f0f);
  x += (x >> 8);
  x += (x >> 16);
  return (x & 0x0000003f);
}

#if !HAVE_FLS
static unsigned int
fls (register unsigned int x)
{
  return ffs (x & (-x));
}
#endif /* !HAVE_FLS */

#define nds32_elf_local_tlsdesc_gotent(bfd) \
  (elf_nds32_tdata (bfd)->local_tlsdesc_gotent)

static int
nds32_elf_relocate_section (bfd *		   output_bfd ATTRIBUTE_UNUSED,
			    struct bfd_link_info * info,
			    bfd *		   input_bfd,
			    asection *		   input_section,
			    bfd_byte *		   contents,
			    Elf_Internal_Rela *	   relocs,
			    Elf_Internal_Sym *	   local_syms,
			    asection **		   local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel, *relend;
  bool ret = true;		/* Assume success.  */
  int align = 0;
  bfd_reloc_status_type r;
  const char *errmsg = NULL;
  bfd_vma gp;
  struct elf_link_hash_table *ehtab;
  struct elf_nds32_link_hash_table *htab;
  bfd *dynobj;
  bfd_vma *local_got_offsets;
  asection *sgot, *splt, *sreloc;
  bfd_vma high_address;
  struct elf_nds32_link_hash_table *table;
  int eliminate_gc_relocs;
  bfd_vma fpbase_addr;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  ehtab = elf_hash_table (info);
  htab = nds32_elf_hash_table (info);
  high_address = bfd_get_section_limit (input_bfd, input_section);

  dynobj = htab->root.dynobj;
  local_got_offsets = elf_local_got_offsets (input_bfd);

  sgot = ehtab->sgot;
  splt = ehtab->splt;
  sreloc = NULL;

  rel = relocs;
  relend = relocs + input_section->reloc_count;

  table = nds32_elf_hash_table (info);
  eliminate_gc_relocs = table->eliminate_gc_relocs;

  /* By this time, we can adjust the value of _SDA_BASE_.  */
  /* Explain _SDA_BASE_  */
  if ((!bfd_link_relocatable (info)))
    {
      is_SDA_BASE_set = 1;
      r = nds32_elf_final_sda_base (output_bfd, info, &gp, true);
      if (r != bfd_reloc_ok)
	return false;
    }

  /* Do TLS model conversion once at first.  */
  nds32_elf_unify_tls_model (input_bfd, input_section, contents, info);

  /* Use gp as fp to prevent truncated fit.  Because in relaxation time
     the fp value is set as gp, and it has be reverted for instruction
     setting fp.  */
  fpbase_addr = elf_gp (output_bfd);

  /* Deal with (dynamic) relocations.  */
  for (rel = relocs; rel < relend; rel++)
    {
      enum elf_nds32_reloc_type r_type;
      reloc_howto_type *howto = NULL;
      unsigned long r_symndx;
      struct elf_link_hash_entry *h = NULL;
      Elf_Internal_Sym *sym = NULL;
      asection *sec;
      bfd_vma relocation;
      bfd_vma relocation_sym = 0xdeadbeef;
      Elf_Internal_Rela *lorel;
      bfd_vma off;

      /* We can't modify r_addend here as elf_link_input_bfd has an assert to
	 ensure it's zero (we use REL relocs, not RELA).  Therefore this
	 should be assigning zero to `addend', but for clarity we use
	 `r_addend'.  */

      bfd_vma addend = rel->r_addend;
      bfd_vma offset = rel->r_offset;

      r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type >= R_NDS32_max)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      input_bfd, r_type);
	  bfd_set_error (bfd_error_bad_value);
	  ret = false;
	  continue;
	}

      if (r_type == R_NDS32_GNU_VTENTRY
	  || r_type == R_NDS32_GNU_VTINHERIT
	  || r_type == R_NDS32_NONE
	  || r_type == R_NDS32_RELA_GNU_VTENTRY
	  || r_type == R_NDS32_RELA_GNU_VTINHERIT
	  || (r_type >= R_NDS32_INSN16 && r_type <= R_NDS32_25_FIXED_RELA)
	  || r_type == R_NDS32_DATA
	  || r_type == R_NDS32_TRAN)
	continue;

      /* If we enter the fp-as-gp region.  Resolve the address
	 of best fp-base.  */
      if (ELF32_R_TYPE (rel->r_info) == R_NDS32_RELAX_REGION_BEGIN
	  && (rel->r_addend & R_NDS32_RELAX_REGION_OMIT_FP_FLAG))
	{
	  int dist;

	  /* Distance to relocation of best fp-base is encoded in R_SYM.  */
	  dist =  rel->r_addend >> 16;
	  fpbase_addr = calculate_memory_address (input_bfd, rel + dist,
						  local_syms, symtab_hdr);
	}
      else if (ELF32_R_TYPE (rel->r_info) == R_NDS32_RELAX_REGION_END
	       && (rel->r_addend & R_NDS32_RELAX_REGION_OMIT_FP_FLAG))
	{
	  fpbase_addr = elf_gp (output_bfd);
	}

      /* Skip the relocations used for relaxation.  */
      /* We have to update LONGCALL and LONGJUMP
	 relocations when generating the relocatable files.  */
      if (!bfd_link_relocatable (info)
	  && (r_type >= R_NDS32_RELAX_ENTRY
	      || (r_type >= R_NDS32_LONGCALL4
		  && r_type <= R_NDS32_LONGJUMP7)))
	continue;

      howto = bfd_elf32_bfd_reloc_type_table_lookup (r_type);
      r_symndx = ELF32_R_SYM (rel->r_info);

      /* This is a final link.  */
      sym = NULL;
      sec = NULL;
      h = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* Local symbol.  */
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];

	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	  addend = rel->r_addend;

	  /* keep symbol location for static TLS_IE GOT entry  */
	  relocation_sym = relocation;
	  if (bfd_link_relocatable (info))
	    {
	      /* This is a relocatable link.  We don't have to change
		 anything, unless the reloc is against a section symbol,
		 in which case we have to adjust according to where the
		 section symbol winds up in the output section.  */
	      if (sym != NULL && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
		rel->r_addend += sec->output_offset + sym->st_value;

	      continue;
	    }
	}
      else
	{
	  /* External symbol.  */
	  if (bfd_link_relocatable (info))
	    continue;
	  bool warned, ignored, unresolved_reloc;
	  int symndx = r_symndx - symtab_hdr->sh_info;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes, h, sec,
				   relocation, unresolved_reloc, warned,
				   ignored);

	  /* keep symbol location for static TLS_IE GOT entry  */
	  relocation_sym = relocation;

	  /* la $fp, _FP_BASE_ is per-function (region).
	     Handle it specially.  */
	  switch ((int) r_type)
	    {
	    case R_NDS32_HI20_RELA:
	    case R_NDS32_LO12S0_RELA:
	      if (strcmp (elf_sym_hashes (input_bfd)[symndx]->root.root.string,
			  FP_BASE_NAME) == 0)
		{
		  if (!bfd_link_pie (info))
		    {
		      _bfd_error_handler
			("%pB: warning: _FP_BASE_ setting insns relaxation failed.",
			 input_bfd);
		    }
		  relocation = fpbase_addr;
		}
	      break;
	    case R_NDS32_SDA19S0_RELA:
	    case R_NDS32_SDA15S0_RELA:
	    case R_NDS32_20_RELA:
	      if (strcmp (elf_sym_hashes (input_bfd)[symndx]->root.root.string,
			  FP_BASE_NAME) == 0)
		{
		  relocation = fpbase_addr;
		  break;
		}
	    }
	}

      /* Sanity check the address.  */
      if (offset > high_address)
	{
	  r = bfd_reloc_outofrange;
	  goto check_reloc;
	}

      if (r_type >= R_NDS32_RELAX_ENTRY)
	continue;

      switch ((int) r_type)
	{
	case R_NDS32_GOTOFF:
	  /* Relocation is relative to the start of the global offset
	     table (for ld24 rx, #uimm24), e.g. access at label+addend

	     ld24 rx. #label@GOTOFF + addend
	     sub  rx, r12.  */
	case R_NDS32_GOTOFF_HI20:
	case R_NDS32_GOTOFF_LO12:
	case R_NDS32_GOTOFF_LO15:
	case R_NDS32_GOTOFF_LO19:
	  BFD_ASSERT (sgot != NULL);

	  relocation -= elf_gp (output_bfd);
	  break;

	case R_NDS32_9_PLTREL:
	case R_NDS32_25_PLTREL:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  /* The native assembler will generate a 25_PLTREL reloc
	     for a local symbol if you assemble a call from one
	     section to another when using -K pic.  */
	  if (h == NULL)
	    break;

	  if (h->forced_local)
	    break;

	  /* We didn't make a PLT entry for this symbol.  This
	     happens when statically linking PIC code, or when
	     using -Bsymbolic.  */
	  if (h->plt.offset == (bfd_vma) - 1)
	    break;

	  relocation = (splt->output_section->vma
			+ splt->output_offset + h->plt.offset);
	  break;

	case R_NDS32_PLT_GOTREL_HI20:
	case R_NDS32_PLT_GOTREL_LO12:
	case R_NDS32_PLT_GOTREL_LO15:
	case R_NDS32_PLT_GOTREL_LO19:
	case R_NDS32_PLT_GOTREL_LO20:
	  if (h == NULL
	      || h->forced_local
	      || h->plt.offset == (bfd_vma) -1
	      || (bfd_link_pie (info) && h->def_regular))
	    {
	      /* Maybe we should find better checking to optimize
		 PIE PLT relocations.  */
	      /* We didn't make a PLT entry for this symbol.  This
		 happens when statically linking PIC code, or when
		 using -Bsymbolic.  */
	      if (h)
		h->plt.offset = (bfd_vma) -1;   /* Cancel PLT trampoline.  */
	      relocation -= elf_gp (output_bfd);
	      break;
	    }

	  relocation = (splt->output_section->vma
			+ splt->output_offset + h->plt.offset);

	  relocation -= elf_gp (output_bfd);
	  break;

	case R_NDS32_PLTREL_HI20:
	case R_NDS32_PLTREL_LO12:

	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  /* The native assembler will generate a 25_PLTREL reloc
	     for a local symbol if you assemble a call from one
	     section to another when using -K pic.  */
	  if (h == NULL)
	    break;

	  if (h->forced_local)
	    break;

	  if (h->plt.offset == (bfd_vma) - 1)
	    /* We didn't make a PLT entry for this symbol.  This
	       happens when statically linking PIC code, or when
	       using -Bsymbolic.  */
	    break;

	  if (splt == NULL)
	    break;

	  relocation = (splt->output_section->vma
			+ splt->output_offset
			+ h->plt.offset + 4)
		       - (input_section->output_section->vma
			  + input_section->output_offset
			  + rel->r_offset);

	  break;

	case R_NDS32_GOTPC20:
	  /* .got(_GLOBAL_OFFSET_TABLE_) - pc relocation
	     ld24 rx,#_GLOBAL_OFFSET_TABLE_  */
	  relocation = elf_gp (output_bfd);
	  break;

	case R_NDS32_GOTPC_HI20:
	case R_NDS32_GOTPC_LO12:
	  /* .got(_GLOBAL_OFFSET_TABLE_) - pc relocation
	     bl .+4
	     seth rx,#high(_GLOBAL_OFFSET_TABLE_)
	     or3 rx,rx,#low(_GLOBAL_OFFSET_TABLE_ +4)
	     or
	     bl .+4
	     seth rx,#shigh(_GLOBAL_OFFSET_TABLE_)
	     add3 rx,rx,#low(_GLOBAL_OFFSET_TABLE_ +4)  */
	  relocation = elf_gp (output_bfd);
	  relocation -= (input_section->output_section->vma
			 + input_section->output_offset + rel->r_offset);
	  break;

	case R_NDS32_GOT20:
	  /* Fall through.  */
	case R_NDS32_GOT_HI20:
	case R_NDS32_GOT_LO12:
	case R_NDS32_GOT_LO15:
	case R_NDS32_GOT_LO19:
	  /* Relocation is to the entry for this symbol in the global
	     offset table.  */
	  BFD_ASSERT (sgot != NULL);

	  if (h != NULL)
	    {
	      /* External symbol  */
	      bool dyn;

	      off = h->got.offset;
	      BFD_ASSERT (off != (bfd_vma) - 1);
	      dyn = htab->root.dynamic_sections_created;
	      if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						    bfd_link_pic (info),
						    h)
		  || (bfd_link_pic (info)
		      && (info->symbolic
			  || h->dynindx == -1
			  || h->forced_local) && h->def_regular))
		{
		  /* This is actually a static link, or it is a
		     -Bsymbolic link and the symbol is defined
		     locally, or the symbol was forced to be local
		     because of a version file.  We must initialize
		     this entry in the global offset table.  Since the
		     offset must always be a multiple of 4, we use the
		     least significant bit to record whether we have
		     initialized it already.

		     When doing a dynamic link, we create a .rela.got
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine.  */
		  if ((off & 1) != 0)	/* clear LSB  */
		    off &= ~1;
		  else
		    {
		      bfd_put_32 (output_bfd, relocation, sgot->contents + off);
		      h->got.offset |= 1;
		    }
		}
	      relocation = sgot->output_section->vma + sgot->output_offset + off
			   - elf_gp (output_bfd);
	    }
	  else
	    {
	      /* Local symbol  */
	      bfd_byte *loc;

	      BFD_ASSERT (local_got_offsets != NULL
			  && local_got_offsets[r_symndx] != (bfd_vma) - 1);

	      off = local_got_offsets[r_symndx];

	      /* The offset must always be a multiple of 4.  We use
		 the least significant bit to record whether we have
		 already processed this entry.  */
	      if ((off & 1) != 0)	/* clear LSB  */
		off &= ~1;
	      else
		{
		  bfd_put_32 (output_bfd, relocation, sgot->contents + off);

		  if (bfd_link_pic (info))
		    {
		      asection *srelgot;
		      Elf_Internal_Rela outrel;

		      /* We need to generate a R_NDS32_RELATIVE reloc
			 for the dynamic linker.  */
		      srelgot = bfd_get_section_by_name (dynobj, ".rela.got");
		      BFD_ASSERT (srelgot != NULL);

		      outrel.r_offset = (elf_gp (output_bfd)
					 + sgot->output_offset + off);
		      outrel.r_info = ELF32_R_INFO (0, R_NDS32_RELATIVE);
		      outrel.r_addend = relocation;
		      loc = srelgot->contents;
		      loc +=
			srelgot->reloc_count * sizeof (Elf32_External_Rela);
		      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		      ++srelgot->reloc_count;
		    }
		  local_got_offsets[r_symndx] |= 1;
		}
	      relocation = sgot->output_section->vma + sgot->output_offset + off
			   - elf_gp (output_bfd);
	    }

	  break;

	case R_NDS32_16_RELA:
	case R_NDS32_20_RELA:
	case R_NDS32_5_RELA:
	case R_NDS32_32_RELA:
	case R_NDS32_9_PCREL_RELA:
	case R_NDS32_WORD_9_PCREL_RELA:
	case R_NDS32_10_UPCREL_RELA:
	case R_NDS32_15_PCREL_RELA:
	case R_NDS32_17_PCREL_RELA:
	case R_NDS32_25_PCREL_RELA:
	case R_NDS32_HI20_RELA:
	case R_NDS32_LO12S3_RELA:
	case R_NDS32_LO12S2_RELA:
	case R_NDS32_LO12S2_DP_RELA:
	case R_NDS32_LO12S2_SP_RELA:
	case R_NDS32_LO12S1_RELA:
	case R_NDS32_LO12S0_RELA:
	case R_NDS32_LO12S0_ORI_RELA:
	  if (bfd_link_pic (info) && r_symndx != 0
	      && (input_section->flags & SEC_ALLOC) != 0
	      && (eliminate_gc_relocs == 0
		  || (sec && (sec->flags & SEC_EXCLUDE) == 0))
	      && ((r_type != R_NDS32_9_PCREL_RELA
		   && r_type != R_NDS32_WORD_9_PCREL_RELA
		   && r_type != R_NDS32_10_UPCREL_RELA
		   && r_type != R_NDS32_15_PCREL_RELA
		   && r_type != R_NDS32_17_PCREL_RELA
		   && r_type != R_NDS32_25_PCREL_RELA
		   && !(r_type == R_NDS32_32_RELA
			&& strcmp (input_section->name, ".eh_frame") == 0))
		  || (h != NULL && h->dynindx != -1
		      && (!info->symbolic || !h->def_regular))))
	    {
	      Elf_Internal_Rela outrel;
	      bool skip, relocate;
	      bfd_byte *loc;

	      /* When generating a shared object, these relocations
		 are copied into the output file to be resolved at run
		 time.  */

	      if (sreloc == NULL)
		{
		  const char *name;

		  name = bfd_elf_string_from_elf_section
		    (input_bfd, elf_elfheader (input_bfd)->e_shstrndx,
		     elf_section_data (input_section)->rela.hdr->sh_name);
		  if (name == NULL)
		    return false;

		  BFD_ASSERT (startswith (name, ".rela")
			      && strcmp (bfd_section_name (input_section),
					 name + 5) == 0);

		  sreloc = bfd_get_section_by_name (dynobj, name);
		  BFD_ASSERT (sreloc != NULL);
		}

	      skip = false;
	      relocate = false;

	      outrel.r_offset = _bfd_elf_section_offset (output_bfd,
							 info,
							 input_section,
							 rel->r_offset);
	      if (outrel.r_offset == (bfd_vma) - 1)
		skip = true;
	      else if (outrel.r_offset == (bfd_vma) - 2)
		skip = true, relocate = true;
	      outrel.r_offset += (input_section->output_section->vma
				  + input_section->output_offset);

	      if (skip)
		memset (&outrel, 0, sizeof outrel);
	      else if (r_type == R_NDS32_17_PCREL_RELA
		       || r_type == R_NDS32_15_PCREL_RELA
		       || r_type == R_NDS32_25_PCREL_RELA)
		{
		  BFD_ASSERT (h != NULL && h->dynindx != -1);
		  outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  /* h->dynindx may be -1 if this symbol was marked to
		     become local.  */
		  if (h == NULL
		      || ((info->symbolic || h->dynindx == -1)
			  && h->def_regular)
		      || (bfd_link_pie (info) && h->def_regular))
		    {
		      relocate = true;
		      outrel.r_info = ELF32_R_INFO (0, R_NDS32_RELATIVE);
		      outrel.r_addend = relocation + rel->r_addend;

		      if (h)
			{
			  h->plt.offset = (bfd_vma) -1;   /* cancel PLT trampoline.  */

			  BFD_ASSERT (sgot != NULL);
			  /* If we did not allocate got entry for the symbol,
			     we can not fill the nonexistent got entry.  */
			  if (h->got.offset != (bfd_vma) -1
			      && (h->got.offset & 1) == 0)
			    {
			      bfd_put_32 (output_bfd, outrel.r_addend,
					  sgot->contents + h->got.offset);
			    }
			}
		    }
		  else
		    {
		      if (h->dynindx == -1)
			{
			  _bfd_error_handler
			    (_("%pB: relocation %s against `%s' can not be used when "
			       "making a shared object; recompile with -fPIC"),
			     input_bfd, nds32_elf_howto_table[r_type].name, h->root.root.string);
			  bfd_set_error (bfd_error_bad_value);
			  return false;
			}

		      outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		      outrel.r_addend = rel->r_addend;
		    }
		}

	      loc = sreloc->contents;
	      loc += sreloc->reloc_count * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
	      ++sreloc->reloc_count;

	      /* If this reloc is against an external symbol, we do
		 not want to fiddle with the addend.  Otherwise, we
		 need to include the symbol value so that it becomes
		 an addend for the dynamic reloc.  */
	      if (!relocate)
		continue;
	    }
	  break;

	case R_NDS32_25_ABS_RELA:
	  if (bfd_link_pic (info))
	    {
	      _bfd_error_handler
		(_("%pB: warning: %s unsupported in shared mode"),
		 input_bfd, "R_NDS32_25_ABS_RELA");
	      return false;
	    }
	  break;

	case R_NDS32_9_PCREL:
	  r = nds32_elf_do_9_pcrel_reloc (input_bfd, howto, input_section,
					  contents, offset,
					  sec, relocation, addend);
	  goto check_reloc;

	case R_NDS32_HI20:
	  /* We allow an arbitrary number of HI20 relocs before the
	     LO12 reloc.  This permits gcc to emit the HI and LO relocs
	     itself.  */
	  for (lorel = rel + 1;
	       (lorel < relend
		&& ELF32_R_TYPE (lorel->r_info) == R_NDS32_HI20); lorel++)
	    continue;
	  if (lorel < relend
	      && (ELF32_R_TYPE (lorel->r_info) == R_NDS32_LO12S3
		  || ELF32_R_TYPE (lorel->r_info) == R_NDS32_LO12S2
		  || ELF32_R_TYPE (lorel->r_info) == R_NDS32_LO12S1
		  || ELF32_R_TYPE (lorel->r_info) == R_NDS32_LO12S0))
	    {
	      nds32_elf_relocate_hi20 (input_bfd, r_type, rel, lorel,
				       contents, relocation + addend);
	      r = bfd_reloc_ok;
	    }
	  else
	    r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					  contents, offset, relocation,
					  addend);
	  goto check_reloc;

	case R_NDS32_GOT17S2_RELA:
	case R_NDS32_GOT15S2_RELA:
	  BFD_ASSERT (sgot != NULL);

	  if (h != NULL)
	    {
	      bool dyn;

	      off = h->got.offset;
	      BFD_ASSERT (off != (bfd_vma) - 1);

	      dyn = htab->root.dynamic_sections_created;
	      if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL
		  (dyn, bfd_link_pic (info), h)
		  || (bfd_link_pic (info)
		      && (info->symbolic
			  || h->dynindx == -1
			  || h->forced_local)
		      && h->def_regular))
		{
		  /* This is actually a static link, or it is a
		     -Bsymbolic link and the symbol is defined
		     locally, or the symbol was forced to be local
		     because of a version file.  We must initialize
		     this entry in the global offset table.  Since the
		     offset must always be a multiple of 4, we use the
		     least significant bit to record whether we have
		     initialized it already.

		     When doing a dynamic link, we create a .rela.got
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine.  */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      bfd_put_32 (output_bfd, relocation,
				  sgot->contents + off);
		      h->got.offset |= 1;
		    }
		}
	    }
	  else
	    {
	      bfd_byte *loc;

	      BFD_ASSERT (local_got_offsets != NULL
			  && local_got_offsets[r_symndx] != (bfd_vma) - 1);

	      off = local_got_offsets[r_symndx];

	      /* The offset must always be a multiple of 4.  We use
		 the least significant bit to record whether we have
		 already processed this entry.  */
	      if ((off & 1) != 0)
		off &= ~1;
	      else
		{
		  bfd_put_32 (output_bfd, relocation, sgot->contents + off);

		  if (bfd_link_pic (info))
		    {
		      asection *srelgot;
		      Elf_Internal_Rela outrel;

		      /* We need to generate a R_NDS32_RELATIVE reloc
			 for the dynamic linker.  */
		      srelgot = bfd_get_section_by_name (dynobj, ".rela.got");
		      BFD_ASSERT (srelgot != NULL);

		      outrel.r_offset = (elf_gp (output_bfd)
					 + sgot->output_offset + off);
		      outrel.r_info = ELF32_R_INFO (0, R_NDS32_RELATIVE);
		      outrel.r_addend = relocation;
		      loc = srelgot->contents;
		      loc +=
			srelgot->reloc_count * sizeof (Elf32_External_Rela);
		      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		      ++srelgot->reloc_count;
		    }
		  local_got_offsets[r_symndx] |= 1;
		}
	    }
	  relocation = sgot->output_section->vma + sgot->output_offset + off
	    - elf_gp (output_bfd);

	  if (relocation & align)
	    {
	      /* Incorrect alignment.  */
	      _bfd_error_handler
		(_("%pB: warning: unaligned access to GOT entry"), input_bfd);
	      ret = false;
	      r = bfd_reloc_dangerous;
	      goto check_reloc;
	    }
	  break;

	case R_NDS32_SDA16S3_RELA:
	case R_NDS32_SDA15S3_RELA:
	case R_NDS32_SDA15S3:
	  align = 0x7;
	  goto handle_sda;

	case R_NDS32_SDA17S2_RELA:
	case R_NDS32_SDA15S2_RELA:
	case R_NDS32_SDA12S2_SP_RELA:
	case R_NDS32_SDA12S2_DP_RELA:
	case R_NDS32_SDA15S2:
	case R_NDS32_SDA_FP7U2_RELA:
	  align = 0x3;
	  goto handle_sda;

	case R_NDS32_SDA18S1_RELA:
	case R_NDS32_SDA15S1_RELA:
	case R_NDS32_SDA15S1:
	  align = 0x1;
	  goto handle_sda;

	case R_NDS32_SDA19S0_RELA:
	case R_NDS32_SDA15S0_RELA:
	case R_NDS32_SDA15S0:
	  align = 0x0;
	handle_sda:
	  BFD_ASSERT (sec != NULL);

	  /* If the symbol is in the abs section, the out_bfd will be null.
	     This happens when the relocation has a symbol@GOTOFF.  */
	  r = nds32_elf_final_sda_base (output_bfd, info, &gp, false);
	  if (r != bfd_reloc_ok)
	    {
	      _bfd_error_handler
		(_("%pB: warning: relocate SDA_BASE failed"), input_bfd);
	      ret = false;
	      goto check_reloc;
	    }

	  /* At this point `relocation' contains the object's
	     address.  */
	  if (r_type == R_NDS32_SDA_FP7U2_RELA)
	    {
	      relocation -= fpbase_addr;
	    }
	  else
	    relocation -= gp;
	  /* Now it contains the offset from _SDA_BASE_.  */

	  /* Make sure alignment is correct.  */

	  if (relocation & align)
	    {
	      /* Incorrect alignment.  */
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA): warning: unaligned small data access"
		   " of type %d"),
		 input_bfd, input_section, r_type);
	      ret = false;
	      goto check_reloc;
	    }
	  break;

	case R_NDS32_17IFC_PCREL_RELA:
	case R_NDS32_10IFCU_PCREL_RELA:
	  /* Do nothing.  */
	  break;

	case R_NDS32_TLS_LE_HI20:
	case R_NDS32_TLS_LE_LO12:
	case R_NDS32_TLS_LE_20:
	case R_NDS32_TLS_LE_15S0:
	case R_NDS32_TLS_LE_15S1:
	case R_NDS32_TLS_LE_15S2:
	  /* We do not have garbage collection for got entries.
	     Therefore, IE to LE may have one empty entry, and DESC to
	     LE may have two.  */
	  if (elf_hash_table (info)->tls_sec != NULL)
	    relocation -= (elf_hash_table (info)->tls_sec->vma + TP_OFFSET);
	  break;

	case R_NDS32_TLS_IE_HI20:
	case R_NDS32_TLS_IE_LO12S2:
	case R_NDS32_TLS_DESC_HI20:
	case R_NDS32_TLS_DESC_LO12:
	case R_NDS32_TLS_IE_LO12:
	case R_NDS32_TLS_IEGP_HI20:
	case R_NDS32_TLS_IEGP_LO12:
	case R_NDS32_TLS_IEGP_LO12S2:
	  {
	    /* Relocation is to the entry for this symbol in the global
	       offset table.  */
	    enum elf_nds32_tls_type tls_type, org_tls_type, eff_tls_type;
	    asection *srelgot;
	    Elf_Internal_Rela outrel;
	    bfd_byte *loc;
	    int indx = 0;

	    eff_tls_type = org_tls_type = get_tls_type (r_type, h);

	    BFD_ASSERT (sgot != NULL);
	    if (h != NULL)
	      {
		bool dyn;

		off = h->got.offset;
		BFD_ASSERT (off != (bfd_vma) -1);
		dyn = htab->root.dynamic_sections_created;
		tls_type = ((struct elf_nds32_link_hash_entry *) h)->tls_type;
		if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h)
		    && (!bfd_link_pic (info)
			|| !SYMBOL_REFERENCES_LOCAL (info, h)))
		  indx = h->dynindx;
	      }
	    else
	      {
		BFD_ASSERT (local_got_offsets != NULL
			    && local_got_offsets[r_symndx] != (bfd_vma) - 1);
		off = local_got_offsets[r_symndx];
		tls_type = elf32_nds32_local_got_tls_type (input_bfd)[r_symndx];
	      }

	    relocation = sgot->output_section->vma + sgot->output_offset + off;

	    if (1 < ones32 (tls_type))
	      {
		eff_tls_type = 1 << (fls (tls_type) - 1);
		/* TLS model shall be handled in nds32_elf_unify_tls_model ().  */

		/* TLS model X -> LE is not implement yet!
		   workaround here!  */
		if (eff_tls_type == GOT_TLS_LE)
		  {
		    eff_tls_type = 1 << (fls (tls_type ^ eff_tls_type) - 1);
		  }
	      }

	    /* The offset must always be a multiple of 4.  We use
	       the least significant bit to record whether we have
	       already processed this entry.  */
	    bool need_relocs = false;
	    srelgot = ehtab->srelgot;
	    if ((bfd_link_pic (info) || indx != 0)
		&& (h == NULL || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		    || h->root.type != bfd_link_hash_undefweak))
	      {
		need_relocs = true;
		BFD_ASSERT (srelgot != NULL);
	      }

	    if (off & 1)
	      {
		off &= ~1;
		relocation &= ~1;

		if (eff_tls_type & GOT_TLS_DESC)
		  {
		    relocation -= elf_gp (output_bfd);
		    if ((R_NDS32_TLS_DESC_HI20 == r_type) && (!need_relocs))
		      {
			/* TLS model shall be converted.  */
			BFD_ASSERT(0);
		      }
		  }
		else if (eff_tls_type & GOT_TLS_IEGP)
		  {
		    relocation -= elf_gp (output_bfd);
		  }
	      }
	    else
	      {
		if ((eff_tls_type & GOT_TLS_LE) && (tls_type ^ eff_tls_type))
		  {
		    /* TLS model workaround shall be applied.  */
		    BFD_ASSERT(0);
		  }
		else if (eff_tls_type & (GOT_TLS_IE | GOT_TLS_IEGP))
		  {
		    if (eff_tls_type & GOT_TLS_IEGP)
		      relocation -= elf_gp(output_bfd);

		    if (need_relocs)
		      {
			if (indx == 0)
			  outrel.r_addend = gottpoff (info, relocation_sym);
			else
			  outrel.r_addend = 0;
			outrel.r_offset = (sgot->output_section->vma
					   + sgot->output_offset + off);
			outrel.r_info = ELF32_R_INFO (indx, R_NDS32_TLS_TPOFF);

			elf32_nds32_add_dynreloc (output_bfd, info, srelgot,
						  &outrel);
		      }
		    else
		      {
			bfd_put_32 (output_bfd, gottpoff (info, relocation_sym),
				    sgot->contents + off);
		      }
		  }
		else if (eff_tls_type & GOT_TLS_DESC)
		  {
		    relocation -= elf_gp (output_bfd);
		    if (need_relocs)
		      {
			if (indx == 0)
			  outrel.r_addend = gottpoff (info, relocation_sym);
			else
			  outrel.r_addend = 0;
			outrel.r_offset = (sgot->output_section->vma
					   + sgot->output_offset + off);
			outrel.r_info = ELF32_R_INFO (indx, R_NDS32_TLS_DESC);

			if (htab->tls_desc_trampoline)
			  {
			    asection *srelplt;
			    srelplt = ehtab->srelplt;
			    loc = srelplt->contents;
			    loc += htab->next_tls_desc_index++ * sizeof (Elf32_External_Rela);
			    BFD_ASSERT (loc + sizeof (Elf32_External_Rela)
					<= srelplt->contents + srelplt->size);

			    bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
			  }
			else
			  {
			    loc = srelgot->contents;
			    loc += srelgot->reloc_count * sizeof (Elf32_External_Rela);
			    bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
			    ++srelgot->reloc_count;
			  }
		      }
		    else
		      {
			/* feed me!  */
			bfd_put_32 (output_bfd, 0xdeadbeef,
				    sgot->contents + off);
			bfd_put_32 (output_bfd, gottpoff (info, relocation_sym),
				    sgot->contents + off + 4);
			patch_tls_desc_to_ie (contents, rel, input_bfd);
			BFD_ASSERT(0);
		      }
		  }
		else
		  {
		    /* TLS model workaround shall be applied.  */
		    BFD_ASSERT(0);
		  }

		if (h != NULL)
		  h->got.offset |= 1;
		else
		  local_got_offsets[r_symndx] |= 1;
	      }
	  }
	break;
	  /* DON'T fall through.  */

	default:
	  /* OLD_NDS32_RELOC.  */

	  r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					contents, offset, relocation, addend);
	  goto check_reloc;
	}

      switch ((int) r_type)
	{
	case R_NDS32_20_RELA:
	case R_NDS32_5_RELA:
	case R_NDS32_9_PCREL_RELA:
	case R_NDS32_WORD_9_PCREL_RELA:
	case R_NDS32_10_UPCREL_RELA:
	case R_NDS32_15_PCREL_RELA:
	case R_NDS32_17_PCREL_RELA:
	case R_NDS32_25_PCREL_RELA:
	case R_NDS32_25_ABS_RELA:
	case R_NDS32_HI20_RELA:
	case R_NDS32_LO12S3_RELA:
	case R_NDS32_LO12S2_RELA:
	case R_NDS32_LO12S2_DP_RELA:
	case R_NDS32_LO12S2_SP_RELA:
	case R_NDS32_LO12S1_RELA:
	case R_NDS32_LO12S0_RELA:
	case R_NDS32_LO12S0_ORI_RELA:
	case R_NDS32_SDA16S3_RELA:
	case R_NDS32_SDA17S2_RELA:
	case R_NDS32_SDA18S1_RELA:
	case R_NDS32_SDA19S0_RELA:
	case R_NDS32_SDA15S3_RELA:
	case R_NDS32_SDA15S2_RELA:
	case R_NDS32_SDA12S2_DP_RELA:
	case R_NDS32_SDA12S2_SP_RELA:
	case R_NDS32_SDA15S1_RELA:
	case R_NDS32_SDA15S0_RELA:
	case R_NDS32_SDA_FP7U2_RELA:
	case R_NDS32_9_PLTREL:
	case R_NDS32_25_PLTREL:
	case R_NDS32_GOT20:
	case R_NDS32_GOT_HI20:
	case R_NDS32_GOT_LO12:
	case R_NDS32_GOT_LO15:
	case R_NDS32_GOT_LO19:
	case R_NDS32_GOT15S2_RELA:
	case R_NDS32_GOT17S2_RELA:
	case R_NDS32_GOTPC20:
	case R_NDS32_GOTPC_HI20:
	case R_NDS32_GOTPC_LO12:
	case R_NDS32_GOTOFF:
	case R_NDS32_GOTOFF_HI20:
	case R_NDS32_GOTOFF_LO12:
	case R_NDS32_GOTOFF_LO15:
	case R_NDS32_GOTOFF_LO19:
	case R_NDS32_PLTREL_HI20:
	case R_NDS32_PLTREL_LO12:
	case R_NDS32_PLT_GOTREL_HI20:
	case R_NDS32_PLT_GOTREL_LO12:
	case R_NDS32_PLT_GOTREL_LO15:
	case R_NDS32_PLT_GOTREL_LO19:
	case R_NDS32_PLT_GOTREL_LO20:
	case R_NDS32_17IFC_PCREL_RELA:
	case R_NDS32_10IFCU_PCREL_RELA:
	case R_NDS32_TLS_LE_HI20:
	case R_NDS32_TLS_LE_LO12:
	case R_NDS32_TLS_IE_HI20:
	case R_NDS32_TLS_IE_LO12S2:
	case R_NDS32_TLS_LE_20:
	case R_NDS32_TLS_LE_15S0:
	case R_NDS32_TLS_LE_15S1:
	case R_NDS32_TLS_LE_15S2:
	case R_NDS32_TLS_DESC_HI20:
	case R_NDS32_TLS_DESC_LO12:
	case R_NDS32_TLS_IE_LO12:
	case R_NDS32_TLS_IEGP_HI20:
	case R_NDS32_TLS_IEGP_LO12:
	case R_NDS32_TLS_IEGP_LO12S2:
	  /* Instruction related relocs must handle endian properly.  */
	  /* NOTE: PIC IS NOT HANDLE YET; DO IT LATER.  */
	  r = nds32_elf_final_link_relocate (howto, input_bfd,
					     input_section, contents,
					     rel->r_offset, relocation,
					     rel->r_addend);
	  break;

	default:
	  /* All other relocs can use default handler.  */
	  r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					contents, rel->r_offset,
					relocation, rel->r_addend);
	  break;
	}

    check_reloc:

      if (r != bfd_reloc_ok)
	{
	  /* FIXME: This should be generic enough to go in a utility.  */
	  const char *name;

	  if (h != NULL)
	    name = h->root.root.string;
	  else
	    {
	      name = bfd_elf_string_from_elf_section
		      (input_bfd, symtab_hdr->sh_link, sym->st_name);
	      if (name == NULL || *name == '\0')
		name = bfd_section_name (sec);
	    }

	  if (errmsg != NULL)
	    goto common_error;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow)
		(info, (h ? &h->root : NULL), name, howto->name,
		 (bfd_vma) 0, input_bfd, input_section, offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, name, input_bfd, input_section, offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      errmsg = _("internal error: out of range error");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      errmsg = _("internal error: unsupported relocation error");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      errmsg = _("internal error: dangerous error");
	      goto common_error;

	    default:
	      errmsg = _("internal error: unknown error");
	      /* Fall through.  */

	    common_error:
	      (*info->callbacks->warning) (info, errmsg, name, input_bfd,
					   input_section, offset);
	      break;
	    }
	}
    }

  /* Resotre header size to avoid overflow load.  */
  if (elf_nds32_tdata (input_bfd)->hdr_size != 0)
    symtab_hdr->sh_size = elf_nds32_tdata (input_bfd)->hdr_size;

  return ret;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
nds32_elf_finish_dynamic_symbol (bfd *output_bfd, struct bfd_link_info *info,
				 struct elf_link_hash_entry *h, Elf_Internal_Sym *sym)
{
  struct elf_link_hash_table *ehtab;
  struct elf_nds32_link_hash_entry *hent;
  bfd_byte *loc;

  ehtab = elf_hash_table (info);
  hent = (struct elf_nds32_link_hash_entry *) h;

  if (h->plt.offset != (bfd_vma) - 1)
    {
      asection *splt;
      asection *sgot;
      asection *srela;

      bfd_vma plt_index;
      bfd_vma got_offset;
      bfd_vma local_plt_offset;
      Elf_Internal_Rela rela;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */

      BFD_ASSERT (h->dynindx != -1);

      splt = ehtab->splt;
      sgot = ehtab->sgotplt;
      srela = ehtab->srelplt;
      BFD_ASSERT (splt != NULL && sgot != NULL && srela != NULL);

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.  */
      plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;

      /* Get the offset into the .got table of the entry that
	 corresponds to this function.  Each .got entry is 4 bytes.
	 The first three are reserved.  */
      got_offset = (plt_index + 3) * 4;

      /* Fill in the entry in the procedure linkage table.  */
      if (!bfd_link_pic (info))
	{
	  unsigned long insn;

	  insn = PLT_ENTRY_WORD0 + (((sgot->output_section->vma
				      + sgot->output_offset + got_offset) >> 12)
				    & 0xfffff);
	  bfd_putb32 (insn, splt->contents + h->plt.offset);

	  insn = PLT_ENTRY_WORD1 + (((sgot->output_section->vma
				      + sgot->output_offset + got_offset) & 0x0fff)
				    >> 2);
	  bfd_putb32 (insn, splt->contents + h->plt.offset + 4);

	  insn = PLT_ENTRY_WORD2;
	  bfd_putb32 (insn, splt->contents + h->plt.offset + 8);

	  insn = PLT_ENTRY_WORD3 + (plt_index & 0x7ffff);
	  bfd_putb32 (insn, splt->contents + h->plt.offset + 12);

	  insn = PLT_ENTRY_WORD4
		 + (((unsigned int) ((-(h->plt.offset + 16)) >> 1)) & 0xffffff);
	  bfd_putb32 (insn, splt->contents + h->plt.offset + 16);
	  local_plt_offset = 12;
	}
      else
	{
	  /* sda_base must be set at this time.  */
	  unsigned long insn;
	  long offset;

	  offset = sgot->output_section->vma + sgot->output_offset + got_offset
		   - elf_gp (output_bfd);
	  insn = PLT_PIC_ENTRY_WORD0 + ((offset >> 12) & 0xfffff);
	  bfd_putb32 (insn, splt->contents + h->plt.offset);

	  insn = PLT_PIC_ENTRY_WORD1 + (offset & 0xfff);
	  bfd_putb32 (insn, splt->contents + h->plt.offset + 4);

	  insn = PLT_PIC_ENTRY_WORD2;
	  bfd_putb32 (insn, splt->contents + h->plt.offset + 8);

	  insn = PLT_PIC_ENTRY_WORD3;
	  bfd_putb32 (insn, splt->contents + h->plt.offset + 12);

	  insn = PLT_PIC_ENTRY_WORD4 + (plt_index & 0x7fffff);
	  bfd_putb32 (insn, splt->contents + h->plt.offset + 16);

	  insn = PLT_PIC_ENTRY_WORD5
	    + (((unsigned int) ((-(h->plt.offset + 20)) >> 1)) & 0xffffff);
	  bfd_putb32 (insn, splt->contents + h->plt.offset + 20);

	  local_plt_offset = 16;
	}

      /* Fill in the entry in the global offset table,
	 so it will fall through to the next instruction for the first time.  */
      bfd_put_32 (output_bfd,
		  (splt->output_section->vma + splt->output_offset
		   + h->plt.offset + local_plt_offset),
		  sgot->contents + got_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = (sgot->output_section->vma
		       + sgot->output_offset + got_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_NDS32_JMP_SLOT);
      rela.r_addend = 0;
      loc = srela->contents;
      loc += plt_index * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	  if (!h->ref_regular_nonweak)
	    sym->st_value = 0;
	}
    }

  if (h->got.offset != (bfd_vma) - 1
      && hent->tls_type == GOT_NORMAL)
    {
      asection *sgot;
      asection *srelagot;
      Elf_Internal_Rela rela;

      /* This symbol has an entry in the global offset table.
	 Set it up.  */

      sgot = ehtab->sgot;
      srelagot = ehtab->srelgot;
      BFD_ASSERT (sgot != NULL && srelagot != NULL);

      rela.r_offset = (sgot->output_section->vma
		       + sgot->output_offset + (h->got.offset & ~1));

      /* If this is a -Bsymbolic link, and the symbol is defined
	 locally, we just want to emit a RELATIVE reloc.  Likewise if
	 the symbol was forced to be local because of a version file.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if ((bfd_link_pic (info)
	   && (info->symbolic || h->dynindx == -1 || h->forced_local)
	   && h->def_regular)
	  || (bfd_link_pie (info) && h->def_regular))
	{
	  rela.r_info = ELF32_R_INFO (0, R_NDS32_RELATIVE);
	  rela.r_addend = (h->root.u.def.value
			   + h->root.u.def.section->output_section->vma
			   + h->root.u.def.section->output_offset);

	  if ((h->got.offset & 1) == 0)
	    {
	      bfd_put_32 (output_bfd, rela.r_addend,
			  sgot->contents + h->got.offset);
	    }
	}
      else
	{
	  BFD_ASSERT ((h->got.offset & 1) == 0);
	  bfd_put_32 (output_bfd, (bfd_vma) 0,
		      sgot->contents + h->got.offset);
	  rela.r_info = ELF32_R_INFO (h->dynindx, R_NDS32_GLOB_DAT);
	  rela.r_addend = 0;
	}

      loc = srelagot->contents;
      loc += srelagot->reloc_count * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
      ++srelagot->reloc_count;
      BFD_ASSERT (loc < (srelagot->contents + srelagot->size));
    }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rela;

      /* This symbols needs a copy reloc.  Set it up.  */

      BFD_ASSERT (h->dynindx != -1
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak));

      s = bfd_get_section_by_name (h->root.u.def.section->owner, ".rela.bss");
      BFD_ASSERT (s != NULL);

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_NDS32_COPY);
      rela.r_addend = 0;
      loc = s->contents;
      loc += s->reloc_count * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
      ++s->reloc_count;
    }

  /* Mark some specially defined symbols as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
    sym->st_shndx = SHN_ABS;

  return true;
}


/* Finish up the dynamic sections.  */

static bool
nds32_elf_finish_dynamic_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn;
  asection *sgotplt;
  struct elf_link_hash_table *ehtab;
  struct elf_nds32_link_hash_table *htab;

  ehtab = elf_hash_table (info);
  htab = nds32_elf_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = elf_hash_table (info)->dynobj;

  sgotplt = ehtab->sgotplt;
  /* A broken linker script might have discarded the dynamic sections.
     Catch this here so that we do not seg-fault later on.  */
  if (sgotplt != NULL && bfd_is_abs_section (sgotplt->output_section))
    return false;
  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      BFD_ASSERT (sgotplt != NULL && sdyn != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);

      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      break;

	    case DT_PLTGOT:
	      /* name = ".got";  */
	      s = ehtab->sgot->output_section;
	      goto get_vma;
	    case DT_JMPREL:
	      s = ehtab->srelplt->output_section;
	    get_vma:
	      BFD_ASSERT (s != NULL);
	      dyn.d_un.d_ptr = s->vma;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTRELSZ:
	      s = ehtab->srelplt->output_section;
	      BFD_ASSERT (s != NULL);
	      dyn.d_un.d_val = s->size;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_RELASZ:
	      /* My reading of the SVR4 ABI indicates that the
		 procedure linkage table relocs (DT_JMPREL) should be
		 included in the overall relocs (DT_RELA).  This is
		 what Solaris does.  However, UnixWare can not handle
		 that case.  Therefore, we override the DT_RELASZ entry
		 here to make it not include the JMPREL relocs.  Since
		 the linker script arranges for .rela.plt to follow all
		 other relocation sections, we don't have to worry
		 about changing the DT_RELA entry.  */
	      if (ehtab->srelplt != NULL)
		{
		  s = ehtab->srelplt->output_section;
		  dyn.d_un.d_val -= s->size;
		}
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_TLSDESC_PLT:
	      s = htab->root.splt;
	      dyn.d_un.d_ptr = (s->output_section->vma + s->output_offset
				+ htab->root.tlsdesc_plt);
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_TLSDESC_GOT:
	      s = htab->root.sgot;
	      dyn.d_un.d_ptr = (s->output_section->vma + s->output_offset
				+ htab->root.tlsdesc_got);
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;
	    }
	}

      /* Fill in the first entry in the procedure linkage table.  */
      splt = ehtab->splt;
      if (splt && splt->size > 0)
	{
	  if (bfd_link_pic (info))
	    {
	      unsigned long insn;
	      long offset;

	      offset = sgotplt->output_section->vma + sgotplt->output_offset + 4
		- elf_gp (output_bfd);
	      insn = PLT0_PIC_ENTRY_WORD0 | ((offset >> 12) & 0xfffff);
	      bfd_putb32 (insn, splt->contents);

	      /* here has a typo?  */
	      insn = PLT0_PIC_ENTRY_WORD1 | (offset & 0xfff);
	      bfd_putb32 (insn, splt->contents + 4);

	      insn = PLT0_PIC_ENTRY_WORD2;
	      bfd_putb32 (insn, splt->contents + 8);

	      insn = PLT0_PIC_ENTRY_WORD3;
	      bfd_putb32 (insn, splt->contents + 12);

	      insn = PLT0_PIC_ENTRY_WORD4;
	      bfd_putb32 (insn, splt->contents + 16);

	      insn = PLT0_PIC_ENTRY_WORD5;
	      bfd_putb32 (insn, splt->contents + 20);
	    }
	  else
	    {
	      unsigned long insn;
	      unsigned long addr;

	      /* addr = .got + 4 */
	      addr = sgotplt->output_section->vma + sgotplt->output_offset + 4;
	      insn = PLT0_ENTRY_WORD0 | ((addr >> 12) & 0xfffff);
	      bfd_putb32 (insn, splt->contents);

	      insn = PLT0_ENTRY_WORD1 | (addr & 0x0fff);
	      bfd_putb32 (insn, splt->contents + 4);

	      insn = PLT0_ENTRY_WORD2;
	      bfd_putb32 (insn, splt->contents + 8);

	      insn = PLT0_ENTRY_WORD3;
	      bfd_putb32 (insn, splt->contents + 12);

	      insn = PLT0_ENTRY_WORD4;
	      bfd_putb32 (insn, splt->contents + 16);
	    }

	  elf_section_data (splt->output_section)->this_hdr.sh_entsize =
	    PLT_ENTRY_SIZE;
	}

      if (htab->root.tlsdesc_plt)
	{
	  /* Calculate addresses.  */
	  asection *sgot = sgot = ehtab->sgot;
	  bfd_vma pltgot = sgotplt->output_section->vma
	    + sgotplt->output_offset;
	  bfd_vma tlsdesc_got = sgot->output_section->vma + sgot->output_offset
	    + htab->root.tlsdesc_got;

	  /* Get GP offset.  */
	  pltgot -= elf_gp (output_bfd) - 4; /* PLTGOT[1]  */
	  tlsdesc_got -= elf_gp (output_bfd);

	  /* Do relocation.  */
	  dl_tlsdesc_lazy_trampoline[0] += ((1 << 20) - 1) & (tlsdesc_got >> 12);
	  dl_tlsdesc_lazy_trampoline[1] += 0xfff & tlsdesc_got;
	  dl_tlsdesc_lazy_trampoline[4] += ((1 << 20) - 1) & (pltgot >> 12);
	  dl_tlsdesc_lazy_trampoline[5] +=  0xfff & pltgot;

	  /* Insert .plt.  */
	  nds32_put_trampoline (splt->contents + htab->root.tlsdesc_plt,
				dl_tlsdesc_lazy_trampoline,
				ARRAY_SIZE (dl_tlsdesc_lazy_trampoline));
	}
    }

  /* Fill in the first three entries in the global offset table.  */
  if (sgotplt && sgotplt->size > 0)
    {
      if (sdyn == NULL)
	bfd_put_32 (output_bfd, (bfd_vma) 0, sgotplt->contents);
      else
	bfd_put_32 (output_bfd,
		    sdyn->output_section->vma + sdyn->output_offset,
		    sgotplt->contents);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgotplt->contents + 4);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgotplt->contents + 8);

      elf_section_data (sgotplt->output_section)->this_hdr.sh_entsize = 4;
    }

  return true;
}


/* Set the right machine number.  */

static bool
nds32_elf_object_p (bfd *abfd)
{
  static unsigned int cur_arch = 0;

  if (E_N1_ARCH != (elf_elfheader (abfd)->e_flags & EF_NDS_ARCH))
    {
      /* E_N1_ARCH is a wild card, so it is set only when no others exist.  */
      cur_arch = (elf_elfheader (abfd)->e_flags & EF_NDS_ARCH);
    }

  switch (cur_arch)
    {
    default:
    case E_N1_ARCH:
      bfd_default_set_arch_mach (abfd, bfd_arch_nds32, bfd_mach_n1);
      break;
    case E_N1H_ARCH:
      bfd_default_set_arch_mach (abfd, bfd_arch_nds32, bfd_mach_n1h);
      break;
    case E_NDS_ARCH_STAR_V2_0:
      bfd_default_set_arch_mach (abfd, bfd_arch_nds32, bfd_mach_n1h_v2);
      break;
    case E_NDS_ARCH_STAR_V3_0:
      bfd_default_set_arch_mach (abfd, bfd_arch_nds32, bfd_mach_n1h_v3);
      break;
    case E_NDS_ARCH_STAR_V3_M:
      bfd_default_set_arch_mach (abfd, bfd_arch_nds32, bfd_mach_n1h_v3m);
      break;
    }

  return true;
}

/* Store the machine number in the flags field.  */

static bool
nds32_elf_final_write_processing (bfd *abfd)
{
  unsigned long val;
  static unsigned int cur_mach = 0;

  if (bfd_mach_n1 != bfd_get_mach (abfd))
    {
      cur_mach = bfd_get_mach (abfd);
    }

  switch (cur_mach)
    {
    case bfd_mach_n1:
      /* Only happen when object is empty, since the case is abandon.  */
      val = E_N1_ARCH;
      val |= E_NDS_ABI_AABI;
      val |= E_NDS32_ELF_VER_1_4;
      break;
    case bfd_mach_n1h:
      val = E_N1H_ARCH;
      break;
    case bfd_mach_n1h_v2:
      val = E_NDS_ARCH_STAR_V2_0;
      break;
    case bfd_mach_n1h_v3:
      val = E_NDS_ARCH_STAR_V3_0;
      break;
    case bfd_mach_n1h_v3m:
      val = E_NDS_ARCH_STAR_V3_M;
      break;
    default:
      val = 0;
      break;
    }

  elf_elfheader (abfd)->e_flags &= ~EF_NDS_ARCH;
  elf_elfheader (abfd)->e_flags |= val;
  return _bfd_elf_final_write_processing (abfd);
}

/* Function to keep NDS32 specific file flags.  */

static bool
nds32_elf_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

static unsigned int
convert_e_flags (unsigned int e_flags, unsigned int arch)
{
  if ((e_flags & EF_NDS_ARCH) == E_NDS_ARCH_STAR_V0_9)
    {
      /* From 0.9 to 1.0.  */
      e_flags = (e_flags & (~EF_NDS_ARCH)) | E_NDS_ARCH_STAR_V1_0;

      /* Invert E_NDS32_HAS_NO_MAC_INST.  */
      e_flags ^= E_NDS32_HAS_NO_MAC_INST;
      if (arch == E_NDS_ARCH_STAR_V1_0)
	{
	  /* Done.  */
	  return e_flags;
	}
    }

  /* From 1.0 to 2.0.  */
  e_flags = (e_flags & (~EF_NDS_ARCH)) | E_NDS_ARCH_STAR_V2_0;

  /* Clear E_NDS32_HAS_MFUSR_PC_INST.  */
  e_flags &= ~E_NDS32_HAS_MFUSR_PC_INST;

  /* Invert E_NDS32_HAS_NO_MAC_INST.  */
  e_flags ^= E_NDS32_HAS_NO_MAC_INST;
  return e_flags;
}

static bool
nds32_check_vec_size (bfd *ibfd)
{
  static unsigned int nds32_vec_size = 0;

  asection *sec_t = NULL;
  bfd_byte *contents = NULL;

  sec_t = bfd_get_section_by_name (ibfd, ".nds32_e_flags");

  if (sec_t && sec_t->size >= 4)
    {
      /* Get vec_size in file.  */
      unsigned int flag_t;

      nds32_get_section_contents (ibfd, sec_t, &contents, true);
      flag_t = bfd_get_32 (ibfd, contents);

      /* The value could only be 4 or 16.  */

      if (!nds32_vec_size)
	/* Set if not set yet.  */
	nds32_vec_size = (flag_t & 0x3);
      else if (nds32_vec_size != (flag_t & 0x3))
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: ISR vector size mismatch"
	       " with previous modules, previous %u-byte, current %u-byte"),
	     ibfd,
	     nds32_vec_size == 1 ? 4 : nds32_vec_size == 2 ? 16 : 0xffffffff,
	     (flag_t & 0x3) == 1 ? 4 : (flag_t & 0x3) == 2 ? 16 : 0xffffffff);
	  return false;
	}
      else
	/* Only keep the first vec_size section.  */
	sec_t->flags |= SEC_EXCLUDE;
    }

  return true;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
nds32_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword out_flags;
  flagword in_flags;
  flagword out_16regs;
  flagword in_no_mac;
  flagword out_no_mac;
  flagword in_16regs;
  flagword out_version;
  flagword in_version;
  flagword out_fpu_config;
  flagword in_fpu_config;

  /* FIXME: What should be checked when linking shared libraries?  */
  if ((ibfd->flags & DYNAMIC) != 0)
    return true;

  /* TODO: Revise to use object-attributes instead.  */
  if (!nds32_check_vec_size (ibfd))
    return false;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  if (bfd_little_endian (ibfd) != bfd_little_endian (obfd))
    {
      _bfd_error_handler
	(_("%pB: warning: endian mismatch with previous modules"), ibfd);

      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* -B option in objcopy cannot work as expected. e_flags = 0 shall be
     treat as generic one without checking and merging.  */
  if (elf_elfheader (ibfd)->e_flags)
    {
      in_version = elf_elfheader (ibfd)->e_flags & EF_NDS32_ELF_VERSION;
      if (in_version == E_NDS32_ELF_VER_1_2)
	{
	  _bfd_error_handler
	    (_("%pB: warning: older version of object file encountered, "
	       "please recompile with current tool chain"), ibfd);
	}

      /* We may need to merge V1 and V2 arch object files to V2.  */
      if ((elf_elfheader (ibfd)->e_flags & EF_NDS_ARCH)
	  != (elf_elfheader (obfd)->e_flags & EF_NDS_ARCH))
	{
	  /* Need to convert version.  */
	  if ((elf_elfheader (ibfd)->e_flags & EF_NDS_ARCH)
	      == E_NDS_ARCH_STAR_RESERVED)
	    {
	      elf_elfheader (obfd)->e_flags = elf_elfheader (ibfd)->e_flags;
	    }
	  else if ((elf_elfheader (ibfd)->e_flags & EF_NDS_ARCH)
		   == E_NDS_ARCH_STAR_V3_M
		   && (elf_elfheader (obfd)->e_flags & EF_NDS_ARCH)
		   == E_NDS_ARCH_STAR_V3_0)
	    {
	      elf_elfheader (ibfd)->e_flags =
		(elf_elfheader (ibfd)->e_flags & (~EF_NDS_ARCH))
		| E_NDS_ARCH_STAR_V3_0;
	    }
	  else if ((elf_elfheader (obfd)->e_flags & EF_NDS_ARCH)
		   == E_NDS_ARCH_STAR_V0_9
		   || (elf_elfheader (ibfd)->e_flags & EF_NDS_ARCH)
		   > (elf_elfheader (obfd)->e_flags & EF_NDS_ARCH))
	    {
	      elf_elfheader (obfd)->e_flags =
		convert_e_flags (elf_elfheader (obfd)->e_flags,
				 (elf_elfheader (ibfd)->e_flags & EF_NDS_ARCH));
	    }
	  else
	    {
	      elf_elfheader (ibfd)->e_flags =
		convert_e_flags (elf_elfheader (ibfd)->e_flags,
				 (elf_elfheader (obfd)->e_flags & EF_NDS_ARCH));
	    }
	}

      /* Extract some flags.  */
      in_flags = elf_elfheader (ibfd)->e_flags
	& (~(E_NDS32_HAS_REDUCED_REGS | EF_NDS32_ELF_VERSION
	     | E_NDS32_HAS_NO_MAC_INST | E_NDS32_FPU_REG_CONF));

      /* The following flags need special treatment.  */
      in_16regs = elf_elfheader (ibfd)->e_flags & E_NDS32_HAS_REDUCED_REGS;
      in_no_mac = elf_elfheader (ibfd)->e_flags & E_NDS32_HAS_NO_MAC_INST;
      in_fpu_config = elf_elfheader (ibfd)->e_flags & E_NDS32_FPU_REG_CONF;

      /* Extract some flags.  */
      out_flags = elf_elfheader (obfd)->e_flags
	& (~(E_NDS32_HAS_REDUCED_REGS | EF_NDS32_ELF_VERSION
	     | E_NDS32_HAS_NO_MAC_INST | E_NDS32_FPU_REG_CONF));

      /* The following flags need special treatment.  */
      out_16regs = elf_elfheader (obfd)->e_flags & E_NDS32_HAS_REDUCED_REGS;
      out_no_mac = elf_elfheader (obfd)->e_flags & E_NDS32_HAS_NO_MAC_INST;
      out_fpu_config = elf_elfheader (obfd)->e_flags & E_NDS32_FPU_REG_CONF;
      out_version = elf_elfheader (obfd)->e_flags & EF_NDS32_ELF_VERSION;
      if (!elf_flags_init (obfd))
	{
	  /* If the input is the default architecture then do not
	     bother setting the flags for the output architecture,
	     instead allow future merges to do this.  If no future
	     merges ever set these flags then they will retain their
	     unitialised values, which surprise surprise, correspond
	     to the default values.  */
	  if (bfd_get_arch_info (ibfd)->the_default)
	    return true;

	  elf_flags_init (obfd) = true;
	  elf_elfheader (obfd)->e_flags = elf_elfheader (ibfd)->e_flags;

	  if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
	      && bfd_get_arch_info (obfd)->the_default)
	    {
	      return bfd_set_arch_mach (obfd, bfd_get_arch (ibfd),
					bfd_get_mach (ibfd));
	    }

	  return true;
	}

      /* Check flag compatibility.  */
      if ((in_flags & EF_NDS_ABI) != (out_flags & EF_NDS_ABI))
	{
	  _bfd_error_handler
	    (_("%pB: error: ABI mismatch with previous modules"), ibfd);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      if ((in_flags & EF_NDS_ARCH) != (out_flags & EF_NDS_ARCH))
	{
	  if (((in_flags & EF_NDS_ARCH) != E_N1_ARCH))
	    {
	      _bfd_error_handler
		(_("%pB: error: instruction set mismatch with previous modules"),
		 ibfd);

	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	}

      /* When linking with V1.2 and V1.3 objects together the output is V1.2.
	 and perf ext1 and DIV are mergerd to perf ext1.  */
      if (in_version == E_NDS32_ELF_VER_1_2 || out_version == E_NDS32_ELF_VER_1_2)
	{
	  elf_elfheader (obfd)->e_flags =
	    (in_flags & (~(E_NDS32_HAS_EXT_INST | E_NDS32_HAS_DIV_INST)))
	    | (out_flags & (~(E_NDS32_HAS_EXT_INST | E_NDS32_HAS_DIV_INST)))
	    | (((in_flags & (E_NDS32_HAS_EXT_INST | E_NDS32_HAS_DIV_INST)))
	       ?  E_NDS32_HAS_EXT_INST : 0)
	    | (((out_flags & (E_NDS32_HAS_EXT_INST | E_NDS32_HAS_DIV_INST)))
	       ?  E_NDS32_HAS_EXT_INST : 0)
	    | (in_16regs & out_16regs) | (in_no_mac & out_no_mac)
	    | ((in_version > out_version) ? out_version : in_version);
	}
      else
	{
	  if (in_version != out_version)
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: warning: incompatible elf-versions %s and %s"),
	       ibfd, nds32_elfver_strtab[out_version],
	       nds32_elfver_strtab[in_version]);

	  elf_elfheader (obfd)->e_flags = in_flags | out_flags
	    | (in_16regs & out_16regs) | (in_no_mac & out_no_mac)
	    | (in_fpu_config > out_fpu_config ? in_fpu_config : out_fpu_config)
	    | (in_version > out_version ?  out_version : in_version);
	}
    }

  return true;
}

/* Display the flags field.  */

static bool
nds32_elf_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = (FILE *) ptr;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  _bfd_elf_print_private_bfd_data (abfd, ptr);

  fprintf (file, _("private flags = %lx"), elf_elfheader (abfd)->e_flags);

  switch (elf_elfheader (abfd)->e_flags & EF_NDS_ARCH)
    {
    default:
    case E_N1_ARCH:
      fprintf (file, _(": n1 instructions"));
      break;
    case E_N1H_ARCH:
      fprintf (file, _(": n1h instructions"));
      break;
    }

  fputc ('\n', file);

  return true;
}

static unsigned int
nds32_elf_action_discarded (asection *sec)
{

  if (startswith (sec->name, ".gcc_except_table"))
    return 0;

  return _bfd_elf_default_action_discarded (sec);
}

static asection *
nds32_elf_gc_mark_hook (asection *sec, struct bfd_link_info *info,
			Elf_Internal_Rela *rel, struct elf_link_hash_entry *h,
			Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_NDS32_GNU_VTINHERIT:
      case R_NDS32_GNU_VTENTRY:
      case R_NDS32_RELA_GNU_VTINHERIT:
      case R_NDS32_RELA_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

static enum elf_nds32_tls_type
get_tls_type (enum elf_nds32_reloc_type r_type,
	      struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  enum elf_nds32_tls_type tls_type;

  switch (r_type)
    {
    case R_NDS32_TLS_LE_HI20:
    case R_NDS32_TLS_LE_LO12:
      tls_type = GOT_TLS_LE;
      break;
    case R_NDS32_TLS_IE_HI20:
    case R_NDS32_TLS_IE_LO12S2:
    case R_NDS32_TLS_IE_LO12:
      tls_type = GOT_TLS_IE;
      break;
    case R_NDS32_TLS_IEGP_HI20:
    case R_NDS32_TLS_IEGP_LO12:
    case R_NDS32_TLS_IEGP_LO12S2:
      tls_type = GOT_TLS_IEGP;
      break;
    case R_NDS32_TLS_DESC_HI20:
    case R_NDS32_TLS_DESC_LO12:
    case R_NDS32_TLS_DESC_ADD:
    case R_NDS32_TLS_DESC_FUNC:
    case R_NDS32_TLS_DESC_CALL:
      tls_type = GOT_TLS_DESC;
      break;
    default:
      tls_type = GOT_NORMAL;
      break;
    }

  return tls_type;
}

/* Ensure that we have allocated bookkeeping structures for ABFD's local
   symbols.  */

static bool
elf32_nds32_allocate_local_sym_info (bfd *abfd)
{
  if (elf_local_got_refcounts (abfd) == NULL)
    {
      bfd_size_type num_syms;
      bfd_size_type size;
      char *data;

      num_syms = elf_tdata (abfd)->symtab_hdr.sh_info;
      /* This space is for got_refcounts, got_tls_type, tlsdesc_gotent, and
	 gp_offset.  The details can refer to struct elf_nds32_obj_tdata.  */
      size = num_syms * (sizeof (bfd_signed_vma) + sizeof (char)
			 + sizeof (bfd_vma) + sizeof (int)
			 + sizeof (bool) + sizeof (bfd_vma));
      data = bfd_zalloc (abfd, size);
      if (data == NULL)
	return false;

      elf_local_got_refcounts (abfd) = (bfd_signed_vma *) data;
      data += num_syms * sizeof (bfd_signed_vma);

      elf32_nds32_local_got_tls_type (abfd) = (char *) data;
      data += num_syms * sizeof (char);

      elf32_nds32_local_tlsdesc_gotent (abfd) = (bfd_vma *) data;
      data += num_syms * sizeof (bfd_vma);

      elf32_nds32_local_gp_offset (abfd) = (int *) data;
      data += num_syms * sizeof (int);
    }

  return true;
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

static bool
nds32_elf_check_relocs (bfd *abfd, struct bfd_link_info *info,
			asection *sec, const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  struct elf_link_hash_table *ehtab;
  struct elf_nds32_link_hash_table *htab;
  bfd *dynobj;
  asection *sreloc = NULL;

  /* No need for relocation if relocatable already.  */
  if (bfd_link_relocatable (info))
    {
      elf32_nds32_check_relax_group (abfd, sec);
      return true;
    }

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  ehtab = elf_hash_table (info);
  htab = nds32_elf_hash_table (info);
  dynobj = htab->root.dynobj;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      enum elf_nds32_reloc_type r_type;
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;
      enum elf_nds32_tls_type tls_type, old_tls_type;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      /* Create .got section if necessary.
	 Some relocs require a global offset table.  We create
	 got section here, since these relocation need a got section
	 and if it is not created yet.  */
      if (ehtab->sgot == NULL)
	{
	  switch (r_type)
	    {
	    case R_NDS32_GOT_HI20:
	    case R_NDS32_GOT_LO12:
	    case R_NDS32_GOT_LO15:
	    case R_NDS32_GOT_LO19:
	    case R_NDS32_GOT17S2_RELA:
	    case R_NDS32_GOT15S2_RELA:
	    case R_NDS32_GOTOFF:
	    case R_NDS32_GOTOFF_HI20:
	    case R_NDS32_GOTOFF_LO12:
	    case R_NDS32_GOTOFF_LO15:
	    case R_NDS32_GOTOFF_LO19:
	    case R_NDS32_GOTPC20:
	    case R_NDS32_GOTPC_HI20:
	    case R_NDS32_GOTPC_LO12:
	    case R_NDS32_GOT20:
	    case R_NDS32_TLS_IE_HI20:
	    case R_NDS32_TLS_IE_LO12:
	    case R_NDS32_TLS_IE_LO12S2:
	    case R_NDS32_TLS_IEGP_HI20:
	    case R_NDS32_TLS_IEGP_LO12:
	    case R_NDS32_TLS_IEGP_LO12S2:
	    case R_NDS32_TLS_DESC_HI20:
	    case R_NDS32_TLS_DESC_LO12:
	      if (dynobj == NULL)
		htab->root.dynobj = dynobj = abfd;
	      if (!create_got_section (dynobj, info))
		return false;
	      break;

	    default:
	      break;
	    }
	}

      /* Check relocation type.  */
      switch ((int) r_type)
	{
	case R_NDS32_GOT_HI20:
	case R_NDS32_GOT_LO12:
	case R_NDS32_GOT_LO15:
	case R_NDS32_GOT_LO19:
	case R_NDS32_GOT20:
	case R_NDS32_TLS_LE_HI20:
	case R_NDS32_TLS_LE_LO12:
	case R_NDS32_TLS_IE_HI20:
	case R_NDS32_TLS_IE_LO12:
	case R_NDS32_TLS_IE_LO12S2:
	case R_NDS32_TLS_IEGP_HI20:
	case R_NDS32_TLS_IEGP_LO12:
	case R_NDS32_TLS_IEGP_LO12S2:
	case R_NDS32_TLS_DESC_HI20:
	case R_NDS32_TLS_DESC_LO12:
	  tls_type = get_tls_type (r_type, h);
	  if (h)
	    {
	      if (tls_type != GOT_TLS_LE)
		h->got.refcount += 1;
	      old_tls_type = elf32_nds32_hash_entry (h)->tls_type;
	    }
	  else
	    {
	      /* This is a global offset table entry for a local symbol.  */
	      if (!elf32_nds32_allocate_local_sym_info (abfd))
		return false;

	      BFD_ASSERT (r_symndx < symtab_hdr->sh_info);
	      if (tls_type != GOT_TLS_LE)
		elf_local_got_refcounts (abfd)[r_symndx] += 1;
	      old_tls_type = elf32_nds32_local_got_tls_type (abfd)[r_symndx];
	    }

	  /* We would already have issued an error message if there
	     is a TLS/non-TLS mismatch, based on the symbol
	     type.  So just combine any TLS types needed.  */
	  if (old_tls_type != GOT_UNKNOWN && old_tls_type != GOT_NORMAL
	      && tls_type != GOT_NORMAL)
	    tls_type |= old_tls_type;

	  /* DESC to IE/IEGP if link to executable.  */
	  if ((tls_type & (GOT_TLS_DESC | GOT_TLS_IEGP))
	      && (bfd_link_executable (info)))
	    tls_type |= (bfd_link_pie (info) ? GOT_TLS_IEGP : GOT_TLS_IE);

	  if (old_tls_type != tls_type)
	    {
	      if (h != NULL)
		elf32_nds32_hash_entry (h)->tls_type = tls_type;
	      else
		elf32_nds32_local_got_tls_type (abfd)[r_symndx] = tls_type;
	    }
	  break;
	case R_NDS32_9_PLTREL:
	case R_NDS32_25_PLTREL:
	case R_NDS32_PLTREL_HI20:
	case R_NDS32_PLTREL_LO12:
	case R_NDS32_PLT_GOTREL_HI20:
	case R_NDS32_PLT_GOTREL_LO12:
	case R_NDS32_PLT_GOTREL_LO15:
	case R_NDS32_PLT_GOTREL_LO19:
	case R_NDS32_PLT_GOTREL_LO20:

	  /* This symbol requires a procedure linkage table entry.  We
	     actually build the entry in adjust_dynamic_symbol,
	     because this might be a case of linking PIC code without
	     linking in any dynamic objects, in which case we don't
	     need to generate a procedure linkage table after all.  */

	  /* If this is a local symbol, we resolve it directly without
	     creating a procedure linkage table entry.  */
	  if (h == NULL)
	    continue;

	  if (h->forced_local
	      || (bfd_link_pie (info) && h->def_regular))
	    break;

	  elf32_nds32_hash_entry (h)->tls_type = GOT_NORMAL;
	  h->needs_plt = 1;
	  h->plt.refcount += 1;
	  break;

	case R_NDS32_16_RELA:
	case R_NDS32_20_RELA:
	case R_NDS32_5_RELA:
	case R_NDS32_32_RELA:
	case R_NDS32_HI20_RELA:
	case R_NDS32_LO12S3_RELA:
	case R_NDS32_LO12S2_RELA:
	case R_NDS32_LO12S2_DP_RELA:
	case R_NDS32_LO12S2_SP_RELA:
	case R_NDS32_LO12S1_RELA:
	case R_NDS32_LO12S0_RELA:
	case R_NDS32_LO12S0_ORI_RELA:
	case R_NDS32_SDA16S3_RELA:
	case R_NDS32_SDA17S2_RELA:
	case R_NDS32_SDA18S1_RELA:
	case R_NDS32_SDA19S0_RELA:
	case R_NDS32_SDA15S3_RELA:
	case R_NDS32_SDA15S2_RELA:
	case R_NDS32_SDA12S2_DP_RELA:
	case R_NDS32_SDA12S2_SP_RELA:
	case R_NDS32_SDA15S1_RELA:
	case R_NDS32_SDA15S0_RELA:
	case R_NDS32_SDA_FP7U2_RELA:
	case R_NDS32_15_PCREL_RELA:
	case R_NDS32_17_PCREL_RELA:
	case R_NDS32_25_PCREL_RELA:

	  if (h != NULL && !bfd_link_pic (info))
	    {
	      h->non_got_ref = 1;
	      h->plt.refcount += 1;
	    }

	  /* If we are creating a shared library, and this is a reloc against
	     a global symbol, or a non PC relative reloc against a local
	     symbol, then we need to copy the reloc into the shared library.
	     However, if we are linking with -Bsymbolic, we do not need to
	     copy a reloc against a global symbol which is defined in an
	     object we are including in the link (i.e., DEF_REGULAR is set).
	     At this point we have not seen all the input files, so it is
	     possible that DEF_REGULAR is not set now but will be set later
	     (it is never cleared).  We account for that possibility below by
	     storing information in the dyn_relocs field of the hash table
	     entry.  A similar situation occurs when creating shared libraries
	     and symbol visibility changes render the symbol local.

	     If on the other hand, we are creating an executable, we may need
	     to keep relocations for symbols satisfied by a dynamic library
	     if we manage to avoid copy relocs for the symbol.  */
	  if ((bfd_link_pic (info)
	       && (sec->flags & SEC_ALLOC) != 0
	       && ((r_type != R_NDS32_25_PCREL_RELA
		    && r_type != R_NDS32_15_PCREL_RELA
		    && r_type != R_NDS32_17_PCREL_RELA
		    && !(r_type == R_NDS32_32_RELA
			 && strcmp (sec->name, ".eh_frame") == 0))
		   || (h != NULL
		       && (!info->symbolic
			   || h->root.type == bfd_link_hash_defweak
			   || !h->def_regular))))
	      || (!bfd_link_pic (info)
		  && (sec->flags & SEC_ALLOC) != 0
		  && h != NULL
		  && (h->root.type == bfd_link_hash_defweak
		      || !h->def_regular)))
	    {
	      struct elf_dyn_relocs *p;
	      struct elf_dyn_relocs **head;

	      if (dynobj == NULL)
		htab->root.dynobj = dynobj = abfd;

	      /* When creating a shared object, we must copy these
		 relocs into the output file.  We create a reloc
		 section in dynobj and make room for the reloc.  */
	      if (sreloc == NULL)
		{
		  const char *name;

		  name = bfd_elf_string_from_elf_section
		    (abfd, elf_elfheader (abfd)->e_shstrndx,
		     elf_section_data (sec)->rela.hdr->sh_name);
		  if (name == NULL)
		    return false;

		  BFD_ASSERT (startswith (name, ".rela")
			      && strcmp (bfd_section_name (sec),
					 name + 5) == 0);

		  sreloc = bfd_get_section_by_name (dynobj, name);
		  if (sreloc == NULL)
		    {
		      flagword flags;

		      sreloc = bfd_make_section (dynobj, name);
		      flags = (SEC_HAS_CONTENTS | SEC_READONLY
			       | SEC_IN_MEMORY | SEC_LINKER_CREATED);
		      if ((sec->flags & SEC_ALLOC) != 0)
			flags |= SEC_ALLOC | SEC_LOAD;
		      if (sreloc == NULL
			  || !bfd_set_section_flags (sreloc, flags)
			  || !bfd_set_section_alignment (sreloc, 2))
			return false;

		      elf_section_type (sreloc) = SHT_RELA;
		    }
		  elf_section_data (sec)->sreloc = sreloc;
		}

	      /* If this is a global symbol, we count the number of
		 relocations we need for this symbol.  */
	      if (h != NULL)
		head = &h->dyn_relocs;
	      else
		{
		  asection *s;
		  void *vpp;

		  Elf_Internal_Sym *isym;
		  isym = bfd_sym_from_r_symndx (&htab->root.sym_cache,
						abfd, r_symndx);
		  if (isym == NULL)
		    return false;

		  /* Track dynamic relocs needed for local syms too.  */
		  s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		  if (s == NULL)
		    return false;

		  vpp = &elf_section_data (s)->local_dynrel;
		  head = (struct elf_dyn_relocs **) vpp;
		}

	      p = *head;
	      if (p == NULL || p->sec != sec)
		{
		  size_t amt = sizeof (*p);
		  p = (struct elf_dyn_relocs *) bfd_alloc (dynobj, amt);
		  if (p == NULL)
		    return false;
		  p->next = *head;
		  *head = p;
		  p->sec = sec;
		  p->count = 0;
		  p->pc_count = 0;
		}

	      p->count += 1;

	      /* Since eh_frame is readonly, R_NDS32_32_RELA
		 reloc for eh_frame will cause shared library has
		 TEXTREL entry in the dynamic section. This lead glibc
		 testsuites to failure (bug-13092) and cause kernel fail
		 (bug-11819).  I think the best solution is to replace
		 absolute reloc with pc relative reloc in the eh_frame.
		 To do that, we need to support the following issues:

		 === For GCC ===
		 * gcc/config/nds32/nds32.h: Define
		 ASM_PREFERRED_EH_DATA_FORMAT to encode DW_EH_PE_pcrel
		 and DW_EH_PE_sdata4 into DWARF exception header when
		 option have '-fpic'.

		 === For binutils ===
		 * bfd/: Define new reloc R_NDS32_32_PCREL_RELA.
		 * gas/config/tc-nds32.h: Define DIFF_EXPR_OK. This
		 may break our nds DIFF mechanism, therefore, we
		 must disable all linker relaxations to ensure
		 correctness.
		 * gas/config/tc-nds32.c (nds32_apply_fix): Replace
		 R_NDS32_32_RELA with R_NDS32_32_PCREL_RELA, and
		 do the necessary modification.

		 Unfortunately, it still have some problems for nds32
		 to support pc relative reloc in the eh_frame. So I use
		 another solution to fix this issue.

		 However, I find that ld always emit TEXTREL marker for
		 R_NDS32_NONE relocs in rel.dyn. These none relocs are
		 correspond to R_NDS32_32_RELA for .eh_frame section.
		 It means that we always reserve redundant entries of rel.dyn
		 for these relocs which actually do nothing in dynamic linker.

		 Therefore, we regard these relocs as pc relative relocs
		 here and increase the pc_count.  */
	      if (ELF32_R_TYPE (rel->r_info) == R_NDS32_25_PCREL_RELA
		  || ELF32_R_TYPE (rel->r_info) == R_NDS32_15_PCREL_RELA
		  || ELF32_R_TYPE (rel->r_info) == R_NDS32_17_PCREL_RELA
		  || (r_type == R_NDS32_32_RELA
		      && strcmp (sec->name, ".eh_frame") == 0))
		p->pc_count += 1;
	    }
	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_NDS32_RELA_GNU_VTINHERIT:
	case R_NDS32_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_NDS32_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_offset))
	    return false;
	  break;
	case R_NDS32_RELA_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;
	}
    }

  return true;
}

/* Write VAL in uleb128 format to P, returning a pointer to the
   following byte.
   This code is copied from elf-attr.c.  */

static bfd_byte *
write_uleb128 (bfd_byte *p, unsigned int val)
{
  bfd_byte c;
  do
    {
      c = val & 0x7f;
      val >>= 7;
      if (val)
	c |= 0x80;
      *(p++) = c;
    }
  while (val);
  return p;
}

static bfd_signed_vma
calculate_offset (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
		  Elf_Internal_Sym *isymbuf, Elf_Internal_Shdr *symtab_hdr)
{
  bfd_signed_vma foff;
  bfd_vma symval, addend;
  asection *sym_sec;

  /* Get the value of the symbol referred to by the reloc.  */
  if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
    {
      Elf_Internal_Sym *isym;

      /* A local symbol.  */
      isym = isymbuf + ELF32_R_SYM (irel->r_info);

      if (isym->st_shndx == SHN_UNDEF)
	sym_sec = bfd_und_section_ptr;
      else if (isym->st_shndx == SHN_ABS)
	sym_sec = bfd_abs_section_ptr;
      else if (isym->st_shndx == SHN_COMMON)
	sym_sec = bfd_com_section_ptr;
      else
	sym_sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
      symval = isym->st_value + sym_sec->output_section->vma
	       + sym_sec->output_offset;
    }
  else
    {
      unsigned long indx;
      struct elf_link_hash_entry *h;

      /* An external symbol.  */
      indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
      h = elf_sym_hashes (abfd)[indx];
      BFD_ASSERT (h != NULL);

      if (h->root.type != bfd_link_hash_defined
	  && h->root.type != bfd_link_hash_defweak)
	/* This appears to be a reference to an undefined
	   symbol.  Just ignore it--it will be caught by the
	   regular reloc processing.  */
	return 0;

      if (h->root.u.def.section->flags & SEC_MERGE)
	{
	  sym_sec = h->root.u.def.section;
	  symval = _bfd_merged_section_offset (abfd, &sym_sec,
					       elf_section_data (sym_sec)->sec_info,
					       h->root.u.def.value);
	  symval = symval + sym_sec->output_section->vma
		   + sym_sec->output_offset;
	}
      else
	symval = (h->root.u.def.value
		  + h->root.u.def.section->output_section->vma
		  + h->root.u.def.section->output_offset);
    }

  addend = irel->r_addend;

  foff = (symval + addend
	  - (irel->r_offset + sec->output_section->vma + sec->output_offset));
  return foff;
}


/* Convert a 32-bit instruction to 16-bit one.
   INSN is the input 32-bit instruction, INSN16 is the output 16-bit
   instruction.  If INSN_TYPE is not NULL, it the CGEN instruction
   type of INSN16.  Return 1 if successful.  */

static int
nds32_convert_32_to_16_alu1 (bfd *abfd, uint32_t insn, uint16_t *pinsn16,
			     int *pinsn_type)
{
  uint16_t insn16 = 0;
  int insn_type = 0;
  unsigned long mach = bfd_get_mach (abfd);

  if (N32_SH5 (insn) != 0)
    return 0;

  switch (N32_SUB5 (insn))
    {
    case N32_ALU1_ADD_SLLI:
    case N32_ALU1_ADD_SRLI:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn) && N32_IS_RB3 (insn))
	{
	  insn16 = N16_TYPE333 (ADD333, N32_RT5 (insn), N32_RA5 (insn),
				N32_RB5 (insn));
	  insn_type = NDS32_INSN_ADD333;
	}
      else if (N32_IS_RT4 (insn))
	{
	  if (N32_RT5 (insn) == N32_RA5 (insn))
	    insn16 = N16_TYPE45 (ADD45, N32_RT54 (insn), N32_RB5 (insn));
	  else if (N32_RT5 (insn) == N32_RB5 (insn))
	    insn16 = N16_TYPE45 (ADD45, N32_RT54 (insn), N32_RA5 (insn));
	  insn_type = NDS32_INSN_ADD45;
	}
      break;

    case N32_ALU1_SUB_SLLI:
    case N32_ALU1_SUB_SRLI:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn) && N32_IS_RB3 (insn))
	{
	  insn16 = N16_TYPE333 (SUB333, N32_RT5 (insn), N32_RA5 (insn),
				N32_RB5 (insn));
	  insn_type = NDS32_INSN_SUB333;
	}
      else if (N32_IS_RT4 (insn) && N32_RT5 (insn) == N32_RA5 (insn))
	{
	  insn16 = N16_TYPE45 (SUB45, N32_RT54 (insn), N32_RB5 (insn));
	  insn_type = NDS32_INSN_SUB45;
	}
      break;

    case N32_ALU1_AND_SLLI:
    case N32_ALU1_AND_SRLI:
      /* and $rt, $rt, $rb -> and33 for v3, v3m.  */
      if (mach >= MACH_V3 && N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	  && N32_IS_RB3 (insn))
	{
	  if (N32_RT5 (insn) == N32_RA5 (insn))
	    insn16 = N16_MISC33 (AND33, N32_RT5 (insn), N32_RB5 (insn));
	  else if (N32_RT5 (insn) == N32_RB5 (insn))
	    insn16 = N16_MISC33 (AND33, N32_RT5 (insn), N32_RA5 (insn));
	  if (insn16)
	    insn_type = NDS32_INSN_AND33;
	}
      break;

    case N32_ALU1_XOR_SLLI:
    case N32_ALU1_XOR_SRLI:
      /* xor $rt, $rt, $rb -> xor33 for v3, v3m.  */
      if (mach >= MACH_V3 && N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	  && N32_IS_RB3 (insn))
	{
	  if (N32_RT5 (insn) == N32_RA5 (insn))
	    insn16 = N16_MISC33 (XOR33, N32_RT5 (insn), N32_RB5 (insn));
	  else if (N32_RT5 (insn) == N32_RB5 (insn))
	    insn16 = N16_MISC33 (XOR33, N32_RT5 (insn), N32_RA5 (insn));
	  if (insn16)
	    insn_type = NDS32_INSN_XOR33;
	}
      break;

    case N32_ALU1_OR_SLLI:
    case N32_ALU1_OR_SRLI:
      /* or $rt, $rt, $rb -> or33 for v3, v3m.  */
      if (mach >= MACH_V3 && N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	  && N32_IS_RB3 (insn))
	{
	  if (N32_RT5 (insn) == N32_RA5 (insn))
	    insn16 = N16_MISC33 (OR33, N32_RT5 (insn), N32_RB5 (insn));
	  else if (N32_RT5 (insn) == N32_RB5 (insn))
	    insn16 = N16_MISC33 (OR33, N32_RT5 (insn), N32_RA5 (insn));
	  if (insn16)
	    insn_type = NDS32_INSN_OR33;
	}
      break;
    case N32_ALU1_NOR:
      /* nor $rt, $ra, $ra -> not33 for v3, v3m.  */
      if (mach >= MACH_V3 && N32_IS_RT3 (insn) && N32_IS_RB3 (insn)
	  && N32_RA5 (insn) == N32_RB5 (insn))
	{
	  insn16 = N16_MISC33 (NOT33, N32_RT5 (insn), N32_RA5 (insn));
	  insn_type = NDS32_INSN_NOT33;
	}
      break;
    case N32_ALU1_SRAI:
      if (N32_IS_RT4 (insn) && N32_RT5 (insn) == N32_RA5 (insn))
	{
	  insn16 = N16_TYPE45 (SRAI45, N32_RT54 (insn), N32_UB5 (insn));
	  insn_type = NDS32_INSN_SRAI45;
	}
      break;

    case N32_ALU1_SRLI:
      if (N32_IS_RT4 (insn) && N32_RT5 (insn) == N32_RA5 (insn))
	{
	  insn16 = N16_TYPE45 (SRLI45, N32_RT54 (insn), N32_UB5 (insn));
	  insn_type = NDS32_INSN_SRLI45;
	}
      break;

    case N32_ALU1_SLLI:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn) && N32_UB5 (insn) < 8)
	{
	  insn16 = N16_TYPE333 (SLLI333, N32_RT5 (insn), N32_RA5 (insn),
				N32_UB5 (insn));
	  insn_type = NDS32_INSN_SLLI333;
	}
      break;

    case N32_ALU1_ZEH:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn))
	{
	  insn16 = N16_BFMI333 (ZEH33, N32_RT5 (insn), N32_RA5 (insn));
	  insn_type = NDS32_INSN_ZEH33;
	}
      break;

    case N32_ALU1_SEB:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn))
	{
	  insn16 = N16_BFMI333 (SEB33, N32_RT5 (insn), N32_RA5 (insn));
	  insn_type = NDS32_INSN_SEB33;
	}
      break;

    case N32_ALU1_SEH:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn))
	{
	  insn16 = N16_BFMI333 (SEH33, N32_RT5 (insn), N32_RA5 (insn));
	  insn_type = NDS32_INSN_SEH33;
	}
      break;

    case N32_ALU1_SLT:
      if (N32_RT5 (insn) == REG_R15 && N32_IS_RA4 (insn))
	{
	  /* Implicit r15.  */
	  insn16 = N16_TYPE45 (SLT45, N32_RA54 (insn), N32_RB5 (insn));
	  insn_type = NDS32_INSN_SLT45;
	}
      break;

    case N32_ALU1_SLTS:
      if (N32_RT5 (insn) == REG_R15 && N32_IS_RA4 (insn))
	{
	  /* Implicit r15.  */
	  insn16 = N16_TYPE45 (SLTS45, N32_RA54 (insn), N32_RB5 (insn));
	  insn_type = NDS32_INSN_SLTS45;
	}
      break;
    }

  if ((insn16 & 0x8000) == 0)
    return 0;

  if (pinsn16)
    *pinsn16 = insn16;
  if (pinsn_type)
    *pinsn_type = insn_type;
  return 1;
}

static int
nds32_convert_32_to_16_alu2 (bfd *abfd, uint32_t insn, uint16_t *pinsn16,
			     int *pinsn_type)
{
  uint16_t insn16 = 0;
  int insn_type;
  unsigned long mach = bfd_get_mach (abfd);

  /* TODO: bset, bclr, btgl, btst.  */
  if (__GF (insn, 6, 4) != 0)
    return 0;

  switch (N32_IMMU (insn, 6))
    {
    case N32_ALU2_MUL:
      if (mach >= MACH_V3 && N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	  && N32_IS_RB3 (insn))
	{
	  if (N32_RT5 (insn) == N32_RA5 (insn))
	    insn16 = N16_MISC33 (MUL33, N32_RT5 (insn), N32_RB5 (insn));
	  else if (N32_RT5 (insn) == N32_RB5 (insn))
	    insn16 = N16_MISC33 (MUL33, N32_RT5 (insn), N32_RA5 (insn));
	  if (insn16)
	    insn_type = NDS32_INSN_MUL33;
	}
    }

  if ((insn16 & 0x8000) == 0)
    return 0;

  if (pinsn16)
    *pinsn16 = insn16;
  if (pinsn_type)
    *pinsn_type = insn_type;
  return 1;
}

int
nds32_convert_32_to_16 (bfd *abfd, uint32_t insn, uint16_t *pinsn16,
			int *pinsn_type)
{
  int op6;
  uint16_t insn16 = 0;
  int insn_type = 0;
  unsigned long mach = bfd_get_mach (abfd);

  /* Decode 32-bit instruction.  */
  if (insn & 0x80000000)
    {
      /* Not 32-bit insn.  */
      return 0;
    }

  op6 = N32_OP6 (insn);

  /* Convert it to 16-bit instruction.  */
  switch (op6)
    {
    case N32_OP6_MOVI:
      if (IS_WITHIN_S (N32_IMM20S (insn), 5))
	{
	  insn16 = N16_TYPE55 (MOVI55, N32_RT5 (insn), N32_IMM20S (insn));
	  insn_type = NDS32_INSN_MOVI55;
	}
      else if (mach >= MACH_V3 && N32_IMM20S (insn) >= 16
	       && N32_IMM20S (insn) < 48 && N32_IS_RT4 (insn))
	{
	  insn16 = N16_TYPE45 (MOVPI45, N32_RT54 (insn),
			       N32_IMM20S (insn) - 16);
	  insn_type = NDS32_INSN_MOVPI45;
	}
      break;

    case N32_OP6_ADDI:
      if (N32_IMM15S (insn) == 0)
	{
	  /* Do not convert `addi $sp, $sp, 0' to `mov55 $sp, $sp',
	     because `mov55 $sp, $sp' is ifret16 in V3 ISA.  */
	  if (mach <= MACH_V2
	      || N32_RT5 (insn) != REG_SP || N32_RA5 (insn) != REG_SP)
	    {
	      insn16 = N16_TYPE55 (MOV55, N32_RT5 (insn), N32_RA5 (insn));
	      insn_type = NDS32_INSN_MOV55;
	    }
	}
      else if (N32_IMM15S (insn) > 0)
	{
	  if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn) && N32_IMM15S (insn) < 8)
	    {
	      insn16 = N16_TYPE333 (ADDI333, N32_RT5 (insn), N32_RA5 (insn),
				    N32_IMM15S (insn));
	      insn_type = NDS32_INSN_ADDI333;
	    }
	  else if (N32_IS_RT4 (insn) && N32_RT5 (insn) == N32_RA5 (insn)
		   && N32_IMM15S (insn) < 32)
	    {
	      insn16 = N16_TYPE45 (ADDI45, N32_RT54 (insn), N32_IMM15S (insn));
	      insn_type = NDS32_INSN_ADDI45;
	    }
	  else if (mach >= MACH_V2 && N32_RT5 (insn) == REG_SP
		   && N32_RT5 (insn) == N32_RA5 (insn)
		   && N32_IMM15S (insn) < 512)
	    {
	      insn16 = N16_TYPE10 (ADDI10S, N32_IMM15S (insn));
	      insn_type = NDS32_INSN_ADDI10_SP;
	    }
	  else if (mach >= MACH_V3 && N32_IS_RT3 (insn)
		   && N32_RA5 (insn) == REG_SP && N32_IMM15S (insn) < 256
		   && (N32_IMM15S (insn) % 4 == 0))
	    {
	      insn16 = N16_TYPE36 (ADDRI36_SP, N32_RT5 (insn),
				   N32_IMM15S (insn) >> 2);
	      insn_type = NDS32_INSN_ADDRI36_SP;
	    }
	}
      else
	{
	  /* Less than 0.  */
	  if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn) && N32_IMM15S (insn) > -8)
	    {
	      insn16 = N16_TYPE333 (SUBI333, N32_RT5 (insn), N32_RA5 (insn),
				    0 - N32_IMM15S (insn));
	      insn_type = NDS32_INSN_SUBI333;
	    }
	  else if (N32_IS_RT4 (insn) && N32_RT5 (insn) == N32_RA5 (insn)
		   && N32_IMM15S (insn) > -32)
	    {
	      insn16 = N16_TYPE45 (SUBI45, N32_RT54 (insn),
				   0 - N32_IMM15S (insn));
	      insn_type = NDS32_INSN_SUBI45;
	    }
	  else if (mach >= MACH_V2 && N32_RT5 (insn) == REG_SP
		   && N32_RT5 (insn) == N32_RA5 (insn)
		   && N32_IMM15S (insn) >= -512)
	    {
	      insn16 = N16_TYPE10 (ADDI10S, N32_IMM15S (insn));
	      insn_type = NDS32_INSN_ADDI10_SP;
	    }
	}
      break;

    case N32_OP6_ORI:
      if (N32_IMM15S (insn) == 0)
	{
	  /* Do not convert `ori $sp, $sp, 0' to `mov55 $sp, $sp',
	     because `mov55 $sp, $sp' is ifret16 in V3 ISA.  */
	  if (mach <= MACH_V2
	      || N32_RT5 (insn) != REG_SP || N32_RA5 (insn) != REG_SP)
	    {
	      insn16 = N16_TYPE55 (MOV55, N32_RT5 (insn), N32_RA5 (insn));
	      insn_type = NDS32_INSN_MOV55;
	    }
	}
      break;

    case N32_OP6_SUBRI:
      if (mach >= MACH_V3 && N32_IS_RT3 (insn)
	  && N32_IS_RA3 (insn) && N32_IMM15S (insn) == 0)
	{
	  insn16 = N16_MISC33 (NEG33, N32_RT5 (insn), N32_RA5 (insn));
	  insn_type = NDS32_INSN_NEG33;
	}
      break;

    case N32_OP6_ANDI:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn))
	{
	  if (N32_IMM15U (insn) == 1)
	    {
	      insn16 = N16_BFMI333 (XLSB33, N32_RT5 (insn), N32_RA5 (insn));
	      insn_type = NDS32_INSN_XLSB33;
	    }
	  else if (N32_IMM15U (insn) == 0x7ff)
	    {
	      insn16 = N16_BFMI333 (X11B33, N32_RT5 (insn), N32_RA5 (insn));
	      insn_type = NDS32_INSN_X11B33;
	    }
	  else if (N32_IMM15U (insn) == 0xff)
	    {
	      insn16 = N16_BFMI333 (ZEB33, N32_RT5 (insn), N32_RA5 (insn));
	      insn_type = NDS32_INSN_ZEB33;
	    }
	  else if (mach >= MACH_V3 && N32_RT5 (insn) == N32_RA5 (insn)
		   && N32_IMM15U (insn) < 256)
	    {
	      int imm15u = N32_IMM15U (insn);

	      if (__builtin_popcount (imm15u) == 1)
		{
		  /* BMSKI33 */
		  int imm3u = __builtin_ctz (imm15u);

		  insn16 = N16_BFMI333 (BMSKI33, N32_RT5 (insn), imm3u);
		  insn_type = NDS32_INSN_BMSKI33;
		}
	      else if (imm15u != 0 && __builtin_popcount (imm15u + 1) == 1)
		{
		  /* FEXTI33 */
		  int imm3u = __builtin_ctz (imm15u + 1) - 1;

		  insn16 = N16_BFMI333 (FEXTI33, N32_RT5 (insn), imm3u);
		  insn_type = NDS32_INSN_FEXTI33;
		}
	    }
	}
      break;

    case N32_OP6_SLTI:
      if (N32_RT5 (insn) == REG_R15 && N32_IS_RA4 (insn)
	  && IS_WITHIN_U (N32_IMM15S (insn), 5))
	{
	  insn16 = N16_TYPE45 (SLTI45, N32_RA54 (insn), N32_IMM15S (insn));
	  insn_type = NDS32_INSN_SLTI45;
	}
      break;

    case N32_OP6_SLTSI:
      if (N32_RT5 (insn) == REG_R15 && N32_IS_RA4 (insn)
	  && IS_WITHIN_U (N32_IMM15S (insn), 5))
	{
	  insn16 = N16_TYPE45 (SLTSI45, N32_RA54 (insn), N32_IMM15S (insn));
	  insn_type = NDS32_INSN_SLTSI45;
	}
      break;

    case N32_OP6_LWI:
      if (N32_IS_RT4 (insn) && N32_IMM15S (insn) == 0)
	{
	  insn16 = N16_TYPE45 (LWI450, N32_RT54 (insn), N32_RA5 (insn));
	  insn_type = NDS32_INSN_LWI450;
	}
      else if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	       && IS_WITHIN_U (N32_IMM15S (insn), 3))
	{
	  insn16 = N16_TYPE333 (LWI333, N32_RT5 (insn), N32_RA5 (insn),
				N32_IMM15S (insn));
	  insn_type = NDS32_INSN_LWI333;
	}
      else if (N32_IS_RT3 (insn) && N32_RA5 (insn) == REG_FP
	       && IS_WITHIN_U (N32_IMM15S (insn), 7))
	{
	  insn16 = N16_TYPE37 (XWI37, N32_RT5 (insn), 0, N32_IMM15S (insn));
	  insn_type = NDS32_INSN_LWI37;
	}
      else if (mach >= MACH_V2 && N32_IS_RT3 (insn) && N32_RA5 (insn) == REG_SP
	       && IS_WITHIN_U (N32_IMM15S (insn), 7))
	{
	  insn16 = N16_TYPE37 (XWI37SP, N32_RT5 (insn), 0, N32_IMM15S (insn));
	  insn_type = NDS32_INSN_LWI37_SP;
	}
      else if (mach >= MACH_V2 && N32_IS_RT4 (insn) && N32_RA5 (insn) == REG_R8
	       && -32 <= N32_IMM15S (insn) && N32_IMM15S (insn) < 0)
	{
	  insn16 = N16_TYPE45 (LWI45_FE, N32_RT54 (insn),
			       N32_IMM15S (insn) + 32);
	  insn_type = NDS32_INSN_LWI45_FE;
	}
      break;

    case N32_OP6_SWI:
      if (N32_IS_RT4 (insn) && N32_IMM15S (insn) == 0)
	{
	  insn16 = N16_TYPE45 (SWI450, N32_RT54 (insn), N32_RA5 (insn));
	  insn_type = NDS32_INSN_SWI450;
	}
      else if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	       && IS_WITHIN_U (N32_IMM15S (insn), 3))
	{
	  insn16 = N16_TYPE333 (SWI333, N32_RT5 (insn), N32_RA5 (insn),
				N32_IMM15S (insn));
	  insn_type = NDS32_INSN_SWI333;
	}
      else if (N32_IS_RT3 (insn) && N32_RA5 (insn) == REG_FP
	       && IS_WITHIN_U (N32_IMM15S (insn), 7))
	{
	  insn16 = N16_TYPE37 (XWI37, N32_RT5 (insn), 1, N32_IMM15S (insn));
	  insn_type = NDS32_INSN_SWI37;
	}
      else if (mach >= MACH_V2 && N32_IS_RT3 (insn) && N32_RA5 (insn) == REG_SP
	       && IS_WITHIN_U (N32_IMM15S (insn), 7))
	{
	  insn16 = N16_TYPE37 (XWI37SP, N32_RT5 (insn), 1, N32_IMM15S (insn));
	  insn_type = NDS32_INSN_SWI37_SP;
	}
      break;

    case N32_OP6_LWI_BI:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	  && IS_WITHIN_U (N32_IMM15S (insn), 3))
	{
	  insn16 = N16_TYPE333 (LWI333_BI, N32_RT5 (insn), N32_RA5 (insn),
				N32_IMM15S (insn));
	  insn_type = NDS32_INSN_LWI333_BI;
	}
      break;

    case N32_OP6_SWI_BI:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	  && IS_WITHIN_U (N32_IMM15S (insn), 3))
	{
	  insn16 = N16_TYPE333 (SWI333_BI, N32_RT5 (insn), N32_RA5 (insn),
				N32_IMM15S (insn));
	  insn_type = NDS32_INSN_SWI333_BI;
	}
      break;

    case N32_OP6_LHI:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	  && IS_WITHIN_U (N32_IMM15S (insn), 3))
	{
	  insn16 = N16_TYPE333 (LHI333, N32_RT5 (insn), N32_RA5 (insn),
				N32_IMM15S (insn));
	  insn_type = NDS32_INSN_LHI333;
	}
      break;

    case N32_OP6_SHI:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	  && IS_WITHIN_U (N32_IMM15S (insn), 3))
	{
	  insn16 = N16_TYPE333 (SHI333, N32_RT5 (insn), N32_RA5 (insn),
				N32_IMM15S (insn));
	  insn_type = NDS32_INSN_SHI333;
	}
      break;

    case N32_OP6_LBI:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	  && IS_WITHIN_U (N32_IMM15S (insn), 3))
	{
	  insn16 = N16_TYPE333 (LBI333, N32_RT5 (insn), N32_RA5 (insn),
				N32_IMM15S (insn));
	  insn_type = NDS32_INSN_LBI333;
	}
      break;

    case N32_OP6_SBI:
      if (N32_IS_RT3 (insn) && N32_IS_RA3 (insn)
	  && IS_WITHIN_U (N32_IMM15S (insn), 3))
	{
	  insn16 = N16_TYPE333 (SBI333, N32_RT5 (insn), N32_RA5 (insn),
				N32_IMM15S (insn));
	  insn_type = NDS32_INSN_SBI333;
	}
      break;

    case N32_OP6_ALU1:
      return nds32_convert_32_to_16_alu1 (abfd, insn, pinsn16, pinsn_type);

    case N32_OP6_ALU2:
      return nds32_convert_32_to_16_alu2 (abfd, insn, pinsn16, pinsn_type);

    case N32_OP6_BR1:
      if (!IS_WITHIN_S (N32_IMM14S (insn), 8))
	goto done;

      if ((insn & N32_BIT (14)) == 0)
	{
	  /* N32_BR1_BEQ */
	  if (N32_IS_RT3 (insn) && N32_RA5 (insn) == REG_R5
	      && N32_RT5 (insn) != REG_R5)
	    insn16 = N16_TYPE38 (BEQS38, N32_RT5 (insn), N32_IMM14S (insn));
	  else if (N32_IS_RA3 (insn) && N32_RT5 (insn) == REG_R5
		   && N32_RA5 (insn) != REG_R5)
	    insn16 = N16_TYPE38 (BEQS38, N32_RA5 (insn), N32_IMM14S (insn));
	  insn_type = NDS32_INSN_BEQS38;
	  break;
	}
      else
	{
	  /* N32_BR1_BNE */
	  if (N32_IS_RT3 (insn) && N32_RA5 (insn) == REG_R5
	      && N32_RT5 (insn) != REG_R5)
	    insn16 = N16_TYPE38 (BNES38, N32_RT5 (insn), N32_IMM14S (insn));
	  else if (N32_IS_RA3 (insn) && N32_RT5 (insn) == REG_R5
		   && N32_RA5 (insn) != REG_R5)
	    insn16 = N16_TYPE38 (BNES38, N32_RA5 (insn), N32_IMM14S (insn));
	  insn_type = NDS32_INSN_BNES38;
	  break;
	}
      break;

    case N32_OP6_BR2:
      switch (N32_BR2_SUB (insn))
	{
	case N32_BR2_BEQZ:
	  if (N32_IS_RT3 (insn) && IS_WITHIN_S (N32_IMM16S (insn), 8))
	    {
	      insn16 = N16_TYPE38 (BEQZ38, N32_RT5 (insn), N32_IMM16S (insn));
	      insn_type = NDS32_INSN_BEQZ38;
	    }
	  else if (N32_RT5 (insn) == REG_R15
		   && IS_WITHIN_S (N32_IMM16S (insn), 8))
	    {
	      insn16 = N16_TYPE8 (BEQZS8, N32_IMM16S (insn));
	      insn_type = NDS32_INSN_BEQZS8;
	    }
	  break;

	case N32_BR2_BNEZ:
	  if (N32_IS_RT3 (insn) && IS_WITHIN_S (N32_IMM16S (insn), 8))
	    {
	      insn16 = N16_TYPE38 (BNEZ38, N32_RT5 (insn), N32_IMM16S (insn));
	      insn_type = NDS32_INSN_BNEZ38;
	    }
	  else if (N32_RT5 (insn) == REG_R15
		   && IS_WITHIN_S (N32_IMM16S (insn), 8))
	    {
	      insn16 = N16_TYPE8 (BNEZS8, N32_IMM16S (insn));
	      insn_type = NDS32_INSN_BNEZS8;
	    }
	  break;

	case N32_BR2_SOP0:
	  if (__GF (insn, 20, 5) == 0 && IS_WITHIN_U (N32_IMM16S (insn), 9))
	    {
	      insn16 = N16_TYPE9 (IFCALL9, N32_IMM16S (insn));
	      insn_type = NDS32_INSN_IFCALL9;
	    }
	  break;
	}
      break;

    case N32_OP6_JI:
      if ((insn & N32_BIT (24)) == 0)
	{
	  /* N32_JI_J */
	  if (IS_WITHIN_S (N32_IMM24S (insn), 8))
	    {
	      insn16 = N16_TYPE8 (J8, N32_IMM24S (insn));
	      insn_type = NDS32_INSN_J8;
	    }
	}
      break;

    case N32_OP6_JREG:
      if (__GF (insn, 8, 2) != 0)
	goto done;

      switch (N32_IMMU (insn, 5))
	{
	case N32_JREG_JR:
	  if (N32_JREG_HINT (insn) == 0)
	    {
	      /* jr */
	      insn16 = N16_TYPE5 (JR5, N32_RB5 (insn));
	      insn_type = NDS32_INSN_JR5;
	    }
	  else if (N32_JREG_HINT (insn) == 1)
	    {
	      /* ret */
	      insn16 = N16_TYPE5 (RET5, N32_RB5 (insn));
	      insn_type = NDS32_INSN_RET5;
	    }
	  else if (N32_JREG_HINT (insn) == 3)
	    {
	      /* ifret = mov55 $sp, $sp */
	      insn16 = N16_TYPE55 (MOV55, REG_SP, REG_SP);
	      insn_type = NDS32_INSN_IFRET;
	    }
	  break;

	case N32_JREG_JRAL:
	  /* It's convertible when return rt5 is $lp and address
	     translation is kept.  */
	  if (N32_RT5 (insn) == REG_LP && N32_JREG_HINT (insn) == 0)
	    {
	      insn16 = N16_TYPE5 (JRAL5, N32_RB5 (insn));
	      insn_type = NDS32_INSN_JRAL5;
	    }
	  break;
	}
      break;

    case N32_OP6_MISC:
      if (N32_SUB5 (insn) == N32_MISC_BREAK && N32_SWID (insn) < 32)
	{
	  /* For v3, swid above 31 are used for ex9.it.  */
	  insn16 = N16_TYPE5 (BREAK16, N32_SWID (insn));
	  insn_type = NDS32_INSN_BREAK16;
	}
      break;

    default:
      /* This instruction has no 16-bit variant.  */
      goto done;
    }

 done:
  /* Bit-15 of insn16 should be set for a valid instruction.  */
  if ((insn16 & 0x8000) == 0)
    return 0;

  if (pinsn16)
    *pinsn16 = insn16;
  if (pinsn_type)
    *pinsn_type = insn_type;
  return 1;
}

static int
special_convert_32_to_16 (unsigned long insn, uint16_t *pinsn16,
			  Elf_Internal_Rela *reloc)
{
  uint16_t insn16 = 0;

  if ((reloc->r_addend & R_NDS32_INSN16_FP7U2_FLAG) == 0
      || (ELF32_R_TYPE (reloc->r_info) != R_NDS32_INSN16))
    return 0;

  if (!N32_IS_RT3 (insn))
    return 0;

  switch (N32_OP6 (insn))
    {
    case N32_OP6_LWI:
      if (N32_RA5 (insn) == REG_GP && IS_WITHIN_U (N32_IMM15S (insn), 7))
	insn16 = N16_TYPE37 (XWI37, N32_RT5 (insn), 0, N32_IMM15S (insn));
      break;
    case N32_OP6_SWI:
      if (N32_RA5 (insn) == REG_GP && IS_WITHIN_U (N32_IMM15S (insn), 7))
	insn16 = N16_TYPE37 (XWI37, N32_RT5 (insn), 1, N32_IMM15S (insn));
      break;
    case N32_OP6_HWGP:
      if (!IS_WITHIN_U (N32_IMM17S (insn), 7))
	break;

      if (__GF (insn, 17, 3) == 6)
	insn16 = N16_TYPE37 (XWI37, N32_RT5 (insn), 0, N32_IMM17S (insn));
      else if (__GF (insn, 17, 3) == 7)
	insn16 = N16_TYPE37 (XWI37, N32_RT5 (insn), 1, N32_IMM17S (insn));
      break;
    }

  if ((insn16 & 0x8000) == 0)
    return 0;

  *pinsn16 = insn16;
  return 1;
}

/* Convert a 16-bit instruction to 32-bit one.
   INSN16 it the input and PINSN it the point to output.
   Return non-zero on successful.  Otherwise 0 is returned.  */

int
nds32_convert_16_to_32 (bfd *abfd, uint16_t insn16, uint32_t *pinsn)
{
  uint32_t insn = 0xffffffff;
  unsigned long mach = bfd_get_mach (abfd);

  /* NOTE: push25, pop25 and movd44 do not have 32-bit variants.  */

  switch (__GF (insn16, 9, 6))
    {
    case 0x4:			/* add45 */
      insn = N32_ALU1 (ADD, N16_RT4 (insn16), N16_RT4 (insn16),
		       N16_RA5 (insn16));
      goto done;
    case 0x5:			/* sub45 */
      insn = N32_ALU1 (SUB, N16_RT4 (insn16), N16_RT4 (insn16),
		       N16_RA5 (insn16));
      goto done;
    case 0x6:			/* addi45 */
      insn = N32_TYPE2 (ADDI, N16_RT4 (insn16), N16_RT4 (insn16),
			N16_IMM5U (insn16));
      goto done;
    case 0x7:			/* subi45 */
      insn = N32_TYPE2 (ADDI, N16_RT4 (insn16), N16_RT4 (insn16),
			-N16_IMM5U (insn16));
      goto done;
    case 0x8:			/* srai45 */
      insn = N32_ALU1 (SRAI, N16_RT4 (insn16), N16_RT4 (insn16),
		       N16_IMM5U (insn16));
      goto done;
    case 0x9:			/* srli45 */
      insn = N32_ALU1 (SRLI, N16_RT4 (insn16), N16_RT4 (insn16),
		       N16_IMM5U (insn16));
      goto done;
    case 0xa:			/* slli333 */
      insn = N32_ALU1 (SLLI, N16_RT3 (insn16), N16_RA3 (insn16),
		       N16_IMM3U (insn16));
      goto done;
    case 0xc:			/* add333 */
      insn = N32_ALU1 (ADD, N16_RT3 (insn16), N16_RA3 (insn16),
		       N16_RB3 (insn16));
      goto done;
    case 0xd:			/* sub333 */
      insn = N32_ALU1 (SUB, N16_RT3 (insn16), N16_RA3 (insn16),
		       N16_RB3 (insn16));
      goto done;
    case 0xe:			/* addi333 */
      insn = N32_TYPE2 (ADDI, N16_RT3 (insn16), N16_RA3 (insn16),
			N16_IMM3U (insn16));
      goto done;
    case 0xf:			/* subi333 */
      insn = N32_TYPE2 (ADDI, N16_RT3 (insn16), N16_RA3 (insn16),
			-N16_IMM3U (insn16));
      goto done;
    case 0x10:			/* lwi333 */
      insn = N32_TYPE2 (LWI, N16_RT3 (insn16), N16_RA3 (insn16),
			N16_IMM3U (insn16));
      goto done;
    case 0x12:			/* lhi333 */
      insn = N32_TYPE2 (LHI, N16_RT3 (insn16), N16_RA3 (insn16),
			N16_IMM3U (insn16));
      goto done;
    case 0x13:			/* lbi333 */
      insn = N32_TYPE2 (LBI, N16_RT3 (insn16), N16_RA3 (insn16),
			N16_IMM3U (insn16));
      goto done;
    case 0x11:			/* lwi333.bi */
      insn = N32_TYPE2 (LWI_BI, N16_RT3 (insn16), N16_RA3 (insn16),
			N16_IMM3U (insn16));
      goto done;
    case 0x14:			/* swi333 */
      insn = N32_TYPE2 (SWI, N16_RT3 (insn16), N16_RA3 (insn16),
			N16_IMM3U (insn16));
      goto done;
    case 0x16:			/* shi333 */
      insn = N32_TYPE2 (SHI, N16_RT3 (insn16), N16_RA3 (insn16),
			N16_IMM3U (insn16));
      goto done;
    case 0x17:			/* sbi333 */
      insn = N32_TYPE2 (SBI, N16_RT3 (insn16), N16_RA3 (insn16),
			N16_IMM3U (insn16));
      goto done;
    case 0x15:			/* swi333.bi */
      insn = N32_TYPE2 (SWI_BI, N16_RT3 (insn16), N16_RA3 (insn16),
			N16_IMM3U (insn16));
      goto done;
    case 0x18:			/* addri36.sp */
      insn = N32_TYPE2 (ADDI, N16_RT3 (insn16), REG_SP,
			N16_IMM6U (insn16) << 2);
      goto done;
    case 0x19:			/* lwi45.fe */
      insn = N32_TYPE2 (LWI, N16_RT4 (insn16), REG_R8,
			(N16_IMM5U (insn16) - 32));
      goto done;
    case 0x1a:			/* lwi450 */
      insn = N32_TYPE2 (LWI, N16_RT4 (insn16), N16_RA5 (insn16), 0);
      goto done;
    case 0x1b:			/* swi450 */
      insn = N32_TYPE2 (SWI, N16_RT4 (insn16), N16_RA5 (insn16), 0);
      goto done;

      /* These are r15 implied instructions.  */
    case 0x30:			/* slts45 */
      insn = N32_ALU1 (SLTS, REG_TA, N16_RT4 (insn16), N16_RA5 (insn16));
      goto done;
    case 0x31:			/* slt45 */
      insn = N32_ALU1 (SLT, REG_TA, N16_RT4 (insn16), N16_RA5 (insn16));
      goto done;
    case 0x32:			/* sltsi45 */
      insn = N32_TYPE2 (SLTSI, REG_TA, N16_RT4 (insn16), N16_IMM5U (insn16));
      goto done;
    case 0x33:			/* slti45 */
      insn = N32_TYPE2 (SLTI, REG_TA, N16_RT4 (insn16), N16_IMM5U (insn16));
      goto done;
    case 0x34:			/* beqzs8, bnezs8 */
      if (insn16 & N32_BIT (8))
	insn = N32_BR2 (BNEZ, REG_TA, N16_IMM8S (insn16));
      else
	insn = N32_BR2 (BEQZ, REG_TA, N16_IMM8S (insn16));
      goto done;

    case 0x35:			/* break16, ex9.it */
      /* Only consider range of v3 break16.  */
      insn = N32_TYPE0 (MISC, (N16_IMM5U (insn16) << 5) | N32_MISC_BREAK);
      goto done;

    case 0x3c:			/* ifcall9 */
      insn = N32_BR2 (SOP0, 0, N16_IMM9U (insn16));
      goto done;
    case 0x3d:			/* movpi45 */
      insn = N32_TYPE1 (MOVI, N16_RT4 (insn16), N16_IMM5U (insn16) + 16);
      goto done;

    case 0x3f:			/* MISC33 */
      switch (insn16 & 0x7)
	{
	case 2:			/* neg33 */
	  insn = N32_TYPE2 (SUBRI, N16_RT3 (insn16), N16_RA3 (insn16), 0);
	  break;
	case 3:			/* not33 */
	  insn = N32_ALU1 (NOR, N16_RT3 (insn16), N16_RA3 (insn16),
			   N16_RA3 (insn16));
	  break;
	case 4:			/* mul33 */
	  insn = N32_ALU2 (MUL, N16_RT3 (insn16), N16_RT3 (insn16),
			   N16_RA3 (insn16));
	  break;
	case 5:			/* xor33 */
	  insn = N32_ALU1 (XOR, N16_RT3 (insn16), N16_RT3 (insn16),
			   N16_RA3 (insn16));
	  break;
	case 6:			/* and33 */
	  insn = N32_ALU1 (AND, N16_RT3 (insn16), N16_RT3 (insn16),
			   N16_RA3 (insn16));
	  break;
	case 7:			/* or33 */
	  insn = N32_ALU1 (OR, N16_RT3 (insn16), N16_RT3 (insn16),
			   N16_RA3 (insn16));
	  break;
	}
      goto done;

    case 0xb:
      switch (insn16 & 0x7)
	{
	case 0:			/* zeb33 */
	  insn = N32_TYPE2 (ANDI, N16_RT3 (insn16), N16_RA3 (insn16), 0xff);
	  break;
	case 1:			/* zeh33 */
	  insn = N32_ALU1 (ZEH, N16_RT3 (insn16), N16_RA3 (insn16), 0);
	  break;
	case 2:			/* seb33 */
	  insn = N32_ALU1 (SEB, N16_RT3 (insn16), N16_RA3 (insn16), 0);
	  break;
	case 3:			/* seh33 */
	  insn = N32_ALU1 (SEH, N16_RT3 (insn16), N16_RA3 (insn16), 0);
	  break;
	case 4:			/* xlsb33 */
	  insn = N32_TYPE2 (ANDI, N16_RT3 (insn16), N16_RA3 (insn16), 1);
	  break;
	case 5:			/* x11b33 */
	  insn = N32_TYPE2 (ANDI, N16_RT3 (insn16), N16_RA3 (insn16), 0x7ff);
	  break;
	case 6:			/* bmski33 */
	  insn = N32_TYPE2 (ANDI, N16_RT3 (insn16), N16_RT3 (insn16),
			    1 << __GF (insn16, 3, 3));
	  break;
	case 7:			/* fexti33 */
	  insn = N32_TYPE2 (ANDI, N16_RT3 (insn16), N16_RT3 (insn16),
			    (1 << (__GF (insn16, 3, 3) + 1)) - 1);
	  break;
	}
      goto done;
    }

  switch (__GF (insn16, 10, 5))
    {
    case 0x0:			/* mov55 or ifret16 */
      if (mach >= MACH_V3 && N16_RT5 (insn16) == REG_SP
	  && N16_RT5 (insn16) == N16_RA5 (insn16))
	insn = N32_JREG (JR, 0, 0, 0, 3);
      else
	insn = N32_TYPE2 (ADDI, N16_RT5 (insn16), N16_RA5 (insn16), 0);
      goto done;
    case 0x1:			/* movi55 */
      insn = N32_TYPE1 (MOVI, N16_RT5 (insn16), N16_IMM5S (insn16));
      goto done;
    case 0x1b:			/* addi10s (V2) */
      insn = N32_TYPE2 (ADDI, REG_SP, REG_SP, N16_IMM10S (insn16));
      goto done;
    }

  switch (__GF (insn16, 11, 4))
    {
    case 0x7:			/* lwi37.fp/swi37.fp */
      if (insn16 & N32_BIT (7))	/* swi37.fp */
	insn = N32_TYPE2 (SWI, N16_RT38 (insn16), REG_FP, N16_IMM7U (insn16));
      else			/* lwi37.fp */
	insn = N32_TYPE2 (LWI, N16_RT38 (insn16), REG_FP, N16_IMM7U (insn16));
      goto done;
    case 0x8:			/* beqz38 */
      insn = N32_BR2 (BEQZ, N16_RT38 (insn16), N16_IMM8S (insn16));
      goto done;
    case 0x9:			/* bnez38 */
      insn = N32_BR2 (BNEZ, N16_RT38 (insn16), N16_IMM8S (insn16));
      goto done;
    case 0xa:			/* beqs38/j8, implied r5 */
      if (N16_RT38 (insn16) == 5)
	insn = N32_JI (J, N16_IMM8S (insn16));
      else
	insn = N32_BR1 (BEQ, N16_RT38 (insn16), REG_R5, N16_IMM8S (insn16));
      goto done;
    case 0xb:			/* bnes38 and others.  */
      if (N16_RT38 (insn16) == 5)
	{
	  switch (__GF (insn16, 5, 3))
	    {
	    case 0:		/* jr5 */
	      insn = N32_JREG (JR, 0, N16_RA5 (insn16), 0, 0);
	      break;
	    case 4:		/* ret5 */
	      insn = N32_JREG (JR, 0, N16_RA5 (insn16), 0, 1);
	      break;
	    case 1:		/* jral5 */
	      insn = N32_JREG (JRAL, REG_LP, N16_RA5 (insn16), 0, 0);
	      break;
	    case 2:		/* ex9.it imm5 */
	      /* ex9.it had no 32-bit variantl.  */
	      break;
	    case 5:		/* add5.pc */
	      /* add5.pc had no 32-bit variantl.  */
	      break;
	    }
	}
      else			/* bnes38 */
	insn = N32_BR1 (BNE, N16_RT38 (insn16), REG_R5, N16_IMM8S (insn16));
      goto done;
    case 0xe:			/* lwi37/swi37 */
      if (insn16 & (1 << 7))	/* swi37.sp */
	insn = N32_TYPE2 (SWI, N16_RT38 (insn16), REG_SP, N16_IMM7U (insn16));
      else			/* lwi37.sp */
	insn = N32_TYPE2 (LWI, N16_RT38 (insn16), REG_SP, N16_IMM7U (insn16));
      goto done;
    }

 done:
  if (insn & 0x80000000)
    return 0;

  if (pinsn)
    *pinsn = insn;
  return 1;
}


static bool
is_sda_access_insn (unsigned long insn)
{
  switch (N32_OP6 (insn))
    {
    case N32_OP6_LWI:
    case N32_OP6_LHI:
    case N32_OP6_LHSI:
    case N32_OP6_LBI:
    case N32_OP6_LBSI:
    case N32_OP6_SWI:
    case N32_OP6_SHI:
    case N32_OP6_SBI:
    case N32_OP6_LWC:
    case N32_OP6_LDC:
    case N32_OP6_SWC:
    case N32_OP6_SDC:
      return true;
    default:
      ;
    }
  return false;
}

static unsigned long
turn_insn_to_sda_access (uint32_t insn, bfd_signed_vma type, uint32_t *pinsn)
{
  uint32_t oinsn = 0;

  switch (type)
    {
    case R_NDS32_GOT_LO12:
    case R_NDS32_GOTOFF_LO12:
    case R_NDS32_PLTREL_LO12:
    case R_NDS32_PLT_GOTREL_LO12:
    case R_NDS32_LO12S0_RELA:
      switch (N32_OP6 (insn))
	{
	case N32_OP6_LBI:
	  /* lbi.gp */
	  oinsn = N32_TYPE1 (LBGP, N32_RT5 (insn), 0);
	  break;
	case N32_OP6_LBSI:
	  /* lbsi.gp */
	  oinsn = N32_TYPE1 (LBGP, N32_RT5 (insn), N32_BIT (19));
	  break;
	case N32_OP6_SBI:
	  /* sbi.gp */
	  oinsn = N32_TYPE1 (SBGP, N32_RT5 (insn), 0);
	  break;
	case N32_OP6_ORI:
	  /* addi.gp */
	  oinsn = N32_TYPE1 (SBGP, N32_RT5 (insn), N32_BIT (19));
	  break;
	}
      break;

    case R_NDS32_LO12S1_RELA:
      switch (N32_OP6 (insn))
	{
	case N32_OP6_LHI:
	  /* lhi.gp */
	  oinsn = N32_TYPE1 (HWGP, N32_RT5 (insn), 0);
	  break;
	case N32_OP6_LHSI:
	  /* lhsi.gp */
	  oinsn = N32_TYPE1 (HWGP, N32_RT5 (insn), N32_BIT (18));
	  break;
	case N32_OP6_SHI:
	  /* shi.gp */
	  oinsn = N32_TYPE1 (HWGP, N32_RT5 (insn), N32_BIT (19));
	  break;
	}
      break;

    case R_NDS32_LO12S2_RELA:
      switch (N32_OP6 (insn))
	{
	case N32_OP6_LWI:
	  /* lwi.gp */
	  oinsn = N32_TYPE1 (HWGP, N32_RT5 (insn), __MF (6, 17, 3));
	  break;
	case N32_OP6_SWI:
	  /* swi.gp */
	  oinsn = N32_TYPE1 (HWGP, N32_RT5 (insn), __MF (7, 17, 3));
	  break;
	}
      break;

    case R_NDS32_LO12S2_DP_RELA:
    case R_NDS32_LO12S2_SP_RELA:
      oinsn = (insn & 0x7ff07000) | (REG_GP << 15);
      break;
    }

  if (oinsn)
    *pinsn = oinsn;

  return oinsn != 0;
}

/* Linker hasn't found the correct merge section for non-section symbol
   in relax time, this work is left to the function elf_link_input_bfd().
   So for non-section symbol, _bfd_merged_section_offset is also needed
   to find the correct symbol address.  */

static bfd_vma
nds32_elf_rela_local_sym (bfd *abfd, Elf_Internal_Sym *sym,
			  asection **psec, Elf_Internal_Rela *rel)
{
  asection *sec = *psec;
  bfd_vma relocation;

  relocation = (sec->output_section->vma
		+ sec->output_offset + sym->st_value);
  if ((sec->flags & SEC_MERGE) && sec->sec_info_type == SEC_INFO_TYPE_MERGE)
    {
      if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	rel->r_addend =
	  _bfd_merged_section_offset (abfd, psec,
				      elf_section_data (sec)->sec_info,
				      sym->st_value + rel->r_addend);
      else
	rel->r_addend =
	  _bfd_merged_section_offset (abfd, psec,
				      elf_section_data (sec)->sec_info,
				      sym->st_value) + rel->r_addend;

      if (sec != *psec)
	{
	  /* If we have changed the section, and our original section is
	     marked with SEC_EXCLUDE, it means that the original
	     SEC_MERGE section has been completely subsumed in some
	     other SEC_MERGE section.  In this case, we need to leave
	     some info around for --emit-relocs.  */
	  if ((sec->flags & SEC_EXCLUDE) != 0)
	    sec->kept_section = *psec;
	  sec = *psec;
	}
      rel->r_addend -= relocation;
      rel->r_addend += sec->output_section->vma + sec->output_offset;
    }
  return relocation;
}

static bfd_vma
calculate_memory_address (bfd *abfd, Elf_Internal_Rela *irel,
			  Elf_Internal_Sym *isymbuf,
			  Elf_Internal_Shdr *symtab_hdr)
{
  bfd_signed_vma foff;
  bfd_vma symval, addend;
  Elf_Internal_Rela irel_fn;
  Elf_Internal_Sym *isym;
  asection *sym_sec;

  /* Get the value of the symbol referred to by the reloc.  */
  if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
    {
      /* A local symbol.  */
      isym = isymbuf + ELF32_R_SYM (irel->r_info);

      if (isym->st_shndx == SHN_UNDEF)
	sym_sec = bfd_und_section_ptr;
      else if (isym->st_shndx == SHN_ABS)
	sym_sec = bfd_abs_section_ptr;
      else if (isym->st_shndx == SHN_COMMON)
	sym_sec = bfd_com_section_ptr;
      else
	sym_sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
      memcpy (&irel_fn, irel, sizeof (Elf_Internal_Rela));
      symval = nds32_elf_rela_local_sym (abfd, isym, &sym_sec, &irel_fn);
      addend = irel_fn.r_addend;
    }
  else
    {
      unsigned long indx;
      struct elf_link_hash_entry *h;

      /* An external symbol.  */
      indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
      h = elf_sym_hashes (abfd)[indx];
      BFD_ASSERT (h != NULL);

      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      if (h->root.type != bfd_link_hash_defined
	  && h->root.type != bfd_link_hash_defweak)
	/* This appears to be a reference to an undefined
	   symbol.  Just ignore it--it will be caught by the
	   regular reloc processing.  */
	return 0;

      if (h->root.u.def.section->flags & SEC_MERGE)
	{
	  sym_sec = h->root.u.def.section;
	  symval = _bfd_merged_section_offset (abfd, &sym_sec, elf_section_data
					       (sym_sec)->sec_info, h->root.u.def.value);
	  symval = symval + sym_sec->output_section->vma
		   + sym_sec->output_offset;
	}
      else
	symval = (h->root.u.def.value
		  + h->root.u.def.section->output_section->vma
		  + h->root.u.def.section->output_offset);
      addend = irel->r_addend;
    }

  foff = symval + addend;

  return foff;
}

static int
is_16bit_NOP (bfd *abfd ATTRIBUTE_UNUSED,
	      asection *sec, Elf_Internal_Rela *rel)
{
  bfd_byte *contents;
  unsigned short insn16;

  if (!(rel->r_addend & R_NDS32_INSN16_CONVERT_FLAG))
    return false;
  contents = elf_section_data (sec)->this_hdr.contents;
  insn16 = bfd_getb16 (contents + rel->r_offset);
  if (insn16 == NDS32_NOP16)
    return true;
  return false;
}

/* It checks whether the instruction could be converted to
   16-bit form and returns the converted one.

   `internal_relocs' is supposed to be sorted.  */

static int
is_convert_32_to_16 (bfd *abfd, asection *sec,
		     Elf_Internal_Rela *reloc,
		     Elf_Internal_Rela *internal_relocs,
		     Elf_Internal_Rela *irelend,
		     uint16_t *insn16)
{
#define NORMAL_32_TO_16 (1 << 0)
#define SPECIAL_32_TO_16 (1 << 1)
  bfd_byte *contents = NULL;
  bfd_signed_vma off;
  bfd_vma mem_addr;
  uint32_t insn = 0;
  Elf_Internal_Rela *pc_rel;
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Sym *isymbuf = NULL;
  int convert_type;
  bfd_vma offset;

  if (reloc->r_offset + 4 > sec->size)
    return false;

  offset = reloc->r_offset;

  if (!nds32_get_section_contents (abfd, sec, &contents, true))
    return false;
  insn = bfd_getb32 (contents + offset);

  if (nds32_convert_32_to_16 (abfd, insn, insn16, NULL))
    convert_type = NORMAL_32_TO_16;
  else if (special_convert_32_to_16 (insn, insn16, reloc))
    convert_type = SPECIAL_32_TO_16;
  else
    return false;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  if (!nds32_get_local_syms (abfd, sec, &isymbuf))
    return false;

  /* Find the first relocation of the same relocation-type,
     so we iteratie them forward.  */
  pc_rel = reloc;
  while ((pc_rel - 1) >= internal_relocs && pc_rel[-1].r_offset == offset)
    pc_rel--;

  for (; pc_rel < irelend && pc_rel->r_offset == offset; pc_rel++)
    {
      if (ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_15_PCREL_RELA
	  || ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_17_PCREL_RELA
	  || ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_25_PCREL_RELA
	  || ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_25_PLTREL)
	{
	  off = calculate_offset (abfd, sec, pc_rel, isymbuf, symtab_hdr);
	  if (off >= ACCURATE_8BIT_S1 || off < -ACCURATE_8BIT_S1
	      || off == 0)
	    return false;
	  break;
	}
      else if (ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_20_RELA)
	{
	  /* movi => movi55  */
	  mem_addr = calculate_memory_address (abfd, pc_rel, isymbuf,
					       symtab_hdr);
	  /* mem_addr is unsigned, but the value should
	     be between [-16, 15].  */
	  if ((mem_addr + 0x10) >> 5)
	    return false;
	  break;
	}
      else if ((ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_TLS_LE_20)
	       || (ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_TLS_LE_LO12))
	{
	  /* It never happen movi to movi55 for R_NDS32_TLS_LE_20,
	     because it can be relaxed to addi for TLS_LE_ADD.  */
	  return false;
	}
      else if ((ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_SDA15S2_RELA
		|| ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_SDA17S2_RELA)
	       && (reloc->r_addend & R_NDS32_INSN16_FP7U2_FLAG)
	       && convert_type == SPECIAL_32_TO_16)
	{
	  /* fp-as-gp
	     We've selected a best fp-base for this access, so we can
	     always resolve it anyway.  Do nothing.  */
	  break;
	}
      else if ((ELF32_R_TYPE (pc_rel->r_info) > R_NDS32_NONE
		&& (ELF32_R_TYPE (pc_rel->r_info) < R_NDS32_RELA_GNU_VTINHERIT))
	       || ((ELF32_R_TYPE (pc_rel->r_info) > R_NDS32_RELA_GNU_VTENTRY)
		   && (ELF32_R_TYPE (pc_rel->r_info) < R_NDS32_INSN16))
	       || ((ELF32_R_TYPE (pc_rel->r_info) > R_NDS32_LOADSTORE)
		   && (ELF32_R_TYPE (pc_rel->r_info) < R_NDS32_DWARF2_OP1_RELA)))
	{
	  /* Prevent unresolved addi instruction translate
	     to addi45 or addi333.  */
	  return false;
	}
      else if ((ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_17IFC_PCREL_RELA))
	{
	  off = calculate_offset (abfd, sec, pc_rel, isymbuf, symtab_hdr);
	  if (off >= ACCURATE_U9BIT_S1 || off <= 0)
	    return false;
	  break;
	}
    }

  return true;
}

static void
nds32_elf_write_16 (bfd *abfd ATTRIBUTE_UNUSED, bfd_byte *contents,
		    Elf_Internal_Rela *reloc,
		    Elf_Internal_Rela *internal_relocs,
		    Elf_Internal_Rela *irelend,
		    unsigned short insn16)
{
  Elf_Internal_Rela *pc_rel;
  bfd_vma offset;

  offset = reloc->r_offset;
  bfd_putb16 (insn16, contents + offset);
  /* Find the first relocation of the same relocation-type,
     so we iteratie them forward.  */
  pc_rel = reloc;
  while ((pc_rel - 1) > internal_relocs && pc_rel[-1].r_offset == offset)
    pc_rel--;

  for (; pc_rel < irelend && pc_rel->r_offset == offset; pc_rel++)
    {
      if (ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_15_PCREL_RELA
	  || ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_17_PCREL_RELA
	  || ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_25_PCREL_RELA)
	{
	  pc_rel->r_info =
	    ELF32_R_INFO (ELF32_R_SYM (pc_rel->r_info), R_NDS32_9_PCREL_RELA);
	}
      else if (ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_25_PLTREL)
	pc_rel->r_info =
	  ELF32_R_INFO (ELF32_R_SYM (pc_rel->r_info), R_NDS32_9_PLTREL);
      else if (ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_20_RELA)
	pc_rel->r_info =
	  ELF32_R_INFO (ELF32_R_SYM (pc_rel->r_info), R_NDS32_5_RELA);
      else if (ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_SDA15S2_RELA
	       || ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_SDA17S2_RELA)
	pc_rel->r_info =
	  ELF32_R_INFO (ELF32_R_SYM (pc_rel->r_info), R_NDS32_SDA_FP7U2_RELA);
      else if ((ELF32_R_TYPE (pc_rel->r_info) == R_NDS32_17IFC_PCREL_RELA))
	pc_rel->r_info =
	  ELF32_R_INFO (ELF32_R_SYM (pc_rel->r_info), R_NDS32_10IFCU_PCREL_RELA);
    }
}

/* Find a relocation of type specified by `reloc_type'
   of the same r_offset with reloc.
   If not found, return irelend.

   Assuming relocations are sorted by r_offset,
   we find the relocation from `reloc' backward untill relocs,
   or find it from `reloc' forward untill irelend.  */

static Elf_Internal_Rela *
find_relocs_at_address (Elf_Internal_Rela *reloc,
			Elf_Internal_Rela *relocs,
			Elf_Internal_Rela *irelend,
			enum elf_nds32_reloc_type reloc_type)
{
  Elf_Internal_Rela *rel_t;

  /* Find backward.  */
  for (rel_t = reloc;
       rel_t >= relocs && rel_t->r_offset == reloc->r_offset;
       rel_t--)
    if (ELF32_R_TYPE (rel_t->r_info) == reloc_type)
      return rel_t;

  /* We didn't find it backward.  Try find it forward.  */
  for (rel_t = reloc;
       rel_t < irelend && rel_t->r_offset == reloc->r_offset;
       rel_t++)
    if (ELF32_R_TYPE (rel_t->r_info) == reloc_type)
      return rel_t;

  return irelend;
}

/* Find a relocation of specified type and offset.
   `reloc' is just a refence point to find a relocation at specified offset.
   If not found, return irelend.

   Assuming relocations are sorted by r_offset,
   we find the relocation from `reloc' backward untill relocs,
   or find it from `reloc' forward untill irelend.  */

static Elf_Internal_Rela *
find_relocs_at_address_addr (Elf_Internal_Rela *reloc,
			     Elf_Internal_Rela *relocs,
			     Elf_Internal_Rela *irelend,
			     enum elf_nds32_reloc_type reloc_type,
			     bfd_vma offset_p)
{
  Elf_Internal_Rela *rel_t = NULL;

  /* First, we try to find a relocation of offset `offset_p',
     and then we use find_relocs_at_address to find specific type.  */

  if (reloc->r_offset > offset_p)
    {
      /* Find backward.  */
      for (rel_t = reloc;
	   rel_t >= relocs && rel_t->r_offset > offset_p; rel_t--)
	/* Do nothing.  */;
    }
  else if (reloc->r_offset < offset_p)
    {
      /* Find forward.  */
      for (rel_t = reloc;
	   rel_t < irelend && rel_t->r_offset < offset_p; rel_t++)
	/* Do nothing.  */;
    }
  else
    rel_t = reloc;

  /* Not found?  */
  if (rel_t < relocs || rel_t == irelend || rel_t->r_offset != offset_p)
    return irelend;

  return find_relocs_at_address (rel_t, relocs, irelend, reloc_type);
}

typedef struct nds32_elf_blank nds32_elf_blank_t;
struct nds32_elf_blank
{
  /* Where the blank begins.  */
  bfd_vma offset;
  /* The size of the blank.  */
  bfd_vma size;
  /* The accumulative size before this blank.  */
  bfd_vma total_size;
  nds32_elf_blank_t *next;
  nds32_elf_blank_t *prev;
};

static nds32_elf_blank_t *blank_free_list = NULL;

static nds32_elf_blank_t *
create_nds32_elf_blank (bfd_vma offset_p, bfd_vma size_p)
{
  nds32_elf_blank_t *blank_t;

  if (blank_free_list)
    {
      blank_t = blank_free_list;
      blank_free_list = blank_free_list->next;
    }
  else
    blank_t = bfd_malloc (sizeof (nds32_elf_blank_t));

  if (blank_t == NULL)
    return NULL;

  blank_t->offset = offset_p;
  blank_t->size = size_p;
  blank_t->total_size = 0;
  blank_t->next = NULL;
  blank_t->prev = NULL;

  return blank_t;
}

static void
remove_nds32_elf_blank (nds32_elf_blank_t *blank_p)
{
  if (blank_free_list)
    {
      blank_free_list->prev = blank_p;
      blank_p->next = blank_free_list;
    }
  else
    blank_p->next = NULL;

  blank_p->prev = NULL;
  blank_free_list = blank_p;
}

static void
clean_nds32_elf_blank (void)
{
  nds32_elf_blank_t *blank_t;

  while (blank_free_list)
    {
      blank_t = blank_free_list;
      blank_free_list = blank_free_list->next;
      free (blank_t);
    }
}

static nds32_elf_blank_t *
search_nds32_elf_blank (nds32_elf_blank_t *blank_p, bfd_vma addr)
{
  nds32_elf_blank_t *blank_t;

  if (!blank_p)
    return NULL;
  blank_t = blank_p;

  while (blank_t && addr < blank_t->offset)
    blank_t = blank_t->prev;
  while (blank_t && blank_t->next && addr >= blank_t->next->offset)
    blank_t = blank_t->next;

  return blank_t;
}

static bfd_vma
get_nds32_elf_blank_total (nds32_elf_blank_t **blank_p, bfd_vma addr,
			   int overwrite)
{
  nds32_elf_blank_t *blank_t;

  blank_t = search_nds32_elf_blank (*blank_p, addr);
  if (!blank_t)
    return 0;

  if (overwrite)
    *blank_p = blank_t;

  if (addr < blank_t->offset + blank_t->size)
    return blank_t->total_size + (addr - blank_t->offset);
  else
    return blank_t->total_size + blank_t->size;
}

static bool
insert_nds32_elf_blank (nds32_elf_blank_t **blank_p, bfd_vma addr, bfd_vma len)
{
  nds32_elf_blank_t *blank_t, *blank_t2;

  if (!*blank_p)
    {
      *blank_p = create_nds32_elf_blank (addr, len);
      return *blank_p != NULL;
    }

  blank_t = search_nds32_elf_blank (*blank_p, addr);

  if (blank_t == NULL)
    {
      blank_t = create_nds32_elf_blank (addr, len);
      if (!blank_t)
	return false;
      while ((*blank_p)->prev != NULL)
	*blank_p = (*blank_p)->prev;
      blank_t->next = *blank_p;
      (*blank_p)->prev = blank_t;
      (*blank_p) = blank_t;
      return true;
    }

  if (addr < blank_t->offset + blank_t->size)
    {
      /* Extend the origin blank.  */
      if (addr + len > blank_t->offset + blank_t->size)
	blank_t->size = addr + len - blank_t->offset;
    }
  else
    {
      blank_t2 = create_nds32_elf_blank (addr, len);
      if (!blank_t2)
	return false;
      if (blank_t->next)
	{
	  blank_t->next->prev = blank_t2;
	  blank_t2->next = blank_t->next;
	}
      blank_t2->prev = blank_t;
      blank_t->next = blank_t2;
      *blank_p = blank_t2;
    }

  return true;
}

static bool
insert_nds32_elf_blank_recalc_total (nds32_elf_blank_t **blank_p, bfd_vma addr,
				     bfd_vma len)
{
  nds32_elf_blank_t *blank_t;

  if (!insert_nds32_elf_blank (blank_p, addr, len))
    return false;

  blank_t = *blank_p;

  if (!blank_t->prev)
    {
      blank_t->total_size = 0;
      blank_t = blank_t->next;
    }

  while (blank_t)
    {
      blank_t->total_size = blank_t->prev->total_size + blank_t->prev->size;
      blank_t = blank_t->next;
    }

  return true;
}

static void
calc_nds32_blank_total (nds32_elf_blank_t *blank_p)
{
  nds32_elf_blank_t *blank_t;
  bfd_vma total_size = 0;

  if (!blank_p)
    return;

  blank_t = blank_p;
  while (blank_t->prev)
    blank_t = blank_t->prev;
  while (blank_t)
    {
      blank_t->total_size = total_size;
      total_size += blank_t->size;
      blank_t = blank_t->next;
    }
}

static bool
nds32_elf_relax_delete_blanks (bfd *abfd, asection *sec,
			       nds32_elf_blank_t *blank_p)
{
  Elf_Internal_Shdr *symtab_hdr;	/* Symbol table header of this bfd.  */
  Elf_Internal_Sym *isym = NULL;	/* Symbol table of this bfd.  */
  Elf_Internal_Sym *isymend;		/* Symbol entry iterator.  */
  unsigned int sec_shndx;		/* The section the be relaxed.  */
  bfd_byte *contents;			/* Contents data of iterating section.  */
  Elf_Internal_Rela *internal_relocs;
  Elf_Internal_Rela *irel;
  Elf_Internal_Rela *irelend;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry **end_hashes;
  unsigned int symcount;
  asection *sect;
  nds32_elf_blank_t *blank_t;
  nds32_elf_blank_t *blank_t2;
  nds32_elf_blank_t *blank_head;

  blank_head = blank_t = blank_p;
  while (blank_head->prev != NULL)
    blank_head = blank_head->prev;
  while (blank_t->next != NULL)
    blank_t = blank_t->next;

  if (blank_t->offset + blank_t->size <= sec->size)
    {
      blank_t->next = create_nds32_elf_blank (sec->size + 4, 0);
      blank_t->next->prev = blank_t;
    }
  if (blank_head->offset > 0)
    {
      blank_head->prev = create_nds32_elf_blank (0, 0);
      blank_head->prev->next = blank_head;
      blank_head = blank_head->prev;
    }

  sec_shndx = _bfd_elf_section_from_bfd_section (abfd, sec);

  /* The deletion must stop at the next ALIGN reloc for an alignment
     power larger than the number of bytes we are deleting.  */

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  if (!nds32_get_local_syms (abfd, sec, &isym))
    return false;

  if (isym == NULL)
    {
      isym = bfd_elf_get_elf_syms (abfd, symtab_hdr,
				   symtab_hdr->sh_info, 0, NULL, NULL, NULL);
      symtab_hdr->contents = (bfd_byte *) isym;
    }

  if (isym == NULL || symtab_hdr->sh_info == 0)
    return false;

  blank_t = blank_head;
  calc_nds32_blank_total (blank_head);

  for (sect = abfd->sections; sect != NULL; sect = sect->next)
    {
      /* Adjust all the relocs.  */

      /* Relocations MUST be kept in memory, because relaxation adjust them.  */
      internal_relocs = _bfd_elf_link_read_relocs (abfd, sect, NULL, NULL,
						   true /* keep_memory */);
      irelend = internal_relocs + sect->reloc_count;

      blank_t = blank_head;
      blank_t2 = blank_head;

      if (!(sect->flags & SEC_RELOC))
	continue;

      contents = NULL;
      nds32_get_section_contents (abfd, sect, &contents, true);

      for (irel = internal_relocs; irel < irelend; irel++)
	{
	  bfd_vma raddr;

	  if (ELF32_R_TYPE (irel->r_info) >= R_NDS32_DIFF8
	      && ELF32_R_TYPE (irel->r_info) <= R_NDS32_DIFF32
	      && isym[ELF32_R_SYM (irel->r_info)].st_shndx == sec_shndx)
	    {
	      unsigned long val = 0;
	      unsigned long mask;
	      long before, between;
	      long offset = 0;

	      switch (ELF32_R_TYPE (irel->r_info))
		{
		case R_NDS32_DIFF8:
		  offset = bfd_get_8 (abfd, contents + irel->r_offset);
		  break;
		case R_NDS32_DIFF16:
		  offset = bfd_get_16 (abfd, contents + irel->r_offset);
		  break;
		case R_NDS32_DIFF32:
		  val = bfd_get_32 (abfd, contents + irel->r_offset);
		  /* Get the signed bit and mask for the high part.  The
		     gcc will alarm when right shift 32-bit since the
		     type size of long may be 32-bit.  */
		  mask = 0 - (val >> 31);
		  if (mask)
		    offset = (val | (mask - 0xffffffff));
		  else
		    offset = val;
		  break;
		default:
		  BFD_ASSERT (0);
		}

	      /*		  DIFF value
		0	     |encoded in location|
		|------------|-------------------|---------
			    sym+off(addend)
		-- before ---| *****************
		--------------------- between ---|

		We only care how much data are relax between DIFF,
		marked as ***.  */

	      before = get_nds32_elf_blank_total (&blank_t, irel->r_addend, 0);
	      between = get_nds32_elf_blank_total (&blank_t,
						   irel->r_addend + offset, 0);
	      if (between == before)
		goto done_adjust_diff;

	      switch (ELF32_R_TYPE (irel->r_info))
		{
		case R_NDS32_DIFF8:
		  bfd_put_8 (abfd, offset - (between - before),
			     contents + irel->r_offset);
		  break;
		case R_NDS32_DIFF16:
		  bfd_put_16 (abfd, offset - (between - before),
			      contents + irel->r_offset);
		  break;
		case R_NDS32_DIFF32:
		  bfd_put_32 (abfd, offset - (between - before),
			      contents + irel->r_offset);
		  break;
		}
	    }
	  else if (ELF32_R_TYPE (irel->r_info) == R_NDS32_DIFF_ULEB128
	      && isym[ELF32_R_SYM (irel->r_info)].st_shndx == sec_shndx)
	    {
	      bfd_vma val = 0;
	      unsigned int len = 0;
	      unsigned long before, between;
	      bfd_byte *endp, *p;

	      val = _bfd_read_unsigned_leb128 (abfd, contents + irel->r_offset,
					       &len);

	      before = get_nds32_elf_blank_total (&blank_t, irel->r_addend, 0);
	      between = get_nds32_elf_blank_total (&blank_t,
						   irel->r_addend + val, 0);
	      if (between == before)
		goto done_adjust_diff;

	      p = contents + irel->r_offset;
	      endp = p + len -1;
	      memset (p, 0x80, len);
	      *(endp) = 0;
	      p = write_uleb128 (p, val - (between - before)) - 1;
	      if (p < endp)
		*p |= 0x80;
	    }
	done_adjust_diff:

	  if (sec == sect)
	    {
	      raddr = irel->r_offset;
	      irel->r_offset -= get_nds32_elf_blank_total (&blank_t2,
							   irel->r_offset, 1);

	      if (ELF32_R_TYPE (irel->r_info) == R_NDS32_NONE)
		continue;
	      if (blank_t2 && blank_t2->next
		  && (blank_t2->offset > raddr
		      || blank_t2->next->offset <= raddr))
		_bfd_error_handler
		  (_("%pB: error: search_nds32_elf_blank reports wrong node"),
		   abfd);

	      /* Mark reloc in deleted portion as NONE.
		 For some relocs like R_NDS32_LABEL that doesn't modify the
		 content in the section.  R_NDS32_LABEL doesn't belong to the
		 instruction in the section, so we should preserve it.  */
	      if (raddr >= blank_t2->offset
		  && raddr < blank_t2->offset + blank_t2->size
		  && ELF32_R_TYPE (irel->r_info) != R_NDS32_LABEL
		  && ELF32_R_TYPE (irel->r_info) != R_NDS32_RELAX_REGION_BEGIN
		  && ELF32_R_TYPE (irel->r_info) != R_NDS32_RELAX_REGION_END
		  && ELF32_R_TYPE (irel->r_info) != R_NDS32_RELAX_ENTRY
		  && ELF32_R_TYPE (irel->r_info) != R_NDS32_SUBTRAHEND
		  && ELF32_R_TYPE (irel->r_info) != R_NDS32_MINUEND)
		{
		  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					       R_NDS32_NONE);
		  continue;
		}
	    }

	  if (ELF32_R_TYPE (irel->r_info) == R_NDS32_NONE
	      || ELF32_R_TYPE (irel->r_info) == R_NDS32_LABEL
	      || ELF32_R_TYPE (irel->r_info) == R_NDS32_RELAX_ENTRY)
	    continue;

	  if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info
	      && isym[ELF32_R_SYM (irel->r_info)].st_shndx == sec_shndx
	      && ELF_ST_TYPE (isym[ELF32_R_SYM (irel->r_info)].st_info) == STT_SECTION)
	    {
	      if (irel->r_addend <= sec->size)
		irel->r_addend -=
		  get_nds32_elf_blank_total (&blank_t, irel->r_addend, 1);
	    }
	}
    }

  /* Adjust the local symbols defined in this section.  */
  blank_t = blank_head;
  for (isymend = isym + symtab_hdr->sh_info; isym < isymend; isym++)
    {
      if (isym->st_shndx == sec_shndx)
	{
	  if (isym->st_value <= sec->size)
	    {
	      bfd_vma ahead;
	      bfd_vma orig_addr = isym->st_value;

	      ahead = get_nds32_elf_blank_total (&blank_t, isym->st_value, 1);
	      isym->st_value -= ahead;

	      /* Adjust function size.  */
	      if (ELF32_ST_TYPE (isym->st_info) == STT_FUNC
		  && isym->st_size > 0)
		isym->st_size -=
		  get_nds32_elf_blank_total
		  (&blank_t, orig_addr + isym->st_size, 0) - ahead;
	    }
	}
    }

  /* Now adjust the global symbols defined in this section.  */
  symcount = (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)
	      - symtab_hdr->sh_info);
  sym_hashes = elf_sym_hashes (abfd);
  end_hashes = sym_hashes + symcount;
  blank_t = blank_head;
  for (; sym_hashes < end_hashes; sym_hashes++)
    {
      struct elf_link_hash_entry *sym_hash = *sym_hashes;

      if ((sym_hash->root.type == bfd_link_hash_defined
	   || sym_hash->root.type == bfd_link_hash_defweak)
	  && sym_hash->root.u.def.section == sec)
	{
	  if (sym_hash->root.u.def.value <= sec->size)
	    {
	      bfd_vma ahead;
	      bfd_vma orig_addr = sym_hash->root.u.def.value;

	      ahead = get_nds32_elf_blank_total (&blank_t, sym_hash->root.u.def.value, 1);
	      sym_hash->root.u.def.value -= ahead;

	      /* Adjust function size.  */
	      if (sym_hash->type == STT_FUNC)
		sym_hash->size -=
		  get_nds32_elf_blank_total
		  (&blank_t, orig_addr + sym_hash->size, 0) - ahead;

	    }
	}
    }

  contents = elf_section_data (sec)->this_hdr.contents;
  blank_t = blank_head;
  while (blank_t->next)
    {
      /* Actually delete the bytes.  */

      /* If current blank is the last blank overlap with current section,
	 go to finish process.  */
      if (sec->size <= (blank_t->next->offset))
	break;

      memmove (contents + blank_t->offset - blank_t->total_size,
	       contents + blank_t->offset + blank_t->size,
	       blank_t->next->offset - (blank_t->offset + blank_t->size));

      blank_t = blank_t->next;
    }

  if (sec->size > (blank_t->offset + blank_t->size))
    {
      /* There are remaining code between blank and section boundary.
	 Move the remaining code to appropriate location.  */
      memmove (contents + blank_t->offset - blank_t->total_size,
	       contents + blank_t->offset + blank_t->size,
	       sec->size - (blank_t->offset + blank_t->size));
      sec->size -= blank_t->total_size + blank_t->size;
    }
  else
    /* This blank is not entirely included in the section,
       reduce the section size by only part of the blank size.  */
    sec->size -= blank_t->total_size + (sec->size - blank_t->offset);

  while (blank_head)
    {
      blank_t = blank_head;
      blank_head = blank_head->next;
      remove_nds32_elf_blank (blank_t);
    }

  return true;
}

/* Get the contents of a section.  */

static int
nds32_get_section_contents (bfd *abfd, asection *sec,
			    bfd_byte **contents_p, bool cache)
{
  /* Get the section contents.  */
  if (elf_section_data (sec)->this_hdr.contents != NULL)
    *contents_p = elf_section_data (sec)->this_hdr.contents;
  else
    {
      if (!bfd_get_full_section_contents (abfd, sec, contents_p))
	return false;
      if (cache)
	elf_section_data (sec)->this_hdr.contents = *contents_p;
    }

  return true;
}

/* Get the contents of the internal symbol of abfd.  */

static int
nds32_get_local_syms (bfd *abfd, asection *sec ATTRIBUTE_UNUSED,
		      Elf_Internal_Sym **isymbuf_p)
{
  Elf_Internal_Shdr *symtab_hdr;
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  /* Read this BFD's local symbols if we haven't done so already.  */
  if (*isymbuf_p == NULL && symtab_hdr->sh_info != 0)
    {
      *isymbuf_p = (Elf_Internal_Sym *) symtab_hdr->contents;
      if (*isymbuf_p == NULL)
	{
	  *isymbuf_p = bfd_elf_get_elf_syms (abfd, symtab_hdr,
					     symtab_hdr->sh_info, 0,
					     NULL, NULL, NULL);
	  if (*isymbuf_p == NULL)
	    return false;
	}
    }
  symtab_hdr->contents = (bfd_byte *) (*isymbuf_p);

  return true;
}

/* Range of small data.  */
static bfd_vma sdata_range[2][2];
static bfd_vma const sdata_init_range[2] =
{ ACCURATE_12BIT_S1, ACCURATE_19BIT };

static int
nds32_elf_insn_size (bfd *abfd ATTRIBUTE_UNUSED,
		     bfd_byte *contents, bfd_vma addr)
{
  unsigned long insn = bfd_getb32 (contents + addr);

  if (insn & 0x80000000)
    return 2;

  return 4;
}

/* Set the gp relax range.  We have to measure the safe range
   to do gp relaxation.  */

static void
relax_range_measurement (bfd *abfd, struct bfd_link_info *link_info)
{
  asection *sec_f, *sec_b;
  /* For upper bound.   */
  bfd_vma maxpgsz;
  bfd_vma align;
  static int decide_relax_range = 0;
  int i;
  int range_number = ARRAY_SIZE (sdata_init_range);

  if (decide_relax_range)
    return;
  decide_relax_range = 1;

  if (sda_rela_sec == NULL)
    {
      /* Since there is no data sections, we assume the range is page size.  */
      for (i = 0; i < range_number; i++)
	{
	  sdata_range[i][0] = sdata_init_range[i] - 0x1000;
	  sdata_range[i][1] = sdata_init_range[i] - 0x1000;
	}
      return;
    }

  /* Get the biggest alignment power after the gp located section.  */
  sec_f = sda_rela_sec->output_section;
  sec_b = sec_f->next;
  align = 0;
  while (sec_b != NULL)
    {
      if ((unsigned)(1 << sec_b->alignment_power) > align)
	align = (1 << sec_b->alignment_power);
      sec_b = sec_b->next;
    }

  if (link_info != NULL)
    maxpgsz = link_info->maxpagesize;
  else
    maxpgsz = get_elf_backend_data (abfd)->maxpagesize;
  /* I guess we can not determine the section before
     gp located section, so we assume the align is max page size.  */
  for (i = 0; i < range_number; i++)
    {
      sdata_range[i][1] = sdata_init_range[i] - align;
      BFD_ASSERT (sdata_range[i][1] <= sdata_init_range[i]);
      sdata_range[i][0] = sdata_init_range[i] - maxpgsz;
      BFD_ASSERT (sdata_range[i][0] <= sdata_init_range[i]);
    }
}

/* These are macros used to check flags encoded in r_addend.
   They are only used by nds32_elf_relax_section ().  */
#define GET_SEQ_LEN(addend)     ((addend) & 0x000000ff)
#define IS_1ST_CONVERT(addend)  ((addend) & 0x80000000)
#define IS_OPTIMIZE(addend)     ((addend) & 0x40000000)
#define IS_16BIT_ON(addend)     ((addend) & 0x20000000)

static const char * unrecognized_reloc_msg =
  /* xgettext:c-format */
  N_("%pB: warning: %s points to unrecognized reloc at %#" PRIx64);

/* Relax LONGCALL1 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longcall1 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* There are 3 variations for LONGCALL1
     case 4-4-2; 16-bit on, optimize off or optimize for space
     sethi ta, hi20(symbol)	; LONGCALL1/HI20
     ori   ta, ta, lo12(symbol) ; LO12S0
     jral5 ta			;

     case 4-4-4; 16-bit off, optimize don't care
     sethi ta, hi20(symbol)	; LONGCALL1/HI20
     ori   ta, ta, lo12(symbol) ; LO12S0
     jral  ta			;

     case 4-4-4; 16-bit on, optimize for speed
     sethi ta, hi20(symbol)	; LONGCALL1/HI20
     ori   ta, ta, lo12(symbol) ; LO12S0
     jral  ta			;
     Check code for -mlong-calls output.  */

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */

  bfd_vma laddr;
  int seq_len;	/* Original length of instruction sequence.  */
  uint32_t insn;
  Elf_Internal_Rela *hi_irelfn, *lo_irelfn, *irelend;
  bfd_signed_vma foff;
  uint16_t insn16;

  irelend = internal_relocs + sec->reloc_count;
  seq_len = GET_SEQ_LEN (irel->r_addend);
  laddr = irel->r_offset;
  *insn_len = seq_len;

  hi_irelfn = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					   R_NDS32_HI20_RELA, laddr);
  lo_irelfn = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					   R_NDS32_LO12S0_ORI_RELA,
					   laddr + 4);

  if (hi_irelfn == irelend || lo_irelfn == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGCALL1",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, hi_irelfn, isymbuf, symtab_hdr);

  /* This condition only happened when symbol is undefined.  */
  if (foff == 0
      || foff < -CONSERVATIVE_24BIT_S1
      || foff >= CONSERVATIVE_24BIT_S1)
    return false;

  /* Relax to: jal symbol; 25_PCREL.  */
  /* For simplicity of coding, we are going to modify the section
     contents, the section relocs, and the BFD symbol table.  We
     must tell the rest of the code not to free up this
     information.  It would be possible to instead create a table
     of changes which have to be made, as is done in coff-mips.c;
     that would be more work, but would require less memory when
     the linker is run.  */

  /* Replace the long call with a jal.  */
  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info),
			       R_NDS32_25_PCREL_RELA);
  irel->r_addend = hi_irelfn->r_addend;

  /* We don't resolve this here but resolve it in relocate_section.  */
  insn = INSN_JAL;
  bfd_putb32 (insn, contents + irel->r_offset);

  hi_irelfn->r_info =
    ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info), R_NDS32_NONE);
  lo_irelfn->r_info =
    ELF32_R_INFO (ELF32_R_SYM (lo_irelfn->r_info), R_NDS32_NONE);
  *insn_len = 4;

  if (seq_len & 0x2)
    {
      insn16 = NDS32_NOP16;
      bfd_putb16 (insn16, contents + irel->r_offset + *insn_len);
      lo_irelfn->r_info =
	ELF32_R_INFO (ELF32_R_SYM (lo_irelfn->r_info), R_NDS32_INSN16);
      lo_irelfn->r_addend = R_NDS32_INSN16_CONVERT_FLAG;
      *insn_len += 2;
    }
  return true;
}

#define CONVERT_CONDITION_CALL(insn) (((insn) & 0xffff0000) ^ 0x90000)
/* Relax LONGCALL2 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longcall2 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* bltz  rt, .L1   ; LONGCALL2
     jal   symbol   ; 25_PCREL
     .L1: */

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */

  bfd_vma laddr;
  uint32_t insn;
  Elf_Internal_Rela *i1_irelfn, *cond_irelfn, *irelend;
  bfd_signed_vma foff;

  irelend = internal_relocs + sec->reloc_count;
  laddr = irel->r_offset;
  i1_irelfn =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_25_PCREL_RELA, laddr + 4);

  if (i1_irelfn == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGCALL2",
			  (uint64_t) irel->r_offset);
      return false;
    }

  insn = bfd_getb32 (contents + laddr);

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, i1_irelfn, isymbuf, symtab_hdr);

  if (foff == 0
      || foff < -CONSERVATIVE_16BIT_S1
      || foff >= CONSERVATIVE_16BIT_S1)
    return false;

  /* Relax to	bgezal   rt, label ; 17_PCREL
     or		bltzal   rt, label ; 17_PCREL */

  /* Convert to complimentary conditional call.  */
  insn = CONVERT_CONDITION_CALL (insn);

  /* For simplicity of coding, we are going to modify the section
     contents, the section relocs, and the BFD symbol table.  We
     must tell the rest of the code not to free up this
     information.  It would be possible to instead create a table
     of changes which have to be made, as is done in coff-mips.c;
     that would be more work, but would require less memory when
     the linker is run.  */

  /* Clean unnessary relocations.  */
  i1_irelfn->r_info =
    ELF32_R_INFO (ELF32_R_SYM (i1_irelfn->r_info), R_NDS32_NONE);
  cond_irelfn =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_17_PCREL_RELA, laddr);
  if (cond_irelfn != irelend)
    cond_irelfn->r_info =
      ELF32_R_INFO (ELF32_R_SYM (cond_irelfn->r_info), R_NDS32_NONE);

  /* Replace the long call with a bgezal.  */
  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (i1_irelfn->r_info),
			       R_NDS32_17_PCREL_RELA);
  irel->r_addend = i1_irelfn->r_addend;

  bfd_putb32 (insn, contents + irel->r_offset);

  *insn_len = 4;
  return true;
}

/* Relax LONGCALL3 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longcall3 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* There are 3 variations for LONGCALL3
     case 4-4-4-2; 16-bit on, optimize off or optimize for space
     bltz  rt,	 $1		   ; LONGCALL3
     sethi ta,	 hi20(symbol)	   ; HI20
     ori   ta, ta,  lo12(symbol)   ; LO12S0
     jral5 ta			   ;
     $1

     case 4-4-4-4; 16-bit off, optimize don't care
     bltz  rt,	 $1		   ; LONGCALL3
     sethi ta,	 hi20(symbol)	   ; HI20
     ori   ta, ta,  lo12(symbol)   ; LO12S0
     jral  ta			   ;
     $1

     case 4-4-4-4; 16-bit on, optimize for speed
     bltz  rt,	 $1		   ; LONGCALL3
     sethi ta,	 hi20(symbol)	   ; HI20
     ori   ta, ta,  lo12(symbol)   ; LO12S0
     jral  ta			   ;
     $1 */

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */

  bfd_vma laddr;
  int seq_len;	/* Original length of instruction sequence.  */
  uint32_t insn;
  Elf_Internal_Rela *hi_irelfn, *lo_irelfn, *cond_irelfn, *irelend;
  bfd_signed_vma foff;
  uint16_t insn16;

  irelend = internal_relocs + sec->reloc_count;
  seq_len = GET_SEQ_LEN (irel->r_addend);
  laddr = irel->r_offset;
  *insn_len = seq_len;

  hi_irelfn =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_HI20_RELA, laddr + 4);
  lo_irelfn =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_LO12S0_ORI_RELA, laddr + 8);

  if (hi_irelfn == irelend || lo_irelfn == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGCALL3",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, hi_irelfn, isymbuf, symtab_hdr);

  if (foff == 0
      || foff < -CONSERVATIVE_24BIT_S1
      || foff >= CONSERVATIVE_24BIT_S1)
    return false;

  insn = bfd_getb32 (contents + laddr);
  if (foff >= -CONSERVATIVE_16BIT_S1 && foff < CONSERVATIVE_16BIT_S1)
    {
      /* Relax to  bgezal   rt, label ; 17_PCREL
	 or	   bltzal   rt, label ; 17_PCREL */

      /* Convert to complimentary conditional call.  */
      insn = CONVERT_CONDITION_CALL (insn);
      bfd_putb32 (insn, contents + irel->r_offset);

      *insn_len = 4;
      irel->r_info =
	ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info), R_NDS32_NONE);
      hi_irelfn->r_info =
	ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info), R_NDS32_NONE);
      lo_irelfn->r_info =
	ELF32_R_INFO (ELF32_R_SYM (lo_irelfn->r_info), R_NDS32_NONE);

      cond_irelfn =
	find_relocs_at_address_addr (irel, internal_relocs, irelend,
				     R_NDS32_17_PCREL_RELA, laddr);
      if (cond_irelfn != irelend)
	{
	  cond_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info),
					      R_NDS32_17_PCREL_RELA);
	  cond_irelfn->r_addend = hi_irelfn->r_addend;
	}

      if (seq_len & 0x2)
	{
	  insn16 = NDS32_NOP16;
	  bfd_putb16 (insn16, contents + irel->r_offset + *insn_len);
	  hi_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info),
					    R_NDS32_INSN16);
	  hi_irelfn->r_addend = R_NDS32_INSN16_CONVERT_FLAG;
	  insn_len += 2;
	}
    }
  else if (foff >= -CONSERVATIVE_24BIT_S1 && foff < CONSERVATIVE_24BIT_S1)
    {
      /* Relax to the following instruction sequence
	 bltz  rt,   $1 ; LONGCALL2
	 jal   symbol   ; 25_PCREL
	 $1	*/
      *insn_len = 8;
      insn = INSN_JAL;
      bfd_putb32 (insn, contents + hi_irelfn->r_offset);

      hi_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info),
					R_NDS32_25_PCREL_RELA);
      irel->r_info =
	ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_LONGCALL2);

      lo_irelfn->r_info =
	ELF32_R_INFO (ELF32_R_SYM (lo_irelfn->r_info), R_NDS32_NONE);

      if (seq_len & 0x2)
	{
	  insn16 = NDS32_NOP16;
	  bfd_putb16 (insn16, contents + irel->r_offset + *insn_len);
	  lo_irelfn->r_info =
	    ELF32_R_INFO (ELF32_R_SYM (lo_irelfn->r_info), R_NDS32_INSN16);
	  lo_irelfn->r_addend = R_NDS32_INSN16_CONVERT_FLAG;
	  insn_len += 2;
	}
    }
  return true;
}

/* Relax LONGJUMP1 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longjump1 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* There are 3 variations for LONGJUMP1
     case 4-4-2; 16-bit bit on, optimize off or optimize for space
     sethi ta, hi20(symbol)	 ; LONGJUMP1/HI20
     ori   ta, ta, lo12(symbol)	 ; LO12S0
     jr5   ta			 ;

     case 4-4-4; 16-bit off, optimize don't care
     sethi ta, hi20(symbol)	 ; LONGJUMP1/HI20
     ori   ta, ta, lo12(symbol)	 ; LO12S0
     jr	   ta			 ;

     case 4-4-4; 16-bit on, optimize for speed
     sethi ta, hi20(symbol)	 ; LONGJUMP1/HI20
     ori   ta, ta, lo12(symbol)	 ; LO12S0
     jr	   ta			 ;	*/

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */

  bfd_vma laddr;
  int seq_len;	/* Original length of instruction sequence.  */
  int insn16_on;	/* 16-bit on/off.  */
  uint32_t insn;
  Elf_Internal_Rela *hi_irelfn, *lo_irelfn, *irelend;
  bfd_signed_vma foff;
  uint16_t insn16;
  unsigned long reloc;

  irelend = internal_relocs + sec->reloc_count;
  seq_len = GET_SEQ_LEN (irel->r_addend);
  laddr = irel->r_offset;
  *insn_len = seq_len;
  insn16_on = IS_16BIT_ON (irel->r_addend);

  hi_irelfn =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_HI20_RELA, laddr);
  lo_irelfn =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_LO12S0_ORI_RELA, laddr + 4);
  if (hi_irelfn == irelend || lo_irelfn == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGJUMP1",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, hi_irelfn, isymbuf, symtab_hdr);

  if (foff == 0
      || foff >= CONSERVATIVE_24BIT_S1
      || foff < -CONSERVATIVE_24BIT_S1)
    return false;

  if (insn16_on
      && foff >= -ACCURATE_8BIT_S1
      && foff < ACCURATE_8BIT_S1
      && (seq_len & 0x2))
    {
      /* j8	label */
      /* 16-bit on, but not optimized for speed.  */
      reloc = R_NDS32_9_PCREL_RELA;
      insn16 = INSN_J8;
      bfd_putb16 (insn16, contents + irel->r_offset);
      *insn_len = 2;
      irel->r_info =
	ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);
    }
  else
    {
      /* j     label */
      reloc = R_NDS32_25_PCREL_RELA;
      insn = INSN_J;
      bfd_putb32 (insn, contents + irel->r_offset);
      *insn_len = 4;
      irel->r_info =
	ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_INSN16);
      irel->r_addend = 0;
    }

  hi_irelfn->r_info =
    ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info), reloc);
  lo_irelfn->r_info =
    ELF32_R_INFO (ELF32_R_SYM (lo_irelfn->r_info), R_NDS32_NONE);

  if ((seq_len & 0x2) && ((*insn_len & 2) == 0))
    {
      insn16 = NDS32_NOP16;
      bfd_putb16 (insn16, contents + irel->r_offset + *insn_len);
      lo_irelfn->r_info =
	ELF32_R_INFO (ELF32_R_SYM (lo_irelfn->r_info),
		      R_NDS32_INSN16);
      lo_irelfn->r_addend = R_NDS32_INSN16_CONVERT_FLAG;
      *insn_len += 2;
    }
  return true;
}

/* Revert condition branch.  This function does not check if the input
   instruction is condition branch or not.  */

static void
nds32_elf_convert_branch (uint16_t insn16, uint32_t insn,
			   uint16_t *re_insn16, uint32_t *re_insn)
{
  uint32_t comp_insn = 0;
  uint16_t comp_insn16 = 0;

  if (insn)
    {
      if (N32_OP6 (insn) == N32_OP6_BR1)
	{
	  /* beqs label.  */
	  comp_insn = (insn ^ 0x4000) & 0xffffc000;
	  if (N32_IS_RT3 (insn) && N32_RA5 (insn) == REG_R5)
	    {
	      /* Insn can be contracted to 16-bit implied r5.  */
	      comp_insn16 =
		(comp_insn & 0x4000) ? INSN_BNES38 : INSN_BEQS38;
	      comp_insn16 |= (N32_RT5 (insn) & 0x7) << 8;
	    }
	}
      else if (N32_OP6 (insn) == N32_OP6_BR3)
	{
	  /* bnec $ta, imm11, label.  */
	  comp_insn = (insn ^ 0x80000) & 0xffffff00;
	}
      else
	{
	  comp_insn = (insn ^ 0x10000) & 0xffffc000;
	  if (N32_BR2_SUB (insn) == N32_BR2_BEQZ
	      || N32_BR2_SUB (insn) == N32_BR2_BNEZ)
	    {
	      if (N32_IS_RT3 (insn))
		{
		  /* Insn can be contracted to 16-bit.  */
		  comp_insn16 =
		    (comp_insn & 0x10000) ? INSN_BNEZ38 : INSN_BEQZ38;
		  comp_insn16 |= (N32_RT5 (insn) & 0x7) << 8;
		}
	      else if (N32_RT5 (insn) == REG_R15)
		{
		  /* Insn can be contracted to 16-bit.  */
		  comp_insn16 =
		    (comp_insn & 0x10000) ? INSN_BNES38 : INSN_BEQS38;
		}
	    }
	}
    }
  else
    {
      switch ((insn16 & 0xf000) >> 12)
	{
	case 0xc:
	  /* beqz38 or bnez38 */
	  comp_insn16 = (insn16 ^ 0x0800) & 0xff00;
	  comp_insn = (comp_insn16 & 0x0800) ? INSN_BNEZ : INSN_BEQZ;
	  comp_insn |= ((comp_insn16 & 0x0700) >> 8) << 20;
	  break;

	case 0xd:
	  /* beqs38 or bnes38 */
	  comp_insn16 = (insn16 ^ 0x0800) & 0xff00;
	  comp_insn = (comp_insn16 & 0x0800) ? INSN_BNE : INSN_BEQ;
	  comp_insn |= (((comp_insn16 & 0x0700) >> 8) << 20)
	    | (REG_R5 << 15);
	  break;

	case 0xe:
	  /* beqzS8 or bnezS8 */
	  comp_insn16 = (insn16 ^ 0x0100) & 0xff00;
	  comp_insn = (comp_insn16 & 0x0100) ? INSN_BNEZ : INSN_BEQZ;
	  comp_insn |= REG_R15 << 20;
	  break;

	default:
	  break;
	}
    }
  if (comp_insn && re_insn)
    *re_insn = comp_insn;
  if (comp_insn16 && re_insn16)
    *re_insn16 = comp_insn16;
}

/* Relax LONGJUMP2 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longjump2 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* There are 3 variations for LONGJUMP2
     case 2-4;  1st insn convertible, 16-bit on,
     optimize off or optimize for space
     bnes38  rt, ra, $1 ; LONGJUMP2
     j       label      ; 25_PCREL
     $1:

     case 4-4; 1st insn not convertible
     bne  rt, ra, $1 ; LONGJUMP2
     j    label      ; 25_PCREL
     $1:

     case 4-4; 1st insn convertible, 16-bit on, optimize for speed
     bne  rt, ra, $1 ; LONGJUMP2
     j    label      ; 25_PCREL
     $1: */

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */

  bfd_vma laddr;
  int seq_len;	/* Original length of instruction sequence.  */
  Elf_Internal_Rela *i2_irelfn, *cond_irelfn, *irelend;
  int first_size;
  unsigned int i;
  bfd_signed_vma foff;
  uint32_t insn, re_insn = 0;
  uint16_t insn16, re_insn16 = 0;
  unsigned long reloc, cond_reloc;

  enum elf_nds32_reloc_type checked_types[] =
    { R_NDS32_15_PCREL_RELA, R_NDS32_9_PCREL_RELA };

  irelend = internal_relocs + sec->reloc_count;
  seq_len = GET_SEQ_LEN (irel->r_addend);
  laddr = irel->r_offset;
  *insn_len = seq_len;
  first_size = (seq_len == 6) ? 2 : 4;

  i2_irelfn =
    find_relocs_at_address_addr (irel, internal_relocs,
				 irelend, R_NDS32_25_PCREL_RELA,
				 laddr + first_size);

  for (i = 0; i < ARRAY_SIZE (checked_types); i++)
    {
      cond_irelfn =
	find_relocs_at_address_addr (irel, internal_relocs, irelend,
				     checked_types[i], laddr);
      if (cond_irelfn != irelend)
	break;
    }

  if (i2_irelfn == irelend || cond_irelfn == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGJUMP2",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, i2_irelfn, isymbuf, symtab_hdr);
  if (foff == 0
      || foff < -CONSERVATIVE_16BIT_S1
      || foff >= CONSERVATIVE_16BIT_S1)
    return false;

  /* Get the all corresponding instructions.  */
  if (first_size == 4)
    {
      insn = bfd_getb32 (contents + laddr);
      nds32_elf_convert_branch (0, insn, &re_insn16, &re_insn);
    }
  else
    {
      insn16 = bfd_getb16 (contents + laddr);
      nds32_elf_convert_branch (insn16, 0, &re_insn16, &re_insn);
    }

  if (re_insn16 && foff >= -(ACCURATE_8BIT_S1 - first_size)
      && foff < ACCURATE_8BIT_S1 - first_size)
    {
      if (first_size == 4)
	{
	  /* Don't convert it to 16-bit now, keep this as relaxable for
	     ``label reloc; INSN16''.  */

	  /* Save comp_insn32 to buffer.  */
	  bfd_putb32 (re_insn, contents + irel->r_offset);
	  *insn_len = 4;
	  reloc = (N32_OP6 (re_insn) == N32_OP6_BR1) ?
	    R_NDS32_15_PCREL_RELA : R_NDS32_17_PCREL_RELA;
	  cond_reloc = R_NDS32_INSN16;
	}
      else
	{
	  bfd_putb16 (re_insn16, contents + irel->r_offset);
	  *insn_len = 2;
	  reloc = R_NDS32_9_PCREL_RELA;
	  cond_reloc = R_NDS32_NONE;
	}
    }
  else if (N32_OP6 (re_insn) == N32_OP6_BR1
	   && (foff >= -(ACCURATE_14BIT_S1 - first_size)
	       && foff < ACCURATE_14BIT_S1 - first_size))
    {
      /* beqs     label    ; 15_PCREL */
      bfd_putb32 (re_insn, contents + irel->r_offset);
      *insn_len = 4;
      reloc = R_NDS32_15_PCREL_RELA;
      cond_reloc = R_NDS32_NONE;
    }
  else if (N32_OP6 (re_insn) == N32_OP6_BR2
	   && foff >= -CONSERVATIVE_16BIT_S1
	   && foff < CONSERVATIVE_16BIT_S1)
    {
      /* beqz     label ; 17_PCREL */
      bfd_putb32 (re_insn, contents + irel->r_offset);
      *insn_len = 4;
      reloc = R_NDS32_17_PCREL_RELA;
      cond_reloc = R_NDS32_NONE;
    }
  else
    return false;

  /* Set all relocations.  */
  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (i2_irelfn->r_info), reloc);
  irel->r_addend = i2_irelfn->r_addend;

  cond_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (cond_irelfn->r_info),
				      cond_reloc);
  cond_irelfn->r_addend = 0;

  if ((seq_len ^ *insn_len ) & 0x2)
    {
      insn16 = NDS32_NOP16;
      bfd_putb16 (insn16, contents + irel->r_offset + 4);
      i2_irelfn->r_offset = 4;
      i2_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (i2_irelfn->r_info),
					R_NDS32_INSN16);
      i2_irelfn->r_addend = R_NDS32_INSN16_CONVERT_FLAG;
      *insn_len += 2;
    }
  else
    i2_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (i2_irelfn->r_info),
				      R_NDS32_NONE);
  return true;
}

/* Relax LONGJUMP3 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longjump3 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* There are 5 variations for LONGJUMP3
     case 1: 2-4-4-2; 1st insn convertible, 16-bit on,
     optimize off or optimize for space
     bnes38   rt, ra, $1	    ; LONGJUMP3
     sethi    ta, hi20(symbol)	    ; HI20
     ori      ta, ta, lo12(symbol)  ; LO12S0
     jr5      ta		    ;
     $1:			    ;

     case 2: 2-4-4-2; 1st insn convertible, 16-bit on, optimize for speed
     bnes38   rt, ra, $1	   ; LONGJUMP3
     sethi    ta, hi20(symbol)	   ; HI20
     ori      ta, ta, lo12(symbol) ; LO12S0
     jr5      ta		   ;
     $1:			   ; LABEL

     case 3: 4-4-4-2; 1st insn not convertible, 16-bit on,
     optimize off or optimize for space
     bne   rt, ra, $1		; LONGJUMP3
     sethi ta, hi20(symbol)	; HI20
     ori   ta, ta, lo12(symbol) ; LO12S0
     jr5   ta			;
     $1:			;

     case 4: 4-4-4-4; 1st insn don't care, 16-bit off, optimize don't care
     16-bit off if no INSN16
     bne   rt, ra, $1		; LONGJUMP3
     sethi ta, hi20(symbol)	; HI20
     ori   ta, ta, lo12(symbol) ; LO12S0
     jr	   ta			;
     $1:			;

     case 5: 4-4-4-4; 1st insn not convertible, 16-bit on, optimize for speed
     16-bit off if no INSN16
     bne   rt, ra, $1		; LONGJUMP3
     sethi ta, hi20(symbol)	; HI20
     ori   ta, ta, lo12(symbol) ; LO12S0
     jr	   ta			;
     $1:			; LABEL */

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */
  enum elf_nds32_reloc_type checked_types[] =
    { R_NDS32_15_PCREL_RELA, R_NDS32_9_PCREL_RELA };

  int reloc_off = 0, cond_removed = 0, convertible;
  bfd_vma laddr;
  int seq_len;	/* Original length of instruction sequence.  */
  Elf_Internal_Rela *hi_irelfn, *lo_irelfn, *cond_irelfn, *irelend;
  int first_size;
  unsigned int i;
  bfd_signed_vma foff;
  uint32_t insn, re_insn = 0;
  uint16_t insn16, re_insn16 = 0;
  unsigned long reloc, cond_reloc;

  irelend = internal_relocs + sec->reloc_count;
  seq_len = GET_SEQ_LEN (irel->r_addend);
  laddr = irel->r_offset;
  *insn_len = seq_len;

  convertible = IS_1ST_CONVERT (irel->r_addend);

  if (convertible)
    first_size = 2;
  else
    first_size = 4;

  /* Get all needed relocations.  */
  hi_irelfn =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_HI20_RELA, laddr + first_size);
  lo_irelfn =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_LO12S0_ORI_RELA,
				 laddr + first_size + 4);

  for (i = 0; i < ARRAY_SIZE (checked_types); i++)
    {
      cond_irelfn =
	find_relocs_at_address_addr (irel, internal_relocs, irelend,
				     checked_types[i], laddr);
      if (cond_irelfn != irelend)
	break;
    }

  if (hi_irelfn == irelend
      || lo_irelfn == irelend
      || cond_irelfn == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGJUMP3",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, hi_irelfn, isymbuf, symtab_hdr);

  if (foff == 0
      || foff < -CONSERVATIVE_24BIT_S1
      || foff >= CONSERVATIVE_24BIT_S1)
    return false;

  /* Get the all corresponding instructions.  */
  if (first_size == 4)
    {
      insn = bfd_getb32 (contents + laddr);
      nds32_elf_convert_branch (0, insn, &re_insn16, &re_insn);
    }
  else
    {
      insn16 = bfd_getb16 (contents + laddr);
      nds32_elf_convert_branch (insn16, 0, &re_insn16, &re_insn);
    }

  /* For simplicity of coding, we are going to modify the section
     contents, the section relocs, and the BFD symbol table.  We
     must tell the rest of the code not to free up this
     information.  It would be possible to instead create a table
     of changes which have to be made, as is done in coff-mips.c;
     that would be more work, but would require less memory when
     the linker is run.  */

  if (re_insn16
      && foff >= -ACCURATE_8BIT_S1 - first_size
      && foff < ACCURATE_8BIT_S1 - first_size)
    {
      if (!(seq_len & 0x2))
	{
	  /* Don't convert it to 16-bit now, keep this as relaxable
	     for ``label reloc; INSN1a''6.  */
	  /* Save comp_insn32 to buffer.  */
	  bfd_putb32 (re_insn, contents + irel->r_offset);
	  *insn_len = 4;
	  reloc = (N32_OP6 (re_insn) == N32_OP6_BR1) ?
	    R_NDS32_15_PCREL_RELA : R_NDS32_17_PCREL_RELA;
	  cond_reloc = R_NDS32_INSN16;
	}
      else
	{
	  /* Not optimize for speed; convert sequence to 16-bit.  */
	  /* Save comp_insn16 to buffer.  */
	  bfd_putb16 (re_insn16, contents + irel->r_offset);
	  *insn_len = 2;
	  reloc = R_NDS32_9_PCREL_RELA;
	  cond_reloc = R_NDS32_NONE;
	}
      cond_removed = 1;
    }
  else if (N32_OP6 (re_insn) == N32_OP6_BR1
	   && (foff >= -(ACCURATE_14BIT_S1 - first_size)
	       && foff < ACCURATE_14BIT_S1 - first_size))
    {
      /* beqs     label    ; 15_PCREL */
      bfd_putb32 (re_insn, contents + irel->r_offset);
      *insn_len = 4;
      reloc = R_NDS32_15_PCREL_RELA;
      cond_reloc = R_NDS32_NONE;
      cond_removed = 1;
    }
  else if (N32_OP6 (re_insn) == N32_OP6_BR2
	   && foff >= -CONSERVATIVE_16BIT_S1
	   && foff < CONSERVATIVE_16BIT_S1)
    {
      /* beqz     label ; 17_PCREL */
      bfd_putb32 (re_insn, contents + irel->r_offset);
      *insn_len = 4;
      reloc = R_NDS32_17_PCREL_RELA;
      cond_reloc = R_NDS32_NONE;
      cond_removed = 1;
    }
  else if (foff >= -CONSERVATIVE_24BIT_S1 - reloc_off
	   && foff < CONSERVATIVE_24BIT_S1 - reloc_off)
    {
      /* Relax to one of the following 3 variations

	 case 2-4; 1st insn convertible, 16-bit on, optimize off or optimize
	 for space
	 bnes38  rt, $1 ; LONGJUMP2
	 j       label  ; 25_PCREL
	 $1

	 case 4-4; 1st insn not convertible, others don't care
	 bne   rt, ra, $1 ; LONGJUMP2
	 j     label      ; 25_PCREL
	 $1

	 case 4-4; 1st insn convertible, 16-bit on, optimize for speed
	 bne   rt, ra, $1 ; LONGJUMP2
	 j     label      ; 25_PCREL
	 $1 */

      /* Offset for first instruction.  */

      /* Use j label as second instruction.  */
      *insn_len = 4 + first_size;
      insn = INSN_J;
      bfd_putb32 (insn, contents + hi_irelfn->r_offset);
      reloc = R_NDS32_LONGJUMP2;
      cond_reloc = R_NDS32_25_PLTREL;
    }
    else
      return false;

    if (cond_removed == 1)
      {
	/* Set all relocations.  */
	irel->r_info = ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info), reloc);
	irel->r_addend = hi_irelfn->r_addend;

	cond_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (cond_irelfn->r_info),
					    cond_reloc);
	cond_irelfn->r_addend = 0;
	hi_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info),
					  R_NDS32_NONE);
      }
    else
      {
	irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), reloc);
	irel->r_addend = irel->r_addend;
	hi_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info),
					  cond_reloc);
      }

  if ((seq_len ^ *insn_len ) & 0x2)
    {
      insn16 = NDS32_NOP16;
      bfd_putb16 (insn16, contents + irel->r_offset + *insn_len);
      lo_irelfn->r_offset = *insn_len;
      lo_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (lo_irelfn->r_info),
					R_NDS32_INSN16);
      lo_irelfn->r_addend = R_NDS32_INSN16_CONVERT_FLAG;
      *insn_len += 2;
    }
  else
    lo_irelfn->r_info = ELF32_R_INFO (ELF32_R_SYM (lo_irelfn->r_info),
				      R_NDS32_NONE);
  return true;
}

/* Relax LONGCALL4 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longcall4 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* The pattern for LONGCALL4.  Support for function cse.
     sethi ta, hi20(symbol)	; LONGCALL4/HI20
     ori   ta, ta, lo12(symbol)	; LO12S0_ORI/PTR
     jral  ta			; PTR_RES/EMPTY/INSN16  */

  bfd_vma laddr;
  uint32_t insn;
  Elf_Internal_Rela *hi_irel, *ptr_irel, *insn_irel, *em_irel, *call_irel;
  Elf_Internal_Rela *irelend;
  bfd_signed_vma foff;

  irelend = internal_relocs + sec->reloc_count;
  laddr = irel->r_offset;

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */
  hi_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					 R_NDS32_HI20_RELA, laddr);

  if (hi_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGCALL4",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, hi_irel, isymbuf, symtab_hdr);

  /* This condition only happened when symbol is undefined.  */
  if (foff == 0
      || foff < -CONSERVATIVE_24BIT_S1
      || foff >= CONSERVATIVE_24BIT_S1)
    return false;

  /* Relax to: jal symbol; 25_PCREL.  */
  /* For simplicity of coding, we are going to modify the section
     contents, the section relocs, and the BFD symbol table.  We
     must tell the rest of the code not to free up this
     information.  It would be possible to instead create a table
     of changes which have to be made, as is done in coff-mips.c;
     that would be more work, but would require less memory when
     the linker is run.  */

  ptr_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					  R_NDS32_PTR_RESOLVED, irel->r_addend);
  em_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					  R_NDS32_EMPTY, irel->r_addend);

  if (ptr_irel == irelend || em_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGCALL4",
			  (uint64_t) irel->r_offset);
      return false;
    }
  /* Check these is enough space to insert jal in R_NDS32_EMPTY.  */
  insn = bfd_getb32 (contents + irel->r_addend);
  if (insn & 0x80000000)
    return false;

  /* Replace the long call with a jal.  */
  em_irel->r_info = ELF32_R_INFO (ELF32_R_SYM (em_irel->r_info),
				  R_NDS32_25_PCREL_RELA);
  ptr_irel->r_addend = 1;

  /* We don't resolve this here but resolve it in relocate_section.  */
  insn = INSN_JAL;
  bfd_putb32 (insn, contents + em_irel->r_offset);

  irel->r_info =
    ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);

  /* If there is function cse, HI20 can not remove now.  */
  call_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					   R_NDS32_LONGCALL4, laddr);
  if (call_irel == irelend)
    {
      *insn_len = 0;
      hi_irel->r_info =
	ELF32_R_INFO (ELF32_R_SYM (hi_irel->r_info), R_NDS32_NONE);
    }

  insn_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					  R_NDS32_INSN16, irel->r_addend);
  if (insn_irel != irelend)
    insn_irel->r_info =
      ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);

  return true;
}

/* Relax LONGCALL5 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longcall5 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* The pattern for LONGCALL5.
     bltz  rt, .L1	; LONGCALL5/17_PCREL
     jal   symbol	; 25_PCREL
     .L1:  */

  bfd_vma laddr;
  uint32_t insn;
  Elf_Internal_Rela *cond_irel, *irelend;
  bfd_signed_vma foff;

  irelend = internal_relocs + sec->reloc_count;
  laddr = irel->r_offset;
  insn = bfd_getb32 (contents + laddr);

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */
  cond_irel =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_25_PCREL_RELA, irel->r_addend);
  if (cond_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGCALL5",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, cond_irel, isymbuf, symtab_hdr);

  if (foff == 0
      || foff < -CONSERVATIVE_16BIT_S1
      || foff >= CONSERVATIVE_16BIT_S1)
    return false;

  /* Relax to	bgezal   rt, label ; 17_PCREL
     or		bltzal   rt, label ; 17_PCREL.  */

  /* Convert to complimentary conditional call.  */
  insn = CONVERT_CONDITION_CALL (insn);

  /* For simplicity of coding, we are going to modify the section
     contents, the section relocs, and the BFD symbol table.  We
     must tell the rest of the code not to free up this
     information.  It would be possible to instead create a table
     of changes which have to be made, as is done in coff-mips.c;
     that would be more work, but would require less memory when
     the linker is run.  */

  /* Modify relocation and contents.  */
  cond_irel->r_info =
    ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info), R_NDS32_17_PCREL_RELA);

  /* Replace the long call with a bgezal.  */
  bfd_putb32 (insn, contents + cond_irel->r_offset);
  *insn_len = 0;

  /* Clean unnessary relocations.  */
  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);

  cond_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					   R_NDS32_17_PCREL_RELA, laddr);
  cond_irel->r_info =
    ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info), R_NDS32_NONE);

  return true;
}

/* Relax LONGCALL6 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longcall6 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* The pattern for LONGCALL6.
     bltz  rt,   .L1			; LONGCALL6/17_PCREL
     sethi ta,   hi20(symbol)		; HI20/PTR
     ori   ta, ta,  lo12(symbol)	; LO12S0_ORI/PTR
     jral  ta				; PTR_RES/EMPTY/INSN16
     .L1  */

  bfd_vma laddr;
  uint32_t insn;
  Elf_Internal_Rela *em_irel, *cond_irel, *irelend;
  bfd_signed_vma foff;

  irelend = internal_relocs + sec->reloc_count;
  laddr = irel->r_offset;

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */
  em_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					 R_NDS32_EMPTY, irel->r_addend);

  if (em_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGCALL6",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, em_irel, isymbuf, symtab_hdr);

  if (foff == 0
      || foff < -CONSERVATIVE_24BIT_S1
      || foff >= CONSERVATIVE_24BIT_S1)
    return false;

  /* Check these is enough space to insert jal in R_NDS32_EMPTY.  */
  insn = bfd_getb32 (contents + irel->r_addend);
  if (insn & 0x80000000)
    return false;

  insn = bfd_getb32 (contents + laddr);
  if (foff >= -CONSERVATIVE_16BIT_S1 && foff < CONSERVATIVE_16BIT_S1)
    {
      /* Relax to  bgezal   rt, label ; 17_PCREL
	 or	   bltzal   rt, label ; 17_PCREL.  */

      /* Convert to complimentary conditional call.  */
      *insn_len = 0;
      insn = CONVERT_CONDITION_CALL (insn);
      bfd_putb32 (insn, contents + em_irel->r_offset);

      em_irel->r_info =
	ELF32_R_INFO (ELF32_R_SYM (em_irel->r_info), R_NDS32_17_PCREL_RELA);

      /* Set resolved relocation.  */
      cond_irel =
	find_relocs_at_address_addr (irel, internal_relocs, irelend,
				     R_NDS32_PTR_RESOLVED, irel->r_addend);
      if (cond_irel == irelend)
	{
	  _bfd_error_handler (unrecognized_reloc_msg, abfd,
			      "R_NDS32_LONGCALL6", (uint64_t) irel->r_offset);
	  return false;
	}
      cond_irel->r_addend = 1;

      /* Clear relocations.  */

      irel->r_info =
	ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);

      cond_irel =
	find_relocs_at_address_addr (irel, internal_relocs, irelend,
				     R_NDS32_17_PCREL_RELA, laddr);
      if (cond_irel != irelend)
	cond_irel->r_info =
	  ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info), R_NDS32_NONE);

      cond_irel =
	find_relocs_at_address_addr (irel, internal_relocs, irelend,
				     R_NDS32_INSN16, irel->r_addend);
      if (cond_irel != irelend)
	cond_irel->r_info =
	  ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info), R_NDS32_NONE);

    }
  else if (foff >= -CONSERVATIVE_24BIT_S1 && foff < CONSERVATIVE_24BIT_S1)
    {
      /* Relax to the following instruction sequence
	 bltz  rt, .L1	; LONGCALL2/17_PCREL
	 jal   symbol	; 25_PCREL/PTR_RES
	 .L1  */
      *insn_len = 4;
      /* Convert instruction.  */
      insn = INSN_JAL;
      bfd_putb32 (insn, contents + em_irel->r_offset);

      /* Convert relocations.  */
      em_irel->r_info = ELF32_R_INFO (ELF32_R_SYM (em_irel->r_info),
				      R_NDS32_25_PCREL_RELA);
      irel->r_info =
	ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_LONGCALL5);

      /* Set resolved relocation.  */
      cond_irel =
	find_relocs_at_address_addr (irel, internal_relocs, irelend,
				     R_NDS32_PTR_RESOLVED, irel->r_addend);
      if (cond_irel == irelend)
	{
	  _bfd_error_handler (unrecognized_reloc_msg, abfd,
			      "R_NDS32_LONGCALL6", (uint64_t) irel->r_offset);
	  return false;
	}
      cond_irel->r_addend = 1;

      cond_irel =
	find_relocs_at_address_addr (irel, internal_relocs, irelend,
				     R_NDS32_INSN16, irel->r_addend);
      if (cond_irel != irelend)
	cond_irel->r_info =
	  ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info), R_NDS32_NONE);
    }
  return true;
}

/* Relax LONGJUMP4 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longjump4 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* The pattern for LONGJUMP4.
     sethi ta, hi20(symbol)	; LONGJUMP4/HI20
     ori   ta, ta, lo12(symbol)	; LO12S0_ORI/PTR
     jr    ta			; PTR_RES/INSN16/EMPTY  */

  bfd_vma laddr;
  int seq_len;	/* Original length of instruction sequence.  */
  uint32_t insn;
  Elf_Internal_Rela *hi_irel, *ptr_irel, *em_irel, *call_irel, *irelend;
  bfd_signed_vma foff;

  irelend = internal_relocs + sec->reloc_count;
  seq_len = GET_SEQ_LEN (irel->r_addend);
  laddr = irel->r_offset;
  *insn_len = seq_len;

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */

  hi_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					 R_NDS32_HI20_RELA, laddr);

  if (hi_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGJUMP4",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, hi_irel, isymbuf, symtab_hdr);

  if (foff == 0
      || foff >= CONSERVATIVE_24BIT_S1
      || foff < -CONSERVATIVE_24BIT_S1)
    return false;

  /* Convert it to "j label", it may be converted to j8 in the final
     pass of relaxation.  Therefore, we do not consider this currently.  */
  ptr_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					  R_NDS32_PTR_RESOLVED, irel->r_addend);
  em_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					 R_NDS32_EMPTY, irel->r_addend);

  if (ptr_irel == irelend || em_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGJUMP4",
			  (uint64_t) irel->r_offset);
      return false;
    }

  em_irel->r_info =
    ELF32_R_INFO (ELF32_R_SYM (em_irel->r_info), R_NDS32_25_PCREL_RELA);
  ptr_irel->r_addend = 1;

  /* Write instruction.  */
  insn = INSN_J;
  bfd_putb32 (insn, contents + em_irel->r_offset);

  /* Clear relocations.  */
  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);

  /* If there is function cse, HI20 can not remove now.  */
  call_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					   R_NDS32_LONGJUMP4, laddr);
  if (call_irel == irelend)
    {
      *insn_len = 0;
      hi_irel->r_info =
	ELF32_R_INFO (ELF32_R_SYM (hi_irel->r_info), R_NDS32_NONE);
    }

  return true;
}

/* Relax LONGJUMP5 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longjump5 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   int *seq_len, bfd_byte *contents,
			   Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* There are 2 variations for LONGJUMP5
     case 2-4;  1st insn convertible, 16-bit on.
     bnes38  rt, ra, .L1	; LONGJUMP5/9_PCREL/INSN16
     j       label		; 25_PCREL/INSN16
     $1:

     case 4-4; 1st insn not convertible
     bne  rt, ra, .L1	; LONGJUMP5/15_PCREL/INSN16
     j    label		; 25_PCREL/INSN16
     .L1:  */

  bfd_vma laddr;
  Elf_Internal_Rela *cond_irel,  *irelend;
  unsigned int i;
  bfd_signed_vma foff;
  uint32_t insn, re_insn = 0;
  uint16_t insn16, re_insn16 = 0;
  unsigned long reloc;

  enum elf_nds32_reloc_type checked_types[] =
    { R_NDS32_17_PCREL_RELA, R_NDS32_15_PCREL_RELA,
      R_NDS32_9_PCREL_RELA, R_NDS32_INSN16 };

  irelend = internal_relocs + sec->reloc_count;
  laddr = irel->r_offset;

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */

  cond_irel =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_25_PCREL_RELA, irel->r_addend);
  if (cond_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGJUMP5",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, cond_irel, isymbuf, symtab_hdr);

  if (foff == 0
      || foff < -CONSERVATIVE_16BIT_S1
      || foff >= CONSERVATIVE_16BIT_S1)
    return false;

  /* Get the all corresponding instructions.  */
  insn = bfd_getb32 (contents + laddr);
  /* Check instruction size.  */
  if (insn & 0x80000000)
    {
      *seq_len = 0;
      insn16 = insn >> 16;
      nds32_elf_convert_branch (insn16, 0, &re_insn16, &re_insn);
    }
  else
    nds32_elf_convert_branch (0, insn, &re_insn16, &re_insn);

  if (N32_OP6 (re_insn) == N32_OP6_BR1
      && (foff >= -CONSERVATIVE_14BIT_S1 && foff < CONSERVATIVE_14BIT_S1))
    {
      /* beqs label ; 15_PCREL.  */
      bfd_putb32 (re_insn, contents + cond_irel->r_offset);
      reloc = R_NDS32_15_PCREL_RELA;
    }
  else if (N32_OP6 (re_insn) == N32_OP6_BR2
	   && foff >= -CONSERVATIVE_16BIT_S1 && foff < CONSERVATIVE_16BIT_S1)
    {
      /* beqz label ; 17_PCREL.  */
      bfd_putb32 (re_insn, contents + cond_irel->r_offset);
      reloc = R_NDS32_17_PCREL_RELA;
    }
  else if ( N32_OP6 (re_insn) == N32_OP6_BR3
	   && foff >= -CONSERVATIVE_8BIT_S1 && foff < CONSERVATIVE_8BIT_S1)
    {
      /* beqc label ; 9_PCREL.  */
      bfd_putb32 (re_insn, contents + cond_irel->r_offset);
      reloc = R_NDS32_WORD_9_PCREL_RELA;
    }
  else
    return false;

  /* Set all relocations.  */
  cond_irel->r_info = ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info), reloc);

  /* Clean relocations.  */
  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);
  for (i = 0; i < ARRAY_SIZE (checked_types); i++)
    {
      cond_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					       checked_types[i], laddr);
      if (cond_irel != irelend)
	{
	  if (*seq_len == 0
	      && (ELF32_R_TYPE (cond_irel->r_info) == R_NDS32_INSN16))
	    {
	      /* If the branch instruction is 2 byte, it cannot remove
		 directly.  Only convert it to nop16 and remove it after
		 checking alignment issue.  */
	      insn16 = NDS32_NOP16;
	      bfd_putb16 (insn16, contents + laddr);
	      cond_irel->r_addend = R_NDS32_INSN16_CONVERT_FLAG;
	    }
	  else
	    cond_irel->r_info = ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info),
					      R_NDS32_NONE);
	}
    }
  *insn_len = 0;

  return true;
}

/* Relax LONGJUMP6 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longjump6 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   int *seq_len, bfd_byte *contents,
			   Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* There are 5 variations for LONGJUMP6
     case : 2-4-4-4; 1st insn convertible, 16-bit on.
     bnes38   rt, ra, .L1		; LONGJUMP6/15_PCREL/INSN16
     sethi    ta, hi20(symbol)		; HI20/PTR
     ori      ta, ta, lo12(symbol)	; LO12S0_ORI/PTR
     jr       ta			; PTR_RES/INSN16/EMPTY
     .L1:

     case : 4-4-4-4; 1st insn not convertible, 16-bit on.
     bne   rt, ra, .L1		; LONGJUMP6/15_PCREL/INSN16
     sethi ta, hi20(symbol)	; HI20/PTR
     ori   ta, ta, lo12(symbol)	; LO12S0_ORI/PTR
     jr    ta			; PTR_RES/INSN16/EMPTY
     .L1:  */

  enum elf_nds32_reloc_type checked_types[] =
    { R_NDS32_17_PCREL_RELA, R_NDS32_15_PCREL_RELA,
      R_NDS32_9_PCREL_RELA, R_NDS32_INSN16 };

  int reloc_off = 0, cond_removed = 0;
  bfd_vma laddr;
  Elf_Internal_Rela *cond_irel, *em_irel, *irelend, *insn_irel;
  unsigned int i;
  bfd_signed_vma foff;
  uint32_t insn, re_insn = 0;
  uint16_t insn16, re_insn16 = 0;
  unsigned long reloc;

  irelend = internal_relocs + sec->reloc_count;
  laddr = irel->r_offset;

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */
  em_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					 R_NDS32_EMPTY, irel->r_addend);

  if (em_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGJUMP6",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, em_irel, isymbuf, symtab_hdr);

  if (foff == 0
      || foff < -CONSERVATIVE_24BIT_S1
      || foff >= CONSERVATIVE_24BIT_S1)
    return false;

  insn = bfd_getb32 (contents + laddr);
  /* Check instruction size.  */
  if (insn & 0x80000000)
    {
      *seq_len = 0;
      insn16 = insn >> 16;
      nds32_elf_convert_branch (insn16, 0, &re_insn16, &re_insn);
    }
  else
    nds32_elf_convert_branch (0, insn, &re_insn16, &re_insn);

  /* For simplicity of coding, we are going to modify the section
     contents, the section relocs, and the BFD symbol table.  We
     must tell the rest of the code not to free up this
     information.  It would be possible to instead create a table
     of changes which have to be made, as is done in coff-mips.c;
     that would be more work, but would require less memory when
     the linker is run.  */

  if (N32_OP6 (re_insn) == N32_OP6_BR1
      && (foff >= -CONSERVATIVE_14BIT_S1 && foff < CONSERVATIVE_14BIT_S1))
    {
      /* beqs     label    ; 15_PCREL.  */
      bfd_putb32 (re_insn, contents + em_irel->r_offset);
      reloc = R_NDS32_15_PCREL_RELA;
      cond_removed = 1;
    }
  else if (N32_OP6 (re_insn) == N32_OP6_BR2
	   && foff >= -CONSERVATIVE_16BIT_S1 && foff < CONSERVATIVE_16BIT_S1)
    {
      /* beqz     label ; 17_PCREL.  */
      bfd_putb32 (re_insn, contents + em_irel->r_offset);
      reloc = R_NDS32_17_PCREL_RELA;
      cond_removed = 1;
    }
  else if (foff >= -CONSERVATIVE_24BIT_S1 - reloc_off
	   && foff < CONSERVATIVE_24BIT_S1 - reloc_off)
    {
      /* Relax to one of the following 2 variations

	 case 2-4;  1st insn convertible, 16-bit on.
	 bnes38  rt, ra, .L1	; LONGJUMP5/9_PCREL/INSN16
	 j       label		; 25_PCREL/INSN16
	 $1:

	 case 4-4; 1st insn not convertible
	 bne  rt, ra, .L1	; LONGJUMP5/15_PCREL/INSN16
	 j    label		; 25_PCREL/INSN16
	 .L1:  */

      /* Use j label as second instruction.  */
      insn = INSN_J;
      reloc = R_NDS32_25_PCREL_RELA;
      bfd_putb32 (insn, contents + em_irel->r_offset);
    }
  else
    return false;

  /* Set all relocations.  */
  em_irel->r_info = ELF32_R_INFO (ELF32_R_SYM (em_irel->r_info), reloc);

  cond_irel =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_PTR_RESOLVED, em_irel->r_offset);
  cond_irel->r_addend = 1;

  /* Use INSN16 of first branch instruction to distinguish if keeping
     INSN16 of final instruction or not.  */
  insn_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					   R_NDS32_INSN16, irel->r_offset);
  if (insn_irel == irelend)
    {
      /* Clean the final INSN16.  */
      insn_irel =
	find_relocs_at_address_addr (irel, internal_relocs, irelend,
				     R_NDS32_INSN16, em_irel->r_offset);
      insn_irel->r_info = ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info),
					R_NDS32_NONE);
    }

  if (cond_removed == 1)
    {
      *insn_len = 0;

      /* Clear relocations.  */
      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);

      for (i = 0; i < ARRAY_SIZE (checked_types); i++)
	{
	  cond_irel =
	    find_relocs_at_address_addr (irel, internal_relocs, irelend,
					 checked_types[i], laddr);
	  if (cond_irel != irelend)
	    {
	      if (*seq_len == 0
		  && (ELF32_R_TYPE (cond_irel->r_info) == R_NDS32_INSN16))
		{
		  /* If the branch instruction is 2 byte, it cannot remove
		     directly.  Only convert it to nop16 and remove it after
		     checking alignment issue.  */
		  insn16 = NDS32_NOP16;
		  bfd_putb16 (insn16, contents + laddr);
		  cond_irel->r_addend = R_NDS32_INSN16_CONVERT_FLAG;
		}
	      else
		cond_irel->r_info =
		  ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info), R_NDS32_NONE);
	    }
	}
    }
  else
    {
      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
				   R_NDS32_LONGJUMP5);
    }

  return true;
}

/* Relax LONGJUMP7 relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_longjump7 (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   int *seq_len, bfd_byte *contents,
			   Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr)
{
  /* There are 2 variations for LONGJUMP5
     case 2-4;  1st insn convertible, 16-bit on.
     movi55  ta, imm11		; LONGJUMP7/INSN16
     beq     rt, ta, label	; 15_PCREL

     case 4-4; 1st insn not convertible
     movi55  ta, imm11		; LONGJUMP7/INSN16
     beq     rt, ta, label	; 15_PCREL  */

  bfd_vma laddr;
  Elf_Internal_Rela *cond_irel,  *irelend, *insn_irel;
  bfd_signed_vma foff;
  uint32_t insn, re_insn = 0;
  uint16_t insn16;
  uint32_t imm11;

  irelend = internal_relocs + sec->reloc_count;
  laddr = irel->r_offset;

  /* Get the reloc for the address from which the register is
     being loaded.  This reloc will tell us which function is
     actually being called.  */

  cond_irel =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_15_PCREL_RELA, irel->r_addend);
  if (cond_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LONGJUMP7",
			  (uint64_t) irel->r_offset);
      return false;
    }

  /* Get the value of the symbol referred to by the reloc.  */
  foff = calculate_offset (abfd, sec, cond_irel, isymbuf, symtab_hdr);

  if (foff == 0
      || foff < -CONSERVATIVE_8BIT_S1
      || foff >= CONSERVATIVE_8BIT_S1)
    return false;

  /* Get the first instruction for its size.  */
  insn = bfd_getb32 (contents + laddr);
  if (insn & 0x80000000)
    {
      *seq_len = 0;
      /* Get the immediate from movi55.  */
      imm11 = N16_IMM5S (insn >> 16);
    }
  else
    {
      /* Get the immediate from movi.  */
      imm11 = N32_IMM20S (insn);
    }

  /* Get the branch instruction.  */
  insn = bfd_getb32 (contents + irel->r_addend);
  /* Convert instruction to BR3.  */
  if ((insn >> 14) & 0x1)
    re_insn = N32_BR3 (BNEC, N32_RT5 (insn), imm11, 0);
  else
    re_insn = N32_BR3 (BEQC, N32_RT5 (insn), imm11, 0);

  bfd_putb32 (re_insn, contents + cond_irel->r_offset);

  /* Set all relocations.  */
  cond_irel->r_info = ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info),
				    R_NDS32_WORD_9_PCREL_RELA);

  /* Clean relocations.  */
  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);
  insn_irel = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					   R_NDS32_INSN16, irel->r_offset);
  if (insn_irel != irelend)
    {
      if (*seq_len == 0)
	{
	  /* If the first insntruction is 16bit, convert it to nop16.  */
	  insn16 = NDS32_NOP16;
	  bfd_putb16 (insn16, contents + laddr);
	  insn_irel->r_addend = R_NDS32_INSN16_CONVERT_FLAG;
	}
      else
	cond_irel->r_info = ELF32_R_INFO (ELF32_R_SYM (cond_irel->r_info),
					  R_NDS32_NONE);
    }
  *insn_len = 0;

  return true;
}

/* We figure out and reassign the best gp value in nds32_elf_final_sda_base
   for each relax round. But the gp may changed dramatically and then cause
   the truncated to fit errors for the the converted gp instructions.
   Therefore, we must reserve the minimum but safe enough size to prevent it.  */

static bool
nds32_elf_relax_guard (bfd_vma *access_addr, bfd_vma local_sda, asection *sec,
		       Elf_Internal_Rela *irel, bool *again,
		       bool init,
		       struct elf_nds32_link_hash_table *table,
		       Elf_Internal_Sym *isymbuf, Elf_Internal_Shdr *symtab_hdr)

{
  int offset_to_gp;
  static bool sec_pass = false;
  static asection *first_sec = NULL, *sym_sec;
  /* Record the number of instructions which may be removed.  */
  static int count = 0, record_count;
  Elf_Internal_Sym *isym;
  struct elf_link_hash_entry *h = NULL;
  int indx;
  unsigned long r_symndx;
  bfd *abfd = sec->owner;
  static bfd_vma record_sda = 0;
  int sda_offset = 0;

  /* Force doing relaxation when hyper-relax is high.  */
  if (table->hyper_relax == 2)
    return true;

  /* Do not relax the load/store patterns for the first
     relax round.  */
  if (init)
    {
      if (!first_sec)
	first_sec = sec;
      else if (first_sec == sec)
	{
	  record_count = count;
	  count = 0;
	  sec_pass = true;
	}

      if (!sec_pass)
	*again = true;

      return true;
    }

  /* Generally, _SDA_BASE_ is fixed or smaller. But the large
     DATA_SEGMENT_ALIGN size in the linker script may make it
     get even bigger.  */
  if (record_sda == 0)
    record_sda = local_sda;
  else if (local_sda > record_sda)
    sda_offset = local_sda - record_sda;

  /* Assume the instruction will be removed in the best case.  */
  count++;

  /* We record the offset to gp for each symbol, and then check
     if it is changed dramatically after relaxing.
     (global symbol): elf32_nds32_hash_entry (h)->offset_to_gp
     (local symbol) : elf32_nds32_local_gp_offset (abfd)[r_symndx].  */
  r_symndx = ELF32_R_SYM (irel->r_info);
  if (r_symndx >= symtab_hdr->sh_info)
    {
      /* Global symbols.  */
      indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
      h = elf_sym_hashes (abfd)[indx];
      sym_sec = h->root.u.def.section;
      if (NDS32_GUARD_SEC_P (sym_sec->flags)
	  || bfd_is_abs_section (sym_sec))
	{
	  /* Forbid doing relaxation when hyper-relax is low.  */
	  if (table->hyper_relax == 0)
	    return false;

	  offset_to_gp = *access_addr - local_sda;
	  if (elf32_nds32_hash_entry (h)->offset_to_gp == 0)
	    elf32_nds32_hash_entry (h)->offset_to_gp = offset_to_gp;
	  else if (abs (elf32_nds32_hash_entry (h)->offset_to_gp)
		   < abs (offset_to_gp) - sda_offset)
	    {
	      /* This may cause the error, so we reserve the
		 safe enough size for relaxing.  */
	      if (*access_addr >= local_sda)
		*access_addr += (record_count * 4);
	      else
		*access_addr -= (record_count * 4);
	    }
	  return sec_pass;
	}
    }
  else
    {
      /* Local symbols.  */
      if (!elf32_nds32_allocate_local_sym_info (abfd))
	return false;
      isym = isymbuf + r_symndx;

      sym_sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
      if (NDS32_GUARD_SEC_P (sym_sec->flags))
	{
	  /* Forbid doing relaxation when hyper-relax is low.  */
	  if (table->hyper_relax == 0)
	    return false;

	  offset_to_gp = *access_addr - local_sda;
	  if (elf32_nds32_local_gp_offset (abfd)[r_symndx] == 0)
	    elf32_nds32_local_gp_offset (abfd)[r_symndx] = offset_to_gp;
	  else if (abs (elf32_nds32_local_gp_offset (abfd)[r_symndx])
		   < abs (offset_to_gp) - sda_offset)
	    {
	      /* This may cause the error, so we reserve the
		 safe enough size for relaxing.  */
	      if (*access_addr >= local_sda)
		*access_addr += (record_count * 4);
	      else
		*access_addr -= (record_count * 4);
	    }
	  return sec_pass;
	}
    }

  return true;
}

#define GET_LOADSTORE_RANGE(addend) (((addend) >> 8) & 0x3f)

/* Relax LOADSTORE relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_loadstore (struct bfd_link_info *link_info, bfd *abfd,
			   asection *sec, Elf_Internal_Rela *irel,
			   Elf_Internal_Rela *internal_relocs, int *insn_len,
			   bfd_byte *contents, Elf_Internal_Sym *isymbuf,
			   Elf_Internal_Shdr *symtab_hdr, int load_store_relax,
			   struct elf_nds32_link_hash_table *table)
{
  int eliminate_sethi = 0, range_type;
  unsigned int i;
  bfd_vma local_sda, laddr;
  int seq_len;	/* Original length of instruction sequence.  */
  uint32_t insn;
  Elf_Internal_Rela *hi_irelfn = NULL, *irelend;
  bfd_vma access_addr = 0;
  bfd_vma range_l = 0, range_h = 0;	/* Upper/lower bound.  */
  struct elf_link_hash_entry *h = NULL;
  int indx;
  enum elf_nds32_reloc_type checked_types[] =
    { R_NDS32_HI20_RELA, R_NDS32_GOT_HI20,
      R_NDS32_GOTPC_HI20, R_NDS32_GOTOFF_HI20,
      R_NDS32_PLTREL_HI20, R_NDS32_PLT_GOTREL_HI20,
      R_NDS32_TLS_LE_HI20
    };

  irelend = internal_relocs + sec->reloc_count;
  seq_len = GET_SEQ_LEN (irel->r_addend);
  laddr = irel->r_offset;
  *insn_len = seq_len;

  /* Get the high part relocation.  */
  for (i = 0; i < ARRAY_SIZE (checked_types); i++)
    {
      hi_irelfn = find_relocs_at_address_addr (irel, internal_relocs, irelend,
					       checked_types[i], laddr);
      if (hi_irelfn != irelend)
	break;
    }

  if (hi_irelfn == irelend)
    {
      /* Not R_NDS32_HI20_RELA.  */
      if (i != 0)
	_bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LOADSTORE",
			    (uint64_t) irel->r_offset);
      return false;
    }

  range_type = GET_LOADSTORE_RANGE (irel->r_addend);
  nds32_elf_final_sda_base (sec->output_section->owner,
			    link_info, &local_sda, false);

  switch (ELF32_R_TYPE (hi_irelfn->r_info))
    {
    case R_NDS32_HI20_RELA:
      insn = bfd_getb32 (contents + laddr);
      access_addr =
	calculate_memory_address (abfd, hi_irelfn, isymbuf, symtab_hdr);

      if (ELF32_R_SYM (hi_irelfn->r_info) >= symtab_hdr->sh_info)
	{
	  indx = ELF32_R_SYM (hi_irelfn->r_info) - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	}

      /* Try movi.  */
      if (range_type == NDS32_LOADSTORE_IMM
	  && access_addr < CONSERVATIVE_20BIT
	  && (!h || (h && strcmp (h->root.root.string, FP_BASE_NAME) != 0)))
	{
	  eliminate_sethi = 1;
	  break;
	}

      if (h && strcmp (h->root.root.string, FP_BASE_NAME) == 0)
	{
	  eliminate_sethi = 1;
	  break;
	}
      else if (!nds32_elf_relax_guard (&access_addr, local_sda, sec, hi_irelfn,
				       NULL, false, table, isymbuf, symtab_hdr))
	return false;

      if (!load_store_relax)
	return false;

      /* Case for set gp register.  */
      if (N32_RT5 (insn) == REG_GP)
	return false;

      if (range_type == NDS32_LOADSTORE_FLOAT_S
	  || range_type == NDS32_LOADSTORE_FLOAT_D)
	{
	  range_l = sdata_range[0][0];
	  range_h = sdata_range[0][1];
	}
      else
	{
	  range_l = sdata_range[1][0];
	  range_h = sdata_range[1][1];
	}
      break;

    default:
      return false;
    }

  /* Delete sethi instruction.  */
  if (eliminate_sethi == 1
      || (local_sda <= access_addr && (access_addr - local_sda) < range_h)
      || (local_sda > access_addr && (local_sda - access_addr) <= range_l))
    {
      hi_irelfn->r_info =
	ELF32_R_INFO (ELF32_R_SYM (hi_irelfn->r_info), R_NDS32_NONE);
      irel->r_info =
	ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);
      *insn_len = 0;
      return true;
    }

  return false;
}

/* Relax LO12 relocation for nds32_elf_relax_section.  */

static void
nds32_elf_relax_lo12 (struct bfd_link_info *link_info, bfd *abfd,
		      asection *sec, Elf_Internal_Rela *irel,
		      Elf_Internal_Rela *internal_relocs, bfd_byte *contents,
		      Elf_Internal_Sym *isymbuf, Elf_Internal_Shdr *symtab_hdr,
		      struct elf_nds32_link_hash_table *table)
{
  uint32_t insn;
  bfd_vma local_sda, laddr;
  unsigned long reloc;
  bfd_vma access_addr;
  bfd_vma range_l = 0, range_h = 0;	/* Upper/lower bound.  */
  Elf_Internal_Rela *irelfn = NULL, *irelend;
  struct elf_link_hash_entry *h = NULL;
  int indx;

  /* For SDA base relative relaxation.  */
  nds32_elf_final_sda_base (sec->output_section->owner, link_info,
			    &local_sda, false);

  irelend = internal_relocs + sec->reloc_count;
  laddr = irel->r_offset;
  insn = bfd_getb32 (contents + laddr);

  if (!is_sda_access_insn (insn) && N32_OP6 (insn) != N32_OP6_ORI)
    return;

  access_addr = calculate_memory_address (abfd, irel, isymbuf, symtab_hdr);

  if (ELF32_R_SYM (irel->r_info) >= symtab_hdr->sh_info)
    {
      indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
      h = elf_sym_hashes (abfd)[indx];
    }

  /* Try movi.  */
  if (N32_OP6 (insn) == N32_OP6_ORI && access_addr < CONSERVATIVE_20BIT
      && (!h || (h && strcmp (h->root.root.string, FP_BASE_NAME) != 0)))
    {
      reloc = R_NDS32_20_RELA;
      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), reloc);
      insn = N32_TYPE1 (MOVI, N32_RT5 (insn), 0);
      bfd_putb32 (insn, contents + laddr);
    }
  else
    {
      if (h && strcmp (h->root.root.string, FP_BASE_NAME) == 0)
	{
	  /* Fall through.  */
	}
      else if (!nds32_elf_relax_guard (&access_addr, local_sda, sec, irel, NULL,
				       false, table, isymbuf, symtab_hdr))
	return;

      range_l = sdata_range[1][0];
      range_h = sdata_range[1][1];
      switch (ELF32_R_TYPE (irel->r_info))
	{
	case R_NDS32_LO12S0_RELA:
	  reloc = R_NDS32_SDA19S0_RELA;
	  break;
	case R_NDS32_LO12S1_RELA:
	  reloc = R_NDS32_SDA18S1_RELA;
	  break;
	case R_NDS32_LO12S2_RELA:
	  reloc = R_NDS32_SDA17S2_RELA;
	  break;
	case R_NDS32_LO12S2_DP_RELA:
	  range_l = sdata_range[0][0];
	  range_h = sdata_range[0][1];
	  reloc = R_NDS32_SDA12S2_DP_RELA;
	  break;
	case R_NDS32_LO12S2_SP_RELA:
	  range_l = sdata_range[0][0];
	  range_h = sdata_range[0][1];
	  reloc = R_NDS32_SDA12S2_SP_RELA;
	  break;
	default:
	  return;
	}

      /* There are range_h and range_l because linker has to promise
	 all sections move cross one page together.  */
      if ((local_sda <= access_addr && (access_addr - local_sda) < range_h)
	  || (local_sda > access_addr && (local_sda - access_addr) <= range_l)
	  || (h && strcmp (h->root.root.string, FP_BASE_NAME) == 0))
	{
	  if (N32_OP6 (insn) == N32_OP6_ORI && N32_RT5 (insn) == REG_GP)
	    {
	      /* Maybe we should add R_NDS32_INSN16 reloc type here
		 or manually do some optimization.  sethi can't be
		 eliminated when updating $gp so the relative ori
		 needs to be preserved.  */
	      return;
	    }
	  if (!turn_insn_to_sda_access (insn, ELF32_R_TYPE (irel->r_info),
					&insn))
	    return;
	  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), reloc);
	  bfd_putb32 (insn, contents + laddr);

	  irelfn = find_relocs_at_address (irel, internal_relocs, irelend,
					   R_NDS32_INSN16);
	  /* SDA17 must keep INSN16 for converting fp_as_gp.  */
	  if (irelfn != irelend && reloc != R_NDS32_SDA17S2_RELA)
	    irelfn->r_info =
	      ELF32_R_INFO (ELF32_R_SYM (irelfn->r_info), R_NDS32_NONE);

	}
    }
  return;
}

/* Relax PTR relocation for nds32_elf_relax_section.  */

static bool
nds32_elf_relax_ptr (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
		     Elf_Internal_Rela *internal_relocs, int *insn_len,
		     int *seq_len, bfd_byte *contents)
{
  Elf_Internal_Rela *ptr_irel, *irelend, *count_irel, *re_irel;

  irelend = internal_relocs + sec->reloc_count;

  re_irel =
    find_relocs_at_address_addr (irel, internal_relocs, irelend,
				 R_NDS32_PTR_RESOLVED, irel->r_addend);

  if (re_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_PTR",
			  (uint64_t) irel->r_offset);
      return false;
    }

  if (re_irel->r_addend != 1)
    return false;

  /* Pointed target is relaxed and no longer needs this void *,
     change the type to NONE.  */
  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);

  /* Find PTR_COUNT to decide remove it or not.  If PTR_COUNT does
     not exist, it means only count 1 and remove it directly.  */
  /* TODO: I hope we can obsolate R_NDS32_COUNT in the future.  */
  count_irel = find_relocs_at_address (irel, internal_relocs, irelend,
				       R_NDS32_PTR_COUNT);
  ptr_irel = find_relocs_at_address (irel, internal_relocs, irelend,
				     R_NDS32_PTR);
  if (count_irel != irelend)
    {
      if (--count_irel->r_addend > 0)
	return false;
    }

  if (ptr_irel != irelend)
    return false;

  /* If the PTR_COUNT is already 0, remove current instruction.  */
  *seq_len = nds32_elf_insn_size (abfd, contents, irel->r_offset);
  *insn_len = 0;
  return true;
}

/* Relax LWC relocation for nds32_elf_relax_section.  */

static void
nds32_elf_relax_flsi (struct bfd_link_info *link_info, bfd *abfd,
		      asection *sec, Elf_Internal_Rela *irel,
		      Elf_Internal_Rela *internal_relocs,
		      bfd_byte *contents, Elf_Internal_Sym *isymbuf,
		      Elf_Internal_Shdr *symtab_hdr, bool *again)
{
  /* Pattern:
     sethi    ra, hi20(symbol)      ; HI20/LOADSTORE
     ori      ra, ra, lo12(symbol)  ; LO12S0/PTR/PTR/.../INSN16
     flsi     fsa, [ra + offset1]   ; LSI/PTR_RESOLVED/INSN16
     flsi     fsb, [ra + offset2]   ; LSI/PTR_RESOLVED/INSN16
     ...  */

  uint32_t insn;
  bfd_vma local_sda, laddr;
  unsigned long reloc;
  bfd_vma access_addr, flsi_offset;
  bfd_vma range_l = 0, range_h = 0;	/* Upper/lower bound.  */
  Elf_Internal_Rela *irelend, *re_irel;
  unsigned int opcode;

  irelend = internal_relocs + sec->reloc_count;
  laddr = irel->r_offset;
  insn = bfd_getb32 (contents + laddr);

  if ((insn & 0x80000000) || !is_sda_access_insn (insn))
    return;

  /* Can not do relaxation for bi format.  */
  if ((insn & 0x1000))
    return;

  /* Only deal with flsi, fssi, fldi, fsdi, so far.  */
  opcode = N32_OP6 (insn);
  if ((opcode == N32_OP6_LWC) || (opcode == N32_OP6_SWC))
    reloc = R_NDS32_SDA12S2_SP_RELA;
  else if ((opcode == N32_OP6_LDC) || (opcode == N32_OP6_SDC))
    reloc = R_NDS32_SDA12S2_DP_RELA;
  else
    return;

  re_irel = find_relocs_at_address (irel, internal_relocs, irelend,
				    R_NDS32_PTR_RESOLVED);
  if (re_irel == irelend)
    {
      _bfd_error_handler (unrecognized_reloc_msg, abfd, "R_NDS32_LSI",
			  (uint64_t) irel->r_offset);
      return;
    }

  /* For SDA base relative relaxation.  */
  nds32_elf_final_sda_base (sec->output_section->owner, link_info,
			    &local_sda, false);
  access_addr = calculate_memory_address (abfd, irel, isymbuf, symtab_hdr);
  flsi_offset = (insn & 0xfff) << 2;
  access_addr += flsi_offset;
  range_l = sdata_range[0][0];
  range_h = sdata_range[0][1];

  if ((local_sda <= access_addr && (access_addr - local_sda) < range_h)
      || (local_sda > access_addr && (local_sda - access_addr) <= range_l))
    {
      /* Turn flsi instruction into sda access format.  */
      insn = (insn & 0x7ff07000) | (REG_GP << 15);

      /* Add relocation type to flsi.  */
      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), reloc);
      irel->r_addend += flsi_offset;
      bfd_putb32 (insn, contents + re_irel->r_offset);

      re_irel->r_addend |= 1;
      *again = true;
    }
}

static bool
nds32_relax_adjust_label (bfd *abfd, asection *sec,
			  Elf_Internal_Rela *internal_relocs,
			  bfd_byte *contents,
			  nds32_elf_blank_t **relax_blank_list,
			  int optimize, int opt_size)
{
  /* This code block is used to adjust 4-byte alignment by relax a pair
     of instruction a time.

     It recognizes three types of relocations.
     1. R_NDS32_LABEL - a alignment.
     2. R_NDS32_INSN16 - relax a 32-bit instruction to 16-bit.
     3. is_16bit_NOP () - remove a 16-bit instruction.  */

  /* TODO: It seems currently implementation only support 4-byte alignment.
     We should handle any-alignment.  */

  Elf_Internal_Rela *insn_rel = NULL, *label_rel = NULL, *irel;
  Elf_Internal_Rela *tmp_rel, *tmp2_rel = NULL;
  Elf_Internal_Rela rel_temp;
  Elf_Internal_Rela *irelend;
  bfd_vma address;
  uint16_t insn16;

  /* Checking for branch relaxation relies on the relocations to
     be sorted on 'r_offset'.  This is not guaranteed so we must sort.  */
  nds32_insertion_sort (internal_relocs, sec->reloc_count,
			sizeof (Elf_Internal_Rela), compar_reloc);

  irelend = internal_relocs + sec->reloc_count;

  /* Force R_NDS32_LABEL before R_NDS32_INSN16.  */
  /* FIXME: Can we generate the right order in assembler?
     So we don't have to swapping them here.  */

  for (label_rel = internal_relocs, insn_rel = internal_relocs;
       label_rel < irelend; label_rel++)
    {
      if (ELF32_R_TYPE (label_rel->r_info) != R_NDS32_LABEL)
	continue;

      /* Find the first reloc has the same offset with label_rel.  */
      while (insn_rel < irelend && insn_rel->r_offset < label_rel->r_offset)
	insn_rel++;

      for (;insn_rel < irelend && insn_rel->r_offset == label_rel->r_offset;
	   insn_rel++)
	/* Check if there were R_NDS32_INSN16 and R_NDS32_LABEL at the same
	   address.  */
	if (ELF32_R_TYPE (insn_rel->r_info) == R_NDS32_INSN16)
	  break;

      if (insn_rel < irelend && insn_rel->r_offset == label_rel->r_offset
	  && insn_rel < label_rel)
	{
	  /* Swap the two reloc if the R_NDS32_INSN16 is
	     before R_NDS32_LABEL.  */
	  memcpy (&rel_temp, insn_rel, sizeof (Elf_Internal_Rela));
	  memcpy (insn_rel, label_rel, sizeof (Elf_Internal_Rela));
	  memcpy (label_rel, &rel_temp, sizeof (Elf_Internal_Rela));
	}
    }

  label_rel = NULL;
  insn_rel = NULL;
  /* If there were a sequence of R_NDS32_LABEL end up with .align 2
     or higher, remove other R_NDS32_LABEL with lower alignment.
     If an R_NDS32_INSN16 in between R_NDS32_LABELs must be converted,
     then the R_NDS32_LABEL sequence is broke.  */
  for (tmp_rel = internal_relocs; tmp_rel < irelend; tmp_rel++)
    {
      if (ELF32_R_TYPE (tmp_rel->r_info) == R_NDS32_LABEL)
	{
	  if (label_rel == NULL)
	    {
	      if (tmp_rel->r_addend < 2)
		label_rel = tmp_rel;
	      continue;
	    }
	  else if (tmp_rel->r_addend > 1)
	    {
	      /* Remove all LABEL relocation from label_rel to tmp_rel
		 including relocations with same offset as tmp_rel.  */
	      for (tmp2_rel = label_rel; tmp2_rel < tmp_rel; tmp2_rel++)
		{
		  if (tmp2_rel->r_offset == tmp_rel->r_offset)
		    break;

		  if (ELF32_R_TYPE (tmp2_rel->r_info) == R_NDS32_LABEL
		      && tmp2_rel->r_addend < 2)
		    tmp2_rel->r_info =
		      ELF32_R_INFO (ELF32_R_SYM (tmp2_rel->r_info),
				    R_NDS32_NONE);
		}
	      label_rel = NULL;
	    }
	}
      else if (ELF32_R_TYPE (tmp_rel->r_info) == R_NDS32_INSN16 && label_rel)
	{
	  /* A new INSN16 which can be converted, so clear label_rel.  */
	  if (is_convert_32_to_16 (abfd, sec, tmp_rel, internal_relocs,
				   irelend, &insn16)
	      || is_16bit_NOP (abfd, sec, tmp_rel))
	    label_rel = NULL;
	}
    }

  label_rel = NULL;
  insn_rel = NULL;
  /* Optimized for speed and nothing has not been relaxed.
     It's time to align labels.
     We may convert a 16-bit instruction right before a label to
     32-bit, in order to align the label if necessary
     all reloc entries has been sorted by r_offset.  */
  for (irel = internal_relocs;
       irel < irelend && irel->r_offset < sec->size; irel++)
    {
      if (ELF32_R_TYPE (irel->r_info) != R_NDS32_INSN16
	  && ELF32_R_TYPE (irel->r_info) != R_NDS32_LABEL)
	continue;

      if (ELF32_R_TYPE (irel->r_info) == R_NDS32_INSN16)
	{
	  /* A new INSN16 found, resize the old one.  */
	  if (is_convert_32_to_16
	      (abfd, sec, irel, internal_relocs, irelend, &insn16)
	      || is_16bit_NOP (abfd, sec, irel))
	    {
	      if (insn_rel)
		{
		  /* Previous INSN16 reloc exists, reduce its
		     size to 16-bit.  */
		  if (is_convert_32_to_16 (abfd, sec, insn_rel, internal_relocs,
					   irelend, &insn16))
		    {
		      nds32_elf_write_16 (abfd, contents, insn_rel,
					  internal_relocs, irelend, insn16);

		      if (!insert_nds32_elf_blank_recalc_total
			  (relax_blank_list, insn_rel->r_offset + 2, 2))
			return false;
		    }
		  else if (is_16bit_NOP (abfd, sec, insn_rel))
		    {
		      if (!insert_nds32_elf_blank_recalc_total
			  (relax_blank_list, insn_rel->r_offset, 2))
			return false;
		    }
		  insn_rel->r_info =
		    ELF32_R_INFO (ELF32_R_SYM (insn_rel->r_info), R_NDS32_NONE);
		}
	      /* Save the new one for later use.  */
	      insn_rel = irel;
	    }
	  else
	    irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					 R_NDS32_NONE);
	}
      else if (ELF32_R_TYPE (irel->r_info) == R_NDS32_LABEL)
	{
	  /* Search for label.  */
	  int force_relax = 0;

	  /* Label on 16-bit instruction or optimization
	     needless, just reset this reloc.  */
	  insn16 = bfd_getb16 (contents + irel->r_offset);
	  if ((irel->r_addend & 0x1f) < 2 && (!optimize || (insn16 & 0x8000)))
	    {
	      irel->r_info =
		ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_NDS32_NONE);
	      continue;
	    }

	  address =
	    irel->r_offset - get_nds32_elf_blank_total (relax_blank_list,
							irel->r_offset, 1);

	  if (!insn_rel)
	    {
	      /* Check if there is case which can not be aligned.  */
	      if (irel->r_addend == 2 && address & 0x2)
		return false;
	      continue;
	    }

	  /* Try to align this label.  */

	  if ((irel->r_addend & 0x1f) < 2)
	    {
	      /* Check if there is a INSN16 at the same address.
		 Label_rel always seats before insn_rel after
		 our sort.  */

	      /* Search for INSN16 at LABEL location.  If INSN16 is at
		 same location and this LABEL alignment is lower than 2,
		 the INSN16 can be converted to 2-byte.  */
	      for (tmp_rel = irel;
		   tmp_rel < irelend && tmp_rel->r_offset == irel->r_offset;
		   tmp_rel++)
		{
		  if (ELF32_R_TYPE (tmp_rel->r_info) == R_NDS32_INSN16
		      && (is_convert_32_to_16
			  (abfd, sec, tmp_rel, internal_relocs,
			   irelend, &insn16)
			  || is_16bit_NOP (abfd, sec, tmp_rel)))
		    {
		      force_relax = 1;
		      break;
		    }
		}
	    }

	  if (force_relax || irel->r_addend == 1 || address & 0x2)
	    {
	      /* Label not aligned.  */
	      /* Previous reloc exists, reduce its size to 16-bit.  */
	      if (is_convert_32_to_16 (abfd, sec, insn_rel,
				       internal_relocs, irelend, &insn16))
		{
		  nds32_elf_write_16 (abfd, contents, insn_rel,
				      internal_relocs, irelend, insn16);

		  if (!insert_nds32_elf_blank_recalc_total
		      (relax_blank_list, insn_rel->r_offset + 2, 2))
		    return false;
		}
	      else if (is_16bit_NOP (abfd, sec, insn_rel))
		{
		  if (!insert_nds32_elf_blank_recalc_total
		      (relax_blank_list, insn_rel->r_offset, 2))
		    return false;
		}

	    }
	  /* INSN16 reloc is used.  */
	  insn_rel = NULL;
	}
    }

  address =
    sec->size - get_nds32_elf_blank_total (relax_blank_list, sec->size, 0);
  if (insn_rel && (address & 0x2 || opt_size))
    {
      if (is_convert_32_to_16 (abfd, sec, insn_rel, internal_relocs,
			       irelend, &insn16))
	{
	  nds32_elf_write_16 (abfd, contents, insn_rel, internal_relocs,
			      irelend, insn16);
	  if (!insert_nds32_elf_blank_recalc_total
	      (relax_blank_list, insn_rel->r_offset + 2, 2))
	    return false;
	  insn_rel->r_info = ELF32_R_INFO (ELF32_R_SYM (insn_rel->r_info),
					   R_NDS32_NONE);
	}
      else if (is_16bit_NOP (abfd, sec, insn_rel))
	{
	  if (!insert_nds32_elf_blank_recalc_total
	      (relax_blank_list, insn_rel->r_offset, 2))
	    return false;
	  insn_rel->r_info = ELF32_R_INFO (ELF32_R_SYM (insn_rel->r_info),
					   R_NDS32_NONE);
	}
    }
  insn_rel = NULL;
  return true;
}

static bool
nds32_elf_relax_section (bfd *abfd, asection *sec,
			 struct bfd_link_info *link_info, bool *again)
{
  nds32_elf_blank_t *relax_blank_list = NULL;
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs;
  Elf_Internal_Rela *irel;
  Elf_Internal_Rela *irelend;
  Elf_Internal_Sym *isymbuf = NULL;
  bfd_byte *contents = NULL;
  bool result = true;
  int optimize = 0;
  int opt_size = 0;
  uint32_t insn;
  uint16_t insn16;

  /* Target dependnet option.  */
  struct elf_nds32_link_hash_table *table;
  int load_store_relax;

  relax_blank_list = NULL;

  *again = false;

  /* Nothing to do for
   * relocatable link or
   * non-relocatable section or
   * non-code section or
   * empty content or
   * no reloc entry.  */
  if (bfd_link_relocatable (link_info)
      || (sec->flags & SEC_RELOC) == 0
      || (sec->flags & SEC_EXCLUDE) != 0
      || (sec->flags & SEC_CODE) == 0
      || sec->size == 0
      || sec->reloc_count == 0)
    return true;

  /* 09.12.11 Workaround.  */
  /*  We have to adjust align for R_NDS32_LABEL if needed.
      The adjust approach only can fix 2-byte align once.  */
  if (sec->alignment_power > 2)
    return true;

  /* Do TLS model conversion once at first.  */
  nds32_elf_unify_tls_model (abfd, sec, contents, link_info);

  /* The optimization type to do.  */

  table = nds32_elf_hash_table (link_info);

  /* Save the first section for abs symbol relaxation.
     This is used for checking gp relaxation in the
     nds32_elf_relax_loadstore and nds32_elf_relax_lo12.  */
  nds32_elf_relax_guard (NULL, 0, sec, NULL, again, true,
			 table, NULL, NULL);

  /* The begining of general relaxation.  */

  if (is_SDA_BASE_set == 0)
    {
      bfd_vma gp;
      is_SDA_BASE_set = 1;
      nds32_elf_final_sda_base (sec->output_section->owner, link_info,
				&gp, false);
      relax_range_measurement (abfd, link_info);
    }

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  /* Relocations MUST be kept in memory, because relaxation adjust them.  */
  internal_relocs = _bfd_elf_link_read_relocs (abfd, sec, NULL, NULL,
					       true /* keep_memory */);
  if (internal_relocs == NULL)
    goto error_return;

  irelend = internal_relocs + sec->reloc_count;
  irel = find_relocs_at_address (internal_relocs, internal_relocs,
				 irelend, R_NDS32_RELAX_ENTRY);

  if (irel == irelend)
    return true;

  if (ELF32_R_TYPE (irel->r_info) == R_NDS32_RELAX_ENTRY)
    {
      if (irel->r_addend & R_NDS32_RELAX_ENTRY_DISABLE_RELAX_FLAG)
	return true;

      if (irel->r_addend & R_NDS32_RELAX_ENTRY_OPTIMIZE_FLAG)
	optimize = 1;

      if (irel->r_addend & R_NDS32_RELAX_ENTRY_OPTIMIZE_FOR_SPACE_FLAG)
	opt_size = 1;
    }

  load_store_relax = table->load_store_relax;

  /* Get symbol table and section content.  */
  contents = NULL;
  if (!nds32_get_section_contents (abfd, sec, &contents, true)
      || !nds32_get_local_syms (abfd, sec, &isymbuf))
    goto error_return;

  /* Do relax loop only when finalize is not done.
     Take care of relaxable relocs except INSN16.  */
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      int seq_len;		/* Original length of instruction sequence.  */
      int insn_len = 0;		/* Final length of instruction sequence.  */
      bool removed;

      insn = 0;
      if (ELF32_R_TYPE (irel->r_info) == R_NDS32_LABEL
	  && (irel->r_addend & 0x1f) >= 2)
	optimize = 1;

      /* Relocation Types
	 R_NDS32_LONGCALL1	53
	 R_NDS32_LONGCALL2	54
	 R_NDS32_LONGCALL3	55
	 R_NDS32_LONGJUMP1	56
	 R_NDS32_LONGJUMP2	57
	 R_NDS32_LONGJUMP3	58
	 R_NDS32_LOADSTORE	59  */
      if (ELF32_R_TYPE (irel->r_info) >= R_NDS32_LONGCALL1
	  && ELF32_R_TYPE (irel->r_info) <= R_NDS32_LOADSTORE)
	seq_len = GET_SEQ_LEN (irel->r_addend);

      /* Relocation Types
	 R_NDS32_LONGCALL4	107
	 R_NDS32_LONGCALL5	108
	 R_NDS32_LONGCALL6	109
	 R_NDS32_LONGJUMP4	110
	 R_NDS32_LONGJUMP5	111
	 R_NDS32_LONGJUMP6	112
	 R_NDS32_LONGJUMP7	113  */
      else if (ELF32_R_TYPE (irel->r_info) >= R_NDS32_LONGCALL4
	       && ELF32_R_TYPE (irel->r_info) <= R_NDS32_LONGJUMP7)
	seq_len = 4;

	/* Relocation Types
	 R_NDS32_LO12S0_RELA		30
	 R_NDS32_LO12S1_RELA		29
	 R_NDS32_LO12S2_RELA		28
	 R_NDS32_LO12S2_SP_RELA		71
	 R_NDS32_LO12S2_DP_RELA		70
	 R_NDS32_GOT_LO12		46
	 R_NDS32_GOTOFF_LO12		50
	 R_NDS32_PLTREL_LO12		65
	 R_NDS32_PLT_GOTREL_LO12	67
	 R_NDS32_17IFC_PCREL_RELA	96
	 R_NDS32_GOT_SUFF		193
	 R_NDS32_GOTOFF_SUFF		194
	 R_NDS32_PLT_GOT_SUFF		195
	 R_NDS32_MULCALL_SUFF		196
	 R_NDS32_PTR			197  */
      else if ((ELF32_R_TYPE (irel->r_info) <= R_NDS32_LO12S0_RELA
		&& ELF32_R_TYPE (irel->r_info) >= R_NDS32_LO12S2_RELA)
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_LO12S2_SP_RELA
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_LO12S2_DP_RELA
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_GOT_LO12
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_GOTOFF_LO12
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_GOTPC_LO12
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_PLTREL_LO12
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_PLT_GOTREL_LO12
	       || (ELF32_R_TYPE (irel->r_info) >= R_NDS32_GOT_SUFF
		   && ELF32_R_TYPE (irel->r_info) <= R_NDS32_PTR)
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_17IFC_PCREL_RELA
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_TLS_LE_LO12
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_TLS_LE_ADD
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_TLS_LE_LS
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_LSI)
	seq_len = 0;
      else
	continue;

      insn_len = seq_len;
      removed = false;

      switch (ELF32_R_TYPE (irel->r_info))
	{
	case R_NDS32_LONGCALL1:
	  removed = nds32_elf_relax_longcall1 (abfd, sec, irel, internal_relocs,
					       &insn_len, contents, isymbuf,
					       symtab_hdr);
	  break;
	case R_NDS32_LONGCALL2:
	  removed = nds32_elf_relax_longcall2 (abfd, sec, irel, internal_relocs,
					       &insn_len, contents, isymbuf,
					       symtab_hdr);
	  break;
	case R_NDS32_LONGCALL3:
	  removed = nds32_elf_relax_longcall3 (abfd, sec, irel, internal_relocs,
					       &insn_len, contents, isymbuf,
					       symtab_hdr);
	  break;
	case R_NDS32_LONGJUMP1:
	  removed = nds32_elf_relax_longjump1 (abfd, sec, irel, internal_relocs,
					       &insn_len, contents, isymbuf,
					       symtab_hdr);
	  break;
	case R_NDS32_LONGJUMP2:
	  removed = nds32_elf_relax_longjump2 (abfd, sec, irel, internal_relocs,
					       &insn_len, contents, isymbuf,
					       symtab_hdr);
	  break;
	case R_NDS32_LONGJUMP3:
	  removed = nds32_elf_relax_longjump3 (abfd, sec, irel, internal_relocs,
					       &insn_len, contents, isymbuf,
					       symtab_hdr);
	  break;
	case R_NDS32_LONGCALL4:
	  removed = nds32_elf_relax_longcall4 (abfd, sec, irel, internal_relocs,
					       &insn_len, contents, isymbuf,
					       symtab_hdr);
	  break;
	case R_NDS32_LONGCALL5:
	  removed = nds32_elf_relax_longcall5 (abfd, sec, irel, internal_relocs,
					       &insn_len, contents, isymbuf,
					       symtab_hdr);
	  break;
	case R_NDS32_LONGCALL6:
	  removed = nds32_elf_relax_longcall6 (abfd, sec, irel, internal_relocs,
					       &insn_len, contents, isymbuf,
					       symtab_hdr);
	  break;
	case R_NDS32_LONGJUMP4:
	  removed = nds32_elf_relax_longjump4 (abfd, sec, irel, internal_relocs,
					       &insn_len, contents, isymbuf,
					       symtab_hdr);
	  break;
	case R_NDS32_LONGJUMP5:
	  removed = nds32_elf_relax_longjump5 (abfd, sec, irel, internal_relocs,
					       &insn_len, &seq_len, contents,
					       isymbuf, symtab_hdr);
	  break;
	case R_NDS32_LONGJUMP6:
	  removed = nds32_elf_relax_longjump6 (abfd, sec, irel, internal_relocs,
					       &insn_len, &seq_len, contents,
					       isymbuf, symtab_hdr);
	  break;
	case R_NDS32_LONGJUMP7:
	  removed = nds32_elf_relax_longjump7 (abfd, sec, irel, internal_relocs,
					       &insn_len, &seq_len, contents,
					       isymbuf, symtab_hdr);
	  break;
	case R_NDS32_LOADSTORE:
	  removed = nds32_elf_relax_loadstore (link_info, abfd, sec, irel,
					       internal_relocs, &insn_len,
					       contents, isymbuf, symtab_hdr,
					       load_store_relax, table);
	  break;
	case R_NDS32_LO12S0_RELA:
	case R_NDS32_LO12S1_RELA:
	case R_NDS32_LO12S2_RELA:
	case R_NDS32_LO12S2_DP_RELA:
	case R_NDS32_LO12S2_SP_RELA:
	  /* Relax for low part.  */
	  nds32_elf_relax_lo12 (link_info, abfd, sec, irel, internal_relocs,
				contents, isymbuf, symtab_hdr, table);

	  /* It is impossible to delete blank, so just continue.  */
	  continue;
	case R_NDS32_PTR:
	  removed = nds32_elf_relax_ptr (abfd, sec, irel, internal_relocs,
					 &insn_len, &seq_len, contents);
	  break;
	case R_NDS32_LSI:
	  nds32_elf_relax_flsi (link_info, abfd, sec, irel, internal_relocs,
				contents, isymbuf, symtab_hdr, again);
	  continue;
	case R_NDS32_GOT_LO12:
	case R_NDS32_GOTOFF_LO12:
	case R_NDS32_PLTREL_LO12:
	case R_NDS32_PLT_GOTREL_LO12:
	case R_NDS32_GOTPC_LO12:
	case R_NDS32_TLS_LE_LO12:
	case R_NDS32_TLS_LE_ADD:
	case R_NDS32_TLS_LE_LS:
	case R_NDS32_PLT_GOT_SUFF:
	case R_NDS32_GOT_SUFF:
	case R_NDS32_GOTOFF_SUFF:
	  continue;
	default:
	  continue;
	}

      if (removed && seq_len - insn_len > 0)
	{
	  if (!insert_nds32_elf_blank
	      (&relax_blank_list, irel->r_offset + insn_len,
	       seq_len - insn_len))
	    goto error_return;
	  *again = true;
	}
    }

  calc_nds32_blank_total (relax_blank_list);

  if (table->relax_fp_as_gp)
    {
      if (!nds32_relax_fp_as_gp (link_info, abfd, sec, internal_relocs,
				 irelend, isymbuf))
	goto error_return;

      if (!*again)
	{
	  if (!nds32_fag_remove_unused_fpbase (abfd, sec, internal_relocs,
					       irelend))
	    goto error_return;
	}
    }

  if (!*again)
    {
      if (!nds32_relax_adjust_label (abfd, sec, internal_relocs, contents,
				     &relax_blank_list, optimize, opt_size))
	goto error_return;
    }

  /* It doesn't matter optimize_for_space_no_align anymore.
       If object file is assembled with flag '-Os',
       the we don't adjust jump-destination on 4-byte boundary.  */

  if (relax_blank_list)
    {
      nds32_elf_relax_delete_blanks (abfd, sec, relax_blank_list);
      relax_blank_list = NULL;
    }

  if (!*again)
    {
      /* Closing the section, so we don't relax it anymore.  */
      bfd_vma sec_size_align;
      Elf_Internal_Rela *tmp_rel;

      /* Pad to alignment boundary.  Only handle current section alignment.  */
      sec_size_align = (sec->size + (~((-1U) << sec->alignment_power)))
		       & ((-1U) << sec->alignment_power);
      if ((sec_size_align - sec->size) & 0x2)
	{
	  insn16 = NDS32_NOP16;
	  bfd_putb16 (insn16, contents + sec->size);
	  sec->size += 2;
	}

      while (sec_size_align != sec->size)
	{
	  insn = NDS32_NOP32;
	  bfd_putb32 (insn, contents + sec->size);
	  sec->size += 4;
	}

      tmp_rel = find_relocs_at_address (internal_relocs, internal_relocs,
					irelend, R_NDS32_RELAX_ENTRY);
      if (tmp_rel != irelend)
	tmp_rel->r_addend |= R_NDS32_RELAX_ENTRY_DISABLE_RELAX_FLAG;

      clean_nds32_elf_blank ();
    }

 finish:
  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);

  if (elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);

  if (symtab_hdr->contents != (bfd_byte *) isymbuf)
    free (isymbuf);

  return result;

 error_return:
  result = false;
  goto finish;
}

static struct bfd_elf_special_section const nds32_elf_special_sections[] =
{
  {".sdata", 6, -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE},
  {".sbss", 5, -2, SHT_NOBITS, SHF_ALLOC + SHF_WRITE},
  {NULL, 0, 0, 0, 0}
};

static bool
nds32_elf_section_flags (const Elf_Internal_Shdr *hdr)
{
  const char *name = hdr->bfd_section->name;

  if (startswith (name, ".sbss")
      || startswith (name, ".sdata"))
    hdr->bfd_section->flags |= SEC_SMALL_DATA;

  return true;
}

static bool
nds32_elf_output_arch_syms (bfd *output_bfd ATTRIBUTE_UNUSED,
			    struct bfd_link_info *info,
			    void *finfo ATTRIBUTE_UNUSED,
			    int (*func) (void *, const char *,
					 Elf_Internal_Sym *,
					 asection *,
					 struct elf_link_hash_entry *)
			    ATTRIBUTE_UNUSED)
{
  FILE *sym_ld_script = NULL;
  struct elf_nds32_link_hash_table *table;

  table = nds32_elf_hash_table (info);
  sym_ld_script = table->sym_ld_script;

  if (check_start_export_sym)
    fprintf (sym_ld_script, "}\n");

  return true;
}

static enum elf_reloc_type_class
nds32_elf_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			    const asection *rel_sec ATTRIBUTE_UNUSED,
			    const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_NDS32_RELATIVE:
      return reloc_class_relative;
    case R_NDS32_JMP_SLOT:
      return reloc_class_plt;
    case R_NDS32_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Put target dependent option into info hash table.  */
void
bfd_elf32_nds32_set_target_option (struct bfd_link_info *link_info,
				   int relax_fp_as_gp,
				   int eliminate_gc_relocs,
				   FILE * sym_ld_script,
				   int hyper_relax,
				   int tls_desc_trampoline,
				   int load_store_relax)
{
  struct elf_nds32_link_hash_table *table;

  table = nds32_elf_hash_table (link_info);
  if (table == NULL)
    return;

  table->relax_fp_as_gp = relax_fp_as_gp;
  table->eliminate_gc_relocs = eliminate_gc_relocs;
  table->sym_ld_script = sym_ld_script;
  table->hyper_relax = hyper_relax;
  table->tls_desc_trampoline = tls_desc_trampoline;
  table ->load_store_relax = load_store_relax;
}


/* These functions and data-structures are used for fp-as-gp
   optimization.  */

#define FAG_THRESHOLD	3	/* At least 3 gp-access.  */
/* lwi37.fp covers 508 bytes, but there may be 32-byte padding between
   the read-only section and read-write section.  */
#define FAG_WINDOW	(508 - 32)

/* An nds32_fag represent a gp-relative access.
   We find best fp-base by using a sliding window
   to find a base address which can cover most gp-access.  */
struct nds32_fag
{
  struct nds32_fag *next;	/* NULL-teminated linked list.  */
  bfd_vma addr;			/* The address of this fag.  */
  Elf_Internal_Rela **relas;	/* The relocations associated with this fag.
				   It is used for applying FP7U2_FLAG.  */
  int count;			/* How many times this address is referred.
				   There should be exactly `count' relocations
				   in relas.  */
  int relas_capcity;		/* The buffer size of relas.
				   We use an array instead of linked-list,
				   and realloc is used to adjust buffer size.  */
};

static void
nds32_fag_init (struct nds32_fag *head)
{
  memset (head, 0, sizeof (struct nds32_fag));
}

static void
nds32_fag_verify (struct nds32_fag *head)
{
  struct nds32_fag *iter;
  struct nds32_fag *prev;

  prev = NULL;
  iter = head->next;
  while (iter)
    {
      if (prev && prev->addr >= iter->addr)
	puts ("Bug in fp-as-gp insertion.");
      prev = iter;
      iter = iter->next;
    }
}

/* Insert a fag in ascending order.
   If a fag of the same address already exists,
   they are chained by relas array.  */

static void
nds32_fag_insert (struct nds32_fag *head, bfd_vma addr,
		  Elf_Internal_Rela * rel)
{
  struct nds32_fag *iter;
  struct nds32_fag *new_fag;
  const int INIT_RELAS_CAP = 4;

  for (iter = head;
       iter->next && iter->next->addr <= addr;
       iter = iter->next)
    /* Find somewhere to insert.  */ ;

  /* `iter' will be equal to `head' if the list is empty.  */
  if (iter != head && iter->addr == addr)
    {
      /* The address exists in the list.
	 Insert `rel' into relocation list, relas.  */

      /* Check whether relas is big enough.  */
      if (iter->count >= iter->relas_capcity)
	{
	  iter->relas_capcity *= 2;
	  iter->relas = bfd_realloc
	    (iter->relas, iter->relas_capcity * sizeof (void *));
	}
      iter->relas[iter->count++] = rel;
      return;
    }

  /* This is a new address.  Create a fag node for it.  */
  new_fag = bfd_malloc (sizeof (struct nds32_fag));
  memset (new_fag, 0, sizeof (*new_fag));
  new_fag->addr = addr;
  new_fag->count = 1;
  new_fag->next = iter->next;
  new_fag->relas_capcity = INIT_RELAS_CAP;
  new_fag->relas = (Elf_Internal_Rela **)
    bfd_malloc (new_fag->relas_capcity * sizeof (void *));
  new_fag->relas[0] = rel;
  iter->next = new_fag;

  nds32_fag_verify (head);
}

static void
nds32_fag_free_list (struct nds32_fag *head)
{
  struct nds32_fag *iter;

  iter = head->next;
  while (iter)
    {
      struct nds32_fag *tmp = iter;
      iter = iter->next;
      free (tmp->relas);
      tmp->relas = NULL;
      free (tmp);
    }
}

/* Find the best fp-base address.
   The relocation associated with that address is returned,
   so we can track the symbol instead of a fixed address.

   When relaxation, the address of an datum may change,
   because a text section is shrinked, so the data section
   moves forward.  If the aligments of text and data section
   are different, their distance may change too.
   Therefore, tracking a fixed address is not appriate.  */

static int
nds32_fag_find_base (struct nds32_fag *head, struct nds32_fag **bestpp)
{
  struct nds32_fag *base;	/* First fag in the window.  */
  struct nds32_fag *last;	/* First fag outside the window.  */
  int accu = 0;			/* Usage accumulation.  */
  struct nds32_fag *best;	/* Best fag.  */
  int baccu = 0;		/* Best accumulation.  */

  /* Use first fag for initial, and find the last fag in the window.

     In each iteration, we could simply subtract previous fag
     and accumulate following fags which are inside the window,
     untill we each the end.  */

  if (head->next == NULL)
    {
      *bestpp = NULL;
      return 0;
    }

  /* Initialize base.  */
  base = head->next;
  best = base;
  for (last = base;
       last && last->addr < base->addr + FAG_WINDOW;
       last = last->next)
    accu += last->count;

  baccu = accu;

  /* Record the best base in each iteration.  */
  while (base->next)
    {
      accu -= base->count;
      base = base->next;
      /* Account fags in window.  */
      for (/* Nothing.  */;
	   last && last->addr < base->addr + FAG_WINDOW;
	   last = last->next)
	accu += last->count;

      /* A better fp-base?  */
      if (accu > baccu)
	{
	  best = base;
	  baccu = accu;
	}
    }

  if (bestpp)
    *bestpp = best;
  return baccu;
}

/* Apply R_NDS32_INSN16_FP7U2_FLAG on gp-relative accesses,
   so we can convert it fo fp-relative access later.
   `best_fag' is the best fp-base.  Only those inside the window
   of best_fag is applied the flag.  */

static bool
nds32_fag_mark_relax (struct bfd_link_info *link_info,
		      asection *sec, struct nds32_fag *best_fag,
		      Elf_Internal_Rela *internal_relocs,
		      Elf_Internal_Rela *irelend)
{
  struct nds32_fag *ifag;
  bfd_vma best_fpbase, gp;
  bfd *output_bfd;

  output_bfd = sec->output_section->owner;
  nds32_elf_final_sda_base (output_bfd, link_info, &gp, false);
  best_fpbase = best_fag->addr;

  if (best_fpbase > gp + sdata_range[1][1]
      || best_fpbase < gp - sdata_range[1][0])
    return false;

  /* Mark these inside the window R_NDS32_INSN16_FP7U2_FLAG flag,
     so we know they can be converted to lwi37.fp.   */
  for (ifag = best_fag;
       ifag && ifag->addr < best_fpbase + FAG_WINDOW; ifag = ifag->next)
    {
      int i;

      for (i = 0; i < ifag->count; i++)
	{
	  Elf_Internal_Rela *insn16_rel;
	  Elf_Internal_Rela *fag_rel;

	  fag_rel = ifag->relas[i];

	  /* Only if this is within the WINDOWS, FP7U2_FLAG
	     is applied.  */

	  insn16_rel = find_relocs_at_address
	    (fag_rel, internal_relocs, irelend, R_NDS32_INSN16);

	  if (insn16_rel != irelend)
	    insn16_rel->r_addend = R_NDS32_INSN16_FP7U2_FLAG;
	}
    }
  return true;
}

/* Reset INSN16 to clean fp as gp.  */

static void
nds32_fag_unmark_relax (struct nds32_fag *fag,
			Elf_Internal_Rela *internal_relocs,
			Elf_Internal_Rela *irelend)
{
  struct nds32_fag *ifag;
  int i;
  Elf_Internal_Rela *insn16_rel;
  Elf_Internal_Rela *fag_rel;

  for (ifag = fag; ifag; ifag = ifag->next)
    {
      for (i = 0; i < ifag->count; i++)
	{
	  fag_rel = ifag->relas[i];

	  /* Restore the INSN16 relocation.  */
	  insn16_rel = find_relocs_at_address
	    (fag_rel, internal_relocs, irelend, R_NDS32_INSN16);

	  if (insn16_rel != irelend)
	    insn16_rel->r_addend &= ~R_NDS32_INSN16_FP7U2_FLAG;
	}
    }
}

/* This is the main function of fp-as-gp optimization.
   It should be called by relax_section.  */

static bool
nds32_relax_fp_as_gp (struct bfd_link_info *link_info,
		      bfd *abfd, asection *sec,
		      Elf_Internal_Rela *internal_relocs,
		      Elf_Internal_Rela *irelend,
		      Elf_Internal_Sym *isymbuf)
{
  Elf_Internal_Rela *begin_rel = NULL;
  Elf_Internal_Rela *irel;
  struct nds32_fag fag_head;
  Elf_Internal_Shdr *symtab_hdr;
  bfd_byte *contents;
  bool ifc_inside = false;

  /* FIXME: Can we bfd_elf_link_read_relocs for the relocs?  */

  /* Per-function fp-base selection.
     1. Create a list for all the gp-relative access.
     2. Base on those gp-relative address,
	find a fp-base which can cover most access.
     3. Use the fp-base for fp-as-gp relaxation.

     NOTE: If fp-as-gp is not worth to do, (e.g., less than 3 times),
     we should
     1. delete the `la $fp, _FP_BASE_' instruction and
     2. not convert lwi.gp to lwi37.fp.

     To delete the _FP_BASE_ instruction, we simply apply
     R_NDS32_RELAX_REGION_NOT_OMIT_FP_FLAG flag in the r_addend to disable it.

     To suppress the conversion, we simply NOT to apply
     R_NDS32_INSN16_FP7U2_FLAG flag.  */

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  contents = NULL;
  if (!nds32_get_section_contents (abfd, sec, &contents, true)
      || !nds32_get_local_syms (abfd, sec, &isymbuf))
    return false;

  /* Check whether it is worth for fp-as-gp optimization,
     i.e., at least 3 gp-load.

     Set R_NDS32_RELAX_REGION_NOT_OMIT_FP_FLAG if we should NOT
     apply this optimization.  */

  for (irel = internal_relocs; irel < irelend; irel++)
    {
      /* We recognize R_NDS32_RELAX_REGION_BEGIN/_END for the region.
	 One we enter the begin of the region, we track all the LW/ST
	 instructions, so when we leave the region, we try to find
	 the best fp-base address for those LW/ST instructions.  */

      if (ELF32_R_TYPE (irel->r_info) == R_NDS32_RELAX_REGION_BEGIN
	  && (irel->r_addend & R_NDS32_RELAX_REGION_OMIT_FP_FLAG))
	{
	  /* Begin of the region.  */
	  if (begin_rel)
	    /* xgettext:c-format */
	    _bfd_error_handler (_("%pB: nested OMIT_FP in %pA"), abfd, sec);

	  begin_rel = irel;
	  nds32_fag_init (&fag_head);
	  ifc_inside = false;
	}
      else if (ELF32_R_TYPE (irel->r_info) == R_NDS32_RELAX_REGION_END
	       && (irel->r_addend & R_NDS32_RELAX_REGION_OMIT_FP_FLAG))
	{
	  int accu;
	  struct nds32_fag *best_fag, *tmp_fag;
	  int dist;

	  /* End of the region.
	     Check whether it is worth to do fp-as-gp.  */

	  if (begin_rel == NULL)
	    {
	      /* xgettext:c-format */
	      _bfd_error_handler (_("%pB: unmatched OMIT_FP in %pA"),
				  abfd, sec);
	      continue;
	    }

	  accu = nds32_fag_find_base (&fag_head, &best_fag);

	  /* Clean FP7U2_FLAG because they may set ever.  */
	  tmp_fag = fag_head.next;
	  nds32_fag_unmark_relax (tmp_fag, internal_relocs, irelend);

	  /* Check if it is worth, and FP_BASE is near enough to SDA_BASE.  */
	  if (accu < FAG_THRESHOLD
	      || !nds32_fag_mark_relax (link_info, sec, best_fag,
					internal_relocs, irelend))
	    {
	      /* Not worth to do fp-as-gp.  */
	      begin_rel->r_addend |= R_NDS32_RELAX_REGION_NOT_OMIT_FP_FLAG;
	      begin_rel->r_addend &= ~R_NDS32_RELAX_REGION_OMIT_FP_FLAG;
	      irel->r_addend |= R_NDS32_RELAX_REGION_NOT_OMIT_FP_FLAG;
	      irel->r_addend &= ~R_NDS32_RELAX_REGION_OMIT_FP_FLAG;
	      nds32_fag_free_list (&fag_head);
	      begin_rel = NULL;
	      continue;
	    }

	  /* R_SYM of R_NDS32_RELAX_REGION_BEGIN is not used by assembler,
	     so we use it to record the distance to the reloction of best
	     fp-base.  */
	  dist = best_fag->relas[0] - begin_rel;
	  BFD_ASSERT (dist > 0 && dist < 0xffffff);
	  /* Use high 16 bits of addend to record the _FP_BASE_ matched
	     relocation.  And get the base value when relocating.  */
	  begin_rel->r_addend &= (0x1 << 16) - 1;
	  begin_rel->r_addend |= dist << 16;

	  nds32_fag_free_list (&fag_head);
	  begin_rel = NULL;
	}

      if (begin_rel == NULL || ifc_inside)
	/* Skip if we are not in the region of fp-as-gp.  */
	continue;

      if (ELF32_R_TYPE (irel->r_info) == R_NDS32_SDA15S2_RELA
	  || ELF32_R_TYPE (irel->r_info) == R_NDS32_SDA17S2_RELA)
	{
	  bfd_vma addr;
	  uint32_t insn;

	  /* A gp-relative access is found.  Insert it to the fag-list.  */

	  /* Rt is necessary an RT3, so it can be converted to lwi37.fp.  */
	  insn = bfd_getb32 (contents + irel->r_offset);
	  if (!N32_IS_RT3 (insn))
	    continue;

	  addr = calculate_memory_address (abfd, irel, isymbuf, symtab_hdr);
	  nds32_fag_insert (&fag_head, addr, irel);
	}
      else if (ELF32_R_TYPE (irel->r_info) == R_NDS32_SDA_FP7U2_RELA)
	{
	  begin_rel = NULL;
	}
      else if (ELF32_R_TYPE (irel->r_info) == R_NDS32_17IFC_PCREL_RELA
	       || ELF32_R_TYPE (irel->r_info) == R_NDS32_10IFCU_PCREL_RELA)
	{
	  /* Suppress fp as gp when encounter ifc.  */
	  ifc_inside = true;
	}
    }

  return true;
}

/* Remove unused `la $fp, _FD_BASE_' instruction.  */

static bool
nds32_fag_remove_unused_fpbase (bfd *abfd, asection *sec,
				Elf_Internal_Rela *internal_relocs,
				Elf_Internal_Rela *irelend)
{
  Elf_Internal_Rela *irel;
  Elf_Internal_Shdr *symtab_hdr;
  bfd_byte *contents = NULL;
  nds32_elf_blank_t *relax_blank_list = NULL;
  bool result = true;
  bool unused_region = false;

  /*
     NOTE: Disable fp-as-gp if we encounter ifcall relocations:
       R_NDS32_17IFC_PCREL_RELA
       R_NDS32_10IFCU_PCREL_RELA.  */

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  nds32_get_section_contents (abfd, sec, &contents, true);

  for (irel = internal_relocs; irel < irelend; irel++)
    {
      /* To remove unused fp-base, we simply find the REGION_NOT_OMIT_FP
	 we marked to in previous pass.
	 DO NOT scan relocations again, since we've alreadly decided it
	 and set the flag.  */
      const char *syname;
      int syndx;
      uint32_t insn;

      if (ELF32_R_TYPE (irel->r_info) == R_NDS32_RELAX_REGION_BEGIN
	  && (irel->r_addend & R_NDS32_RELAX_REGION_NOT_OMIT_FP_FLAG))
	unused_region = true;
      else if (ELF32_R_TYPE (irel->r_info) == R_NDS32_RELAX_REGION_END
	       && (irel->r_addend & R_NDS32_RELAX_REGION_NOT_OMIT_FP_FLAG))
	unused_region = false;

      /* We're not in the region.  */
      if (!unused_region)
	continue;

      /* _FP_BASE_ must be a GLOBAL symbol.  */
      syndx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
      if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
	continue;

      /* The symbol name must be _FP_BASE_.  */
      syname = elf_sym_hashes (abfd)[syndx]->root.root.string;
      if (strcmp (syname, FP_BASE_NAME) != 0)
	continue;

      if (ELF32_R_TYPE (irel->r_info) == R_NDS32_SDA19S0_RELA)
	{
	  /* addi.gp  $fp, -256  */
	  insn = bfd_getb32 (contents + irel->r_offset);
	  if (insn != INSN_ADDIGP_TO_FP)
	    continue;
	}
      else if (ELF32_R_TYPE (irel->r_info) == R_NDS32_SDA15S0_RELA)
	{
	  /* addi  $fp, $gp, -256  */
	  insn = bfd_getb32 (contents + irel->r_offset);
	  if (insn != INSN_ADDI_GP_TO_FP)
	    continue;
	}
      else if (ELF32_R_TYPE (irel->r_info) == R_NDS32_20_RELA)
	{
	  /* movi  $fp, FP_BASE  */
	  insn = bfd_getb32 (contents + irel->r_offset);
	  if (insn != INSN_MOVI_TO_FP)
	    continue;
	}
      else
	continue;

      /* We got here because a FP_BASE instruction is found.  */
      if (!insert_nds32_elf_blank_recalc_total
	  (&relax_blank_list, irel->r_offset, 4))
	goto error_return;
    }

 finish:
  if (relax_blank_list)
    {
      nds32_elf_relax_delete_blanks (abfd, sec, relax_blank_list);
      relax_blank_list = NULL;
    }
  return result;

 error_return:
  result = false;
  goto finish;
}

/* This is a version of bfd_generic_get_relocated_section_contents.
   We need this variety because relaxation will modify the dwarf
   infomation.  When there is undefined symbol reference error mesage,
   linker need to dump line number where the symbol be used.  However
   the address is be relaxed, it can not get the original dwarf contents.
   The variety only modify function call for reading in the section.  */

static bfd_byte *
nds32_elf_get_relocated_section_contents (bfd *abfd,
					  struct bfd_link_info *link_info,
					  struct bfd_link_order *link_order,
					  bfd_byte *data,
					  bool relocatable,
					  asymbol **symbols)
{
  bfd *input_bfd = link_order->u.indirect.section->owner;
  asection *input_section = link_order->u.indirect.section;
  long reloc_size;
  arelent **reloc_vector;
  long reloc_count;

  reloc_size = bfd_get_reloc_upper_bound (input_bfd, input_section);
  if (reloc_size < 0)
    return NULL;

  /* Read in the section.  */
  bfd_byte *orig_data = data;
  if (!nds32_get_section_contents (input_bfd, input_section, &data, false))
    return NULL;

  if (reloc_size == 0)
    return data;

  reloc_vector = (arelent **) bfd_malloc (reloc_size);
  if (reloc_vector == NULL)
    goto error_return;

  reloc_count = bfd_canonicalize_reloc (input_bfd, input_section,
					reloc_vector, symbols);
  if (reloc_count < 0)
    goto error_return;

  if (reloc_count > 0)
    {
      arelent **parent;
      for (parent = reloc_vector; *parent != NULL; parent++)
	{
	  char *error_message = NULL;
	  asymbol *symbol;
	  bfd_reloc_status_type r;

	  symbol = *(*parent)->sym_ptr_ptr;
	  if (symbol->section && discarded_section (symbol->section))
	    {
	      bfd_vma off;
	      static reloc_howto_type none_howto
		= HOWTO (0, 0, 0, 0, false, 0, complain_overflow_dont, NULL,
			 "unused", false, 0, 0, false);

	      off = (*parent)->address * OCTETS_PER_BYTE (input_bfd,
							  input_section);
	      _bfd_clear_contents ((*parent)->howto, input_bfd,
				   input_section, data, off);
	      (*parent)->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	      (*parent)->addend = 0;
	      (*parent)->howto = &none_howto;
	      r = bfd_reloc_ok;
	    }
	  else
	    r = bfd_perform_relocation (input_bfd, *parent, data,
					input_section,
					relocatable ? abfd : NULL,
					&error_message);

	  if (relocatable)
	    {
	      asection *os = input_section->output_section;

	      /* A partial link, so keep the relocs.  */
	      os->orelocation[os->reloc_count] = *parent;
	      os->reloc_count++;
	    }

	  if (r != bfd_reloc_ok)
	    {
	      switch (r)
		{
		case bfd_reloc_undefined:
		  (*link_info->callbacks->undefined_symbol)
		    (link_info, bfd_asymbol_name (*(*parent)->sym_ptr_ptr),
		     input_bfd, input_section, (*parent)->address, true);
		  break;
		case bfd_reloc_dangerous:
		  BFD_ASSERT (error_message != NULL);
		  (*link_info->callbacks->reloc_dangerous)
		    (link_info, error_message,
		     input_bfd, input_section, (*parent)->address);
		  break;
		case bfd_reloc_overflow:
		  (*link_info->callbacks->reloc_overflow)
		    (link_info, NULL,
		     bfd_asymbol_name (*(*parent)->sym_ptr_ptr),
		     (*parent)->howto->name, (*parent)->addend,
		     input_bfd, input_section, (*parent)->address);
		  break;
		case bfd_reloc_outofrange:
		  /* PR ld/13730:
		     This error can result when processing some partially
		     complete binaries.  Do not abort, but issue an error
		     message instead.  */
		  link_info->callbacks->einfo
		    /* xgettext:c-format */
		    (_("%X%P: %pB(%pA): relocation \"%pR\" goes out of range\n"),
		     abfd, input_section, * parent);
		  goto error_return;

		default:
		  abort ();
		  break;
		}
	    }
	}
    }

  free (reloc_vector);
  return data;

 error_return:
  free (reloc_vector);
  if (orig_data == NULL)
    free (data);
  return NULL;
}

/* Check target symbol.  */

static bool
nds32_elf_is_target_special_symbol (bfd *abfd ATTRIBUTE_UNUSED, asymbol *sym)
{
  if (!sym || !sym->name || sym->name[0] != '$')
    return false;
  return true;
}

/* nds32 find maybe function sym.  Ignore target special symbol
   first, and then go the general function.  */

static bfd_size_type
nds32_elf_maybe_function_sym (const asymbol *sym, asection *sec,
			      bfd_vma *code_off)
{
  if (nds32_elf_is_target_special_symbol (NULL, (asymbol *) sym))
    return 0;

  return _bfd_elf_maybe_function_sym (sym, sec, code_off);
}


/* Do TLS model conversion.  */

typedef struct relax_group_list_t
{
  Elf_Internal_Rela *relo;
  struct relax_group_list_t *next;
  struct relax_group_list_t *next_sibling;
  int id;
} relax_group_list_t;

int
list_insert (relax_group_list_t *pHead, Elf_Internal_Rela *pElem);

int
list_insert_sibling (relax_group_list_t *pNode, Elf_Internal_Rela *pElem);

void
dump_chain (relax_group_list_t *pHead);

int
list_insert (relax_group_list_t *pHead, Elf_Internal_Rela *pElem)
{
  relax_group_list_t *pNext = pHead;

  /* Find place.  */
  while (pNext->next)
    {
      if (pNext->next->id > (int) pElem->r_addend)
	break;

      pNext = pNext->next;
    }

  /* Insert node.  */
  relax_group_list_t *pNew = bfd_malloc (sizeof (relax_group_list_t));
  if (!pNew)
    return false;

  relax_group_list_t *tmp = pNext->next;
  pNext->next = pNew;

  pNew->id = pElem->r_addend;
  pNew->relo = pElem;
  pNew->next = tmp;
  pNew->next_sibling = NULL;

  return true;
}

int
list_insert_sibling (relax_group_list_t *pNode, Elf_Internal_Rela *pElem)
{
  relax_group_list_t *pNext = pNode;

  /* Find place.  */
  while (pNext->next_sibling)
    {
      pNext = pNext->next_sibling;
    }

  /* Insert node.  */
  relax_group_list_t *pNew = bfd_malloc (sizeof (relax_group_list_t));
  if (!pNew)
    return false;

  relax_group_list_t *tmp = pNext->next_sibling;
  pNext->next_sibling = pNew;

  pNew->id = -1;
  pNew->relo = pElem;
  pNew->next = NULL;
  pNew->next_sibling = tmp;

  return true;
}

void
dump_chain (relax_group_list_t *pHead)
{
  relax_group_list_t *pNext = pHead->next;
  while (pNext)
    {
      printf("group %d @ 0x%08x", pNext->id, (unsigned)pNext->relo->r_offset);
      relax_group_list_t *pNextSib = pNext->next_sibling;
      while (pNextSib)
	{
	  printf(", %d", (unsigned) ELF32_R_TYPE (pNextSib->relo->r_info));
	  pNextSib = pNextSib->next_sibling;
	}
      pNext = pNext->next;
      printf("\n");
    }
}

/* Check R_NDS32_RELAX_GROUP of each section.
   There might be multiple sections in one object file.  */

int
elf32_nds32_check_relax_group (bfd *abfd, asection *asec)
{
  elf32_nds32_relax_group_t *relax_group_ptr =
    elf32_nds32_relax_group_ptr (abfd);

  int min_id = relax_group_ptr->min_id;
  int max_id = relax_group_ptr->max_id;

  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  Elf_Internal_Rela *relocs;
  enum elf_nds32_reloc_type rtype;

  do
    {
      /* Relocations MUST be kept in memory, because relaxation adjust them.  */
      relocs = _bfd_elf_link_read_relocs (abfd, asec, NULL, NULL,
					  true /* keep_memory  */);
      if (relocs == NULL)
	break;

      /* Check R_NDS32_RELAX_GROUP.  */
      relend = relocs + asec->reloc_count;
      for (rel = relocs; rel < relend; rel++)
	{
	  int id;
	  rtype = ELF32_R_TYPE (rel->r_info);
	  if (rtype != R_NDS32_RELAX_GROUP)
	    continue;

	  id = rel->r_addend;
	  if (id < min_id)
	    min_id = id;
	  else if (id > max_id)
	    max_id = id;
	}
    }
  while (false);

  if (elf_section_data (asec)->relocs != relocs)
    free (relocs);

  if ((min_id != relax_group_ptr->min_id)
      || (max_id != relax_group_ptr->max_id))
    {
      relax_group_ptr->count = max_id - min_id + 1;
      BFD_ASSERT(min_id <= relax_group_ptr->min_id);
      relax_group_ptr->min_id = min_id;
      BFD_ASSERT(max_id >= relax_group_ptr->max_id);
      relax_group_ptr->max_id = max_id;
    }

  return relax_group_ptr->count;
}

/* Reorder RELAX_GROUP ID when command line option '-r' is applied.  */
static struct section_id_list_t *relax_group_section_id_list = NULL;

struct section_id_list_t *
elf32_nds32_lookup_section_id (int id, struct section_id_list_t **lst_ptr)
{
  struct section_id_list_t *result = NULL;
  struct section_id_list_t *lst = *lst_ptr;

  if (NULL == lst)
    {
      result = (struct section_id_list_t *) calloc
	(1, sizeof (struct section_id_list_t));
      BFD_ASSERT (result); /* Feed me.  */
      result->id = id;
      *lst_ptr = result;
    }
  else
    {
      struct section_id_list_t *cur = lst;
      struct section_id_list_t *prv = NULL;
      struct section_id_list_t *sec = NULL;

      while (cur)
	{
	  if (cur->id < id)
	    {
	      prv = cur;
	      cur = cur->next;
	      continue;
	    }

	  if (cur->id > id)
	    {
	      cur = NULL; /* To insert after prv.  */
	      sec = cur;  /* In case prv == NULL.  */
	    }

	  break;
	}

      if (NULL == cur)
	{
	  /* Insert after prv.  */
	  result = (struct section_id_list_t *) calloc
	    (1, sizeof (struct section_id_list_t));
	  BFD_ASSERT (result); /* Feed me.  */
	  result->id = id;
	  if (NULL != prv)
	    {
	      result->next = prv->next;
	      prv->next = result;
	    }
	  else
	    {
	      *lst_ptr = result;
	      result->next = sec;
	    }
	}
    }

  return result;
}

int
elf32_nds32_unify_relax_group (bfd *abfd, asection *asec)
{
  static int next_relax_group_bias = 0;

  elf32_nds32_relax_group_t *relax_group_ptr =
    elf32_nds32_relax_group_ptr (abfd);

  bool result = true;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  Elf_Internal_Rela *relocs = NULL;
  enum elf_nds32_reloc_type rtype;
  struct section_id_list_t *node = NULL;

  do
    {
      if (0 == relax_group_ptr->count)
	break;

      /* Check if this section has been handled.  */
      node = elf32_nds32_lookup_section_id (asec->id, &relax_group_section_id_list);
      if (NULL == node)
	break; /* Hit, the section id has handled.  */

      /* Relocations MUST be kept in memory, because relaxation adjust them.  */
      relocs = _bfd_elf_link_read_relocs (abfd, asec, NULL, NULL,
					  true /* keep_memory  */);
      if (relocs == NULL)
	{
	  BFD_ASSERT (0); /* feed me */
	  break;
	}

      /* Allocate group id bias for this bfd!  */
      if (0 == relax_group_ptr->init)
	{
	  relax_group_ptr->bias = next_relax_group_bias;
	  next_relax_group_bias += relax_group_ptr->count;
	  relax_group_ptr->init = 1;
	}

      /* Reorder relax group groups.  */
      relend = relocs + asec->reloc_count;
      for (rel = relocs; rel < relend; rel++)
	{
	  rtype = ELF32_R_TYPE(rel->r_info);
	  if (rtype != R_NDS32_RELAX_GROUP)
	    continue;

	  /* Change it.  */
	  rel->r_addend += relax_group_ptr->bias;
	}
    }
  while (false);

  if (elf_section_data (asec)->relocs != relocs)
    free (relocs);

  return result;
}

int
nds32_elf_unify_tls_model (bfd *inbfd, asection *insec, bfd_byte *incontents,
			   struct bfd_link_info *lnkinfo)
{
  bool result = true;
  Elf_Internal_Rela *irel;
  Elf_Internal_Rela *irelend;
  Elf_Internal_Rela *internal_relocs;
  unsigned long r_symndx;
  enum elf_nds32_reloc_type r_type;

  Elf_Internal_Sym *local_syms = NULL;
  bfd_byte *contents = NULL;

  relax_group_list_t chain = { .id = -1, .next = NULL, .next_sibling = NULL };

  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (inbfd)->symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  sym_hashes = elf_sym_hashes (inbfd);

  /* Reorder RELAX_GROUP when command line option '-r' is applied.  */
  if (bfd_link_relocatable (lnkinfo))
    {
      elf32_nds32_unify_relax_group (inbfd, insec);
      return result;
    }

  /* Relocations MUST be kept in memory, because relaxation adjust them.  */
  internal_relocs = _bfd_elf_link_read_relocs (inbfd, insec, NULL, NULL,
					       true /* keep_memory  */);
  if (internal_relocs == NULL)
    goto error_return;

  irelend = internal_relocs + insec->reloc_count;
  irel = find_relocs_at_address (internal_relocs, internal_relocs,
				 irelend, R_NDS32_RELAX_ENTRY);
  if (irel == irelend)
    goto finish;

  /* Chain/remove groups.  */
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      r_symndx = ELF32_R_SYM (irel->r_info);
      r_type = ELF32_R_TYPE (irel->r_info);
      if (r_type != R_NDS32_RELAX_GROUP)
	continue;

      /* Remove it.  */
      irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_NONE);
      /* Chain it now.  */
      if (!list_insert (&chain, irel))
	goto error_return;
    }

  /* Collect group relocations.  */
  /* Presume relocations are sorted.  */
  relax_group_list_t *pNext = chain.next;
  while (pNext)
    {
      for (irel = internal_relocs; irel < irelend; irel++)
	{
	  if (irel->r_offset == pNext->relo->r_offset)
	    {
	      /* Ignore Non-TLS relocation types.  */
	      r_type = ELF32_R_TYPE (irel->r_info);
	      if ((R_NDS32_TLS_LE_HI20 > r_type)
		  || (R_NDS32_RELAX_ENTRY == r_type))
		continue;

	      if (!list_insert_sibling (pNext, irel))
		goto error_return;
	    }
	  else if (irel->r_offset > pNext->relo->r_offset)
	    {
	      pNext = pNext->next;
	      if (!pNext)
		break;

	      bfd_vma current_offset = pNext->relo->r_offset;
	      if (irel->r_offset > current_offset)
		irel = internal_relocs; /* restart from head */
	      else
		--irel; /* Check current irel again.  */
	      continue;
	    }
	  else
	    {
	      /* This shouldn't be reached.  */
	    }
	}
      if (pNext)
	pNext = pNext->next;
    }

#ifdef DUBUG_VERBOSE
  dump_chain(&chain);
#endif

  /* Get symbol table and section content.  */
  if (incontents)
    contents = incontents;
  else if (!nds32_get_section_contents (inbfd, insec, &contents, true)
	   || !nds32_get_local_syms (inbfd, insec, &local_syms))
    goto error_return;

  char *local_got_tls_type = elf32_nds32_local_got_tls_type (inbfd);

  /* Convert TLS model each group if necessary.  */
  pNext = chain.next;

  int cur_grp_id = -1;
  int sethi_rt = -1;
  int add_rt = -1;
  enum elf_nds32_tls_type tls_type, org_tls_type, eff_tls_type;

  tls_type = org_tls_type = eff_tls_type = 0;

  while (pNext)
    {
      relax_group_list_t *pNextSig = pNext->next_sibling;
      while (pNextSig)
	{
	  struct elf_link_hash_entry *h = NULL;

	  irel = pNextSig->relo;
	  r_symndx = ELF32_R_SYM(irel->r_info);
	  r_type = ELF32_R_TYPE(irel->r_info);

	  if (pNext->id != cur_grp_id)
	    {
	      cur_grp_id = pNext->id;
	      org_tls_type = get_tls_type (r_type, NULL);
	      if (r_symndx >= symtab_hdr->sh_info)
		{
		  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
		  while (h->root.type == bfd_link_hash_indirect
			 || h->root.type == bfd_link_hash_warning)
		    h = (struct elf_link_hash_entry *) h->root.u.i.link;
		  tls_type = ((struct elf_nds32_link_hash_entry *) h)->tls_type;
		}
	      else
		{
		  tls_type = local_got_tls_type
		    ? local_got_tls_type[r_symndx]
		    : GOT_NORMAL;
		}

	      eff_tls_type = 1 << (fls (tls_type) - 1);
	      sethi_rt = N32_RT5(bfd_getb32 (contents + irel->r_offset));
	    }

	  if (eff_tls_type != org_tls_type)
	    {
	      switch (org_tls_type)
		{
		  /* DESC to IEGP/IE/LE.  */
		case GOT_TLS_DESC:
		  switch (eff_tls_type)
		    {
		    case GOT_TLS_IE:
		      switch (r_type)
			{
			case R_NDS32_TLS_DESC_HI20:
			  irel->r_info = ELF32_R_INFO(r_symndx,
						      R_NDS32_TLS_IE_HI20);
			  break;
			case R_NDS32_TLS_DESC_LO12:
			  irel->r_info = ELF32_R_INFO(r_symndx,
						      R_NDS32_TLS_IE_LO12);
			  break;
			case R_NDS32_TLS_DESC_ADD:
			  {
			    uint32_t insn = bfd_getb32 (contents + irel->r_offset);
			    add_rt = N32_RT5 (insn);
			    insn = N32_TYPE2 (LWI, add_rt, sethi_rt, 0);
			    bfd_putb32 (insn, contents + irel->r_offset);

			    irel->r_info = ELF32_R_INFO(r_symndx, R_NDS32_NONE);
			  }
			  break;
			case R_NDS32_TLS_DESC_FUNC:
			  bfd_putb32 (INSN_NOP, contents + irel->r_offset);
			  irel->r_info = ELF32_R_INFO(r_symndx,
						      R_NDS32_RELAX_REMOVE);
			  break;
			case R_NDS32_TLS_DESC_CALL:
			  {
			    uint32_t insn = N32_ALU1(ADD, REG_R0, add_rt,
						     REG_TP);
			    bfd_putb32 (insn, contents + irel->r_offset);

			    irel->r_info = ELF32_R_INFO(r_symndx, R_NDS32_NONE);
			  }
			  break;
			case R_NDS32_LOADSTORE:
			case R_NDS32_PTR:
			case R_NDS32_PTR_RESOLVED:
			case R_NDS32_NONE:
			case R_NDS32_LABEL:
			  break;
			default:
			  BFD_ASSERT(0);
			  break;
			}
		      break;
		    case GOT_TLS_IEGP:
		      switch (r_type)
			{
			case R_NDS32_TLS_DESC_HI20:
			  irel->r_info = ELF32_R_INFO(r_symndx,
						      R_NDS32_TLS_IEGP_HI20);
			  break;
			case R_NDS32_TLS_DESC_LO12:
			  irel->r_info = ELF32_R_INFO(r_symndx,
						      R_NDS32_TLS_IEGP_LO12);
			  break;
			case R_NDS32_TLS_DESC_ADD:
			  {
			    uint32_t insn = bfd_getb32 (contents + irel->r_offset);
			    add_rt = N32_RT5 (insn);
			    insn = N32_MEM(LW, add_rt, sethi_rt, REG_GP, 0);
			    bfd_putb32 (insn, contents + irel->r_offset);

			    irel->r_info = ELF32_R_INFO(r_symndx, R_NDS32_NONE);
			  }
			  break;
			case R_NDS32_TLS_DESC_FUNC:
			  bfd_putb32 (INSN_NOP, contents + irel->r_offset);
			  irel->r_info = ELF32_R_INFO(r_symndx,
						      R_NDS32_RELAX_REMOVE);
			  break;
			case R_NDS32_TLS_DESC_CALL:
			  {
			    uint32_t insn = N32_ALU1(ADD, REG_R0, add_rt,
						     REG_TP);
			    bfd_putb32 (insn, contents + irel->r_offset);

			    irel->r_info = ELF32_R_INFO(r_symndx, R_NDS32_NONE);
			  }
			  break;
			case R_NDS32_LOADSTORE:
			case R_NDS32_PTR:
			case R_NDS32_PTR_RESOLVED:
			case R_NDS32_NONE:
			case R_NDS32_LABEL:
			  break;
			default:
			  BFD_ASSERT(0);
			  break;
			}
		      break;
		    case GOT_TLS_LE:
		      switch (r_type)
			{
			case R_NDS32_TLS_DESC_HI20:
			  irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_TLS_LE_HI20);
			  break;
			case R_NDS32_TLS_DESC_LO12:
			  irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_TLS_LE_LO12);
			  break;
			case R_NDS32_TLS_DESC_ADD:
			  {
			    uint32_t insn = bfd_getb32 (contents + irel->r_offset);

			    add_rt = N32_RT5 (insn);
			    insn = N32_ALU1 (ADD, REG_R0, sethi_rt, REG_TP);
			    bfd_putb32 (insn, contents + irel->r_offset);

			    irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_TLS_LE_ADD);
			  }
			  break;
			case R_NDS32_TLS_DESC_FUNC:
			  bfd_putb32 (INSN_NOP, contents + irel->r_offset);
			  irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_RELAX_REMOVE);
			  break;
			case R_NDS32_TLS_DESC_CALL:
			  bfd_putb32 (INSN_NOP, contents + irel->r_offset);
			  irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_RELAX_REMOVE);
			  break;
			case R_NDS32_LOADSTORE:
			case R_NDS32_PTR:
			case R_NDS32_PTR_RESOLVED:
			case R_NDS32_NONE:
			case R_NDS32_LABEL:
			  break;
			default:
			  BFD_ASSERT(0);
			  break;
			}
		      break;
		    default:
		      break;
		    }
		  break;
		  /* IEGP to IE/LE.  */
		case GOT_TLS_IEGP:
		  switch (eff_tls_type)
		    {
		    case GOT_TLS_IE:
		      switch (r_type)
			{
			case R_NDS32_TLS_IEGP_HI20:
			  irel->r_info = ELF32_R_INFO(r_symndx,
						      R_NDS32_TLS_IE_HI20);
			  break;
			case R_NDS32_TLS_IEGP_LO12:
			  irel->r_info = ELF32_R_INFO(r_symndx,
						      R_NDS32_TLS_IE_LO12);
			  break;
			case R_NDS32_PTR_RESOLVED:
			  {
			    uint32_t insn = bfd_getb32 (contents + irel->r_offset);

			    add_rt = N32_RT5 (insn);
			    insn = N32_TYPE2 (LWI, add_rt, sethi_rt, 0);
			    bfd_putb32 (insn, contents + irel->r_offset);
			  }
			  break;
			case R_NDS32_TLS_IEGP_LW:
			  break;
			case R_NDS32_LOADSTORE:
			case R_NDS32_PTR:
			case R_NDS32_NONE:
			case R_NDS32_LABEL:
			  break;
			default:
			  BFD_ASSERT(0);
			  break;
			}
		      break;
		    case GOT_TLS_LE:
		      switch (r_type)
			{
			case R_NDS32_TLS_IEGP_HI20:
			  irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_TLS_LE_HI20);
			  break;
			case R_NDS32_TLS_IEGP_LO12:
			  irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_TLS_LE_LO12);
			  break;
			case R_NDS32_TLS_IEGP_LW:
			  bfd_putb32 (INSN_NOP, contents + irel->r_offset);
			  irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_RELAX_REMOVE);
			  break;
			case R_NDS32_LOADSTORE:
			case R_NDS32_PTR:
			case R_NDS32_NONE:
			case R_NDS32_LABEL:
			case R_NDS32_PTR_RESOLVED:
			  break;
			default:
			  BFD_ASSERT(0);
			  break;
			}
		      break;
		    default:
		      break;
		    }
		  break;
		  /* IE to LE. */
		case GOT_TLS_IE:
		  switch (eff_tls_type)
		    {
		    case GOT_TLS_LE:
		      switch (r_type)
			{
			case R_NDS32_TLS_IE_HI20:
			  irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_TLS_LE_HI20);
			  break;
			case R_NDS32_TLS_IE_LO12S2:
			  {
			    uint32_t insn = bfd_getb32 (contents + irel->r_offset);

			    add_rt = N32_RT5 (insn);
			    insn = N32_TYPE2 (ORI, add_rt, sethi_rt, 0);
			    bfd_putb32 (insn, contents + irel->r_offset);

			    irel->r_info = ELF32_R_INFO (r_symndx, R_NDS32_TLS_LE_LO12);
			  }
			  break;
			case R_NDS32_LOADSTORE:
			case R_NDS32_PTR:
			case R_NDS32_NONE:
			case R_NDS32_LABEL:
			  break;
			default:
			  BFD_ASSERT(0);
			  break;
			}
		      break;
		    default:
		      break;
		    }
		  break;
		default:
		  break;
		}
	    }
	  pNextSig = pNextSig->next_sibling;
	}

#if 1
      pNext = pNext->next;
#else
      while (pNext)
	{
	  if (pNext->id != cur_grp_id)
	    break;
	  pNext = pNext->next;
	}
#endif
    }

 finish:
  if (incontents)
    contents = NULL;

  if (elf_section_data (insec)->relocs != internal_relocs)
    free (internal_relocs);

  if (elf_section_data (insec)->this_hdr.contents != contents)
    free (contents);

  if (symtab_hdr->contents != (bfd_byte *) local_syms)
    free (local_syms);

  if (chain.next)
    {
      pNext = chain.next;
      relax_group_list_t *pDel;
      while (pNext)
	{
	  pDel = pNext;
	  pNext = pNext->next;
	  free (pDel);
	}
    }

  return result;

 error_return:
  result = false;
  goto finish;
}

/* End TLS model conversion.  */

#define ELF_ARCH				bfd_arch_nds32
#define ELF_MACHINE_CODE			EM_NDS32
#define ELF_MAXPAGESIZE				0x1000
#define ELF_TARGET_ID				NDS32_ELF_DATA

#define TARGET_BIG_SYM				nds32_elf32_be_vec
#define TARGET_BIG_NAME				"elf32-nds32be"
#define TARGET_LITTLE_SYM			nds32_elf32_le_vec
#define TARGET_LITTLE_NAME			"elf32-nds32le"

#define elf_info_to_howto			nds32_info_to_howto
#define elf_info_to_howto_rel			nds32_info_to_howto_rel

#define bfd_elf32_bfd_link_hash_table_create	nds32_elf_link_hash_table_create
#define bfd_elf32_bfd_merge_private_bfd_data	nds32_elf_merge_private_bfd_data
#define bfd_elf32_bfd_print_private_bfd_data	nds32_elf_print_private_bfd_data
#define bfd_elf32_bfd_relax_section		nds32_elf_relax_section
#define bfd_elf32_bfd_set_private_flags		nds32_elf_set_private_flags

#define bfd_elf32_mkobject			nds32_elf_mkobject
#define elf_backend_action_discarded		nds32_elf_action_discarded
#define elf_backend_add_symbol_hook		nds32_elf_add_symbol_hook
#define elf_backend_check_relocs		nds32_elf_check_relocs
#define elf_backend_adjust_dynamic_symbol	nds32_elf_adjust_dynamic_symbol
#define elf_backend_create_dynamic_sections	nds32_elf_create_dynamic_sections
#define elf_backend_finish_dynamic_sections	nds32_elf_finish_dynamic_sections
#define elf_backend_finish_dynamic_symbol	nds32_elf_finish_dynamic_symbol
#define elf_backend_size_dynamic_sections	nds32_elf_size_dynamic_sections
#define elf_backend_relocate_section		nds32_elf_relocate_section
#define elf_backend_gc_mark_hook		nds32_elf_gc_mark_hook
#define elf_backend_grok_prstatus		nds32_elf_grok_prstatus
#define elf_backend_grok_psinfo			nds32_elf_grok_psinfo
#define elf_backend_reloc_type_class		nds32_elf_reloc_type_class
#define elf_backend_copy_indirect_symbol	nds32_elf_copy_indirect_symbol
#define elf_backend_link_output_symbol_hook	nds32_elf_output_symbol_hook
#define elf_backend_output_arch_syms		nds32_elf_output_arch_syms
#define elf_backend_object_p			nds32_elf_object_p
#define elf_backend_final_write_processing	nds32_elf_final_write_processing
#define elf_backend_special_sections		nds32_elf_special_sections
#define elf_backend_section_flags		nds32_elf_section_flags
#define bfd_elf32_bfd_get_relocated_section_contents \
				nds32_elf_get_relocated_section_contents
#define bfd_elf32_bfd_is_target_special_symbol	nds32_elf_is_target_special_symbol
#define elf_backend_maybe_function_sym		nds32_elf_maybe_function_sym

#define elf_backend_can_gc_sections		1
#define elf_backend_can_refcount		1
#define elf_backend_want_got_plt		1
#define elf_backend_plt_readonly		1
#define elf_backend_want_plt_sym		0
#define elf_backend_got_header_size		12
#define elf_backend_may_use_rel_p		1
#define elf_backend_default_use_rela_p		1
#define elf_backend_may_use_rela_p		1
#define elf_backend_dtrel_excludes_plt		0

#include "elf32-target.h"

#undef ELF_MAXPAGESIZE
#define ELF_MAXPAGESIZE				0x2000

#undef  TARGET_BIG_SYM
#define TARGET_BIG_SYM				nds32_elf32_linux_be_vec
#undef  TARGET_BIG_NAME
#define TARGET_BIG_NAME				"elf32-nds32be-linux"
#undef  TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM			nds32_elf32_linux_le_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME			"elf32-nds32le-linux"
#undef  elf32_bed
#define elf32_bed				elf32_nds32_lin_bed

#include "elf32-target.h"
