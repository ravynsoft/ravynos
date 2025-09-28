/* Renesas RL78 specific support for 32-bit ELF.
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

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/rl78.h"
#include "libiberty.h"

#define valid_16bit_address(v) ((v) <= 0x0ffff || (v) >= 0xf0000)

#define RL78REL(n,sz,bit,mask,shift,complain,pcrel) \
  HOWTO (R_RL78_##n, shift, sz, bit, pcrel, 0, complain_overflow_ ## complain, \
	 bfd_elf_generic_reloc, "R_RL78_" #n, false, 0, mask, false)

static bfd_reloc_status_type rl78_special_reloc (bfd *, arelent *, asymbol *, void *,
						 asection *, bfd *, char **);

#define RL78_OP_REL(n,sz,bit,mask,shift,complain,pcrel)			\
  HOWTO (R_RL78_##n, shift, sz, bit, pcrel, 0, complain_overflow_ ## complain, \
	 rl78_special_reloc, "R_RL78_" #n, false, 0, mask, false)

/* Note that the relocations around 0x7f are internal to this file;
   feel free to move them as needed to avoid conflicts with published
   relocation numbers.  */

static reloc_howto_type rl78_elf_howto_table [] =
{
  RL78REL (NONE,	 0,  0, 0,          0, dont,     false),
  RL78REL (DIR32,	 4, 32, 0xffffffff, 0, dont,     false),
  RL78REL (DIR24S,	 4, 24, 0xffffff,   0, signed,   false),
  RL78REL (DIR16,	 2, 16, 0xffff,     0, bitfield, false),
  RL78REL (DIR16U,	 2, 16, 0xffff,     0, unsigned, false),
  RL78REL (DIR16S,	 2, 16, 0xffff,     0, bitfield, false),
  RL78REL (DIR8,	 1,  8, 0xff,       0, dont,     false),
  RL78REL (DIR8U,	 1,  8, 0xff,       0, unsigned, false),
  RL78REL (DIR8S,	 1,  8, 0xff,       0, bitfield, false),
  RL78REL (DIR24S_PCREL, 4, 24, 0xffffff,   0, signed,   true),
  RL78REL (DIR16S_PCREL, 2, 16, 0xffff,     0, signed,   true),
  RL78REL (DIR8S_PCREL,	 1,  8, 0xff,       0, signed,   true),
  RL78REL (DIR16UL,	 2, 16, 0xffff,     2, unsigned, false),
  RL78REL (DIR16UW,	 2, 16, 0xffff,     1, unsigned, false),
  RL78REL (DIR8UL,	 1,  8, 0xff,       2, unsigned, false),
  RL78REL (DIR8UW,	 1,  8, 0xff,       1, unsigned, false),
  RL78REL (DIR32_REV,	 4, 32, 0xffffffff, 0, dont,     false),
  RL78REL (DIR16_REV,	 2, 16, 0xffff,     0, bitfield, false),
  RL78REL (DIR3U_PCREL,	 1,  3, 0x7,        0, unsigned, true),

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

  EMPTY_HOWTO (0x20),
  EMPTY_HOWTO (0x21),
  EMPTY_HOWTO (0x22),
  EMPTY_HOWTO (0x23),
  EMPTY_HOWTO (0x24),
  EMPTY_HOWTO (0x25),
  EMPTY_HOWTO (0x26),
  EMPTY_HOWTO (0x27),
  EMPTY_HOWTO (0x28),
  EMPTY_HOWTO (0x29),
  EMPTY_HOWTO (0x2a),
  EMPTY_HOWTO (0x2b),
  EMPTY_HOWTO (0x2c),

  RL78REL (RH_RELAX,	 0,  0, 0,          0, dont,	 false),
  RL78REL (RH_SFR,	 1,  8, 0xff,       0, unsigned, false),
  RL78REL (RH_SADDR,	 1,  8, 0xff,       0, unsigned, false),

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

  RL78_OP_REL (ABS32,	     4, 32, 0xffffffff, 0, dont,	false),
  RL78_OP_REL (ABS24S,	     4, 24, 0xffffff,   0, signed,	false),
  RL78_OP_REL (ABS16,	     2, 16, 0xffff,     0, bitfield,	false),
  RL78_OP_REL (ABS16U,	     2, 16, 0xffff,     0, unsigned,	false),
  RL78_OP_REL (ABS16S,	     2, 16, 0xffff,     0, signed,	false),
  RL78_OP_REL (ABS8,	     1,	 8, 0xff,       0, bitfield,	false),
  RL78_OP_REL (ABS8U,	     1,	 8, 0xff,       0, unsigned,	false),
  RL78_OP_REL (ABS8S,	     1,	 8, 0xff,       0, signed,	false),
  RL78_OP_REL (ABS24S_PCREL, 4, 24, 0xffffff,   0, signed,	true),
  RL78_OP_REL (ABS16S_PCREL, 2, 16, 0xffff,     0, signed,	true),
  RL78_OP_REL (ABS8S_PCREL,  1,	 8, 0xff,       0, signed,	true),
  RL78_OP_REL (ABS16UL,	     2, 16, 0xffff,     0, unsigned,	false),
  RL78_OP_REL (ABS16UW,	     2, 16, 0xffff,     0, unsigned,	false),
  RL78_OP_REL (ABS8UL,	     1,	 8, 0xff,       0, unsigned,	false),
  RL78_OP_REL (ABS8UW,	     1,	 8, 0xff,       0, unsigned,	false),
  RL78_OP_REL (ABS32_REV,    4, 32, 0xffffffff, 0, dont,	false),
  RL78_OP_REL (ABS16_REV,    2, 16, 0xffff,     0, bitfield,	false),

#define STACK_REL_P(x) ((x) <= R_RL78_ABS16_REV && (x) >= R_RL78_ABS32)

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

  EMPTY_HOWTO (0x78),
  EMPTY_HOWTO (0x79),
  EMPTY_HOWTO (0x7a),
  EMPTY_HOWTO (0x7b),
  EMPTY_HOWTO (0x7c),
  EMPTY_HOWTO (0x7d),
  EMPTY_HOWTO (0x7e),
  EMPTY_HOWTO (0x7f),

  RL78_OP_REL (SYM,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPneg,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPadd,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPsub,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPmul,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPdiv,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPshla,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPshra,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPsctsize, 0, 0, 0, 0, dont, false),
  EMPTY_HOWTO (0x89),
  EMPTY_HOWTO (0x8a),
  EMPTY_HOWTO (0x8b),
  EMPTY_HOWTO (0x8c),
  RL78_OP_REL (OPscttop,  0, 0, 0, 0, dont, false),
  EMPTY_HOWTO (0x8e),
  EMPTY_HOWTO (0x8f),
  RL78_OP_REL (OPand,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPor,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPxor,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPnot,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPmod,	  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPromtop,  0, 0, 0, 0, dont, false),
  RL78_OP_REL (OPramtop,  0, 0, 0, 0, dont, false)
};

/* Map BFD reloc types to RL78 ELF reloc types.  */

struct rl78_reloc_map
{
  bfd_reloc_code_real_type  bfd_reloc_val;
  unsigned int		    rl78_reloc_val;
};

static const struct rl78_reloc_map rl78_reloc_map [] =
{
  { BFD_RELOC_NONE,		R_RL78_NONE },
  { BFD_RELOC_8,		R_RL78_DIR8S },
  { BFD_RELOC_16,		R_RL78_DIR16S },
  { BFD_RELOC_24,		R_RL78_DIR24S },
  { BFD_RELOC_32,		R_RL78_DIR32 },
  { BFD_RELOC_RL78_16_OP,	R_RL78_DIR16 },
  { BFD_RELOC_RL78_DIR3U_PCREL,	R_RL78_DIR3U_PCREL },
  { BFD_RELOC_8_PCREL,		R_RL78_DIR8S_PCREL },
  { BFD_RELOC_16_PCREL,		R_RL78_DIR16S_PCREL },
  { BFD_RELOC_24_PCREL,		R_RL78_DIR24S_PCREL },
  { BFD_RELOC_RL78_8U,		R_RL78_DIR8U },
  { BFD_RELOC_RL78_16U,		R_RL78_DIR16U },
  { BFD_RELOC_RL78_SYM,		R_RL78_SYM },
  { BFD_RELOC_RL78_OP_SUBTRACT,	R_RL78_OPsub },
  { BFD_RELOC_RL78_OP_NEG,	R_RL78_OPneg },
  { BFD_RELOC_RL78_OP_AND,	R_RL78_OPand },
  { BFD_RELOC_RL78_OP_SHRA,	R_RL78_OPshra },
  { BFD_RELOC_RL78_ABS8,	R_RL78_ABS8 },
  { BFD_RELOC_RL78_ABS16,	R_RL78_ABS16 },
  { BFD_RELOC_RL78_ABS16_REV,	R_RL78_ABS16_REV },
  { BFD_RELOC_RL78_ABS32,	R_RL78_ABS32 },
  { BFD_RELOC_RL78_ABS32_REV,	R_RL78_ABS32_REV },
  { BFD_RELOC_RL78_ABS16UL,	R_RL78_ABS16UL },
  { BFD_RELOC_RL78_ABS16UW,	R_RL78_ABS16UW },
  { BFD_RELOC_RL78_ABS16U,	R_RL78_ABS16U },
  { BFD_RELOC_RL78_SADDR,	R_RL78_RH_SADDR },
  { BFD_RELOC_RL78_RELAX,	R_RL78_RH_RELAX }
};

static reloc_howto_type *
rl78_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
			bfd_reloc_code_real_type code)
{
  unsigned int i;

  if (code == BFD_RELOC_RL78_32_OP)
    return rl78_elf_howto_table + R_RL78_DIR32;

  for (i = ARRAY_SIZE (rl78_reloc_map); i--;)
    if (rl78_reloc_map [i].bfd_reloc_val == code)
      return rl78_elf_howto_table + rl78_reloc_map[i].rl78_reloc_val;

  return NULL;
}

static reloc_howto_type *
rl78_reloc_name_lookup (bfd * abfd ATTRIBUTE_UNUSED, const char * r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (rl78_elf_howto_table); i++)
    if (rl78_elf_howto_table[i].name != NULL
	&& strcasecmp (rl78_elf_howto_table[i].name, r_name) == 0)
      return rl78_elf_howto_table + i;

  return NULL;
}

/* Set the howto pointer for an RL78 ELF reloc.  */

static bool
rl78_info_to_howto_rela (bfd *		     abfd,
			 arelent *	     cache_ptr,
			 Elf_Internal_Rela * dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= (unsigned int) R_RL78_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = rl78_elf_howto_table + r_type;
  return true;
}

static bfd_vma
get_symbol_value (const char *		  name,
		  struct bfd_link_info *  info,
		  bfd *			  input_bfd,
		  asection *		  input_section,
		  int			  offset)
{
  struct bfd_link_hash_entry * h;

  if (info == NULL)
    return 0;

  h = bfd_link_hash_lookup (info->hash, name, false, false, true);

  if (h == NULL
      || (h->type != bfd_link_hash_defined
	  && h->type != bfd_link_hash_defweak))
    {
      (*info->callbacks->undefined_symbol)
	(info, name, input_bfd, input_section, offset, true);
      return 0;
    }

  return (h->u.def.value
	  + h->u.def.section->output_section->vma
	  + h->u.def.section->output_offset);
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
static int32_t rl78_stack [ NUM_STACK_ENTRIES ];
static unsigned int rl78_stack_top;

static inline void
rl78_stack_push (bfd_vma val, bfd_reloc_status_type *r)
{
  if (rl78_stack_top < NUM_STACK_ENTRIES)
    rl78_stack[rl78_stack_top++] = val;
  else
    *r = bfd_reloc_dangerous;
}

static inline bfd_vma
rl78_stack_pop (bfd_reloc_status_type *r)
{
  if (rl78_stack_top > 0)
    return rl78_stack[-- rl78_stack_top];
  else
    *r = bfd_reloc_dangerous;
  return 0;
}

/* Special handling for RL78 complex relocs.  Returns the
   value of the reloc, or 0 for relocs which do not generate
   a result.  SYMVAL is the value of the symbol for relocs
   which use a symbolic argument.  */

static bfd_vma
rl78_compute_complex_reloc (unsigned long  r_type,
			    bfd_vma symval,
			    asection *input_section,
			    bfd_reloc_status_type *r,
			    char **error_message)
{
  int32_t tmp1, tmp2;
  bfd_vma relocation = 0;
  bfd_reloc_status_type status = bfd_reloc_ok;

  switch (r_type)
    {
    default:
      status = bfd_reloc_notsupported;
      break;

    case R_RL78_ABS24S_PCREL:
    case R_RL78_ABS16S_PCREL:
    case R_RL78_ABS8S_PCREL:
      relocation = rl78_stack_pop (&status);
      relocation -= input_section->output_section->vma + input_section->output_offset;
      break;

    case R_RL78_ABS32:
    case R_RL78_ABS32_REV:
    case R_RL78_ABS16:
    case R_RL78_ABS16_REV:
    case R_RL78_ABS16S:
    case R_RL78_ABS16U:
    case R_RL78_ABS8:
    case R_RL78_ABS8U:
    case R_RL78_ABS8S:
      relocation = rl78_stack_pop (&status);
      break;

    case R_RL78_ABS16UL:
    case R_RL78_ABS8UL:
      relocation = rl78_stack_pop (&status) >> 2;
      break;;

    case R_RL78_ABS16UW:
    case R_RL78_ABS8UW:
      relocation = rl78_stack_pop (&status) >> 1;
      break;

      /* The rest of the relocs compute values and then push them onto the stack.  */
    case R_RL78_OPramtop:
    case R_RL78_OPromtop:
    case R_RL78_SYM:
      rl78_stack_push (symval, &status);
      break;

    case R_RL78_OPneg:
      tmp1 = rl78_stack_pop (&status);
      tmp1 = - tmp1;
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPadd:
      tmp2 = rl78_stack_pop (&status);
      tmp1 = rl78_stack_pop (&status);
      tmp1 += tmp2;
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPsub:
      /* For the expression "A - B", the assembler pushes A,
	 then B, then OPSUB.  So the first op we pop is B, not A.  */
      tmp2 = rl78_stack_pop (&status);	/* B */
      tmp1 = rl78_stack_pop (&status);	/* A */
      tmp1 -= tmp2;		/* A - B */
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPmul:
      tmp2 = rl78_stack_pop (&status);
      tmp1 = rl78_stack_pop (&status);
      tmp1 *= tmp2;
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPdiv:
      tmp2 = rl78_stack_pop (&status);
      tmp1 = rl78_stack_pop (&status);
      if (tmp2 != 0)
	tmp1 /= tmp2;
      else
	{
	  tmp1 = 0;
	  status = bfd_reloc_overflow;
	}
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPshla:
      tmp2 = rl78_stack_pop (&status);
      tmp1 = rl78_stack_pop (&status);
      tmp1 <<= tmp2;
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPshra:
      tmp2 = rl78_stack_pop (&status);
      tmp1 = rl78_stack_pop (&status);
      tmp1 >>= tmp2;
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPsctsize:
      rl78_stack_push (input_section->size, &status);
      break;

    case R_RL78_OPscttop:
      rl78_stack_push (input_section->output_section->vma, &status);
      break;

    case R_RL78_OPand:
      tmp2 = rl78_stack_pop (&status);
      tmp1 = rl78_stack_pop (&status);
      tmp1 &= tmp2;
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPor:
      tmp2 = rl78_stack_pop (&status);
      tmp1 = rl78_stack_pop (&status);
      tmp1 |= tmp2;
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPxor:
      tmp2 = rl78_stack_pop (&status);
      tmp1 = rl78_stack_pop (&status);
      tmp1 ^= tmp2;
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPnot:
      tmp1 = rl78_stack_pop (&status);
      tmp1 = ~ tmp1;
      rl78_stack_push (tmp1, &status);
      break;

    case R_RL78_OPmod:
      tmp2 = rl78_stack_pop (&status);
      tmp1 = rl78_stack_pop (&status);
      if (tmp2 != 0)
	tmp1 %= tmp2;
      else
	{
	  tmp1 = 0;
	  status = bfd_reloc_overflow;
	}
      rl78_stack_push (tmp1, &status);
      break;
    }

  if (r)
    {
      if (status == bfd_reloc_dangerous)
	*error_message = (_("RL78 reloc stack overflow/underflow"));
      else if (status == bfd_reloc_overflow)
	{
	  status = bfd_reloc_dangerous;
	  *error_message = (_("RL78 reloc divide by zero"));
	}
      *r = status;
    }
  return relocation;
}

/* Check whether RELOCATION overflows a relocation field described by
   HOWTO.  */

static bfd_reloc_status_type
check_overflow (reloc_howto_type *howto, bfd_vma relocation)
{
  switch (howto->complain_on_overflow)
    {
    case complain_overflow_dont:
      break;

    case complain_overflow_bitfield:
      if ((bfd_signed_vma) relocation < -(1LL << (howto->bitsize - 1))
	  || (bfd_signed_vma) relocation >= 1LL << howto->bitsize)
	return bfd_reloc_overflow;
      break;

    case complain_overflow_signed:
      if ((bfd_signed_vma) relocation < -(1LL << (howto->bitsize - 1))
	  || (bfd_signed_vma) relocation >= 1LL << (howto->bitsize - 1))
	return bfd_reloc_overflow;
      break;

    case complain_overflow_unsigned:
      if (relocation >= 1ULL << howto->bitsize)
	return bfd_reloc_overflow;
      break;
    }
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
rl78_special_reloc (bfd *      input_bfd,
		    arelent *  reloc,
		    asymbol *  symbol,
		    void *     data,
		    asection * input_section,
		    bfd *      output_bfd ATTRIBUTE_UNUSED,
		    char **    error_message)
{
  bfd_reloc_status_type	 r = bfd_reloc_ok;
  bfd_vma		 relocation = 0;
  unsigned long		 r_type = reloc->howto->type;
  bfd_byte *		 contents = data;

  /* If necessary, compute the symbolic value of the relocation.  */
  switch (r_type)
    {
    case R_RL78_SYM:
      relocation = (symbol->value
		    + symbol->section->output_section->vma
		    + symbol->section->output_offset
		    + reloc->addend);
	break;

    case R_RL78_OPromtop:
      relocation = get_romstart (NULL, input_bfd, input_section,
				 reloc->address);
      break;

    case R_RL78_OPramtop:
      relocation = get_ramstart (NULL, input_bfd, input_section,
				 reloc->address);
      break;
    }

  /* Get the value of the relocation.  */
  relocation = rl78_compute_complex_reloc (r_type, relocation, input_section,
					   &r, error_message);

  if (STACK_REL_P (r_type))
    {
      bfd_size_type limit;
      unsigned int nbytes;

      if (r == bfd_reloc_ok)
	r = check_overflow (reloc->howto, relocation);

      if (r_type == R_RL78_ABS16_REV)
	relocation = ((relocation & 0xff) << 8) | ((relocation >> 8) & 0xff);
      else if (r_type == R_RL78_ABS32_REV)
	relocation = (((relocation & 0xff) << 24)
		      | ((relocation & 0xff00) << 8)
		      | ((relocation >> 8) & 0xff00)
		      | ((relocation >> 24) & 0xff));

      limit = bfd_get_section_limit_octets (input_bfd, input_section);
      nbytes = reloc->howto->bitsize / 8;
      if (reloc->address < limit
	  && nbytes <= limit - reloc->address)
	{
	  unsigned int i;

	  for (i = 0; i < nbytes; i++)
	    {
	      contents[reloc->address + i] = relocation;
	      relocation >>= 8;
	    }
	}
      else
	r = bfd_reloc_outofrange;
    }

  return r;
}

#define OP(i)      (contents[rel->r_offset + (i)])

/* Relocate an RL78 ELF section.
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
rl78_elf_relocate_section
    (bfd *		     output_bfd,
     struct bfd_link_info *  info,
     bfd *		     input_bfd,
     asection *		     input_section,
     bfd_byte *		     contents,
     Elf_Internal_Rela *     relocs,
     Elf_Internal_Sym *	     local_syms,
     asection **	     local_sections)
{
  Elf_Internal_Shdr *		symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  Elf_Internal_Rela *		rel;
  Elf_Internal_Rela *		relend;
  asection *splt;
  bool ret;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;

  splt = elf_hash_table (info)->splt;
  ret = true;
  for (rel = relocs; rel < relend; rel ++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name = NULL;
      bool unresolved_reloc = true;
      int r_type;
      char *error_message;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);

      howto  = rl78_elf_howto_table + ELF32_R_TYPE (rel->r_info);
      h	     = NULL;
      sym    = NULL;
      sec    = NULL;
      relocation = 0;

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
	  bool warned ATTRIBUTE_UNUSED;
	  bool ignored ATTRIBUTE_UNUSED;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes, h,
				   sec, relocation, unresolved_reloc,
				   warned, ignored);

	  name = h->root.root.string;
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

      switch (ELF32_R_TYPE (rel->r_info))
	{
	case R_RL78_DIR16S:
	  {
	    bfd_vma *plt_offset;

	    if (h != NULL)
	      plt_offset = &h->plt.offset;
	    else
	      plt_offset = elf_local_got_offsets (input_bfd) + r_symndx;

	    if (! valid_16bit_address (relocation))
	      {
		/* If this is the first time we've processed this symbol,
		   fill in the plt entry with the correct symbol address.  */
		if ((*plt_offset & 1) == 0)
		  {
		    unsigned int x;

		    x = 0x000000ec;  /* br !!abs24 */
		    x |= (relocation << 8) & 0xffffff00;
		    bfd_put_32 (input_bfd, x, splt->contents + *plt_offset);
		    *plt_offset |= 1;
		  }

		relocation = (splt->output_section->vma
			      + splt->output_offset
			      + (*plt_offset & -2));
		if (name)
		{
		  char *newname = bfd_malloc (strlen(name)+5);
		  strcpy (newname, name);
		  strcat(newname, ".plt");
		  _bfd_generic_link_add_one_symbol (info,
						    input_bfd,
						    newname,
						    BSF_FUNCTION | BSF_WEAK,
						    splt,
						    (*plt_offset & -2),
						    0,
						    1,
						    0,
						    0);
		}
	      }
	  }
	  break;
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
	      relocation -= bfd_get_reloc_size (howto);
	    }

	  relocation += rel->r_addend;
	}

      r = bfd_reloc_ok;
      if (howto->bitsize != 0
	  && (rel->r_offset >= input_section->size
	      || ((howto->bitsize + 7u) / 8
		  > input_section->size - rel->r_offset)))
	r = bfd_reloc_outofrange;
      else
	switch (r_type)
	  {
	  case R_RL78_NONE:
	    break;

	  case R_RL78_RH_RELAX:
	    break;

	  case R_RL78_DIR8S_PCREL:
	    OP (0) = relocation;
	    break;

	  case R_RL78_DIR8S:
	    OP (0) = relocation;
	    break;

	  case R_RL78_DIR8U:
	    OP (0) = relocation;
	    break;

	  case R_RL78_DIR16S_PCREL:
	    OP (0) = relocation;
	    OP (1) = relocation >> 8;
	    break;

	  case R_RL78_DIR16S:
	    if ((relocation & 0xf0000) == 0xf0000)
	      relocation &= 0xffff;
	    OP (0) = relocation;
	    OP (1) = relocation >> 8;
	    break;

	  case R_RL78_DIR16U:
	    OP (0) = relocation;
	    OP (1) = relocation >> 8;
	    break;

	  case R_RL78_DIR16:
	    OP (0) = relocation;
	    OP (1) = relocation >> 8;
	    break;

	  case R_RL78_DIR16_REV:
	    OP (1) = relocation;
	    OP (0) = relocation >> 8;
	    break;

	  case R_RL78_DIR3U_PCREL:
	    OP (0) &= 0xf8;
	    OP (0) |= relocation & 0x07;
	    /* Map [3, 10] to [0, 7].  The code below using howto
	       bitsize will check for unsigned overflow.  */
	    relocation -= 3;
	    break;

	  case R_RL78_DIR24S_PCREL:
	    OP (0) = relocation;
	    OP (1) = relocation >> 8;
	    OP (2) = relocation >> 16;
	    break;

	  case R_RL78_DIR24S:
	    OP (0) = relocation;
	    OP (1) = relocation >> 8;
	    OP (2) = relocation >> 16;
	    break;

	  case R_RL78_DIR32:
	    OP (0) = relocation;
	    OP (1) = relocation >> 8;
	    OP (2) = relocation >> 16;
	    OP (3) = relocation >> 24;
	    break;

	  case R_RL78_DIR32_REV:
	    OP (3) = relocation;
	    OP (2) = relocation >> 8;
	    OP (1) = relocation >> 16;
	    OP (0) = relocation >> 24;
	    break;

	  case R_RL78_RH_SFR:
	    relocation -= 0xfff00;
	    OP (0) = relocation;
	    break;

	  case R_RL78_RH_SADDR:
	    relocation -= 0xffe20;
	    OP (0) = relocation;
	    break;

	    /* Complex reloc handling:  */
	  case R_RL78_ABS32:
	  case R_RL78_ABS32_REV:
	  case R_RL78_ABS24S_PCREL:
	  case R_RL78_ABS24S:
	  case R_RL78_ABS16:
	  case R_RL78_ABS16_REV:
	  case R_RL78_ABS16S_PCREL:
	  case R_RL78_ABS16S:
	  case R_RL78_ABS16U:
	  case R_RL78_ABS16UL:
	  case R_RL78_ABS16UW:
	  case R_RL78_ABS8:
	  case R_RL78_ABS8U:
	  case R_RL78_ABS8UL:
	  case R_RL78_ABS8UW:
	  case R_RL78_ABS8S_PCREL:
	  case R_RL78_ABS8S:
	  case R_RL78_OPneg:
	  case R_RL78_OPadd:
	  case R_RL78_OPsub:
	  case R_RL78_OPmul:
	  case R_RL78_OPdiv:
	  case R_RL78_OPshla:
	  case R_RL78_OPshra:
	  case R_RL78_OPsctsize:
	  case R_RL78_OPscttop:
	  case R_RL78_OPand:
	  case R_RL78_OPor:
	  case R_RL78_OPxor:
	  case R_RL78_OPnot:
	  case R_RL78_OPmod:
	    relocation = rl78_compute_complex_reloc (r_type, 0, input_section,
						     &r, &error_message);

	    switch (r_type)
	      {
	      case R_RL78_ABS32:
		OP (0) = relocation;
		OP (1) = relocation >> 8;
		OP (2) = relocation >> 16;
		OP (3) = relocation >> 24;
		break;

	      case R_RL78_ABS32_REV:
		OP (3) = relocation;
		OP (2) = relocation >> 8;
		OP (1) = relocation >> 16;
		OP (0) = relocation >> 24;
		break;

	      case R_RL78_ABS24S_PCREL:
	      case R_RL78_ABS24S:
		OP (0) = relocation;
		OP (1) = relocation >> 8;
		OP (2) = relocation >> 16;
		break;

	      case R_RL78_ABS16:
		OP (0) = relocation;
		OP (1) = relocation >> 8;
		break;

	      case R_RL78_ABS16_REV:
		OP (1) = relocation;
		OP (0) = relocation >> 8;
		break;

	      case R_RL78_ABS16S_PCREL:
	      case R_RL78_ABS16S:
		OP (0) = relocation;
		OP (1) = relocation >> 8;
		break;

	      case R_RL78_ABS16U:
	      case R_RL78_ABS16UL:
	      case R_RL78_ABS16UW:
		OP (0) = relocation;
		OP (1) = relocation >> 8;
		break;

	      case R_RL78_ABS8:
		OP (0) = relocation;
		break;

	      case R_RL78_ABS8U:
	      case R_RL78_ABS8UL:
	      case R_RL78_ABS8UW:
		OP (0) = relocation;
		break;

	      case R_RL78_ABS8S_PCREL:
	      case R_RL78_ABS8S:
		OP (0) = relocation;
		break;

	      default:
		break;
	      }
	    break;

	  case R_RL78_SYM:
	    if (r_symndx < symtab_hdr->sh_info)
	      relocation = sec->output_section->vma + sec->output_offset
		+ sym->st_value + rel->r_addend;
	    else if (h != NULL
		     && (h->root.type == bfd_link_hash_defined
			 || h->root.type == bfd_link_hash_defweak))
	      relocation = h->root.u.def.value
		+ sec->output_section->vma
		+ sec->output_offset
		+ rel->r_addend;
	    else
	      {
		relocation = 0;
		if (h->root.type != bfd_link_hash_undefweak)
		  _bfd_error_handler
		    (_("warning: RL78_SYM reloc with an unknown symbol"));
	      }
	    (void) rl78_compute_complex_reloc (r_type, relocation, input_section,
					       &r, &error_message);
	    break;

	  case R_RL78_OPromtop:
	    relocation = get_romstart (info, input_bfd, input_section,
				       rel->r_offset);
	    (void) rl78_compute_complex_reloc (r_type, relocation, input_section,
					       &r, &error_message);
	    break;

	  case R_RL78_OPramtop:
	    relocation = get_ramstart (info, input_bfd, input_section,
				       rel->r_offset);
	    (void) rl78_compute_complex_reloc (r_type, relocation, input_section,
					       &r, &error_message);
	    break;

	  default:
	    r = bfd_reloc_notsupported;
	    break;
	  }

      if (r == bfd_reloc_ok)
	r = check_overflow (howto, relocation);

      if (r != bfd_reloc_ok)
	{
	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow)
		(info, (h ? &h->root : NULL), name, howto->name, (bfd_vma) 0,
		 input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, name, input_bfd, input_section, rel->r_offset, true);
	      break;

	    case bfd_reloc_outofrange:
	       /* xgettext:c-format */
	      (*info->callbacks->einfo)
		(_("%H: %s out of range\n"),
		 input_bfd, input_section, rel->r_offset, howto->name);
	      break;

	    case bfd_reloc_notsupported:
	      /* xgettext:c-format */
	      (*info->callbacks->einfo)
		(_("%H: relocation type %u is not supported\n"),
		 input_bfd, input_section, rel->r_offset, r_type);
	      break;

	    case bfd_reloc_dangerous:
	      (*info->callbacks->reloc_dangerous)
		(info, error_message, input_bfd, input_section, rel->r_offset);
	      break;

	    default:
	      /* xgettext:c-format */
	      (*info->callbacks->einfo)
		(_("%H: relocation %s returns an unrecognized value %x\n"),
		 input_bfd, input_section, rel->r_offset, howto->name, r);
	      break;
	    }
	  ret = false;
	}
    }

  return ret;
}

/* Function to set the ELF flag bits.  */

static bool
rl78_elf_set_private_flags (bfd * abfd, flagword flags)
{
  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

static bool no_warn_mismatch = false;

void bfd_elf32_rl78_set_target_flags (bool);

void
bfd_elf32_rl78_set_target_flags (bool user_no_warn_mismatch)
{
  no_warn_mismatch = user_no_warn_mismatch;
}

static const char *
rl78_cpu_name (flagword flags)
{
  switch (flags & E_FLAG_RL78_CPU_MASK)
    {
    default: return "";
    case E_FLAG_RL78_G10:     return "G10";
    case E_FLAG_RL78_G13:     return "G13";
    case E_FLAG_RL78_G14:     return "G14";
    }
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
rl78_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword new_flags;
  flagword old_flags;
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
      flagword changed_flags = old_flags ^ new_flags;

      if (changed_flags & E_FLAG_RL78_CPU_MASK)
	{
	  flagword out_cpu = old_flags & E_FLAG_RL78_CPU_MASK;
	  flagword in_cpu = new_flags & E_FLAG_RL78_CPU_MASK;

	  if (in_cpu == E_FLAG_RL78_ANY_CPU || in_cpu == out_cpu)
	    /* It does not matter what new_cpu may have.  */;
	  else if (out_cpu == E_FLAG_RL78_ANY_CPU)
	    {
	      if (in_cpu == E_FLAG_RL78_G10)
		{
		  /* G10 files can only be linked with other G10 files.
		     If the output is set to "any" this means that it is
		     a G14 file that does not use hardware multiply/divide,
		     but that is still incompatible with the G10 ABI.  */
		  error = true;

		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("RL78 ABI conflict: G10 file %pB cannot be linked"
		       " with %s file %pB"),
		     ibfd, rl78_cpu_name (out_cpu), obfd);
		}
	      else
		{
		  old_flags &= ~ E_FLAG_RL78_CPU_MASK;
		  old_flags |= in_cpu;
		  elf_elfheader (obfd)->e_flags = old_flags;
		}
	    }
	  else
	    {
	      error = true;

	      _bfd_error_handler
		/* xgettext:c-format */
		(_("RL78 ABI conflict: cannot link %s file %pB with %s file %pB"),
		 rl78_cpu_name (in_cpu),  ibfd,
		 rl78_cpu_name (out_cpu), obfd);
	    }
	}

      if (changed_flags & E_FLAG_RL78_64BIT_DOUBLES)
	{
	  _bfd_error_handler
	    (_("RL78 merge conflict: cannot link 32-bit and 64-bit objects together"));

	  if (old_flags & E_FLAG_RL78_64BIT_DOUBLES)
	    /* xgettext:c-format */
	    _bfd_error_handler (_("- %pB is 64-bit, %pB is not"),
				obfd, ibfd);
	  else
	    /* xgettext:c-format */
	    _bfd_error_handler (_("- %pB is 64-bit, %pB is not"),
				ibfd, obfd);
	  error = true;
	}
    }

  return !error;
}

static bool
rl78_elf_print_private_bfd_data (bfd * abfd, void * ptr)
{
  FILE * file = (FILE *) ptr;
  flagword flags;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  flags = elf_elfheader (abfd)->e_flags;
  fprintf (file, _("private flags = 0x%lx:"), (long) flags);

  if (flags & E_FLAG_RL78_CPU_MASK)
    fprintf (file, " [%s]", rl78_cpu_name (flags));

  if (flags & E_FLAG_RL78_64BIT_DOUBLES)
    fprintf (file, _(" [64-bit doubles]"));

  fputc ('\n', file);
  return true;
}

/* Return the MACH for an e_flags value.  */

static int
elf32_rl78_machine (bfd * abfd ATTRIBUTE_UNUSED)
{
  return bfd_mach_rl78;
}

static bool
rl78_elf_object_p (bfd * abfd)
{
  bfd_default_set_arch_mach (abfd, bfd_arch_rl78,
			     elf32_rl78_machine (abfd));
  return true;
}

/* support PLT for 16-bit references to 24-bit functions.  */

/* We support 16-bit pointers to code above 64k by generating a thunk
   below 64k containing a JMP instruction to the final address.  */

static bool
rl78_elf_check_relocs
    (bfd *		       abfd,
     struct bfd_link_info *    info,
     asection *		       sec,
     const Elf_Internal_Rela * relocs)
{
  Elf_Internal_Shdr *		symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  const Elf_Internal_Rela *	rel;
  const Elf_Internal_Rela *	rel_end;
  bfd_vma *local_plt_offsets;
  asection *splt;
  bfd *dynobj;

  if (bfd_link_relocatable (info))
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  local_plt_offsets = elf_local_got_offsets (abfd);
  dynobj = elf_hash_table(info)->dynobj;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;
      bfd_vma *offset;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      switch (ELF32_R_TYPE (rel->r_info))
	{
	  /* This relocation describes a 16-bit pointer to a function.
	     We may need to allocate a thunk in low memory; reserve memory
	     for it now.  */
	case R_RL78_DIR16S:
	  if (dynobj == NULL)
	    elf_hash_table (info)->dynobj = dynobj = abfd;
	  splt = elf_hash_table (info)->splt;
	  if (splt == NULL)
	    {
	      flagword flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
				| SEC_IN_MEMORY | SEC_LINKER_CREATED
				| SEC_READONLY | SEC_CODE);
	      splt = bfd_make_section_anyway_with_flags (dynobj, ".plt",
							 flags);
	      elf_hash_table (info)->splt = splt;
	      if (splt == NULL
		  || !bfd_set_section_alignment (splt, 1))
		return false;
	    }

	  if (h != NULL)
	    offset = &h->plt.offset;
	  else
	    {
	      if (local_plt_offsets == NULL)
		{
		  size_t size;
		  unsigned int i;

		  size = symtab_hdr->sh_info * sizeof (bfd_vma);
		  local_plt_offsets = (bfd_vma *) bfd_alloc (abfd, size);
		  if (local_plt_offsets == NULL)
		    return false;
		  elf_local_got_offsets (abfd) = local_plt_offsets;

		  for (i = 0; i < symtab_hdr->sh_info; i++)
		    local_plt_offsets[i] = (bfd_vma) -1;
		}
	      offset = &local_plt_offsets[r_symndx];
	    }

	  if (*offset == (bfd_vma) -1)
	    {
	      *offset = splt->size;
	      splt->size += 4;
	    }
	  break;
	}
    }

  return true;
}

/* This must exist if dynobj is ever set.  */

static bool
rl78_elf_finish_dynamic_sections (bfd *abfd ATTRIBUTE_UNUSED,
				  struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *splt;

  if (!elf_hash_table (info)->dynamic_sections_created)
    return true;

  /* As an extra sanity check, verify that all plt entries have been
     filled in.  However, relaxing might have changed the relocs so
     that some plt entries don't get filled in, so we have to skip
     this check if we're relaxing.  Unfortunately, check_relocs is
     called before relaxation.  */

  if (info->relax_trip > 0)
    return true;

  dynobj = elf_hash_table (info)->dynobj;
  splt = elf_hash_table (info)->splt;
  if (dynobj != NULL && splt != NULL)
    {
      bfd_byte *contents = splt->contents;
      unsigned int i, size = splt->size;

      for (i = 0; i < size; i += 4)
	{
	  unsigned int x = bfd_get_32 (dynobj, contents + i);
	  BFD_ASSERT (x != 0);
	}
    }

  return true;
}

static bool
rl78_elf_always_size_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
			       struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *splt;

  if (bfd_link_relocatable (info))
    return true;

  dynobj = elf_hash_table (info)->dynobj;
  if (dynobj == NULL)
    return true;

  splt = elf_hash_table (info)->splt;
  BFD_ASSERT (splt != NULL);

  splt->contents = (bfd_byte *) bfd_zalloc (dynobj, splt->size);
  if (splt->contents == NULL)
    return false;

  return true;
}



/* Handle relaxing.  */

/* A subroutine of rl78_elf_relax_section.  If the global symbol H
   is within the low 64k, remove any entry for it in the plt.  */

struct relax_plt_data
{
  asection *splt;
  bool *again;
};

static bool
rl78_relax_plt_check (struct elf_link_hash_entry *h, void * xdata)
{
  struct relax_plt_data *data = (struct relax_plt_data *) xdata;

  if (h->plt.offset != (bfd_vma) -1)
    {
      bfd_vma address;

      if (h->root.type == bfd_link_hash_undefined
	  || h->root.type == bfd_link_hash_undefweak)
	address = 0;
      else
	address = (h->root.u.def.section->output_section->vma
		   + h->root.u.def.section->output_offset
		   + h->root.u.def.value);

      if (valid_16bit_address (address))
	{
	  h->plt.offset = -1;
	  data->splt->size -= 4;
	  *data->again = true;
	}
    }

  return true;
}

/* A subroutine of rl78_elf_relax_section.  If the global symbol H
   previously had a plt entry, give it a new entry offset.  */

static bool
rl78_relax_plt_realloc (struct elf_link_hash_entry *h, void * xdata)
{
  bfd_vma *entry = (bfd_vma *) xdata;

  if (h->plt.offset != (bfd_vma) -1)
    {
      h->plt.offset = *entry;
      *entry += 4;
    }

  return true;
}

static bool
rl78_elf_relax_plt_section (bfd *dynobj,
			    asection *splt,
			    struct bfd_link_info *info,
			    bool *again)
{
  struct relax_plt_data relax_plt_data;
  bfd *ibfd;

  /* Assume nothing changes.  */
  *again = false;

  if (bfd_link_relocatable (info))
    return true;

  /* We only relax the .plt section at the moment.  */
  if (dynobj != elf_hash_table (info)->dynobj
      || strcmp (splt->name, ".plt") != 0)
    return true;

  /* Quick check for an empty plt.  */
  if (splt->size == 0)
    return true;

  /* Map across all global symbols; see which ones happen to
     fall in the low 64k.  */
  relax_plt_data.splt = splt;
  relax_plt_data.again = again;
  elf_link_hash_traverse (elf_hash_table (info), rl78_relax_plt_check,
			  &relax_plt_data);

  /* Likewise for local symbols, though that's somewhat less convenient
     as we have to walk the list of input bfds and swap in symbol data.  */
  for (ibfd = info->input_bfds; ibfd ; ibfd = ibfd->link.next)
    {
      bfd_vma *local_plt_offsets = elf_local_got_offsets (ibfd);
      Elf_Internal_Shdr *symtab_hdr;
      Elf_Internal_Sym *isymbuf = NULL;
      unsigned int idx;

      if (! local_plt_offsets)
	continue;

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      if (symtab_hdr->sh_info != 0)
	{
	  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	  if (isymbuf == NULL)
	    isymbuf = bfd_elf_get_elf_syms (ibfd, symtab_hdr,
					    symtab_hdr->sh_info, 0,
					    NULL, NULL, NULL);
	  if (isymbuf == NULL)
	    return false;
	}

      for (idx = 0; idx < symtab_hdr->sh_info; ++idx)
	{
	  Elf_Internal_Sym *isym;
	  asection *tsec;
	  bfd_vma address;

	  if (local_plt_offsets[idx] == (bfd_vma) -1)
	    continue;

	  isym = &isymbuf[idx];
	  if (isym->st_shndx == SHN_UNDEF)
	    continue;
	  else if (isym->st_shndx == SHN_ABS)
	    tsec = bfd_abs_section_ptr;
	  else if (isym->st_shndx == SHN_COMMON)
	    tsec = bfd_com_section_ptr;
	  else
	    tsec = bfd_section_from_elf_index (ibfd, isym->st_shndx);

	  address = (tsec->output_section->vma
		     + tsec->output_offset
		     + isym->st_value);
	  if (valid_16bit_address (address))
	    {
	      local_plt_offsets[idx] = -1;
	      splt->size -= 4;
	      *again = true;
	    }
	}

      if (isymbuf != NULL
	  && symtab_hdr->contents != (unsigned char *) isymbuf)
	{
	  if (! info->keep_memory)
	    free (isymbuf);
	  else
	    {
	      /* Cache the symbols for elf_link_input_bfd.  */
	      symtab_hdr->contents = (unsigned char *) isymbuf;
	    }
	}
    }

  /* If we changed anything, walk the symbols again to reallocate
     .plt entry addresses.  */
  if (*again && splt->size > 0)
    {
      bfd_vma entry = 0;

      elf_link_hash_traverse (elf_hash_table (info),
			      rl78_relax_plt_realloc, &entry);

      for (ibfd = info->input_bfds; ibfd ; ibfd = ibfd->link.next)
	{
	  bfd_vma *local_plt_offsets = elf_local_got_offsets (ibfd);
	  unsigned int nlocals = elf_tdata (ibfd)->symtab_hdr.sh_info;
	  unsigned int idx;

	  if (! local_plt_offsets)
	    continue;

	  for (idx = 0; idx < nlocals; ++idx)
	    if (local_plt_offsets[idx] != (bfd_vma) -1)
	      {
		local_plt_offsets[idx] = entry;
		entry += 4;
	      }
	}
    }

  return true;
}

/* Delete some bytes from a section while relaxing.  */

static bool
elf32_rl78_relax_delete_bytes (bfd *abfd, asection *sec, bfd_vma addr, int count,
			       Elf_Internal_Rela *alignment_rel, int force_snip)
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

  irel = elf_section_data (sec)->relocs;
  if (irel == NULL)
    {
      _bfd_elf_link_read_relocs (sec->owner, sec, NULL, NULL, true);
      irel = elf_section_data (sec)->relocs;
    }

  irelend = irel + sec->reloc_count;

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

  /* Adjust all the relocs.  */
  for (; irel && irel < irelend; irel++)
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
	  && ELF32_R_TYPE (irel->r_info) == R_RL78_RH_RELAX
	  && irel->r_addend & RL78_RELAXA_ALIGN)
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
   we maintain their relative order, except that R_RL78_RH_RELAX
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
	  else if (ELF32_R_TYPE (r[i + 1].r_info) == R_RL78_RH_RELAX
		   && (r[i + 1].r_addend & RL78_RELAXA_ALIGN))
	    swappit = true;
	  else if (ELF32_R_TYPE (r[i + 1].r_info) == R_RL78_RH_RELAX
		   && (r[i + 1].r_addend & RL78_RELAXA_ELIGN)
		   && !(ELF32_R_TYPE (r[i].r_info) == R_RL78_RH_RELAX
			&& (r[i].r_addend & RL78_RELAXA_ALIGN)))
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
  rl78_offset_for_reloc (abfd, rel + 1, symtab_hdr, shndx_buf, intsyms, \
			 lrel, abfd, sec, link_info, scale)

static bfd_vma
rl78_offset_for_reloc (bfd *			abfd,
		       Elf_Internal_Rela *	rel,
		       Elf_Internal_Shdr *	symtab_hdr,
		       bfd_byte *		shndx_buf ATTRIBUTE_UNUSED,
		       Elf_Internal_Sym *	intsyms,
		       Elf_Internal_Rela **	lrel,
		       bfd *			input_bfd,
		       asection *		input_section,
		       struct bfd_link_info *	info,
		       int *			scale)
{
  bfd_vma symval;

  *scale = 1;

  /* REL is the first of 1..N relocations.  We compute the symbol
     value for each relocation, then combine them if needed.  LREL
     gets a pointer to the last relocation used.  */
  while (1)
    {
      unsigned long r_type;

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

      r_type = ELF32_R_TYPE (rel->r_info);
      switch (r_type)
	{
	case R_RL78_SYM:
	  (void) rl78_compute_complex_reloc (r_type, symval, input_section,
					     NULL, NULL);
	  break;

	case R_RL78_OPromtop:
	  symval = get_romstart (info, input_bfd, input_section, rel->r_offset);
	  (void) rl78_compute_complex_reloc (r_type, symval, input_section,
					     NULL, NULL);
	  break;

	case R_RL78_OPramtop:
	  symval = get_ramstart (info, input_bfd, input_section, rel->r_offset);
	  (void) rl78_compute_complex_reloc (r_type, symval, input_section,
					     NULL, NULL);
	  break;

	case R_RL78_OPneg:
	case R_RL78_OPadd:
	case R_RL78_OPsub:
	case R_RL78_OPmul:
	case R_RL78_OPdiv:
	case R_RL78_OPshla:
	case R_RL78_OPshra:
	case R_RL78_OPsctsize:
	case R_RL78_OPscttop:
	case R_RL78_OPand:
	case R_RL78_OPor:
	case R_RL78_OPxor:
	case R_RL78_OPnot:
	case R_RL78_OPmod:
	  (void) rl78_compute_complex_reloc (r_type, 0, input_section,
					     NULL, NULL);
	  break;

	case R_RL78_DIR16UL:
	case R_RL78_DIR8UL:
	case R_RL78_ABS16UL:
	case R_RL78_ABS8UL:
	  *scale = 4;
	  goto reloc_computes_value;

	case R_RL78_DIR16UW:
	case R_RL78_DIR8UW:
	case R_RL78_ABS16UW:
	case R_RL78_ABS8UW:
	  *scale = 2;
	  goto reloc_computes_value;

	default:
	reloc_computes_value:
	  symval = rl78_compute_complex_reloc (r_type, symval, input_section,
					       NULL, NULL);
	  /* Fall through.  */
	case R_RL78_DIR32:
	case R_RL78_DIR24S:
	case R_RL78_DIR16:
	case R_RL78_DIR16U:
	case R_RL78_DIR16S:
	case R_RL78_DIR24S_PCREL:
	case R_RL78_DIR16S_PCREL:
	case R_RL78_DIR8S_PCREL:
	  if (lrel)
	    *lrel = rel;
	  return symval;
	}

      rel ++;
    }
}

const struct {
  int prefix;		/* or -1 for "no prefix" */
  int insn;		/* or -1 for "end of list" */
  int insn_for_saddr;	/* or -1 for "no alternative" */
  int insn_for_sfr;	/* or -1 for "no alternative" */
} relax_addr16[] = {
  { -1, 0x02, 0x06, -1 },	/* ADDW	AX, !addr16 */
  { -1, 0x22, 0x26, -1 },	/* SUBW	AX, !addr16 */
  { -1, 0x42, 0x46, -1 },	/* CMPW	AX, !addr16 */
  { -1, 0x40, 0x4a, -1 },	/* CMP	!addr16, #byte */

  { -1, 0x0f, 0x0b, -1 },	/* ADD	A, !addr16 */
  { -1, 0x1f, 0x1b, -1 },	/* ADDC	A, !addr16 */
  { -1, 0x2f, 0x2b, -1 },	/* SUB	A, !addr16 */
  { -1, 0x3f, 0x3b, -1 },	/* SUBC	A, !addr16 */
  { -1, 0x4f, 0x4b, -1 },	/* CMP	A, !addr16 */
  { -1, 0x5f, 0x5b, -1 },	/* AND	A, !addr16 */
  { -1, 0x6f, 0x6b, -1 },	/* OR	A, !addr16 */
  { -1, 0x7f, 0x7b, -1 },	/* XOR	A, !addr16 */

  { -1, 0x8f, 0x8d, 0x8e },	/* MOV	A, !addr16 */
  { -1, 0x9f, 0x9d, 0x9e },	/* MOV	!addr16, A */
  { -1, 0xaf, 0xad, 0xae },	/* MOVW	AX, !addr16 */
  { -1, 0xbf, 0xbd, 0xbe },	/* MOVW	!addr16, AX */
  { -1, 0xcf, 0xcd, 0xce },	/* MOVW	!addr16, #word */

  { -1, 0xa0, 0xa4, -1 },	/* INC	!addr16 */
  { -1, 0xa2, 0xa6, -1 },	/* INCW	!addr16 */
  { -1, 0xb0, 0xb4, -1 },	/* DEC	!addr16 */
  { -1, 0xb2, 0xb6, -1 },	/* DECW	!addr16 */

  { -1, 0xd5, 0xd4, -1 },	/* CMP0	!addr16 */
  { -1, 0xe5, 0xe4, -1 },	/* ONEB	!addr16 */
  { -1, 0xf5, 0xf4, -1 },	/* CLRB	!addr16 */

  { -1, 0xd9, 0xd8, -1 },	/* MOV	X, !addr16 */
  { -1, 0xe9, 0xe8, -1 },	/* MOV	B, !addr16 */
  { -1, 0xf9, 0xf8, -1 },	/* MOV	C, !addr16 */
  { -1, 0xdb, 0xda, -1 },	/* MOVW	BC, !addr16 */
  { -1, 0xeb, 0xea, -1 },	/* MOVW	DE, !addr16 */
  { -1, 0xfb, 0xfa, -1 },	/* MOVW	HL, !addr16 */

  { 0x61, 0xaa, 0xa8, -1 },	/* XCH	A, !addr16 */

  { 0x71, 0x00, 0x02, 0x0a },	/* SET1	!addr16.0 */
  { 0x71, 0x10, 0x12, 0x1a },	/* SET1	!addr16.0 */
  { 0x71, 0x20, 0x22, 0x2a },	/* SET1	!addr16.0 */
  { 0x71, 0x30, 0x32, 0x3a },	/* SET1	!addr16.0 */
  { 0x71, 0x40, 0x42, 0x4a },	/* SET1	!addr16.0 */
  { 0x71, 0x50, 0x52, 0x5a },	/* SET1	!addr16.0 */
  { 0x71, 0x60, 0x62, 0x6a },	/* SET1	!addr16.0 */
  { 0x71, 0x70, 0x72, 0x7a },	/* SET1	!addr16.0 */

  { 0x71, 0x08, 0x03, 0x0b },	/* CLR1	!addr16.0 */
  { 0x71, 0x18, 0x13, 0x1b },	/* CLR1	!addr16.0 */
  { 0x71, 0x28, 0x23, 0x2b },	/* CLR1	!addr16.0 */
  { 0x71, 0x38, 0x33, 0x3b },	/* CLR1	!addr16.0 */
  { 0x71, 0x48, 0x43, 0x4b },	/* CLR1	!addr16.0 */
  { 0x71, 0x58, 0x53, 0x5b },	/* CLR1	!addr16.0 */
  { 0x71, 0x68, 0x63, 0x6b },	/* CLR1	!addr16.0 */
  { 0x71, 0x78, 0x73, 0x7b },	/* CLR1	!addr16.0 */

  { -1, -1, -1, -1 }
};

/* Relax one section.  */

static bool
rl78_elf_relax_section (bfd *abfd,
			asection *sec,
			struct bfd_link_info *link_info,
			bool *again)
{
  Elf_Internal_Shdr * symtab_hdr;
  Elf_Internal_Shdr * shndx_hdr;
  Elf_Internal_Rela * internal_relocs;
  Elf_Internal_Rela * free_relocs = NULL;
  Elf_Internal_Rela * irel;
  Elf_Internal_Rela * srel;
  Elf_Internal_Rela * irelend;
  Elf_Internal_Rela * next_alignment;
  bfd_byte *	      contents = NULL;
  bfd_byte *	      free_contents = NULL;
  Elf_Internal_Sym *  intsyms = NULL;
  Elf_Internal_Sym *  free_intsyms = NULL;
  bfd_byte *	      shndx_buf = NULL;
  bfd_vma pc;
  bfd_vma symval ATTRIBUTE_UNUSED = 0;
  int pcrel ATTRIBUTE_UNUSED = 0;
  int code ATTRIBUTE_UNUSED = 0;
  int section_alignment_glue;
  int scale;

  if (abfd == elf_hash_table (link_info)->dynobj
      && strcmp (sec->name, ".plt") == 0)
    return rl78_elf_relax_plt_section (abfd, sec, link_info, again);

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
	  bfd_set_error (bfd_error_no_memory);
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
  internal_relocs = (_bfd_elf_link_read_relocs
		     (abfd, sec, NULL, (Elf_Internal_Rela *) NULL,
		      link_info->keep_memory));
  if (internal_relocs == NULL)
    goto error_return;
  if (! link_info->keep_memory)
    free_relocs = internal_relocs;

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

  /* We calculate worst case shrinkage caused by alignment directives.
     No fool-proof, but better than either ignoring the problem or
     doing heavy duty analysis of all the alignment markers in all
     input sections.  */
  section_alignment_glue = 0;
  for (irel = internal_relocs; irel < irelend; irel++)
      if (ELF32_R_TYPE (irel->r_info) == R_RL78_RH_RELAX
	  && irel->r_addend & RL78_RELAXA_ALIGN)
	{
	  int this_glue = 1 << (irel->r_addend & RL78_RELAXA_ANUM);

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
      if (ELF32_R_TYPE (irel->r_info) != R_RL78_RH_RELAX)
	continue;

      if (irel->r_addend & RL78_RELAXA_ALIGN
	  || next_alignment == internal_relocs)
	{
	  /* When we delete bytes, we need to maintain all the alignments
	     indicated.  In addition, we need to be careful about relaxing
	     jumps across alignment boundaries - these displacements
	     *grow* when we delete bytes.  For now, don't shrink
	     displacements across an alignment boundary, just in case.
	     Note that this only affects relocations to the same
	     section.  */
	  next_alignment += 2;
	  while (next_alignment < irelend
		 && (ELF32_R_TYPE (next_alignment->r_info) != R_RL78_RH_RELAX
		     || !(next_alignment->r_addend & RL78_RELAXA_ELIGN)))
	    next_alignment ++;
	  if (next_alignment >= irelend || next_alignment->r_offset == 0)
	    next_alignment = NULL;
	}

      /* When we hit alignment markers, see if we've shrunk enough
	 before them to reduce the gap without violating the alignment
	 requirements.  */
      if (irel->r_addend & RL78_RELAXA_ALIGN)
	{
	  /* At this point, the next relocation *should* be the ELIGN
	     end marker.  */
	  Elf_Internal_Rela *erel = irel + 1;
	  unsigned int alignment, nbytes;

	  if (ELF32_R_TYPE (erel->r_info) != R_RL78_RH_RELAX)
	    continue;
	  if (!(erel->r_addend & RL78_RELAXA_ELIGN))
	    continue;

	  alignment = 1 << (irel->r_addend & RL78_RELAXA_ANUM);

	  if (erel->r_offset - irel->r_offset < alignment)
	    continue;

	  nbytes = erel->r_offset - irel->r_offset;
	  nbytes /= alignment;
	  nbytes *= alignment;

	  elf32_rl78_relax_delete_bytes (abfd, sec, erel->r_offset - nbytes, nbytes,
					 next_alignment, erel->r_offset == sec->size);
	  *again = true;

	  continue;
	}

      if (irel->r_addend & RL78_RELAXA_ELIGN)
	  continue;

      insn = contents + irel->r_offset;

      nrelocs = irel->r_addend & RL78_RELAXA_RNUM;

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

#define GET_RELOC					\
      BFD_ASSERT (nrelocs > 0);				\
      symval = OFFSET_FOR_RELOC (srel, &srel, &scale);	\
      pcrel = symval - pc + srel->r_addend;		\
      nrelocs --;

#define SNIPNR(offset, nbytes) \
	elf32_rl78_relax_delete_bytes (abfd, sec, (insn - contents) + offset, nbytes, next_alignment, 0);

#define SNIP(offset, nbytes, newtype)					\
	SNIPNR (offset, nbytes);					\
	srel->r_info = ELF32_R_INFO (ELF32_R_SYM (srel->r_info), newtype)

      /* The order of these bit tests must match the order that the
	 relocs appear in.  Since we sorted those by offset, we can
	 predict them.  */

      /*----------------------------------------------------------------------*/
      /* EF ad		BR $rel8	pcrel
	 ED al ah	BR !abs16	abs
	 EE al ah	BR $!rel16	pcrel
	 EC al ah as	BR !!abs20	abs

	 FD al ah	CALL !abs16	abs
	 FE al ah	CALL $!rel16	pcrel
	 FC al ah as	CALL !!abs20	abs

	 DC ad		BC  $rel8
	 DE ad		BNC $rel8
	 DD ad		BZ  $rel8
	 DF ad		BNZ $rel8
	 61 C3 ad	BH  $rel8
	 61 D3 ad	BNH $rel8
	 61 C8 EF ad	SKC  ; BR $rel8
	 61 D8 EF ad	SKNC ; BR $rel8
	 61 E8 EF ad	SKZ  ; BR $rel8
	 61 F8 EF ad	SKNZ ; BR $rel8
	 61 E3 EF ad	SKH  ; BR $rel8
	 61 F3 EF ad	SKNH ; BR $rel8
       */

      if ((irel->r_addend & RL78_RELAXA_MASK) == RL78_RELAXA_BRA)
	{
	  /* SKIP opcodes that skip non-branches will have a relax tag
	     but no corresponding symbol to relax against; we just
	     skip those.  */
	  if (irel->r_addend & RL78_RELAXA_RNUM)
	    {
	      GET_RELOC;
	    }

	  switch (insn[0])
	    {
	    case 0xdc: /* BC */
	    case 0xdd: /* BZ */
	    case 0xde: /* BNC */
	    case 0xdf: /* BNZ */
	      if (insn[1] == 0x03 && insn[2] == 0xee /* BR */
		  && (srel->r_offset - irel->r_offset) > 1) /* a B<c> without its own reloc */
		{
		  /* This is a "long" conditional as generated by gas:
		     DC 03 EE ad.dr  */
		  if (pcrel < 127
		      && pcrel > -127)
		    {
		      insn[0] ^= 0x02; /* invert conditional */
		      SNIPNR (4, 1);
		      SNIP (1, 2, R_RL78_DIR8S_PCREL);
		      insn[1] = pcrel;
		      *again = true;
		    }
		}
	      break;

	    case 0xec: /* BR !!abs20 */

	      if (pcrel < 127
		  && pcrel > -127)
		{
		  insn[0] = 0xef;
		  insn[1] = pcrel;
		  SNIP (2, 2, R_RL78_DIR8S_PCREL);
		  *again = true;
		}
	      else if (symval < 65536)
		{
		  insn[0] = 0xed;
		  insn[1] = symval & 0xff;
		  insn[2] = symval >> 8;
		  SNIP (2, 1, R_RL78_DIR16U);
		  *again = true;
		}
	      else if (pcrel < 32767
		       && pcrel > -32767)
		{
		  insn[0] = 0xee;
		  insn[1] = pcrel & 0xff;
		  insn[2] = pcrel >> 8;
		  SNIP (2, 1, R_RL78_DIR16S_PCREL);
		  *again = true;
		}
	      break;

	    case 0xee: /* BR $!pcrel16 */
	    case 0xed: /* BR $!abs16 */
	      if (pcrel < 127
		  && pcrel > -127)
		{
		  insn[0] = 0xef;
		  insn[1] = pcrel;
		  SNIP (2, 1, R_RL78_DIR8S_PCREL);
		  *again = true;
		}
	      break;

	    case 0xfc: /* CALL !!abs20 */
	      if (symval < 65536)
		{
		  insn[0] = 0xfd;
		  insn[1] = symval & 0xff;
		  insn[2] = symval >> 8;
		  SNIP (2, 1, R_RL78_DIR16U);
		  *again = true;
		}
	      else if (pcrel < 32767
		       && pcrel > -32767)
		{
		  insn[0] = 0xfe;
		  insn[1] = pcrel & 0xff;
		  insn[2] = pcrel >> 8;
		  SNIP (2, 1, R_RL78_DIR16S_PCREL);
		  *again = true;
		}
	      break;

	    case 0x61: /* PREFIX */
	      /* For SKIP/BR, we change the BR opcode and delete the
		 SKIP.  That way, we don't have to find and change the
		 relocation for the BR.  */
	      /* Note that, for the case where we're skipping some
		 other insn, we have no "other" reloc but that's safe
		 here anyway. */
	      switch (insn[1])
		{
		case 0xd3: /* BNH */
		case 0xc3: /* BH */
		  if (insn[2] == 0x03 && insn[3] == 0xee
		      && (srel->r_offset - irel->r_offset) > 2) /* a B<c> without its own reloc */
		    {
		      /* Another long branch by gas:
			 61 D3 03 EE ad.dr  */
		      if (pcrel < 127
			  && pcrel > -127)
			{
			  insn[1] ^= 0x10; /* invert conditional */
			  SNIPNR (5, 1);
			  SNIP (2, 2, R_RL78_DIR8S_PCREL);
			  insn[2] = pcrel;
			  *again = true;
			}
		    }
		  break;

		case 0xc8: /* SKC */
		  if (insn[2] == 0xef)
		    {
		      insn[2] = 0xde; /* BNC */
		      SNIPNR (0, 2);
		    }
		  break;

		case 0xd8: /* SKNC */
		  if (insn[2] == 0xef)
		    {
		      insn[2] = 0xdc; /* BC */
		      SNIPNR (0, 2);
		    }
		  break;

		case 0xe8: /* SKZ */
		  if (insn[2] == 0xef)
		    {
		      insn[2] = 0xdf; /* BNZ */
		      SNIPNR (0, 2);
		    }
		  break;

		case 0xf8: /* SKNZ */
		  if (insn[2] == 0xef)
		    {
		      insn[2] = 0xdd; /* BZ */
		      SNIPNR (0, 2);
		    }
		  break;

		case 0xe3: /* SKH */
		  if (insn[2] == 0xef)
		    {
		      insn[2] = 0xd3; /* BNH */
		      SNIPNR (1, 1); /* we reuse the 0x61 prefix from the SKH */
		    }
		  break;

		case 0xf3: /* SKNH */
		  if (insn[2] == 0xef)
		    {
		      insn[2] = 0xc3; /* BH */
		      SNIPNR (1, 1); /* we reuse the 0x61 prefix from the SKH */
		    }
		  break;
		}
	      break;
	    }
	}

      if ((irel->r_addend &  RL78_RELAXA_MASK) == RL78_RELAXA_ADDR16
	  && nrelocs > 0)
	{
	  /*----------------------------------------------------------------------*/
	  /* Some insns have both a 16-bit address operand and an 8-bit
	     variant if the address is within a special range:

	     Address		16-bit operand	SADDR range	SFR range
	     FFF00-FFFFF	0xff00-0xffff	0x00-0xff
	     FFE20-FFF1F	0xfe20-0xff1f			0x00-0xff

	     The RELAX_ADDR16[] array has the insn encodings for the
	     16-bit operand version, as well as the SFR and SADDR
	     variants.  We only need to replace the encodings and
	     adjust the operand.

	     Note: we intentionally do not attempt to decode and skip
	     any ES: prefix, as adding ES: means the addr16 (likely)
	     no longer points to saddr/sfr space.
	  */

	  int is_sfr;
	  int is_saddr;
	  int idx;
	  int poff;

	  GET_RELOC;

	  if (0xffe20 <= symval && symval <= 0xfffff)
	    {

	      is_saddr = (0xffe20 <= symval && symval <= 0xfff1f);
	      is_sfr   = (0xfff00 <= symval && symval <= 0xfffff);

	      for (idx = 0; relax_addr16[idx].insn != -1; idx ++)
		{
		  if (relax_addr16[idx].prefix != -1
		      && insn[0] == relax_addr16[idx].prefix
		      && insn[1] == relax_addr16[idx].insn)
		    {
		      poff = 1;
		    }
		  else if (relax_addr16[idx].prefix == -1
			   && insn[0] == relax_addr16[idx].insn)
		    {
		      poff = 0;
		    }
		  else
		    continue;

		  /* We have a matched insn, and poff is 0 or 1 depending
		     on the base pattern size.  */

		  if (is_sfr && relax_addr16[idx].insn_for_sfr != -1)
		    {
		      insn[poff] = relax_addr16[idx].insn_for_sfr;
		      SNIP (poff+2, 1, R_RL78_RH_SFR);
		    }

		  else if  (is_saddr && relax_addr16[idx].insn_for_saddr != -1)
		    {
		      insn[poff] = relax_addr16[idx].insn_for_saddr;
		      SNIP (poff+2, 1, R_RL78_RH_SADDR);
		    }
		}
	    }
	}
      /*----------------------------------------------------------------------*/
    }

  return true;

 error_return:
  free (free_relocs);
  free (free_contents);

  if (shndx_buf != NULL)
    {
      shndx_hdr->contents = NULL;
      free (shndx_buf);
    }

  free (free_intsyms);

  return true;
}



#define ELF_ARCH		bfd_arch_rl78
#define ELF_MACHINE_CODE	EM_RL78
#define ELF_MAXPAGESIZE		0x1000

#define TARGET_LITTLE_SYM	rl78_elf32_vec
#define TARGET_LITTLE_NAME	"elf32-rl78"

#define elf_info_to_howto_rel			NULL
#define elf_info_to_howto			rl78_info_to_howto_rela
#define elf_backend_object_p			rl78_elf_object_p
#define elf_backend_relocate_section		rl78_elf_relocate_section
#define elf_symbol_leading_char			('_')
#define elf_backend_can_gc_sections		1

#define bfd_elf32_bfd_reloc_type_lookup		rl78_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		rl78_reloc_name_lookup
#define bfd_elf32_bfd_set_private_flags		rl78_elf_set_private_flags
#define bfd_elf32_bfd_merge_private_bfd_data	rl78_elf_merge_private_bfd_data
#define bfd_elf32_bfd_print_private_bfd_data	rl78_elf_print_private_bfd_data

#define bfd_elf32_bfd_relax_section		rl78_elf_relax_section
#define elf_backend_check_relocs		rl78_elf_check_relocs
#define elf_backend_always_size_sections \
  rl78_elf_always_size_sections
#define elf_backend_finish_dynamic_sections \
  rl78_elf_finish_dynamic_sections

#include "elf32-target.h"
