/* SPU specific support for 32-bit ELF

   Copyright (C) 2006-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "sysdep.h"
#include "libiberty.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/spu.h"
#include "elf32-spu.h"

/* All users of this file have bfd_octets_per_byte (abfd, sec) == 1.  */
#define OCTETS_PER_BYTE(ABFD, SEC) 1

/* We use RELA style relocs.  Don't define USE_REL.  */

static bfd_reloc_status_type spu_elf_rel9 (bfd *, arelent *, asymbol *,
					   void *, asection *,
					   bfd *, char **);

/* Values of type 'enum elf_spu_reloc_type' are used to index this
   array, so it must be declared in the order of that type.  */

static reloc_howto_type elf_howto_table[] = {
  HOWTO (R_SPU_NONE,	   0, 0,  0, false,  0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "SPU_NONE",
	 false, 0, 0x00000000, false),
  HOWTO (R_SPU_ADDR10,	   4, 4, 10, false, 14, complain_overflow_bitfield,
	 bfd_elf_generic_reloc, "SPU_ADDR10",
	 false, 0, 0x00ffc000, false),
  HOWTO (R_SPU_ADDR16,	   2, 4, 16, false,  7, complain_overflow_bitfield,
	 bfd_elf_generic_reloc, "SPU_ADDR16",
	 false, 0, 0x007fff80, false),
  HOWTO (R_SPU_ADDR16_HI, 16, 4, 16, false,  7, complain_overflow_bitfield,
	 bfd_elf_generic_reloc, "SPU_ADDR16_HI",
	 false, 0, 0x007fff80, false),
  HOWTO (R_SPU_ADDR16_LO,  0, 4, 16, false,  7, complain_overflow_dont,
	 bfd_elf_generic_reloc, "SPU_ADDR16_LO",
	 false, 0, 0x007fff80, false),
  HOWTO (R_SPU_ADDR18,	   0, 4, 18, false,  7, complain_overflow_bitfield,
	 bfd_elf_generic_reloc, "SPU_ADDR18",
	 false, 0, 0x01ffff80, false),
  HOWTO (R_SPU_ADDR32,	   0, 4, 32, false,  0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "SPU_ADDR32",
	 false, 0, 0xffffffff, false),
  HOWTO (R_SPU_REL16,	   2, 4, 16,  true,  7, complain_overflow_bitfield,
	 bfd_elf_generic_reloc, "SPU_REL16",
	 false, 0, 0x007fff80, true),
  HOWTO (R_SPU_ADDR7,	   0, 4,  7, false, 14, complain_overflow_dont,
	 bfd_elf_generic_reloc, "SPU_ADDR7",
	 false, 0, 0x001fc000, false),
  HOWTO (R_SPU_REL9,	   2, 4,  9,  true,  0, complain_overflow_signed,
	 spu_elf_rel9,		"SPU_REL9",
	 false, 0, 0x0180007f, true),
  HOWTO (R_SPU_REL9I,	   2, 4,  9,  true,  0, complain_overflow_signed,
	 spu_elf_rel9,		"SPU_REL9I",
	 false, 0, 0x0000c07f, true),
  HOWTO (R_SPU_ADDR10I,	   0, 4, 10, false, 14, complain_overflow_signed,
	 bfd_elf_generic_reloc, "SPU_ADDR10I",
	 false, 0, 0x00ffc000, false),
  HOWTO (R_SPU_ADDR16I,	   0, 4, 16, false,  7, complain_overflow_signed,
	 bfd_elf_generic_reloc, "SPU_ADDR16I",
	 false, 0, 0x007fff80, false),
  HOWTO (R_SPU_REL32,	   0, 4, 32, true,  0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "SPU_REL32",
	 false, 0, 0xffffffff, true),
  HOWTO (R_SPU_ADDR16X,	   0, 4, 16, false,  7, complain_overflow_bitfield,
	 bfd_elf_generic_reloc, "SPU_ADDR16X",
	 false, 0, 0x007fff80, false),
  HOWTO (R_SPU_PPU32,	   0, 4, 32, false,  0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "SPU_PPU32",
	 false, 0, 0xffffffff, false),
  HOWTO (R_SPU_PPU64,	   0, 8, 64, false,  0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "SPU_PPU64",
	 false, 0, -1, false),
  HOWTO (R_SPU_ADD_PIC,	   0, 0,  0, false,  0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "SPU_ADD_PIC",
	 false, 0, 0x00000000, false),
};

static struct bfd_elf_special_section const spu_elf_special_sections[] = {
  { "._ea", 4, 0, SHT_PROGBITS, SHF_WRITE },
  { ".toe", 4, 0, SHT_NOBITS, SHF_ALLOC },
  { NULL, 0, 0, 0, 0 }
};

static enum elf_spu_reloc_type
spu_elf_bfd_to_reloc_type (bfd_reloc_code_real_type code)
{
  switch (code)
    {
    default:
      return (enum elf_spu_reloc_type) -1;
    case BFD_RELOC_NONE:
      return R_SPU_NONE;
    case BFD_RELOC_SPU_IMM10W:
      return R_SPU_ADDR10;
    case BFD_RELOC_SPU_IMM16W:
      return R_SPU_ADDR16;
    case BFD_RELOC_SPU_LO16:
      return R_SPU_ADDR16_LO;
    case BFD_RELOC_SPU_HI16:
      return R_SPU_ADDR16_HI;
    case BFD_RELOC_SPU_IMM18:
      return R_SPU_ADDR18;
    case BFD_RELOC_SPU_PCREL16:
      return R_SPU_REL16;
    case BFD_RELOC_SPU_IMM7:
      return R_SPU_ADDR7;
    case BFD_RELOC_SPU_IMM8:
      return R_SPU_NONE;
    case BFD_RELOC_SPU_PCREL9a:
      return R_SPU_REL9;
    case BFD_RELOC_SPU_PCREL9b:
      return R_SPU_REL9I;
    case BFD_RELOC_SPU_IMM10:
      return R_SPU_ADDR10I;
    case BFD_RELOC_SPU_IMM16:
      return R_SPU_ADDR16I;
    case BFD_RELOC_32:
      return R_SPU_ADDR32;
    case BFD_RELOC_32_PCREL:
      return R_SPU_REL32;
    case BFD_RELOC_SPU_PPU32:
      return R_SPU_PPU32;
    case BFD_RELOC_SPU_PPU64:
      return R_SPU_PPU64;
    case BFD_RELOC_SPU_ADD_PIC:
      return R_SPU_ADD_PIC;
    }
}

static bool
spu_elf_info_to_howto (bfd *abfd,
		       arelent *cache_ptr,
		       Elf_Internal_Rela *dst)
{
  enum elf_spu_reloc_type r_type;

  r_type = (enum elf_spu_reloc_type) ELF32_R_TYPE (dst->r_info);
  /* PR 17512: file: 90c2a92e.  */
  if (r_type >= R_SPU_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = &elf_howto_table[(int) r_type];
  return true;
}

static reloc_howto_type *
spu_elf_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			   bfd_reloc_code_real_type code)
{
  enum elf_spu_reloc_type r_type = spu_elf_bfd_to_reloc_type (code);

  if (r_type == (enum elf_spu_reloc_type) -1)
    return NULL;

  return elf_howto_table + r_type;
}

static reloc_howto_type *
spu_elf_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			   const char *r_name)
{
  unsigned int i;

  for (i = 0; i < sizeof (elf_howto_table) / sizeof (elf_howto_table[0]); i++)
    if (elf_howto_table[i].name != NULL
	&& strcasecmp (elf_howto_table[i].name, r_name) == 0)
      return &elf_howto_table[i];

  return NULL;
}

/* Apply R_SPU_REL9 and R_SPU_REL9I relocs.  */

static bfd_reloc_status_type
spu_elf_rel9 (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
	      void *data, asection *input_section,
	      bfd *output_bfd, char **error_message)
{
  bfd_size_type octets;
  bfd_vma val;
  long insn;

  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;
  octets = reloc_entry->address * OCTETS_PER_BYTE (abfd, input_section);

  /* Get symbol value.  */
  val = 0;
  if (!bfd_is_com_section (symbol->section))
    val = symbol->value;
  if (symbol->section->output_section)
    val += symbol->section->output_section->vma;

  val += reloc_entry->addend;

  /* Make it pc-relative.  */
  val -= input_section->output_section->vma + input_section->output_offset;

  val >>= 2;
  if (val + 256 >= 512)
    return bfd_reloc_overflow;

  insn = bfd_get_32 (abfd, (bfd_byte *) data + octets);

  /* Move two high bits of value to REL9I and REL9 position.
     The mask will take care of selecting the right field.  */
  val = (val & 0x7f) | ((val & 0x180) << 7) | ((val & 0x180) << 16);
  insn &= ~reloc_entry->howto->dst_mask;
  insn |= val & reloc_entry->howto->dst_mask;
  bfd_put_32 (abfd, insn, (bfd_byte *) data + octets);
  return bfd_reloc_ok;
}

static bool
spu_elf_new_section_hook (bfd *abfd, asection *sec)
{
  if (!sec->used_by_bfd)
    {
      struct _spu_elf_section_data *sdata;

      sdata = bfd_zalloc (abfd, sizeof (*sdata));
      if (sdata == NULL)
	return false;
      sec->used_by_bfd = sdata;
    }

  return _bfd_elf_new_section_hook (abfd, sec);
}

/* Set up overlay info for executables.  */

static bool
spu_elf_object_p (bfd *abfd)
{
  if ((abfd->flags & (EXEC_P | DYNAMIC)) != 0)
    {
      unsigned int i, num_ovl, num_buf;
      Elf_Internal_Phdr *phdr = elf_tdata (abfd)->phdr;
      Elf_Internal_Ehdr *ehdr = elf_elfheader (abfd);
      Elf_Internal_Phdr *last_phdr = NULL;

      for (num_buf = 0, num_ovl = 0, i = 0; i < ehdr->e_phnum; i++, phdr++)
	if (phdr->p_type == PT_LOAD && (phdr->p_flags & PF_OVERLAY) != 0)
	  {
	    unsigned int j;

	    ++num_ovl;
	    if (last_phdr == NULL
		|| ((last_phdr->p_vaddr ^ phdr->p_vaddr) & 0x3ffff) != 0)
	      ++num_buf;
	    last_phdr = phdr;
	    for (j = 1; j < elf_numsections (abfd); j++)
	      {
		Elf_Internal_Shdr *shdr = elf_elfsections (abfd)[j];

		if (shdr->bfd_section != NULL
		    && ELF_SECTION_SIZE (shdr, phdr) != 0
		    && ELF_SECTION_IN_SEGMENT (shdr, phdr))
		  {
		    asection *sec = shdr->bfd_section;
		    spu_elf_section_data (sec)->u.o.ovl_index = num_ovl;
		    spu_elf_section_data (sec)->u.o.ovl_buf = num_buf;
		  }
	      }
	  }
    }
  return true;
}

/* Specially mark defined symbols named _EAR_* with BSF_KEEP so that
   strip --strip-unneeded will not remove them.  */

static void
spu_elf_backend_symbol_processing (bfd *abfd ATTRIBUTE_UNUSED, asymbol *sym)
{
  if (sym->name != NULL
      && sym->section != bfd_abs_section_ptr
      && startswith (sym->name, "_EAR_"))
    sym->flags |= BSF_KEEP;
}

/* SPU ELF linker hash table.  */

struct spu_link_hash_table
{
  struct elf_link_hash_table elf;

  struct spu_elf_params *params;

  /* Shortcuts to overlay sections.  */
  asection *ovtab;
  asection *init;
  asection *toe;
  asection **ovl_sec;

  /* Count of stubs in each overlay section.  */
  unsigned int *stub_count;

  /* The stub section for each overlay section.  */
  asection **stub_sec;

  struct elf_link_hash_entry *ovly_entry[2];

  /* Number of overlay buffers.  */
  unsigned int num_buf;

  /* Total number of overlays.  */
  unsigned int num_overlays;

  /* For soft icache.  */
  unsigned int line_size_log2;
  unsigned int num_lines_log2;
  unsigned int fromelem_size_log2;

  /* How much memory we have.  */
  unsigned int local_store;

  /* Count of overlay stubs needed in non-overlay area.  */
  unsigned int non_ovly_stub;

  /* Pointer to the fixup section */
  asection *sfixup;

  /* Set on error.  */
  unsigned int stub_err : 1;
};

/* Hijack the generic got fields for overlay stub accounting.  */

struct got_entry
{
  struct got_entry *next;
  unsigned int ovl;
  union {
    bfd_vma addend;
    bfd_vma br_addr;
  };
  bfd_vma stub_addr;
};

#define spu_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == SPU_ELF_DATA)		\
   ? (struct spu_link_hash_table *) (p)->hash : NULL)

struct call_info
{
  struct function_info *fun;
  struct call_info *next;
  unsigned int count;
  unsigned int max_depth;
  unsigned int is_tail : 1;
  unsigned int is_pasted : 1;
  unsigned int broken_cycle : 1;
  unsigned int priority : 13;
};

struct function_info
{
  /* List of functions called.  Also branches to hot/cold part of
     function.  */
  struct call_info *call_list;
  /* For hot/cold part of function, point to owner.  */
  struct function_info *start;
  /* Symbol at start of function.  */
  union {
    Elf_Internal_Sym *sym;
    struct elf_link_hash_entry *h;
  } u;
  /* Function section.  */
  asection *sec;
  asection *rodata;
  /* Where last called from, and number of sections called from.  */
  asection *last_caller;
  unsigned int call_count;
  /* Address range of (this part of) function.  */
  bfd_vma lo, hi;
  /* Offset where we found a store of lr, or -1 if none found.  */
  bfd_vma lr_store;
  /* Offset where we found the stack adjustment insn.  */
  bfd_vma sp_adjust;
  /* Stack usage.  */
  int stack;
  /* Distance from root of call tree.  Tail and hot/cold branches
     count as one deeper.  We aren't counting stack frames here.  */
  unsigned int depth;
  /* Set if global symbol.  */
  unsigned int global : 1;
  /* Set if known to be start of function (as distinct from a hunk
     in hot/cold section.  */
  unsigned int is_func : 1;
  /* Set if not a root node.  */
  unsigned int non_root : 1;
  /* Flags used during call tree traversal.  It's cheaper to replicate
     the visit flags than have one which needs clearing after a traversal.  */
  unsigned int visit1 : 1;
  unsigned int visit2 : 1;
  unsigned int marking : 1;
  unsigned int visit3 : 1;
  unsigned int visit4 : 1;
  unsigned int visit5 : 1;
  unsigned int visit6 : 1;
  unsigned int visit7 : 1;
};

struct spu_elf_stack_info
{
  int num_fun;
  int max_fun;
  /* Variable size array describing functions, one per contiguous
     address range belonging to a function.  */
  struct function_info fun[1];
};

static struct function_info *find_function (asection *, bfd_vma,
					    struct bfd_link_info *);

/* Create a spu ELF linker hash table.  */

static struct bfd_link_hash_table *
spu_elf_link_hash_table_create (bfd *abfd)
{
  struct spu_link_hash_table *htab;

  htab = bfd_zmalloc (sizeof (*htab));
  if (htab == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&htab->elf, abfd,
				      _bfd_elf_link_hash_newfunc,
				      sizeof (struct elf_link_hash_entry),
				      SPU_ELF_DATA))
    {
      free (htab);
      return NULL;
    }

  htab->elf.init_got_refcount.refcount = 0;
  htab->elf.init_got_refcount.glist = NULL;
  htab->elf.init_got_offset.offset = 0;
  htab->elf.init_got_offset.glist = NULL;
  return &htab->elf.root;
}

void
spu_elf_setup (struct bfd_link_info *info, struct spu_elf_params *params)
{
  bfd_vma max_branch_log2;

  struct spu_link_hash_table *htab = spu_hash_table (info);
  htab->params = params;
  htab->line_size_log2 = bfd_log2 (htab->params->line_size);
  htab->num_lines_log2 = bfd_log2 (htab->params->num_lines);

  /* For the software i-cache, we provide a "from" list whose size
     is a power-of-two number of quadwords, big enough to hold one
     byte per outgoing branch.  Compute this number here.  */
  max_branch_log2 = bfd_log2 (htab->params->max_branch);
  htab->fromelem_size_log2 = max_branch_log2 > 4 ? max_branch_log2 - 4 : 0;
}

/* Find the symbol for the given R_SYMNDX in IBFD and set *HP and *SYMP
   to (hash, NULL) for global symbols, and (NULL, sym) for locals.  Set
   *SYMSECP to the symbol's section.  *LOCSYMSP caches local syms.  */

static bool
get_sym_h (struct elf_link_hash_entry **hp,
	   Elf_Internal_Sym **symp,
	   asection **symsecp,
	   Elf_Internal_Sym **locsymsp,
	   unsigned long r_symndx,
	   bfd *ibfd)
{
  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;

  if (r_symndx >= symtab_hdr->sh_info)
    {
      struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (ibfd);
      struct elf_link_hash_entry *h;

      h = sym_hashes[r_symndx - symtab_hdr->sh_info];
      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      if (hp != NULL)
	*hp = h;

      if (symp != NULL)
	*symp = NULL;

      if (symsecp != NULL)
	{
	  asection *symsec = NULL;
	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    symsec = h->root.u.def.section;
	  *symsecp = symsec;
	}
    }
  else
    {
      Elf_Internal_Sym *sym;
      Elf_Internal_Sym *locsyms = *locsymsp;

      if (locsyms == NULL)
	{
	  locsyms = (Elf_Internal_Sym *) symtab_hdr->contents;
	  if (locsyms == NULL)
	    locsyms = bfd_elf_get_elf_syms (ibfd, symtab_hdr,
					    symtab_hdr->sh_info,
					    0, NULL, NULL, NULL);
	  if (locsyms == NULL)
	    return false;
	  *locsymsp = locsyms;
	}
      sym = locsyms + r_symndx;

      if (hp != NULL)
	*hp = NULL;

      if (symp != NULL)
	*symp = sym;

      if (symsecp != NULL)
	*symsecp = bfd_section_from_elf_index (ibfd, sym->st_shndx);
    }

  return true;
}

/* Create the note section if not already present.  This is done early so
   that the linker maps the sections to the right place in the output.  */

bool
spu_elf_create_sections (struct bfd_link_info *info)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);
  bfd *ibfd;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    if (bfd_get_section_by_name (ibfd, SPU_PTNOTE_SPUNAME) != NULL)
      break;

  if (ibfd == NULL)
    {
      /* Make SPU_PTNOTE_SPUNAME section.  */
      asection *s;
      size_t name_len;
      size_t size;
      bfd_byte *data;
      flagword flags;

      ibfd = info->input_bfds;
      /* This should really be SEC_LINKER_CREATED, but then we'd need
	 to write out the section ourselves.  */
      flags = SEC_LOAD | SEC_READONLY | SEC_HAS_CONTENTS | SEC_IN_MEMORY;
      s = bfd_make_section_anyway_with_flags (ibfd, SPU_PTNOTE_SPUNAME, flags);
      if (s == NULL
	  || !bfd_set_section_alignment (s, 4))
	return false;
      /* Because we didn't set SEC_LINKER_CREATED we need to set the
	 proper section type.  */
      elf_section_type (s) = SHT_NOTE;

      name_len = strlen (bfd_get_filename (info->output_bfd)) + 1;
      size = 12 + ((sizeof (SPU_PLUGIN_NAME) + 3) & -4);
      size += (name_len + 3) & -4;

      if (!bfd_set_section_size (s, size))
	return false;

      data = bfd_zalloc (ibfd, size);
      if (data == NULL)
	return false;

      bfd_put_32 (ibfd, sizeof (SPU_PLUGIN_NAME), data + 0);
      bfd_put_32 (ibfd, name_len, data + 4);
      bfd_put_32 (ibfd, 1, data + 8);
      memcpy (data + 12, SPU_PLUGIN_NAME, sizeof (SPU_PLUGIN_NAME));
      memcpy (data + 12 + ((sizeof (SPU_PLUGIN_NAME) + 3) & -4),
	      bfd_get_filename (info->output_bfd), name_len);
      s->contents = data;
    }

  if (htab->params->emit_fixups)
    {
      asection *s;
      flagword flags;

      if (htab->elf.dynobj == NULL)
	htab->elf.dynobj = ibfd;
      ibfd = htab->elf.dynobj;
      flags = (SEC_LOAD | SEC_ALLOC | SEC_READONLY | SEC_HAS_CONTENTS
	       | SEC_IN_MEMORY | SEC_LINKER_CREATED);
      s = bfd_make_section_anyway_with_flags (ibfd, ".fixup", flags);
      if (s == NULL || !bfd_set_section_alignment (s, 2))
	return false;
      htab->sfixup = s;
    }

  return true;
}

/* qsort predicate to sort sections by vma.  */

static int
sort_sections (const void *a, const void *b)
{
  const asection *const *s1 = a;
  const asection *const *s2 = b;
  bfd_signed_vma delta = (*s1)->vma - (*s2)->vma;

  if (delta != 0)
    return delta < 0 ? -1 : 1;

  return (*s1)->index - (*s2)->index;
}

/* Identify overlays in the output bfd, and number them.
   Returns 0 on error, 1 if no overlays, 2 if overlays.  */

int
spu_elf_find_overlays (struct bfd_link_info *info)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);
  asection **alloc_sec;
  unsigned int i, n, ovl_index, num_buf;
  asection *s;
  bfd_vma ovl_end;
  static const char *const entry_names[2][2] = {
    { "__ovly_load", "__icache_br_handler" },
    { "__ovly_return", "__icache_call_handler" }
  };

  if (info->output_bfd->section_count < 2)
    return 1;

  alloc_sec
    = bfd_malloc (info->output_bfd->section_count * sizeof (*alloc_sec));
  if (alloc_sec == NULL)
    return 0;

  /* Pick out all the alloced sections.  */
  for (n = 0, s = info->output_bfd->sections; s != NULL; s = s->next)
    if ((s->flags & SEC_ALLOC) != 0
	&& (s->flags & (SEC_LOAD | SEC_THREAD_LOCAL)) != SEC_THREAD_LOCAL
	&& s->size != 0)
      alloc_sec[n++] = s;

  if (n == 0)
    {
      free (alloc_sec);
      return 1;
    }

  /* Sort them by vma.  */
  qsort (alloc_sec, n, sizeof (*alloc_sec), sort_sections);

  ovl_end = alloc_sec[0]->vma + alloc_sec[0]->size;
  if (htab->params->ovly_flavour == ovly_soft_icache)
    {
      unsigned int prev_buf = 0, set_id = 0;

      /* Look for an overlapping vma to find the first overlay section.  */
      bfd_vma vma_start = 0;

      for (i = 1; i < n; i++)
	{
	  s = alloc_sec[i];
	  if (s->vma < ovl_end)
	    {
	      asection *s0 = alloc_sec[i - 1];
	      vma_start = s0->vma;
	      ovl_end = (s0->vma
			 + ((bfd_vma) 1
			    << (htab->num_lines_log2 + htab->line_size_log2)));
	      --i;
	      break;
	    }
	  else
	    ovl_end = s->vma + s->size;
	}

      /* Now find any sections within the cache area.  */
      for (ovl_index = 0, num_buf = 0; i < n; i++)
	{
	  s = alloc_sec[i];
	  if (s->vma >= ovl_end)
	    break;

	  /* A section in an overlay area called .ovl.init is not
	     an overlay, in the sense that it might be loaded in
	     by the overlay manager, but rather the initial
	     section contents for the overlay buffer.  */
	  if (!startswith (s->name, ".ovl.init"))
	    {
	      num_buf = ((s->vma - vma_start) >> htab->line_size_log2) + 1;
	      set_id = (num_buf == prev_buf)? set_id + 1 : 0;
	      prev_buf = num_buf;

	      if ((s->vma - vma_start) & (htab->params->line_size - 1))
		{
		  info->callbacks->einfo (_("%X%P: overlay section %pA "
					    "does not start on a cache line\n"),
					  s);
		  bfd_set_error (bfd_error_bad_value);
		  return 0;
		}
	      else if (s->size > htab->params->line_size)
		{
		  info->callbacks->einfo (_("%X%P: overlay section %pA "
					    "is larger than a cache line\n"),
					  s);
		  bfd_set_error (bfd_error_bad_value);
		  return 0;
		}

	      alloc_sec[ovl_index++] = s;
	      spu_elf_section_data (s)->u.o.ovl_index
		= (set_id << htab->num_lines_log2) + num_buf;
	      spu_elf_section_data (s)->u.o.ovl_buf = num_buf;
	    }
	}

      /* Ensure there are no more overlay sections.  */
      for ( ; i < n; i++)
	{
	  s = alloc_sec[i];
	  if (s->vma < ovl_end)
	    {
	      info->callbacks->einfo (_("%X%P: overlay section %pA "
					"is not in cache area\n"),
				      alloc_sec[i-1]);
	      bfd_set_error (bfd_error_bad_value);
	      return 0;
	    }
	  else
	    ovl_end = s->vma + s->size;
	}
    }
  else
    {
      /* Look for overlapping vmas.  Any with overlap must be overlays.
	 Count them.  Also count the number of overlay regions.  */
      for (ovl_index = 0, num_buf = 0, i = 1; i < n; i++)
	{
	  s = alloc_sec[i];
	  if (s->vma < ovl_end)
	    {
	      asection *s0 = alloc_sec[i - 1];

	      if (spu_elf_section_data (s0)->u.o.ovl_index == 0)
		{
		  ++num_buf;
		  if (!startswith (s0->name, ".ovl.init"))
		    {
		      alloc_sec[ovl_index] = s0;
		      spu_elf_section_data (s0)->u.o.ovl_index = ++ovl_index;
		      spu_elf_section_data (s0)->u.o.ovl_buf = num_buf;
		    }
		  else
		    ovl_end = s->vma + s->size;
		}
	      if (!startswith (s->name, ".ovl.init"))
		{
		  alloc_sec[ovl_index] = s;
		  spu_elf_section_data (s)->u.o.ovl_index = ++ovl_index;
		  spu_elf_section_data (s)->u.o.ovl_buf = num_buf;
		  if (s0->vma != s->vma)
		    {
		      /* xgettext:c-format */
		      info->callbacks->einfo (_("%X%P: overlay sections %pA "
						"and %pA do not start at the "
						"same address\n"),
					      s0, s);
		      bfd_set_error (bfd_error_bad_value);
		      return 0;
		    }
		  if (ovl_end < s->vma + s->size)
		    ovl_end = s->vma + s->size;
		}
	    }
	  else
	    ovl_end = s->vma + s->size;
	}
    }

  htab->num_overlays = ovl_index;
  htab->num_buf = num_buf;
  htab->ovl_sec = alloc_sec;

  if (ovl_index == 0)
    return 1;

  for (i = 0; i < 2; i++)
    {
      const char *name;
      struct elf_link_hash_entry *h;

      name = entry_names[i][htab->params->ovly_flavour];
      h = elf_link_hash_lookup (&htab->elf, name, true, false, false);
      if (h == NULL)
	return 0;

      if (h->root.type == bfd_link_hash_new)
	{
	  h->root.type = bfd_link_hash_undefined;
	  h->ref_regular = 1;
	  h->ref_regular_nonweak = 1;
	  h->non_elf = 0;
	}
      htab->ovly_entry[i] = h;
    }

  return 2;
}

/* Non-zero to use bra in overlay stubs rather than br.  */
#define BRA_STUBS 0

#define BRA	0x30000000
#define BRASL	0x31000000
#define BR	0x32000000
#define BRSL	0x33000000
#define NOP	0x40200000
#define LNOP	0x00200000
#define ILA	0x42000000

/* Return true for all relative and absolute branch instructions.
   bra   00110000 0..
   brasl 00110001 0..
   br    00110010 0..
   brsl  00110011 0..
   brz   00100000 0..
   brnz  00100001 0..
   brhz  00100010 0..
   brhnz 00100011 0..  */

static bool
is_branch (const unsigned char *insn)
{
  return (insn[0] & 0xec) == 0x20 && (insn[1] & 0x80) == 0;
}

/* Return true for all indirect branch instructions.
   bi     00110101 000
   bisl   00110101 001
   iret   00110101 010
   bisled 00110101 011
   biz    00100101 000
   binz   00100101 001
   bihz   00100101 010
   bihnz  00100101 011  */

static bool
is_indirect_branch (const unsigned char *insn)
{
  return (insn[0] & 0xef) == 0x25 && (insn[1] & 0x80) == 0;
}

/* Return true for branch hint instructions.
   hbra  0001000..
   hbrr  0001001..  */

static bool
is_hint (const unsigned char *insn)
{
  return (insn[0] & 0xfc) == 0x10;
}

/* True if INPUT_SECTION might need overlay stubs.  */

static bool
maybe_needs_stubs (asection *input_section)
{
  /* No stubs for debug sections and suchlike.  */
  if ((input_section->flags & SEC_ALLOC) == 0)
    return false;

  /* No stubs for link-once sections that will be discarded.  */
  if (input_section->output_section == bfd_abs_section_ptr)
    return false;

  /* Don't create stubs for .eh_frame references.  */
  if (strcmp (input_section->name, ".eh_frame") == 0)
    return false;

  return true;
}

enum _stub_type
{
  no_stub,
  call_ovl_stub,
  br000_ovl_stub,
  br001_ovl_stub,
  br010_ovl_stub,
  br011_ovl_stub,
  br100_ovl_stub,
  br101_ovl_stub,
  br110_ovl_stub,
  br111_ovl_stub,
  nonovl_stub,
  stub_error
};

/* Return non-zero if this reloc symbol should go via an overlay stub.
   Return 2 if the stub must be in non-overlay area.  */

static enum _stub_type
needs_ovl_stub (struct elf_link_hash_entry *h,
		Elf_Internal_Sym *sym,
		asection *sym_sec,
		asection *input_section,
		Elf_Internal_Rela *irela,
		bfd_byte *contents,
		struct bfd_link_info *info)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);
  enum elf_spu_reloc_type r_type;
  unsigned int sym_type;
  bool branch, hint, call;
  enum _stub_type ret = no_stub;
  bfd_byte insn[4];

  if (sym_sec == NULL
      || sym_sec->output_section == bfd_abs_section_ptr
      || spu_elf_section_data (sym_sec->output_section) == NULL)
    return ret;

  if (h != NULL)
    {
      /* Ensure no stubs for user supplied overlay manager syms.  */
      if (h == htab->ovly_entry[0] || h == htab->ovly_entry[1])
	return ret;

      /* setjmp always goes via an overlay stub, because then the return
	 and hence the longjmp goes via __ovly_return.  That magically
	 makes setjmp/longjmp between overlays work.  */
      if (startswith (h->root.root.string, "setjmp")
	  && (h->root.root.string[6] == '\0' || h->root.root.string[6] == '@'))
	ret = call_ovl_stub;
    }

  if (h != NULL)
    sym_type = h->type;
  else
    sym_type = ELF_ST_TYPE (sym->st_info);

  r_type = ELF32_R_TYPE (irela->r_info);
  branch = false;
  hint = false;
  call = false;
  if (r_type == R_SPU_REL16 || r_type == R_SPU_ADDR16)
    {
      if (contents == NULL)
	{
	  contents = insn;
	  if (!bfd_get_section_contents (input_section->owner,
					 input_section,
					 contents,
					 irela->r_offset, 4))
	    return stub_error;
	}
      else
	contents += irela->r_offset;

      branch = is_branch (contents);
      hint = is_hint (contents);
      if (branch || hint)
	{
	  call = (contents[0] & 0xfd) == 0x31;
	  if (call
	      && sym_type != STT_FUNC
	      && contents != insn)
	    {
	      /* It's common for people to write assembly and forget
		 to give function symbols the right type.  Handle
		 calls to such symbols, but warn so that (hopefully)
		 people will fix their code.  We need the symbol
		 type to be correct to distinguish function pointer
		 initialisation from other pointer initialisations.  */
	      const char *sym_name;

	      if (h != NULL)
		sym_name = h->root.root.string;
	      else
		{
		  Elf_Internal_Shdr *symtab_hdr;
		  symtab_hdr = &elf_tdata (input_section->owner)->symtab_hdr;
		  sym_name = bfd_elf_sym_name (input_section->owner,
					       symtab_hdr,
					       sym,
					       sym_sec);
		}
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("warning: call to non-function symbol %s defined in %pB"),
		 sym_name, sym_sec->owner);

	    }
	}
    }

  if ((!branch && htab->params->ovly_flavour == ovly_soft_icache)
      || (sym_type != STT_FUNC
	  && !(branch || hint)
	  && (sym_sec->flags & SEC_CODE) == 0))
    return no_stub;

  /* Usually, symbols in non-overlay sections don't need stubs.  */
  if (spu_elf_section_data (sym_sec->output_section)->u.o.ovl_index == 0
      && !htab->params->non_overlay_stubs)
    return ret;

  /* A reference from some other section to a symbol in an overlay
     section needs a stub.  */
  if (spu_elf_section_data (sym_sec->output_section)->u.o.ovl_index
       != spu_elf_section_data (input_section->output_section)->u.o.ovl_index)
    {
      unsigned int lrlive = 0;
      if (branch)
	lrlive = (contents[1] & 0x70) >> 4;

      if (!lrlive && (call || sym_type == STT_FUNC))
	ret = call_ovl_stub;
      else
	ret = br000_ovl_stub + lrlive;
    }

  /* If this insn isn't a branch then we are possibly taking the
     address of a function and passing it out somehow.  Soft-icache code
     always generates inline code to do indirect branches.  */
  if (!(branch || hint)
      && sym_type == STT_FUNC
      && htab->params->ovly_flavour != ovly_soft_icache)
    ret = nonovl_stub;

  return ret;
}

static bool
count_stub (struct spu_link_hash_table *htab,
	    bfd *ibfd,
	    asection *isec,
	    enum _stub_type stub_type,
	    struct elf_link_hash_entry *h,
	    const Elf_Internal_Rela *irela)
{
  unsigned int ovl = 0;
  struct got_entry *g, **head;
  bfd_vma addend;

  /* If this instruction is a branch or call, we need a stub
     for it.  One stub per function per overlay.
     If it isn't a branch, then we are taking the address of
     this function so need a stub in the non-overlay area
     for it.  One stub per function.  */
  if (stub_type != nonovl_stub)
    ovl = spu_elf_section_data (isec->output_section)->u.o.ovl_index;

  if (h != NULL)
    head = &h->got.glist;
  else
    {
      if (elf_local_got_ents (ibfd) == NULL)
	{
	  bfd_size_type amt = (elf_tdata (ibfd)->symtab_hdr.sh_info
			       * sizeof (*elf_local_got_ents (ibfd)));
	  elf_local_got_ents (ibfd) = bfd_zmalloc (amt);
	  if (elf_local_got_ents (ibfd) == NULL)
	    return false;
	}
      head = elf_local_got_ents (ibfd) + ELF32_R_SYM (irela->r_info);
    }

  if (htab->params->ovly_flavour == ovly_soft_icache)
    {
      htab->stub_count[ovl] += 1;
      return true;
    }

  addend = 0;
  if (irela != NULL)
    addend = irela->r_addend;

  if (ovl == 0)
    {
      struct got_entry *gnext;

      for (g = *head; g != NULL; g = g->next)
	if (g->addend == addend && g->ovl == 0)
	  break;

      if (g == NULL)
	{
	  /* Need a new non-overlay area stub.  Zap other stubs.  */
	  for (g = *head; g != NULL; g = gnext)
	    {
	      gnext = g->next;
	      if (g->addend == addend)
		{
		  htab->stub_count[g->ovl] -= 1;
		  free (g);
		}
	    }
	}
    }
  else
    {
      for (g = *head; g != NULL; g = g->next)
	if (g->addend == addend && (g->ovl == ovl || g->ovl == 0))
	  break;
    }

  if (g == NULL)
    {
      g = bfd_malloc (sizeof *g);
      if (g == NULL)
	return false;
      g->ovl = ovl;
      g->addend = addend;
      g->stub_addr = (bfd_vma) -1;
      g->next = *head;
      *head = g;

      htab->stub_count[ovl] += 1;
    }

  return true;
}

/* Support two sizes of overlay stubs, a slower more compact stub of two
   instructions, and a faster stub of four instructions.
   Soft-icache stubs are four or eight words.  */

static unsigned int
ovl_stub_size (struct spu_elf_params *params)
{
  return 16 << params->ovly_flavour >> params->compact_stub;
}

static unsigned int
ovl_stub_size_log2 (struct spu_elf_params *params)
{
  return 4 + params->ovly_flavour - params->compact_stub;
}

/* Two instruction overlay stubs look like:

   brsl $75,__ovly_load
   .word target_ovl_and_address

   ovl_and_address is a word with the overlay number in the top 14 bits
   and local store address in the bottom 18 bits.

   Four instruction overlay stubs look like:

   ila $78,ovl_number
   lnop
   ila $79,target_address
   br __ovly_load

   Software icache stubs are:

   .word target_index
   .word target_ia;
   .word lrlive_branchlocalstoreaddr;
   brasl $75,__icache_br_handler
   .quad xor_pattern
*/

static bool
build_stub (struct bfd_link_info *info,
	    bfd *ibfd,
	    asection *isec,
	    enum _stub_type stub_type,
	    struct elf_link_hash_entry *h,
	    const Elf_Internal_Rela *irela,
	    bfd_vma dest,
	    asection *dest_sec)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);
  unsigned int ovl, dest_ovl, set_id;
  struct got_entry *g, **head;
  asection *sec;
  bfd_vma addend, from, to, br_dest, patt;
  unsigned int lrlive;

  ovl = 0;
  if (stub_type != nonovl_stub)
    ovl = spu_elf_section_data (isec->output_section)->u.o.ovl_index;

  if (h != NULL)
    head = &h->got.glist;
  else
    head = elf_local_got_ents (ibfd) + ELF32_R_SYM (irela->r_info);

  addend = 0;
  if (irela != NULL)
    addend = irela->r_addend;

  if (htab->params->ovly_flavour == ovly_soft_icache)
    {
      g = bfd_malloc (sizeof *g);
      if (g == NULL)
	return false;
      g->ovl = ovl;
      g->br_addr = 0;
      if (irela != NULL)
	g->br_addr = (irela->r_offset
		      + isec->output_offset
		      + isec->output_section->vma);
      g->next = *head;
      *head = g;
    }
  else
    {
      for (g = *head; g != NULL; g = g->next)
	if (g->addend == addend && (g->ovl == ovl || g->ovl == 0))
	  break;
      if (g == NULL)
	abort ();

      if (g->ovl == 0 && ovl != 0)
	return true;

      if (g->stub_addr != (bfd_vma) -1)
	return true;
    }

  sec = htab->stub_sec[ovl];
  dest += dest_sec->output_offset + dest_sec->output_section->vma;
  from = sec->size + sec->output_offset + sec->output_section->vma;
  g->stub_addr = from;
  to = (htab->ovly_entry[0]->root.u.def.value
	+ htab->ovly_entry[0]->root.u.def.section->output_offset
	+ htab->ovly_entry[0]->root.u.def.section->output_section->vma);

  if (((dest | to | from) & 3) != 0)
    {
      htab->stub_err = 1;
      return false;
    }
  dest_ovl = spu_elf_section_data (dest_sec->output_section)->u.o.ovl_index;

  if (htab->params->ovly_flavour == ovly_normal
      && !htab->params->compact_stub)
    {
      bfd_put_32 (sec->owner, ILA + ((dest_ovl << 7) & 0x01ffff80) + 78,
		  sec->contents + sec->size);
      bfd_put_32 (sec->owner, LNOP,
		  sec->contents + sec->size + 4);
      bfd_put_32 (sec->owner, ILA + ((dest << 7) & 0x01ffff80) + 79,
		  sec->contents + sec->size + 8);
      if (!BRA_STUBS)
	bfd_put_32 (sec->owner, BR + (((to - (from + 12)) << 5) & 0x007fff80),
		    sec->contents + sec->size + 12);
      else
	bfd_put_32 (sec->owner, BRA + ((to << 5) & 0x007fff80),
		    sec->contents + sec->size + 12);
    }
  else if (htab->params->ovly_flavour == ovly_normal
	   && htab->params->compact_stub)
    {
      if (!BRA_STUBS)
	bfd_put_32 (sec->owner, BRSL + (((to - from) << 5) & 0x007fff80) + 75,
		    sec->contents + sec->size);
      else
	bfd_put_32 (sec->owner, BRASL + ((to << 5) & 0x007fff80) + 75,
		    sec->contents + sec->size);
      bfd_put_32 (sec->owner, (dest & 0x3ffff) | (dest_ovl << 18),
		  sec->contents + sec->size + 4);
    }
  else if (htab->params->ovly_flavour == ovly_soft_icache
	   && htab->params->compact_stub)
    {
      lrlive = 0;
      if (stub_type == nonovl_stub)
	;
      else if (stub_type == call_ovl_stub)
	/* A brsl makes lr live and *(*sp+16) is live.
	   Tail calls have the same liveness.  */
	lrlive = 5;
      else if (!htab->params->lrlive_analysis)
	/* Assume stack frame and lr save.  */
	lrlive = 1;
      else if (irela != NULL)
	{
	  /* Analyse branch instructions.  */
	  struct function_info *caller;
	  bfd_vma off;

	  caller = find_function (isec, irela->r_offset, info);
	  if (caller->start == NULL)
	    off = irela->r_offset;
	  else
	    {
	      struct function_info *found = NULL;

	      /* Find the earliest piece of this function that
		 has frame adjusting instructions.  We might
		 see dynamic frame adjustment (eg. for alloca)
		 in some later piece, but functions using
		 alloca always set up a frame earlier.  Frame
		 setup instructions are always in one piece.  */
	      if (caller->lr_store != (bfd_vma) -1
		  || caller->sp_adjust != (bfd_vma) -1)
		found = caller;
	      while (caller->start != NULL)
		{
		  caller = caller->start;
		  if (caller->lr_store != (bfd_vma) -1
		      || caller->sp_adjust != (bfd_vma) -1)
		    found = caller;
		}
	      if (found != NULL)
		caller = found;
	      off = (bfd_vma) -1;
	    }

	  if (off > caller->sp_adjust)
	    {
	      if (off > caller->lr_store)
		/* Only *(*sp+16) is live.  */
		lrlive = 1;
	      else
		/* If no lr save, then we must be in a
		   leaf function with a frame.
		   lr is still live.  */
		lrlive = 4;
	    }
	  else if (off > caller->lr_store)
	    {
	      /* Between lr save and stack adjust.  */
	      lrlive = 3;
	      /* This should never happen since prologues won't
		 be split here.  */
	      BFD_ASSERT (0);
	    }
	  else
	    /* On entry to function.  */
	    lrlive = 5;

	  if (stub_type != br000_ovl_stub
	      && lrlive != stub_type - br000_ovl_stub)
	    /* xgettext:c-format */
	    info->callbacks->einfo (_("%pA:0x%v lrlive .brinfo (%u) differs "
				      "from analysis (%u)\n"),
				    isec, irela->r_offset, lrlive,
				    stub_type - br000_ovl_stub);
	}

      /* If given lrlive info via .brinfo, use it.  */
      if (stub_type > br000_ovl_stub)
	lrlive = stub_type - br000_ovl_stub;

      if (ovl == 0)
	to = (htab->ovly_entry[1]->root.u.def.value
	      + htab->ovly_entry[1]->root.u.def.section->output_offset
	      + htab->ovly_entry[1]->root.u.def.section->output_section->vma);

      /* The branch that uses this stub goes to stub_addr + 4.  We'll
	 set up an xor pattern that can be used by the icache manager
	 to modify this branch to go directly to its destination.  */
      g->stub_addr += 4;
      br_dest = g->stub_addr;
      if (irela == NULL)
	{
	  /* Except in the case of _SPUEAR_ stubs, the branch in
	     question is the one in the stub itself.  */
	  BFD_ASSERT (stub_type == nonovl_stub);
	  g->br_addr = g->stub_addr;
	  br_dest = to;
	}

      set_id = ((dest_ovl - 1) >> htab->num_lines_log2) + 1;
      bfd_put_32 (sec->owner, (set_id << 18) | (dest & 0x3ffff),
		  sec->contents + sec->size);
      bfd_put_32 (sec->owner, BRASL + ((to << 5) & 0x007fff80) + 75,
		  sec->contents + sec->size + 4);
      bfd_put_32 (sec->owner, (lrlive << 29) | (g->br_addr & 0x3ffff),
		  sec->contents + sec->size + 8);
      patt = dest ^ br_dest;
      if (irela != NULL && ELF32_R_TYPE (irela->r_info) == R_SPU_REL16)
	patt = (dest - g->br_addr) ^ (br_dest - g->br_addr);
      bfd_put_32 (sec->owner, (patt << 5) & 0x007fff80,
		  sec->contents + sec->size + 12);

      if (ovl == 0)
	/* Extra space for linked list entries.  */
	sec->size += 16;
    }
  else
    abort ();

  sec->size += ovl_stub_size (htab->params);

  if (htab->params->emit_stub_syms)
    {
      size_t len;
      char *name;
      int add;

      len = 8 + sizeof (".ovl_call.") - 1;
      if (h != NULL)
	len += strlen (h->root.root.string);
      else
	len += 8 + 1 + 8;
      add = 0;
      if (irela != NULL)
	add = (int) irela->r_addend & 0xffffffff;
      if (add != 0)
	len += 1 + 8;
      name = bfd_malloc (len + 1);
      if (name == NULL)
	return false;

      sprintf (name, "%08x.ovl_call.", g->ovl);
      if (h != NULL)
	strcpy (name + 8 + sizeof (".ovl_call.") - 1, h->root.root.string);
      else
	sprintf (name + 8 + sizeof (".ovl_call.") - 1, "%x:%x",
		 dest_sec->id & 0xffffffff,
		 (int) ELF32_R_SYM (irela->r_info) & 0xffffffff);
      if (add != 0)
	sprintf (name + len - 9, "+%x", add);

      h = elf_link_hash_lookup (&htab->elf, name, true, true, false);
      free (name);
      if (h == NULL)
	return false;
      if (h->root.type == bfd_link_hash_new)
	{
	  h->root.type = bfd_link_hash_defined;
	  h->root.u.def.section = sec;
	  h->size = ovl_stub_size (htab->params);
	  h->root.u.def.value = sec->size - h->size;
	  h->type = STT_FUNC;
	  h->ref_regular = 1;
	  h->def_regular = 1;
	  h->ref_regular_nonweak = 1;
	  h->forced_local = 1;
	  h->non_elf = 0;
	}
    }

  return true;
}

/* Called via elf_link_hash_traverse to allocate stubs for any _SPUEAR_
   symbols.  */

static bool
allocate_spuear_stubs (struct elf_link_hash_entry *h, void *inf)
{
  /* Symbols starting with _SPUEAR_ need a stub because they may be
     invoked by the PPU.  */
  struct bfd_link_info *info = inf;
  struct spu_link_hash_table *htab = spu_hash_table (info);
  asection *sym_sec;

  if ((h->root.type == bfd_link_hash_defined
       || h->root.type == bfd_link_hash_defweak)
      && h->def_regular
      && startswith (h->root.root.string, "_SPUEAR_")
      && (sym_sec = h->root.u.def.section) != NULL
      && sym_sec->output_section != bfd_abs_section_ptr
      && spu_elf_section_data (sym_sec->output_section) != NULL
      && (spu_elf_section_data (sym_sec->output_section)->u.o.ovl_index != 0
	  || htab->params->non_overlay_stubs))
    {
      return count_stub (htab, NULL, NULL, nonovl_stub, h, NULL);
    }

  return true;
}

static bool
build_spuear_stubs (struct elf_link_hash_entry *h, void *inf)
{
  /* Symbols starting with _SPUEAR_ need a stub because they may be
     invoked by the PPU.  */
  struct bfd_link_info *info = inf;
  struct spu_link_hash_table *htab = spu_hash_table (info);
  asection *sym_sec;

  if ((h->root.type == bfd_link_hash_defined
       || h->root.type == bfd_link_hash_defweak)
      && h->def_regular
      && startswith (h->root.root.string, "_SPUEAR_")
      && (sym_sec = h->root.u.def.section) != NULL
      && sym_sec->output_section != bfd_abs_section_ptr
      && spu_elf_section_data (sym_sec->output_section) != NULL
      && (spu_elf_section_data (sym_sec->output_section)->u.o.ovl_index != 0
	  || htab->params->non_overlay_stubs))
    {
      return build_stub (info, NULL, NULL, nonovl_stub, h, NULL,
			 h->root.u.def.value, sym_sec);
    }

  return true;
}

/* Size or build stubs.  */

static bool
process_stubs (struct bfd_link_info *info, bool build)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);
  bfd *ibfd;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      extern const bfd_target spu_elf32_vec;
      Elf_Internal_Shdr *symtab_hdr;
      asection *isec;
      Elf_Internal_Sym *local_syms = NULL;

      if (ibfd->xvec != &spu_elf32_vec)
	continue;

      /* We'll need the symbol table in a second.  */
      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      if (symtab_hdr->sh_info == 0)
	continue;

      /* Walk over each section attached to the input bfd.  */
      for (isec = ibfd->sections; isec != NULL; isec = isec->next)
	{
	  Elf_Internal_Rela *internal_relocs, *irelaend, *irela;

	  /* If there aren't any relocs, then there's nothing more to do.  */
	  if ((isec->flags & SEC_RELOC) == 0
	      || isec->reloc_count == 0)
	    continue;

	  if (!maybe_needs_stubs (isec))
	    continue;

	  /* Get the relocs.  */
	  internal_relocs = _bfd_elf_link_read_relocs (ibfd, isec, NULL, NULL,
						       info->keep_memory);
	  if (internal_relocs == NULL)
	    goto error_ret_free_local;

	  /* Now examine each relocation.  */
	  irela = internal_relocs;
	  irelaend = irela + isec->reloc_count;
	  for (; irela < irelaend; irela++)
	    {
	      enum elf_spu_reloc_type r_type;
	      unsigned int r_indx;
	      asection *sym_sec;
	      Elf_Internal_Sym *sym;
	      struct elf_link_hash_entry *h;
	      enum _stub_type stub_type;

	      r_type = ELF32_R_TYPE (irela->r_info);
	      r_indx = ELF32_R_SYM (irela->r_info);

	      if (r_type >= R_SPU_max)
		{
		  bfd_set_error (bfd_error_bad_value);
		error_ret_free_internal:
		  if (elf_section_data (isec)->relocs != internal_relocs)
		    free (internal_relocs);
		error_ret_free_local:
		  if (symtab_hdr->contents != (unsigned char *) local_syms)
		    free (local_syms);
		  return false;
		}

	      /* Determine the reloc target section.  */
	      if (!get_sym_h (&h, &sym, &sym_sec, &local_syms, r_indx, ibfd))
		goto error_ret_free_internal;

	      stub_type = needs_ovl_stub (h, sym, sym_sec, isec, irela,
					  NULL, info);
	      if (stub_type == no_stub)
		continue;
	      else if (stub_type == stub_error)
		goto error_ret_free_internal;

	      if (htab->stub_count == NULL)
		{
		  bfd_size_type amt;
		  amt = (htab->num_overlays + 1) * sizeof (*htab->stub_count);
		  htab->stub_count = bfd_zmalloc (amt);
		  if (htab->stub_count == NULL)
		    goto error_ret_free_internal;
		}

	      if (!build)
		{
		  if (!count_stub (htab, ibfd, isec, stub_type, h, irela))
		    goto error_ret_free_internal;
		}
	      else
		{
		  bfd_vma dest;

		  if (h != NULL)
		    dest = h->root.u.def.value;
		  else
		    dest = sym->st_value;
		  dest += irela->r_addend;
		  if (!build_stub (info, ibfd, isec, stub_type, h, irela,
				   dest, sym_sec))
		    goto error_ret_free_internal;
		}
	    }

	  /* We're done with the internal relocs, free them.  */
	  if (elf_section_data (isec)->relocs != internal_relocs)
	    free (internal_relocs);
	}

      if (local_syms != NULL
	  && symtab_hdr->contents != (unsigned char *) local_syms)
	{
	  if (!info->keep_memory)
	    free (local_syms);
	  else
	    symtab_hdr->contents = (unsigned char *) local_syms;
	}
    }

  return true;
}

/* Allocate space for overlay call and return stubs.
   Return 0 on error, 1 if no overlays, 2 otherwise.  */

int
spu_elf_size_stubs (struct bfd_link_info *info)
{
  struct spu_link_hash_table *htab;
  bfd *ibfd;
  bfd_size_type amt;
  flagword flags;
  unsigned int i;
  asection *stub;

  if (!process_stubs (info, false))
    return 0;

  htab = spu_hash_table (info);
  elf_link_hash_traverse (&htab->elf, allocate_spuear_stubs, info);
  if (htab->stub_err)
    return 0;

  ibfd = info->input_bfds;
  if (htab->stub_count != NULL)
    {
      amt = (htab->num_overlays + 1) * sizeof (*htab->stub_sec);
      htab->stub_sec = bfd_zmalloc (amt);
      if (htab->stub_sec == NULL)
	return 0;

      flags = (SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_READONLY
	       | SEC_HAS_CONTENTS | SEC_IN_MEMORY);
      stub = bfd_make_section_anyway_with_flags (ibfd, ".stub", flags);
      htab->stub_sec[0] = stub;
      if (stub == NULL
	  || !bfd_set_section_alignment (stub,
					 ovl_stub_size_log2 (htab->params)))
	return 0;
      stub->size = htab->stub_count[0] * ovl_stub_size (htab->params);
      if (htab->params->ovly_flavour == ovly_soft_icache)
	/* Extra space for linked list entries.  */
	stub->size += htab->stub_count[0] * 16;

      for (i = 0; i < htab->num_overlays; ++i)
	{
	  asection *osec = htab->ovl_sec[i];
	  unsigned int ovl = spu_elf_section_data (osec)->u.o.ovl_index;
	  stub = bfd_make_section_anyway_with_flags (ibfd, ".stub", flags);
	  htab->stub_sec[ovl] = stub;
	  if (stub == NULL
	      || !bfd_set_section_alignment (stub,
					     ovl_stub_size_log2 (htab->params)))
	    return 0;
	  stub->size = htab->stub_count[ovl] * ovl_stub_size (htab->params);
	}
    }

  if (htab->params->ovly_flavour == ovly_soft_icache)
    {
      /* Space for icache manager tables.
	 a) Tag array, one quadword per cache line.
	 b) Rewrite "to" list, one quadword per cache line.
	 c) Rewrite "from" list, one byte per outgoing branch (rounded up to
	    a power-of-two number of full quadwords) per cache line.  */

      flags = SEC_ALLOC;
      htab->ovtab = bfd_make_section_anyway_with_flags (ibfd, ".ovtab", flags);
      if (htab->ovtab == NULL
	  || !bfd_set_section_alignment (htab->ovtab, 4))
	return 0;

      htab->ovtab->size = (16 + 16 + (16 << htab->fromelem_size_log2))
			  << htab->num_lines_log2;

      flags = SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY;
      htab->init = bfd_make_section_anyway_with_flags (ibfd, ".ovini", flags);
      if (htab->init == NULL
	  || !bfd_set_section_alignment (htab->init, 4))
	return 0;

      htab->init->size = 16;
    }
  else if (htab->stub_count == NULL)
    return 1;
  else
    {
      /* htab->ovtab consists of two arrays.
	 .	struct {
	 .	  u32 vma;
	 .	  u32 size;
	 .	  u32 file_off;
	 .	  u32 buf;
	 .	} _ovly_table[];
	 .
	 .	struct {
	 .	  u32 mapped;
	 .	} _ovly_buf_table[];
	 .  */

      flags = SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY;
      htab->ovtab = bfd_make_section_anyway_with_flags (ibfd, ".ovtab", flags);
      if (htab->ovtab == NULL
	  || !bfd_set_section_alignment (htab->ovtab, 4))
	return 0;

      htab->ovtab->size = htab->num_overlays * 16 + 16 + htab->num_buf * 4;
    }

  htab->toe = bfd_make_section_anyway_with_flags (ibfd, ".toe", SEC_ALLOC);
  if (htab->toe == NULL
      || !bfd_set_section_alignment (htab->toe, 4))
    return 0;
  htab->toe->size = 16;

  return 2;
}

/* Called from ld to place overlay manager data sections.  This is done
   after the overlay manager itself is loaded, mainly so that the
   linker's htab->init section is placed after any other .ovl.init
   sections.  */

void
spu_elf_place_overlay_data (struct bfd_link_info *info)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);
  unsigned int i;

  if (htab->stub_sec != NULL)
    {
      (*htab->params->place_spu_section) (htab->stub_sec[0], NULL, ".text");

      for (i = 0; i < htab->num_overlays; ++i)
	{
	  asection *osec = htab->ovl_sec[i];
	  unsigned int ovl = spu_elf_section_data (osec)->u.o.ovl_index;
	  (*htab->params->place_spu_section) (htab->stub_sec[ovl], osec, NULL);
	}
    }

  if (htab->params->ovly_flavour == ovly_soft_icache)
    (*htab->params->place_spu_section) (htab->init, NULL, ".ovl.init");

  if (htab->ovtab != NULL)
    {
      const char *ovout = ".data";
      if (htab->params->ovly_flavour == ovly_soft_icache)
	ovout = ".bss";
      (*htab->params->place_spu_section) (htab->ovtab, NULL, ovout);
    }

  if (htab->toe != NULL)
    (*htab->params->place_spu_section) (htab->toe, NULL, ".toe");
}

/* Functions to handle embedded spu_ovl.o object.  */

static void *
ovl_mgr_open (struct bfd *nbfd ATTRIBUTE_UNUSED, void *stream)
{
  return stream;
}

static file_ptr
ovl_mgr_pread (struct bfd *abfd ATTRIBUTE_UNUSED,
	       void *stream,
	       void *buf,
	       file_ptr nbytes,
	       file_ptr offset)
{
  struct _ovl_stream *os;
  size_t count;
  size_t max;

  os = (struct _ovl_stream *) stream;
  max = (const char *) os->end - (const char *) os->start;

  if ((ufile_ptr) offset >= max)
    return 0;

  count = nbytes;
  if (count > max - offset)
    count = max - offset;

  memcpy (buf, (const char *) os->start + offset, count);
  return count;
}

static int
ovl_mgr_stat (struct bfd *abfd ATTRIBUTE_UNUSED,
	      void *stream,
	      struct stat *sb)
{
  struct _ovl_stream *os = (struct _ovl_stream *) stream;

  memset (sb, 0, sizeof (*sb));
  sb->st_size = (const char *) os->end - (const char *) os->start;
  return 0;
}

bool
spu_elf_open_builtin_lib (bfd **ovl_bfd, const struct _ovl_stream *stream)
{
  *ovl_bfd = bfd_openr_iovec ("builtin ovl_mgr",
			      "elf32-spu",
			      ovl_mgr_open,
			      (void *) stream,
			      ovl_mgr_pread,
			      NULL,
			      ovl_mgr_stat);
  return *ovl_bfd != NULL;
}

static unsigned int
overlay_index (asection *sec)
{
  if (sec == NULL
      || sec->output_section == bfd_abs_section_ptr)
    return 0;
  return spu_elf_section_data (sec->output_section)->u.o.ovl_index;
}

/* Define an STT_OBJECT symbol.  */

static struct elf_link_hash_entry *
define_ovtab_symbol (struct spu_link_hash_table *htab, const char *name)
{
  struct elf_link_hash_entry *h;

  h = elf_link_hash_lookup (&htab->elf, name, true, false, false);
  if (h == NULL)
    return NULL;

  if (h->root.type != bfd_link_hash_defined
      || !h->def_regular)
    {
      h->root.type = bfd_link_hash_defined;
      h->root.u.def.section = htab->ovtab;
      h->type = STT_OBJECT;
      h->ref_regular = 1;
      h->def_regular = 1;
      h->ref_regular_nonweak = 1;
      h->non_elf = 0;
    }
  else if (h->root.u.def.section->owner != NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB is not allowed to define %s"),
			  h->root.u.def.section->owner,
			  h->root.root.string);
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }
  else
    {
      _bfd_error_handler (_("you are not allowed to define %s in a script"),
			  h->root.root.string);
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }

  return h;
}

/* Fill in all stubs and the overlay tables.  */

static bool
spu_elf_build_stubs (struct bfd_link_info *info)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);
  struct elf_link_hash_entry *h;
  bfd_byte *p;
  asection *s;
  bfd *obfd;
  unsigned int i;

  if (htab->num_overlays != 0)
    {
      for (i = 0; i < 2; i++)
	{
	  h = htab->ovly_entry[i];
	  if (h != NULL
	      && (h->root.type == bfd_link_hash_defined
		  || h->root.type == bfd_link_hash_defweak)
	      && h->def_regular)
	    {
	      s = h->root.u.def.section->output_section;
	      if (spu_elf_section_data (s)->u.o.ovl_index)
		{
		  _bfd_error_handler (_("%s in overlay section"),
				      h->root.root.string);
		  bfd_set_error (bfd_error_bad_value);
		  return false;
		}
	    }
	}
    }

  if (htab->stub_sec != NULL)
    {
      for (i = 0; i <= htab->num_overlays; i++)
	if (htab->stub_sec[i]->size != 0)
	  {
	    htab->stub_sec[i]->contents = bfd_zalloc (htab->stub_sec[i]->owner,
						      htab->stub_sec[i]->size);
	    if (htab->stub_sec[i]->contents == NULL)
	      return false;
	    htab->stub_sec[i]->rawsize = htab->stub_sec[i]->size;
	    htab->stub_sec[i]->size = 0;
	  }

      /* Fill in all the stubs.  */
      process_stubs (info, true);
      if (!htab->stub_err)
	elf_link_hash_traverse (&htab->elf, build_spuear_stubs, info);

      if (htab->stub_err)
	{
	  _bfd_error_handler (_("overlay stub relocation overflow"));
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      for (i = 0; i <= htab->num_overlays; i++)
	{
	  if (htab->stub_sec[i]->size != htab->stub_sec[i]->rawsize)
	    {
	      _bfd_error_handler  (_("stubs don't match calculated size"));
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  htab->stub_sec[i]->rawsize = 0;
	}
    }

  if (htab->ovtab == NULL || htab->ovtab->size == 0)
    return true;

  htab->ovtab->contents = bfd_zalloc (htab->ovtab->owner, htab->ovtab->size);
  if (htab->ovtab->contents == NULL)
    return false;

  p = htab->ovtab->contents;
  if (htab->params->ovly_flavour == ovly_soft_icache)
    {
      bfd_vma off;

      h = define_ovtab_symbol (htab, "__icache_tag_array");
      if (h == NULL)
	return false;
      h->root.u.def.value = 0;
      h->size = 16 << htab->num_lines_log2;
      off = h->size;

      h = define_ovtab_symbol (htab, "__icache_tag_array_size");
      if (h == NULL)
	return false;
      h->root.u.def.value = 16 << htab->num_lines_log2;
      h->root.u.def.section = bfd_abs_section_ptr;

      h = define_ovtab_symbol (htab, "__icache_rewrite_to");
      if (h == NULL)
	return false;
      h->root.u.def.value = off;
      h->size = 16 << htab->num_lines_log2;
      off += h->size;

      h = define_ovtab_symbol (htab, "__icache_rewrite_to_size");
      if (h == NULL)
	return false;
      h->root.u.def.value = 16 << htab->num_lines_log2;
      h->root.u.def.section = bfd_abs_section_ptr;

      h = define_ovtab_symbol (htab, "__icache_rewrite_from");
      if (h == NULL)
	return false;
      h->root.u.def.value = off;
      h->size = 16 << (htab->fromelem_size_log2 + htab->num_lines_log2);
      off += h->size;

      h = define_ovtab_symbol (htab, "__icache_rewrite_from_size");
      if (h == NULL)
	return false;
      h->root.u.def.value = 16 << (htab->fromelem_size_log2
				   + htab->num_lines_log2);
      h->root.u.def.section = bfd_abs_section_ptr;

      h = define_ovtab_symbol (htab, "__icache_log2_fromelemsize");
      if (h == NULL)
	return false;
      h->root.u.def.value = htab->fromelem_size_log2;
      h->root.u.def.section = bfd_abs_section_ptr;

      h = define_ovtab_symbol (htab, "__icache_base");
      if (h == NULL)
	return false;
      h->root.u.def.value = htab->ovl_sec[0]->vma;
      h->root.u.def.section = bfd_abs_section_ptr;
      h->size = htab->num_buf << htab->line_size_log2;

      h = define_ovtab_symbol (htab, "__icache_linesize");
      if (h == NULL)
	return false;
      h->root.u.def.value = 1 << htab->line_size_log2;
      h->root.u.def.section = bfd_abs_section_ptr;

      h = define_ovtab_symbol (htab, "__icache_log2_linesize");
      if (h == NULL)
	return false;
      h->root.u.def.value = htab->line_size_log2;
      h->root.u.def.section = bfd_abs_section_ptr;

      h = define_ovtab_symbol (htab, "__icache_neg_log2_linesize");
      if (h == NULL)
	return false;
      h->root.u.def.value = -htab->line_size_log2;
      h->root.u.def.section = bfd_abs_section_ptr;

      h = define_ovtab_symbol (htab, "__icache_cachesize");
      if (h == NULL)
	return false;
      h->root.u.def.value = 1 << (htab->num_lines_log2 + htab->line_size_log2);
      h->root.u.def.section = bfd_abs_section_ptr;

      h = define_ovtab_symbol (htab, "__icache_log2_cachesize");
      if (h == NULL)
	return false;
      h->root.u.def.value = htab->num_lines_log2 + htab->line_size_log2;
      h->root.u.def.section = bfd_abs_section_ptr;

      h = define_ovtab_symbol (htab, "__icache_neg_log2_cachesize");
      if (h == NULL)
	return false;
      h->root.u.def.value = -(htab->num_lines_log2 + htab->line_size_log2);
      h->root.u.def.section = bfd_abs_section_ptr;

      if (htab->init != NULL && htab->init->size != 0)
	{
	  htab->init->contents = bfd_zalloc (htab->init->owner,
					     htab->init->size);
	  if (htab->init->contents == NULL)
	    return false;

	  h = define_ovtab_symbol (htab, "__icache_fileoff");
	  if (h == NULL)
	    return false;
	  h->root.u.def.value = 0;
	  h->root.u.def.section = htab->init;
	  h->size = 8;
	}
    }
  else
    {
      /* Write out _ovly_table.  */
      /* set low bit of .size to mark non-overlay area as present.  */
      p[7] = 1;
      obfd = htab->ovtab->output_section->owner;
      for (s = obfd->sections; s != NULL; s = s->next)
	{
	  unsigned int ovl_index = spu_elf_section_data (s)->u.o.ovl_index;

	  if (ovl_index != 0)
	    {
	      unsigned long off = ovl_index * 16;
	      unsigned int ovl_buf = spu_elf_section_data (s)->u.o.ovl_buf;

	      bfd_put_32 (htab->ovtab->owner, s->vma, p + off);
	      bfd_put_32 (htab->ovtab->owner, (s->size + 15) & -16,
			  p + off + 4);
	      /* file_off written later in spu_elf_modify_headers.  */
	      bfd_put_32 (htab->ovtab->owner, ovl_buf, p + off + 12);
	    }
	}

      h = define_ovtab_symbol (htab, "_ovly_table");
      if (h == NULL)
	return false;
      h->root.u.def.value = 16;
      h->size = htab->num_overlays * 16;

      h = define_ovtab_symbol (htab, "_ovly_table_end");
      if (h == NULL)
	return false;
      h->root.u.def.value = htab->num_overlays * 16 + 16;
      h->size = 0;

      h = define_ovtab_symbol (htab, "_ovly_buf_table");
      if (h == NULL)
	return false;
      h->root.u.def.value = htab->num_overlays * 16 + 16;
      h->size = htab->num_buf * 4;

      h = define_ovtab_symbol (htab, "_ovly_buf_table_end");
      if (h == NULL)
	return false;
      h->root.u.def.value = htab->num_overlays * 16 + 16 + htab->num_buf * 4;
      h->size = 0;
    }

  h = define_ovtab_symbol (htab, "_EAR_");
  if (h == NULL)
    return false;
  h->root.u.def.section = htab->toe;
  h->root.u.def.value = 0;
  h->size = 16;

  return true;
}

/* Check that all loadable section VMAs lie in the range
   LO .. HI inclusive, and stash some parameters for --auto-overlay.  */

asection *
spu_elf_check_vma (struct bfd_link_info *info)
{
  struct elf_segment_map *m;
  unsigned int i;
  struct spu_link_hash_table *htab = spu_hash_table (info);
  bfd *abfd = info->output_bfd;
  bfd_vma hi = htab->params->local_store_hi;
  bfd_vma lo = htab->params->local_store_lo;

  htab->local_store = hi + 1 - lo;

  for (m = elf_seg_map (abfd); m != NULL; m = m->next)
    if (m->p_type == PT_LOAD)
      for (i = 0; i < m->count; i++)
	if (m->sections[i]->size != 0
	    && (m->sections[i]->vma < lo
		|| m->sections[i]->vma > hi
		|| m->sections[i]->vma + m->sections[i]->size - 1 > hi))
	  return m->sections[i];

  return NULL;
}

/* OFFSET in SEC (presumably) is the beginning of a function prologue.
   Search for stack adjusting insns, and return the sp delta.
   If a store of lr is found save the instruction offset to *LR_STORE.
   If a stack adjusting instruction is found, save that offset to
   *SP_ADJUST.  */

static int
find_function_stack_adjust (asection *sec,
			    bfd_vma offset,
			    bfd_vma *lr_store,
			    bfd_vma *sp_adjust)
{
  int32_t reg[128];

  memset (reg, 0, sizeof (reg));
  for ( ; offset + 4 <= sec->size; offset += 4)
    {
      unsigned char buf[4];
      int rt, ra;
      uint32_t imm;

      /* Assume no relocs on stack adjusing insns.  */
      if (!bfd_get_section_contents (sec->owner, sec, buf, offset, 4))
	break;

      rt = buf[3] & 0x7f;
      ra = ((buf[2] & 0x3f) << 1) | (buf[3] >> 7);

      if (buf[0] == 0x24 /* stqd */)
	{
	  if (rt == 0 /* lr */ && ra == 1 /* sp */)
	    *lr_store = offset;
	  continue;
	}

      /* Partly decoded immediate field.  */
      imm = (buf[1] << 9) | (buf[2] << 1) | (buf[3] >> 7);

      if (buf[0] == 0x1c /* ai */)
	{
	  imm >>= 7;
	  imm = (imm ^ 0x200) - 0x200;
	  reg[rt] = reg[ra] + imm;

	  if (rt == 1 /* sp */)
	    {
	      if (reg[rt] > 0)
		break;
	      *sp_adjust = offset;
	      return reg[rt];
	    }
	}
      else if (buf[0] == 0x18 && (buf[1] & 0xe0) == 0 /* a */)
	{
	  int rb = ((buf[1] & 0x1f) << 2) | ((buf[2] & 0xc0) >> 6);

	  reg[rt] = reg[ra] + reg[rb];
	  if (rt == 1)
	    {
	      if (reg[rt] > 0)
		break;
	      *sp_adjust = offset;
	      return reg[rt];
	    }
	}
      else if (buf[0] == 0x08 && (buf[1] & 0xe0) == 0 /* sf */)
	{
	  int rb = ((buf[1] & 0x1f) << 2) | ((buf[2] & 0xc0) >> 6);

	  reg[rt] = reg[rb] - reg[ra];
	  if (rt == 1)
	    {
	      if (reg[rt] > 0)
		break;
	      *sp_adjust = offset;
	      return reg[rt];
	    }
	}
      else if ((buf[0] & 0xfc) == 0x40 /* il, ilh, ilhu, ila */)
	{
	  if (buf[0] >= 0x42 /* ila */)
	    imm |= (buf[0] & 1) << 17;
	  else
	    {
	      imm &= 0xffff;

	      if (buf[0] == 0x40 /* il */)
		{
		  if ((buf[1] & 0x80) == 0)
		    continue;
		  imm = (imm ^ 0x8000) - 0x8000;
		}
	      else if ((buf[1] & 0x80) == 0 /* ilhu */)
		imm <<= 16;
	    }
	  reg[rt] = imm;
	  continue;
	}
      else if (buf[0] == 0x60 && (buf[1] & 0x80) != 0 /* iohl */)
	{
	  reg[rt] |= imm & 0xffff;
	  continue;
	}
      else if (buf[0] == 0x04 /* ori */)
	{
	  imm >>= 7;
	  imm = (imm ^ 0x200) - 0x200;
	  reg[rt] = reg[ra] | imm;
	  continue;
	}
      else if (buf[0] == 0x32 && (buf[1] & 0x80) != 0 /* fsmbi */)
	{
	  reg[rt] = (  ((imm & 0x8000) ? 0xff000000 : 0)
		     | ((imm & 0x4000) ? 0x00ff0000 : 0)
		     | ((imm & 0x2000) ? 0x0000ff00 : 0)
		     | ((imm & 0x1000) ? 0x000000ff : 0));
	  continue;
	}
      else if (buf[0] == 0x16 /* andbi */)
	{
	  imm >>= 7;
	  imm &= 0xff;
	  imm |= imm << 8;
	  imm |= imm << 16;
	  reg[rt] = reg[ra] & imm;
	  continue;
	}
      else if (buf[0] == 0x33 && imm == 1 /* brsl .+4 */)
	{
	  /* Used in pic reg load.  Say rt is trashed.  Won't be used
	     in stack adjust, but we need to continue past this branch.  */
	  reg[rt] = 0;
	  continue;
	}
      else if (is_branch (buf) || is_indirect_branch (buf))
	/* If we hit a branch then we must be out of the prologue.  */
	break;
    }

  return 0;
}

/* qsort predicate to sort symbols by section and value.  */

static Elf_Internal_Sym *sort_syms_syms;
static asection **sort_syms_psecs;

static int
sort_syms (const void *a, const void *b)
{
  Elf_Internal_Sym *const *s1 = a;
  Elf_Internal_Sym *const *s2 = b;
  asection *sec1,*sec2;
  bfd_signed_vma delta;

  sec1 = sort_syms_psecs[*s1 - sort_syms_syms];
  sec2 = sort_syms_psecs[*s2 - sort_syms_syms];

  if (sec1 != sec2)
    return sec1->index - sec2->index;

  delta = (*s1)->st_value - (*s2)->st_value;
  if (delta != 0)
    return delta < 0 ? -1 : 1;

  delta = (*s2)->st_size - (*s1)->st_size;
  if (delta != 0)
    return delta < 0 ? -1 : 1;

  return *s1 < *s2 ? -1 : 1;
}

/* Allocate a struct spu_elf_stack_info with MAX_FUN struct function_info
   entries for section SEC.  */

static struct spu_elf_stack_info *
alloc_stack_info (asection *sec, int max_fun)
{
  struct _spu_elf_section_data *sec_data = spu_elf_section_data (sec);
  bfd_size_type amt;

  amt = sizeof (struct spu_elf_stack_info);
  amt += (max_fun - 1) * sizeof (struct function_info);
  sec_data->u.i.stack_info = bfd_zmalloc (amt);
  if (sec_data->u.i.stack_info != NULL)
    sec_data->u.i.stack_info->max_fun = max_fun;
  return sec_data->u.i.stack_info;
}

/* Add a new struct function_info describing a (part of a) function
   starting at SYM_H.  Keep the array sorted by address.  */

static struct function_info *
maybe_insert_function (asection *sec,
		       void *sym_h,
		       bool global,
		       bool is_func)
{
  struct _spu_elf_section_data *sec_data = spu_elf_section_data (sec);
  struct spu_elf_stack_info *sinfo = sec_data->u.i.stack_info;
  int i;
  bfd_vma off, size;

  if (sinfo == NULL)
    {
      sinfo = alloc_stack_info (sec, 20);
      if (sinfo == NULL)
	return NULL;
    }

  if (!global)
    {
      Elf_Internal_Sym *sym = sym_h;
      off = sym->st_value;
      size = sym->st_size;
    }
  else
    {
      struct elf_link_hash_entry *h = sym_h;
      off = h->root.u.def.value;
      size = h->size;
    }

  for (i = sinfo->num_fun; --i >= 0; )
    if (sinfo->fun[i].lo <= off)
      break;

  if (i >= 0)
    {
      /* Don't add another entry for an alias, but do update some
	 info.  */
      if (sinfo->fun[i].lo == off)
	{
	  /* Prefer globals over local syms.  */
	  if (global && !sinfo->fun[i].global)
	    {
	      sinfo->fun[i].global = true;
	      sinfo->fun[i].u.h = sym_h;
	    }
	  if (is_func)
	    sinfo->fun[i].is_func = true;
	  return &sinfo->fun[i];
	}
      /* Ignore a zero-size symbol inside an existing function.  */
      else if (sinfo->fun[i].hi > off && size == 0)
	return &sinfo->fun[i];
    }

  if (sinfo->num_fun >= sinfo->max_fun)
    {
      bfd_size_type amt = sizeof (struct spu_elf_stack_info);
      bfd_size_type old = amt;

      old += (sinfo->max_fun - 1) * sizeof (struct function_info);
      sinfo->max_fun += 20 + (sinfo->max_fun >> 1);
      amt += (sinfo->max_fun - 1) * sizeof (struct function_info);
      sinfo = bfd_realloc (sinfo, amt);
      if (sinfo == NULL)
	return NULL;
      memset ((char *) sinfo + old, 0, amt - old);
      sec_data->u.i.stack_info = sinfo;
    }

  if (++i < sinfo->num_fun)
    memmove (&sinfo->fun[i + 1], &sinfo->fun[i],
	     (sinfo->num_fun - i) * sizeof (sinfo->fun[i]));
  sinfo->fun[i].is_func = is_func;
  sinfo->fun[i].global = global;
  sinfo->fun[i].sec = sec;
  if (global)
    sinfo->fun[i].u.h = sym_h;
  else
    sinfo->fun[i].u.sym = sym_h;
  sinfo->fun[i].lo = off;
  sinfo->fun[i].hi = off + size;
  sinfo->fun[i].lr_store = -1;
  sinfo->fun[i].sp_adjust = -1;
  sinfo->fun[i].stack = -find_function_stack_adjust (sec, off,
						     &sinfo->fun[i].lr_store,
						     &sinfo->fun[i].sp_adjust);
  sinfo->num_fun += 1;
  return &sinfo->fun[i];
}

/* Return the name of FUN.  */

static const char *
func_name (struct function_info *fun)
{
  asection *sec;
  bfd *ibfd;
  Elf_Internal_Shdr *symtab_hdr;

  while (fun->start != NULL)
    fun = fun->start;

  if (fun->global)
    return fun->u.h->root.root.string;

  sec = fun->sec;
  if (fun->u.sym->st_name == 0)
    {
      size_t len = strlen (sec->name);
      char *name = bfd_malloc (len + 10);
      if (name == NULL)
	return "(null)";
      sprintf (name, "%s+%lx", sec->name,
	       (unsigned long) fun->u.sym->st_value & 0xffffffff);
      return name;
    }
  ibfd = sec->owner;
  symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
  return bfd_elf_sym_name (ibfd, symtab_hdr, fun->u.sym, sec);
}

/* Read the instruction at OFF in SEC.  Return true iff the instruction
   is a nop, lnop, or stop 0 (all zero insn).  */

static bool
is_nop (asection *sec, bfd_vma off)
{
  unsigned char insn[4];

  if (off + 4 > sec->size
      || !bfd_get_section_contents (sec->owner, sec, insn, off, 4))
    return false;
  if ((insn[0] & 0xbf) == 0 && (insn[1] & 0xe0) == 0x20)
    return true;
  if (insn[0] == 0 && insn[1] == 0 && insn[2] == 0 && insn[3] == 0)
    return true;
  return false;
}

/* Extend the range of FUN to cover nop padding up to LIMIT.
   Return TRUE iff some instruction other than a NOP was found.  */

static bool
insns_at_end (struct function_info *fun, bfd_vma limit)
{
  bfd_vma off = (fun->hi + 3) & -4;

  while (off < limit && is_nop (fun->sec, off))
    off += 4;
  if (off < limit)
    {
      fun->hi = off;
      return true;
    }
  fun->hi = limit;
  return false;
}

/* Check and fix overlapping function ranges.  Return TRUE iff there
   are gaps in the current info we have about functions in SEC.  */

static bool
check_function_ranges (asection *sec, struct bfd_link_info *info)
{
  struct _spu_elf_section_data *sec_data = spu_elf_section_data (sec);
  struct spu_elf_stack_info *sinfo = sec_data->u.i.stack_info;
  int i;
  bool gaps = false;

  if (sinfo == NULL)
    return false;

  for (i = 1; i < sinfo->num_fun; i++)
    if (sinfo->fun[i - 1].hi > sinfo->fun[i].lo)
      {
	/* Fix overlapping symbols.  */
	const char *f1 = func_name (&sinfo->fun[i - 1]);
	const char *f2 = func_name (&sinfo->fun[i]);

	/* xgettext:c-format */
	info->callbacks->einfo (_("warning: %s overlaps %s\n"), f1, f2);
	sinfo->fun[i - 1].hi = sinfo->fun[i].lo;
      }
    else if (insns_at_end (&sinfo->fun[i - 1], sinfo->fun[i].lo))
      gaps = true;

  if (sinfo->num_fun == 0)
    gaps = true;
  else
    {
      if (sinfo->fun[0].lo != 0)
	gaps = true;
      if (sinfo->fun[sinfo->num_fun - 1].hi > sec->size)
	{
	  const char *f1 = func_name (&sinfo->fun[sinfo->num_fun - 1]);

	  info->callbacks->einfo (_("warning: %s exceeds section size\n"), f1);
	  sinfo->fun[sinfo->num_fun - 1].hi = sec->size;
	}
      else if (insns_at_end (&sinfo->fun[sinfo->num_fun - 1], sec->size))
	gaps = true;
    }
  return gaps;
}

/* Search current function info for a function that contains address
   OFFSET in section SEC.  */

static struct function_info *
find_function (asection *sec, bfd_vma offset, struct bfd_link_info *info)
{
  struct _spu_elf_section_data *sec_data = spu_elf_section_data (sec);
  struct spu_elf_stack_info *sinfo = sec_data->u.i.stack_info;
  int lo, hi, mid;

  lo = 0;
  hi = sinfo->num_fun;
  while (lo < hi)
    {
      mid = (lo + hi) / 2;
      if (offset < sinfo->fun[mid].lo)
	hi = mid;
      else if (offset >= sinfo->fun[mid].hi)
	lo = mid + 1;
      else
	return &sinfo->fun[mid];
    }
  /* xgettext:c-format */
  info->callbacks->einfo (_("%pA:0x%v not found in function table\n"),
			  sec, offset);
  bfd_set_error (bfd_error_bad_value);
  return NULL;
}

/* Add CALLEE to CALLER call list if not already present.  Return TRUE
   if CALLEE was new.  If this function return FALSE, CALLEE should
   be freed.  */

static bool
insert_callee (struct function_info *caller, struct call_info *callee)
{
  struct call_info **pp, *p;

  for (pp = &caller->call_list; (p = *pp) != NULL; pp = &p->next)
    if (p->fun == callee->fun)
      {
	/* Tail calls use less stack than normal calls.  Retain entry
	   for normal call over one for tail call.  */
	p->is_tail &= callee->is_tail;
	if (!p->is_tail)
	  {
	    p->fun->start = NULL;
	    p->fun->is_func = true;
	  }
	p->count += callee->count;
	/* Reorder list so most recent call is first.  */
	*pp = p->next;
	p->next = caller->call_list;
	caller->call_list = p;
	return false;
      }
  callee->next = caller->call_list;
  caller->call_list = callee;
  return true;
}

/* Copy CALL and insert the copy into CALLER.  */

static bool
copy_callee (struct function_info *caller, const struct call_info *call)
{
  struct call_info *callee;
  callee = bfd_malloc (sizeof (*callee));
  if (callee == NULL)
    return false;
  *callee = *call;
  if (!insert_callee (caller, callee))
    free (callee);
  return true;
}

/* We're only interested in code sections.  Testing SEC_IN_MEMORY excludes
   overlay stub sections.  */

static bool
interesting_section (asection *s)
{
  return (s->output_section != bfd_abs_section_ptr
	  && ((s->flags & (SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_IN_MEMORY))
	      == (SEC_ALLOC | SEC_LOAD | SEC_CODE))
	  && s->size != 0);
}

/* Rummage through the relocs for SEC, looking for function calls.
   If CALL_TREE is true, fill in call graph.  If CALL_TREE is false,
   mark destination symbols on calls as being functions.  Also
   look at branches, which may be tail calls or go to hot/cold
   section part of same function.  */

static bool
mark_functions_via_relocs (asection *sec,
			   struct bfd_link_info *info,
			   int call_tree)
{
  Elf_Internal_Rela *internal_relocs, *irelaend, *irela;
  Elf_Internal_Shdr *symtab_hdr;
  void *psyms;
  unsigned int priority = 0;
  static bool warned;

  if (!interesting_section (sec)
      || sec->reloc_count == 0)
    return true;

  internal_relocs = _bfd_elf_link_read_relocs (sec->owner, sec, NULL, NULL,
					       info->keep_memory);
  if (internal_relocs == NULL)
    return false;

  symtab_hdr = &elf_tdata (sec->owner)->symtab_hdr;
  psyms = &symtab_hdr->contents;
  irela = internal_relocs;
  irelaend = irela + sec->reloc_count;
  for (; irela < irelaend; irela++)
    {
      enum elf_spu_reloc_type r_type;
      unsigned int r_indx;
      asection *sym_sec;
      Elf_Internal_Sym *sym;
      struct elf_link_hash_entry *h;
      bfd_vma val;
      bool nonbranch, is_call;
      struct function_info *caller;
      struct call_info *callee;

      r_type = ELF32_R_TYPE (irela->r_info);
      nonbranch = r_type != R_SPU_REL16 && r_type != R_SPU_ADDR16;

      r_indx = ELF32_R_SYM (irela->r_info);
      if (!get_sym_h (&h, &sym, &sym_sec, psyms, r_indx, sec->owner))
	return false;

      if (sym_sec == NULL
	  || sym_sec->output_section == bfd_abs_section_ptr)
	continue;

      is_call = false;
      if (!nonbranch)
	{
	  unsigned char insn[4];

	  if (!bfd_get_section_contents (sec->owner, sec, insn,
					 irela->r_offset, 4))
	    return false;
	  if (is_branch (insn))
	    {
	      is_call = (insn[0] & 0xfd) == 0x31;
	      priority = insn[1] & 0x0f;
	      priority <<= 8;
	      priority |= insn[2];
	      priority <<= 8;
	      priority |= insn[3];
	      priority >>= 7;
	      if ((sym_sec->flags & (SEC_ALLOC | SEC_LOAD | SEC_CODE))
		  != (SEC_ALLOC | SEC_LOAD | SEC_CODE))
		{
		  if (!warned)
		    info->callbacks->einfo
		      /* xgettext:c-format */
		      (_("%pB(%pA+0x%v): call to non-code section"
			 " %pB(%pA), analysis incomplete\n"),
		       sec->owner, sec, irela->r_offset,
		       sym_sec->owner, sym_sec);
		  warned = true;
		  continue;
		}
	    }
	  else
	    {
	      nonbranch = true;
	      if (is_hint (insn))
		continue;
	    }
	}

      if (nonbranch)
	{
	  /* For --auto-overlay, count possible stubs we need for
	     function pointer references.  */
	  unsigned int sym_type;
	  if (h)
	    sym_type = h->type;
	  else
	    sym_type = ELF_ST_TYPE (sym->st_info);
	  if (sym_type == STT_FUNC)
	    {
	      if (call_tree && spu_hash_table (info)->params->auto_overlay)
		spu_hash_table (info)->non_ovly_stub += 1;
	      /* If the symbol type is STT_FUNC then this must be a
		 function pointer initialisation.  */
	      continue;
	    }
	  /* Ignore data references.  */
	  if ((sym_sec->flags & (SEC_ALLOC | SEC_LOAD | SEC_CODE))
	      != (SEC_ALLOC | SEC_LOAD | SEC_CODE))
	    continue;
	  /* Otherwise we probably have a jump table reloc for
	     a switch statement or some other reference to a
	     code label.  */
	}

      if (h)
	val = h->root.u.def.value;
      else
	val = sym->st_value;
      val += irela->r_addend;

      if (!call_tree)
	{
	  struct function_info *fun;

	  if (irela->r_addend != 0)
	    {
	      Elf_Internal_Sym *fake = bfd_zmalloc (sizeof (*fake));
	      if (fake == NULL)
		return false;
	      fake->st_value = val;
	      fake->st_shndx
		= _bfd_elf_section_from_bfd_section (sym_sec->owner, sym_sec);
	      sym = fake;
	    }
	  if (sym)
	    fun = maybe_insert_function (sym_sec, sym, false, is_call);
	  else
	    fun = maybe_insert_function (sym_sec, h, true, is_call);
	  if (fun == NULL)
	    return false;
	  if (irela->r_addend != 0
	      && fun->u.sym != sym)
	    free (sym);
	  continue;
	}

      caller = find_function (sec, irela->r_offset, info);
      if (caller == NULL)
	return false;
      callee = bfd_malloc (sizeof *callee);
      if (callee == NULL)
	return false;

      callee->fun = find_function (sym_sec, val, info);
      if (callee->fun == NULL)
	return false;
      callee->is_tail = !is_call;
      callee->is_pasted = false;
      callee->broken_cycle = false;
      callee->priority = priority;
      callee->count = nonbranch? 0 : 1;
      if (callee->fun->last_caller != sec)
	{
	  callee->fun->last_caller = sec;
	  callee->fun->call_count += 1;
	}
      if (!insert_callee (caller, callee))
	free (callee);
      else if (!is_call
	       && !callee->fun->is_func
	       && callee->fun->stack == 0)
	{
	  /* This is either a tail call or a branch from one part of
	     the function to another, ie. hot/cold section.  If the
	     destination has been called by some other function then
	     it is a separate function.  We also assume that functions
	     are not split across input files.  */
	  if (sec->owner != sym_sec->owner)
	    {
	      callee->fun->start = NULL;
	      callee->fun->is_func = true;
	    }
	  else if (callee->fun->start == NULL)
	    {
	      struct function_info *caller_start = caller;
	      while (caller_start->start)
		caller_start = caller_start->start;

	      if (caller_start != callee->fun)
		callee->fun->start = caller_start;
	    }
	  else
	    {
	      struct function_info *callee_start;
	      struct function_info *caller_start;
	      callee_start = callee->fun;
	      while (callee_start->start)
		callee_start = callee_start->start;
	      caller_start = caller;
	      while (caller_start->start)
		caller_start = caller_start->start;
	      if (caller_start != callee_start)
		{
		  callee->fun->start = NULL;
		  callee->fun->is_func = true;
		}
	    }
	}
    }

  return true;
}

/* Handle something like .init or .fini, which has a piece of a function.
   These sections are pasted together to form a single function.  */

static bool
pasted_function (asection *sec)
{
  struct bfd_link_order *l;
  struct _spu_elf_section_data *sec_data;
  struct spu_elf_stack_info *sinfo;
  Elf_Internal_Sym *fake;
  struct function_info *fun, *fun_start;

  fake = bfd_zmalloc (sizeof (*fake));
  if (fake == NULL)
    return false;
  fake->st_value = 0;
  fake->st_size = sec->size;
  fake->st_shndx
    = _bfd_elf_section_from_bfd_section (sec->owner, sec);
  fun = maybe_insert_function (sec, fake, false, false);
  if (!fun)
    return false;

  /* Find a function immediately preceding this section.  */
  fun_start = NULL;
  for (l = sec->output_section->map_head.link_order; l != NULL; l = l->next)
    {
      if (l->u.indirect.section == sec)
	{
	  if (fun_start != NULL)
	    {
	      struct call_info *callee = bfd_malloc (sizeof *callee);
	      if (callee == NULL)
		return false;

	      fun->start = fun_start;
	      callee->fun = fun;
	      callee->is_tail = true;
	      callee->is_pasted = true;
	      callee->broken_cycle = false;
	      callee->priority = 0;
	      callee->count = 1;
	      if (!insert_callee (fun_start, callee))
		free (callee);
	      return true;
	    }
	  break;
	}
      if (l->type == bfd_indirect_link_order
	  && (sec_data = spu_elf_section_data (l->u.indirect.section)) != NULL
	  && (sinfo = sec_data->u.i.stack_info) != NULL
	  && sinfo->num_fun != 0)
	fun_start = &sinfo->fun[sinfo->num_fun - 1];
    }

  /* Don't return an error if we did not find a function preceding this
     section.  The section may have incorrect flags.  */
  return true;
}

/* Map address ranges in code sections to functions.  */

static bool
discover_functions (struct bfd_link_info *info)
{
  bfd *ibfd;
  int bfd_idx;
  Elf_Internal_Sym ***psym_arr;
  asection ***sec_arr;
  bool gaps = false;

  bfd_idx = 0;
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    bfd_idx++;

  psym_arr = bfd_zmalloc (bfd_idx * sizeof (*psym_arr));
  if (psym_arr == NULL)
    return false;
  sec_arr = bfd_zmalloc (bfd_idx * sizeof (*sec_arr));
  if (sec_arr == NULL)
    return false;

  for (ibfd = info->input_bfds, bfd_idx = 0;
       ibfd != NULL;
       ibfd = ibfd->link.next, bfd_idx++)
    {
      extern const bfd_target spu_elf32_vec;
      Elf_Internal_Shdr *symtab_hdr;
      asection *sec;
      size_t symcount;
      Elf_Internal_Sym *syms, *sy, **psyms, **psy;
      asection **psecs, **p;

      if (ibfd->xvec != &spu_elf32_vec)
	continue;

      /* Read all the symbols.  */
      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      symcount = symtab_hdr->sh_size / symtab_hdr->sh_entsize;
      if (symcount == 0)
	{
	  if (!gaps)
	    for (sec = ibfd->sections; sec != NULL && !gaps; sec = sec->next)
	      if (interesting_section (sec))
		{
		  gaps = true;
		  break;
		}
	  continue;
	}

      /* Don't use cached symbols since the generic ELF linker
	 code only reads local symbols, and we need globals too.  */
      free (symtab_hdr->contents);
      symtab_hdr->contents = NULL;
      syms = bfd_elf_get_elf_syms (ibfd, symtab_hdr, symcount, 0,
				   NULL, NULL, NULL);
      symtab_hdr->contents = (void *) syms;
      if (syms == NULL)
	return false;

      /* Select defined function symbols that are going to be output.  */
      psyms = bfd_malloc ((symcount + 1) * sizeof (*psyms));
      if (psyms == NULL)
	return false;
      psym_arr[bfd_idx] = psyms;
      psecs = bfd_malloc (symcount * sizeof (*psecs));
      if (psecs == NULL)
	return false;
      sec_arr[bfd_idx] = psecs;
      for (psy = psyms, p = psecs, sy = syms; sy < syms + symcount; ++p, ++sy)
	if (ELF_ST_TYPE (sy->st_info) == STT_NOTYPE
	    || ELF_ST_TYPE (sy->st_info) == STT_FUNC)
	  {
	    asection *s;

	    *p = s = bfd_section_from_elf_index (ibfd, sy->st_shndx);
	    if (s != NULL && interesting_section (s))
	      *psy++ = sy;
	  }
      symcount = psy - psyms;
      *psy = NULL;

      /* Sort them by section and offset within section.  */
      sort_syms_syms = syms;
      sort_syms_psecs = psecs;
      qsort (psyms, symcount, sizeof (*psyms), sort_syms);

      /* Now inspect the function symbols.  */
      for (psy = psyms; psy < psyms + symcount; )
	{
	  asection *s = psecs[*psy - syms];
	  Elf_Internal_Sym **psy2;

	  for (psy2 = psy; ++psy2 < psyms + symcount; )
	    if (psecs[*psy2 - syms] != s)
	      break;

	  if (!alloc_stack_info (s, psy2 - psy))
	    return false;
	  psy = psy2;
	}

      /* First install info about properly typed and sized functions.
	 In an ideal world this will cover all code sections, except
	 when partitioning functions into hot and cold sections,
	 and the horrible pasted together .init and .fini functions.  */
      for (psy = psyms; psy < psyms + symcount; ++psy)
	{
	  sy = *psy;
	  if (ELF_ST_TYPE (sy->st_info) == STT_FUNC)
	    {
	      asection *s = psecs[sy - syms];
	      if (!maybe_insert_function (s, sy, false, true))
		return false;
	    }
	}

      for (sec = ibfd->sections; sec != NULL && !gaps; sec = sec->next)
	if (interesting_section (sec))
	  gaps |= check_function_ranges (sec, info);
    }

  if (gaps)
    {
      /* See if we can discover more function symbols by looking at
	 relocations.  */
      for (ibfd = info->input_bfds, bfd_idx = 0;
	   ibfd != NULL;
	   ibfd = ibfd->link.next, bfd_idx++)
	{
	  asection *sec;

	  if (psym_arr[bfd_idx] == NULL)
	    continue;

	  for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	    if (!mark_functions_via_relocs (sec, info, false))
	      return false;
	}

      for (ibfd = info->input_bfds, bfd_idx = 0;
	   ibfd != NULL;
	   ibfd = ibfd->link.next, bfd_idx++)
	{
	  Elf_Internal_Shdr *symtab_hdr;
	  asection *sec;
	  Elf_Internal_Sym *syms, *sy, **psyms, **psy;
	  asection **psecs;

	  if ((psyms = psym_arr[bfd_idx]) == NULL)
	    continue;

	  psecs = sec_arr[bfd_idx];

	  symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
	  syms = (Elf_Internal_Sym *) symtab_hdr->contents;

	  gaps = false;
	  for (sec = ibfd->sections; sec != NULL && !gaps; sec = sec->next)
	    if (interesting_section (sec))
	      gaps |= check_function_ranges (sec, info);
	  if (!gaps)
	    continue;

	  /* Finally, install all globals.  */
	  for (psy = psyms; (sy = *psy) != NULL; ++psy)
	    {
	      asection *s;

	      s = psecs[sy - syms];

	      /* Global syms might be improperly typed functions.  */
	      if (ELF_ST_TYPE (sy->st_info) != STT_FUNC
		  && ELF_ST_BIND (sy->st_info) == STB_GLOBAL)
		{
		  if (!maybe_insert_function (s, sy, false, false))
		    return false;
		}
	    }
	}

      for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
	{
	  extern const bfd_target spu_elf32_vec;
	  asection *sec;

	  if (ibfd->xvec != &spu_elf32_vec)
	    continue;

	  /* Some of the symbols we've installed as marking the
	     beginning of functions may have a size of zero.  Extend
	     the range of such functions to the beginning of the
	     next symbol of interest.  */
	  for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	    if (interesting_section (sec))
	      {
		struct _spu_elf_section_data *sec_data;
		struct spu_elf_stack_info *sinfo;

		sec_data = spu_elf_section_data (sec);
		sinfo = sec_data->u.i.stack_info;
		if (sinfo != NULL && sinfo->num_fun != 0)
		  {
		    int fun_idx;
		    bfd_vma hi = sec->size;

		    for (fun_idx = sinfo->num_fun; --fun_idx >= 0; )
		      {
			sinfo->fun[fun_idx].hi = hi;
			hi = sinfo->fun[fun_idx].lo;
		      }

		    sinfo->fun[0].lo = 0;
		  }
		/* No symbols in this section.  Must be .init or .fini
		   or something similar.  */
		else if (!pasted_function (sec))
		  return false;
	      }
	}
    }

  for (ibfd = info->input_bfds, bfd_idx = 0;
       ibfd != NULL;
       ibfd = ibfd->link.next, bfd_idx++)
    {
      if (psym_arr[bfd_idx] == NULL)
	continue;

      free (psym_arr[bfd_idx]);
      free (sec_arr[bfd_idx]);
    }

  free (psym_arr);
  free (sec_arr);

  return true;
}

/* Iterate over all function_info we have collected, calling DOIT on
   each node if ROOT_ONLY is false.  Only call DOIT on root nodes
   if ROOT_ONLY.  */

static bool
for_each_node (bool (*doit) (struct function_info *,
			     struct bfd_link_info *,
			     void *),
	       struct bfd_link_info *info,
	       void *param,
	       int root_only)
{
  bfd *ibfd;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      extern const bfd_target spu_elf32_vec;
      asection *sec;

      if (ibfd->xvec != &spu_elf32_vec)
	continue;

      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	{
	  struct _spu_elf_section_data *sec_data;
	  struct spu_elf_stack_info *sinfo;

	  if ((sec_data = spu_elf_section_data (sec)) != NULL
	      && (sinfo = sec_data->u.i.stack_info) != NULL)
	    {
	      int i;
	      for (i = 0; i < sinfo->num_fun; ++i)
		if (!root_only || !sinfo->fun[i].non_root)
		  if (!doit (&sinfo->fun[i], info, param))
		    return false;
	    }
	}
    }
  return true;
}

/* Transfer call info attached to struct function_info entries for
   all of a given function's sections to the first entry.  */

static bool
transfer_calls (struct function_info *fun,
		struct bfd_link_info *info ATTRIBUTE_UNUSED,
		void *param ATTRIBUTE_UNUSED)
{
  struct function_info *start = fun->start;

  if (start != NULL)
    {
      struct call_info *call, *call_next;

      while (start->start != NULL)
	start = start->start;
      for (call = fun->call_list; call != NULL; call = call_next)
	{
	  call_next = call->next;
	  if (!insert_callee (start, call))
	    free (call);
	}
      fun->call_list = NULL;
    }
  return true;
}

/* Mark nodes in the call graph that are called by some other node.  */

static bool
mark_non_root (struct function_info *fun,
	       struct bfd_link_info *info ATTRIBUTE_UNUSED,
	       void *param ATTRIBUTE_UNUSED)
{
  struct call_info *call;

  if (fun->visit1)
    return true;
  fun->visit1 = true;
  for (call = fun->call_list; call; call = call->next)
    {
      call->fun->non_root = true;
      mark_non_root (call->fun, 0, 0);
    }
  return true;
}

/* Remove cycles from the call graph.  Set depth of nodes.  */

static bool
remove_cycles (struct function_info *fun,
	       struct bfd_link_info *info,
	       void *param)
{
  struct call_info **callp, *call;
  unsigned int depth = *(unsigned int *) param;
  unsigned int max_depth = depth;

  fun->depth = depth;
  fun->visit2 = true;
  fun->marking = true;

  callp = &fun->call_list;
  while ((call = *callp) != NULL)
    {
      call->max_depth = depth + !call->is_pasted;
      if (!call->fun->visit2)
	{
	  if (!remove_cycles (call->fun, info, &call->max_depth))
	    return false;
	  if (max_depth < call->max_depth)
	    max_depth = call->max_depth;
	}
      else if (call->fun->marking)
	{
	  struct spu_link_hash_table *htab = spu_hash_table (info);

	  if (!htab->params->auto_overlay
	      && htab->params->stack_analysis)
	    {
	      const char *f1 = func_name (fun);
	      const char *f2 = func_name (call->fun);

	      /* xgettext:c-format */
	      info->callbacks->info (_("stack analysis will ignore the call "
				       "from %s to %s\n"),
				     f1, f2);
	    }

	  call->broken_cycle = true;
	}
      callp = &call->next;
    }
  fun->marking = false;
  *(unsigned int *) param = max_depth;
  return true;
}

/* Check that we actually visited all nodes in remove_cycles.  If we
   didn't, then there is some cycle in the call graph not attached to
   any root node.  Arbitrarily choose a node in the cycle as a new
   root and break the cycle.  */

static bool
mark_detached_root (struct function_info *fun,
		    struct bfd_link_info *info,
		    void *param)
{
  if (fun->visit2)
    return true;
  fun->non_root = false;
  *(unsigned int *) param = 0;
  return remove_cycles (fun, info, param);
}

/* Populate call_list for each function.  */

static bool
build_call_tree (struct bfd_link_info *info)
{
  bfd *ibfd;
  unsigned int depth;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      extern const bfd_target spu_elf32_vec;
      asection *sec;

      if (ibfd->xvec != &spu_elf32_vec)
	continue;

      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	if (!mark_functions_via_relocs (sec, info, true))
	  return false;
    }

  /* Transfer call info from hot/cold section part of function
     to main entry.  */
  if (!spu_hash_table (info)->params->auto_overlay
      && !for_each_node (transfer_calls, info, 0, false))
    return false;

  /* Find the call graph root(s).  */
  if (!for_each_node (mark_non_root, info, 0, false))
    return false;

  /* Remove cycles from the call graph.  We start from the root node(s)
     so that we break cycles in a reasonable place.  */
  depth = 0;
  if (!for_each_node (remove_cycles, info, &depth, true))
    return false;

  return for_each_node (mark_detached_root, info, &depth, false);
}

/* qsort predicate to sort calls by priority, max_depth then count.  */

static int
sort_calls (const void *a, const void *b)
{
  struct call_info *const *c1 = a;
  struct call_info *const *c2 = b;
  int delta;

  delta = (*c2)->priority - (*c1)->priority;
  if (delta != 0)
    return delta;

  delta = (*c2)->max_depth - (*c1)->max_depth;
  if (delta != 0)
    return delta;

  delta = (*c2)->count - (*c1)->count;
  if (delta != 0)
    return delta;

  return (char *) c1 - (char *) c2;
}

struct _mos_param {
  unsigned int max_overlay_size;
};

/* Set linker_mark and gc_mark on any sections that we will put in
   overlays.  These flags are used by the generic ELF linker, but we
   won't be continuing on to bfd_elf_final_link so it is OK to use
   them.  linker_mark is clear before we get here.  Set segment_mark
   on sections that are part of a pasted function (excluding the last
   section).

   Set up function rodata section if --overlay-rodata.  We don't
   currently include merged string constant rodata sections since

   Sort the call graph so that the deepest nodes will be visited
   first.  */

static bool
mark_overlay_section (struct function_info *fun,
		      struct bfd_link_info *info,
		      void *param)
{
  struct call_info *call;
  unsigned int count;
  struct _mos_param *mos_param = param;
  struct spu_link_hash_table *htab = spu_hash_table (info);

  if (fun->visit4)
    return true;

  fun->visit4 = true;
  if (!fun->sec->linker_mark
      && (htab->params->ovly_flavour != ovly_soft_icache
	  || htab->params->non_ia_text
	  || startswith (fun->sec->name, ".text.ia.")
	  || strcmp (fun->sec->name, ".init") == 0
	  || strcmp (fun->sec->name, ".fini") == 0))
    {
      unsigned int size;

      fun->sec->linker_mark = 1;
      fun->sec->gc_mark = 1;
      fun->sec->segment_mark = 0;
      /* Ensure SEC_CODE is set on this text section (it ought to
	 be!), and SEC_CODE is clear on rodata sections.  We use
	 this flag to differentiate the two overlay section types.  */
      fun->sec->flags |= SEC_CODE;

      size = fun->sec->size;
      if (htab->params->auto_overlay & OVERLAY_RODATA)
	{
	  char *name = NULL;

	  /* Find the rodata section corresponding to this function's
	     text section.  */
	  if (strcmp (fun->sec->name, ".text") == 0)
	    {
	      name = bfd_malloc (sizeof (".rodata"));
	      if (name == NULL)
		return false;
	      memcpy (name, ".rodata", sizeof (".rodata"));
	    }
	  else if (startswith (fun->sec->name, ".text."))
	    {
	      size_t len = strlen (fun->sec->name);
	      name = bfd_malloc (len + 3);
	      if (name == NULL)
		return false;
	      memcpy (name, ".rodata", sizeof (".rodata"));
	      memcpy (name + 7, fun->sec->name + 5, len - 4);
	    }
	  else if (startswith (fun->sec->name, ".gnu.linkonce.t."))
	    {
	      size_t len = strlen (fun->sec->name) + 1;
	      name = bfd_malloc (len);
	      if (name == NULL)
		return false;
	      memcpy (name, fun->sec->name, len);
	      name[14] = 'r';
	    }

	  if (name != NULL)
	    {
	      asection *rodata = NULL;
	      asection *group_sec = elf_section_data (fun->sec)->next_in_group;
	      if (group_sec == NULL)
		rodata = bfd_get_section_by_name (fun->sec->owner, name);
	      else
		while (group_sec != NULL && group_sec != fun->sec)
		  {
		    if (strcmp (group_sec->name, name) == 0)
		      {
			rodata = group_sec;
			break;
		      }
		    group_sec = elf_section_data (group_sec)->next_in_group;
		  }
	      fun->rodata = rodata;
	      if (fun->rodata)
		{
		  size += fun->rodata->size;
		  if (htab->params->line_size != 0
		      && size > htab->params->line_size)
		    {
		      size -= fun->rodata->size;
		      fun->rodata = NULL;
		    }
		  else
		    {
		      fun->rodata->linker_mark = 1;
		      fun->rodata->gc_mark = 1;
		      fun->rodata->flags &= ~SEC_CODE;
		    }
		}
	      free (name);
	    }
	}
      if (mos_param->max_overlay_size < size)
	mos_param->max_overlay_size = size;
    }

  for (count = 0, call = fun->call_list; call != NULL; call = call->next)
    count += 1;

  if (count > 1)
    {
      struct call_info **calls = bfd_malloc (count * sizeof (*calls));
      if (calls == NULL)
	return false;

      for (count = 0, call = fun->call_list; call != NULL; call = call->next)
	calls[count++] = call;

      qsort (calls, count, sizeof (*calls), sort_calls);

      fun->call_list = NULL;
      while (count != 0)
	{
	  --count;
	  calls[count]->next = fun->call_list;
	  fun->call_list = calls[count];
	}
      free (calls);
    }

  for (call = fun->call_list; call != NULL; call = call->next)
    {
      if (call->is_pasted)
	{
	  /* There can only be one is_pasted call per function_info.  */
	  BFD_ASSERT (!fun->sec->segment_mark);
	  fun->sec->segment_mark = 1;
	}
      if (!call->broken_cycle
	  && !mark_overlay_section (call->fun, info, param))
	return false;
    }

  /* Don't put entry code into an overlay.  The overlay manager needs
     a stack!  Also, don't mark .ovl.init as an overlay.  */
  if (fun->lo + fun->sec->output_offset + fun->sec->output_section->vma
      == info->output_bfd->start_address
      || startswith (fun->sec->output_section->name, ".ovl.init"))
    {
      fun->sec->linker_mark = 0;
      if (fun->rodata != NULL)
	fun->rodata->linker_mark = 0;
    }
  return true;
}

/* If non-zero then unmark functions called from those within sections
   that we need to unmark.  Unfortunately this isn't reliable since the
   call graph cannot know the destination of function pointer calls.  */
#define RECURSE_UNMARK 0

struct _uos_param {
  asection *exclude_input_section;
  asection *exclude_output_section;
  unsigned long clearing;
};

/* Undo some of mark_overlay_section's work.  */

static bool
unmark_overlay_section (struct function_info *fun,
			struct bfd_link_info *info,
			void *param)
{
  struct call_info *call;
  struct _uos_param *uos_param = param;
  unsigned int excluded = 0;

  if (fun->visit5)
    return true;

  fun->visit5 = true;

  excluded = 0;
  if (fun->sec == uos_param->exclude_input_section
      || fun->sec->output_section == uos_param->exclude_output_section)
    excluded = 1;

  if (RECURSE_UNMARK)
    uos_param->clearing += excluded;

  if (RECURSE_UNMARK ? uos_param->clearing : excluded)
    {
      fun->sec->linker_mark = 0;
      if (fun->rodata)
	fun->rodata->linker_mark = 0;
    }

  for (call = fun->call_list; call != NULL; call = call->next)
    if (!call->broken_cycle
	&& !unmark_overlay_section (call->fun, info, param))
      return false;

  if (RECURSE_UNMARK)
    uos_param->clearing -= excluded;
  return true;
}

struct _cl_param {
  unsigned int lib_size;
  asection **lib_sections;
};

/* Add sections we have marked as belonging to overlays to an array
   for consideration as non-overlay sections.  The array consist of
   pairs of sections, (text,rodata), for functions in the call graph.  */

static bool
collect_lib_sections (struct function_info *fun,
		      struct bfd_link_info *info,
		      void *param)
{
  struct _cl_param *lib_param = param;
  struct call_info *call;
  unsigned int size;

  if (fun->visit6)
    return true;

  fun->visit6 = true;
  if (!fun->sec->linker_mark || !fun->sec->gc_mark || fun->sec->segment_mark)
    return true;

  size = fun->sec->size;
  if (fun->rodata)
    size += fun->rodata->size;

  if (size <= lib_param->lib_size)
    {
      *lib_param->lib_sections++ = fun->sec;
      fun->sec->gc_mark = 0;
      if (fun->rodata && fun->rodata->linker_mark && fun->rodata->gc_mark)
	{
	  *lib_param->lib_sections++ = fun->rodata;
	  fun->rodata->gc_mark = 0;
	}
      else
	*lib_param->lib_sections++ = NULL;
    }

  for (call = fun->call_list; call != NULL; call = call->next)
    if (!call->broken_cycle)
      collect_lib_sections (call->fun, info, param);

  return true;
}

/* qsort predicate to sort sections by call count.  */

static int
sort_lib (const void *a, const void *b)
{
  asection *const *s1 = a;
  asection *const *s2 = b;
  struct _spu_elf_section_data *sec_data;
  struct spu_elf_stack_info *sinfo;
  int delta;

  delta = 0;
  if ((sec_data = spu_elf_section_data (*s1)) != NULL
      && (sinfo = sec_data->u.i.stack_info) != NULL)
    {
      int i;
      for (i = 0; i < sinfo->num_fun; ++i)
	delta -= sinfo->fun[i].call_count;
    }

  if ((sec_data = spu_elf_section_data (*s2)) != NULL
      && (sinfo = sec_data->u.i.stack_info) != NULL)
    {
      int i;
      for (i = 0; i < sinfo->num_fun; ++i)
	delta += sinfo->fun[i].call_count;
    }

  if (delta != 0)
    return delta;

  return s1 - s2;
}

/* Remove some sections from those marked to be in overlays.  Choose
   those that are called from many places, likely library functions.  */

static unsigned int
auto_ovl_lib_functions (struct bfd_link_info *info, unsigned int lib_size)
{
  bfd *ibfd;
  asection **lib_sections;
  unsigned int i, lib_count;
  struct _cl_param collect_lib_param;
  struct function_info dummy_caller;
  struct spu_link_hash_table *htab;

  memset (&dummy_caller, 0, sizeof (dummy_caller));
  lib_count = 0;
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      extern const bfd_target spu_elf32_vec;
      asection *sec;

      if (ibfd->xvec != &spu_elf32_vec)
	continue;

      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	if (sec->linker_mark
	    && sec->size < lib_size
	    && (sec->flags & SEC_CODE) != 0)
	  lib_count += 1;
    }
  lib_sections = bfd_malloc (lib_count * 2 * sizeof (*lib_sections));
  if (lib_sections == NULL)
    return (unsigned int) -1;
  collect_lib_param.lib_size = lib_size;
  collect_lib_param.lib_sections = lib_sections;
  if (!for_each_node (collect_lib_sections, info, &collect_lib_param,
		      true))
    return (unsigned int) -1;
  lib_count = (collect_lib_param.lib_sections - lib_sections) / 2;

  /* Sort sections so that those with the most calls are first.  */
  if (lib_count > 1)
    qsort (lib_sections, lib_count, 2 * sizeof (*lib_sections), sort_lib);

  htab = spu_hash_table (info);
  for (i = 0; i < lib_count; i++)
    {
      unsigned int tmp, stub_size;
      asection *sec;
      struct _spu_elf_section_data *sec_data;
      struct spu_elf_stack_info *sinfo;

      sec = lib_sections[2 * i];
      /* If this section is OK, its size must be less than lib_size.  */
      tmp = sec->size;
      /* If it has a rodata section, then add that too.  */
      if (lib_sections[2 * i + 1])
	tmp += lib_sections[2 * i + 1]->size;
      /* Add any new overlay call stubs needed by the section.  */
      stub_size = 0;
      if (tmp < lib_size
	  && (sec_data = spu_elf_section_data (sec)) != NULL
	  && (sinfo = sec_data->u.i.stack_info) != NULL)
	{
	  int k;
	  struct call_info *call;

	  for (k = 0; k < sinfo->num_fun; ++k)
	    for (call = sinfo->fun[k].call_list; call; call = call->next)
	      if (call->fun->sec->linker_mark)
		{
		  struct call_info *p;
		  for (p = dummy_caller.call_list; p; p = p->next)
		    if (p->fun == call->fun)
		      break;
		  if (!p)
		    stub_size += ovl_stub_size (htab->params);
		}
	}
      if (tmp + stub_size < lib_size)
	{
	  struct call_info **pp, *p;

	  /* This section fits.  Mark it as non-overlay.  */
	  lib_sections[2 * i]->linker_mark = 0;
	  if (lib_sections[2 * i + 1])
	    lib_sections[2 * i + 1]->linker_mark = 0;
	  lib_size -= tmp + stub_size;
	  /* Call stubs to the section we just added are no longer
	     needed.  */
	  pp = &dummy_caller.call_list;
	  while ((p = *pp) != NULL)
	    if (!p->fun->sec->linker_mark)
	      {
		lib_size += ovl_stub_size (htab->params);
		*pp = p->next;
		free (p);
	      }
	    else
	      pp = &p->next;
	  /* Add new call stubs to dummy_caller.  */
	  if ((sec_data = spu_elf_section_data (sec)) != NULL
	      && (sinfo = sec_data->u.i.stack_info) != NULL)
	    {
	      int k;
	      struct call_info *call;

	      for (k = 0; k < sinfo->num_fun; ++k)
		for (call = sinfo->fun[k].call_list;
		     call;
		     call = call->next)
		  if (call->fun->sec->linker_mark)
		    {
		      struct call_info *callee;
		      callee = bfd_malloc (sizeof (*callee));
		      if (callee == NULL)
			return (unsigned int) -1;
		      *callee = *call;
		      if (!insert_callee (&dummy_caller, callee))
			free (callee);
		    }
	    }
	}
    }
  while (dummy_caller.call_list != NULL)
    {
      struct call_info *call = dummy_caller.call_list;
      dummy_caller.call_list = call->next;
      free (call);
    }
  for (i = 0; i < 2 * lib_count; i++)
    if (lib_sections[i])
      lib_sections[i]->gc_mark = 1;
  free (lib_sections);
  return lib_size;
}

/* Build an array of overlay sections.  The deepest node's section is
   added first, then its parent node's section, then everything called
   from the parent section.  The idea being to group sections to
   minimise calls between different overlays.  */

static bool
collect_overlays (struct function_info *fun,
		  struct bfd_link_info *info,
		  void *param)
{
  struct call_info *call;
  bool added_fun;
  asection ***ovly_sections = param;

  if (fun->visit7)
    return true;

  fun->visit7 = true;
  for (call = fun->call_list; call != NULL; call = call->next)
    if (!call->is_pasted && !call->broken_cycle)
      {
	if (!collect_overlays (call->fun, info, ovly_sections))
	  return false;
	break;
      }

  added_fun = false;
  if (fun->sec->linker_mark && fun->sec->gc_mark)
    {
      fun->sec->gc_mark = 0;
      *(*ovly_sections)++ = fun->sec;
      if (fun->rodata && fun->rodata->linker_mark && fun->rodata->gc_mark)
	{
	  fun->rodata->gc_mark = 0;
	  *(*ovly_sections)++ = fun->rodata;
	}
      else
	*(*ovly_sections)++ = NULL;
      added_fun = true;

      /* Pasted sections must stay with the first section.  We don't
	 put pasted sections in the array, just the first section.
	 Mark subsequent sections as already considered.  */
      if (fun->sec->segment_mark)
	{
	  struct function_info *call_fun = fun;
	  do
	    {
	      for (call = call_fun->call_list; call != NULL; call = call->next)
		if (call->is_pasted)
		  {
		    call_fun = call->fun;
		    call_fun->sec->gc_mark = 0;
		    if (call_fun->rodata)
		      call_fun->rodata->gc_mark = 0;
		    break;
		  }
	      if (call == NULL)
		abort ();
	    }
	  while (call_fun->sec->segment_mark);
	}
    }

  for (call = fun->call_list; call != NULL; call = call->next)
    if (!call->broken_cycle
	&& !collect_overlays (call->fun, info, ovly_sections))
      return false;

  if (added_fun)
    {
      struct _spu_elf_section_data *sec_data;
      struct spu_elf_stack_info *sinfo;

      if ((sec_data = spu_elf_section_data (fun->sec)) != NULL
	  && (sinfo = sec_data->u.i.stack_info) != NULL)
	{
	  int i;
	  for (i = 0; i < sinfo->num_fun; ++i)
	    if (!collect_overlays (&sinfo->fun[i], info, ovly_sections))
	      return false;
	}
    }

  return true;
}

struct _sum_stack_param {
  size_t cum_stack;
  size_t overall_stack;
  bool emit_stack_syms;
};

/* Descend the call graph for FUN, accumulating total stack required.  */

static bool
sum_stack (struct function_info *fun,
	   struct bfd_link_info *info,
	   void *param)
{
  struct call_info *call;
  struct function_info *max;
  size_t stack, cum_stack;
  const char *f1;
  bool has_call;
  struct _sum_stack_param *sum_stack_param = param;
  struct spu_link_hash_table *htab;

  cum_stack = fun->stack;
  sum_stack_param->cum_stack = cum_stack;
  if (fun->visit3)
    return true;

  has_call = false;
  max = NULL;
  for (call = fun->call_list; call; call = call->next)
    {
      if (call->broken_cycle)
	continue;
      if (!call->is_pasted)
	has_call = true;
      if (!sum_stack (call->fun, info, sum_stack_param))
	return false;
      stack = sum_stack_param->cum_stack;
      /* Include caller stack for normal calls, don't do so for
	 tail calls.  fun->stack here is local stack usage for
	 this function.  */
      if (!call->is_tail || call->is_pasted || call->fun->start != NULL)
	stack += fun->stack;
      if (cum_stack < stack)
	{
	  cum_stack = stack;
	  max = call->fun;
	}
    }

  sum_stack_param->cum_stack = cum_stack;
  stack = fun->stack;
  /* Now fun->stack holds cumulative stack.  */
  fun->stack = cum_stack;
  fun->visit3 = true;

  if (!fun->non_root
      && sum_stack_param->overall_stack < cum_stack)
    sum_stack_param->overall_stack = cum_stack;

  htab = spu_hash_table (info);
  if (htab->params->auto_overlay)
    return true;

  f1 = func_name (fun);
  if (htab->params->stack_analysis)
    {
      if (!fun->non_root)
	info->callbacks->info ("  %s: 0x%v\n", f1, (bfd_vma) cum_stack);
      info->callbacks->minfo ("%s: 0x%v 0x%v\n",
			      f1, (bfd_vma) stack, (bfd_vma) cum_stack);

      if (has_call)
	{
	  info->callbacks->minfo (_("  calls:\n"));
	  for (call = fun->call_list; call; call = call->next)
	    if (!call->is_pasted && !call->broken_cycle)
	      {
		const char *f2 = func_name (call->fun);
		const char *ann1 = call->fun == max ? "*" : " ";
		const char *ann2 = call->is_tail ? "t" : " ";

		info->callbacks->minfo ("   %s%s %s\n", ann1, ann2, f2);
	      }
	}
    }

  if (sum_stack_param->emit_stack_syms)
    {
      char *name = bfd_malloc (18 + strlen (f1));
      struct elf_link_hash_entry *h;

      if (name == NULL)
	return false;

      if (fun->global || ELF_ST_BIND (fun->u.sym->st_info) == STB_GLOBAL)
	sprintf (name, "__stack_%s", f1);
      else
	sprintf (name, "__stack_%x_%s", fun->sec->id & 0xffffffff, f1);

      h = elf_link_hash_lookup (&htab->elf, name, true, true, false);
      free (name);
      if (h != NULL
	  && (h->root.type == bfd_link_hash_new
	      || h->root.type == bfd_link_hash_undefined
	      || h->root.type == bfd_link_hash_undefweak))
	{
	  h->root.type = bfd_link_hash_defined;
	  h->root.u.def.section = bfd_abs_section_ptr;
	  h->root.u.def.value = cum_stack;
	  h->size = 0;
	  h->type = 0;
	  h->ref_regular = 1;
	  h->def_regular = 1;
	  h->ref_regular_nonweak = 1;
	  h->forced_local = 1;
	  h->non_elf = 0;
	}
    }

  return true;
}

/* SEC is part of a pasted function.  Return the call_info for the
   next section of this function.  */

static struct call_info *
find_pasted_call (asection *sec)
{
  struct _spu_elf_section_data *sec_data = spu_elf_section_data (sec);
  struct spu_elf_stack_info *sinfo = sec_data->u.i.stack_info;
  struct call_info *call;
  int k;

  for (k = 0; k < sinfo->num_fun; ++k)
    for (call = sinfo->fun[k].call_list; call != NULL; call = call->next)
      if (call->is_pasted)
	return call;
  abort ();
  return 0;
}

/* qsort predicate to sort bfds by file name.  */

static int
sort_bfds (const void *a, const void *b)
{
  bfd *const *abfd1 = a;
  bfd *const *abfd2 = b;

  return filename_cmp (bfd_get_filename (*abfd1), bfd_get_filename (*abfd2));
}

static unsigned int
print_one_overlay_section (FILE *script,
			   unsigned int base,
			   unsigned int count,
			   unsigned int ovlynum,
			   unsigned int *ovly_map,
			   asection **ovly_sections,
			   struct bfd_link_info *info)
{
  unsigned int j;

  for (j = base; j < count && ovly_map[j] == ovlynum; j++)
    {
      asection *sec = ovly_sections[2 * j];

      if (fprintf (script, "   %s%c%s (%s)\n",
		   (sec->owner->my_archive != NULL
		    ? bfd_get_filename (sec->owner->my_archive) : ""),
		   info->path_separator,
		   bfd_get_filename (sec->owner),
		   sec->name) <= 0)
	return -1;
      if (sec->segment_mark)
	{
	  struct call_info *call = find_pasted_call (sec);
	  while (call != NULL)
	    {
	      struct function_info *call_fun = call->fun;
	      sec = call_fun->sec;
	      if (fprintf (script, "   %s%c%s (%s)\n",
			   (sec->owner->my_archive != NULL
			    ? bfd_get_filename (sec->owner->my_archive) : ""),
			   info->path_separator,
			   bfd_get_filename (sec->owner),
			   sec->name) <= 0)
		return -1;
	      for (call = call_fun->call_list; call; call = call->next)
		if (call->is_pasted)
		  break;
	    }
	}
    }

  for (j = base; j < count && ovly_map[j] == ovlynum; j++)
    {
      asection *sec = ovly_sections[2 * j + 1];
      if (sec != NULL
	  && fprintf (script, "   %s%c%s (%s)\n",
		      (sec->owner->my_archive != NULL
		       ? bfd_get_filename (sec->owner->my_archive) : ""),
		      info->path_separator,
		      bfd_get_filename (sec->owner),
		      sec->name) <= 0)
	return -1;

      sec = ovly_sections[2 * j];
      if (sec->segment_mark)
	{
	  struct call_info *call = find_pasted_call (sec);
	  while (call != NULL)
	    {
	      struct function_info *call_fun = call->fun;
	      sec = call_fun->rodata;
	      if (sec != NULL
		  && fprintf (script, "   %s%c%s (%s)\n",
			      (sec->owner->my_archive != NULL
			       ? bfd_get_filename (sec->owner->my_archive) : ""),
			      info->path_separator,
			      bfd_get_filename (sec->owner),
			      sec->name) <= 0)
		return -1;
	      for (call = call_fun->call_list; call; call = call->next)
		if (call->is_pasted)
		  break;
	    }
	}
    }

  return j;
}

/* Handle --auto-overlay.  */

static void
spu_elf_auto_overlay (struct bfd_link_info *info)
{
  bfd *ibfd;
  bfd **bfd_arr;
  struct elf_segment_map *m;
  unsigned int fixed_size, lo, hi;
  unsigned int reserved;
  struct spu_link_hash_table *htab;
  unsigned int base, i, count, bfd_count;
  unsigned int region, ovlynum;
  asection **ovly_sections, **ovly_p;
  unsigned int *ovly_map;
  FILE *script;
  unsigned int total_overlay_size, overlay_size;
  const char *ovly_mgr_entry;
  struct elf_link_hash_entry *h;
  struct _mos_param mos_param;
  struct _uos_param uos_param;
  struct function_info dummy_caller;

  /* Find the extents of our loadable image.  */
  lo = (unsigned int) -1;
  hi = 0;
  for (m = elf_seg_map (info->output_bfd); m != NULL; m = m->next)
    if (m->p_type == PT_LOAD)
      for (i = 0; i < m->count; i++)
	if (m->sections[i]->size != 0)
	  {
	    if (m->sections[i]->vma < lo)
	      lo = m->sections[i]->vma;
	    if (m->sections[i]->vma + m->sections[i]->size - 1 > hi)
	      hi = m->sections[i]->vma + m->sections[i]->size - 1;
	  }
  fixed_size = hi + 1 - lo;

  if (!discover_functions (info))
    goto err_exit;

  if (!build_call_tree (info))
    goto err_exit;

  htab = spu_hash_table (info);
  reserved = htab->params->auto_overlay_reserved;
  if (reserved == 0)
    {
      struct _sum_stack_param sum_stack_param;

      sum_stack_param.emit_stack_syms = 0;
      sum_stack_param.overall_stack = 0;
      if (!for_each_node (sum_stack, info, &sum_stack_param, true))
	goto err_exit;
      reserved = (sum_stack_param.overall_stack
		  + htab->params->extra_stack_space);
    }

  /* No need for overlays if everything already fits.  */
  if (fixed_size + reserved <= htab->local_store
      && htab->params->ovly_flavour != ovly_soft_icache)
    {
      htab->params->auto_overlay = 0;
      return;
    }

  uos_param.exclude_input_section = 0;
  uos_param.exclude_output_section
    = bfd_get_section_by_name (info->output_bfd, ".interrupt");

  ovly_mgr_entry = "__ovly_load";
  if (htab->params->ovly_flavour == ovly_soft_icache)
    ovly_mgr_entry = "__icache_br_handler";
  h = elf_link_hash_lookup (&htab->elf, ovly_mgr_entry,
			    false, false, false);
  if (h != NULL
      && (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak)
      && h->def_regular)
    {
      /* We have a user supplied overlay manager.  */
      uos_param.exclude_input_section = h->root.u.def.section;
    }
  else
    {
      /* If no user overlay manager, spu_elf_load_ovl_mgr will add our
	 builtin version to .text, and will adjust .text size.  */
      fixed_size += (*htab->params->spu_elf_load_ovl_mgr) ();
    }

  /* Mark overlay sections, and find max overlay section size.  */
  mos_param.max_overlay_size = 0;
  if (!for_each_node (mark_overlay_section, info, &mos_param, true))
    goto err_exit;

  /* We can't put the overlay manager or interrupt routines in
     overlays.  */
  uos_param.clearing = 0;
  if ((uos_param.exclude_input_section
       || uos_param.exclude_output_section)
      && !for_each_node (unmark_overlay_section, info, &uos_param, true))
    goto err_exit;

  bfd_count = 0;
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    ++bfd_count;
  bfd_arr = bfd_malloc (bfd_count * sizeof (*bfd_arr));
  if (bfd_arr == NULL)
    goto err_exit;

  /* Count overlay sections, and subtract their sizes from "fixed_size".  */
  count = 0;
  bfd_count = 0;
  total_overlay_size = 0;
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      extern const bfd_target spu_elf32_vec;
      asection *sec;
      unsigned int old_count;

      if (ibfd->xvec != &spu_elf32_vec)
	continue;

      old_count = count;
      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	if (sec->linker_mark)
	  {
	    if ((sec->flags & SEC_CODE) != 0)
	      count += 1;
	    fixed_size -= sec->size;
	    total_overlay_size += sec->size;
	  }
	else if ((sec->flags & (SEC_ALLOC | SEC_LOAD)) == (SEC_ALLOC | SEC_LOAD)
		 && sec->output_section->owner == info->output_bfd
		 && startswith (sec->output_section->name, ".ovl.init"))
	  fixed_size -= sec->size;
      if (count != old_count)
	bfd_arr[bfd_count++] = ibfd;
    }

  /* Since the overlay link script selects sections by file name and
     section name, ensure that file names are unique.  */
  if (bfd_count > 1)
    {
      bool ok = true;

      qsort (bfd_arr, bfd_count, sizeof (*bfd_arr), sort_bfds);
      for (i = 1; i < bfd_count; ++i)
	if (filename_cmp (bfd_get_filename (bfd_arr[i - 1]),
			  bfd_get_filename (bfd_arr[i])) == 0)
	  {
	    if (bfd_arr[i - 1]->my_archive == bfd_arr[i]->my_archive)
	      {
		if (bfd_arr[i - 1]->my_archive && bfd_arr[i]->my_archive)
		  /* xgettext:c-format */
		  info->callbacks->einfo (_("%s duplicated in %s\n"),
					  bfd_get_filename (bfd_arr[i]),
					  bfd_get_filename (bfd_arr[i]->my_archive));
		else
		  info->callbacks->einfo (_("%s duplicated\n"),
					  bfd_get_filename (bfd_arr[i]));
		ok = false;
	      }
	  }
      if (!ok)
	{
	  info->callbacks->einfo (_("sorry, no support for duplicate "
				    "object files in auto-overlay script\n"));
	  bfd_set_error (bfd_error_bad_value);
	  goto err_exit;
	}
    }
  free (bfd_arr);

  fixed_size += reserved;
  fixed_size += htab->non_ovly_stub * ovl_stub_size (htab->params);
  if (fixed_size + mos_param.max_overlay_size <= htab->local_store)
    {
      if (htab->params->ovly_flavour == ovly_soft_icache)
	{
	  /* Stubs in the non-icache area are bigger.  */
	  fixed_size += htab->non_ovly_stub * 16;
	  /* Space for icache manager tables.
	     a) Tag array, one quadword per cache line.
	     - word 0: ia address of present line, init to zero.  */
	  fixed_size += 16 << htab->num_lines_log2;
	  /* b) Rewrite "to" list, one quadword per cache line.  */
	  fixed_size += 16 << htab->num_lines_log2;
	  /* c) Rewrite "from" list, one byte per outgoing branch (rounded up
		to a power-of-two number of full quadwords) per cache line.  */
	  fixed_size += 16 << (htab->fromelem_size_log2
			       + htab->num_lines_log2);
	  /* d) Pointer to __ea backing store (toe), 1 quadword.  */
	  fixed_size += 16;
	}
      else
	{
	  /* Guess number of overlays.  Assuming overlay buffer is on
	     average only half full should be conservative.  */
	  ovlynum = (total_overlay_size * 2 * htab->params->num_lines
		     / (htab->local_store - fixed_size));
	  /* Space for _ovly_table[], _ovly_buf_table[] and toe.  */
	  fixed_size += ovlynum * 16 + 16 + 4 + 16;
	}
    }

  if (fixed_size + mos_param.max_overlay_size > htab->local_store)
    /* xgettext:c-format */
    info->callbacks->einfo (_("non-overlay size of 0x%v plus maximum overlay "
			      "size of 0x%v exceeds local store\n"),
			    (bfd_vma) fixed_size,
			    (bfd_vma) mos_param.max_overlay_size);

  /* Now see if we should put some functions in the non-overlay area.  */
  else if (fixed_size < htab->params->auto_overlay_fixed)
    {
      unsigned int max_fixed, lib_size;

      max_fixed = htab->local_store - mos_param.max_overlay_size;
      if (max_fixed > htab->params->auto_overlay_fixed)
	max_fixed = htab->params->auto_overlay_fixed;
      lib_size = max_fixed - fixed_size;
      lib_size = auto_ovl_lib_functions (info, lib_size);
      if (lib_size == (unsigned int) -1)
	goto err_exit;
      fixed_size = max_fixed - lib_size;
    }

  /* Build an array of sections, suitably sorted to place into
     overlays.  */
  ovly_sections = bfd_malloc (2 * count * sizeof (*ovly_sections));
  if (ovly_sections == NULL)
    goto err_exit;
  ovly_p = ovly_sections;
  if (!for_each_node (collect_overlays, info, &ovly_p, true))
    goto err_exit;
  count = (size_t) (ovly_p - ovly_sections) / 2;
  ovly_map = bfd_malloc (count * sizeof (*ovly_map));
  if (ovly_map == NULL)
    goto err_exit;

  memset (&dummy_caller, 0, sizeof (dummy_caller));
  overlay_size = (htab->local_store - fixed_size) / htab->params->num_lines;
  if (htab->params->line_size != 0)
    overlay_size = htab->params->line_size;
  base = 0;
  ovlynum = 0;
  while (base < count)
    {
      unsigned int size = 0, rosize = 0, roalign = 0;

      for (i = base; i < count; i++)
	{
	  asection *sec, *rosec;
	  unsigned int tmp, rotmp;
	  unsigned int num_stubs;
	  struct call_info *call, *pasty;
	  struct _spu_elf_section_data *sec_data;
	  struct spu_elf_stack_info *sinfo;
	  unsigned int k;

	  /* See whether we can add this section to the current
	     overlay without overflowing our overlay buffer.  */
	  sec = ovly_sections[2 * i];
	  tmp = align_power (size, sec->alignment_power) + sec->size;
	  rotmp = rosize;
	  rosec = ovly_sections[2 * i + 1];
	  if (rosec != NULL)
	    {
	      rotmp = align_power (rotmp, rosec->alignment_power) + rosec->size;
	      if (roalign < rosec->alignment_power)
		roalign = rosec->alignment_power;
	    }
	  if (align_power (tmp, roalign) + rotmp > overlay_size)
	    break;
	  if (sec->segment_mark)
	    {
	      /* Pasted sections must stay together, so add their
		 sizes too.  */
	      pasty = find_pasted_call (sec);
	      while (pasty != NULL)
		{
		  struct function_info *call_fun = pasty->fun;
		  tmp = (align_power (tmp, call_fun->sec->alignment_power)
			 + call_fun->sec->size);
		  if (call_fun->rodata)
		    {
		      rotmp = (align_power (rotmp,
					    call_fun->rodata->alignment_power)
			       + call_fun->rodata->size);
		      if (roalign < rosec->alignment_power)
			roalign = rosec->alignment_power;
		    }
		  for (pasty = call_fun->call_list; pasty; pasty = pasty->next)
		    if (pasty->is_pasted)
		      break;
		}
	    }
	  if (align_power (tmp, roalign) + rotmp > overlay_size)
	    break;

	  /* If we add this section, we might need new overlay call
	     stubs.  Add any overlay section calls to dummy_call.  */
	  pasty = NULL;
	  sec_data = spu_elf_section_data (sec);
	  sinfo = sec_data->u.i.stack_info;
	  for (k = 0; k < (unsigned) sinfo->num_fun; ++k)
	    for (call = sinfo->fun[k].call_list; call; call = call->next)
	      if (call->is_pasted)
		{
		  BFD_ASSERT (pasty == NULL);
		  pasty = call;
		}
	      else if (call->fun->sec->linker_mark)
		{
		  if (!copy_callee (&dummy_caller, call))
		    goto err_exit;
		}
	  while (pasty != NULL)
	    {
	      struct function_info *call_fun = pasty->fun;
	      pasty = NULL;
	      for (call = call_fun->call_list; call; call = call->next)
		if (call->is_pasted)
		  {
		    BFD_ASSERT (pasty == NULL);
		    pasty = call;
		  }
		else if (!copy_callee (&dummy_caller, call))
		  goto err_exit;
	    }

	  /* Calculate call stub size.  */
	  num_stubs = 0;
	  for (call = dummy_caller.call_list; call; call = call->next)
	    {
	      unsigned int stub_delta = 1;

	      if (htab->params->ovly_flavour == ovly_soft_icache)
		stub_delta = call->count;
	      num_stubs += stub_delta;

	      /* If the call is within this overlay, we won't need a
		 stub.  */
	      for (k = base; k < i + 1; k++)
		if (call->fun->sec == ovly_sections[2 * k])
		  {
		    num_stubs -= stub_delta;
		    break;
		  }
	    }
	  if (htab->params->ovly_flavour == ovly_soft_icache
	      && num_stubs > htab->params->max_branch)
	    break;
	  if (align_power (tmp, roalign) + rotmp
	      + num_stubs * ovl_stub_size (htab->params) > overlay_size)
	    break;
	  size = tmp;
	  rosize = rotmp;
	}

      if (i == base)
	{
	  /* xgettext:c-format */
	  info->callbacks->einfo (_("%pB:%pA%s exceeds overlay size\n"),
				  ovly_sections[2 * i]->owner,
				  ovly_sections[2 * i],
				  ovly_sections[2 * i + 1] ? " + rodata" : "");
	  bfd_set_error (bfd_error_bad_value);
	  goto err_exit;
	}

      while (dummy_caller.call_list != NULL)
	{
	  struct call_info *call = dummy_caller.call_list;
	  dummy_caller.call_list = call->next;
	  free (call);
	}

      ++ovlynum;
      while (base < i)
	ovly_map[base++] = ovlynum;
    }

  script = htab->params->spu_elf_open_overlay_script ();

  if (htab->params->ovly_flavour == ovly_soft_icache)
    {
      if (fprintf (script, "SECTIONS\n{\n") <= 0)
	goto file_err;

      if (fprintf (script,
		   " . = ALIGN (%u);\n"
		   " .ovl.init : { *(.ovl.init) }\n"
		   " . = ABSOLUTE (ADDR (.ovl.init));\n",
		   htab->params->line_size) <= 0)
	goto file_err;

      base = 0;
      ovlynum = 1;
      while (base < count)
	{
	  unsigned int indx = ovlynum - 1;
	  unsigned int vma, lma;

	  vma = (indx & (htab->params->num_lines - 1)) << htab->line_size_log2;
	  lma = vma + (((indx >> htab->num_lines_log2) + 1) << 18);

	  if (fprintf (script, " .ovly%u ABSOLUTE (ADDR (.ovl.init)) + %u "
			       ": AT (LOADADDR (.ovl.init) + %u) {\n",
		       ovlynum, vma, lma) <= 0)
	    goto file_err;

	  base = print_one_overlay_section (script, base, count, ovlynum,
					    ovly_map, ovly_sections, info);
	  if (base == (unsigned) -1)
	    goto file_err;

	  if (fprintf (script, "  }\n") <= 0)
	    goto file_err;

	  ovlynum++;
	}

      if (fprintf (script, " . = ABSOLUTE (ADDR (.ovl.init)) + %u;\n",
		   1 << (htab->num_lines_log2 + htab->line_size_log2)) <= 0)
	goto file_err;

      if (fprintf (script, "}\nINSERT AFTER .toe;\n") <= 0)
	goto file_err;
    }
  else
    {
      if (fprintf (script, "SECTIONS\n{\n") <= 0)
	goto file_err;

      if (fprintf (script,
		   " . = ALIGN (16);\n"
		   " .ovl.init : { *(.ovl.init) }\n"
		   " . = ABSOLUTE (ADDR (.ovl.init));\n") <= 0)
	goto file_err;

      for (region = 1; region <= htab->params->num_lines; region++)
	{
	  ovlynum = region;
	  base = 0;
	  while (base < count && ovly_map[base] < ovlynum)
	    base++;

	  if (base == count)
	    break;

	  if (region == 1)
	    {
	      /* We need to set lma since we are overlaying .ovl.init.  */
	      if (fprintf (script,
			   " OVERLAY : AT (ALIGN (LOADADDR (.ovl.init) + SIZEOF (.ovl.init), 16))\n {\n") <= 0)
		goto file_err;
	    }
	  else
	    {
	      if (fprintf (script, " OVERLAY :\n {\n") <= 0)
		goto file_err;
	    }

	  while (base < count)
	    {
	      if (fprintf (script, "  .ovly%u {\n", ovlynum) <= 0)
		goto file_err;

	      base = print_one_overlay_section (script, base, count, ovlynum,
						ovly_map, ovly_sections, info);
	      if (base == (unsigned) -1)
		goto file_err;

	      if (fprintf (script, "  }\n") <= 0)
		goto file_err;

	      ovlynum += htab->params->num_lines;
	      while (base < count && ovly_map[base] < ovlynum)
		base++;
	    }

	  if (fprintf (script, " }\n") <= 0)
	    goto file_err;
	}

      if (fprintf (script, "}\nINSERT BEFORE .text;\n") <= 0)
	goto file_err;
    }

  free (ovly_map);
  free (ovly_sections);

  if (fclose (script) != 0)
    goto file_err;

  if (htab->params->auto_overlay & AUTO_RELINK)
    (*htab->params->spu_elf_relink) ();

  xexit (0);

 file_err:
  bfd_set_error (bfd_error_system_call);
 err_exit:
  info->callbacks->einfo (_("%F%P: auto overlay error: %E\n"));
  xexit (1);
}

/* Provide an estimate of total stack required.  */

static bool
spu_elf_stack_analysis (struct bfd_link_info *info)
{
  struct spu_link_hash_table *htab;
  struct _sum_stack_param sum_stack_param;

  if (!discover_functions (info))
    return false;

  if (!build_call_tree (info))
    return false;

  htab = spu_hash_table (info);
  if (htab->params->stack_analysis)
    {
      info->callbacks->info (_("Stack size for call graph root nodes.\n"));
      info->callbacks->minfo (_("\nStack size for functions.  "
				"Annotations: '*' max stack, 't' tail call\n"));
    }

  sum_stack_param.emit_stack_syms = htab->params->emit_stack_syms;
  sum_stack_param.overall_stack = 0;
  if (!for_each_node (sum_stack, info, &sum_stack_param, true))
    return false;

  if (htab->params->stack_analysis)
    info->callbacks->info (_("Maximum stack required is 0x%v\n"),
			   (bfd_vma) sum_stack_param.overall_stack);
  return true;
}

/* Perform a final link.  */

static bool
spu_elf_final_link (bfd *output_bfd, struct bfd_link_info *info)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);

  if (htab->params->auto_overlay)
    spu_elf_auto_overlay (info);

  if ((htab->params->stack_analysis
       || (htab->params->ovly_flavour == ovly_soft_icache
	   && htab->params->lrlive_analysis))
      && !spu_elf_stack_analysis (info))
    info->callbacks->einfo (_("%X%P: stack/lrlive analysis error: %E\n"));

  if (!spu_elf_build_stubs (info))
    info->callbacks->einfo (_("%F%P: can not build overlay stubs: %E\n"));

  return bfd_elf_final_link (output_bfd, info);
}

/* Called when not normally emitting relocs, ie. !bfd_link_relocatable (info)
   and !info->emitrelocations.  Returns a count of special relocs
   that need to be emitted.  */

static unsigned int
spu_elf_count_relocs (struct bfd_link_info *info, asection *sec)
{
  Elf_Internal_Rela *relocs;
  unsigned int count = 0;

  relocs = _bfd_elf_link_read_relocs (sec->owner, sec, NULL, NULL,
				      info->keep_memory);
  if (relocs != NULL)
    {
      Elf_Internal_Rela *rel;
      Elf_Internal_Rela *relend = relocs + sec->reloc_count;

      for (rel = relocs; rel < relend; rel++)
	{
	  int r_type = ELF32_R_TYPE (rel->r_info);
	  if (r_type == R_SPU_PPU32 || r_type == R_SPU_PPU64)
	    ++count;
	}

      if (elf_section_data (sec)->relocs != relocs)
	free (relocs);
    }

  return count;
}

/* Functions for adding fixup records to .fixup */

#define FIXUP_RECORD_SIZE 4

#define FIXUP_PUT(output_bfd,htab,index,addr) \
	  bfd_put_32 (output_bfd, addr, \
		      htab->sfixup->contents + FIXUP_RECORD_SIZE * (index))
#define FIXUP_GET(output_bfd,htab,index) \
	  bfd_get_32 (output_bfd, \
		      htab->sfixup->contents + FIXUP_RECORD_SIZE * (index))

/* Store OFFSET in .fixup.  This assumes it will be called with an
   increasing OFFSET.  When this OFFSET fits with the last base offset,
   it just sets a bit, otherwise it adds a new fixup record.  */
static void
spu_elf_emit_fixup (bfd * output_bfd, struct bfd_link_info *info,
		    bfd_vma offset)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);
  asection *sfixup = htab->sfixup;
  bfd_vma qaddr = offset & ~(bfd_vma) 15;
  bfd_vma bit = ((bfd_vma) 8) >> ((offset & 15) >> 2);
  if (sfixup->reloc_count == 0)
    {
      FIXUP_PUT (output_bfd, htab, 0, qaddr | bit);
      sfixup->reloc_count++;
    }
  else
    {
      bfd_vma base = FIXUP_GET (output_bfd, htab, sfixup->reloc_count - 1);
      if (qaddr != (base & ~(bfd_vma) 15))
	{
	  if ((sfixup->reloc_count + 1) * FIXUP_RECORD_SIZE > sfixup->size)
	    _bfd_error_handler (_("fatal error while creating .fixup"));
	  FIXUP_PUT (output_bfd, htab, sfixup->reloc_count, qaddr | bit);
	  sfixup->reloc_count++;
	}
      else
	FIXUP_PUT (output_bfd, htab, sfixup->reloc_count - 1, base | bit);
    }
}

/* Apply RELOCS to CONTENTS of INPUT_SECTION from INPUT_BFD.  */

static int
spu_elf_relocate_section (bfd *output_bfd,
			  struct bfd_link_info *info,
			  bfd *input_bfd,
			  asection *input_section,
			  bfd_byte *contents,
			  Elf_Internal_Rela *relocs,
			  Elf_Internal_Sym *local_syms,
			  asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel, *relend;
  struct spu_link_hash_table *htab;
  asection *ea;
  int ret = true;
  bool emit_these_relocs = false;
  bool is_ea_sym;
  bool stubs;
  unsigned int iovl = 0;

  htab = spu_hash_table (info);
  stubs = (htab->stub_sec != NULL
	   && maybe_needs_stubs (input_section));
  iovl = overlay_index (input_section);
  ea = bfd_get_section_by_name (output_bfd, "._ea");
  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = (struct elf_link_hash_entry **) (elf_sym_hashes (input_bfd));

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int r_type;
      reloc_howto_type *howto;
      unsigned int r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      const char *sym_name;
      bfd_vma relocation;
      bfd_vma addend;
      bfd_reloc_status_type r;
      bool unresolved_reloc;
      enum _stub_type stub_type;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);
      howto = elf_howto_table + r_type;
      unresolved_reloc = false;
      h = NULL;
      sym = NULL;
      sec = NULL;
      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  sym_name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym, sec);
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}
      else
	{
	  if (sym_hashes == NULL)
	    return false;

	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];

	  if (info->wrap_hash != NULL
	      && (input_section->flags & SEC_DEBUGGING) != 0)
	    h = ((struct elf_link_hash_entry *)
		 unwrap_hash_lookup (info, input_bfd, &h->root));

	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  relocation = 0;
	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    {
	      sec = h->root.u.def.section;
	      if (sec == NULL
		  || sec->output_section == NULL)
		/* Set a flag that will be cleared later if we find a
		   relocation value for this symbol.  output_section
		   is typically NULL for symbols satisfied by a shared
		   library.  */
		unresolved_reloc = true;
	      else
		relocation = (h->root.u.def.value
			      + sec->output_section->vma
			      + sec->output_offset);
	    }
	  else if (h->root.type == bfd_link_hash_undefweak)
	    ;
	  else if (info->unresolved_syms_in_objects == RM_IGNORE
		   && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
	    ;
	  else if (!bfd_link_relocatable (info)
		   && !(r_type == R_SPU_PPU32 || r_type == R_SPU_PPU64))
	    {
	      bool err;

	      err = (info->unresolved_syms_in_objects == RM_DIAGNOSE
		     && !info->warn_unresolved_syms)
		|| ELF_ST_VISIBILITY (h->other) != STV_DEFAULT;

	      info->callbacks->undefined_symbol
		(info, h->root.root.string, input_bfd,
		 input_section, rel->r_offset, err);
	    }
	  sym_name = h->root.root.string;
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      /* Change "a rt,ra,rb" to "ai rt,ra,0". */
      if (r_type == R_SPU_ADD_PIC
	  && h != NULL
	  && !(h->def_regular || ELF_COMMON_DEF_P (h)))
	{
	  bfd_byte *loc = contents + rel->r_offset;
	  loc[0] = 0x1c;
	  loc[1] = 0x00;
	  loc[2] &= 0x3f;
	}

      is_ea_sym = (ea != NULL
		   && sec != NULL
		   && sec->output_section == ea);

      /* If this symbol is in an overlay area, we may need to relocate
	 to the overlay stub.  */
      addend = rel->r_addend;
      if (stubs
	  && !is_ea_sym
	  && (stub_type = needs_ovl_stub (h, sym, sec, input_section, rel,
					  contents, info)) != no_stub)
	{
	  unsigned int ovl = 0;
	  struct got_entry *g, **head;

	  if (stub_type != nonovl_stub)
	    ovl = iovl;

	  if (h != NULL)
	    head = &h->got.glist;
	  else
	    head = elf_local_got_ents (input_bfd) + r_symndx;

	  for (g = *head; g != NULL; g = g->next)
	    if (htab->params->ovly_flavour == ovly_soft_icache
		? (g->ovl == ovl
		   && g->br_addr == (rel->r_offset
				     + input_section->output_offset
				     + input_section->output_section->vma))
		: g->addend == addend && (g->ovl == ovl || g->ovl == 0))
	      break;
	  if (g == NULL)
	    abort ();

	  relocation = g->stub_addr;
	  addend = 0;
	}
      else
	{
	  /* For soft icache, encode the overlay index into addresses.  */
	  if (htab->params->ovly_flavour == ovly_soft_icache
	      && (r_type == R_SPU_ADDR16_HI
		  || r_type == R_SPU_ADDR32 || r_type == R_SPU_REL32)
	      && !is_ea_sym)
	    {
	      unsigned int ovl = overlay_index (sec);
	      if (ovl != 0)
		{
		  unsigned int set_id = ((ovl - 1) >> htab->num_lines_log2) + 1;
		  relocation += set_id << 18;
		}
	    }
	}

      if (htab->params->emit_fixups && !bfd_link_relocatable (info)
	  && (input_section->flags & SEC_ALLOC) != 0
	  && r_type == R_SPU_ADDR32)
	{
	  bfd_vma offset;
	  offset = rel->r_offset + input_section->output_section->vma
		   + input_section->output_offset;
	  spu_elf_emit_fixup (output_bfd, info, offset);
	}

      if (unresolved_reloc)
	;
      else if (r_type == R_SPU_PPU32 || r_type == R_SPU_PPU64)
	{
	  if (is_ea_sym)
	    {
	      /* ._ea is a special section that isn't allocated in SPU
		 memory, but rather occupies space in PPU memory as
		 part of an embedded ELF image.  If this reloc is
		 against a symbol defined in ._ea, then transform the
		 reloc into an equivalent one without a symbol
		 relative to the start of the ELF image.  */
	      rel->r_addend += (relocation
				- ea->vma
				+ elf_section_data (ea)->this_hdr.sh_offset);
	      rel->r_info = ELF32_R_INFO (0, r_type);
	    }
	  emit_these_relocs = true;
	  continue;
	}
      else if (is_ea_sym)
	unresolved_reloc = true;

      if (unresolved_reloc
	  && _bfd_elf_section_offset (output_bfd, info, input_section,
				      rel->r_offset) != (bfd_vma) -1)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB(%s+%#" PRIx64 "): "
	       "unresolvable %s relocation against symbol `%s'"),
	     input_bfd,
	     bfd_section_name (input_section),
	     (uint64_t) rel->r_offset,
	     howto->name,
	     sym_name);
	  ret = false;
	}

      r = _bfd_final_link_relocate (howto,
				    input_bfd,
				    input_section,
				    contents,
				    rel->r_offset, relocation, addend);

      if (r != bfd_reloc_ok)
	{
	  const char *msg = (const char *) 0;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow)
		(info, (h ? &h->root : NULL), sym_name, howto->name,
		 (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, sym_name, input_bfd, input_section, rel->r_offset, true);
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

	    default:
	      msg = _("internal error: unknown error");
	      /* fall through */

	    common_error:
	      ret = false;
	      (*info->callbacks->warning) (info, msg, sym_name, input_bfd,
					   input_section, rel->r_offset);
	      break;
	    }
	}
    }

  if (ret
      && emit_these_relocs
      && !info->emitrelocations)
    {
      Elf_Internal_Rela *wrel;
      Elf_Internal_Shdr *rel_hdr;

      wrel = rel = relocs;
      relend = relocs + input_section->reloc_count;
      for (; rel < relend; rel++)
	{
	  int r_type;

	  r_type = ELF32_R_TYPE (rel->r_info);
	  if (r_type == R_SPU_PPU32 || r_type == R_SPU_PPU64)
	    *wrel++ = *rel;
	}
      input_section->reloc_count = wrel - relocs;
      /* Backflips for _bfd_elf_link_output_relocs.  */
      rel_hdr = _bfd_elf_single_rel_hdr (input_section);
      rel_hdr->sh_size = input_section->reloc_count * rel_hdr->sh_entsize;
      ret = 2;
    }

  return ret;
}

static bool
spu_elf_finish_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				 struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return true;
}

/* Adjust _SPUEAR_ syms to point at their overlay stubs.  */

static int
spu_elf_output_symbol_hook (struct bfd_link_info *info,
			    const char *sym_name ATTRIBUTE_UNUSED,
			    Elf_Internal_Sym *sym,
			    asection *sym_sec ATTRIBUTE_UNUSED,
			    struct elf_link_hash_entry *h)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);

  if (!bfd_link_relocatable (info)
      && htab->stub_sec != NULL
      && h != NULL
      && (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak)
      && h->def_regular
      && startswith (h->root.root.string, "_SPUEAR_"))
    {
      struct got_entry *g;

      for (g = h->got.glist; g != NULL; g = g->next)
	if (htab->params->ovly_flavour == ovly_soft_icache
	    ? g->br_addr == g->stub_addr
	    : g->addend == 0 && g->ovl == 0)
	  {
	    sym->st_shndx = (_bfd_elf_section_from_bfd_section
			     (htab->stub_sec[0]->output_section->owner,
			      htab->stub_sec[0]->output_section));
	    sym->st_value = g->stub_addr;
	    break;
	  }
    }

  return 1;
}

static int spu_plugin = 0;

void
spu_elf_plugin (int val)
{
  spu_plugin = val;
}

/* Set ELF header e_type for plugins.  */

static bool
spu_elf_init_file_header (bfd *abfd, struct bfd_link_info *info)
{
  if (!_bfd_elf_init_file_header (abfd, info))
    return false;

  if (spu_plugin)
    {
      Elf_Internal_Ehdr *i_ehdrp = elf_elfheader (abfd);

      i_ehdrp->e_type = ET_DYN;
    }
  return true;
}

/* We may add an extra PT_LOAD segment for .toe.  We also need extra
   segments for overlays.  */

static int
spu_elf_additional_program_headers (bfd *abfd, struct bfd_link_info *info)
{
  int extra = 0;
  asection *sec;

  if (info != NULL)
    {
      struct spu_link_hash_table *htab = spu_hash_table (info);
      extra = htab->num_overlays;
    }

  if (extra)
    ++extra;

  sec = bfd_get_section_by_name (abfd, ".toe");
  if (sec != NULL && (sec->flags & SEC_LOAD) != 0)
    ++extra;

  return extra;
}

/* Remove .toe section from other PT_LOAD segments and put it in
   a segment of its own.  Put overlays in separate segments too.  */

static bool
spu_elf_modify_segment_map (bfd *abfd, struct bfd_link_info *info)
{
  asection *toe, *s;
  struct elf_segment_map *m, *m_overlay;
  struct elf_segment_map **p, **p_overlay, **first_load;
  unsigned int i;

  if (info == NULL)
    return true;

  toe = bfd_get_section_by_name (abfd, ".toe");
  for (m = elf_seg_map (abfd); m != NULL; m = m->next)
    if (m->p_type == PT_LOAD && m->count > 1)
      for (i = 0; i < m->count; i++)
	if ((s = m->sections[i]) == toe
	    || spu_elf_section_data (s)->u.o.ovl_index != 0)
	  {
	    struct elf_segment_map *m2;
	    bfd_vma amt;

	    if (i + 1 < m->count)
	      {
		amt = sizeof (struct elf_segment_map);
		amt += (m->count - (i + 2)) * sizeof (m->sections[0]);
		m2 = bfd_zalloc (abfd, amt);
		if (m2 == NULL)
		  return false;
		m2->count = m->count - (i + 1);
		memcpy (m2->sections, m->sections + i + 1,
			m2->count * sizeof (m->sections[0]));
		m2->p_type = PT_LOAD;
		m2->next = m->next;
		m->next = m2;
	      }
	    m->count = 1;
	    if (i != 0)
	      {
		m->count = i;
		amt = sizeof (struct elf_segment_map);
		m2 = bfd_zalloc (abfd, amt);
		if (m2 == NULL)
		  return false;
		m2->p_type = PT_LOAD;
		m2->count = 1;
		m2->sections[0] = s;
		m2->next = m->next;
		m->next = m2;
	      }
	    break;
	  }


  /* Some SPU ELF loaders ignore the PF_OVERLAY flag and just load all
     PT_LOAD segments.  This can cause the .ovl.init section to be
     overwritten with the contents of some overlay segment.  To work
     around this issue, we ensure that all PF_OVERLAY segments are
     sorted first amongst the program headers; this ensures that even
     with a broken loader, the .ovl.init section (which is not marked
     as PF_OVERLAY) will be placed into SPU local store on startup.  */

  /* Move all overlay segments onto a separate list.  */
  p = &elf_seg_map (abfd);
  p_overlay = &m_overlay;
  m_overlay = NULL;
  first_load = NULL;
  while (*p != NULL)
    {
      if ((*p)->p_type == PT_LOAD)
	{
	  if (!first_load)
	    first_load = p;
	  if ((*p)->count == 1
	      && spu_elf_section_data ((*p)->sections[0])->u.o.ovl_index != 0)
	    {
	      m = *p;
	      m->no_sort_lma = 1;
	      *p = m->next;
	      *p_overlay = m;
	      p_overlay = &m->next;
	      continue;
	    }
	}
      p = &((*p)->next);
    }

  /* Re-insert overlay segments at the head of the segment map.  */
  if (m_overlay != NULL)
    {
      p = first_load;
      if (*p != NULL && (*p)->p_type == PT_LOAD && (*p)->includes_filehdr)
	/* It doesn't really make sense for someone to include the ELF
	   file header into an spu image, but if they do the code that
	   assigns p_offset needs to see the segment containing the
	   header first.  */
	p = &(*p)->next;
      *p_overlay = *p;
      *p = m_overlay;
    }

  return true;
}

/* Tweak the section type of .note.spu_name.  */

static bool
spu_elf_fake_sections (bfd *obfd ATTRIBUTE_UNUSED,
		       Elf_Internal_Shdr *hdr,
		       asection *sec)
{
  if (strcmp (sec->name, SPU_PTNOTE_SPUNAME) == 0)
    hdr->sh_type = SHT_NOTE;
  return true;
}

/* Tweak phdrs before writing them out.  */

static bool
spu_elf_modify_headers (bfd *abfd, struct bfd_link_info *info)
{
  if (info != NULL)
    {
      const struct elf_backend_data *bed;
      struct elf_obj_tdata *tdata;
      Elf_Internal_Phdr *phdr, *last;
      struct spu_link_hash_table *htab;
      unsigned int count;
      unsigned int i;

      bed = get_elf_backend_data (abfd);
      tdata = elf_tdata (abfd);
      phdr = tdata->phdr;
      count = elf_program_header_size (abfd) / bed->s->sizeof_phdr;
      htab = spu_hash_table (info);
      if (htab->num_overlays != 0)
	{
	  struct elf_segment_map *m;
	  unsigned int o;

	  for (i = 0, m = elf_seg_map (abfd); m; ++i, m = m->next)
	    if (m->count != 0
		&& ((o = spu_elf_section_data (m->sections[0])->u.o.ovl_index)
		    != 0))
	      {
		/* Mark this as an overlay header.  */
		phdr[i].p_flags |= PF_OVERLAY;

		if (htab->ovtab != NULL && htab->ovtab->size != 0
		    && htab->params->ovly_flavour != ovly_soft_icache)
		  {
		    bfd_byte *p = htab->ovtab->contents;
		    unsigned int off = o * 16 + 8;

		    /* Write file_off into _ovly_table.  */
		    bfd_put_32 (htab->ovtab->owner, phdr[i].p_offset, p + off);
		  }
	      }
	  /* Soft-icache has its file offset put in .ovl.init.  */
	  if (htab->init != NULL && htab->init->size != 0)
	    {
	      bfd_vma val
		= elf_section_data (htab->ovl_sec[0])->this_hdr.sh_offset;

	      bfd_put_32 (htab->init->owner, val, htab->init->contents + 4);
	    }
	}

      /* Round up p_filesz and p_memsz of PT_LOAD segments to multiples
	 of 16.  This should always be possible when using the standard
	 linker scripts, but don't create overlapping segments if
	 someone is playing games with linker scripts.  */
      last = NULL;
      for (i = count; i-- != 0; )
	if (phdr[i].p_type == PT_LOAD)
	  {
	    unsigned adjust;

	    adjust = -phdr[i].p_filesz & 15;
	    if (adjust != 0
		&& last != NULL
		&& (phdr[i].p_offset + phdr[i].p_filesz
		    > last->p_offset - adjust))
	      break;

	    adjust = -phdr[i].p_memsz & 15;
	    if (adjust != 0
		&& last != NULL
		&& phdr[i].p_filesz != 0
		&& phdr[i].p_vaddr + phdr[i].p_memsz > last->p_vaddr - adjust
		&& phdr[i].p_vaddr + phdr[i].p_memsz <= last->p_vaddr)
	      break;

	    if (phdr[i].p_filesz != 0)
	      last = &phdr[i];
	  }

      if (i == (unsigned int) -1)
	for (i = count; i-- != 0; )
	  if (phdr[i].p_type == PT_LOAD)
	    {
	      unsigned adjust;

	      adjust = -phdr[i].p_filesz & 15;
	      phdr[i].p_filesz += adjust;

	      adjust = -phdr[i].p_memsz & 15;
	      phdr[i].p_memsz += adjust;
	    }
    }

  return _bfd_elf_modify_headers (abfd, info);
}

bool
spu_elf_size_sections (bfd *obfd ATTRIBUTE_UNUSED, struct bfd_link_info *info)
{
  struct spu_link_hash_table *htab = spu_hash_table (info);
  if (htab->params->emit_fixups)
    {
      asection *sfixup = htab->sfixup;
      int fixup_count = 0;
      bfd *ibfd;
      size_t size;

      for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
	{
	  asection *isec;

	  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	    continue;

	  /* Walk over each section attached to the input bfd.  */
	  for (isec = ibfd->sections; isec != NULL; isec = isec->next)
	    {
	      Elf_Internal_Rela *internal_relocs, *irelaend, *irela;
	      bfd_vma base_end;

	      /* If there aren't any relocs, then there's nothing more
		 to do.  */
	      if ((isec->flags & SEC_ALLOC) == 0
		  || (isec->flags & SEC_RELOC) == 0
		  || isec->reloc_count == 0)
		continue;

	      /* Get the relocs.  */
	      internal_relocs =
		_bfd_elf_link_read_relocs (ibfd, isec, NULL, NULL,
					   info->keep_memory);
	      if (internal_relocs == NULL)
		return false;

	      /* 1 quadword can contain up to 4 R_SPU_ADDR32
		 relocations.  They are stored in a single word by
		 saving the upper 28 bits of the address and setting the
		 lower 4 bits to a bit mask of the words that have the
		 relocation.  BASE_END keeps track of the next quadword. */
	      irela = internal_relocs;
	      irelaend = irela + isec->reloc_count;
	      base_end = 0;
	      for (; irela < irelaend; irela++)
		if (ELF32_R_TYPE (irela->r_info) == R_SPU_ADDR32
		    && irela->r_offset >= base_end)
		  {
		    base_end = (irela->r_offset & ~(bfd_vma) 15) + 16;
		    fixup_count++;
		  }
	    }
	}

      /* We always have a NULL fixup as a sentinel */
      size = (fixup_count + 1) * FIXUP_RECORD_SIZE;
      if (!bfd_set_section_size (sfixup, size))
	return false;
      sfixup->contents = (bfd_byte *) bfd_zalloc (info->input_bfds, size);
      if (sfixup->contents == NULL)
	return false;
    }
  return true;
}

#define TARGET_BIG_SYM		spu_elf32_vec
#define TARGET_BIG_NAME		"elf32-spu"
#define ELF_ARCH		bfd_arch_spu
#define ELF_TARGET_ID		SPU_ELF_DATA
#define ELF_MACHINE_CODE	EM_SPU
/* This matches the alignment need for DMA.  */
#define ELF_MAXPAGESIZE		0x80
#define elf_backend_rela_normal		1
#define elf_backend_can_gc_sections	1

#define bfd_elf32_bfd_reloc_type_lookup		spu_elf_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		spu_elf_reloc_name_lookup
#define elf_info_to_howto			spu_elf_info_to_howto
#define elf_backend_count_relocs		spu_elf_count_relocs
#define elf_backend_relocate_section		spu_elf_relocate_section
#define elf_backend_finish_dynamic_sections	spu_elf_finish_dynamic_sections
#define elf_backend_symbol_processing		spu_elf_backend_symbol_processing
#define elf_backend_link_output_symbol_hook	spu_elf_output_symbol_hook
#define elf_backend_object_p			spu_elf_object_p
#define bfd_elf32_new_section_hook		spu_elf_new_section_hook
#define bfd_elf32_bfd_link_hash_table_create	spu_elf_link_hash_table_create

#define elf_backend_additional_program_headers	spu_elf_additional_program_headers
#define elf_backend_modify_segment_map		spu_elf_modify_segment_map
#define elf_backend_modify_headers		spu_elf_modify_headers
#define elf_backend_init_file_header		spu_elf_init_file_header
#define elf_backend_fake_sections		spu_elf_fake_sections
#define elf_backend_special_sections		spu_elf_special_sections
#define bfd_elf32_bfd_final_link		spu_elf_final_link

#include "elf32-target.h"
