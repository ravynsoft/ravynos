/* Renesas RX specific support for 32-bit ELF.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/rx.h"
#include "libiberty.h"
#include "elf32-rx.h"

#define RX_OPCODE_BIG_ENDIAN 0

/* This is a meta-target that's used only with objcopy, to avoid the
   endian-swap we would otherwise get.  We check for this in
   rx_elf_object_p().  */
const bfd_target rx_elf32_be_ns_vec;
const bfd_target rx_elf32_be_vec;

#ifdef DEBUG
char * rx_get_reloc (long);
void rx_dump_symtab (bfd *, void *, void *);
#endif

#define RXREL(n,sz,bit,shift,complain,pcrel)				     \
  HOWTO (R_RX_##n, shift, sz, bit, pcrel, 0, complain_overflow_ ## complain, \
	 bfd_elf_generic_reloc, "R_RX_" #n, false, 0, ~0, false)

/* Note that the relocations around 0x7f are internal to this file;
   feel free to move them as needed to avoid conflicts with published
   relocation numbers.  */

static reloc_howto_type rx_elf_howto_table [] =
{
  RXREL (NONE,	       0,  0, 0, dont,	   false),
  RXREL (DIR32,	       4, 32, 0, signed,   false),
  RXREL (DIR24S,       4, 24, 0, signed,   false),
  RXREL (DIR16,	       2, 16, 0, dont,	   false),
  RXREL (DIR16U,       2, 16, 0, unsigned, false),
  RXREL (DIR16S,       2, 16, 0, signed,   false),
  RXREL (DIR8,	       1,  8, 0, dont,	   false),
  RXREL (DIR8U,	       1,  8, 0, unsigned, false),
  RXREL (DIR8S,	       1,  8, 0, signed,   false),
  RXREL (DIR24S_PCREL, 4, 24, 0, signed,   true),
  RXREL (DIR16S_PCREL, 2, 16, 0, signed,   true),
  RXREL (DIR8S_PCREL,  1,  8, 0, signed,   true),
  RXREL (DIR16UL,      2, 16, 2, unsigned, false),
  RXREL (DIR16UW,      2, 16, 1, unsigned, false),
  RXREL (DIR8UL,       1,  8, 2, unsigned, false),
  RXREL (DIR8UW,       1,  8, 1, unsigned, false),
  RXREL (DIR32_REV,    2, 16, 0, dont,	   false),
  RXREL (DIR16_REV,    2, 16, 0, dont,	   false),
  RXREL (DIR3U_PCREL,  1,  3, 0, dont,	   true),

  EMPTY_HOWTO (0x13),
  EMPTY_HOWTO (0x14),
  EMPTY_HOWTO (0x15),
  EMPTY_HOWTO (0x16),
  EMPTY_HOWTO (0x17),
  EMPTY_HOWTO (0x18),
  EMPTY_HOWTO (0x19),
  EMPTY_HOWTO (0x1a),
  EMPTY_HOWTO (0x1b),
  EMPTY_HOWTO (0x1c),
  EMPTY_HOWTO (0x1d),
  EMPTY_HOWTO (0x1e),
  EMPTY_HOWTO (0x1f),

  RXREL (RH_3_PCREL, 1,	 3, 0, signed,	 true),
  RXREL (RH_16_OP,   2, 16, 0, signed,	 false),
  RXREL (RH_24_OP,   4, 24, 0, signed,	 false),
  RXREL (RH_32_OP,   4, 32, 0, signed,	 false),
  RXREL (RH_24_UNS,  4, 24, 0, unsigned, false),
  RXREL (RH_8_NEG,   1,	 8, 0, signed,	 false),
  RXREL (RH_16_NEG,  2, 16, 0, signed,	 false),
  RXREL (RH_24_NEG,  4, 24, 0, signed,	 false),
  RXREL (RH_32_NEG,  4, 32, 0, signed,	 false),
  RXREL (RH_DIFF,    4, 32, 0, signed,	 false),
  RXREL (RH_GPRELB,  2, 16, 0, unsigned, false),
  RXREL (RH_GPRELW,  2, 16, 0, unsigned, false),
  RXREL (RH_GPRELL,  2, 16, 0, unsigned, false),
  RXREL (RH_RELAX,   0,	 0, 0, dont,	 false),

  EMPTY_HOWTO (0x2e),
  EMPTY_HOWTO (0x2f),
  EMPTY_HOWTO (0x30),
  EMPTY_HOWTO (0x31),
  EMPTY_HOWTO (0x32),
  EMPTY_HOWTO (0x33),
  EMPTY_HOWTO (0x34),
  EMPTY_HOWTO (0x35),
  EMPTY_HOWTO (0x36),
  EMPTY_HOWTO (0x37),
  EMPTY_HOWTO (0x38),
  EMPTY_HOWTO (0x39),
  EMPTY_HOWTO (0x3a),
  EMPTY_HOWTO (0x3b),
  EMPTY_HOWTO (0x3c),
  EMPTY_HOWTO (0x3d),
  EMPTY_HOWTO (0x3e),
  EMPTY_HOWTO (0x3f),
  EMPTY_HOWTO (0x40),

  RXREL (ABS32,	       4, 32, 0, dont,	   false),
  RXREL (ABS24S,       4, 24, 0, signed,   false),
  RXREL (ABS16,	       2, 16, 0, dont,	   false),
  RXREL (ABS16U,       2, 16, 0, unsigned, false),
  RXREL (ABS16S,       2, 16, 0, signed,   false),
  RXREL (ABS8,	       1,  8, 0, dont,	   false),
  RXREL (ABS8U,	       1,  8, 0, unsigned, false),
  RXREL (ABS8S,	       1,  8, 0, signed,   false),
  RXREL (ABS24S_PCREL, 4, 24, 0, signed,   true),
  RXREL (ABS16S_PCREL, 2, 16, 0, signed,   true),
  RXREL (ABS8S_PCREL,  1,  8, 0, signed,   true),
  RXREL (ABS16UL,      2, 16, 0, unsigned, false),
  RXREL (ABS16UW,      2, 16, 0, unsigned, false),
  RXREL (ABS8UL,       1,  8, 0, unsigned, false),
  RXREL (ABS8UW,       1,  8, 0, unsigned, false),
  RXREL (ABS32_REV,    4, 32, 0, dont,	   false),
  RXREL (ABS16_REV,    2, 16, 0, dont,	   false),

#define STACK_REL_P(x) ((x) <= R_RX_ABS16_REV && (x) >= R_RX_ABS32)

  EMPTY_HOWTO (0x52),
  EMPTY_HOWTO (0x53),
  EMPTY_HOWTO (0x54),
  EMPTY_HOWTO (0x55),
  EMPTY_HOWTO (0x56),
  EMPTY_HOWTO (0x57),
  EMPTY_HOWTO (0x58),
  EMPTY_HOWTO (0x59),
  EMPTY_HOWTO (0x5a),
  EMPTY_HOWTO (0x5b),
  EMPTY_HOWTO (0x5c),
  EMPTY_HOWTO (0x5d),
  EMPTY_HOWTO (0x5e),
  EMPTY_HOWTO (0x5f),
  EMPTY_HOWTO (0x60),
  EMPTY_HOWTO (0x61),
  EMPTY_HOWTO (0x62),
  EMPTY_HOWTO (0x63),
  EMPTY_HOWTO (0x64),
  EMPTY_HOWTO (0x65),
  EMPTY_HOWTO (0x66),
  EMPTY_HOWTO (0x67),
  EMPTY_HOWTO (0x68),
  EMPTY_HOWTO (0x69),
  EMPTY_HOWTO (0x6a),
  EMPTY_HOWTO (0x6b),
  EMPTY_HOWTO (0x6c),
  EMPTY_HOWTO (0x6d),
  EMPTY_HOWTO (0x6e),
  EMPTY_HOWTO (0x6f),
  EMPTY_HOWTO (0x70),
  EMPTY_HOWTO (0x71),
  EMPTY_HOWTO (0x72),
  EMPTY_HOWTO (0x73),
  EMPTY_HOWTO (0x74),
  EMPTY_HOWTO (0x75),
  EMPTY_HOWTO (0x76),
  EMPTY_HOWTO (0x77),

  /* These are internal.  */
  /* A 5-bit unsigned displacement to a B/W/L address, at bit position 8/12.  */
  /* ---- ----   4--- 3210.  */
#define R_RX_RH_ABS5p8B 0x78
  RXREL (RH_ABS5p8B,   0,  0, 0, dont,	   false),
#define R_RX_RH_ABS5p8W 0x79
  RXREL (RH_ABS5p8W,   0,  0, 0, dont,	   false),
#define R_RX_RH_ABS5p8L 0x7a
  RXREL (RH_ABS5p8L,   0,  0, 0, dont,	   false),
  /* A 5-bit unsigned displacement to a B/W/L address, at bit position 5/12.  */
  /* ---- -432   1--- 0---.  */
#define R_RX_RH_ABS5p5B 0x7b
  RXREL (RH_ABS5p5B,   0,  0, 0, dont,	   false),
#define R_RX_RH_ABS5p5W 0x7c
  RXREL (RH_ABS5p5W,   0,  0, 0, dont,	   false),
#define R_RX_RH_ABS5p5L 0x7d
  RXREL (RH_ABS5p5L,   0,  0, 0, dont,	   false),
  /* A 4-bit unsigned immediate at bit position 8.  */
#define R_RX_RH_UIMM4p8 0x7e
  RXREL (RH_UIMM4p8,   0,  0, 0, dont,	   false),
  /* A 4-bit negative unsigned immediate at bit position 8.  */
#define R_RX_RH_UNEG4p8 0x7f
  RXREL (RH_UNEG4p8,   0,  0, 0, dont,	   false),
  /* End of internal relocs.  */

  RXREL (SYM,	    4, 32, 0, dont, false),
  RXREL (OPneg,	    4, 32, 0, dont, false),
  RXREL (OPadd,	    4, 32, 0, dont, false),
  RXREL (OPsub,	    4, 32, 0, dont, false),
  RXREL (OPmul,	    4, 32, 0, dont, false),
  RXREL (OPdiv,	    4, 32, 0, dont, false),
  RXREL (OPshla,    4, 32, 0, dont, false),
  RXREL (OPshra,    4, 32, 0, dont, false),
  RXREL (OPsctsize, 4, 32, 0, dont, false),

  EMPTY_HOWTO (0x89),
  EMPTY_HOWTO (0x8a),
  EMPTY_HOWTO (0x8b),
  EMPTY_HOWTO (0x8c),

  RXREL (OPscttop,  4, 32, 0, dont, false),

  EMPTY_HOWTO (0x8e),
  EMPTY_HOWTO (0x8f),

  RXREL (OPand,	    4, 32, 0, dont, false),
  RXREL (OPor,	    4, 32, 0, dont, false),
  RXREL (OPxor,	    4, 32, 0, dont, false),
  RXREL (OPnot,	    4, 32, 0, dont, false),
  RXREL (OPmod,	    4, 32, 0, dont, false),
  RXREL (OPromtop,  4, 32, 0, dont, false),
  RXREL (OPramtop,  4, 32, 0, dont, false)
};

/* Map BFD reloc types to RX ELF reloc types.  */

struct rx_reloc_map
{
  bfd_reloc_code_real_type  bfd_reloc_val;
  unsigned int		    rx_reloc_val;
};

static const struct rx_reloc_map rx_reloc_map [] =
{
  { BFD_RELOC_NONE,		R_RX_NONE },
  { BFD_RELOC_8,		R_RX_DIR8S },
  { BFD_RELOC_16,		R_RX_DIR16S },
  { BFD_RELOC_24,		R_RX_DIR24S },
  { BFD_RELOC_32,		R_RX_DIR32 },
  { BFD_RELOC_RX_16_OP,		R_RX_DIR16 },
  { BFD_RELOC_RX_DIR3U_PCREL,	R_RX_DIR3U_PCREL },
  { BFD_RELOC_8_PCREL,		R_RX_DIR8S_PCREL },
  { BFD_RELOC_16_PCREL,		R_RX_DIR16S_PCREL },
  { BFD_RELOC_24_PCREL,		R_RX_DIR24S_PCREL },
  { BFD_RELOC_RX_8U,		R_RX_DIR8U },
  { BFD_RELOC_RX_16U,		R_RX_DIR16U },
  { BFD_RELOC_RX_24U,		R_RX_RH_24_UNS },
  { BFD_RELOC_RX_NEG8,		R_RX_RH_8_NEG },
  { BFD_RELOC_RX_NEG16,		R_RX_RH_16_NEG },
  { BFD_RELOC_RX_NEG24,		R_RX_RH_24_NEG },
  { BFD_RELOC_RX_NEG32,		R_RX_RH_32_NEG },
  { BFD_RELOC_RX_DIFF,		R_RX_RH_DIFF },
  { BFD_RELOC_RX_GPRELB,	R_RX_RH_GPRELB },
  { BFD_RELOC_RX_GPRELW,	R_RX_RH_GPRELW },
  { BFD_RELOC_RX_GPRELL,	R_RX_RH_GPRELL },
  { BFD_RELOC_RX_RELAX,		R_RX_RH_RELAX },
  { BFD_RELOC_RX_SYM,		R_RX_SYM },
  { BFD_RELOC_RX_OP_SUBTRACT,	R_RX_OPsub },
  { BFD_RELOC_RX_OP_NEG,	R_RX_OPneg },
  { BFD_RELOC_RX_ABS8,		R_RX_ABS8 },
  { BFD_RELOC_RX_ABS16,		R_RX_ABS16 },
  { BFD_RELOC_RX_ABS16_REV,	R_RX_ABS16_REV },
  { BFD_RELOC_RX_ABS32,		R_RX_ABS32 },
  { BFD_RELOC_RX_ABS32_REV,	R_RX_ABS32_REV },
  { BFD_RELOC_RX_ABS16UL,	R_RX_ABS16UL },
  { BFD_RELOC_RX_ABS16UW,	R_RX_ABS16UW },
  { BFD_RELOC_RX_ABS16U,	R_RX_ABS16U }
};

#define BIGE(abfd)       ((abfd)->xvec->byteorder == BFD_ENDIAN_BIG)

static reloc_howto_type *
rx_reloc_type_lookup (bfd *		       abfd ATTRIBUTE_UNUSED,
		      bfd_reloc_code_real_type code)
{
  unsigned int i;

  if (code == BFD_RELOC_RX_32_OP)
    return rx_elf_howto_table + R_RX_DIR32;

  for (i = ARRAY_SIZE (rx_reloc_map); i--;)
    if (rx_reloc_map [i].bfd_reloc_val == code)
      return rx_elf_howto_table + rx_reloc_map[i].rx_reloc_val;

  return NULL;
}

static reloc_howto_type *
rx_reloc_name_lookup (bfd * abfd ATTRIBUTE_UNUSED, const char * r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (rx_elf_howto_table); i++)
    if (rx_elf_howto_table[i].name != NULL
	&& strcasecmp (rx_elf_howto_table[i].name, r_name) == 0)
      return rx_elf_howto_table + i;

  return NULL;
}

/* Set the howto pointer for an RX ELF reloc.  */

static bool
rx_info_to_howto_rela (bfd *		   abfd,
		       arelent *	   cache_ptr,
		       Elf_Internal_Rela * dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  BFD_ASSERT (R_RX_max == ARRAY_SIZE (rx_elf_howto_table));
  if (r_type >= ARRAY_SIZE (rx_elf_howto_table))
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = rx_elf_howto_table + r_type;
  if (cache_ptr->howto->name == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  return true;
}

static bfd_vma
get_symbol_value (const char *		  name,
		  struct bfd_link_info *  info,
		  bfd *			  input_bfd,
		  asection *		  input_section,
		  int			  offset)
{
  bfd_vma value = 0;
  struct bfd_link_hash_entry * h;

  h = bfd_link_hash_lookup (info->hash, name, false, false, true);

  if (h == NULL
      || (h->type != bfd_link_hash_defined
	  && h->type != bfd_link_hash_defweak))
    (*info->callbacks->undefined_symbol)
      (info, name, input_bfd, input_section, offset, true);
  else
    value = (h->u.def.value
	     + h->u.def.section->output_section->vma
	     + h->u.def.section->output_offset);

  return value;
}

static bfd_vma
get_symbol_value_maybe (const char *		name,
			struct bfd_link_info *  info)
{
  bfd_vma value = 0;
  struct bfd_link_hash_entry * h;

  h = bfd_link_hash_lookup (info->hash, name, false, false, true);

  if (h == NULL
      || (h->type != bfd_link_hash_defined
	  && h->type != bfd_link_hash_defweak))
    return 0;
  else
    value = (h->u.def.value
	     + h->u.def.section->output_section->vma
	     + h->u.def.section->output_offset);

  return value;
}

static bfd_vma
get_gp (struct bfd_link_info *	info,
	bfd *			abfd,
	asection *		sec,
	int			offset)
{
  static bool cached = false;
  static bfd_vma cached_value = 0;

  if (!cached)
    {
      cached_value = get_symbol_value ("__gp", info, abfd, sec, offset);
      cached = true;
    }
  return cached_value;
}

static bfd_vma
get_romstart (struct bfd_link_info *  info,
	      bfd *		      abfd,
	      asection *	      sec,
	      int		      offset)
{
  static bool cached = false;
  static bfd_vma cached_value = 0;

  if (!cached)
    {
      cached_value = get_symbol_value ("_start", info, abfd, sec, offset);
      cached = true;
    }
  return cached_value;
}

static bfd_vma
get_ramstart (struct bfd_link_info *  info,
	      bfd *		      abfd,
	      asection *	      sec,
	      int		      offset)
{
  static bool cached = false;
  static bfd_vma cached_value = 0;

  if (!cached)
    {
      cached_value = get_symbol_value ("__datastart", info, abfd, sec, offset);
      cached = true;
    }
  return cached_value;
}

#define NUM_STACK_ENTRIES 16
static int32_t rx_stack [ NUM_STACK_ENTRIES ];
static unsigned int rx_stack_top;

#define RX_STACK_PUSH(val)			\
  do						\
    {						\
      if (rx_stack_top < NUM_STACK_ENTRIES)	\
	rx_stack [rx_stack_top ++] = (val);	\
      else					\
	r = bfd_reloc_dangerous;		\
    }						\
  while (0)

#define RX_STACK_POP(dest)			\
  do						\
    {						\
      if (rx_stack_top > 0)			\
	(dest) = rx_stack [-- rx_stack_top];	\
      else					\
	(dest) = 0, r = bfd_reloc_dangerous;	\
    }						\
  while (0)

/* Relocate an RX ELF section.
   There is some attempt to make this function usable for many architectures,
   both USE_REL and USE_RELA ['twould be nice if such a critter existed],
   if only to serve as a learning tool.

   The RELOCATE_SECTION function is called by the new ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjusting the section contents as
   necessary, and (if using Rela relocs and generating a relocatable
   output file) adjusting the reloc addend as necessary.

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

static int
rx_elf_relocate_section
    (bfd *		     output_bfd,
     struct bfd_link_info *  info,
     bfd *		     input_bfd,
     asection *		     input_section,
     bfd_byte *		     contents,
     Elf_Internal_Rela *     relocs,
     Elf_Internal_Sym *	     local_syms,
     asection **	     local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  bool pid_mode;
  bool saw_subtract = false;
  const char *table_default_cache = NULL;
  bfd_vma table_start_cache = 0;
  bfd_vma table_end_cache = 0;

  if (elf_elfheader (output_bfd)->e_flags & E_FLAG_RX_PID)
    pid_mode = true;
  else
    pid_mode = false;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;
  for (rel = relocs; rel < relend; rel ++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char * name = NULL;
      bool unresolved_reloc = true;
      int r_type;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);

      howto  = rx_elf_howto_table + ELF32_R_TYPE (rel->r_info);
      h	     = NULL;
      sym    = NULL;
      sec    = NULL;
      relocation = 0;

      if (rx_stack_top == 0)
	saw_subtract = false;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections [r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, & sec, rel);

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = sym->st_name == 0 ? bfd_section_name (sec) : name;
	}
      else
	{
	  bool warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes, h,
				   sec, relocation, unresolved_reloc,
				   warned, ignored);

	  name = h->root.root.string;
	}

      if (startswith (name, "$tableentry$default$"))
	{
	  bfd_vma entry_vma;
	  int idx;
	  char *buf;

	  if (table_default_cache != name)
	    {

	      /* All relocs for a given table should be to the same
		 (weak) default symbol) so we can use it to detect a
		 cache miss.  We use the offset into the table to find
		 the "real" symbol.  Calculate and store the table's
		 offset here.  */

	      table_default_cache = name;

	      /* We have already done error checking in rx_table_find().  */

	      buf = (char *) bfd_malloc (13 + strlen (name + 20));
	      if (buf == NULL)
		return false;

	      sprintf (buf, "$tablestart$%s", name + 20);
	      table_start_cache = get_symbol_value (buf,
						    info,
						    input_bfd,
						    input_section,
						    rel->r_offset);

	      sprintf (buf, "$tableend$%s", name + 20);
	      table_end_cache = get_symbol_value (buf,
						  info,
						  input_bfd,
						  input_section,
						  rel->r_offset);

	      free (buf);
	    }

	  entry_vma = (input_section->output_section->vma
		       + input_section->output_offset
		       + rel->r_offset);

	  if (table_end_cache <= entry_vma || entry_vma < table_start_cache)
	    {
	      /* xgettext:c-format */
	      _bfd_error_handler (_("%pB:%pA: table entry %s outside table"),
				  input_bfd, input_section,
				  name);
	    }
	  else if ((int) (entry_vma - table_start_cache) % 4)
	    {
	      /* xgettext:c-format */
	      _bfd_error_handler (_("%pB:%pA: table entry %s not word-aligned within table"),
				  input_bfd, input_section,
				  name);
	    }
	  else
	    {
	      idx = (int) (entry_vma - table_start_cache) / 4;

	      /* This will look like $tableentry$<N>$<name> */
	      buf = (char *) bfd_malloc (12 + 20 + strlen (name + 20));
	      if (buf == NULL)
		return false;

	      sprintf (buf, "$tableentry$%d$%s", idx, name + 20);

	      h = (struct elf_link_hash_entry *) bfd_link_hash_lookup (info->hash, buf, false, false, true);

	      if (h)
		{
		  relocation = (h->root.u.def.value
				+ h->root.u.def.section->output_section->vma
				+ h->root.u.def.section->output_offset);;
		}

	      free (buf);
	    }
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	{
	  /* This is a relocatable link.  We don't have to change
	     anything, unless the reloc is against a section symbol,
	     in which case we have to adjust according to where the
	     section symbol winds up in the output section.  */
	  if (sym != NULL && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    rel->r_addend += sec->output_offset;
	  continue;
	}

      if (h != NULL && h->root.type == bfd_link_hash_undefweak)
	/* If the symbol is undefined and weak
	   then the relocation resolves to zero.  */
	relocation = 0;
      else
	{
	  if (howto->pc_relative)
	    {
	      relocation -= (input_section->output_section->vma
			     + input_section->output_offset
			     + rel->r_offset);
	      if (r_type != R_RX_RH_3_PCREL
		  && r_type != R_RX_DIR3U_PCREL)
		relocation ++;
	    }

	  relocation += rel->r_addend;
	}

      r = bfd_reloc_ok;

#define RANGE(a,b) \
  if (a > (long) relocation || (long) relocation > b)		\
    r = bfd_reloc_overflow
#define ALIGN(m) \
  if (relocation & m)						\
    r = bfd_reloc_other
#define OP(i) \
  (contents[rel->r_offset + (i)])
#define WARN_REDHAT(type) \
  /* xgettext:c-format */					\
  _bfd_error_handler						\
    (_("%pB:%pA: warning: deprecated Red Hat reloc "		\
       "%s detected against: %s"),				\
     input_bfd, input_section, #type, name)

      /* Check for unsafe relocs in PID mode.  These are any relocs where
	 an absolute address is being computed.  There are special cases
	 for relocs against symbols that are known to be referenced in
	 crt0.o before the PID base address register has been initialised.  */
#define UNSAFE_FOR_PID							\
  do									\
    {									\
      if (pid_mode							\
	  && sec != NULL						\
	  && sec->flags & SEC_READONLY					\
	  && !(input_section->flags & SEC_DEBUGGING)			\
	  && strcmp (name, "__pid_base") != 0				\
	  && strcmp (name, "__gp") != 0					\
	  && strcmp (name, "__romdatastart") != 0			\
	  && !saw_subtract)						\
	/* xgettext:c-format */						\
	_bfd_error_handler (_("%pB(%pA): unsafe PID relocation %s "	\
			      "at %#" PRIx64 " (against %s in %s)"),	\
			    input_bfd, input_section, howto->name,	\
			    (uint64_t) (input_section->output_section->vma \
					+ input_section->output_offset	\
					+ rel->r_offset),		\
			    name, sec->name);				\
    }									\
  while (0)

      /* Opcode relocs are always big endian.  Data relocs are bi-endian.  */
      switch (r_type)
	{
	case R_RX_NONE:
	  break;

	case R_RX_RH_RELAX:
	  break;

	case R_RX_RH_3_PCREL:
	  WARN_REDHAT ("RX_RH_3_PCREL");
	  RANGE (3, 10);
	  OP (0) &= 0xf8;
	  OP (0) |= relocation & 0x07;
	  break;

	case R_RX_RH_8_NEG:
	  WARN_REDHAT ("RX_RH_8_NEG");
	  relocation = - relocation;
	  /* Fall through.  */
	case R_RX_DIR8S_PCREL:
	  UNSAFE_FOR_PID;
	  RANGE (-128, 127);
	  OP (0) = relocation;
	  break;

	case R_RX_DIR8S:
	  UNSAFE_FOR_PID;
	  RANGE (-128, 255);
	  OP (0) = relocation;
	  break;

	case R_RX_DIR8U:
	  UNSAFE_FOR_PID;
	  RANGE (0, 255);
	  OP (0) = relocation;
	  break;

	case R_RX_RH_16_NEG:
	  WARN_REDHAT ("RX_RH_16_NEG");
	  relocation = - relocation;
	  /* Fall through.  */
	case R_RX_DIR16S_PCREL:
	  UNSAFE_FOR_PID;
	  RANGE (-32768, 32767);
#if RX_OPCODE_BIG_ENDIAN
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	case R_RX_RH_16_OP:
	  WARN_REDHAT ("RX_RH_16_OP");
	  UNSAFE_FOR_PID;
	  RANGE (-32768, 32767);
#if RX_OPCODE_BIG_ENDIAN
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	case R_RX_DIR16S:
	  UNSAFE_FOR_PID;
	  RANGE (-32768, 65535);
	  if (BIGE (output_bfd) && !(input_section->flags & SEC_CODE))
	    {
	      OP (1) = relocation;
	      OP (0) = relocation >> 8;
	    }
	  else
	    {
	      OP (0) = relocation;
	      OP (1) = relocation >> 8;
	    }
	  break;

	case R_RX_DIR16U:
	  UNSAFE_FOR_PID;
	  RANGE (0, 65536);
#if RX_OPCODE_BIG_ENDIAN
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	case R_RX_DIR16:
	  UNSAFE_FOR_PID;
	  RANGE (-32768, 65536);
#if RX_OPCODE_BIG_ENDIAN
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	case R_RX_DIR16_REV:
	  UNSAFE_FOR_PID;
	  RANGE (-32768, 65536);
#if RX_OPCODE_BIG_ENDIAN
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#else
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#endif
	  break;

	case R_RX_DIR3U_PCREL:
	  RANGE (3, 10);
	  OP (0) &= 0xf8;
	  OP (0) |= relocation & 0x07;
	  break;

	case R_RX_RH_24_NEG:
	  UNSAFE_FOR_PID;
	  WARN_REDHAT ("RX_RH_24_NEG");
	  relocation = - relocation;
	  /* Fall through.  */
	case R_RX_DIR24S_PCREL:
	  RANGE (-0x800000, 0x7fffff);
#if RX_OPCODE_BIG_ENDIAN
	  OP (2) = relocation;
	  OP (1) = relocation >> 8;
	  OP (0) = relocation >> 16;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
	  OP (2) = relocation >> 16;
#endif
	  break;

	case R_RX_RH_24_OP:
	  UNSAFE_FOR_PID;
	  WARN_REDHAT ("RX_RH_24_OP");
	  RANGE (-0x800000, 0x7fffff);
#if RX_OPCODE_BIG_ENDIAN
	  OP (2) = relocation;
	  OP (1) = relocation >> 8;
	  OP (0) = relocation >> 16;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
	  OP (2) = relocation >> 16;
#endif
	  break;

	case R_RX_DIR24S:
	  UNSAFE_FOR_PID;
	  RANGE (-0x800000, 0x7fffff);
	  if (BIGE (output_bfd) && !(input_section->flags & SEC_CODE))
	    {
	      OP (2) = relocation;
	      OP (1) = relocation >> 8;
	      OP (0) = relocation >> 16;
	    }
	  else
	    {
	      OP (0) = relocation;
	      OP (1) = relocation >> 8;
	      OP (2) = relocation >> 16;
	    }
	  break;

	case R_RX_RH_24_UNS:
	  UNSAFE_FOR_PID;
	  WARN_REDHAT ("RX_RH_24_UNS");
	  RANGE (0, 0xffffff);
#if RX_OPCODE_BIG_ENDIAN
	  OP (2) = relocation;
	  OP (1) = relocation >> 8;
	  OP (0) = relocation >> 16;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
	  OP (2) = relocation >> 16;
#endif
	  break;

	case R_RX_RH_32_NEG:
	  UNSAFE_FOR_PID;
	  WARN_REDHAT ("RX_RH_32_NEG");
	  relocation = - relocation;
#if RX_OPCODE_BIG_ENDIAN
	  OP (3) = relocation;
	  OP (2) = relocation >> 8;
	  OP (1) = relocation >> 16;
	  OP (0) = relocation >> 24;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
	  OP (2) = relocation >> 16;
	  OP (3) = relocation >> 24;
#endif
	  break;

	case R_RX_RH_32_OP:
	  UNSAFE_FOR_PID;
	  WARN_REDHAT ("RX_RH_32_OP");
#if RX_OPCODE_BIG_ENDIAN
	  OP (3) = relocation;
	  OP (2) = relocation >> 8;
	  OP (1) = relocation >> 16;
	  OP (0) = relocation >> 24;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
	  OP (2) = relocation >> 16;
	  OP (3) = relocation >> 24;
#endif
	  break;

	case R_RX_DIR32:
	  if (BIGE (output_bfd) && !(input_section->flags & SEC_CODE))
	    {
	      OP (3) = relocation;
	      OP (2) = relocation >> 8;
	      OP (1) = relocation >> 16;
	      OP (0) = relocation >> 24;
	    }
	  else
	    {
	      OP (0) = relocation;
	      OP (1) = relocation >> 8;
	      OP (2) = relocation >> 16;
	      OP (3) = relocation >> 24;
	    }
	  break;

	case R_RX_DIR32_REV:
	  if (BIGE (output_bfd))
	    {
	      OP (0) = relocation;
	      OP (1) = relocation >> 8;
	      OP (2) = relocation >> 16;
	      OP (3) = relocation >> 24;
	    }
	  else
	    {
	      OP (3) = relocation;
	      OP (2) = relocation >> 8;
	      OP (1) = relocation >> 16;
	      OP (0) = relocation >> 24;
	    }
	  break;

	case R_RX_RH_DIFF:
	  {
	    bfd_vma val;
	    WARN_REDHAT ("RX_RH_DIFF");
	    val = bfd_get_32 (output_bfd, & OP (0));
	    val -= relocation;
	    bfd_put_32 (output_bfd, val, & OP (0));
	  }
	  break;

	case R_RX_RH_GPRELB:
	  WARN_REDHAT ("RX_RH_GPRELB");
	  relocation -= get_gp (info, input_bfd, input_section, rel->r_offset);
	  RANGE (0, 65535);
#if RX_OPCODE_BIG_ENDIAN
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	case R_RX_RH_GPRELW:
	  WARN_REDHAT ("RX_RH_GPRELW");
	  relocation -= get_gp (info, input_bfd, input_section, rel->r_offset);
	  ALIGN (1);
	  relocation >>= 1;
	  RANGE (0, 65535);
#if RX_OPCODE_BIG_ENDIAN
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	case R_RX_RH_GPRELL:
	  WARN_REDHAT ("RX_RH_GPRELL");
	  relocation -= get_gp (info, input_bfd, input_section, rel->r_offset);
	  ALIGN (3);
	  relocation >>= 2;
	  RANGE (0, 65535);
#if RX_OPCODE_BIG_ENDIAN
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	/* Internal relocations just for relaxation:  */
	case R_RX_RH_ABS5p5B:
	  RX_STACK_POP (relocation);
	  RANGE (0, 31);
	  OP (0) &= 0xf8;
	  OP (0) |= relocation >> 2;
	  OP (1) &= 0x77;
	  OP (1) |= (relocation << 6) & 0x80;
	  OP (1) |= (relocation << 3) & 0x08;
	  break;

	case R_RX_RH_ABS5p5W:
	  RX_STACK_POP (relocation);
	  RANGE (0, 62);
	  ALIGN (1);
	  relocation >>= 1;
	  OP (0) &= 0xf8;
	  OP (0) |= relocation >> 2;
	  OP (1) &= 0x77;
	  OP (1) |= (relocation << 6) & 0x80;
	  OP (1) |= (relocation << 3) & 0x08;
	  break;

	case R_RX_RH_ABS5p5L:
	  RX_STACK_POP (relocation);
	  RANGE (0, 124);
	  ALIGN (3);
	  relocation >>= 2;
	  OP (0) &= 0xf8;
	  OP (0) |= relocation >> 2;
	  OP (1) &= 0x77;
	  OP (1) |= (relocation << 6) & 0x80;
	  OP (1) |= (relocation << 3) & 0x08;
	  break;

	case R_RX_RH_ABS5p8B:
	  RX_STACK_POP (relocation);
	  RANGE (0, 31);
	  OP (0) &= 0x70;
	  OP (0) |= (relocation << 3) & 0x80;
	  OP (0) |= relocation & 0x0f;
	  break;

	case R_RX_RH_ABS5p8W:
	  RX_STACK_POP (relocation);
	  RANGE (0, 62);
	  ALIGN (1);
	  relocation >>= 1;
	  OP (0) &= 0x70;
	  OP (0) |= (relocation << 3) & 0x80;
	  OP (0) |= relocation & 0x0f;
	  break;

	case R_RX_RH_ABS5p8L:
	  RX_STACK_POP (relocation);
	  RANGE (0, 124);
	  ALIGN (3);
	  relocation >>= 2;
	  OP (0) &= 0x70;
	  OP (0) |= (relocation << 3) & 0x80;
	  OP (0) |= relocation & 0x0f;
	  break;

	case R_RX_RH_UIMM4p8:
	  RANGE (0, 15);
	  OP (0) &= 0x0f;
	  OP (0) |= relocation << 4;
	  break;

	case R_RX_RH_UNEG4p8:
	  RANGE (-15, 0);
	  OP (0) &= 0x0f;
	  OP (0) |= (-relocation) << 4;
	  break;

	  /* Complex reloc handling:  */

	case R_RX_ABS32:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
#if RX_OPCODE_BIG_ENDIAN
	  OP (3) = relocation;
	  OP (2) = relocation >> 8;
	  OP (1) = relocation >> 16;
	  OP (0) = relocation >> 24;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
	  OP (2) = relocation >> 16;
	  OP (3) = relocation >> 24;
#endif
	  break;

	case R_RX_ABS32_REV:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
#if RX_OPCODE_BIG_ENDIAN
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
	  OP (2) = relocation >> 16;
	  OP (3) = relocation >> 24;
#else
	  OP (3) = relocation;
	  OP (2) = relocation >> 8;
	  OP (1) = relocation >> 16;
	  OP (0) = relocation >> 24;
#endif
	  break;

	case R_RX_ABS24S_PCREL:
	case R_RX_ABS24S:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
	  RANGE (-0x800000, 0x7fffff);
	  if (BIGE (output_bfd) && !(input_section->flags & SEC_CODE))
	    {
	      OP (2) = relocation;
	      OP (1) = relocation >> 8;
	      OP (0) = relocation >> 16;
	    }
	  else
	    {
	      OP (0) = relocation;
	      OP (1) = relocation >> 8;
	      OP (2) = relocation >> 16;
	    }
	  break;

	case R_RX_ABS16:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
	  RANGE (-32768, 65535);
#if RX_OPCODE_BIG_ENDIAN
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	case R_RX_ABS16_REV:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
	  RANGE (-32768, 65535);
#if RX_OPCODE_BIG_ENDIAN
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#else
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#endif
	  break;

	case R_RX_ABS16S_PCREL:
	case R_RX_ABS16S:
	  RX_STACK_POP (relocation);
	  RANGE (-32768, 32767);
	  if (BIGE (output_bfd) && !(input_section->flags & SEC_CODE))
	    {
	      OP (1) = relocation;
	      OP (0) = relocation >> 8;
	    }
	  else
	    {
	      OP (0) = relocation;
	      OP (1) = relocation >> 8;
	    }
	  break;

	case R_RX_ABS16U:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
	  RANGE (0, 65536);
#if RX_OPCODE_BIG_ENDIAN
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	case R_RX_ABS16UL:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
	  relocation >>= 2;
	  RANGE (0, 65536);
#if RX_OPCODE_BIG_ENDIAN
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	case R_RX_ABS16UW:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
	  relocation >>= 1;
	  RANGE (0, 65536);
#if RX_OPCODE_BIG_ENDIAN
	  OP (1) = relocation;
	  OP (0) = relocation >> 8;
#else
	  OP (0) = relocation;
	  OP (1) = relocation >> 8;
#endif
	  break;

	case R_RX_ABS8:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
	  RANGE (-128, 255);
	  OP (0) = relocation;
	  break;

	case R_RX_ABS8U:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
	  RANGE (0, 255);
	  OP (0) = relocation;
	  break;

	case R_RX_ABS8UL:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
	  relocation >>= 2;
	  RANGE (0, 255);
	  OP (0) = relocation;
	  break;

	case R_RX_ABS8UW:
	  UNSAFE_FOR_PID;
	  RX_STACK_POP (relocation);
	  relocation >>= 1;
	  RANGE (0, 255);
	  OP (0) = relocation;
	  break;

	case R_RX_ABS8S:
	  UNSAFE_FOR_PID;
	  /* Fall through.  */
	case R_RX_ABS8S_PCREL:
	  RX_STACK_POP (relocation);
	  RANGE (-128, 127);
	  OP (0) = relocation;
	  break;

	case R_RX_SYM:
	  if (r_symndx < symtab_hdr->sh_info)
	    RX_STACK_PUSH (sec->output_section->vma
			   + sec->output_offset
			   + sym->st_value
			   + rel->r_addend);
	  else
	    {
	      if (h != NULL
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak))
		RX_STACK_PUSH (h->root.u.def.value
			       + sec->output_section->vma
			       + sec->output_offset
			       + rel->r_addend);
	      else
		_bfd_error_handler
		  (_("warning: RX_SYM reloc with an unknown symbol"));
	    }
	  break;

	case R_RX_OPneg:
	  {
	    int32_t tmp;

	    saw_subtract = true;
	    RX_STACK_POP (tmp);
	    tmp = - tmp;
	    RX_STACK_PUSH (tmp);
	  }
	  break;

	case R_RX_OPadd:
	  {
	    int32_t tmp1, tmp2;

	    RX_STACK_POP (tmp1);
	    RX_STACK_POP (tmp2);
	    tmp1 += tmp2;
	    RX_STACK_PUSH (tmp1);
	  }
	  break;

	case R_RX_OPsub:
	  {
	    int32_t tmp1, tmp2;

	    saw_subtract = true;
	    RX_STACK_POP (tmp1);
	    RX_STACK_POP (tmp2);
	    tmp2 -= tmp1;
	    RX_STACK_PUSH (tmp2);
	  }
	  break;

	case R_RX_OPmul:
	  {
	    int32_t tmp1, tmp2;

	    RX_STACK_POP (tmp1);
	    RX_STACK_POP (tmp2);
	    tmp1 *= tmp2;
	    RX_STACK_PUSH (tmp1);
	  }
	  break;

	case R_RX_OPdiv:
	  {
	    int32_t tmp1, tmp2;

	    RX_STACK_POP (tmp1);
	    RX_STACK_POP (tmp2);
	    tmp1 /= tmp2;
	    RX_STACK_PUSH (tmp1);
	  }
	  break;

	case R_RX_OPshla:
	  {
	    int32_t tmp1, tmp2;

	    RX_STACK_POP (tmp1);
	    RX_STACK_POP (tmp2);
	    tmp1 <<= tmp2;
	    RX_STACK_PUSH (tmp1);
	  }
	  break;

	case R_RX_OPshra:
	  {
	    int32_t tmp1, tmp2;

	    RX_STACK_POP (tmp1);
	    RX_STACK_POP (tmp2);
	    tmp1 >>= tmp2;
	    RX_STACK_PUSH (tmp1);
	  }
	  break;

	case R_RX_OPsctsize:
	  RX_STACK_PUSH (input_section->size);
	  break;

	case R_RX_OPscttop:
	  RX_STACK_PUSH (input_section->output_section->vma);
	  break;

	case R_RX_OPand:
	  {
	    int32_t tmp1, tmp2;

	    RX_STACK_POP (tmp1);
	    RX_STACK_POP (tmp2);
	    tmp1 &= tmp2;
	    RX_STACK_PUSH (tmp1);
	  }
	  break;

	case R_RX_OPor:
	  {
	    int32_t tmp1, tmp2;

	    RX_STACK_POP (tmp1);
	    RX_STACK_POP (tmp2);
	    tmp1 |= tmp2;
	    RX_STACK_PUSH (tmp1);
	  }
	  break;

	case R_RX_OPxor:
	  {
	    int32_t tmp1, tmp2;

	    RX_STACK_POP (tmp1);
	    RX_STACK_POP (tmp2);
	    tmp1 ^= tmp2;
	    RX_STACK_PUSH (tmp1);
	  }
	  break;

	case R_RX_OPnot:
	  {
	    int32_t tmp;

	    RX_STACK_POP (tmp);
	    tmp = ~ tmp;
	    RX_STACK_PUSH (tmp);
	  }
	  break;

	case R_RX_OPmod:
	  {
	    int32_t tmp1, tmp2;

	    RX_STACK_POP (tmp1);
	    RX_STACK_POP (tmp2);
	    tmp1 %= tmp2;
	    RX_STACK_PUSH (tmp1);
	  }
	  break;

	case R_RX_OPromtop:
	  RX_STACK_PUSH (get_romstart (info, input_bfd, input_section, rel->r_offset));
	  break;

	case R_RX_OPramtop:
	  RX_STACK_PUSH (get_ramstart (info, input_bfd, input_section, rel->r_offset));
	  break;

	default:
	  r = bfd_reloc_notsupported;
	  break;
	}

      if (r != bfd_reloc_ok)
	{
	  const char * msg = NULL;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      /* Catch the case of a missing function declaration
		 and emit a more helpful error message.  */
	      if (r_type == R_RX_DIR24S_PCREL)
		/* xgettext:c-format */
		msg = _("%pB(%pA): error: call to undefined function '%s'");
	      else
		(*info->callbacks->reloc_overflow)
		  (info, (h ? &h->root : NULL), name, howto->name, (bfd_vma) 0,
		   input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, name, input_bfd, input_section, rel->r_offset, true);
	      break;

	    case bfd_reloc_other:
	      /* xgettext:c-format */
	      msg = _("%pB(%pA): warning: unaligned access to symbol '%s' in the small data area");
	      break;

	    case bfd_reloc_outofrange:
	      /* xgettext:c-format */
	      msg = _("%pB(%pA): internal error: out of range error");
	      break;

	    case bfd_reloc_notsupported:
	      /* xgettext:c-format */
	      msg = _("%pB(%pA): internal error: unsupported relocation error");
	      break;

	    case bfd_reloc_dangerous:
	      /* xgettext:c-format */
	      msg = _("%pB(%pA): internal error: dangerous relocation");
	      break;

	    default:
	      /* xgettext:c-format */
	      msg = _("%pB(%pA): internal error: unknown error");
	      break;
	    }

	  if (msg)
	    _bfd_error_handler (msg, input_bfd, input_section, name);
	}
    }

  return true;
}

/* Relaxation Support.  */

/* Progression of relocations from largest operand size to smallest
   operand size.  */

static int
next_smaller_reloc (int r)
{
  switch (r)
    {
    case R_RX_DIR32:		return R_RX_DIR24S;
    case R_RX_DIR24S:		return R_RX_DIR16S;
    case R_RX_DIR16S:		return R_RX_DIR8S;
    case R_RX_DIR8S:		return R_RX_NONE;

    case R_RX_DIR16:		return R_RX_DIR8;
    case R_RX_DIR8:		return R_RX_NONE;

    case R_RX_DIR16U:		return R_RX_DIR8U;
    case R_RX_DIR8U:		return R_RX_NONE;

    case R_RX_DIR24S_PCREL:	return R_RX_DIR16S_PCREL;
    case R_RX_DIR16S_PCREL:	return R_RX_DIR8S_PCREL;
    case R_RX_DIR8S_PCREL:	return R_RX_DIR3U_PCREL;

    case R_RX_DIR16UL:		return R_RX_DIR8UL;
    case R_RX_DIR8UL:		return R_RX_NONE;
    case R_RX_DIR16UW:		return R_RX_DIR8UW;
    case R_RX_DIR8UW:		return R_RX_NONE;

    case R_RX_RH_32_OP:		return R_RX_RH_24_OP;
    case R_RX_RH_24_OP:		return R_RX_RH_16_OP;
    case R_RX_RH_16_OP:		return R_RX_DIR8;

    case R_RX_ABS32:		return R_RX_ABS24S;
    case R_RX_ABS24S:		return R_RX_ABS16S;
    case R_RX_ABS16:		return R_RX_ABS8;
    case R_RX_ABS16U:		return R_RX_ABS8U;
    case R_RX_ABS16S:		return R_RX_ABS8S;
    case R_RX_ABS8:		return R_RX_NONE;
    case R_RX_ABS8U:		return R_RX_NONE;
    case R_RX_ABS8S:		return R_RX_NONE;
    case R_RX_ABS24S_PCREL:	return R_RX_ABS16S_PCREL;
    case R_RX_ABS16S_PCREL:	return R_RX_ABS8S_PCREL;
    case R_RX_ABS8S_PCREL:	return R_RX_NONE;
    case R_RX_ABS16UL:		return R_RX_ABS8UL;
    case R_RX_ABS16UW:		return R_RX_ABS8UW;
    case R_RX_ABS8UL:		return R_RX_NONE;
    case R_RX_ABS8UW:		return R_RX_NONE;
    }
  return r;
};

/* Delete some bytes from a section while relaxing.  */

static bool
elf32_rx_relax_delete_bytes (bfd *abfd, asection *sec, bfd_vma addr, int count,
			     Elf_Internal_Rela *alignment_rel, int force_snip,
			     Elf_Internal_Rela *irelstart)
{
  Elf_Internal_Shdr * symtab_hdr;
  unsigned int	      sec_shndx;
  bfd_byte *	      contents;
  Elf_Internal_Rela * irel;
  Elf_Internal_Rela * irelend;
  Elf_Internal_Sym *  isym;
  Elf_Internal_Sym *  isymend;
  bfd_vma	      toaddr;
  unsigned int	      symcount;
  struct elf_link_hash_entry ** sym_hashes;
  struct elf_link_hash_entry ** end_hashes;

  if (!alignment_rel)
    force_snip = 1;

  sec_shndx = _bfd_elf_section_from_bfd_section (abfd, sec);

  contents = elf_section_data (sec)->this_hdr.contents;

  /* The deletion must stop at the next alignment boundary, if
     ALIGNMENT_REL is non-NULL.  */
  toaddr = sec->size;
  if (alignment_rel)
    toaddr = alignment_rel->r_offset;

  BFD_ASSERT (toaddr > addr);

  /* Actually delete the bytes.  */
  memmove (contents + addr, contents + addr + count,
	   (size_t) (toaddr - addr - count));

  /* If we don't have an alignment marker to worry about, we can just
     shrink the section.  Otherwise, we have to fill in the newly
     created gap with NOP insns (0x03).  */
  if (force_snip)
    sec->size -= count;
  else
    memset (contents + toaddr - count, 0x03, count);

  irel = irelstart;
  BFD_ASSERT (irel != NULL || sec->reloc_count == 0);
  irelend = irel + sec->reloc_count;

  /* Adjust all the relocs.  */
  for (; irel < irelend; irel++)
    {
      /* Get the new reloc address.  */
      if (irel->r_offset > addr
	  && (irel->r_offset < toaddr
	      || (force_snip && irel->r_offset == toaddr)))
	irel->r_offset -= count;

      /* If we see an ALIGN marker at the end of the gap, we move it
	 to the beginning of the gap, since marking these gaps is what
	 they're for.  */
      if (irel->r_offset == toaddr
	  && ELF32_R_TYPE (irel->r_info) == R_RX_RH_RELAX
	  && irel->r_addend & RX_RELAXA_ALIGN)
	irel->r_offset -= count;
    }

  /* Adjust the local symbols defined in this section.  */
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  isym = (Elf_Internal_Sym *) symtab_hdr->contents;
  isymend = isym + symtab_hdr->sh_info;

  for (; isym < isymend; isym++)
    {
      /* If the symbol is in the range of memory we just moved, we
	 have to adjust its value.  */
      if (isym->st_shndx == sec_shndx
	  && isym->st_value > addr
	  && isym->st_value < toaddr)
	isym->st_value -= count;

      /* If the symbol *spans* the bytes we just deleted (i.e. it's
	 *end* is in the moved bytes but it's *start* isn't), then we
	 must adjust its size.  */
      if (isym->st_shndx == sec_shndx
	  && isym->st_value < addr
	  && isym->st_value + isym->st_size > addr
	  && isym->st_value + isym->st_size < toaddr)
	isym->st_size -= count;
    }

  /* Now adjust the global symbols defined in this section.  */
  symcount = (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)
	      - symtab_hdr->sh_info);
  sym_hashes = elf_sym_hashes (abfd);
  end_hashes = sym_hashes + symcount;

  for (; sym_hashes < end_hashes; sym_hashes++)
    {
      struct elf_link_hash_entry *sym_hash = *sym_hashes;

      if ((sym_hash->root.type == bfd_link_hash_defined
	   || sym_hash->root.type == bfd_link_hash_defweak)
	  && sym_hash->root.u.def.section == sec)
	{
	  /* As above, adjust the value if needed.  */
	  if (sym_hash->root.u.def.value > addr
	      && sym_hash->root.u.def.value < toaddr)
	    sym_hash->root.u.def.value -= count;

	  /* As above, adjust the size if needed.  */
	  if (sym_hash->root.u.def.value < addr
	      && sym_hash->root.u.def.value + sym_hash->size > addr
	      && sym_hash->root.u.def.value + sym_hash->size < toaddr)
	    sym_hash->size -= count;
	}
    }

  return true;
}

/* Used to sort relocs by address.  If relocs have the same address,
   we maintain their relative order, except that R_RX_RH_RELAX
   alignment relocs must be the first reloc for any given address.  */

static void
reloc_bubblesort (Elf_Internal_Rela * r, int count)
{
  int i;
  bool again;
  bool swappit;

  /* This is almost a classic bubblesort.  It's the slowest sort, but
     we're taking advantage of the fact that the relocations are
     mostly in order already (the assembler emits them that way) and
     we need relocs with the same address to remain in the same
     relative order.  */
  again = true;
  while (again)
    {
      again = false;
      for (i = 0; i < count - 1; i ++)
	{
	  if (r[i].r_offset > r[i + 1].r_offset)
	    swappit = true;
	  else if (r[i].r_offset < r[i + 1].r_offset)
	    swappit = false;
	  else if (ELF32_R_TYPE (r[i + 1].r_info) == R_RX_RH_RELAX
		   && (r[i + 1].r_addend & RX_RELAXA_ALIGN))
	    swappit = true;
	  else if (ELF32_R_TYPE (r[i + 1].r_info) == R_RX_RH_RELAX
		   && (r[i + 1].r_addend & RX_RELAXA_ELIGN)
		   && !(ELF32_R_TYPE (r[i].r_info) == R_RX_RH_RELAX
			&& (r[i].r_addend & RX_RELAXA_ALIGN)))
	    swappit = true;
	  else
	    swappit = false;

	  if (swappit)
	    {
	      Elf_Internal_Rela tmp;

	      tmp = r[i];
	      r[i] = r[i + 1];
	      r[i + 1] = tmp;
	      /* If we do move a reloc back, re-scan to see if it
		 needs to be moved even further back.  This avoids
		 most of the O(n^2) behavior for our cases.  */
	      if (i > 0)
		i -= 2;
	      again = true;
	    }
	}
    }
}


#define OFFSET_FOR_RELOC(rel, lrel, scale) \
  rx_offset_for_reloc (abfd, rel + 1, symtab_hdr, shndx_buf, intsyms, \
		       lrel, abfd, sec, link_info, scale)

static bfd_vma
rx_offset_for_reloc (bfd *		      abfd,
		     Elf_Internal_Rela *      rel,
		     Elf_Internal_Shdr *      symtab_hdr,
		     bfd_byte *		      shndx_buf ATTRIBUTE_UNUSED,
		     Elf_Internal_Sym *	      intsyms,
		     Elf_Internal_Rela **     lrel,
		     bfd *		      input_bfd,
		     asection *		      input_section,
		     struct bfd_link_info *   info,
		     int *		      scale)
{
  bfd_vma symval;
  bfd_reloc_status_type r;

  *scale = 1;

  /* REL is the first of 1..N relocations.  We compute the symbol
     value for each relocation, then combine them if needed.  LREL
     gets a pointer to the last relocation used.  */
  while (1)
    {
      int32_t tmp1, tmp2;

      /* Get the value of the symbol referred to by the reloc.  */
      if (ELF32_R_SYM (rel->r_info) < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  Elf_Internal_Sym *isym;
	  asection *ssec;

	  isym = intsyms + ELF32_R_SYM (rel->r_info);

	  if (isym->st_shndx == SHN_UNDEF)
	    ssec = bfd_und_section_ptr;
	  else if (isym->st_shndx == SHN_ABS)
	    ssec = bfd_abs_section_ptr;
	  else if (isym->st_shndx == SHN_COMMON)
	    ssec = bfd_com_section_ptr;
	  else
	    ssec = bfd_section_from_elf_index (abfd,
					       isym->st_shndx);

	  /* Initial symbol value.  */
	  symval = isym->st_value;

	  /* GAS may have made this symbol relative to a section, in
	     which case, we have to add the addend to find the
	     symbol.  */
	  if (ELF_ST_TYPE (isym->st_info) == STT_SECTION)
	    symval += rel->r_addend;

	  if (ssec)
	    {
	      if ((ssec->flags & SEC_MERGE)
		  && ssec->sec_info_type == SEC_INFO_TYPE_MERGE)
		symval = _bfd_merged_section_offset (abfd, & ssec,
						     elf_section_data (ssec)->sec_info,
						     symval);
	    }

	  /* Now make the offset relative to where the linker is putting it.  */
	  if (ssec)
	    symval +=
	      ssec->output_section->vma + ssec->output_offset;

	  symval += rel->r_addend;
	}
      else
	{
	  unsigned long indx;
	  struct elf_link_hash_entry * h;

	  /* An external symbol.  */
	  indx = ELF32_R_SYM (rel->r_info) - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	  BFD_ASSERT (h != NULL);

	  if (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	    {
	      /* This appears to be a reference to an undefined
		 symbol.  Just ignore it--it will be caught by the
		 regular reloc processing.  */
	      if (lrel)
		*lrel = rel;
	      return 0;
	    }

	  symval = (h->root.u.def.value
		    + h->root.u.def.section->output_section->vma
		    + h->root.u.def.section->output_offset);

	  symval += rel->r_addend;
	}

      switch (ELF32_R_TYPE (rel->r_info))
	{
	case R_RX_SYM:
	  RX_STACK_PUSH (symval);
	  break;

	case R_RX_OPneg:
	  RX_STACK_POP (tmp1);
	  tmp1 = - tmp1;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPadd:
	  RX_STACK_POP (tmp1);
	  RX_STACK_POP (tmp2);
	  tmp1 += tmp2;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPsub:
	  RX_STACK_POP (tmp1);
	  RX_STACK_POP (tmp2);
	  tmp2 -= tmp1;
	  RX_STACK_PUSH (tmp2);
	  break;

	case R_RX_OPmul:
	  RX_STACK_POP (tmp1);
	  RX_STACK_POP (tmp2);
	  tmp1 *= tmp2;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPdiv:
	  RX_STACK_POP (tmp1);
	  RX_STACK_POP (tmp2);
	  tmp1 /= tmp2;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPshla:
	  RX_STACK_POP (tmp1);
	  RX_STACK_POP (tmp2);
	  tmp1 <<= tmp2;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPshra:
	  RX_STACK_POP (tmp1);
	  RX_STACK_POP (tmp2);
	  tmp1 >>= tmp2;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPsctsize:
	  RX_STACK_PUSH (input_section->size);
	  break;

	case R_RX_OPscttop:
	  RX_STACK_PUSH (input_section->output_section->vma);
	  break;

	case R_RX_OPand:
	  RX_STACK_POP (tmp1);
	  RX_STACK_POP (tmp2);
	  tmp1 &= tmp2;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPor:
	  RX_STACK_POP (tmp1);
	  RX_STACK_POP (tmp2);
	  tmp1 |= tmp2;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPxor:
	  RX_STACK_POP (tmp1);
	  RX_STACK_POP (tmp2);
	  tmp1 ^= tmp2;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPnot:
	  RX_STACK_POP (tmp1);
	  tmp1 = ~ tmp1;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPmod:
	  RX_STACK_POP (tmp1);
	  RX_STACK_POP (tmp2);
	  tmp1 %= tmp2;
	  RX_STACK_PUSH (tmp1);
	  break;

	case R_RX_OPromtop:
	  RX_STACK_PUSH (get_romstart (info, input_bfd, input_section, rel->r_offset));
	  break;

	case R_RX_OPramtop:
	  RX_STACK_PUSH (get_ramstart (info, input_bfd, input_section, rel->r_offset));
	  break;

	case R_RX_DIR16UL:
	case R_RX_DIR8UL:
	case R_RX_ABS16UL:
	case R_RX_ABS8UL:
	  if (rx_stack_top)
	    RX_STACK_POP (symval);
	  if (lrel)
	    *lrel = rel;
	  *scale = 4;
	  return symval;

	case R_RX_DIR16UW:
	case R_RX_DIR8UW:
	case R_RX_ABS16UW:
	case R_RX_ABS8UW:
	  if (rx_stack_top)
	    RX_STACK_POP (symval);
	  if (lrel)
	    *lrel = rel;
	  *scale = 2;
	  return symval;

	default:
	  if (rx_stack_top)
	    RX_STACK_POP (symval);
	  if (lrel)
	    *lrel = rel;
	  return symval;
	}

      rel ++;
    }
  /* FIXME.  */
  (void) r;
}

static void
move_reloc (Elf_Internal_Rela * irel, Elf_Internal_Rela * srel, int delta)
{
  bfd_vma old_offset = srel->r_offset;

  irel ++;
  while (irel <= srel)
    {
      if (irel->r_offset == old_offset)
	irel->r_offset += delta;
      irel ++;
    }
}

/* Relax one section.  */

static bool
elf32_rx_relax_section (bfd *abfd,
			asection *sec,
			struct bfd_link_info *link_info,
			bool *again,
			bool allow_pcrel3)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Shdr *shndx_hdr;
  Elf_Internal_Rela *internal_relocs;
  Elf_Internal_Rela *irel;
  Elf_Internal_Rela *srel;
  Elf_Internal_Rela *irelend;
  Elf_Internal_Rela *next_alignment;
  Elf_Internal_Rela *prev_alignment;
  bfd_byte *contents = NULL;
  bfd_byte *free_contents = NULL;
  Elf_Internal_Sym *intsyms = NULL;
  Elf_Internal_Sym *free_intsyms = NULL;
  bfd_byte *shndx_buf = NULL;
  bfd_vma pc;
  bfd_vma sec_start;
  bfd_vma symval = 0;
  int pcrel = 0;
  int code = 0;
  int section_alignment_glue;
  /* how much to scale the relocation by - 1, 2, or 4.  */
  int scale;

  /* Assume nothing changes.  */
  *again = false;

  /* We don't have to do anything for a relocatable link, if
     this section does not have relocs, or if this is not a
     code section.  */
  if (bfd_link_relocatable (link_info)
      || sec->reloc_count == 0
      || (sec->flags & SEC_RELOC) == 0
      || (sec->flags & SEC_HAS_CONTENTS) == 0
      || (sec->flags & SEC_CODE) == 0)
    return true;

  symtab_hdr = & elf_symtab_hdr (abfd);
  if (elf_symtab_shndx_list (abfd))
    shndx_hdr = & elf_symtab_shndx_list (abfd)->hdr;
  else
    shndx_hdr = NULL;

  sec_start = sec->output_section->vma + sec->output_offset;

  /* Get the section contents.  */
  if (elf_section_data (sec)->this_hdr.contents != NULL)
    contents = elf_section_data (sec)->this_hdr.contents;
  /* Go get them off disk.  */
  else
    {
      if (! bfd_malloc_and_get_section (abfd, sec, &contents))
	goto error_return;
      elf_section_data (sec)->this_hdr.contents = contents;
    }

  /* Read this BFD's symbols.  */
  /* Get cached copy if it exists.  */
  if (symtab_hdr->contents != NULL)
    intsyms = (Elf_Internal_Sym *) symtab_hdr->contents;
  else
    {
      intsyms = bfd_elf_get_elf_syms (abfd, symtab_hdr, symtab_hdr->sh_info, 0, NULL, NULL, NULL);
      symtab_hdr->contents = (bfd_byte *) intsyms;
    }

  if (shndx_hdr && shndx_hdr->sh_size != 0)
    {
      size_t amt;

      if (_bfd_mul_overflow (symtab_hdr->sh_info,
			     sizeof (Elf_External_Sym_Shndx), &amt))
	{
	  bfd_set_error (bfd_error_file_too_big);
	  goto error_return;
	}
      if (bfd_seek (abfd, shndx_hdr->sh_offset, SEEK_SET) != 0)
	goto error_return;
      shndx_buf = _bfd_malloc_and_read (abfd, amt, amt);
      if (shndx_buf == NULL)
	goto error_return;
      shndx_hdr->contents = shndx_buf;
    }

  /* Get a copy of the native relocations.  */
  /* Note - we ignore the setting of link_info->keep_memory when reading
     in these relocs.  We have to maintain a permanent copy of the relocs
     because we are going to walk over them multiple times, adjusting them
     as bytes are deleted from the section, and with this relaxation
     function itself being called multiple times on the same section...  */
  internal_relocs = _bfd_elf_link_read_relocs
    (abfd, sec, NULL, (Elf_Internal_Rela *) NULL, true);
  if (internal_relocs == NULL)
    goto error_return;

  /* The RL_ relocs must be just before the operand relocs they go
     with, so we must sort them to guarantee this.  We use bubblesort
     instead of qsort so we can guarantee that relocs with the same
     address remain in the same relative order.  */
  reloc_bubblesort (internal_relocs, sec->reloc_count);

  /* Walk through them looking for relaxing opportunities.  */
  irelend = internal_relocs + sec->reloc_count;

  /* This will either be NULL or a pointer to the next alignment
     relocation.  */
  next_alignment = internal_relocs;
  /* This will be the previous alignment, although at first it points
     to the first real relocation.  */
  prev_alignment = internal_relocs;

  /* We calculate worst case shrinkage caused by alignment directives.
     No fool-proof, but better than either ignoring the problem or
     doing heavy duty analysis of all the alignment markers in all
     input sections.  */
  section_alignment_glue = 0;
  for (irel = internal_relocs; irel < irelend; irel++)
      if (ELF32_R_TYPE (irel->r_info) == R_RX_RH_RELAX
	  && irel->r_addend & RX_RELAXA_ALIGN)
	{
	  int this_glue = 1 << (irel->r_addend & RX_RELAXA_ANUM);

	  if (section_alignment_glue < this_glue)
	    section_alignment_glue = this_glue;
	}
  /* Worst case is all 0..N alignments, in order, causing 2*N-1 byte
     shrinkage.  */
  section_alignment_glue *= 2;

  for (irel = internal_relocs; irel < irelend; irel++)
    {
      unsigned char *insn;
      int nrelocs;

      /* The insns we care about are all marked with one of these.  */
      if (ELF32_R_TYPE (irel->r_info) != R_RX_RH_RELAX)
	continue;

      if (irel->r_addend & RX_RELAXA_ALIGN
	  || next_alignment == internal_relocs)
	{
	  /* When we delete bytes, we need to maintain all the alignments
	     indicated.  In addition, we need to be careful about relaxing
	     jumps across alignment boundaries - these displacements
	     *grow* when we delete bytes.  For now, don't shrink
	     displacements across an alignment boundary, just in case.
	     Note that this only affects relocations to the same
	     section.  */
	  prev_alignment = next_alignment;
	  next_alignment += 2;
	  while (next_alignment < irelend
		 && (ELF32_R_TYPE (next_alignment->r_info) != R_RX_RH_RELAX
		     || !(next_alignment->r_addend & RX_RELAXA_ELIGN)))
	    next_alignment ++;
	  if (next_alignment >= irelend || next_alignment->r_offset == 0)
	    next_alignment = NULL;
	}

      /* When we hit alignment markers, see if we've shrunk enough
	 before them to reduce the gap without violating the alignment
	 requirements.  */
      if (irel->r_addend & RX_RELAXA_ALIGN)
	{
	  /* At this point, the next relocation *should* be the ELIGN
	     end marker.  */
	  Elf_Internal_Rela *erel = irel + 1;
	  unsigned int alignment, nbytes;

	  if (ELF32_R_TYPE (erel->r_info) != R_RX_RH_RELAX)
	    continue;
	  if (!(erel->r_addend & RX_RELAXA_ELIGN))
	    continue;

	  alignment = 1 << (irel->r_addend & RX_RELAXA_ANUM);

	  if (erel->r_offset - irel->r_offset < alignment)
	    continue;

	  nbytes = erel->r_offset - irel->r_offset;
	  nbytes /= alignment;
	  nbytes *= alignment;

	  elf32_rx_relax_delete_bytes (abfd, sec, erel->r_offset-nbytes, nbytes, next_alignment,
				       erel->r_offset == sec->size, internal_relocs);
	  *again = true;

	  continue;
	}

      if (irel->r_addend & RX_RELAXA_ELIGN)
	  continue;

      insn = contents + irel->r_offset;

      nrelocs = irel->r_addend & RX_RELAXA_RNUM;

      /* At this point, we have an insn that is a candidate for linker
	 relaxation.  There are NRELOCS relocs following that may be
	 relaxed, although each reloc may be made of more than one
	 reloc entry (such as gp-rel symbols).  */

      /* Get the value of the symbol referred to by the reloc.  Just
	 in case this is the last reloc in the list, use the RL's
	 addend to choose between this reloc (no addend) or the next
	 (yes addend, which means at least one following reloc).  */

      /* srel points to the "current" reloction for this insn -
	 actually the last reloc for a given operand, which is the one
	 we need to update.  We check the relaxations in the same
	 order that the relocations happen, so we'll just push it
	 along as we go.  */
      srel = irel;

      pc = sec->output_section->vma + sec->output_offset
	+ srel->r_offset;

#define GET_RELOC \
      symval = OFFSET_FOR_RELOC (srel, &srel, &scale); \
      pcrel = symval - pc + srel->r_addend; \
      nrelocs --;

#define SNIPNR(offset, nbytes) \
      elf32_rx_relax_delete_bytes (abfd, sec, (insn - contents) + offset, nbytes, next_alignment, 0, internal_relocs);
#define SNIP(offset, nbytes, newtype) \
	SNIPNR (offset, nbytes);						\
	srel->r_info = ELF32_R_INFO (ELF32_R_SYM (srel->r_info), newtype)

      /* The order of these bit tests must match the order that the
	 relocs appear in.  Since we sorted those by offset, we can
	 predict them.  */

      /* Note that the numbers in, say, DSP6 are the bit offsets of
	 the code fields that describe the operand.  Bits number 0 for
	 the MSB of insn[0].  */

      /* DSP* codes:
	   0  00  [reg]
	   1  01  dsp:8[reg]
	   2  10  dsp:16[reg]
	   3  11  reg  */
      if (irel->r_addend & RX_RELAXA_DSP6)
	{
	  GET_RELOC;

	  code = insn[0] & 3;
	  if (code == 2 && symval/scale <= 255)
	    {
	      unsigned int newrel = ELF32_R_TYPE (srel->r_info);
	      insn[0] &= 0xfc;
	      insn[0] |= 0x01;
	      newrel = next_smaller_reloc (ELF32_R_TYPE (srel->r_info));
	      if (newrel != ELF32_R_TYPE (srel->r_info))
		{
		  SNIP (3, 1, newrel);
		  *again = true;
		}
	    }

	  else if (code == 1 && symval == 0)
	    {
	      insn[0] &= 0xfc;
	      SNIP (2, 1, R_RX_NONE);
	      *again = true;
	    }

	  /* Special case DSP:5 format: MOV.bwl dsp:5[Rsrc],Rdst.  */
	  else if (code == 1 && symval/scale <= 31
		   /* Decodable bits.  */
		   && (insn[0] & 0xcc) == 0xcc
		   /* Width.  */
		   && (insn[0] & 0x30) != 0x30
		   /* Register MSBs.  */
		   && (insn[1] & 0x88)  == 0x00)
	    {
	      int newrel = 0;

	      insn[0] = 0x88 | (insn[0] & 0x30);
	      /* The register fields are in the right place already.  */

	      /* We can't relax this new opcode.  */
	      irel->r_addend = 0;

	      switch ((insn[0] & 0x30) >> 4)
		{
		case 0:
		  newrel = R_RX_RH_ABS5p5B;
		  break;
		case 1:
		  newrel = R_RX_RH_ABS5p5W;
		  break;
		case 2:
		  newrel = R_RX_RH_ABS5p5L;
		  break;
		}

	      move_reloc (irel, srel, -2);
	      SNIP (2, 1, newrel);
	    }

	  /* Special case DSP:5 format: MOVU.bw dsp:5[Rsrc],Rdst.  */
	  else if (code == 1 && symval/scale <= 31
		   /* Decodable bits.  */
		   && (insn[0] & 0xf8) == 0x58
		   /* Register MSBs.  */
		   && (insn[1] & 0x88)  == 0x00)
	    {
	      int newrel = 0;

	      insn[0] = 0xb0 | ((insn[0] & 0x04) << 1);
	      /* The register fields are in the right place already.  */

	      /* We can't relax this new opcode.  */
	      irel->r_addend = 0;

	      switch ((insn[0] & 0x08) >> 3)
		{
		case 0:
		  newrel = R_RX_RH_ABS5p5B;
		  break;
		case 1:
		  newrel = R_RX_RH_ABS5p5W;
		  break;
		}

	      move_reloc (irel, srel, -2);
	      SNIP (2, 1, newrel);
	    }
	}

      /* A DSP4 operand always follows a DSP6 operand, even if there's
	 no relocation for it.  We have to read the code out of the
	 opcode to calculate the offset of the operand.  */
      if (irel->r_addend & RX_RELAXA_DSP4)
	{
	  int code6, offset = 0;

	  GET_RELOC;

	  code6 = insn[0] & 0x03;
	  switch (code6)
	    {
	    case 0: offset = 2; break;
	    case 1: offset = 3; break;
	    case 2: offset = 4; break;
	    case 3: offset = 2; break;
	    }

	  code = (insn[0] & 0x0c) >> 2;

	  if (code == 2 && symval / scale <= 255)
	    {
	      unsigned int newrel = ELF32_R_TYPE (srel->r_info);

	      insn[0] &= 0xf3;
	      insn[0] |= 0x04;
	      newrel = next_smaller_reloc (ELF32_R_TYPE (srel->r_info));
	      if (newrel != ELF32_R_TYPE (srel->r_info))
		{
		  SNIP (offset+1, 1, newrel);
		  *again = true;
		}
	    }

	  else if (code == 1 && symval == 0)
	    {
	      insn[0] &= 0xf3;
	      SNIP (offset, 1, R_RX_NONE);
	      *again = true;
	    }
	  /* Special case DSP:5 format: MOV.bwl Rsrc,dsp:5[Rdst] */
	  else if (code == 1 && symval/scale <= 31
		   /* Decodable bits.  */
		   && (insn[0] & 0xc3) == 0xc3
		   /* Width.  */
		   && (insn[0] & 0x30) != 0x30
		   /* Register MSBs.  */
		   && (insn[1] & 0x88)  == 0x00)
	    {
	      int newrel = 0;

	      insn[0] = 0x80 | (insn[0] & 0x30);
	      /* The register fields are in the right place already.  */

	      /* We can't relax this new opcode.  */
	      irel->r_addend = 0;

	      switch ((insn[0] & 0x30) >> 4)
		{
		case 0:
		  newrel = R_RX_RH_ABS5p5B;
		  break;
		case 1:
		  newrel = R_RX_RH_ABS5p5W;
		  break;
		case 2:
		  newrel = R_RX_RH_ABS5p5L;
		  break;
		}

	      move_reloc (irel, srel, -2);
	      SNIP (2, 1, newrel);
	    }
	}

      /* These always occur alone, but the offset depends on whether
	 it's a MEMEX opcode (0x06) or not.  */
      if (irel->r_addend & RX_RELAXA_DSP14)
	{
	  int offset;
	  GET_RELOC;

	  if (insn[0] == 0x06)
	    offset = 3;
	  else
	    offset = 4;

	  code = insn[1] & 3;

	  if (code == 2 && symval / scale <= 255)
	    {
	      unsigned int newrel = ELF32_R_TYPE (srel->r_info);

	      insn[1] &= 0xfc;
	      insn[1] |= 0x01;
	      newrel = next_smaller_reloc (ELF32_R_TYPE (srel->r_info));
	      if (newrel != ELF32_R_TYPE (srel->r_info))
		{
		  SNIP (offset, 1, newrel);
		  *again = true;
		}
	    }
	  else if (code == 1 && symval == 0)
	    {
	      insn[1] &= 0xfc;
	      SNIP (offset, 1, R_RX_NONE);
	      *again = true;
	    }
	}

      /* IMM* codes:
	   0  00  imm:32
	   1  01  simm:8
	   2  10  simm:16
	   3  11  simm:24.  */

      /* These always occur alone.  */
      if (irel->r_addend & RX_RELAXA_IMM6)
	{
	  long ssymval;

	  GET_RELOC;

	  /* These relocations sign-extend, so we must do signed compares.  */
	  ssymval = (long) symval;

	  code = insn[0] & 0x03;

	  if (code == 0 && ssymval <= 8388607 && ssymval >= -8388608)
	    {
	      unsigned int newrel = ELF32_R_TYPE (srel->r_info);

	      insn[0] &= 0xfc;
	      insn[0] |= 0x03;
	      newrel = next_smaller_reloc (ELF32_R_TYPE (srel->r_info));
	      if (newrel != ELF32_R_TYPE (srel->r_info))
		{
		  SNIP (2, 1, newrel);
		  *again = true;
		}
	    }

	  else if (code == 3 && ssymval <= 32767 && ssymval >= -32768)
	    {
	      unsigned int newrel = ELF32_R_TYPE (srel->r_info);

	      insn[0] &= 0xfc;
	      insn[0] |= 0x02;
	      newrel = next_smaller_reloc (ELF32_R_TYPE (srel->r_info));
	      if (newrel != ELF32_R_TYPE (srel->r_info))
		{
		  SNIP (2, 1, newrel);
		  *again = true;
		}
	    }

	  /* Special case UIMM8 format: CMP #uimm8,Rdst.  */
	  else if (code == 2 && ssymval <= 255 && ssymval >= 16
		   /* Decodable bits.  */
		   && (insn[0] & 0xfc) == 0x74
		   /* Decodable bits.  */
		   && ((insn[1] & 0xf0) == 0x00))
	    {
	      int newrel;

	      insn[0] = 0x75;
	      insn[1] = 0x50 | (insn[1] & 0x0f);

	      /* We can't relax this new opcode.  */
	      irel->r_addend = 0;

	      if (STACK_REL_P (ELF32_R_TYPE (srel->r_info)))
		newrel = R_RX_ABS8U;
	      else
		newrel = R_RX_DIR8U;

	      SNIP (2, 1, newrel);
	      *again = true;
	    }

	  else if (code == 2 && ssymval <= 127 && ssymval >= -128)
	    {
	      unsigned int newrel = ELF32_R_TYPE (srel->r_info);

	      insn[0] &= 0xfc;
	      insn[0] |= 0x01;
	      newrel = next_smaller_reloc (ELF32_R_TYPE (srel->r_info));
	      if (newrel != ELF32_R_TYPE (srel->r_info))
		{
		  SNIP (2, 1, newrel);
		  *again = true;
		}
	    }

	  /* Special case UIMM4 format: CMP, MUL, AND, OR.  */
	  else if (code == 1 && ssymval <= 15 && ssymval >= 0
		   /* Decodable bits and immediate type.  */
		   && insn[0] == 0x75
		   /* Decodable bits.  */
		   && (insn[1] & 0xc0)  == 0x00)
	    {
	      static const int newop[4] = { 1, 3, 4, 5 };

	      insn[0] = 0x60 | newop[insn[1] >> 4];
	      /* The register number doesn't move.  */

	      /* We can't relax this new opcode.  */
	      irel->r_addend = 0;

	      move_reloc (irel, srel, -1);

	      SNIP (2, 1, R_RX_RH_UIMM4p8);
	      *again = true;
	    }

	  /* Special case UIMM4 format: ADD -> ADD/SUB.  */
	  else if (code == 1 && ssymval <= 15 && ssymval >= -15
		   /* Decodable bits and immediate type.  */
		   && insn[0] == 0x71
		   /* Same register for source and destination.  */
		   && ((insn[1] >> 4) == (insn[1] & 0x0f)))
	    {
	      int newrel;

	      /* Note that we can't turn "add $0,Rs" into a NOP
		 because the flags need to be set right.  */

	      if (ssymval < 0)
		{
		  insn[0] = 0x60; /* Subtract.  */
		  newrel = R_RX_RH_UNEG4p8;
		}
	      else
		{
		  insn[0] = 0x62; /* Add.  */
		  newrel = R_RX_RH_UIMM4p8;
		}

	      /* The register number is in the right place.  */

	      /* We can't relax this new opcode.  */
	      irel->r_addend = 0;

	      move_reloc (irel, srel, -1);

	      SNIP (2, 1, newrel);
	      *again = true;
	    }
	}

      /* These are either matched with a DSP6 (2-byte base) or an id24
	 (3-byte base).  */
      if (irel->r_addend & RX_RELAXA_IMM12)
	{
	  int dspcode, offset = 0;
	  long ssymval;

	  GET_RELOC;

	  if ((insn[0] & 0xfc) == 0xfc)
	    dspcode = 1; /* Just something with one byte operand.  */
	  else
	    dspcode = insn[0] & 3;
	  switch (dspcode)
	    {
	    case 0: offset = 2; break;
	    case 1: offset = 3; break;
	    case 2: offset = 4; break;
	    case 3: offset = 2; break;
	    }

	  /* These relocations sign-extend, so we must do signed compares.  */
	  ssymval = (long) symval;

	  code = (insn[1] >> 2) & 3;
	  if (code == 0 && ssymval <= 8388607 && ssymval >= -8388608)
	    {
	      unsigned int newrel = ELF32_R_TYPE (srel->r_info);

	      insn[1] &= 0xf3;
	      insn[1] |= 0x0c;
	      newrel = next_smaller_reloc (ELF32_R_TYPE (srel->r_info));
	      if (newrel != ELF32_R_TYPE (srel->r_info))
		{
		  SNIP (offset, 1, newrel);
		  *again = true;
		}
	    }

	  else if (code == 3 && ssymval <= 32767 && ssymval >= -32768)
	    {
	      unsigned int newrel = ELF32_R_TYPE (srel->r_info);

	      insn[1] &= 0xf3;
	      insn[1] |= 0x08;
	      newrel = next_smaller_reloc (ELF32_R_TYPE (srel->r_info));
	      if (newrel != ELF32_R_TYPE (srel->r_info))
		{
		  SNIP (offset, 1, newrel);
		  *again = true;
		}
	    }

	  /* Special case UIMM8 format: MOV #uimm8,Rdst.  */
	  else if (code == 2 && ssymval <= 255 && ssymval >= 16
		   /* Decodable bits.  */
		   && insn[0] == 0xfb
		   /* Decodable bits.  */
		   && ((insn[1] & 0x03) == 0x02))
	    {
	      int newrel;

	      insn[0] = 0x75;
	      insn[1] = 0x40 | (insn[1] >> 4);

	      /* We can't relax this new opcode.  */
	      irel->r_addend = 0;

	      if (STACK_REL_P (ELF32_R_TYPE (srel->r_info)))
		newrel = R_RX_ABS8U;
	      else
		newrel = R_RX_DIR8U;

	      SNIP (2, 1, newrel);
	      *again = true;
	    }

	  else if (code == 2 && ssymval <= 127 && ssymval >= -128)
	    {
	      unsigned int newrel = ELF32_R_TYPE(srel->r_info);

	      insn[1] &= 0xf3;
	      insn[1] |= 0x04;
	      newrel = next_smaller_reloc (ELF32_R_TYPE (srel->r_info));
	      if (newrel != ELF32_R_TYPE(srel->r_info))
		{
		  SNIP (offset, 1, newrel);
		  *again = true;
		}
	    }

	  /* Special case UIMM4 format: MOV #uimm4,Rdst.  */
	  else if (code == 1 && ssymval <= 15 && ssymval >= 0
		   /* Decodable bits.  */
		   && insn[0] == 0xfb
		   /* Decodable bits.  */
		   && ((insn[1] & 0x03) == 0x02))
	    {
	      insn[0] = 0x66;
	      insn[1] = insn[1] >> 4;

	      /* We can't relax this new opcode.  */
	      irel->r_addend = 0;

	      move_reloc (irel, srel, -1);

	      SNIP (2, 1, R_RX_RH_UIMM4p8);
	      *again = true;
	    }
	}

      if (irel->r_addend & RX_RELAXA_BRA)
	{
	  unsigned int newrel = ELF32_R_TYPE (srel->r_info);
	  int max_pcrel3 = 4;
	  int alignment_glue = 0;

	  GET_RELOC;

	  /* Branches over alignment chunks are problematic, as
	     deleting bytes here makes the branch *further* away.  We
	     can be agressive with branches within this alignment
	     block, but not branches outside it.  */
	  if ((prev_alignment == NULL
	       || symval < (bfd_vma)(sec_start + prev_alignment->r_offset))
	      && (next_alignment == NULL
		  || symval > (bfd_vma)(sec_start + next_alignment->r_offset)))
	    alignment_glue = section_alignment_glue;

	  if (ELF32_R_TYPE(srel[1].r_info) == R_RX_RH_RELAX
	      && srel[1].r_addend & RX_RELAXA_BRA
	      && srel[1].r_offset < irel->r_offset + pcrel)
	    max_pcrel3 ++;

	  newrel = next_smaller_reloc (ELF32_R_TYPE (srel->r_info));

	  /* The values we compare PCREL with are not what you'd
	     expect; they're off by a little to compensate for (1)
	     where the reloc is relative to the insn, and (2) how much
	     the insn is going to change when we relax it.  */

	  /* These we have to decode.  */
	  switch (insn[0])
	    {
	    case 0x04: /* BRA pcdsp:24 */
	      if (-32768 + alignment_glue <= pcrel
		  && pcrel <= 32765 - alignment_glue)
		{
		  insn[0] = 0x38;
		  SNIP (3, 1, newrel);
		  *again = true;
		}
	      break;

	    case 0x38: /* BRA pcdsp:16 */
	      if (-128 + alignment_glue <= pcrel
		  && pcrel <= 127 - alignment_glue)
		{
		  insn[0] = 0x2e;
		  SNIP (2, 1, newrel);
		  *again = true;
		}
	      break;

	    case 0x2e: /* BRA pcdsp:8 */
	      /* Note that there's a risk here of shortening things so
		 much that we no longer fit this reloc; it *should*
		 only happen when you branch across a branch, and that
		 branch also devolves into BRA.S.  "Real" code should
		 be OK.  */
	      if (max_pcrel3 + alignment_glue <= pcrel
		  && pcrel <= 10 - alignment_glue
		  && allow_pcrel3)
		{
		  insn[0] = 0x08;
		  SNIP (1, 1, newrel);
		  move_reloc (irel, srel, -1);
		  *again = true;
		}
	      break;

	    case 0x05: /* BSR pcdsp:24 */
	      if (-32768 + alignment_glue <= pcrel
		  && pcrel <= 32765 - alignment_glue)
		{
		  insn[0] = 0x39;
		  SNIP (1, 1, newrel);
		  *again = true;
		}
	      break;

	    case 0x3a: /* BEQ.W pcdsp:16 */
	    case 0x3b: /* BNE.W pcdsp:16 */
	      if (-128 + alignment_glue <= pcrel
		  && pcrel <= 127 - alignment_glue)
		{
		  insn[0] = 0x20 | (insn[0] & 1);
		  SNIP (1, 1, newrel);
		  *again = true;
		}
	      break;

	    case 0x20: /* BEQ.B pcdsp:8 */
	    case 0x21: /* BNE.B pcdsp:8 */
	      if (max_pcrel3 + alignment_glue <= pcrel
		  && pcrel - alignment_glue <= 10
		  && allow_pcrel3)
		{
		  insn[0] = 0x10 | ((insn[0] & 1) << 3);
		  SNIP (1, 1, newrel);
		  move_reloc (irel, srel, -1);
		  *again = true;
		}
	      break;

	    case 0x16: /* synthetic BNE dsp24 */
	    case 0x1e: /* synthetic BEQ dsp24 */
	      if (-32767 + alignment_glue <= pcrel
		  && pcrel <= 32766 - alignment_glue
		  && insn[1] == 0x04)
		{
		  if (insn[0] == 0x16)
		    insn[0] = 0x3b;
		  else
		    insn[0] = 0x3a;
		  /* We snip out the bytes at the end else the reloc
		     will get moved too, and too much.  */
		  SNIP (3, 2, newrel);
		  move_reloc (irel, srel, -1);
		  *again = true;
		}
	      break;
	    }

	  /* Special case - synthetic conditional branches, pcrel24.
	     Note that EQ and NE have been handled above.  */
	  if ((insn[0] & 0xf0) == 0x20
	      && insn[1] == 0x06
	      && insn[2] == 0x04
	      && srel->r_offset != irel->r_offset + 1
	      && -32767 + alignment_glue <= pcrel
	      && pcrel <= 32766 - alignment_glue)
	    {
	      insn[1] = 0x05;
	      insn[2] = 0x38;
	      SNIP (5, 1, newrel);
	      *again = true;
	    }

	  /* Special case - synthetic conditional branches, pcrel16 */
	  if ((insn[0] & 0xf0) == 0x20
	      && insn[1] == 0x05
	      && insn[2] == 0x38
	      && srel->r_offset != irel->r_offset + 1
	      && -127 + alignment_glue <= pcrel
	      && pcrel <= 126 - alignment_glue)
	    {
	      int cond = (insn[0] & 0x0f) ^ 0x01;

	      insn[0] = 0x20 | cond;
	      /* By moving the reloc first, we avoid having
		 delete_bytes move it also.  */
	      move_reloc (irel, srel, -2);
	      SNIP (2, 3, newrel);
	      *again = true;
	    }
	}

      BFD_ASSERT (nrelocs == 0);

      /* Special case - check MOV.bwl #IMM, dsp[reg] and see if we can
	 use MOV.bwl #uimm:8, dsp:5[r7] format.  This is tricky
	 because it may have one or two relocations.  */
      if ((insn[0] & 0xfc) == 0xf8
	  && (insn[1] & 0x80) == 0x00
	  && (insn[0] & 0x03) != 0x03)
	{
	  int dcode, icode, reg, ioff, dscale, ilen;
	  bfd_vma disp_val = 0;
	  long imm_val = 0;
	  Elf_Internal_Rela * disp_rel = 0;
	  Elf_Internal_Rela * imm_rel = 0;

	  /* Reset this.  */
	  srel = irel;

	  dcode = insn[0] & 0x03;
	  icode = (insn[1] >> 2) & 0x03;
	  reg = (insn[1] >> 4) & 0x0f;

	  ioff = dcode == 1 ? 3 : dcode == 2 ? 4 : 2;

	  /* Figure out what the dispacement is.  */
	  if (dcode == 1 || dcode == 2)
	    {
	      /* There's a displacement.  See if there's a reloc for it.  */
	      if (srel[1].r_offset == irel->r_offset + 2)
		{
		  GET_RELOC;
		  disp_val = symval;
		  disp_rel = srel;
		}
	      else
		{
		  if (dcode == 1)
		    disp_val = insn[2];
		  else
		    {
#if RX_OPCODE_BIG_ENDIAN
		      disp_val = insn[2] * 256 + insn[3];
#else
		      disp_val = insn[2] + insn[3] * 256;
#endif
		    }
		  switch (insn[1] & 3)
		    {
		    case 1:
		      disp_val *= 2;
		      scale = 2;
		      break;
		    case 2:
		      disp_val *= 4;
		      scale = 4;
		      break;
		    }
		}
	    }

	  dscale = scale;

	  /* Figure out what the immediate is.  */
	  if (srel[1].r_offset == irel->r_offset + ioff)
	    {
	      GET_RELOC;
	      imm_val = (long) symval;
	      imm_rel = srel;
	    }
	  else
	    {
	      unsigned char * ip = insn + ioff;

	      switch (icode)
		{
		case 1:
		  /* For byte writes, we don't sign extend.  Makes the math easier later.  */
		  if (scale == 1)
		    imm_val = ip[0];
		  else
		    imm_val = (char) ip[0];
		  break;
		case 2:
#if RX_OPCODE_BIG_ENDIAN
		  imm_val = ((char) ip[0] << 8) | ip[1];
#else
		  imm_val = ((char) ip[1] << 8) | ip[0];
#endif
		  break;
		case 3:
#if RX_OPCODE_BIG_ENDIAN
		  imm_val = ((char) ip[0] << 16) | (ip[1] << 8) | ip[2];
#else
		  imm_val = ((char) ip[2] << 16) | (ip[1] << 8) | ip[0];
#endif
		  break;
		case 0:
#if RX_OPCODE_BIG_ENDIAN
		  imm_val = ((unsigned) ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | ip[3];
#else
		  imm_val = ((unsigned) ip[3] << 24) | (ip[2] << 16) | (ip[1] << 8) | ip[0];
#endif
		  break;
		}
	    }

	  ilen = 2;

	  switch (dcode)
	    {
	    case 1:
	      ilen += 1;
	      break;
	    case 2:
	      ilen += 2;
	      break;
	    }

	  switch (icode)
	    {
	    case 1:
	      ilen += 1;
	      break;
	    case 2:
	      ilen += 2;
	      break;
	    case 3:
	      ilen += 3;
	      break;
	    case 4:
	      ilen += 4;
	      break;
	    }

	  /* The shortcut happens when the immediate is 0..255,
	     register r0 to r7, and displacement (scaled) 0..31.  */

	  if (0 <= imm_val && imm_val <= 255
	      && 0 <= reg && reg <= 7
	      && disp_val / dscale <= 31)
	    {
	      insn[0] = 0x3c | (insn[1] & 0x03);
	      insn[1] = (((disp_val / dscale) << 3) & 0x80) | (reg << 4) | ((disp_val/dscale) & 0x0f);
	      insn[2] = imm_val;

	      if (disp_rel)
		{
		  int newrel = R_RX_NONE;

		  switch (dscale)
		    {
		    case 1:
		      newrel = R_RX_RH_ABS5p8B;
		      break;
		    case 2:
		      newrel = R_RX_RH_ABS5p8W;
		      break;
		    case 4:
		      newrel = R_RX_RH_ABS5p8L;
		      break;
		    }
		  disp_rel->r_info = ELF32_R_INFO (ELF32_R_SYM (disp_rel->r_info), newrel);
		  move_reloc (irel, disp_rel, -1);
		}
	      if (imm_rel)
		{
		  imm_rel->r_info = ELF32_R_INFO (ELF32_R_SYM (imm_rel->r_info), R_RX_DIR8U);
		  move_reloc (disp_rel ? disp_rel : irel,
			      imm_rel,
			      irel->r_offset - imm_rel->r_offset + 2);
		}

	      SNIPNR (3, ilen - 3);
	      *again = true;

	      /* We can't relax this new opcode.  */
	      irel->r_addend = 0;
	    }
	}
    }

  /* We can't reliably relax branches to DIR3U_PCREL unless we know
     whatever they're branching over won't shrink any more.  If we're
     basically done here, do one more pass just for branches - but
     don't request a pass after that one!  */
  if (!*again && !allow_pcrel3)
    {
      bool ignored;

      elf32_rx_relax_section (abfd, sec, link_info, &ignored, true);
    }

  return true;

 error_return:
  free (free_contents);

  if (shndx_buf != NULL)
    {
      shndx_hdr->contents = NULL;
      free (shndx_buf);
    }

  free (free_intsyms);

  return false;
}

static bool
elf32_rx_relax_section_wrapper (bfd *abfd,
				asection *sec,
				struct bfd_link_info *link_info,
				bool *again)
{
  return elf32_rx_relax_section (abfd, sec, link_info, again, false);
}

/* Function to set the ELF flag bits.  */

static bool
rx_elf_set_private_flags (bfd * abfd, flagword flags)
{
  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

static bool no_warn_mismatch = false;
static bool ignore_lma = true;

void bfd_elf32_rx_set_target_flags (bool, bool);

void
bfd_elf32_rx_set_target_flags (bool user_no_warn_mismatch,
			       bool user_ignore_lma)
{
  no_warn_mismatch = user_no_warn_mismatch;
  ignore_lma = user_ignore_lma;
}

/* Converts FLAGS into a descriptive string.
   Returns a static pointer.  */

static const char *
describe_flags (flagword flags, char *buf)
{
  buf[0] = 0;

  if (flags & E_FLAG_RX_64BIT_DOUBLES)
    strcat (buf, "64-bit doubles");
  else
    strcat (buf, "32-bit doubles");

  if (flags & E_FLAG_RX_DSP)
    strcat (buf, ", dsp");
  else
    strcat (buf, ", no dsp");

  if (flags & E_FLAG_RX_PID)
    strcat (buf, ", pid");
  else
    strcat (buf, ", no pid");

  if (flags & E_FLAG_RX_ABI)
    strcat (buf, ", RX ABI");
  else
    strcat (buf, ", GCC ABI");

  if (flags & E_FLAG_RX_SINSNS_SET)
    strcat (buf, flags & E_FLAG_RX_SINSNS_YES ? ", uses String instructions" : ", bans String instructions");

  return buf;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
rx_elf_merge_private_bfd_data (bfd * ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword old_flags;
  flagword new_flags;
  bool error = false;

  new_flags = elf_elfheader (ibfd)->e_flags;
  old_flags = elf_elfheader (obfd)->e_flags;

  if (!elf_flags_init (obfd))
    {
      /* First call, no flags set.  */
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = new_flags;
    }
  else if (old_flags != new_flags)
    {
      flagword known_flags;

      if (old_flags & E_FLAG_RX_SINSNS_SET)
	{
	  if ((new_flags & E_FLAG_RX_SINSNS_SET) == 0)
	    {
	      new_flags &= ~ E_FLAG_RX_SINSNS_MASK;
	      new_flags |= (old_flags & E_FLAG_RX_SINSNS_MASK);
	    }
	}
      else if (new_flags & E_FLAG_RX_SINSNS_SET)
	{
	  old_flags &= ~ E_FLAG_RX_SINSNS_MASK;
	  old_flags |= (new_flags & E_FLAG_RX_SINSNS_MASK);
	}

      known_flags = E_FLAG_RX_ABI | E_FLAG_RX_64BIT_DOUBLES
	| E_FLAG_RX_DSP | E_FLAG_RX_PID | E_FLAG_RX_SINSNS_MASK;

      if ((old_flags ^ new_flags) & known_flags)
	{
	  /* Only complain if flag bits we care about do not match.
	     Other bits may be set, since older binaries did use some
	     deprecated flags.  */
	  if (no_warn_mismatch)
	    {
	      elf_elfheader (obfd)->e_flags = (new_flags | old_flags) & known_flags;
	    }
	  else
	    {
	      char buf[128];

	      _bfd_error_handler (_("there is a conflict merging the"
				    " ELF header flags from %pB"),
				  ibfd);
	      _bfd_error_handler (_("  the input  file's flags: %s"),
				  describe_flags (new_flags, buf));
	      _bfd_error_handler (_("  the output file's flags: %s"),
				  describe_flags (old_flags, buf));
	      error = true;
	    }
	}
      else
	elf_elfheader (obfd)->e_flags = new_flags & known_flags;
    }

  if (error)
    bfd_set_error (bfd_error_bad_value);

  return !error;
}

static bool
rx_elf_print_private_bfd_data (bfd * abfd, void * ptr)
{
  FILE * file = (FILE *) ptr;
  flagword flags;
  char buf[128];

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  flags = elf_elfheader (abfd)->e_flags;
  fprintf (file, _("private flags = 0x%lx:"), (long) flags);

  fprintf (file, "%s", describe_flags (flags, buf));
  return true;
}

/* Return the MACH for an e_flags value.  */

static int
elf32_rx_machine (bfd * abfd ATTRIBUTE_UNUSED)
{
#if 0 /* FIXME: EF_RX_CPU_MASK collides with E_FLAG_RX_...
	 Need to sort out how these flag bits are used.
	 For now we assume that the flags are OK.  */
  if ((elf_elfheader (abfd)->e_flags & EF_RX_CPU_MASK) == EF_RX_CPU_RX)
#endif
    if ((elf_elfheader (abfd)->e_flags & E_FLAG_RX_V2))
      return bfd_mach_rx_v2;
    else if ((elf_elfheader (abfd)->e_flags & E_FLAG_RX_V3))
      return bfd_mach_rx_v3;
    else
      return bfd_mach_rx;

  return 0;
}

static bool
rx_elf_object_p (bfd * abfd)
{
  int i;
  unsigned int u;
  Elf_Internal_Phdr *phdr = elf_tdata (abfd)->phdr;
  Elf_Internal_Ehdr *ehdr = elf_elfheader (abfd);
  int nphdrs = ehdr->e_phnum;
  sec_ptr bsec;
  static int saw_be = false;
  bfd_vma end_phdroff;

  /* We never want to automatically choose the non-swapping big-endian
     target.  The user can only get that explicitly, such as with -I
     and objcopy.  */
  if (abfd->xvec == &rx_elf32_be_ns_vec
      && abfd->target_defaulted)
    return false;

  /* BFD->target_defaulted is not set to TRUE when a target is chosen
     as a fallback, so we check for "scanning" to know when to stop
     using the non-swapping target.  */
  if (abfd->xvec == &rx_elf32_be_ns_vec
      && saw_be)
    return false;
  if (abfd->xvec == &rx_elf32_be_vec)
    saw_be = true;

  bfd_default_set_arch_mach (abfd, bfd_arch_rx,
			     elf32_rx_machine (abfd));

  /* For each PHDR in the object, we must find some section that
     corresponds (based on matching file offsets) and use its VMA
     information to reconstruct the p_vaddr field we clobbered when we
     wrote it out.  */
  /* If PT_LOAD headers include the ELF file header or program headers
     then the PT_LOAD header does not start with some section contents.
     Making adjustments based on the difference between sh_offset and
     p_offset is nonsense in such cases.  Exclude them.  Note that
     since standard linker scripts for RX do not use SIZEOF_HEADERS,
     the linker won't normally create PT_LOAD segments covering the
     headers so this is mainly for passing the ld testsuite.
     FIXME.  Why are we looking at non-PT_LOAD headers here?  */
  end_phdroff = ehdr->e_ehsize;
  if (ehdr->e_phoff != 0)
    end_phdroff = ehdr->e_phoff + nphdrs * ehdr->e_phentsize;
  for (i=0; i<nphdrs; i++)
    {
      for (u=0; u<elf_tdata(abfd)->num_elf_sections; u++)
	{
	  Elf_Internal_Shdr *sec = elf_tdata(abfd)->elf_sect_ptr[u];

	  if (phdr[i].p_filesz
	      && phdr[i].p_offset >= end_phdroff
	      && phdr[i].p_offset <= (bfd_vma) sec->sh_offset
	      && sec->sh_size > 0
	      && sec->sh_type != SHT_NOBITS
	      && (bfd_vma)sec->sh_offset <= phdr[i].p_offset + (phdr[i].p_filesz - 1))
	    {
	      /* Found one!  The difference between the two addresses,
		 plus the difference between the two file offsets, is
		 enough information to reconstruct the lma.  */

	      /* Example where they aren't:
		 PHDR[1] = lma fffc0100 offset 00002010 size 00000100
		 SEC[6]  = vma 00000050 offset 00002050 size 00000040

		 The correct LMA for the section is fffc0140 + (2050-2010).
	      */

	      phdr[i].p_vaddr = sec->sh_addr + (sec->sh_offset - phdr[i].p_offset);
	      break;
	    }
	}

      /* We must update the bfd sections as well, so we don't stop
	 with one match.  */
      bsec = abfd->sections;
      while (bsec)
	{
	  if (phdr[i].p_filesz
	      && phdr[i].p_vaddr <= bsec->vma
	      && bsec->vma <= phdr[i].p_vaddr + (phdr[i].p_filesz - 1))
	    {
	      bsec->lma = phdr[i].p_paddr + (bsec->vma - phdr[i].p_vaddr);
	    }
	  bsec = bsec->next;
	}
    }

  return true;
}

static bool
rx_linux_object_p (bfd * abfd)
{
  bfd_default_set_arch_mach (abfd, bfd_arch_rx, elf32_rx_machine (abfd));
  return true;
}
 

#ifdef DEBUG
void
rx_dump_symtab (bfd * abfd, void * internal_syms, void * external_syms)
{
  size_t locsymcount;
  Elf_Internal_Sym * isymbuf;
  Elf_Internal_Sym * isymend;
  Elf_Internal_Sym * isym;
  Elf_Internal_Shdr * symtab_hdr;
  char * st_info_str;
  char * st_info_stb_str;
  char * st_other_str;
  char * st_shndx_str;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  locsymcount = symtab_hdr->sh_size / get_elf_backend_data (abfd)->s->sizeof_sym;
  if (!internal_syms)
    isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr,
				    symtab_hdr->sh_info, 0,
				    internal_syms, external_syms, NULL);
  else
    isymbuf = internal_syms;
  isymend = isymbuf + locsymcount;

  for (isym = isymbuf ; isym < isymend ; isym++)
    {
      switch (ELF_ST_TYPE (isym->st_info))
	{
	case STT_FUNC: st_info_str = "STT_FUNC"; break;
	case STT_SECTION: st_info_str = "STT_SECTION"; break;
	case STT_FILE: st_info_str = "STT_FILE"; break;
	case STT_OBJECT: st_info_str = "STT_OBJECT"; break;
	case STT_TLS: st_info_str = "STT_TLS"; break;
	default: st_info_str = "";
	}
      switch (ELF_ST_BIND (isym->st_info))
	{
	case STB_LOCAL: st_info_stb_str = "STB_LOCAL"; break;
	case STB_GLOBAL: st_info_stb_str = "STB_GLOBAL"; break;
	default: st_info_stb_str = "";
	}
      switch (ELF_ST_VISIBILITY (isym->st_other))
	{
	case STV_DEFAULT: st_other_str = "STV_DEFAULT"; break;
	case STV_INTERNAL: st_other_str = "STV_INTERNAL"; break;
	case STV_PROTECTED: st_other_str = "STV_PROTECTED"; break;
	default: st_other_str = "";
	}
      switch (isym->st_shndx)
	{
	case SHN_ABS: st_shndx_str = "SHN_ABS"; break;
	case SHN_COMMON: st_shndx_str = "SHN_COMMON"; break;
	case SHN_UNDEF: st_shndx_str = "SHN_UNDEF"; break;
	default: st_shndx_str = "";
	}

      printf ("isym = %p st_value = %lx st_size = %lx st_name = (%lu) %s "
	      "st_info = (%d) %s %s st_other = (%d) %s st_shndx = (%d) %s\n",
	      isym,
	      (unsigned long) isym->st_value,
	      (unsigned long) isym->st_size,
	      isym->st_name,
	      bfd_elf_string_from_elf_section (abfd, symtab_hdr->sh_link,
					       isym->st_name),
	      isym->st_info, st_info_str, st_info_stb_str,
	      isym->st_other, st_other_str,
	      isym->st_shndx, st_shndx_str);
    }
}

char *
rx_get_reloc (long reloc)
{
  if (0 <= reloc && reloc < R_RX_max)
    return rx_elf_howto_table[reloc].name;
  return "";
}
#endif /* DEBUG */


/* We must take care to keep the on-disk copy of any code sections
   that are fully linked swapped if the target is big endian, to match
   the Renesas tools.  */

/* The rule is: big endian object that are final-link executables,
   have code sections stored with 32-bit words swapped relative to
   what you'd get by default.  */

static bool
rx_get_section_contents (bfd *	       abfd,
			 sec_ptr       section,
			 void *	       location,
			 file_ptr      offset,
			 bfd_size_type count)
{
  int exec = (abfd->flags & EXEC_P) ? 1 : 0;
  int s_code = (section->flags & SEC_CODE) ? 1 : 0;
  bool rv;

#ifdef DJDEBUG
  fprintf (stderr, "dj: get %ld %ld from %s  %s e%d sc%d  %08lx:%08lx\n",
	   (long) offset, (long) count, section->name,
	   bfd_big_endian(abfd) ? "be" : "le",
	   exec, s_code, (long unsigned) section->filepos,
	   (long unsigned) offset);
#endif

  if (exec && s_code && bfd_big_endian (abfd))
    {
      char * cloc = (char *) location;
      bfd_size_type cnt, end_cnt;

      rv = true;

      /* Fetch and swap unaligned bytes at the beginning.  */
      if (offset % 4)
	{
	  char buf[4];

	  rv = _bfd_generic_get_section_contents (abfd, section, buf,
						  (offset & -4), 4);
	  if (!rv)
	    return false;

	  bfd_putb32 (bfd_getl32 (buf), buf);

	  cnt = 4 - (offset % 4);
	  if (cnt > count)
	    cnt = count;

	  memcpy (location, buf + (offset % 4), cnt);

	  count -= cnt;
	  offset += cnt;
	  cloc += count;
	}

      end_cnt = count % 4;

      /* Fetch and swap the middle bytes.  */
      if (count >= 4)
	{
	  rv = _bfd_generic_get_section_contents (abfd, section, cloc, offset,
						  count - end_cnt);
	  if (!rv)
	    return false;

	  for (cnt = count; cnt >= 4; cnt -= 4, cloc += 4)
	    bfd_putb32 (bfd_getl32 (cloc), cloc);
	}

      /* Fetch and swap the end bytes.  */
      if (end_cnt > 0)
	{
	  char buf[4];

	  /* Fetch the end bytes.  */
	  rv = _bfd_generic_get_section_contents (abfd, section, buf,
						  offset + count - end_cnt, 4);
	  if (!rv)
	    return false;

	  bfd_putb32 (bfd_getl32 (buf), buf);
	  memcpy (cloc, buf, end_cnt);
	}
    }
  else
    rv = _bfd_generic_get_section_contents (abfd, section, location, offset, count);

  return rv;
}

#ifdef DJDEBUG
static bool
rx2_set_section_contents (bfd *	       abfd,
			 sec_ptr       section,
			 const void *  location,
			 file_ptr      offset,
			 bfd_size_type count)
{
  bfd_size_type i;

  fprintf (stderr, "   set sec %s %08x loc %p offset %#x count %#x\n",
	   section->name, (unsigned) section->vma, location, (int) offset, (int) count);
  for (i = 0; i < count; i++)
    {
      if (i % 16 == 0 && i > 0)
	fprintf (stderr, "\n");

      if (i % 16  && i % 4 == 0)
	fprintf (stderr, " ");

      if (i % 16 == 0)
	fprintf (stderr, "      %08x:", (int) (section->vma + offset + i));

      fprintf (stderr, " %02x", ((unsigned char *) location)[i]);
    }
  fprintf (stderr, "\n");

  return _bfd_elf_set_section_contents (abfd, section, location, offset, count);
}
#define _bfd_elf_set_section_contents rx2_set_section_contents
#endif

static bool
rx_set_section_contents (bfd *	       abfd,
			 sec_ptr       section,
			 const void *  location,
			 file_ptr      offset,
			 bfd_size_type count)
{
  bool exec = (abfd->flags & EXEC_P) != 0;
  bool s_code = (section->flags & SEC_CODE) != 0;
  bool rv;
  char * swapped_data = NULL;
  bfd_size_type i;
  bfd_vma caddr = section->vma + offset;
  file_ptr faddr = 0;
  bfd_size_type scount;

#ifdef DJDEBUG
  bfd_size_type i;

  fprintf (stderr, "\ndj: set %ld %ld to %s  %s e%d sc%d\n",
	   (long) offset, (long) count, section->name,
	   bfd_big_endian (abfd) ? "be" : "le",
	   exec, s_code);

  for (i = 0; i < count; i++)
    {
      int a = section->vma + offset + i;

      if (a % 16 == 0 && a > 0)
	fprintf (stderr, "\n");

      if (a % 16  && a % 4 == 0)
	fprintf (stderr, " ");

      if (a % 16 == 0 || i == 0)
	fprintf (stderr, "      %08x:", (int) (section->vma + offset + i));

      fprintf (stderr, " %02x", ((unsigned char *) location)[i]);
    }

  fprintf (stderr, "\n");
#endif

  if (! exec || ! s_code || ! bfd_big_endian (abfd))
    return _bfd_elf_set_section_contents (abfd, section, location, offset, count);

  while (count > 0 && caddr > 0 && caddr % 4)
    {
      switch (caddr % 4)
	{
	case 0: faddr = offset + 3; break;
	case 1: faddr = offset + 1; break;
	case 2: faddr = offset - 1; break;
	case 3: faddr = offset - 3; break;
	}

      rv = _bfd_elf_set_section_contents (abfd, section, location, faddr, 1);
      if (! rv)
	return rv;

      location = (bfd_byte *) location + 1;
      offset ++;
      count --;
      caddr ++;
    }

  scount = (int)(count / 4) * 4;
  if (scount > 0)
    {
      char * cloc = (char *) location;

      swapped_data = (char *) bfd_alloc (abfd, count);
      if (swapped_data == NULL)
	return false;

      for (i = 0; i < count; i += 4)
	{
	  bfd_vma v = bfd_getl32 (cloc + i);
	  bfd_putb32 (v, swapped_data + i);
	}

      rv = _bfd_elf_set_section_contents (abfd, section, swapped_data, offset, scount);

      if (!rv)
	return rv;
    }

  count -= scount;
  location = (bfd_byte *) location + scount;
  offset += scount;

  if (count > 0)
    {
      caddr = section->vma + offset;
      while (count > 0)
	{
	  switch (caddr % 4)
	    {
	    case 0: faddr = offset + 3; break;
	    case 1: faddr = offset + 1; break;
	    case 2: faddr = offset - 1; break;
	    case 3: faddr = offset - 3; break;
	    }
	  rv = _bfd_elf_set_section_contents (abfd, section, location, faddr, 1);
	  if (! rv)
	    return rv;

	  location = (bfd_byte *) location + 1;
	  offset ++;
	  count --;
	  caddr ++;
	}
    }

  return true;
}

static bool
rx_final_link (bfd * abfd, struct bfd_link_info * info)
{
  asection * o;

  for (o = abfd->sections; o != NULL; o = o->next)
    {
#ifdef DJDEBUG
      fprintf (stderr, "sec %s fl %x vma %lx lma %lx size %lx raw %lx\n",
	       o->name, o->flags, o->vma, o->lma, o->size, o->rawsize);
#endif
      if (o->flags & SEC_CODE
	  && bfd_big_endian (abfd)
	  && o->size % 4)
	{
#ifdef DJDEBUG
	  fprintf (stderr, "adjusting...\n");
#endif
	  o->size += 4 - (o->size % 4);
	}
    }

  return bfd_elf_final_link (abfd, info);
}

static bool
elf32_rx_modify_headers (bfd *abfd, struct bfd_link_info *info)
{
  const struct elf_backend_data * bed;
  struct elf_obj_tdata * tdata;
  Elf_Internal_Phdr * phdr;
  unsigned int count;
  unsigned int i;

  bed = get_elf_backend_data (abfd);
  tdata = elf_tdata (abfd);
  phdr = tdata->phdr;
  count = elf_program_header_size (abfd) / bed->s->sizeof_phdr;

  if (ignore_lma)
    for (i = count; i-- != 0;)
      if (phdr[i].p_type == PT_LOAD)
	{
	  /* The Renesas tools expect p_paddr to be zero.  However,
	     there is no other way to store the writable data in ROM for
	     startup initialization.  So, we let the linker *think*
	     we're using paddr and vaddr the "usual" way, but at the
	     last minute we move the paddr into the vaddr (which is what
	     the simulator uses) and zero out paddr.  Note that this
	     does not affect the section headers, just the program
	     headers.  We hope.  */
	  phdr[i].p_vaddr = phdr[i].p_paddr;
#if 0	  /* If we zero out p_paddr, then the LMA in the section table
	     becomes wrong.  */
	  phdr[i].p_paddr = 0;
#endif
	}

  return _bfd_elf_modify_headers (abfd, info);
}

/* The default literal sections should always be marked as "code" (i.e.,
   SHF_EXECINSTR).  This is particularly important for big-endian mode
   when we do not want their contents byte reversed.  */
static const struct bfd_elf_special_section elf32_rx_special_sections[] =
{
  { STRING_COMMA_LEN (".init_array"),	 0, SHT_INIT_ARRAY, SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".fini_array"),	 0, SHT_FINI_ARRAY, SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".preinit_array"), 0, SHT_PREINIT_ARRAY, SHF_ALLOC + SHF_EXECINSTR },
  { NULL,			 0,	 0, 0,		  0 }
};

typedef struct {
  bfd *abfd;
  struct bfd_link_info *info;
  bfd_vma table_start;
  int table_size;
  bfd_vma *table_handlers;
  bfd_vma table_default_handler;
  struct bfd_link_hash_entry **table_entries;
  struct bfd_link_hash_entry *table_default_entry;
  FILE *mapfile;
} RX_Table_Info;

static bool
rx_table_find (struct bfd_hash_entry *vent, void *vinfo)
{
  RX_Table_Info *info = (RX_Table_Info *)vinfo;
  struct bfd_link_hash_entry *ent = (struct bfd_link_hash_entry *)vent;
  const char *name; /* of the symbol we've found */
  asection *sec;
  struct bfd *abfd;
  int idx;
  const char *tname; /* name of the table */
  bfd_vma start_addr, end_addr;
  char *buf;
  struct bfd_link_hash_entry * h;

  /* We're looking for globally defined symbols of the form
     $tablestart$<NAME>.  */
  if (ent->type != bfd_link_hash_defined
      && ent->type != bfd_link_hash_defweak)
    return true;

  name = ent->root.string;
  sec = ent->u.def.section;
  abfd = sec->owner;

  if (!startswith (name, "$tablestart$"))
    return true;

  sec->flags |= SEC_KEEP;

  tname = name + 12;

  start_addr = ent->u.def.value;

  /* At this point, we can't build the table but we can (and must)
     find all the related symbols and mark their sections as SEC_KEEP
     so we don't garbage collect them.  */

  buf = (char *) bfd_malloc (12 + 10 + strlen (tname));
  if (buf == NULL)
    return false;

  sprintf (buf, "$tableend$%s", tname);
  h = bfd_link_hash_lookup (info->info->hash, buf, false, false, true);
  if (!h || (h->type != bfd_link_hash_defined
	     && h->type != bfd_link_hash_defweak))
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB:%pA: table %s missing corresponding %s"),
			  abfd, sec, name, buf);
      return true;
    }

  if (h->u.def.section != ent->u.def.section)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB:%pA: %s and %s must be in the same input section"),
			  h->u.def.section->owner, h->u.def.section,
			  name, buf);
      return true;
    }

  end_addr = h->u.def.value;

  sprintf (buf, "$tableentry$default$%s", tname);
  h = bfd_link_hash_lookup (info->info->hash, buf, false, false, true);
  if (h && (h->type == bfd_link_hash_defined
	    || h->type == bfd_link_hash_defweak))
    {
      h->u.def.section->flags |= SEC_KEEP;
    }

  for (idx = 0; idx < (int) (end_addr - start_addr) / 4; idx ++)
    {
      sprintf (buf, "$tableentry$%d$%s", idx, tname);
      h = bfd_link_hash_lookup (info->info->hash, buf, false, false, true);
      if (h && (h->type == bfd_link_hash_defined
		|| h->type == bfd_link_hash_defweak))
	{
	  h->u.def.section->flags |= SEC_KEEP;
	}
    }

  /* Return TRUE to keep scanning, FALSE to end the traversal.  */
  return true;
}

/* We need to check for table entry symbols and build the tables, and
   we need to do it before the linker does garbage collection.  This function is
   called once per input object file.  */
static bool
rx_check_directives
    (bfd *		       abfd ATTRIBUTE_UNUSED,
     struct bfd_link_info *    info ATTRIBUTE_UNUSED)
{
  RX_Table_Info stuff;

  stuff.abfd = abfd;
  stuff.info = info;
  bfd_hash_traverse (&(info->hash->table), rx_table_find, &stuff);

  return true;
}


static bool
rx_table_map_2 (struct bfd_hash_entry *vent, void *vinfo)
{
  RX_Table_Info *info = (RX_Table_Info *)vinfo;
  struct bfd_link_hash_entry *ent = (struct bfd_link_hash_entry *)vent;
  int idx;
  const char *name;
  bfd_vma addr;

  /* See if the symbol ENT has an address listed in the table, and
     isn't a debug/special symbol.  If so, put it in the table.  */

  if (ent->type != bfd_link_hash_defined
      && ent->type != bfd_link_hash_defweak)
    return true;

  name = ent->root.string;

  if (name[0] == '$' || name[0] == '.' || name[0] < ' ')
    return true;

  addr = (ent->u.def.value
	  + ent->u.def.section->output_section->vma
	  + ent->u.def.section->output_offset);

  for (idx = 0; idx < info->table_size; idx ++)
    if (addr == info->table_handlers[idx])
      info->table_entries[idx] = ent;

  if (addr == info->table_default_handler)
    info->table_default_entry = ent;

  return true;
}

static bool
rx_table_map (struct bfd_hash_entry *vent, void *vinfo)
{
  RX_Table_Info *info = (RX_Table_Info *)vinfo;
  struct bfd_link_hash_entry *ent = (struct bfd_link_hash_entry *)vent;
  const char *name; /* of the symbol we've found */
  int idx;
  const char *tname; /* name of the table */
  bfd_vma start_addr, end_addr;
  char *buf;
  struct bfd_link_hash_entry * h;
  int need_elipses;

  /* We're looking for globally defined symbols of the form
     $tablestart$<NAME>.  */
  if (ent->type != bfd_link_hash_defined
      && ent->type != bfd_link_hash_defweak)
    return true;

  name = ent->root.string;

  if (!startswith (name, "$tablestart$"))
    return true;

  tname = name + 12;
  start_addr = (ent->u.def.value
		+ ent->u.def.section->output_section->vma
		+ ent->u.def.section->output_offset);

  buf = (char *) bfd_malloc (12 + 10 + strlen (tname));
  if (buf == NULL)
    return false;

  sprintf (buf, "$tableend$%s", tname);
  end_addr = get_symbol_value_maybe (buf, info->info);

  sprintf (buf, "$tableentry$default$%s", tname);
  h = bfd_link_hash_lookup (info->info->hash, buf, false, false, true);
  if (h)
    {
      info->table_default_handler = (h->u.def.value
				     + h->u.def.section->output_section->vma
				     + h->u.def.section->output_offset);
    }
  else
    /* Zero is a valid handler address!  */
    info->table_default_handler = (bfd_vma) (-1);
  info->table_default_entry = NULL;

  info->table_start = start_addr;
  info->table_size = (int) (end_addr - start_addr) / 4;
  info->table_handlers = (bfd_vma *)
    bfd_malloc (info->table_size * sizeof (bfd_vma));
  if (info->table_handlers == NULL)
    {
      free (buf);
      return false;
    }
  info->table_entries = (struct bfd_link_hash_entry **)
    bfd_malloc (info->table_size * sizeof (struct bfd_link_hash_entry));
  if (info->table_entries == NULL)
    {
      free (info->table_handlers);
      free (buf);
      return false;
    }

  for (idx = 0; idx < (int) (end_addr - start_addr) / 4; idx ++)
    {
      sprintf (buf, "$tableentry$%d$%s", idx, tname);
      h = bfd_link_hash_lookup (info->info->hash, buf, false, false, true);
      if (h && (h->type == bfd_link_hash_defined
		|| h->type == bfd_link_hash_defweak))
	{
	  info->table_handlers[idx] = (h->u.def.value
				       + h->u.def.section->output_section->vma
				       + h->u.def.section->output_offset);
	}
      else
	info->table_handlers[idx] = info->table_default_handler;
      info->table_entries[idx] = NULL;
    }

  free (buf);

  bfd_hash_traverse (&(info->info->hash->table), rx_table_map_2, info);

  fprintf (info->mapfile,
	   "\nRX Vector Table: %s has %d entries at 0x%08" PRIx64 "\n\n",
	   tname, info->table_size, (uint64_t) start_addr);

  if (info->table_default_entry)
    fprintf (info->mapfile, "  default handler is: %s at 0x%08" PRIx64 "\n",
	     info->table_default_entry->root.string,
	     (uint64_t) info->table_default_handler);
  else if (info->table_default_handler != (bfd_vma)(-1))
    fprintf (info->mapfile, "  default handler is at 0x%08" PRIx64 "\n",
	     (uint64_t) info->table_default_handler);
  else
    fprintf (info->mapfile, "  no default handler\n");

  need_elipses = 1;
  for (idx = 0; idx < info->table_size; idx ++)
    {
      if (info->table_handlers[idx] == info->table_default_handler)
	{
	  if (need_elipses)
	    fprintf (info->mapfile, "  . . .\n");
	  need_elipses = 0;
	  continue;
	}
      need_elipses = 1;

      fprintf (info->mapfile,
	       "  0x%08" PRIx64 " [%3d] ", (uint64_t) start_addr + 4 * idx, idx);

      if (info->table_handlers[idx] == (bfd_vma) (-1))
	fprintf (info->mapfile, "(no handler found)\n");

      else if (info->table_handlers[idx] == info->table_default_handler)
	{
	  if (info->table_default_entry)
	    fprintf (info->mapfile, "(default)\n");
	  else
	    fprintf (info->mapfile, "(default)\n");
	}

      else if (info->table_entries[idx])
	{
	  fprintf (info->mapfile, "0x%08" PRIx64 " %s\n",
		   (uint64_t) info->table_handlers[idx],
		   info->table_entries[idx]->root.string);
	}

      else
	{
	  fprintf (info->mapfile, "0x%08" PRIx64 " ???\n",
		   (uint64_t) info->table_handlers[idx]);
	}
    }
  if (need_elipses)
    fprintf (info->mapfile, "  . . .\n");

  return true;
}

void
rx_additional_link_map_text (bfd *obfd, struct bfd_link_info *info, FILE *mapfile)
{
  /* We scan the symbol table looking for $tableentry$'s, and for
     each, try to deduce which handlers go with which entries.  */

  RX_Table_Info stuff;

  stuff.abfd = obfd;
  stuff.info = info;
  stuff.mapfile = mapfile;
  bfd_hash_traverse (&(info->hash->table), rx_table_map, &stuff);
}


#define ELF_ARCH		bfd_arch_rx
#define ELF_MACHINE_CODE	EM_RX
#define ELF_MAXPAGESIZE		0x1000

#define TARGET_BIG_SYM		rx_elf32_be_vec
#define TARGET_BIG_NAME		"elf32-rx-be"

#define TARGET_LITTLE_SYM	rx_elf32_le_vec
#define TARGET_LITTLE_NAME	"elf32-rx-le"

#define elf_info_to_howto_rel			NULL
#define elf_info_to_howto			rx_info_to_howto_rela
#define elf_backend_object_p			rx_elf_object_p
#define elf_backend_relocate_section		rx_elf_relocate_section
#define elf_symbol_leading_char			('_')
#define elf_backend_can_gc_sections		1
#define elf_backend_modify_headers		elf32_rx_modify_headers

#define bfd_elf32_bfd_reloc_type_lookup		rx_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		rx_reloc_name_lookup
#define bfd_elf32_bfd_set_private_flags		rx_elf_set_private_flags
#define bfd_elf32_bfd_merge_private_bfd_data	rx_elf_merge_private_bfd_data
#define bfd_elf32_bfd_print_private_bfd_data	rx_elf_print_private_bfd_data
#define bfd_elf32_get_section_contents		rx_get_section_contents
#define bfd_elf32_set_section_contents		rx_set_section_contents
#define bfd_elf32_bfd_final_link		rx_final_link
#define bfd_elf32_bfd_relax_section		elf32_rx_relax_section_wrapper
#define elf_backend_special_sections		elf32_rx_special_sections
#define elf_backend_check_directives		rx_check_directives

#include "elf32-target.h"

/* We define a second big-endian target that doesn't have the custom
   section get/set hooks, for times when we want to preserve the
   pre-swapped .text sections (like objcopy).  */

#undef	TARGET_BIG_SYM
#define TARGET_BIG_SYM		rx_elf32_be_ns_vec
#undef	TARGET_BIG_NAME
#define TARGET_BIG_NAME		"elf32-rx-be-ns"
#undef	TARGET_LITTLE_SYM

#undef bfd_elf32_get_section_contents
#undef bfd_elf32_set_section_contents

#undef	elf32_bed
#define elf32_bed				elf32_rx_be_ns_bed

#include "elf32-target.h"

#undef	TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM	rx_elf32_linux_le_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME	"elf32-rx-linux"
#undef  TARGET_BIG_SYM
#undef  TARGET_BIG_NAME

#undef  elf_backend_object_p
#define elf_backend_object_p			rx_linux_object_p
#undef  elf_symbol_leading_char
#undef	elf32_bed
#define	elf32_bed				elf32_rx_le_linux_bed

#include "elf32-target.h"
