/* 32-bit ELF support for S+core.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.
   Contributed by
   Brain.lin (brain.lin@sunplusct.com)
   Mei Ligang (ligang@sunnorth.com.cn)
   Pei-Lin Tsai (pltsai@sunplus.com)

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
#include "libiberty.h"
#include "elf-bfd.h"
#include "elf/score.h"
#include "elf/common.h"
#include "elf/internal.h"
#include "hashtab.h"
#include "elf32-score.h"


int score3 = 0;
int score7 = 1;

/* The SCORE ELF linker needs additional information for each symbol in
   the global hash table.  */
struct score_elf_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* Number of R_SCORE_ABS32, R_SCORE_REL32 relocs against this symbol.  */
  unsigned int possibly_dynamic_relocs;

  /* If the R_SCORE_ABS32, R_SCORE_REL32 reloc is against a readonly section.  */
  bool readonly_reloc;

  /* We must not create a stub for a symbol that has relocations related to
     taking the function's address, i.e. any but R_SCORE_CALL15 ones.  */
  bool no_fn_stub;

  /* Are we forced local?  This will only be set if we have converted
     the initial global GOT entry to a local GOT entry.  */
  bool forced_local;
};

/* Traverse a score ELF linker hash table.  */
#define score_elf_link_hash_traverse(table, func, info) \
  (elf_link_hash_traverse					\
   ((table),							\
    (bool (*) (struct elf_link_hash_entry *, void *)) (func),	\
    (info)))

/* This structure is used to hold .got entries while estimating got sizes.  */
struct score_got_entry
{
  /* The input bfd in which the symbol is defined.  */
  bfd *abfd;
  /* The index of the symbol, as stored in the relocation r_info, if
     we have a local symbol; -1 otherwise.  */
  long symndx;
  union
  {
    /* If abfd == NULL, an address that must be stored in the got.  */
    bfd_vma address;
    /* If abfd != NULL && symndx != -1, the addend of the relocation
       that should be added to the symbol value.  */
    bfd_vma addend;
    /* If abfd != NULL && symndx == -1, the hash table entry
       corresponding to a global symbol in the got (or, local, if
       h->forced_local).  */
    struct score_elf_link_hash_entry *h;
  } d;

  /* The offset from the beginning of the .got section to the entry
     corresponding to this symbol+addend.  If it's a global symbol
     whose offset is yet to be decided, it's going to be -1.  */
  long gotidx;
};

/* This structure is passed to score_elf_sort_hash_table_f when sorting
   the dynamic symbols.  */

struct score_elf_hash_sort_data
{
  /* The symbol in the global GOT with the lowest dynamic symbol table index.  */
  struct elf_link_hash_entry *low;
  /* The least dynamic symbol table index corresponding to a symbol with a GOT entry.  */
  long min_got_dynindx;
  /* The greatest dynamic symbol table index corresponding to a symbol
     with a GOT entry that is not referenced (e.g., a dynamic symbol
     with dynamic relocations pointing to it from non-primary GOTs).  */
  long max_unref_got_dynindx;
  /* The greatest dynamic symbol table index not corresponding to a
     symbol without a GOT entry.  */
  long max_non_got_dynindx;
};

struct score_got_info
{
  /* The global symbol in the GOT with the lowest index in the dynamic
     symbol table.  */
  struct elf_link_hash_entry *global_gotsym;
  /* The number of global .got entries.  */
  unsigned int global_gotno;
  /* The number of local .got entries.  */
  unsigned int local_gotno;
  /* The number of local .got entries we have used.  */
  unsigned int assigned_gotno;
  /* A hash table holding members of the got.  */
  struct htab *got_entries;
  /* In multi-got links, a pointer to the next got (err, rather, most
     of the time, it points to the previous got).  */
  struct score_got_info *next;
};

/* A structure used to count GOT entries, for GOT entry or ELF symbol table traversal.  */
struct _score_elf_section_data
{
  struct bfd_elf_section_data elf;
  union
  {
    struct score_got_info *got_info;
    bfd_byte *tdata;
  }
  u;
};

#define score_elf_section_data(sec) \
  ((struct _score_elf_section_data *) elf_section_data (sec))

/* The size of a symbol-table entry.  */
#define SCORE_ELF_SYM_SIZE(abfd)  \
  (get_elf_backend_data (abfd)->s->sizeof_sym)

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value
   from smaller values.  Start with zero, widen, *then* decrement.  */
#define MINUS_ONE (((bfd_vma)0) - 1)
#define MINUS_TWO (((bfd_vma)0) - 2)

#define PDR_SIZE 32


/* The number of local .got entries we reserve.  */
#define SCORE_RESERVED_GOTNO (2)
#define ELF_DYNAMIC_INTERPRETER     "/usr/lib/ld.so.1"

/* The offset of $gp from the beginning of the .got section.  */
#define ELF_SCORE_GP_OFFSET(abfd) (0x3ff0)
/* The maximum size of the GOT for it to be addressable using 15-bit offsets from $gp.  */
#define SCORE_ELF_GOT_MAX_SIZE(abfd) (ELF_SCORE_GP_OFFSET(abfd) + 0x3fff)

#define SCORE_ELF_STUB_SECTION_NAME  (".SCORE.stub")
#define SCORE_FUNCTION_STUB_SIZE (16)

#define STUB_LW      0xc3bcc010     /* lw r29, [r28, -0x3ff0]  */
#define STUB_MOVE    0x8363bc56     /* mv r27, r3  */
#define STUB_LI16    0x87548000     /* ori r26, .dynsym_index  */
#define STUB_BRL     0x801dbc09     /* brl r29  */

#define SCORE_ELF_GOT_SIZE(abfd)   \
  (get_elf_backend_data (abfd)->s->arch_size / 8)

#define SCORE_ELF_ADD_DYNAMIC_ENTRY(info, tag, val) \
	(_bfd_elf_add_dynamic_entry (info, (bfd_vma) tag, (bfd_vma) val))

/* The size of an external dynamic table entry.  */
#define SCORE_ELF_DYN_SIZE(abfd) \
  (get_elf_backend_data (abfd)->s->sizeof_dyn)

/* The size of an external REL relocation.  */
#define SCORE_ELF_REL_SIZE(abfd) \
  (get_elf_backend_data (abfd)->s->sizeof_rel)

/* The default alignment for sections, as a power of two.  */
#define SCORE_ELF_LOG_FILE_ALIGN(abfd)\
  (get_elf_backend_data (abfd)->s->log_file_align)

static bfd_byte *hi16_rel_addr;

/* This will be used when we sort the dynamic relocation records.  */
static bfd *reldyn_sorting_bfd;

/* SCORE ELF uses two common sections.  One is the usual one, and the
   other is for small objects.  All the small objects are kept
   together, and then referenced via the gp pointer, which yields
   faster assembler code.  This is what we use for the small common
   section.  This approach is copied from ecoff.c.  */
static asection score_elf_scom_section;
static const asymbol score_elf_scom_symbol =
  GLOBAL_SYM_INIT (".scommon", &score_elf_scom_section);
static asection score_elf_scom_section =
  BFD_FAKE_SECTION (score_elf_scom_section, &score_elf_scom_symbol,
		    ".scommon", 0, SEC_IS_COMMON | SEC_SMALL_DATA);

static bfd_vma
score_bfd_get_16 (bfd *abfd, const void *data)
{
  return bfd_get_16 (abfd, data);
}

static bfd_vma
score3_bfd_getl32 (const void *p)
{
  const bfd_byte *addr = p;
  unsigned long v;

  v = (unsigned long) addr[2];
  v |= (unsigned long) addr[3] << 8;
  v |= (unsigned long) addr[0] << 16;
  v |= (unsigned long) addr[1] << 24;
  return v;
}

static bfd_vma
score3_bfd_getl48 (const void *p)
{
  const bfd_byte *addr = p;
  uint64_t v;

  v = (uint64_t) addr[4];
  v |= (uint64_t) addr[5] << 8;
  v |= (uint64_t) addr[2] << 16;
  v |= (uint64_t) addr[3] << 24;
  v |= (uint64_t) addr[0] << 32;
  v |= (uint64_t) addr[1] << 40;
  return v;
}

static bfd_vma
score_bfd_get_32 (bfd *abfd, const void *data)
{
  if (/* score3 && */ abfd->xvec->byteorder == BFD_ENDIAN_LITTLE)
    return score3_bfd_getl32 (data);
  else
    return bfd_get_32 (abfd, data);
}

static bfd_vma
score_bfd_get_48 (bfd *abfd, const void *p)
{
  if (/* score3 && */ abfd->xvec->byteorder == BFD_ENDIAN_LITTLE)
    return score3_bfd_getl48 (p);
  else
    return bfd_get_bits (p, 48, 1);
}

static void
score_bfd_put_16 (bfd *abfd, bfd_vma addr, void *data)
{
  return bfd_put_16 (abfd, addr, data);
}

static void
score3_bfd_putl32 (bfd_vma data, void *p)
{
  bfd_byte *addr = p;
  addr[0] = (data >> 16) & 0xff;
  addr[1] = (data >> 24) & 0xff;
  addr[2] = data & 0xff;
  addr[3] = (data >>  8) & 0xff;
}

static void
score3_bfd_putl48 (bfd_vma data, void *p)
{
  bfd_byte *addr = p;
  addr[0] = (data >> 32) & 0xff;
  addr[1] = (data >> 40) & 0xff;
  addr[2] = (data >> 16) & 0xff;
  addr[3] = (data >> 24) & 0xff;
  addr[4] = data & 0xff;
  addr[5] = (data >>  8) & 0xff;
}

static void
score_bfd_put_32 (bfd *abfd, bfd_vma addr, void *data)
{
  if (/* score3 && */ abfd->xvec->byteorder == BFD_ENDIAN_LITTLE)
    return score3_bfd_putl32 (addr, data);
  else
    return bfd_put_32 (abfd, addr, data);
}

static void
score_bfd_put_48 (bfd *abfd, bfd_vma val, void *p)
{
  if (/* score3 && */ abfd->xvec->byteorder == BFD_ENDIAN_LITTLE)
    return score3_bfd_putl48 (val, p);
  else
    return bfd_put_bits (val, p, 48, 1);
}

static bfd_reloc_status_type
score_elf_hi16_reloc (bfd *abfd ATTRIBUTE_UNUSED,
		      arelent *reloc_entry,
		      asymbol *symbol ATTRIBUTE_UNUSED,
		      void * data,
		      asection *input_section ATTRIBUTE_UNUSED,
		      bfd *output_bfd ATTRIBUTE_UNUSED,
		      char **error_message ATTRIBUTE_UNUSED)
{
  hi16_rel_addr = (bfd_byte *) data + reloc_entry->address;
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
score_elf_lo16_reloc (bfd *abfd,
		      arelent *reloc_entry,
		      asymbol *symbol ATTRIBUTE_UNUSED,
		      void * data,
		      asection *input_section,
		      bfd *output_bfd ATTRIBUTE_UNUSED,
		      char **error_message ATTRIBUTE_UNUSED)
{
  bfd_vma addend = 0, offset = 0;
  unsigned long val;
  unsigned long hi16_offset, hi16_value, uvalue;

  hi16_value = score_bfd_get_32 (abfd, hi16_rel_addr);
  hi16_offset = ((((hi16_value >> 16) & 0x3) << 15) | (hi16_value & 0x7fff)) >> 1;
  addend = score_bfd_get_32 (abfd, (bfd_byte *) data + reloc_entry->address);
  offset = ((((addend >> 16) & 0x3) << 15) | (addend & 0x7fff)) >> 1;
  val = reloc_entry->addend;
  if (reloc_entry->address > input_section->size)
    return bfd_reloc_outofrange;
  uvalue = ((hi16_offset << 16) | (offset & 0xffff)) + val;
  hi16_offset = (uvalue >> 16) << 1;
  hi16_value = (hi16_value & ~0x37fff) | (hi16_offset & 0x7fff) | ((hi16_offset << 1) & 0x30000);
  score_bfd_put_32 (abfd, hi16_value, hi16_rel_addr);
  offset = (uvalue & 0xffff) << 1;
  addend = (addend & ~0x37fff) | (offset & 0x7fff) | ((offset << 1) & 0x30000);
  score_bfd_put_32 (abfd, addend, (bfd_byte *) data + reloc_entry->address);
  return bfd_reloc_ok;
}

/* Set the GP value for OUTPUT_BFD.  Returns FALSE if this is a
   dangerous relocation.  */

static bool
score_elf_assign_gp (bfd *output_bfd, bfd_vma *pgp)
{
  unsigned int count;
  asymbol **sym;
  unsigned int i;

  /* If we've already figured out what GP will be, just return it.  */
  *pgp = _bfd_get_gp_value (output_bfd);
  if (*pgp)
    return true;

  count = bfd_get_symcount (output_bfd);
  sym = bfd_get_outsymbols (output_bfd);

  /* The linker script will have created a symbol named `_gp' with the
     appropriate value.  */
  if (sym == NULL)
    i = count;
  else
    {
      for (i = 0; i < count; i++, sym++)
	{
	  const char *name;

	  name = bfd_asymbol_name (*sym);
	  if (*name == '_' && strcmp (name, "_gp") == 0)
	    {
	      *pgp = bfd_asymbol_value (*sym);
	      _bfd_set_gp_value (output_bfd, *pgp);
	      break;
	    }
	}
    }

  if (i >= count)
    {
      /* Only get the error once.  */
      *pgp = 4;
      _bfd_set_gp_value (output_bfd, *pgp);
      return false;
    }

  return true;
}

/* We have to figure out the gp value, so that we can adjust the
   symbol value correctly.  We look up the symbol _gp in the output
   BFD.  If we can't find it, we're stuck.  We cache it in the ELF
   target data.  We don't need to adjust the symbol value for an
   external symbol if we are producing relocatable output.  */
static bfd_reloc_status_type
score_elf_final_gp (bfd *output_bfd,
		    asymbol *symbol,
		    bool relocatable,
		     char **error_message,
		    bfd_vma *pgp)
{
  if (bfd_is_und_section (symbol->section)
      && ! relocatable)
    {
      *pgp = 0;
      return bfd_reloc_undefined;
    }

  *pgp = _bfd_get_gp_value (output_bfd);
  if (*pgp == 0
      && (! relocatable
	  || (symbol->flags & BSF_SECTION_SYM) != 0))
    {
      if (relocatable)
	{
	  /* Make up a value.  */
	  *pgp = symbol->section->output_section->vma + 0x4000;
	  _bfd_set_gp_value (output_bfd, *pgp);
	}
      else if (!score_elf_assign_gp (output_bfd, pgp))
	{
	    *error_message =
	      (char *) _("GP relative relocation when _gp not defined");
	    return bfd_reloc_dangerous;
	}
    }

  return bfd_reloc_ok;
}

static bfd_reloc_status_type
score_elf_gprel15_with_gp (bfd *abfd,
			   arelent *reloc_entry,
			   asection *input_section,
			   bool relocateable,
			   void * data,
			   bfd_vma gp ATTRIBUTE_UNUSED)
{
  unsigned long insn;

  if (reloc_entry->address > input_section->size)
    return bfd_reloc_outofrange;

  insn = score_bfd_get_32 (abfd, (bfd_byte *) data + reloc_entry->address);
  if (((reloc_entry->addend & 0xffffc000) != 0)
      && ((reloc_entry->addend & 0xffffc000) != 0xffffc000))
    return bfd_reloc_overflow;

  insn = (insn & ~0x7fff) | (reloc_entry->addend & 0x7fff);
  score_bfd_put_32 (abfd, insn, (bfd_byte *) data + reloc_entry->address);
  if (relocateable)
    reloc_entry->address += input_section->output_offset;

  return bfd_reloc_ok;
}

static bfd_reloc_status_type
gprel32_with_gp (bfd *abfd, asymbol *symbol, arelent *reloc_entry,
		 asection *input_section, bool relocatable,
		 void *data, bfd_vma gp)
{
  bfd_vma relocation;
  bfd_vma val;

  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  relocation += symbol->section->output_section->vma;
  relocation += symbol->section->output_offset;

  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  /* Set val to the offset into the section or symbol.  */
  val = reloc_entry->addend;

  if (reloc_entry->howto->partial_inplace)
    val += score_bfd_get_32 (abfd, (bfd_byte *) data + reloc_entry->address);

  /* Adjust val for the final section location and GP value.  If we
     are producing relocatable output, we don't want to do this for
     an external symbol.  */
  if (! relocatable
      || (symbol->flags & BSF_SECTION_SYM) != 0)
    val += relocation - gp;

  if (reloc_entry->howto->partial_inplace)
    score_bfd_put_32 (abfd, val, (bfd_byte *) data + reloc_entry->address);
  else
    reloc_entry->addend = val;

  if (relocatable)
    reloc_entry->address += input_section->output_offset;

  return bfd_reloc_ok;
}

static bfd_reloc_status_type
score_elf_gprel15_reloc (bfd *abfd,
			 arelent *reloc_entry,
			 asymbol *symbol,
			 void * data,
			 asection *input_section,
			 bfd *output_bfd,
			 char **error_message)
{
  bool relocateable;
  bfd_reloc_status_type ret;
  bfd_vma gp;

  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0 && reloc_entry->addend == 0)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }
  if (output_bfd != NULL)
    relocateable = true;
  else
    {
      relocateable = false;
      output_bfd = symbol->section->output_section->owner;
    }

  ret = score_elf_final_gp (output_bfd, symbol, relocateable, error_message, &gp);
  if (ret != bfd_reloc_ok)
    return ret;

  return score_elf_gprel15_with_gp (abfd, reloc_entry,
				    input_section, relocateable, data, gp);
}

/* Do a R_SCORE_GPREL32 relocation.  This is a 32 bit value which must
   become the offset from the gp register.  */

static bfd_reloc_status_type
score_elf_gprel32_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			void *data, asection *input_section, bfd *output_bfd,
			char **error_message)
{
  bool relocatable;
  bfd_reloc_status_type ret;
  bfd_vma gp;

  /* R_SCORE_GPREL32 relocations are defined for local symbols only.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (symbol->flags & BSF_LOCAL) != 0)
    {
      *error_message = (char *)
	_("32bits gp relative relocation occurs for an external symbol");
      return bfd_reloc_outofrange;
    }

  if (output_bfd != NULL)
    relocatable = true;
  else
    {
      relocatable = false;
      output_bfd = symbol->section->output_section->owner;
    }

  ret = score_elf_final_gp (output_bfd, symbol, relocatable, error_message, &gp);
  if (ret != bfd_reloc_ok)
    return ret;

  gp = 0;
  return gprel32_with_gp (abfd, symbol, reloc_entry, input_section,
			  relocatable, data, gp);
}

/* A howto special_function for R_SCORE_GOT15 relocations.  This is just
   like any other 16-bit relocation when applied to global symbols, but is
   treated in the same as R_SCORE_HI16 when applied to local symbols.  */
static bfd_reloc_status_type
score_elf_got15_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
		       void *data, asection *input_section,
		       bfd *output_bfd, char **error_message)
{
  if ((symbol->flags & (BSF_GLOBAL | BSF_WEAK)) != 0
      || bfd_is_und_section (bfd_asymbol_section (symbol))
      || bfd_is_com_section (bfd_asymbol_section (symbol)))
    /* The relocation is against a global symbol.  */
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd,
				  error_message);

  return score_elf_hi16_reloc (abfd, reloc_entry, symbol, data,
			       input_section, output_bfd, error_message);
}

static bfd_reloc_status_type
score_elf_got_lo16_reloc (bfd *abfd,
			  arelent *reloc_entry,
			  asymbol *symbol ATTRIBUTE_UNUSED,
			  void * data,
			  asection *input_section,
			  bfd *output_bfd ATTRIBUTE_UNUSED,
			  char **error_message ATTRIBUTE_UNUSED)
{
  bfd_vma addend = 0, offset = 0;
  signed long val;
  signed long hi16_offset, hi16_value, uvalue;

  hi16_value = score_bfd_get_32 (abfd, hi16_rel_addr);
  hi16_offset = ((((hi16_value >> 16) & 0x3) << 15) | (hi16_value & 0x7fff)) >> 1;
  addend = score_bfd_get_32 (abfd, (bfd_byte *) data + reloc_entry->address);
  offset = ((((addend >> 16) & 0x3) << 15) | (addend & 0x7fff)) >> 1;
  val = reloc_entry->addend;
  if (reloc_entry->address > input_section->size)
    return bfd_reloc_outofrange;
  uvalue = ((hi16_offset << 16) | (offset & 0xffff)) + val;
  if ((uvalue > -0x8000) && (uvalue < 0x7fff))
    hi16_offset = 0;
  else
    hi16_offset = (uvalue >> 16) & 0x7fff;
  hi16_value = (hi16_value & ~0x37fff) | (hi16_offset & 0x7fff) | ((hi16_offset << 1) & 0x30000);
  score_bfd_put_32 (abfd, hi16_value, hi16_rel_addr);
  offset = (uvalue & 0xffff) << 1;
  addend = (addend & ~0x37fff) | (offset & 0x7fff) | ((offset << 1) & 0x30000);
  score_bfd_put_32 (abfd, addend, (bfd_byte *) data + reloc_entry->address);
  return bfd_reloc_ok;
}

static reloc_howto_type elf32_score_howto_table[] =
{
  /* No relocation.  */
  HOWTO (R_SCORE_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* R_SCORE_HI16 */
  HOWTO (R_SCORE_HI16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 1,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 score_elf_hi16_reloc,	/* special_function */
	 "R_SCORE_HI16",	/* name */
	 true,			/* partial_inplace */
	 0x37fff,		/* src_mask */
	 0x37fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* R_SCORE_LO16 */
  HOWTO (R_SCORE_LO16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 1,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 score_elf_lo16_reloc,	/* special_function */
	 "R_SCORE_LO16",	/* name */
	 true,			/* partial_inplace */
	 0x37fff,		/* src_mask */
	 0x37fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /*  R_SCORE_BCMP */
  HOWTO (R_SCORE_BCMP,		/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 1,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_BCMP",	/* name */
	 false,			/* partial_inplace */
	 0x03e00381,		/* src_mask */
	 0x03e00381,		/* dst_mask */
	 false),		/* pcrel_offset */

  /*R_SCORE_24 */
  HOWTO (R_SCORE_24,		/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 24,			/* bitsize */
	 false,			/* pc_relative */
	 1,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_24",		/* name */
	 false,			/* partial_inplace */
	 0x3ff7fff,		/* src_mask */
	 0x3ff7fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /*R_SCORE_PC19 */
  HOWTO (R_SCORE_PC19,		/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 19,			/* bitsize */
	 true,			/* pc_relative */
	 1,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_PC19",	/* name */
	 false,			/* partial_inplace */
	 0x3ff03fe,		/* src_mask */
	 0x3ff03fe,		/* dst_mask */
	 false),		/* pcrel_offset */

  /*R_SCORE16_11 */
  HOWTO (R_SCORE16_11,		/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 11,			/* bitsize */
	 false,			/* pc_relative */
	 1,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE16_11",	/* name */
	 false,			/* partial_inplace */
	 0x000000ffe,		/* src_mask */
	 0x000000ffe,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* R_SCORE16_PC8 */
  HOWTO (R_SCORE16_PC8,		/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 9,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE16_PC8",	/* name */
	 false,			/* partial_inplace */
	 0x000001ff,		/* src_mask */
	 0x000001ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit absolute */
  HOWTO (R_SCORE_ABS32,		/* type	 8 */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_ABS32",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 16 bit absolute */
  HOWTO (R_SCORE_ABS16,		/* type 11 */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_ABS16",	/* name */
	 false,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* R_SCORE_DUMMY2 */
  HOWTO (R_SCORE_DUMMY2,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_DUMMY2",	/* name */
	 true,			/* partial_inplace */
	 0x00007fff,		/* src_mask */
	 0x00007fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* R_SCORE_GP15 */
  HOWTO (R_SCORE_GP15,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 score_elf_gprel15_reloc,/* special_function */
	 "R_SCORE_GP15",	/* name */
	 true,			/* partial_inplace */
	 0x00007fff,		/* src_mask */
	 0x00007fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_SCORE_GNU_VTINHERIT, /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_SCORE_GNU_VTINHERIT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* GNU extension to record C++ vtable member usage */
  HOWTO (R_SCORE_GNU_VTENTRY,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn,	/* special_function */
	 "R_SCORE_GNU_VTENTRY", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* Reference to global offset table.  */
  HOWTO (R_SCORE_GOT15,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 score_elf_got15_reloc, /* special_function */
	 "R_SCORE_GOT15",	/* name */
	 true,			/* partial_inplace */
	 0x00007fff,		/* src_mask */
	 0x00007fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_SCORE_GOT_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 1,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 score_elf_got_lo16_reloc, /* special_function */
	 "R_SCORE_GOT_LO16",	/* name */
	 true,			/* partial_inplace */
	 0x37ffe,		/* src_mask */
	 0x37ffe,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 15 bit call through global offset table.  */
  HOWTO (R_SCORE_CALL15,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_CALL15",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit GP relative reference.  */
  HOWTO (R_SCORE_GPREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 score_elf_gprel32_reloc, /* special_function */
	 "R_SCORE_GPREL32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit symbol relative relocation.  */
  HOWTO (R_SCORE_REL32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_REL32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* R_SCORE_DUMMY_HI16 */
  HOWTO (R_SCORE_DUMMY_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 1,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 score_elf_hi16_reloc,	/* special_function */
	 "R_SCORE_DUMMY_HI16",	/* name */
	 true,			/* partial_inplace */
	 0x37fff,		/* src_mask */
	 0x37fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* R_SCORE_IMM30 */
  HOWTO (R_SCORE_IMM30,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 30,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_IMM30",	/* name */
	 false,			/* partial_inplace */
	 0x7f7fff7f80LL,	/* src_mask */
	 0x7f7fff7f80LL,	/* dst_mask */
	 false),		/* pcrel_offset */

  /* R_SCORE_IMM32 */
  HOWTO (R_SCORE_IMM32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 5,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_SCORE_IMM32",	/* name */
	 false,			/* partial_inplace */
	 0x7f7fff7fe0LL,	/* src_mask */
	 0x7f7fff7fe0LL,	/* dst_mask */
	 false),		/* pcrel_offset */
};

struct score_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct score_reloc_map elf32_score_reloc_map[] =
{
  {BFD_RELOC_NONE,		 R_SCORE_NONE},
  {BFD_RELOC_HI16_S,		 R_SCORE_HI16},
  {BFD_RELOC_LO16,		 R_SCORE_LO16},
  {BFD_RELOC_SCORE_BCMP,	 R_SCORE_BCMP},
  {BFD_RELOC_SCORE_JMP,		 R_SCORE_24},
  {BFD_RELOC_SCORE_BRANCH,	 R_SCORE_PC19},
  {BFD_RELOC_SCORE16_JMP,	 R_SCORE16_11},
  {BFD_RELOC_SCORE16_BRANCH,	 R_SCORE16_PC8},
  {BFD_RELOC_32,		 R_SCORE_ABS32},
  {BFD_RELOC_16,		 R_SCORE_ABS16},
  {BFD_RELOC_SCORE_DUMMY2,	 R_SCORE_DUMMY2},
  {BFD_RELOC_SCORE_GPREL15,	 R_SCORE_GP15},
  {BFD_RELOC_VTABLE_INHERIT,	 R_SCORE_GNU_VTINHERIT},
  {BFD_RELOC_VTABLE_ENTRY,	 R_SCORE_GNU_VTENTRY},
  {BFD_RELOC_SCORE_GOT15,	 R_SCORE_GOT15},
  {BFD_RELOC_SCORE_GOT_LO16,	 R_SCORE_GOT_LO16},
  {BFD_RELOC_SCORE_CALL15,	 R_SCORE_CALL15},
  {BFD_RELOC_GPREL32,		 R_SCORE_GPREL32},
  {BFD_RELOC_32_PCREL,		 R_SCORE_REL32},
  {BFD_RELOC_SCORE_DUMMY_HI16,	 R_SCORE_DUMMY_HI16},
  {BFD_RELOC_SCORE_IMM30,	 R_SCORE_IMM30},
  {BFD_RELOC_SCORE_IMM32,	 R_SCORE_IMM32},
};

/* got_entries only match if they're identical, except for gotidx, so
   use all fields to compute the hash, and compare the appropriate
   union members.  */
static hashval_t
score_elf_got_entry_hash (const void *entry_)
{
  const struct score_got_entry *entry = (struct score_got_entry *)entry_;

  return entry->symndx
    + (!entry->abfd ? entry->d.address : entry->abfd->id);
}

static int
score_elf_got_entry_eq (const void *entry1, const void *entry2)
{
  const struct score_got_entry *e1 = (struct score_got_entry *)entry1;
  const struct score_got_entry *e2 = (struct score_got_entry *)entry2;

  return e1->abfd == e2->abfd && e1->symndx == e2->symndx
    && (! e1->abfd ? e1->d.address == e2->d.address
	: e1->symndx >= 0 ? e1->d.addend == e2->d.addend
	: e1->d.h == e2->d.h);
}

/* If H needs a GOT entry, assign it the highest available dynamic
   index.  Otherwise, assign it the lowest available dynamic
   index.  */
static bool
score_elf_sort_hash_table_f (struct score_elf_link_hash_entry *h, void *data)
{
  struct score_elf_hash_sort_data *hsd = data;

  /* Symbols without dynamic symbol table entries aren't interesting at all.  */
  if (h->root.dynindx == -1)
    return true;

  /* Global symbols that need GOT entries that are not explicitly
     referenced are marked with got offset 2.  Those that are
     referenced get a 1, and those that don't need GOT entries get
     -1.  */
  if (h->root.got.offset == 2)
    {
      if (hsd->max_unref_got_dynindx == hsd->min_got_dynindx)
	hsd->low = (struct elf_link_hash_entry *) h;
      h->root.dynindx = hsd->max_unref_got_dynindx++;
    }
  else if (h->root.got.offset != 1)
    h->root.dynindx = hsd->max_non_got_dynindx++;
  else
    {
      h->root.dynindx = --hsd->min_got_dynindx;
      hsd->low = (struct elf_link_hash_entry *) h;
    }

  return true;
}

static asection *
score_elf_got_section (bfd *abfd, bool maybe_excluded)
{
  asection *sgot = bfd_get_linker_section (abfd, ".got");

  if (sgot == NULL || (! maybe_excluded && (sgot->flags & SEC_EXCLUDE) != 0))
    return NULL;
  return sgot;
}

/* Returns the GOT information associated with the link indicated by
   INFO.  If SGOTP is non-NULL, it is filled in with the GOT section.  */
static struct score_got_info *
score_elf_got_info (bfd *abfd, asection **sgotp)
{
  asection *sgot;
  struct score_got_info *g;

  sgot = score_elf_got_section (abfd, true);
  BFD_ASSERT (sgot != NULL);
  BFD_ASSERT (elf_section_data (sgot) != NULL);
  g = score_elf_section_data (sgot)->u.got_info;
  BFD_ASSERT (g != NULL);

  if (sgotp)
    *sgotp = sgot;
  return g;
}

/* Sort the dynamic symbol table so that symbols that need GOT entries
   appear towards the end.  This reduces the amount of GOT space
   required.  MAX_LOCAL is used to set the number of local symbols
   known to be in the dynamic symbol table.  During
   s3_bfd_score_elf_size_dynamic_sections, this value is 1.  Afterward, the
   section symbols are added and the count is higher.  */
static bool
score_elf_sort_hash_table (struct bfd_link_info *info,
			   unsigned long max_local)
{
  struct score_elf_hash_sort_data hsd;
  struct score_got_info *g;
  bfd *dynobj;

  dynobj = elf_hash_table (info)->dynobj;

  g = score_elf_got_info (dynobj, NULL);

  hsd.low = NULL;
  hsd.max_unref_got_dynindx =
    hsd.min_got_dynindx = elf_hash_table (info)->dynsymcount
    /* In the multi-got case, assigned_gotno of the master got_info
       indicate the number of entries that aren't referenced in the
       primary GOT, but that must have entries because there are
       dynamic relocations that reference it.  Since they aren't
       referenced, we move them to the end of the GOT, so that they
       don't prevent other entries that are referenced from getting
       too large offsets.  */
    - (g->next ? g->assigned_gotno : 0);
  hsd.max_non_got_dynindx = max_local;
  score_elf_link_hash_traverse (elf_hash_table (info),
				score_elf_sort_hash_table_f,
				&hsd);

  /* There should have been enough room in the symbol table to
     accommodate both the GOT and non-GOT symbols.  */
  BFD_ASSERT (hsd.max_non_got_dynindx <= hsd.min_got_dynindx);
  BFD_ASSERT ((unsigned long)hsd.max_unref_got_dynindx
	      <= elf_hash_table (info)->dynsymcount);

  /* Now we know which dynamic symbol has the lowest dynamic symbol
     table index in the GOT.  */
  g->global_gotsym = hsd.low;

  return true;
}

/* Create an entry in an score ELF linker hash table.  */

static struct bfd_hash_entry *
score_elf_link_hash_newfunc (struct bfd_hash_entry *entry,
			     struct bfd_hash_table *table,
			     const char *string)
{
  struct score_elf_link_hash_entry *ret = (struct score_elf_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a subclass.  */
  if (ret == NULL)
    ret = bfd_hash_allocate (table, sizeof (struct score_elf_link_hash_entry));
  if (ret == NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct score_elf_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret, table, string));

  if (ret != NULL)
    {
      ret->possibly_dynamic_relocs = 0;
      ret->readonly_reloc = false;
      ret->no_fn_stub = false;
      ret->forced_local = false;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Returns the first relocation of type r_type found, beginning with
   RELOCATION.  RELEND is one-past-the-end of the relocation table.  */
static const Elf_Internal_Rela *
score_elf_next_relocation (bfd *abfd ATTRIBUTE_UNUSED, unsigned int r_type,
			    const Elf_Internal_Rela *relocation,
			   const Elf_Internal_Rela *relend)
{
  while (relocation < relend)
    {
      if (ELF32_R_TYPE (relocation->r_info) == r_type)
	return relocation;

      ++relocation;
    }

  /* We didn't find it.  */
  bfd_set_error (bfd_error_bad_value);
  return NULL;
}

/* This function is called via qsort() to sort the dynamic relocation
   entries by increasing r_symndx value.  */
static int
score_elf_sort_dynamic_relocs (const void *arg1, const void *arg2)
{
  Elf_Internal_Rela int_reloc1;
  Elf_Internal_Rela int_reloc2;

  bfd_elf32_swap_reloc_in (reldyn_sorting_bfd, arg1, &int_reloc1);
  bfd_elf32_swap_reloc_in (reldyn_sorting_bfd, arg2, &int_reloc2);

  return (ELF32_R_SYM (int_reloc1.r_info) - ELF32_R_SYM (int_reloc2.r_info));
}

/* Return whether a relocation is against a local symbol.  */
static bool
score_elf_local_relocation_p (bfd *input_bfd,
			      const Elf_Internal_Rela *relocation,
			      asection **local_sections,
			      bool check_forced)
{
  unsigned long r_symndx;
  Elf_Internal_Shdr *symtab_hdr;
  struct score_elf_link_hash_entry *h;
  size_t extsymoff;

  r_symndx = ELF32_R_SYM (relocation->r_info);
  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  extsymoff = (elf_bad_symtab (input_bfd)) ? 0 : symtab_hdr->sh_info;

  if (r_symndx < extsymoff)
    return true;
  if (elf_bad_symtab (input_bfd) && local_sections[r_symndx] != NULL)
    return true;

  if (check_forced)
    {
      /* Look up the hash table to check whether the symbol was forced local.  */
      h = (struct score_elf_link_hash_entry *)
	elf_sym_hashes (input_bfd) [r_symndx - extsymoff];
      /* Find the real hash-table entry for this symbol.  */
      while (h->root.root.type == bfd_link_hash_indirect
	     || h->root.root.type == bfd_link_hash_warning)
	h = (struct score_elf_link_hash_entry *) h->root.root.u.i.link;
      if (h->root.forced_local)
	return true;
    }

  return false;
}

/* Returns the dynamic relocation section for DYNOBJ.  */
static asection *
score_elf_rel_dyn_section (bfd *dynobj, bool create_p)
{
  static const char dname[] = ".rel.dyn";
  asection *sreloc;

  sreloc = bfd_get_linker_section (dynobj, dname);
  if (sreloc == NULL && create_p)
    {
      sreloc = bfd_make_section_anyway_with_flags (dynobj, dname,
						   (SEC_ALLOC
						    | SEC_LOAD
						    | SEC_HAS_CONTENTS
						    | SEC_IN_MEMORY
						    | SEC_LINKER_CREATED
						    | SEC_READONLY));
      if (sreloc == NULL
	  || !bfd_set_section_alignment (sreloc,
					 SCORE_ELF_LOG_FILE_ALIGN (dynobj)))
	return NULL;
    }
  return sreloc;
}

static void
score_elf_allocate_dynamic_relocations (bfd *abfd, unsigned int n)
{
  asection *s;

  s = score_elf_rel_dyn_section (abfd, false);
  BFD_ASSERT (s != NULL);

  if (s->size == 0)
    {
      /* Make room for a null element.  */
      s->size += SCORE_ELF_REL_SIZE (abfd);
      ++s->reloc_count;
    }
  s->size += n * SCORE_ELF_REL_SIZE (abfd);
}

/* Create a rel.dyn relocation for the dynamic linker to resolve.  REL
   is the original relocation, which is now being transformed into a
   dynamic relocation.  The ADDENDP is adjusted if necessary; the
   caller should store the result in place of the original addend.  */
static bool
score_elf_create_dynamic_relocation (bfd *output_bfd,
				     struct bfd_link_info *info,
				     const Elf_Internal_Rela *rel,
				     struct score_elf_link_hash_entry *h,
				     bfd_vma symbol,
				     bfd_vma *addendp, asection *input_section)
{
  Elf_Internal_Rela outrel[3];
  asection *sreloc;
  bfd *dynobj;
  int r_type;
  long indx;
  bool defined_p;

  r_type = ELF32_R_TYPE (rel->r_info);
  dynobj = elf_hash_table (info)->dynobj;
  sreloc = score_elf_rel_dyn_section (dynobj, false);
  BFD_ASSERT (sreloc != NULL);
  BFD_ASSERT (sreloc->contents != NULL);
  BFD_ASSERT (sreloc->reloc_count * SCORE_ELF_REL_SIZE (output_bfd) < sreloc->size);

  outrel[0].r_offset =
    _bfd_elf_section_offset (output_bfd, info, input_section, rel[0].r_offset);
  outrel[1].r_offset =
    _bfd_elf_section_offset (output_bfd, info, input_section, rel[1].r_offset);
  outrel[2].r_offset =
    _bfd_elf_section_offset (output_bfd, info, input_section, rel[2].r_offset);

  if (outrel[0].r_offset == MINUS_ONE)
    /* The relocation field has been deleted.  */
    return true;

  if (outrel[0].r_offset == MINUS_TWO)
    {
      /* The relocation field has been converted into a relative value of
	 some sort.  Functions like _bfd_elf_write_section_eh_frame expect
	 the field to be fully relocated, so add in the symbol's value.  */
      *addendp += symbol;
      return true;
    }

  /* We must now calculate the dynamic symbol table index to use
     in the relocation.  */
  if (h != NULL
      && (! info->symbolic || !h->root.def_regular)
      /* h->root.dynindx may be -1 if this symbol was marked to
	 become local.  */
      && h->root.dynindx != -1)
    {
      indx = h->root.dynindx;
	/* ??? glibc's ld.so just adds the final GOT entry to the
	   relocation field.  It therefore treats relocs against
	   defined symbols in the same way as relocs against
	   undefined symbols.  */
      defined_p = false;
    }
  else
    {
      indx = 0;
      defined_p = true;
    }

  /* If the relocation was previously an absolute relocation and
     this symbol will not be referred to by the relocation, we must
     adjust it by the value we give it in the dynamic symbol table.
     Otherwise leave the job up to the dynamic linker.  */
  if (defined_p && r_type != R_SCORE_REL32)
    *addendp += symbol;

  /* The relocation is always an REL32 relocation because we don't
     know where the shared library will wind up at load-time.  */
  outrel[0].r_info = ELF32_R_INFO ((unsigned long) indx, R_SCORE_REL32);

  /* For strict adherence to the ABI specification, we should
     generate a R_SCORE_64 relocation record by itself before the
     _REL32/_64 record as well, such that the addend is read in as
     a 64-bit value (REL32 is a 32-bit relocation, after all).
     However, since none of the existing ELF64 SCORE dynamic
     loaders seems to care, we don't waste space with these
     artificial relocations.  If this turns out to not be true,
     score_elf_allocate_dynamic_relocations() should be tweaked so
     as to make room for a pair of dynamic relocations per
     invocation if ABI_64_P, and here we should generate an
     additional relocation record with R_SCORE_64 by itself for a
     NULL symbol before this relocation record.  */
  outrel[1].r_info = ELF32_R_INFO (0, R_SCORE_NONE);
  outrel[2].r_info = ELF32_R_INFO (0, R_SCORE_NONE);

  /* Adjust the output offset of the relocation to reference the
     correct location in the output file.  */
  outrel[0].r_offset += (input_section->output_section->vma
			 + input_section->output_offset);
  outrel[1].r_offset += (input_section->output_section->vma
			 + input_section->output_offset);
  outrel[2].r_offset += (input_section->output_section->vma
			 + input_section->output_offset);

  /* Put the relocation back out.  We have to use the special
     relocation outputter in the 64-bit case since the 64-bit
     relocation format is non-standard.  */
  bfd_elf32_swap_reloc_out
      (output_bfd, &outrel[0],
       (sreloc->contents + sreloc->reloc_count * sizeof (Elf32_External_Rel)));

  /* We've now added another relocation.  */
  ++sreloc->reloc_count;

  /* Make sure the output section is writable.  The dynamic linker
     will be writing to it.  */
  elf_section_data (input_section->output_section)->this_hdr.sh_flags |= SHF_WRITE;

  return true;
}

static bool
score_elf_create_got_section (bfd *abfd,
			      struct bfd_link_info *info,
			      bool maybe_exclude)
{
  flagword flags;
  asection *s;
  struct elf_link_hash_entry *h;
  struct bfd_link_hash_entry *bh;
  struct score_got_info *g;
  size_t amt;

  /* This function may be called more than once.  */
  s = score_elf_got_section (abfd, true);
  if (s)
    {
      if (! maybe_exclude)
	s->flags &= ~SEC_EXCLUDE;
      return true;
    }

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_LINKER_CREATED);

  if (maybe_exclude)
    flags |= SEC_EXCLUDE;

  /* We have to use an alignment of 2**4 here because this is hardcoded
     in the function stub generation and in the linker script.  */
  s = bfd_make_section_anyway_with_flags (abfd, ".got", flags);
  elf_hash_table (info)->sgot = s;
  if (s == NULL
      || !bfd_set_section_alignment (s, 4))
    return false;

  /* Define the symbol _GLOBAL_OFFSET_TABLE_.  We don't do this in the
     linker script because we don't want to define the symbol if we
     are not creating a global offset table.  */
  bh = NULL;
  if (! (_bfd_generic_link_add_one_symbol
	 (info, abfd, "_GLOBAL_OFFSET_TABLE_", BSF_GLOBAL, s,
	  0, NULL, false, get_elf_backend_data (abfd)->collect, &bh)))
    return false;

  h = (struct elf_link_hash_entry *) bh;
  h->non_elf = 0;
  h->def_regular = 1;
  h->type = STT_OBJECT;
  elf_hash_table (info)->hgot = h;

  if (bfd_link_pic (info) && ! bfd_elf_link_record_dynamic_symbol (info, h))
    return false;

  amt = sizeof (struct score_got_info);
  g = bfd_alloc (abfd, amt);
  if (g == NULL)
    return false;

  g->global_gotsym = NULL;
  g->global_gotno = 0;

  g->local_gotno = SCORE_RESERVED_GOTNO;
  g->assigned_gotno = SCORE_RESERVED_GOTNO;
  g->next = NULL;

  g->got_entries = htab_try_create (1, score_elf_got_entry_hash,
				    score_elf_got_entry_eq, NULL);
  if (g->got_entries == NULL)
    return false;
  score_elf_section_data (s)->u.got_info = g;
  score_elf_section_data (s)->elf.this_hdr.sh_flags |= SHF_ALLOC | SHF_WRITE | SHF_SCORE_GPREL;

  return true;
}

/* Calculate the %high function.  */
static bfd_vma
score_elf_high (bfd_vma value)
{
  return ((value + (bfd_vma) 0x8000) >> 16) & 0xffff;
}

/* Create a local GOT entry for VALUE.  Return the index of the entry,
   or -1 if it could not be created.  */
static struct score_got_entry *
score_elf_create_local_got_entry (bfd *abfd,
				  bfd *ibfd ATTRIBUTE_UNUSED,
				  struct score_got_info *gg,
				  asection *sgot, bfd_vma value,
				  unsigned long r_symndx ATTRIBUTE_UNUSED,
				  struct score_elf_link_hash_entry *h ATTRIBUTE_UNUSED,
				  int r_type ATTRIBUTE_UNUSED)
{
  struct score_got_entry entry, **loc;
  struct score_got_info *g;

  entry.abfd = NULL;
  entry.symndx = -1;
  entry.d.address = value;

  g = gg;
  loc = (struct score_got_entry **) htab_find_slot (g->got_entries, &entry, INSERT);
  if (*loc)
    return *loc;

  entry.gotidx = SCORE_ELF_GOT_SIZE (abfd) * g->assigned_gotno++;

  *loc = bfd_alloc (abfd, sizeof entry);

  if (! *loc)
    return NULL;

  memcpy (*loc, &entry, sizeof entry);

  if (g->assigned_gotno >= g->local_gotno)
    {
      (*loc)->gotidx = -1;
      /* We didn't allocate enough space in the GOT.  */
      _bfd_error_handler
	(_("not enough GOT space for local GOT entries"));
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }

  score_bfd_put_32 (abfd, value, (sgot->contents + entry.gotidx));

  return *loc;
}

/* Find a GOT entry whose higher-order 16 bits are the same as those
   for value.  Return the index into the GOT for this entry.  */
static bfd_vma
score_elf_got16_entry (bfd *abfd, bfd *ibfd, struct bfd_link_info *info,
		       bfd_vma value, bool external)
{
  asection *sgot;
  struct score_got_info *g;
  struct score_got_entry *entry;

  if (!external)
    {
      /* Although the ABI says that it is "the high-order 16 bits" that we
	 want, it is really the %high value.  The complete value is
	 calculated with a `addiu' of a LO16 relocation, just as with a
	 HI16/LO16 pair.  */
      value = score_elf_high (value) << 16;
    }

  g = score_elf_got_info (elf_hash_table (info)->dynobj, &sgot);

  entry = score_elf_create_local_got_entry (abfd, ibfd, g, sgot, value, 0, NULL,
					    R_SCORE_GOT15);
  if (entry)
    return entry->gotidx;
  else
    return MINUS_ONE;
}

static void
s3_bfd_score_elf_hide_symbol (struct bfd_link_info *info,
			      struct elf_link_hash_entry *entry,
			      bool force_local)
{
  bfd *dynobj;
  asection *got;
  struct score_got_info *g;
  struct score_elf_link_hash_entry *h;

  h = (struct score_elf_link_hash_entry *) entry;
  if (h->forced_local)
    return;
  h->forced_local = true;

  dynobj = elf_hash_table (info)->dynobj;
  if (dynobj != NULL && force_local)
    {
      got = score_elf_got_section (dynobj, false);
      if (got == NULL)
	return;
      g = score_elf_section_data (got)->u.got_info;

      if (g->next)
	{
	  struct score_got_entry e;
	  struct score_got_info *gg = g;

	  /* Since we're turning what used to be a global symbol into a
	     local one, bump up the number of local entries of each GOT
	     that had an entry for it.  This will automatically decrease
	     the number of global entries, since global_gotno is actually
	     the upper limit of global entries.  */
	  e.abfd = dynobj;
	  e.symndx = -1;
	  e.d.h = h;

	  for (g = g->next; g != gg; g = g->next)
	    if (htab_find (g->got_entries, &e))
	      {
		BFD_ASSERT (g->global_gotno > 0);
		g->local_gotno++;
		g->global_gotno--;
	      }

	  /* If this was a global symbol forced into the primary GOT, we
	     no longer need an entry for it.  We can't release the entry
	     at this point, but we must at least stop counting it as one
	     of the symbols that required a forced got entry.  */
	  if (h->root.got.offset == 2)
	    {
	      BFD_ASSERT (gg->assigned_gotno > 0);
	      gg->assigned_gotno--;
	    }
	}
      else if (g->global_gotno == 0 && g->global_gotsym == NULL)
	/* If we haven't got through GOT allocation yet, just bump up the
	      number of local entries, as this symbol won't be counted as
	      global.  */
	g->local_gotno++;
      else if (h->root.got.offset == 1)
	{
	  /* If we're past non-multi-GOT allocation and this symbol had
		  been marked for a global got entry, give it a local entry
		  instead.  */
	  BFD_ASSERT (g->global_gotno > 0);
	  g->local_gotno++;
	  g->global_gotno--;
	}
    }

  _bfd_elf_link_hash_hide_symbol (info, &h->root, force_local);
}

/* If H is a symbol that needs a global GOT entry, but has a dynamic
   symbol table index lower than any we've seen to date, record it for
   posterity.  */
static bool
score_elf_record_global_got_symbol (struct elf_link_hash_entry *h,
				    bfd *abfd,
				    struct bfd_link_info *info,
				    struct score_got_info *g)
{
  struct score_got_entry entry, **loc;

  /* A global symbol in the GOT must also be in the dynamic symbol table.  */
  if (h->dynindx == -1)
    {
      switch (ELF_ST_VISIBILITY (h->other))
	{
	case STV_INTERNAL:
	case STV_HIDDEN:
	  s3_bfd_score_elf_hide_symbol (info, h, true);
	  break;
	}
      if (!bfd_elf_link_record_dynamic_symbol (info, h))
	return false;
    }

  entry.abfd = abfd;
  entry.symndx = -1;
  entry.d.h = (struct score_elf_link_hash_entry *)h;

  loc = (struct score_got_entry **)htab_find_slot (g->got_entries, &entry, INSERT);

  /* If we've already marked this entry as needing GOT space, we don't
     need to do it again.  */
  if (*loc)
    return true;

  *loc = bfd_alloc (abfd, sizeof entry);
  if (! *loc)
    return false;

  entry.gotidx = -1;

  memcpy (*loc, &entry, sizeof (entry));

  if (h->got.offset != MINUS_ONE)
    return true;

  /* By setting this to a value other than -1, we are indicating that
     there needs to be a GOT entry for H.  Avoid using zero, as the
     generic ELF copy_indirect_symbol tests for <= 0.  */
  h->got.offset = 1;

  return true;
}

/* Reserve space in G for a GOT entry containing the value of symbol
   SYMNDX in input bfd ABDF, plus ADDEND.  */
static bool
score_elf_record_local_got_symbol (bfd *abfd,
				   long symndx,
				   bfd_vma addend,
				   struct score_got_info *g)
{
  struct score_got_entry entry, **loc;

  entry.abfd = abfd;
  entry.symndx = symndx;
  entry.d.addend = addend;
  loc = (struct score_got_entry **)htab_find_slot (g->got_entries, &entry, INSERT);

  if (*loc)
    return true;

  entry.gotidx = g->local_gotno++;

  *loc = bfd_alloc (abfd, sizeof(entry));
  if (! *loc)
    return false;

  memcpy (*loc, &entry, sizeof (entry));

  return true;
}

/* Returns the GOT offset at which the indicated address can be found.
   If there is not yet a GOT entry for this value, create one.
   Returns -1 if no satisfactory GOT offset can be found.  */
static bfd_vma
score_elf_local_got_index (bfd *abfd, bfd *ibfd, struct bfd_link_info *info,
			   bfd_vma value, unsigned long r_symndx,
			   struct score_elf_link_hash_entry *h, int r_type)
{
  asection *sgot;
  struct score_got_info *g;
  struct score_got_entry *entry;

  g = score_elf_got_info (elf_hash_table (info)->dynobj, &sgot);

  entry = score_elf_create_local_got_entry (abfd, ibfd, g, sgot, value,
					     r_symndx, h, r_type);
  if (!entry)
    return MINUS_ONE;

  else
    return entry->gotidx;
}

/* Returns the GOT index for the global symbol indicated by H.  */

static bfd_vma
score_elf_global_got_index (bfd *abfd, struct elf_link_hash_entry *h)
{
  bfd_vma got_index;
  asection *sgot;
  struct score_got_info *g;
  long global_got_dynindx = 0;

  g = score_elf_got_info (abfd, &sgot);
  if (g->global_gotsym != NULL)
    global_got_dynindx = g->global_gotsym->dynindx;

  /* Once we determine the global GOT entry with the lowest dynamic
     symbol table index, we must put all dynamic symbols with greater
     indices into the GOT.  That makes it easy to calculate the GOT
     offset.  */
  BFD_ASSERT (h->dynindx >= global_got_dynindx);
  got_index = ((h->dynindx - global_got_dynindx + g->local_gotno) * SCORE_ELF_GOT_SIZE (abfd));
  BFD_ASSERT (got_index < sgot->size);

  return got_index;
}

/* Returns the offset for the entry at the INDEXth position in the GOT.  */

static bfd_vma
score_elf_got_offset_from_index (bfd *dynobj,
				 bfd *output_bfd,
				 bfd *input_bfd ATTRIBUTE_UNUSED,
				 bfd_vma got_index)
{
  asection *sgot;
  bfd_vma gp;

  score_elf_got_info (dynobj, &sgot);
  gp = _bfd_get_gp_value (output_bfd);

  return sgot->output_section->vma + sgot->output_offset + got_index - gp;
}

/* Follow indirect and warning hash entries so that each got entry
   points to the final symbol definition.  P must point to a pointer
   to the hash table we're traversing.  Since this traversal may
   modify the hash table, we set this pointer to NULL to indicate
   we've made a potentially-destructive change to the hash table, so
   the traversal must be restarted.  */
static int
score_elf_resolve_final_got_entry (void **entryp, void *p)
{
  struct score_got_entry *entry = (struct score_got_entry *)*entryp;
  htab_t got_entries = *(htab_t *)p;

  if (entry->abfd != NULL && entry->symndx == -1)
    {
      struct score_elf_link_hash_entry *h = entry->d.h;

      while (h->root.root.type == bfd_link_hash_indirect
	     || h->root.root.type == bfd_link_hash_warning)
	h = (struct score_elf_link_hash_entry *) h->root.root.u.i.link;

      if (entry->d.h == h)
	return 1;

      entry->d.h = h;

      /* If we can't find this entry with the new bfd hash, re-insert
	 it, and get the traversal restarted.  */
      if (! htab_find (got_entries, entry))
	{
	  htab_clear_slot (got_entries, entryp);
	  entryp = htab_find_slot (got_entries, entry, INSERT);
	  if (! *entryp)
	    *entryp = entry;
	  /* Abort the traversal, since the whole table may have
	     moved, and leave it up to the parent to restart the
	     process.  */
	  *(htab_t *)p = NULL;
	  return 0;
	}
      /* We might want to decrement the global_gotno count, but it's
	 either too early or too late for that at this point.  */
    }

  return 1;
}

/* Turn indirect got entries in a got_entries table into their final locations.  */
static void
score_elf_resolve_final_got_entries (struct score_got_info *g)
{
  htab_t got_entries;

  do
    {
      got_entries = g->got_entries;

      htab_traverse (got_entries,
		     score_elf_resolve_final_got_entry,
		     &got_entries);
    }
  while (got_entries == NULL);
}

/* Add INCREMENT to the reloc (of type HOWTO) at ADDRESS. for -r  */
static void
score_elf_add_to_rel (bfd *abfd,
		      bfd_byte *address,
		      reloc_howto_type *howto,
		      bfd_signed_vma increment)
{
  bfd_signed_vma addend;
  bfd_vma contents;
  unsigned long offset;
  unsigned long r_type = howto->type;
  unsigned long hi16_addend, hi16_offset, hi16_value, uvalue;

  contents = score_bfd_get_32 (abfd, address);
  /* Get the (signed) value from the instruction.  */
  addend = contents & howto->src_mask;
  if (addend & ((howto->src_mask + 1) >> 1))
    {
      bfd_signed_vma mask;

      mask = -1;
      mask &= ~howto->src_mask;
      addend |= mask;
    }
  /* Add in the increment, (which is a byte value).  */
  switch (r_type)
    {
    case R_SCORE_PC19:
      offset =
	(((contents & howto->src_mask) & 0x3ff0000) >> 6) | ((contents & howto->src_mask) & 0x3ff);
      offset += increment;
      contents =
	(contents & ~howto->
	 src_mask) | (((offset << 6) & howto->src_mask) & 0x3ff0000) | (offset & 0x3ff);
      score_bfd_put_32 (abfd, contents, address);
      break;
    case R_SCORE_HI16:
      break;
    case R_SCORE_LO16:
      hi16_addend = score_bfd_get_32 (abfd, address - 4);
      hi16_offset = ((((hi16_addend >> 16) & 0x3) << 15) | (hi16_addend & 0x7fff)) >> 1;
      offset = ((((contents >> 16) & 0x3) << 15) | (contents & 0x7fff)) >> 1;
      offset = (hi16_offset << 16) | (offset & 0xffff);
      uvalue = increment + offset;
      hi16_offset = (uvalue >> 16) << 1;
      hi16_value = (hi16_addend & (~(howto->dst_mask)))
	| (hi16_offset & 0x7fff) | ((hi16_offset << 1) & 0x30000);
      score_bfd_put_32 (abfd, hi16_value, address - 4);
      offset = (uvalue & 0xffff) << 1;
      contents = (contents & (~(howto->dst_mask))) | (offset & 0x7fff) | ((offset << 1) & 0x30000);
      score_bfd_put_32 (abfd, contents, address);
      break;
    case R_SCORE_24:
      offset =
	(((contents & howto->src_mask) >> 1) & 0x1ff8000) | ((contents & howto->src_mask) & 0x7fff);
      offset += increment;
      contents =
	(contents & ~howto->
	 src_mask) | (((offset << 1) & howto->src_mask) & 0x3ff0000) | (offset & 0x7fff);
      score_bfd_put_32 (abfd, contents, address);
      break;

    case R_SCORE16_11:

      contents = score_bfd_get_16 (abfd, address);
      offset = contents & howto->src_mask;
      offset += increment;
      contents = (contents & ~howto->src_mask) | (offset & howto->src_mask);
      score_bfd_put_16 (abfd, contents, address);

      break;
    case R_SCORE16_PC8:

      contents = score_bfd_get_16 (abfd, address);
      offset = (contents & howto->src_mask) + ((increment >> 1) & 0x1ff);
      contents = (contents & (~howto->src_mask)) | (offset & howto->src_mask);
      score_bfd_put_16 (abfd, contents, address);

      break;

    case R_SCORE_BCMP:
      contents = score_bfd_get_32 (abfd, address);
      offset = (contents & howto->src_mask);
      offset <<= howto->rightshift;
      offset += increment;
      offset >>= howto->rightshift;
      contents = (contents & (~howto->src_mask)) | (offset & howto->src_mask);
      score_bfd_put_32 (abfd, contents, address);
      break;

    case R_SCORE_IMM30:
      contents = score_bfd_get_48 (abfd, address);
      offset = (contents & howto->src_mask);
      offset <<= howto->rightshift;
      offset += increment;
      offset >>= howto->rightshift;
      contents = (contents & (~howto->src_mask)) | (offset & howto->src_mask);
      score_bfd_put_48 (abfd, contents, address);
      break;

    case R_SCORE_IMM32:
      contents = score_bfd_get_48 (abfd, address);
      offset = (contents & howto->src_mask);
      offset += increment;
      contents = (contents & (~howto->src_mask)) | (offset & howto->src_mask);
      score_bfd_put_48 (abfd, contents, address);
      break;

    default:
      addend += increment;
      contents = (contents & ~howto->dst_mask) | (addend & howto->dst_mask);
      score_bfd_put_32 (abfd, contents, address);
      break;
    }
}

/* Perform a relocation as part of a final link.  */
static bfd_reloc_status_type
score_elf_final_link_relocate (reloc_howto_type *howto,
			       bfd *input_bfd,
			       bfd *output_bfd,
			       asection *input_section,
			       bfd_byte *contents,
			       Elf_Internal_Rela *rel,
			       Elf_Internal_Rela *relocs,
			       bfd_vma symbol,
			       struct bfd_link_info *info,
			       const char *sym_name ATTRIBUTE_UNUSED,
			       int sym_flags ATTRIBUTE_UNUSED,
			       struct score_elf_link_hash_entry *h,
			       asection **local_sections,
			       bool gp_disp_p)
{
  unsigned long r_type;
  unsigned long r_symndx;
  bfd_byte *hit_data = contents + rel->r_offset;
  bfd_vma addend;
  /* The final GP value to be used for the relocatable, executable, or
     shared object file being produced.  */
  bfd_vma gp = MINUS_ONE;
  /* The place (section offset or address) of the storage unit being relocated.  */
  bfd_vma rel_addr;
  /* The offset into the global offset table at which the address of the relocation entry
     symbol, adjusted by the addend, resides during execution.  */
  bfd_vma g = MINUS_ONE;
  /* TRUE if the symbol referred to by this relocation is a local symbol.  */
  bool local_p;
  /* The eventual value we will relocate.  */
  bfd_vma value = symbol;
  unsigned long hi16_addend, hi16_offset, hi16_value, uvalue, offset, abs_value = 0;


  if (elf_gp (output_bfd) == 0)
    {
      struct bfd_link_hash_entry *bh;
      asection *o;

      bh = bfd_link_hash_lookup (info->hash, "_gp", 0, 0, 1);
      if (bh != NULL && bh->type == bfd_link_hash_defined)
	elf_gp (output_bfd) = (bh->u.def.value
			       + bh->u.def.section->output_section->vma
			       + bh->u.def.section->output_offset);
      else if (bfd_link_relocatable (info))
	{
	  bfd_vma lo = -1;

	  /* Find the GP-relative section with the lowest offset.  */
	  for (o = output_bfd->sections; o != NULL; o = o->next)
	    if (o->vma < lo)
	      lo = o->vma;
	  /* And calculate GP relative to that.  */
	  elf_gp (output_bfd) = lo + ELF_SCORE_GP_OFFSET (input_bfd);
	}
      else
	{
	  /* If the relocate_section function needs to do a reloc
	     involving the GP value, it should make a reloc_dangerous
	     callback to warn that GP is not defined.  */
	}
    }

  /* Parse the relocation.  */
  r_symndx = ELF32_R_SYM (rel->r_info);
  r_type = ELF32_R_TYPE (rel->r_info);
  rel_addr = (input_section->output_section->vma + input_section->output_offset + rel->r_offset);
  local_p = score_elf_local_relocation_p (input_bfd, rel, local_sections, true);

  if (r_type == R_SCORE_GOT15)
    {
      const Elf_Internal_Rela *relend;
      const Elf_Internal_Rela *lo16_rel;
      bfd_vma lo_value = 0;

      relend = relocs + input_section->reloc_count;
      lo16_rel = score_elf_next_relocation (input_bfd, R_SCORE_GOT_LO16, rel, relend);
      if ((local_p) && (lo16_rel != NULL))
	{
	  bfd_vma tmp = 0;
	  tmp = score_bfd_get_32 (input_bfd, contents + lo16_rel->r_offset);
	  lo_value = (((tmp >> 16) & 0x3) << 14) | ((tmp & 0x7fff) >> 1);
	}
      addend = lo_value;
    }
  /* For score3 R_SCORE_ABS32.  */
  else if (r_type == R_SCORE_ABS32 || r_type == R_SCORE_REL32)
    {
      addend = (bfd_get_32 (input_bfd, hit_data) >> howto->bitpos) & howto->src_mask;
    }
  else
    {
      addend = (score_bfd_get_32 (input_bfd, hit_data) >> howto->bitpos) & howto->src_mask;
    }

  /* If we haven't already determined the GOT offset, or the GP value,
     and we're going to need it, get it now.  */
  switch (r_type)
    {
    case R_SCORE_CALL15:
    case R_SCORE_GOT15:
      if (!local_p)
	{
	  g = score_elf_global_got_index (elf_hash_table (info)->dynobj,
					  (struct elf_link_hash_entry *) h);
	  if ((! elf_hash_table (info)->dynamic_sections_created
	       || (bfd_link_pic (info)
		   && (info->symbolic || h->root.dynindx == -1)
		   && h->root.def_regular)))
	    {
	      /* This is a static link or a -Bsymbolic link.  The
		 symbol is defined locally, or was forced to be local.
		 We must initialize this entry in the GOT.  */
	      bfd *tmpbfd = elf_hash_table (info)->dynobj;
	      asection *sgot = score_elf_got_section (tmpbfd, false);
	      score_bfd_put_32 (tmpbfd, value, sgot->contents + g);
	    }
	}
      else if (r_type == R_SCORE_GOT15 || r_type == R_SCORE_CALL15)
	{
	  /* There's no need to create a local GOT entry here; the
	     calculation for a local GOT15 entry does not involve G.  */
	  ;
	}
      else
	{
	  g = score_elf_local_got_index (output_bfd, input_bfd, info,
					 symbol + addend, r_symndx, h, r_type);
	    if (g == MINUS_ONE)
	    return bfd_reloc_outofrange;
	}

      /* Convert GOT indices to actual offsets.  */
      g = score_elf_got_offset_from_index (elf_hash_table (info)->dynobj,
					   output_bfd, input_bfd, g);
      break;

    case R_SCORE_HI16:
    case R_SCORE_LO16:
    case R_SCORE_GPREL32:
      gp = _bfd_get_gp_value (output_bfd);
      break;

    case R_SCORE_GP15:
      gp = _bfd_get_gp_value (output_bfd);

    default:
      break;
    }

  switch (r_type)
    {
    case R_SCORE_NONE:
      return bfd_reloc_ok;

    case R_SCORE_ABS32:
    case R_SCORE_REL32:
      if ((bfd_link_pic (info)
	   || (elf_hash_table (info)->dynamic_sections_created
	       && h != NULL
	       && h->root.def_dynamic
	       && !h->root.def_regular))
	   && r_symndx != STN_UNDEF
	   && (input_section->flags & SEC_ALLOC) != 0)
	{
	  /* If we're creating a shared library, or this relocation is against a symbol
	     in a shared library, then we can't know where the symbol will end up.
	     So, we create a relocation record in the output, and leave the job up
	     to the dynamic linker.  */
	  value = addend;
	  if (!score_elf_create_dynamic_relocation (output_bfd, info, rel, h,
						    symbol, &value,
						    input_section))
	    return bfd_reloc_undefined;
	}
      else if (r_symndx == STN_UNDEF)
	/* r_symndx will be STN_UNDEF (zero) only for relocs against symbols
	   from removed linkonce sections, or sections discarded by
	   a linker script.  */
	value = 0;
      else
	{
	  if (r_type != R_SCORE_REL32)
	    value = symbol + addend;
	  else
	    value = addend;
	}
      value &= howto->dst_mask;
      bfd_put_32 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_SCORE_ABS16:
      value += addend;
      if ((long)value > 0x7fff || (long)value < -0x8000)
	return bfd_reloc_overflow;
      score_bfd_put_16 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_SCORE_24:
      addend = score_bfd_get_32 (input_bfd, hit_data);
      offset = (((addend & howto->src_mask) >> 1) & 0x1ff8000) | ((addend & howto->src_mask) & 0x7fff);
      if ((offset & 0x1000000) != 0)
	offset |= 0xfe000000;
      value += offset;
      abs_value = value - rel_addr;
      if ((abs_value & 0xfe000000) != 0)
	return bfd_reloc_overflow;
      addend = (addend & ~howto->src_mask)
		| (((value << 1) & howto->src_mask) & 0x3ff0000) | (value & 0x7fff);
      score_bfd_put_32 (input_bfd, addend, hit_data);
      return bfd_reloc_ok;

    /* signed imm32.  */
    case R_SCORE_IMM30:
      {
	int not_word_align_p = 0;
	bfd_vma imm_offset = 0;
	addend = score_bfd_get_48 (input_bfd, hit_data);
	imm_offset = ((addend >> 7) & 0xff)
		     | (((addend >> 16) & 0x7fff) << 8)
		     | (((addend >> 32) & 0x7f) << 23);
	imm_offset <<= howto->rightshift;
	value += imm_offset;
	value &= 0xffffffff;

	/* Check lw48/sw48 rd, value/label word align.  */
	if ((value & 0x3) != 0)
	  not_word_align_p = 1;

	value >>= howto->rightshift;
	addend = (addend & ~howto->src_mask)
		 | (((value & 0xff) >> 0) << 7)
		 | (((value & 0x7fff00) >> 8) << 16)
		 | (((value & 0x3f800000) >> 23) << 32);
	score_bfd_put_48 (input_bfd, addend, hit_data);
	if (not_word_align_p)
	  return bfd_reloc_other;
	else
	  return bfd_reloc_ok;
      }

    case R_SCORE_IMM32:
      {
	bfd_vma imm_offset = 0;
	addend = score_bfd_get_48 (input_bfd, hit_data);
	imm_offset = ((addend >> 5) & 0x3ff)
		     | (((addend >> 16) & 0x7fff) << 10)
		     | (((addend >> 32) & 0x7f) << 25);
	value += imm_offset;
	value &= 0xffffffff;
	addend = (addend & ~howto->src_mask)
		 | ((value & 0x3ff) << 5)
		 | (((value >> 10) & 0x7fff) << 16)
		 | (((value >> 25) & 0x7f) << 32);
	score_bfd_put_48 (input_bfd, addend, hit_data);
	return bfd_reloc_ok;
      }

    case R_SCORE_PC19:
      addend = score_bfd_get_32 (input_bfd, hit_data);
      offset = (((addend & howto->src_mask) & 0x3ff0000) >> 6) | ((addend & howto->src_mask) & 0x3ff);
      if ((offset & 0x80000) != 0)
	offset |= 0xfff00000;
      abs_value = value = value - rel_addr + offset;
      /* exceed 20 bit : overflow.  */
      if ((abs_value & 0x80000000) == 0x80000000)
	abs_value = 0xffffffff - value + 1;
      if ((abs_value & 0xfff80000) != 0)
	return bfd_reloc_overflow;
      addend = (addend & ~howto->src_mask)
		| (((value << 6) & howto->src_mask) & 0x3ff0000) | (value & 0x3ff);
      score_bfd_put_32 (input_bfd, addend, hit_data);
      return bfd_reloc_ok;

    case R_SCORE16_11:
      addend = score_bfd_get_16 (input_bfd, hit_data);
      offset = addend & howto->src_mask;
      if ((offset & 0x800) != 0)	/* Offset is negative.  */
	offset |= 0xfffff000;
      value += offset;
      abs_value = value - rel_addr;
      if ((abs_value & 0xfffff000) != 0)
	return bfd_reloc_overflow;
      addend = (addend & ~howto->src_mask) | (value & howto->src_mask);
      score_bfd_put_16 (input_bfd, addend, hit_data);
      return bfd_reloc_ok;

    case R_SCORE16_PC8:
      addend = score_bfd_get_16 (input_bfd, hit_data);
      offset = (addend & howto->src_mask) << 1;
      if ((offset & 0x200) != 0)	/* Offset is negative.  */
	offset |= 0xfffffe00;
      abs_value = value = value - rel_addr + offset;
      /* Sign bit + exceed 9 bit.  */
      if (((value & 0xfffffe00) != 0) && ((value & 0xfffffe00) != 0xfffffe00))
	return bfd_reloc_overflow;
      value >>= 1;
      addend = (addend & ~howto->src_mask) | (value & howto->src_mask);
      score_bfd_put_16 (input_bfd, addend, hit_data);
      return bfd_reloc_ok;

    case R_SCORE_BCMP:
      addend = score_bfd_get_32 (input_bfd, hit_data);
      offset = (addend & howto->src_mask) << howto->rightshift;
      if ((offset & 0x200) != 0)	/* Offset is negative.  */
	offset |= 0xfffffe00;
      value = value - rel_addr + offset;
      /* Sign bit + exceed 9 bit.  */
      if (((value & 0xfffffe00) != 0) && ((value & 0xfffffe00) != 0xfffffe00))
	return bfd_reloc_overflow;
      value >>= howto->rightshift;
      addend = (addend & ~howto->src_mask)
	       | (value & 0x1)
	       | (((value >> 1) & 0x7) << 7)
	       | (((value >> 4) & 0x1f) << 21);
      score_bfd_put_32 (input_bfd, addend, hit_data);
      return bfd_reloc_ok;

    case R_SCORE_HI16:
      return bfd_reloc_ok;

    case R_SCORE_LO16:
      hi16_addend = score_bfd_get_32 (input_bfd, hit_data - 4);
      hi16_offset = ((((hi16_addend >> 16) & 0x3) << 15) | (hi16_addend & 0x7fff)) >> 1;
      addend = score_bfd_get_32 (input_bfd, hit_data);
      offset = ((((addend >> 16) & 0x3) << 15) | (addend & 0x7fff)) >> 1;
      offset = (hi16_offset << 16) | (offset & 0xffff);

      if (!gp_disp_p)
	uvalue = value + offset;
      else
	uvalue = offset + gp - rel_addr + 4;

      hi16_offset = (uvalue >> 16) << 1;
      hi16_value = (hi16_addend & (~(howto->dst_mask)))
			| (hi16_offset & 0x7fff) | ((hi16_offset << 1) & 0x30000);
      score_bfd_put_32 (input_bfd, hi16_value, hit_data - 4);
      offset = (uvalue & 0xffff) << 1;
      value = (addend & (~(howto->dst_mask))) | (offset & 0x7fff) | ((offset << 1) & 0x30000);
      score_bfd_put_32 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_SCORE_GP15:
      addend = score_bfd_get_32 (input_bfd, hit_data);
      offset = addend & 0x7fff;
      if ((offset & 0x4000) == 0x4000)
	offset |= 0xffffc000;
      value = value + offset - gp;
      if (((value & 0xffffc000) != 0) && ((value & 0xffffc000) != 0xffffc000))
	return bfd_reloc_overflow;
      value = (addend & ~howto->src_mask) | (value & howto->src_mask);
      score_bfd_put_32 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_SCORE_GOT15:
    case R_SCORE_CALL15:
      if (local_p)
	{
	  bool forced;

	  /* The special case is when the symbol is forced to be local.  We need the
	     full address in the GOT since no R_SCORE_GOT_LO16 relocation follows.  */
	  forced = ! score_elf_local_relocation_p (input_bfd, rel,
						   local_sections, false);
	  value = score_elf_got16_entry (output_bfd, input_bfd, info,
					 symbol + addend, forced);
	  if (value == MINUS_ONE)
	    return bfd_reloc_outofrange;
	  value = score_elf_got_offset_from_index (elf_hash_table (info)->dynobj,
						   output_bfd, input_bfd, value);
	}
      else
	{
	  value = g;
	}

      if ((long) value > 0x3fff || (long) value < -0x4000)
	return bfd_reloc_overflow;

      addend = score_bfd_get_32 (input_bfd, hit_data);
      value = (addend & ~howto->dst_mask) | (value & howto->dst_mask);
      score_bfd_put_32 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_SCORE_GPREL32:
      value = (addend + symbol - gp);
      value &= howto->dst_mask;
      score_bfd_put_32 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_SCORE_GOT_LO16:
      addend = score_bfd_get_32 (input_bfd, hit_data);
      value = (((addend >> 16) & 0x3) << 14) | ((addend & 0x7fff) >> 1);
      value += symbol;
      value = (addend & (~(howto->dst_mask))) | ((value & 0x3fff) << 1)
	       | (((value >> 14) & 0x3) << 16);

      score_bfd_put_32 (input_bfd, value, hit_data);
      return bfd_reloc_ok;

    case R_SCORE_DUMMY_HI16:
      return bfd_reloc_ok;

    case R_SCORE_GNU_VTINHERIT:
    case R_SCORE_GNU_VTENTRY:
      /* We don't do anything with these at present.  */
      return bfd_reloc_continue;

    default:
      return bfd_reloc_notsupported;
    }
}

/* Score backend functions.  */
static bool
s3_bfd_score_info_to_howto (bfd *abfd,
			    arelent *bfd_reloc,
			    Elf_Internal_Rela *elf_reloc)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (elf_reloc->r_info);
  if (r_type >= ARRAY_SIZE (elf32_score_howto_table))
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  bfd_reloc->howto = &elf32_score_howto_table[r_type];
  return true;
}

/* Relocate an score ELF section.  */
static int
s3_bfd_score_elf_relocate_section (bfd *output_bfd,
				   struct bfd_link_info *info,
				   bfd *input_bfd,
				   asection *input_section,
				   bfd_byte *contents,
				   Elf_Internal_Rela *relocs,
				   Elf_Internal_Sym *local_syms,
				   asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  const char *name;
  unsigned long offset;
  unsigned long hi16_addend, hi16_offset, hi16_value, uvalue;
  size_t extsymoff;
  bool gp_disp_p = false;

  /* Sort dynsym.  */
  if (elf_hash_table (info)->dynamic_sections_created)
    {
      bfd_size_type dynsecsymcount = 0;
      if (bfd_link_pic (info))
	{
	  asection * p;
	  const struct elf_backend_data *bed = get_elf_backend_data (output_bfd);

	  for (p = output_bfd->sections; p ; p = p->next)
	    if ((p->flags & SEC_EXCLUDE) == 0
		&& (p->flags & SEC_ALLOC) != 0
		&& !(*bed->elf_backend_omit_section_dynsym) (output_bfd, info, p))
	      ++ dynsecsymcount;
	}

      if (!score_elf_sort_hash_table (info, dynsecsymcount + 1))
	return false;
    }

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  extsymoff = (elf_bad_symtab (input_bfd)) ? 0 : symtab_hdr->sh_info;
  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct score_elf_link_hash_entry *h;
      bfd_vma relocation = 0;
      bfd_reloc_status_type r;
      arelent bfd_reloc;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (! s3_bfd_score_info_to_howto (input_bfd, &bfd_reloc, (Elf_Internal_Rela *) rel))
	continue;
      howto = bfd_reloc.howto;

      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < extsymoff)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = (sec->output_section->vma
			+ sec->output_offset
			+ sym->st_value);
	  name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym, sec);

	  if (!bfd_link_relocatable (info)
	      && (sec->flags & SEC_MERGE)
	      && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    {
	      asection *msec;
	      bfd_vma addend, value;

	      switch (r_type)
		{
		case R_SCORE_HI16:
		  break;
		case R_SCORE_LO16:
		  hi16_addend = score_bfd_get_32 (input_bfd, contents + rel->r_offset - 4);
		  hi16_offset = ((((hi16_addend >> 16) & 0x3) << 15) | (hi16_addend & 0x7fff)) >> 1;
		  value = score_bfd_get_32 (input_bfd, contents + rel->r_offset);
		  offset = ((((value >> 16) & 0x3) << 15) | (value & 0x7fff)) >> 1;
		  addend = (hi16_offset << 16) | (offset & 0xffff);
		  msec = sec;
		  addend = _bfd_elf_rel_local_sym (output_bfd, sym, &msec, addend);
		  addend -= relocation;
		  addend += msec->output_section->vma + msec->output_offset;
		  uvalue = addend;
		  hi16_offset = (uvalue >> 16) << 1;
		  hi16_value = (hi16_addend & (~(howto->dst_mask)))
		    | (hi16_offset & 0x7fff) | ((hi16_offset << 1) & 0x30000);
		  score_bfd_put_32 (input_bfd, hi16_value, contents + rel->r_offset - 4);
		  offset = (uvalue & 0xffff) << 1;
		  value = (value & (~(howto->dst_mask)))
		    | (offset & 0x7fff) | ((offset << 1) & 0x30000);
		  score_bfd_put_32 (input_bfd, value, contents + rel->r_offset);
		  break;

		case R_SCORE_IMM32:
		  {
		    value = score_bfd_get_48 (input_bfd, contents + rel->r_offset);
		    addend = ((value >> 5) & 0x3ff)
			      | (((value >> 16) & 0x7fff) << 10)
			      | (((value >> 32) & 0x7f) << 25);
		    msec = sec;
		    addend = _bfd_elf_rel_local_sym (output_bfd, sym, &msec, addend);
		    addend -= relocation;
		    addend += msec->output_section->vma + msec->output_offset;
		    addend &= 0xffffffff;
		    value = (value & ~howto->src_mask)
			     | ((addend & 0x3ff) << 5)
			     | (((addend >> 10) & 0x7fff) << 16)
			     | (((addend >> 25) & 0x7f) << 32);
		    score_bfd_put_48 (input_bfd, value, contents + rel->r_offset);
		    break;
		  }

		case R_SCORE_IMM30:
		  {
		    int not_word_align_p = 0;
		    value = score_bfd_get_48 (input_bfd, contents + rel->r_offset);
		    addend = ((value >> 7) & 0xff)
			      | (((value >> 16) & 0x7fff) << 8)
			      | (((value >> 32) & 0x7f) << 23);
		    addend <<= howto->rightshift;
		    msec = sec;
		    addend = _bfd_elf_rel_local_sym (output_bfd, sym, &msec, addend);
		    addend -= relocation;
		    addend += msec->output_section->vma + msec->output_offset;
		    addend &= 0xffffffff;

		    /* Check lw48/sw48 rd, value/label word align.  */
		    if ((addend & 0x3) != 0)
		      not_word_align_p = 1;

		    addend >>= howto->rightshift;
		    value = (value & ~howto->src_mask)
			     | (((addend & 0xff) >> 0) << 7)
			     | (((addend & 0x7fff00) >> 8) << 16)
			     | (((addend & 0x3f800000) >> 23) << 32);
		    score_bfd_put_48 (input_bfd, value, contents + rel->r_offset);

		    if (not_word_align_p)
		      return bfd_reloc_other;
		    else
		      break;
		  }

		case R_SCORE_GOT_LO16:
		  value = score_bfd_get_32 (input_bfd, contents + rel->r_offset);
		  addend = (((value >> 16) & 0x3) << 14) | ((value & 0x7fff) >> 1);
		  msec = sec;
		  addend = _bfd_elf_rel_local_sym (output_bfd, sym, &msec, addend) - relocation;
		  addend += msec->output_section->vma + msec->output_offset;
		  value = (value & (~(howto->dst_mask))) | ((addend & 0x3fff) << 1)
			   | (((addend >> 14) & 0x3) << 16);

		  score_bfd_put_32 (input_bfd, value, contents + rel->r_offset);
		  break;

		case R_SCORE_ABS32:
		case R_SCORE_REL32:
		  value = bfd_get_32 (input_bfd, contents + rel->r_offset);
		  /* Get the (signed) value from the instruction.  */
		  addend = value & howto->src_mask;
		  if (addend & ((howto->src_mask + 1) >> 1))
		    {
		      bfd_signed_vma mask;

		      mask = -1;
		      mask &= ~howto->src_mask;
		      addend |= mask;
		    }
		  msec = sec;
		  addend = _bfd_elf_rel_local_sym (output_bfd, sym, &msec, addend) - relocation;
		  addend += msec->output_section->vma + msec->output_offset;
		  value = (value & ~howto->dst_mask) | (addend & howto->dst_mask);
		  bfd_put_32 (input_bfd, value, contents + rel->r_offset);
		  break;

		default:
		  value = score_bfd_get_32 (input_bfd, contents + rel->r_offset);
		  /* Get the (signed) value from the instruction.  */
		  addend = value & howto->src_mask;
		  if (addend & ((howto->src_mask + 1) >> 1))
		    {
		      bfd_signed_vma mask;

		      mask = -1;
		      mask &= ~howto->src_mask;
		      addend |= mask;
		    }
		  msec = sec;
		  addend = _bfd_elf_rel_local_sym (output_bfd, sym, &msec, addend) - relocation;
		  addend += msec->output_section->vma + msec->output_offset;
		  value = (value & ~howto->dst_mask) | (addend & howto->dst_mask);
		  score_bfd_put_32 (input_bfd, value, contents + rel->r_offset);
		  break;
		}
	    }
	}
      else
	{
	  /* For global symbols we look up the symbol in the hash-table.  */
	  h = ((struct score_elf_link_hash_entry *)
	       elf_sym_hashes (input_bfd) [r_symndx - extsymoff]);

	  if (info->wrap_hash != NULL
	      && (input_section->flags & SEC_DEBUGGING) != 0)
	    h = ((struct score_elf_link_hash_entry *)
		 unwrap_hash_lookup (info, input_bfd, &h->root.root));

	  /* Find the real hash-table entry for this symbol.  */
	  while (h->root.root.type == bfd_link_hash_indirect
		 || h->root.root.type == bfd_link_hash_warning)
	    h = (struct score_elf_link_hash_entry *) h->root.root.u.i.link;

	  /* Record the name of this symbol, for our caller.  */
	  name = h->root.root.root.string;

	  /* See if this is the special GP_DISP_LABEL symbol.  Note that such a
	     symbol must always be a global symbol.  */
	  if (strcmp (name, GP_DISP_LABEL) == 0)
	    {
	      /* Relocations against GP_DISP_LABEL are permitted only with
		 R_SCORE_HI16 and R_SCORE_LO16 relocations.  */
	      if (r_type != R_SCORE_HI16 && r_type != R_SCORE_LO16)
		return bfd_reloc_notsupported;

	      gp_disp_p = true;
	    }

	  /* If this symbol is defined, calculate its address.  Note that
	      GP_DISP_LABEL is a magic symbol, always implicitly defined by the
	      linker, so it's inappropriate to check to see whether or not
	      its defined.  */
	  else if ((h->root.root.type == bfd_link_hash_defined
		    || h->root.root.type == bfd_link_hash_defweak)
		   && h->root.root.u.def.section)
	    {
	      sec = h->root.root.u.def.section;
	      if (sec->output_section)
		relocation = (h->root.root.u.def.value
			      + sec->output_section->vma
			      + sec->output_offset);
	      else
		{
		  relocation = h->root.root.u.def.value;
		}
	    }
	  else if (h->root.root.type == bfd_link_hash_undefweak)
	    /* We allow relocations against undefined weak symbols, giving
	       it the value zero, so that you can undefined weak functions
	       and check to see if they exist by looking at their addresses.  */
	    relocation = 0;
	  else if (info->unresolved_syms_in_objects == RM_IGNORE
		   && ELF_ST_VISIBILITY (h->root.other) == STV_DEFAULT)
	    relocation = 0;
	  else if (strcmp (name, "_DYNAMIC_LINK") == 0)
	    {
	      /* If this is a dynamic link, we should have created a _DYNAMIC_LINK symbol
		 in s3_bfd_score_elf_create_dynamic_sections.  Otherwise, we should define
		 the symbol with a value of 0.  */
	      BFD_ASSERT (! bfd_link_pic (info));
	      BFD_ASSERT (bfd_get_section_by_name (output_bfd, ".dynamic") == NULL);
	      relocation = 0;
	    }
	  else if (!bfd_link_relocatable (info))
	    {
              info->callbacks->undefined_symbol
		(info, h->root.root.root.string, input_bfd, input_section,
		 rel->r_offset,
		 (info->unresolved_syms_in_objects == RM_DIAGNOSE
		  && !info->warn_unresolved_syms)
		 || ELF_ST_VISIBILITY (h->root.other));
              relocation = 0;
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
	  if (r_symndx < symtab_hdr->sh_info)
	    {
	      sym = local_syms + r_symndx;
	      if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
		{
		  sec = local_sections[r_symndx];
		  score_elf_add_to_rel (input_bfd, contents + rel->r_offset,
				    howto, (bfd_signed_vma) (sec->output_offset + sym->st_value));
		}
	    }
	  continue;
	}

      /* This is a final link.  */
      r = score_elf_final_link_relocate (howto, input_bfd, output_bfd,
					 input_section, contents, rel, relocs,
					 relocation, info, name,
					 (h ? ELF_ST_TYPE ((unsigned int)h->root.root.type) :
					 ELF_ST_TYPE ((unsigned int)sym->st_info)), h, local_sections,
					 gp_disp_p);

      if (r != bfd_reloc_ok)
	{
	  const char *msg = (const char *)0;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      /* If the overflowing reloc was to an undefined symbol,
		 we have already printed one error message and there
		 is no point complaining again.  */
	      if (!h || h->root.root.type != bfd_link_hash_undefined)
		(*info->callbacks->reloc_overflow)
		  (info, NULL, name, howto->name, (bfd_vma) 0,
		   input_bfd, input_section, rel->r_offset);
	      break;
	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, name, input_bfd, input_section, rel->r_offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      msg = _("internal error: out of range error");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      msg = _("internal error: unsupported relocation error");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      msg = _("internal error: dangerous error");
	      goto common_error;

	    /* Use bfd_reloc_other to check lw48, sw48 word align.  */
	    case bfd_reloc_other:
	      msg = _("address not word aligned");
	      goto common_error;

	    default:
	      msg = _("internal error: unknown error");
	      /* Fall through.  */

	    common_error:
	      (*info->callbacks->warning) (info, msg, name, input_bfd,
					   input_section, rel->r_offset);
	      break;
	    }
	}
    }

  return true;
}

/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table.  */
static bool
s3_bfd_score_elf_check_relocs (bfd *abfd,
			       struct bfd_link_info *info,
			       asection *sec,
			       const Elf_Internal_Rela *relocs)
{
  bfd *dynobj;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  struct score_got_info *g;
  size_t extsymoff;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sgot;
  asection *sreloc;

  if (bfd_link_relocatable (info))
    return true;

  dynobj = elf_hash_table (info)->dynobj;
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  extsymoff = (elf_bad_symtab (abfd)) ? 0 : symtab_hdr->sh_info;

  if (dynobj == NULL)
    {
      sgot = NULL;
      g = NULL;
    }
  else
    {
      sgot = score_elf_got_section (dynobj, false);
      if (sgot == NULL)
	g = NULL;
      else
	{
	  BFD_ASSERT (score_elf_section_data (sgot) != NULL);
	  g = score_elf_section_data (sgot)->u.got_info;
	  BFD_ASSERT (g != NULL);
	}
    }

  sreloc = NULL;
  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; ++rel)
    {
      unsigned long r_symndx;
      unsigned int r_type;
      struct elf_link_hash_entry *h;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_symndx < extsymoff)
	{
	  h = NULL;
	}
      else if (r_symndx >= extsymoff + NUM_SHDR_ENTRIES (symtab_hdr))
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: malformed reloc detected for section %pA"), abfd, sec);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      else
	{
	  h = sym_hashes[r_symndx - extsymoff];

	  /* This may be an indirect symbol created because of a version.  */
	  if (h != NULL)
	    {
	      while (h->root.type == bfd_link_hash_indirect)
		h = (struct elf_link_hash_entry *)h->root.u.i.link;
	    }
	}

      /* Some relocs require a global offset table.  */
      if (dynobj == NULL || sgot == NULL)
	{
	  switch (r_type)
	    {
	    case R_SCORE_GOT15:
	    case R_SCORE_CALL15:
	      if (dynobj == NULL)
		elf_hash_table (info)->dynobj = dynobj = abfd;
	      if (!score_elf_create_got_section (dynobj, info, false))
		return false;
	      g = score_elf_got_info (dynobj, &sgot);
	      break;
	    case R_SCORE_ABS32:
	    case R_SCORE_REL32:
	      if (dynobj == NULL
		  && (bfd_link_pic (info) || h != NULL)
		  && (sec->flags & SEC_ALLOC) != 0)
		elf_hash_table (info)->dynobj = dynobj = abfd;
	      break;
	    default:
	      break;
	    }
	}

      if (!h && (r_type == R_SCORE_GOT_LO16))
	{
	  if (! score_elf_record_local_got_symbol (abfd, r_symndx, rel->r_addend, g))
	    return false;
	}

      switch (r_type)
	{
	case R_SCORE_CALL15:
	  if (h == NULL)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: CALL15 reloc at %#" PRIx64 " not against global symbol"),
		 abfd, (uint64_t) rel->r_offset);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  else
	    {
	      /* This symbol requires a global offset table entry.  */
	      if (! score_elf_record_global_got_symbol (h, abfd, info, g))
		return false;

	      /* We need a stub, not a plt entry for the undefined function.  But we record
		 it as if it needs plt.  See _bfd_elf_adjust_dynamic_symbol.  */
	      h->needs_plt = 1;
	      h->type = STT_FUNC;
	    }
	  break;
	case R_SCORE_GOT15:
	  if (h && ! score_elf_record_global_got_symbol (h, abfd, info, g))
	    return false;
	  break;
	case R_SCORE_ABS32:
	case R_SCORE_REL32:
	  if ((bfd_link_pic (info) || h != NULL)
	      && (sec->flags & SEC_ALLOC) != 0)
	    {
	      if (sreloc == NULL)
		{
		  sreloc = score_elf_rel_dyn_section (dynobj, true);
		  if (sreloc == NULL)
		    return false;
		}
#define SCORE_READONLY_SECTION (SEC_ALLOC | SEC_LOAD | SEC_READONLY)
	      if (bfd_link_pic (info))
		{
		  /* When creating a shared object, we must copy these reloc types into
		     the output file as R_SCORE_REL32 relocs.  We make room for this reloc
		     in the .rel.dyn reloc section.  */
		  score_elf_allocate_dynamic_relocations (dynobj, 1);
		  if ((sec->flags & SCORE_READONLY_SECTION)
		      == SCORE_READONLY_SECTION)
		    /* We tell the dynamic linker that there are
		       relocations against the text segment.  */
		    info->flags |= DF_TEXTREL;
		}
	      else
		{
		  struct score_elf_link_hash_entry *hscore;

		  /* We only need to copy this reloc if the symbol is
		     defined in a dynamic object.  */
		  hscore = (struct score_elf_link_hash_entry *)h;
		  ++hscore->possibly_dynamic_relocs;
		  if ((sec->flags & SCORE_READONLY_SECTION)
		      == SCORE_READONLY_SECTION)
		    /* We need it to tell the dynamic linker if there
		       are relocations against the text segment.  */
		    hscore->readonly_reloc = true;
		}

	      /* Even though we don't directly need a GOT entry for this symbol,
		 a symbol must have a dynamic symbol table index greater that
		 DT_SCORE_GOTSYM if there are dynamic relocations against it.  */
	      if (h != NULL)
		{
		  if (dynobj == NULL)
		    elf_hash_table (info)->dynobj = dynobj = abfd;
		  if (! score_elf_create_got_section (dynobj, info, true))
		    return false;
		  g = score_elf_got_info (dynobj, &sgot);
		  if (! score_elf_record_global_got_symbol (h, abfd, info, g))
		    return false;
		}
	    }
	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_SCORE_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_SCORE_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_offset))
	    return false;
	  break;
	default:
	  break;
	}

      /* We must not create a stub for a symbol that has relocations
	 related to taking the function's address.  */
      switch (r_type)
	{
	default:
	  if (h != NULL)
	    {
	      struct score_elf_link_hash_entry *sh;

	      sh = (struct score_elf_link_hash_entry *) h;
	      sh->no_fn_stub = true;
	    }
	  break;
	case R_SCORE_CALL15:
	  break;
	}
    }

  return true;
}

static bool
s3_bfd_score_elf_add_symbol_hook (bfd *abfd,
				  struct bfd_link_info *info ATTRIBUTE_UNUSED,
				  Elf_Internal_Sym *sym,
				  const char **namep ATTRIBUTE_UNUSED,
				  flagword *flagsp ATTRIBUTE_UNUSED,
				  asection **secp,
				  bfd_vma *valp)
{
  switch (sym->st_shndx)
    {
    case SHN_COMMON:
      if (sym->st_size > elf_gp_size (abfd))
	break;
      /* Fall through.  */
    case SHN_SCORE_SCOMMON:
      *secp = bfd_make_section_old_way (abfd, ".scommon");
      (*secp)->flags |= SEC_IS_COMMON | SEC_SMALL_DATA;
      *valp = sym->st_size;
      break;
    }

  return true;
}

static void
s3_bfd_score_elf_symbol_processing (bfd *abfd, asymbol *asym)
{
  elf_symbol_type *elfsym;

  elfsym = (elf_symbol_type *) asym;
  switch (elfsym->internal_elf_sym.st_shndx)
    {
    case SHN_COMMON:
      if (asym->value > elf_gp_size (abfd))
	break;
      /* Fall through.  */
    case SHN_SCORE_SCOMMON:
      asym->section = &score_elf_scom_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      break;
    }
}

static int
s3_bfd_score_elf_link_output_symbol_hook (struct bfd_link_info *info ATTRIBUTE_UNUSED,
					  const char *name ATTRIBUTE_UNUSED,
					  Elf_Internal_Sym *sym,
					  asection *input_sec,
					  struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  /* If we see a common symbol, which implies a relocatable link, then
     if a symbol was small common in an input file, mark it as small
     common in the output file.  */
  if (sym->st_shndx == SHN_COMMON && strcmp (input_sec->name, ".scommon") == 0)
    sym->st_shndx = SHN_SCORE_SCOMMON;

  return 1;
}

static bool
s3_bfd_score_elf_section_from_bfd_section (bfd *abfd ATTRIBUTE_UNUSED,
					   asection *sec,
					   int *retval)
{
  if (strcmp (bfd_section_name (sec), ".scommon") == 0)
    {
      *retval = SHN_SCORE_SCOMMON;
      return true;
    }

  return false;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can understand.  */
static bool
s3_bfd_score_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
					struct elf_link_hash_entry *h)
{
  bfd *dynobj;
  struct score_elf_link_hash_entry *hscore;
  asection *s;

  dynobj = elf_hash_table (info)->dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
	      && (h->needs_plt
		  || h->is_weakalias
		  || (h->def_dynamic && h->ref_regular && !h->def_regular)));

  /* If this symbol is defined in a dynamic object, we need to copy
     any R_SCORE_ABS32 or R_SCORE_REL32 relocs against it into the output
     file.  */
  hscore = (struct score_elf_link_hash_entry *)h;
  if (!bfd_link_relocatable (info)
      && hscore->possibly_dynamic_relocs != 0
      && (h->root.type == bfd_link_hash_defweak || !h->def_regular))
    {
      score_elf_allocate_dynamic_relocations (dynobj, hscore->possibly_dynamic_relocs);
      if (hscore->readonly_reloc)
	/* We tell the dynamic linker that there are relocations
	   against the text segment.  */
	info->flags |= DF_TEXTREL;
    }

  /* For a function, create a stub, if allowed.  */
  if (!hscore->no_fn_stub && h->needs_plt)
    {
      if (!elf_hash_table (info)->dynamic_sections_created)
	return true;

      /* If this symbol is not defined in a regular file, then set
	 the symbol to the stub location.  This is required to make
	 function pointers compare as equal between the normal
	 executable and the shared library.  */
      if (!h->def_regular)
	{
	  /* We need .stub section.  */
	  s = bfd_get_linker_section (dynobj, SCORE_ELF_STUB_SECTION_NAME);
	  BFD_ASSERT (s != NULL);

	  h->root.u.def.section = s;
	  h->root.u.def.value = s->size;

	  /* XXX Write this stub address somewhere.  */
	  h->plt.offset = s->size;

	  /* Make room for this stub code.  */
	  s->size += SCORE_FUNCTION_STUB_SIZE;

	  /* The last half word of the stub will be filled with the index
	     of this symbol in .dynsym section.  */
	  return true;
	}
    }
  else if ((h->type == STT_FUNC) && !h->needs_plt)
    {
      /* This will set the entry for this symbol in the GOT to 0, and
	 the dynamic linker will take care of this.  */
      h->root.u.def.value = 0;
      return true;
    }

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
  return true;
}

/* This function is called after all the input files have been read,
   and the input sections have been assigned to output sections.  */
static bool
s3_bfd_score_elf_always_size_sections (bfd *output_bfd,
				       struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *s;
  struct score_got_info *g;
  int i;
  bfd_size_type loadable_size = 0;
  bfd_size_type local_gotno;
  bfd *sub;

  dynobj = elf_hash_table (info)->dynobj;
  if (dynobj == NULL)
    /* Relocatable links don't have it.  */
    return true;

  g = score_elf_got_info (dynobj, &s);
  if (s == NULL)
    return true;

  /* Calculate the total loadable size of the output.  That will give us the
     maximum number of GOT_PAGE entries required.  */
  for (sub = info->input_bfds; sub; sub = sub->link.next)
    {
      asection *subsection;

      for (subsection = sub->sections;
	   subsection;
	   subsection = subsection->next)
	{
	  if ((subsection->flags & SEC_ALLOC) == 0)
	    continue;
	  loadable_size += ((subsection->size + 0xf)
			    &~ (bfd_size_type) 0xf);
	}
    }

  /* There has to be a global GOT entry for every symbol with
     a dynamic symbol table index of DT_SCORE_GOTSYM or
     higher.  Therefore, it make sense to put those symbols
     that need GOT entries at the end of the symbol table.  We
     do that here.  */
  if (! score_elf_sort_hash_table (info, 1))
    return false;

  if (g->global_gotsym != NULL)
    i = elf_hash_table (info)->dynsymcount - g->global_gotsym->dynindx;
  else
    /* If there are no global symbols, or none requiring
       relocations, then GLOBAL_GOTSYM will be NULL.  */
    i = 0;

  /* In the worst case, we'll get one stub per dynamic symbol.  */
  loadable_size += SCORE_FUNCTION_STUB_SIZE * i;

  /* Assume there are two loadable segments consisting of
     contiguous sections.  Is 5 enough?  */
  local_gotno = (loadable_size >> 16) + 5;

  g->local_gotno += local_gotno;
  s->size += g->local_gotno * SCORE_ELF_GOT_SIZE (output_bfd);

  g->global_gotno = i;
  s->size += i * SCORE_ELF_GOT_SIZE (output_bfd);

  score_elf_resolve_final_got_entries (g);

  if (s->size > SCORE_ELF_GOT_MAX_SIZE (output_bfd))
    {
      /* Fixme. Error message or Warning message should be issued here.  */
    }

  return true;
}

/* Set the sizes of the dynamic sections.  */
static bool
s3_bfd_score_elf_size_dynamic_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *s;
  bool reltext;

  dynobj = elf_hash_table (info)->dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size = strlen (ELF_DYNAMIC_INTERPRETER) + 1;
	  s->contents = (bfd_byte *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  reltext = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char *name;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      /* It's OK to base decisions on the section name, because none
	 of the dynobj section names depend upon the input files.  */
      name = bfd_section_name (s);

      if (startswith (name, ".rel"))
	{
	  if (s->size == 0)
	    {
	      /* We only strip the section if the output section name
		 has the same name.  Otherwise, there might be several
		 input sections for this output section.  FIXME: This
		 code is probably not needed these days anyhow, since
		 the linker now does not create empty output sections.  */
	      if (s->output_section != NULL
		  && strcmp (name,
			     bfd_section_name (s->output_section)) == 0)
		s->flags |= SEC_EXCLUDE;
	    }
	  else
	    {
	      const char *outname;
	      asection *target;

	      /* If this relocation section applies to a read only
		 section, then we probably need a DT_TEXTREL entry.
		 If the relocation section is .rel.dyn, we always
		 assert a DT_TEXTREL entry rather than testing whether
		 there exists a relocation to a read only section or
		 not.  */
	      outname = bfd_section_name (s->output_section);
	      target = bfd_get_section_by_name (output_bfd, outname + 4);
	      if ((target != NULL
		   && (target->flags & SEC_READONLY) != 0
		   && (target->flags & SEC_ALLOC) != 0) || strcmp (outname, ".rel.dyn") == 0)
		reltext = true;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      if (strcmp (name, ".rel.dyn") != 0)
		s->reloc_count = 0;
	    }
	}
      else if (startswith (name, ".got"))
	{
	  /* s3_bfd_score_elf_always_size_sections() has already done
	     most of the work, but some symbols may have been mapped
	     to versions that we must now resolve in the got_entries
	     hash tables.  */
	}
      else if (strcmp (name, SCORE_ELF_STUB_SECTION_NAME) == 0)
	{
	  /* IRIX rld assumes that the function stub isn't at the end
	     of .text section. So put a dummy. XXX  */
	  s->size += SCORE_FUNCTION_STUB_SIZE;
	}
      else if (! startswith (name, ".init"))
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      /* Allocate memory for the section contents.  */
      s->contents = bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL && s->size != 0)
	{
	  bfd_set_error (bfd_error_no_memory);
	  return false;
	}
    }

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in s3_bfd_score_elf_finish_dynamic_sections, but we
	 must add the entries now so that we get the correct size for
	 the .dynamic section.  The DT_DEBUG entry is filled in by the
	 dynamic linker and used by the debugger.  */

      if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_DEBUG, 0))
	return false;

      if (reltext)
	info->flags |= DF_TEXTREL;

      if ((info->flags & DF_TEXTREL) != 0)
	{
	  if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_TEXTREL, 0))
	    return false;
	}

      if (! SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_PLTGOT, 0))
	return false;

      if (score_elf_rel_dyn_section (dynobj, false))
	{
	  if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_REL, 0))
	    return false;

	  if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_RELSZ, 0))
	    return false;

	  if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_RELENT, 0))
	    return false;
	}

      if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_SCORE_BASE_ADDRESS, 0))
	return false;

      if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_SCORE_LOCAL_GOTNO, 0))
	return false;

      if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_SCORE_SYMTABNO, 0))
	return false;

      if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_SCORE_UNREFEXTNO, 0))
	return false;

      if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_SCORE_GOTSYM, 0))
	return false;

      if (!SCORE_ELF_ADD_DYNAMIC_ENTRY (info, DT_SCORE_HIPAGENO, 0))
	return false;
    }

  return true;
}

static bool
s3_bfd_score_elf_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_link_hash_entry *h;
  struct bfd_link_hash_entry *bh;
  flagword flags;
  asection *s;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED | SEC_READONLY);

  /* ABI requests the .dynamic section to be read only.  */
  s = bfd_get_linker_section (abfd, ".dynamic");
  if (s != NULL)
    {
      if (!bfd_set_section_flags (s, flags))
	return false;
    }

  /* We need to create .got section.  */
  if (!score_elf_create_got_section (abfd, info, false))
    return false;

  if (!score_elf_rel_dyn_section (elf_hash_table (info)->dynobj, true))
    return false;

  /* Create .stub section.  */
  if (bfd_get_linker_section (abfd, SCORE_ELF_STUB_SECTION_NAME) == NULL)
    {
      s = bfd_make_section_anyway_with_flags (abfd, SCORE_ELF_STUB_SECTION_NAME,
					      flags | SEC_CODE);
      if (s == NULL
	  || !bfd_set_section_alignment (s, 2))

	return false;
    }

  if (!bfd_link_pic (info))
    {
      const char *name;

      name = "_DYNAMIC_LINK";
      bh = NULL;
      if (!(_bfd_generic_link_add_one_symbol
	    (info, abfd, name, BSF_GLOBAL, bfd_abs_section_ptr,
	     (bfd_vma) 0, NULL, false, get_elf_backend_data (abfd)->collect, &bh)))
	return false;

      h = (struct elf_link_hash_entry *)bh;
      h->non_elf = 0;
      h->def_regular = 1;
      h->type = STT_SECTION;

      if (!bfd_elf_link_record_dynamic_symbol (info, h))
	return false;
    }

  return true;
}


/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */
static bool
s3_bfd_score_elf_finish_dynamic_symbol (bfd *output_bfd,
					struct bfd_link_info *info,
					struct elf_link_hash_entry *h,
					Elf_Internal_Sym *sym)
{
  bfd *dynobj;
  asection *sgot;
  struct score_got_info *g;
  const char *name;

  dynobj = elf_hash_table (info)->dynobj;

  if (h->plt.offset != MINUS_ONE)
    {
      asection *s;
      bfd_byte stub[SCORE_FUNCTION_STUB_SIZE];

      /* This symbol has a stub.  Set it up.  */
      BFD_ASSERT (h->dynindx != -1);

      s = bfd_get_linker_section (dynobj, SCORE_ELF_STUB_SECTION_NAME);
      BFD_ASSERT (s != NULL);

      /* FIXME: Can h->dynindex be more than 64K?  */
      if (h->dynindx & 0xffff0000)
	return false;

      /* Fill the stub.  */
      score_bfd_put_32 (output_bfd, STUB_LW, stub);
      score_bfd_put_32 (output_bfd, STUB_MOVE, stub + 4);
      score_bfd_put_32 (output_bfd, STUB_LI16 | (h->dynindx << 1), stub + 8);
      score_bfd_put_32 (output_bfd, STUB_BRL, stub + 12);

      BFD_ASSERT (h->plt.offset <= s->size);
      memcpy (s->contents + h->plt.offset, stub, SCORE_FUNCTION_STUB_SIZE);

      /* Mark the symbol as undefined.  plt.offset != -1 occurs
	 only for the referenced symbol.  */
      sym->st_shndx = SHN_UNDEF;

      /* The run-time linker uses the st_value field of the symbol
	  to reset the global offset table entry for this external
	  to its stub address when unlinking a shared object.  */
      sym->st_value = (s->output_section->vma + s->output_offset + h->plt.offset);
    }

  BFD_ASSERT (h->dynindx != -1 || h->forced_local);

  sgot = score_elf_got_section (dynobj, false);
  BFD_ASSERT (sgot != NULL);
  BFD_ASSERT (score_elf_section_data (sgot) != NULL);
  g = score_elf_section_data (sgot)->u.got_info;
  BFD_ASSERT (g != NULL);

  /* Run through the global symbol table, creating GOT entries for all
     the symbols that need them.  */
  if (g->global_gotsym != NULL && h->dynindx >= g->global_gotsym->dynindx)
    {
      bfd_vma offset;
      bfd_vma value;

      value = sym->st_value;
      offset = score_elf_global_got_index (dynobj, h);
      score_bfd_put_32 (output_bfd, value, sgot->contents + offset);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  name = h->root.root.string;
  if (h == elf_hash_table (info)->hdynamic
      || h == elf_hash_table (info)->hgot)
    sym->st_shndx = SHN_ABS;
  else if (strcmp (name, "_DYNAMIC_LINK") == 0)
    {
      sym->st_shndx = SHN_ABS;
      sym->st_info = ELF_ST_INFO (STB_GLOBAL, STT_SECTION);
      sym->st_value = 1;
    }
  else if (strcmp (name, GP_DISP_LABEL) == 0)
    {
      sym->st_shndx = SHN_ABS;
      sym->st_info = ELF_ST_INFO (STB_GLOBAL, STT_SECTION);
      sym->st_value = elf_gp (output_bfd);
    }

  return true;
}

/* Finish up the dynamic sections.  */
static bool
s3_bfd_score_elf_finish_dynamic_sections (bfd *output_bfd,
					  struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn;
  asection *sgot;
  asection *s;
  struct score_got_info *g;

  dynobj = elf_hash_table (info)->dynobj;

  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  sgot = score_elf_got_section (dynobj, false);
  if (sgot == NULL)
    g = NULL;
  else
    {
      BFD_ASSERT (score_elf_section_data (sgot) != NULL);
      g = score_elf_section_data (sgot)->u.got_info;
      BFD_ASSERT (g != NULL);
    }

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      bfd_byte *b;

      BFD_ASSERT (sdyn != NULL);
      BFD_ASSERT (g != NULL);

      for (b = sdyn->contents;
	   b < sdyn->contents + sdyn->size;
	   b += SCORE_ELF_DYN_SIZE (dynobj))
	{
	  Elf_Internal_Dyn dyn;
	  const char *name;
	  size_t elemsize;
	  bool swap_out_p;

	  /* Read in the current dynamic entry.  */
	  (*get_elf_backend_data (dynobj)->s->swap_dyn_in) (dynobj, b, &dyn);

	  /* Assume that we're going to modify it and write it out.  */
	  swap_out_p = true;

	  switch (dyn.d_tag)
	    {
	    case DT_RELENT:
	      dyn.d_un.d_val = SCORE_ELF_REL_SIZE (dynobj);
	      break;

	    case DT_STRSZ:
	      /* Rewrite DT_STRSZ.  */
	      dyn.d_un.d_val
		= _bfd_elf_strtab_size (elf_hash_table (info)->dynstr);
	      break;

	    case DT_PLTGOT:
	      s = elf_hash_table (info)->sgot;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_SCORE_BASE_ADDRESS:
	      s = output_bfd->sections;
	      BFD_ASSERT (s != NULL);
	      dyn.d_un.d_ptr = s->vma & ~(bfd_vma) 0xffff;
	      break;

	    case DT_SCORE_LOCAL_GOTNO:
	      dyn.d_un.d_val = g->local_gotno;
	      break;

	    case DT_SCORE_UNREFEXTNO:
	      /* The index into the dynamic symbol table which is the
		 entry of the first external symbol that is not
		 referenced within the same object.  */
	      dyn.d_un.d_val = bfd_count_sections (output_bfd) + 1;
	      break;

	    case DT_SCORE_GOTSYM:
	      if (g->global_gotsym)
		{
		  dyn.d_un.d_val = g->global_gotsym->dynindx;
		  break;
		}
	      /* In case if we don't have global got symbols we default
		  to setting DT_SCORE_GOTSYM to the same value as
		  DT_SCORE_SYMTABNO.  */
	      /* Fall through.  */

	    case DT_SCORE_SYMTABNO:
	      name = ".dynsym";
	      elemsize = SCORE_ELF_SYM_SIZE (output_bfd);
	      s = bfd_get_linker_section (dynobj, name);
	      dyn.d_un.d_val = s->size / elemsize;
	      break;

	    case DT_SCORE_HIPAGENO:
	      dyn.d_un.d_val = g->local_gotno - SCORE_RESERVED_GOTNO;
	      break;

	    default:
	      swap_out_p = false;
	      break;
	    }

	  if (swap_out_p)
	    (*get_elf_backend_data (dynobj)->s->swap_dyn_out) (dynobj, &dyn, b);
	}
    }

  /* The first entry of the global offset table will be filled at
     runtime. The second entry will be used by some runtime loaders.
     This isn't the case of IRIX rld.  */
  if (sgot != NULL && sgot->size > 0)
    {
      score_bfd_put_32 (output_bfd, 0, sgot->contents);
      score_bfd_put_32 (output_bfd, 0x80000000, sgot->contents + SCORE_ELF_GOT_SIZE (output_bfd));
    }

  if (sgot != NULL)
    elf_section_data (sgot->output_section)->this_hdr.sh_entsize
      = SCORE_ELF_GOT_SIZE (output_bfd);


  /* We need to sort the entries of the dynamic relocation section.  */
  s = score_elf_rel_dyn_section (dynobj, false);

  if (s != NULL && s->size > (bfd_vma)2 * SCORE_ELF_REL_SIZE (output_bfd))
    {
      reldyn_sorting_bfd = output_bfd;
      qsort ((Elf32_External_Rel *) s->contents + 1, s->reloc_count - 1,
	     sizeof (Elf32_External_Rel), score_elf_sort_dynamic_relocs);
    }

  return true;
}

/* This function set up the ELF section header for a BFD section in preparation for writing
   it out.  This is where the flags and type fields are set for unusual sections.  */
static bool
s3_bfd_score_elf_fake_sections (bfd *abfd ATTRIBUTE_UNUSED,
				Elf_Internal_Shdr *hdr,
				asection *sec)
{
  const char *name;

  name = bfd_section_name (sec);

  if (strcmp (name, ".got") == 0
      || strcmp (name, ".srdata") == 0
      || strcmp (name, ".sdata") == 0
      || strcmp (name, ".sbss") == 0)
    hdr->sh_flags |= SHF_SCORE_GPREL;

  return true;
}

/* This function do additional processing on the ELF section header before writing
   it out.  This is used to set the flags and type fields for some sections.  */

/* assign_file_positions_except_relocs() check section flag and if it is allocatable,
   warning message will be issued.  backend_fake_section is called before
   assign_file_positions_except_relocs(); backend_section_processing after it.  so, we
   modify section flag there, but not backend_fake_section.  */
static bool
s3_bfd_score_elf_section_processing (bfd *abfd ATTRIBUTE_UNUSED, Elf_Internal_Shdr *hdr)
{
  if (hdr->bfd_section != NULL)
    {
      const char *name = bfd_section_name (hdr->bfd_section);

      if (strcmp (name, ".sdata") == 0)
	{
	  hdr->sh_flags |= SHF_ALLOC | SHF_WRITE | SHF_SCORE_GPREL;
	  hdr->sh_type = SHT_PROGBITS;
	}
      else if (strcmp (name, ".sbss") == 0)
	{
	  hdr->sh_flags |= SHF_ALLOC | SHF_WRITE | SHF_SCORE_GPREL;
	  hdr->sh_type = SHT_NOBITS;
	}
      else if (strcmp (name, ".srdata") == 0)
	{
	  hdr->sh_flags |= SHF_ALLOC | SHF_SCORE_GPREL;
	  hdr->sh_type = SHT_PROGBITS;
	}
    }

  return true;
}

static bool
s3_bfd_score_elf_write_section (bfd *output_bfd, asection *sec, bfd_byte *contents)
{
  bfd_byte *to, *from, *end;
  int i;

  if (strcmp (sec->name, ".pdr") != 0)
    return false;

  if (score_elf_section_data (sec)->u.tdata == NULL)
    return false;

  to = contents;
  end = contents + sec->size;
  for (from = contents, i = 0; from < end; from += PDR_SIZE, i++)
    {
      if ((score_elf_section_data (sec)->u.tdata)[i] == 1)
	continue;

      if (to != from)
	memcpy (to, from, PDR_SIZE);

      to += PDR_SIZE;
    }
  bfd_set_section_contents (output_bfd, sec->output_section, contents,
			    (file_ptr) sec->output_offset, sec->size);

  return true;
}

/* Copy data from a SCORE ELF indirect symbol to its direct symbol, hiding the old
   indirect symbol.  Process additional relocation information.  */
static void
s3_bfd_score_elf_copy_indirect_symbol (struct bfd_link_info *info,
				       struct elf_link_hash_entry *dir,
				       struct elf_link_hash_entry *ind)
{
  struct score_elf_link_hash_entry *dirscore, *indscore;

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);

  if (ind->root.type != bfd_link_hash_indirect)
    return;

  dirscore = (struct score_elf_link_hash_entry *) dir;
  indscore = (struct score_elf_link_hash_entry *) ind;
  dirscore->possibly_dynamic_relocs += indscore->possibly_dynamic_relocs;

  if (indscore->readonly_reloc)
    dirscore->readonly_reloc = true;

  if (indscore->no_fn_stub)
    dirscore->no_fn_stub = true;
}

/* Remove information about discarded functions from other sections which mention them.  */
static bool
s3_bfd_score_elf_discard_info (bfd *abfd, struct elf_reloc_cookie *cookie,
			       struct bfd_link_info *info)
{
  asection *o;
  bool ret = false;
  unsigned char *tdata;
  size_t i, skip;

  o = bfd_get_section_by_name (abfd, ".pdr");
  if ((!o) || (o->size == 0) || (o->size % PDR_SIZE != 0)
      || (o->output_section != NULL && bfd_is_abs_section (o->output_section)))
    return false;

  tdata = bfd_zmalloc (o->size / PDR_SIZE);
  if (!tdata)
    return false;

  cookie->rels = _bfd_elf_link_read_relocs (abfd, o, NULL, NULL, info->keep_memory);
  if (!cookie->rels)
    {
      free (tdata);
      return false;
    }

  cookie->rel = cookie->rels;
  cookie->relend = cookie->rels + o->reloc_count;

  for (i = 0, skip = 0; i < o->size; i++)
    {
      if (bfd_elf_reloc_symbol_deleted_p (i * PDR_SIZE, cookie))
	{
	  tdata[i] = 1;
	  skip++;
	}
    }

  if (skip != 0)
    {
      score_elf_section_data (o)->u.tdata = tdata;
      o->size -= skip * PDR_SIZE;
      ret = true;
    }
  else
    free (tdata);

  if (!info->keep_memory)
    free (cookie->rels);

  return ret;
}

/* Signal that discard_info() has removed the discarded relocations for this section.  */
static bool
s3_bfd_score_elf_ignore_discarded_relocs (asection *sec)
{
  if (strcmp (sec->name, ".pdr") == 0)
    return true;
  return false;
}

/* Return the section that should be marked against GC for a given
   relocation.  */
static asection *
s3_bfd_score_elf_gc_mark_hook (asection *sec,
			       struct bfd_link_info *info,
			       Elf_Internal_Rela *rel,
			       struct elf_link_hash_entry *h,
			       Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_SCORE_GNU_VTINHERIT:
      case R_SCORE_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Support for core dump NOTE sections.  */

static bool
s3_bfd_score_elf_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  unsigned int raw_size;

  switch (note->descsz)
    {
    default:
      return false;

    case 148:		       /* Linux/Score 32-bit.  */
      /* pr_cursig */
      elf_tdata (abfd)->core->signal
	= score_bfd_get_16 (abfd, note->descdata + 12);

      /* pr_pid */
      elf_tdata (abfd)->core->lwpid
	= score_bfd_get_32 (abfd, note->descdata + 24);

      /* pr_reg */
      offset = 72;
      raw_size = 72;

      break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg", raw_size,
					  note->descpos + offset);
}

static bool
s3_bfd_score_elf_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    default:
      return false;

    case 124:		       /* Linux/Score elf_prpsinfo.  */
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, note->descdata + 28, 16);
      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, note->descdata + 44, 80);
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


/* Score BFD functions.  */
static reloc_howto_type *
s3_elf32_score_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (elf32_score_reloc_map); i++)
    if (elf32_score_reloc_map[i].bfd_reloc_val == code)
      return &elf32_score_howto_table[elf32_score_reloc_map[i].elf_reloc_val];

  return NULL;
}

static reloc_howto_type *
elf32_score_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			       const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (elf32_score_howto_table)
	    / sizeof (elf32_score_howto_table[0]));
       i++)
    if (elf32_score_howto_table[i].name != NULL
	&& strcasecmp (elf32_score_howto_table[i].name, r_name) == 0)
      return &elf32_score_howto_table[i];

  return NULL;
}

static bool
s3_elf32_score_print_private_bfd_data (bfd *abfd, void * ptr)
{
  FILE *file = (FILE *) ptr;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);
  if (elf_elfheader (abfd)->e_flags & EF_SCORE_PIC)
    {
      fprintf (file, _(" [pic]"));
    }
  if (elf_elfheader (abfd)->e_flags & EF_SCORE_FIXDEP)
    {
      fprintf (file, _(" [fix dep]"));
    }
  fputc ('\n', file);

  return true;
}

static bool
s3_elf32_score_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword in_flags;
  flagword out_flags;

  if (!_bfd_generic_verify_endian_match (ibfd, info))
    return false;

  /* FIXME: What should be checked when linking shared libraries?  */
  if ((ibfd->flags & DYNAMIC) != 0)
    return true;

  in_flags  = elf_elfheader (ibfd)->e_flags;
  out_flags = elf_elfheader (obfd)->e_flags;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  in_flags = elf_elfheader (ibfd)->e_flags;
  out_flags = elf_elfheader (obfd)->e_flags;

  if (! elf_flags_init (obfd))
    {
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = in_flags;

      if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
	  && bfd_get_arch_info (obfd)->the_default)
	{
	  return bfd_set_arch_mach (obfd, bfd_get_arch (ibfd), bfd_get_mach (ibfd));
	}

      return true;
    }

  if (((in_flags & EF_SCORE_PIC) != 0) != ((out_flags & EF_SCORE_PIC) != 0))
    _bfd_error_handler
      (_("%pB: warning: linking PIC files with non-PIC files"), ibfd);

  /* FIXME: Maybe dependency fix compatibility should be checked here.  */

  return true;
}

static bool
s3_elf32_score_new_section_hook (bfd *abfd, asection *sec)
{
  struct _score_elf_section_data *sdata;
  size_t amt = sizeof (*sdata);

  sdata = bfd_zalloc (abfd, amt);
  if (sdata == NULL)
    return false;
  sec->used_by_bfd = sdata;

  return _bfd_elf_new_section_hook (abfd, sec);
}

/*****************************************************************************/

/* s3_s7: backend hooks.  */
static bool
_bfd_score_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
			  arelent *bfd_reloc,
			  Elf_Internal_Rela *elf_reloc)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_info_to_howto (abfd, bfd_reloc, elf_reloc);
  else
    return s7_bfd_score_info_to_howto (abfd, bfd_reloc, elf_reloc);
}

static int
_bfd_score_elf_relocate_section (bfd *output_bfd,
				 struct bfd_link_info *info,
				 bfd *input_bfd,
				 asection *input_section,
				 bfd_byte *contents,
				 Elf_Internal_Rela *relocs,
				 Elf_Internal_Sym *local_syms,
				 asection **local_sections)
{
  if (bfd_get_mach (output_bfd) == bfd_mach_score3)
    return s3_bfd_score_elf_relocate_section (output_bfd,
	     info, input_bfd, input_section, contents, relocs,
	     local_syms, local_sections);
  else
    return s7_bfd_score_elf_relocate_section (output_bfd,
	     info, input_bfd, input_section, contents, relocs,
	     local_syms, local_sections);
}

static bool
_bfd_score_elf_check_relocs (bfd *abfd,
			     struct bfd_link_info *info,
			     asection *sec,
			     const Elf_Internal_Rela *relocs)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_elf_check_relocs (abfd, info, sec, relocs);
  else
    return s7_bfd_score_elf_check_relocs (abfd, info, sec, relocs);
}

static bool
_bfd_score_elf_add_symbol_hook (bfd *abfd,
				struct bfd_link_info *info ATTRIBUTE_UNUSED,
				Elf_Internal_Sym *sym,
				const char **namep ATTRIBUTE_UNUSED,
				flagword *flagsp ATTRIBUTE_UNUSED,
				asection **secp,
				bfd_vma *valp)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_elf_add_symbol_hook (abfd, info, sym, namep, flagsp,
					     secp, valp);
  else
    return s7_bfd_score_elf_add_symbol_hook (abfd, info, sym, namep, flagsp,
					     secp, valp);
}

static void
_bfd_score_elf_symbol_processing (bfd *abfd, asymbol *asym)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_elf_symbol_processing (abfd, asym);
  else
    return s7_bfd_score_elf_symbol_processing (abfd, asym);
}

static int
_bfd_score_elf_link_output_symbol_hook (struct bfd_link_info *info ATTRIBUTE_UNUSED,
     const char *name ATTRIBUTE_UNUSED,
     Elf_Internal_Sym *sym,
     asection *input_sec,
     struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  /* If link a empty .o, then this filed is NULL.  */
  if (info->input_bfds == NULL)
    {
      /* If we see a common symbol, which implies a relocatable link, then
	 if a symbol was small common in an input file, mark it as small
	 common in the output file.  */
      if (sym->st_shndx == SHN_COMMON && strcmp (input_sec->name, ".scommon") == 0)
	sym->st_shndx = SHN_SCORE_SCOMMON;
      return 1;
    }

  if (bfd_get_mach (info->input_bfds) == bfd_mach_score3)
    return s3_bfd_score_elf_link_output_symbol_hook (info, name, sym, input_sec, h);
  else
    return s7_bfd_score_elf_link_output_symbol_hook (info, name, sym, input_sec, h);
}

static bool
_bfd_score_elf_section_from_bfd_section (bfd *abfd ATTRIBUTE_UNUSED,
					 asection *sec,
					 int *retval)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_elf_section_from_bfd_section (abfd, sec, retval);
  else
    return s7_bfd_score_elf_section_from_bfd_section (abfd, sec, retval);
}

static bool
_bfd_score_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				      struct elf_link_hash_entry *h)
{
  if (bfd_get_mach (info->input_bfds) == bfd_mach_score3)
    return s3_bfd_score_elf_adjust_dynamic_symbol (info, h);
  else
    return s7_bfd_score_elf_adjust_dynamic_symbol (info, h);
}

static bool
_bfd_score_elf_always_size_sections (bfd *output_bfd,
				     struct bfd_link_info *info)
{
  if (bfd_get_mach (output_bfd) == bfd_mach_score3)
    return s3_bfd_score_elf_always_size_sections (output_bfd, info);
  else
    return s7_bfd_score_elf_always_size_sections (output_bfd, info);
}

static bool
_bfd_score_elf_size_dynamic_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  if (bfd_get_mach (output_bfd) == bfd_mach_score3)
    return s3_bfd_score_elf_size_dynamic_sections (output_bfd, info);
  else
    return s7_bfd_score_elf_size_dynamic_sections (output_bfd, info);
}

static bool
_bfd_score_elf_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_elf_create_dynamic_sections (abfd, info);
  else
    return s7_bfd_score_elf_create_dynamic_sections (abfd, info);
}

static bool
_bfd_score_elf_finish_dynamic_symbol (bfd *output_bfd,
				      struct bfd_link_info *info,
				      struct elf_link_hash_entry *h,
				      Elf_Internal_Sym *sym)
{
  if (bfd_get_mach (output_bfd) == bfd_mach_score3)
    return s3_bfd_score_elf_finish_dynamic_symbol (output_bfd, info, h, sym);
  else
    return s7_bfd_score_elf_finish_dynamic_symbol (output_bfd, info, h, sym);
}

static bool
_bfd_score_elf_finish_dynamic_sections (bfd *output_bfd,
					struct bfd_link_info *info)
{
  if (bfd_get_mach (output_bfd) == bfd_mach_score3)
    return s3_bfd_score_elf_finish_dynamic_sections (output_bfd, info);
  else
    return s7_bfd_score_elf_finish_dynamic_sections (output_bfd, info);
}

static bool
_bfd_score_elf_fake_sections (bfd *abfd ATTRIBUTE_UNUSED,
			      Elf_Internal_Shdr *hdr,
			      asection *sec)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_elf_fake_sections (abfd, hdr, sec);
  else
    return s7_bfd_score_elf_fake_sections (abfd, hdr, sec);
}

static bool
_bfd_score_elf_section_processing (bfd *abfd ATTRIBUTE_UNUSED, Elf_Internal_Shdr *hdr)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_elf_section_processing (abfd, hdr);
  else
    return s7_bfd_score_elf_section_processing (abfd, hdr);
}

static bool
_bfd_score_elf_write_section (bfd *output_bfd,
			      struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			      asection *sec, bfd_byte *contents)
{
  if (bfd_get_mach (output_bfd) == bfd_mach_score3)
    return s3_bfd_score_elf_write_section (output_bfd, sec, contents);
  else
    return s7_bfd_score_elf_write_section (output_bfd, sec, contents);
}

static void
_bfd_score_elf_copy_indirect_symbol (struct bfd_link_info *info,
				     struct elf_link_hash_entry *dir,
				     struct elf_link_hash_entry *ind)
{
  if (bfd_get_mach (info->input_bfds) == bfd_mach_score3)
    return s3_bfd_score_elf_copy_indirect_symbol (info, dir, ind);
  else
    return s7_bfd_score_elf_copy_indirect_symbol (info, dir, ind);
}

static void
_bfd_score_elf_hide_symbol (struct bfd_link_info *info,
			    struct elf_link_hash_entry *entry,
			    bool force_local)
{
  if (bfd_get_mach (info->input_bfds) == bfd_mach_score3)
    return s3_bfd_score_elf_hide_symbol (info, entry, force_local);
  else
    return s7_bfd_score_elf_hide_symbol (info, entry, force_local);
}

static bool
_bfd_score_elf_discard_info (bfd *abfd, struct elf_reloc_cookie *cookie,
			 struct bfd_link_info *info)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_elf_discard_info (abfd, cookie, info);
  else
    return s7_bfd_score_elf_discard_info (abfd, cookie, info);
}

static bool
_bfd_score_elf_ignore_discarded_relocs (asection *sec)
{
  if (bfd_get_mach (sec->owner) == bfd_mach_score3)
    return s3_bfd_score_elf_ignore_discarded_relocs (sec);
  else
    return s7_bfd_score_elf_ignore_discarded_relocs (sec);
}

static asection *
_bfd_score_elf_gc_mark_hook (asection *sec,
			     struct bfd_link_info *info,
			     Elf_Internal_Rela *rel,
			     struct elf_link_hash_entry *h,
			     Elf_Internal_Sym *sym)
{
  if (bfd_get_mach (info->input_bfds) == bfd_mach_score3)
    return s3_bfd_score_elf_gc_mark_hook (sec, info, rel, h, sym);
  else
    return s7_bfd_score_elf_gc_mark_hook (sec, info, rel, h, sym);
}

static bool
_bfd_score_elf_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_elf_grok_prstatus (abfd, note);
  else
    return s7_bfd_score_elf_grok_prstatus (abfd, note);
}

static bool
_bfd_score_elf_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_bfd_score_elf_grok_psinfo (abfd, note);
  else
    return s7_bfd_score_elf_grok_psinfo (abfd, note);
}

static reloc_howto_type *
elf32_score_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code)
{
  /* s3: NOTE!!!
     gas will call elf32_score_reloc_type_lookup, and don't write elf file.
     So just using score3, but we don't know ld will call this or not.
     If so, this way can't work.  */

  if (score3)
    return s3_elf32_score_reloc_type_lookup (abfd, code);
  else
    return s7_elf32_score_reloc_type_lookup (abfd, code);
}

/* Create a score elf linker hash table.
   This is a copy of _bfd_elf_link_hash_table_create() except with a
   different hash table entry creation function.  */

static struct bfd_link_hash_table *
elf32_score_link_hash_table_create (bfd *abfd)
{
  struct elf_link_hash_table *ret;
  size_t amt = sizeof (struct elf_link_hash_table);

  ret = (struct elf_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (ret, abfd, score_elf_link_hash_newfunc,
				      sizeof (struct score_elf_link_hash_entry),
				      GENERIC_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->root;
}

static bool
elf32_score_print_private_bfd_data (bfd *abfd, void * ptr)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_elf32_score_print_private_bfd_data (abfd, ptr);
  else
    return s7_elf32_score_print_private_bfd_data (abfd, ptr);
}

static bool
elf32_score_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  if (bfd_get_mach (info->output_bfd) == bfd_mach_score3)
    return s3_elf32_score_merge_private_bfd_data (ibfd, info);
  else
    return s7_elf32_score_merge_private_bfd_data (ibfd, info);
}

static bool
elf32_score_new_section_hook (bfd *abfd, asection *sec)
{
  if (bfd_get_mach (abfd) == bfd_mach_score3)
    return s3_elf32_score_new_section_hook (abfd, sec);
  else
    return s7_elf32_score_new_section_hook (abfd, sec);
}


/* s3_s7: don't need to split.  */

/* Set the right machine number.  */
static bool
_bfd_score_elf_score_object_p (bfd * abfd)
{
  int e_set = bfd_mach_score7;

  if (elf_elfheader (abfd)->e_machine == EM_SCORE)
    {
      int e_mach = elf_elfheader (abfd)->e_flags & EF_SCORE_MACH & EF_OMIT_PIC_FIXDD;
      switch (e_mach)
	{
	/* Set default target is score7.  */
	default:
	case E_SCORE_MACH_SCORE7:
	  e_set = bfd_mach_score7;
	  break;

	case E_SCORE_MACH_SCORE3:
	  e_set = bfd_mach_score3;
	  break;
	}
    }

  return bfd_default_set_arch_mach (abfd, bfd_arch_score, e_set);
}

bool
_bfd_score_elf_common_definition (Elf_Internal_Sym *sym)
{
  return (sym->st_shndx == SHN_COMMON || sym->st_shndx == SHN_SCORE_SCOMMON);
}

/*****************************************************************************/


#define USE_REL				1
#define TARGET_LITTLE_SYM		score_elf32_le_vec
#define TARGET_LITTLE_NAME		"elf32-littlescore"
#define TARGET_BIG_SYM			score_elf32_be_vec
#define TARGET_BIG_NAME			"elf32-bigscore"
#define ELF_ARCH			bfd_arch_score
#define ELF_MACHINE_CODE		EM_SCORE
#define ELF_MACHINE_ALT1		EM_SCORE_OLD
#define ELF_MAXPAGESIZE			0x8000

#define elf_info_to_howto		NULL
#define elf_info_to_howto_rel		_bfd_score_info_to_howto
#define elf_backend_relocate_section	_bfd_score_elf_relocate_section
#define elf_backend_check_relocs	_bfd_score_elf_check_relocs
#define elf_backend_add_symbol_hook	_bfd_score_elf_add_symbol_hook
#define elf_backend_symbol_processing	_bfd_score_elf_symbol_processing
#define elf_backend_link_output_symbol_hook \
  _bfd_score_elf_link_output_symbol_hook
#define elf_backend_section_from_bfd_section \
  _bfd_score_elf_section_from_bfd_section
#define elf_backend_adjust_dynamic_symbol \
  _bfd_score_elf_adjust_dynamic_symbol
#define elf_backend_always_size_sections \
  _bfd_score_elf_always_size_sections
#define elf_backend_size_dynamic_sections \
  _bfd_score_elf_size_dynamic_sections
#define elf_backend_omit_section_dynsym   _bfd_elf_omit_section_dynsym_all
#define elf_backend_create_dynamic_sections \
  _bfd_score_elf_create_dynamic_sections
#define elf_backend_finish_dynamic_symbol \
  _bfd_score_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
  _bfd_score_elf_finish_dynamic_sections
#define elf_backend_fake_sections	  _bfd_score_elf_fake_sections
#define elf_backend_section_processing	  _bfd_score_elf_section_processing
#define elf_backend_write_section	  _bfd_score_elf_write_section
#define elf_backend_copy_indirect_symbol  _bfd_score_elf_copy_indirect_symbol
#define elf_backend_hide_symbol		  _bfd_score_elf_hide_symbol
#define elf_backend_discard_info	  _bfd_score_elf_discard_info
#define elf_backend_ignore_discarded_relocs \
  _bfd_score_elf_ignore_discarded_relocs
#define elf_backend_gc_mark_hook	  _bfd_score_elf_gc_mark_hook
#define elf_backend_grok_prstatus	  _bfd_score_elf_grok_prstatus
#define elf_backend_grok_psinfo		  _bfd_score_elf_grok_psinfo
#define elf_backend_can_gc_sections	  1
#define elf_backend_want_plt_sym	  0
#define elf_backend_got_header_size	  (4 * SCORE_RESERVED_GOTNO)
#define elf_backend_plt_header_size	  0
#define elf_backend_collect		  true
#define elf_backend_type_change_ok	  true
#define elf_backend_object_p		      _bfd_score_elf_score_object_p

#define bfd_elf32_bfd_reloc_type_lookup	     elf32_score_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup \
  elf32_score_reloc_name_lookup
#define bfd_elf32_bfd_link_hash_table_create elf32_score_link_hash_table_create
#define bfd_elf32_bfd_print_private_bfd_data elf32_score_print_private_bfd_data
#define bfd_elf32_bfd_merge_private_bfd_data elf32_score_merge_private_bfd_data
#define bfd_elf32_new_section_hook	     elf32_score_new_section_hook

#include "elf32-target.h"
