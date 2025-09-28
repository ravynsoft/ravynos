/* LoongArch-specific support for NN-bit ELF.
   Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Loongson Ltd.

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
   along with this program; see the file COPYING3.  If not,
   see <http://www.gnu.org/licenses/>.  */

#include "ansidecl.h"
#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#define ARCH_SIZE NN
#include "elf-bfd.h"
#include "objalloc.h"
#include "elf/loongarch.h"
#include "elfxx-loongarch.h"
#include "opcode/loongarch.h"

static bool
loongarch_info_to_howto_rela (bfd *abfd, arelent *cache_ptr,
			      Elf_Internal_Rela *dst)
{
  cache_ptr->howto = loongarch_elf_rtype_to_howto (abfd,
						   ELFNN_R_TYPE (dst->r_info));
  return cache_ptr->howto != NULL;
}

/* LoongArch ELF linker hash entry.  */
struct loongarch_elf_link_hash_entry
{
  struct elf_link_hash_entry elf;

#define GOT_UNKNOWN 0
#define GOT_NORMAL  1
#define GOT_TLS_GD  2
#define GOT_TLS_IE  4
#define GOT_TLS_LE  8
  char tls_type;
};

#define loongarch_elf_hash_entry(ent)	\
  ((struct loongarch_elf_link_hash_entry *) (ent))

struct _bfd_loongarch_elf_obj_tdata
{
  struct elf_obj_tdata root;

  /* The tls_type for each local got entry.  */
  char *local_got_tls_type;
};

#define _bfd_loongarch_elf_tdata(abfd)	\
  ((struct _bfd_loongarch_elf_obj_tdata *) (abfd)->tdata.any)

#define _bfd_loongarch_elf_local_got_tls_type(abfd)	\
  (_bfd_loongarch_elf_tdata (abfd)->local_got_tls_type)

#define _bfd_loongarch_elf_tls_type(abfd, h, symndx)			\
  (*((h) != NULL							\
     ? &loongarch_elf_hash_entry (h)->tls_type				\
     : &_bfd_loongarch_elf_local_got_tls_type (abfd)[symndx]))

#define is_loongarch_elf(bfd)						\
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour			\
   && elf_tdata (bfd) != NULL						\
   && elf_object_id (bfd) == LARCH_ELF_DATA)

struct loongarch_elf_link_hash_table
{
  struct elf_link_hash_table elf;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *sdyntdata;

  /* Small local sym to section mapping cache.  */
  struct sym_cache sym_cache;

  /* Used by local STT_GNU_IFUNC symbols.  */
  htab_t loc_hash_table;
  void *loc_hash_memory;

  /* The max alignment of output sections.  */
  bfd_vma max_alignment;

  /* The data segment phase, don't relax the section
     when it is exp_seg_relro_adjust.  */
  int *data_segment_phase;
};

/* Get the LoongArch ELF linker hash table from a link_info structure.  */
#define loongarch_elf_hash_table(p)					\
  (elf_hash_table_id (elf_hash_table (p)) == LARCH_ELF_DATA		\
   ? ((struct loongarch_elf_link_hash_table *) ((p)->hash))		\
   : NULL)

#define MINUS_ONE ((bfd_vma) 0 - 1)

#define sec_addr(sec) ((sec)->output_section->vma + (sec)->output_offset)

#define LARCH_ELF_LOG_WORD_BYTES (ARCH_SIZE == 32 ? 2 : 3)
#define LARCH_ELF_WORD_BYTES (1 << LARCH_ELF_LOG_WORD_BYTES)

#define PLT_HEADER_INSNS 8
#define PLT_HEADER_SIZE (PLT_HEADER_INSNS * 4)

#define PLT_ENTRY_INSNS 4
#define PLT_ENTRY_SIZE (PLT_ENTRY_INSNS * 4)

#define GOT_ENTRY_SIZE (LARCH_ELF_WORD_BYTES)

#define GOTPLT_HEADER_SIZE (GOT_ENTRY_SIZE * 2)

#define elf_backend_want_got_plt 1

#define elf_backend_plt_readonly 1

#define elf_backend_want_plt_sym 1
#define elf_backend_plt_alignment 4
#define elf_backend_can_gc_sections 1
#define elf_backend_can_refcount 1
#define elf_backend_want_got_sym 1

#define elf_backend_got_header_size (GOT_ENTRY_SIZE * 1)

#define elf_backend_want_dynrelro 1
#define elf_backend_rela_normal 1
#define elf_backend_default_execstack 0

/* Generate a PLT header.  */

static bool
loongarch_make_plt_header (bfd_vma got_plt_addr, bfd_vma plt_header_addr,
			   uint32_t *entry)
{
  bfd_vma pcrel = got_plt_addr - plt_header_addr;
  bfd_vma hi, lo;

  if (pcrel + 0x80000800 > 0xffffffff)
    {
      _bfd_error_handler (_("%#" PRIx64 " invaild imm"), (uint64_t) pcrel);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  hi = ((pcrel + 0x800) >> 12) & 0xfffff;
  lo = pcrel & 0xfff;

  /* pcaddu12i  $t2, %hi(%pcrel(.got.plt))
     sub.[wd]   $t1, $t1, $t3
     ld.[wd]    $t3, $t2, %lo(%pcrel(.got.plt)) # _dl_runtime_resolve
     addi.[wd]  $t1, $t1, -(PLT_HEADER_SIZE + 12)
     addi.[wd]  $t0, $t2, %lo(%pcrel(.got.plt))
     srli.[wd]  $t1, $t1, log2(16 / GOT_ENTRY_SIZE)
     ld.[wd]    $t0, $t0, GOT_ENTRY_SIZE
     jirl   $r0, $t3, 0 */

  if (GOT_ENTRY_SIZE == 8)
    {
      entry[0] = 0x1c00000e | (hi & 0xfffff) << 5;
      entry[1] = 0x0011bdad;
      entry[2] = 0x28c001cf | (lo & 0xfff) << 10;
      entry[3] = 0x02c001ad | ((-(PLT_HEADER_SIZE + 12)) & 0xfff) << 10;
      entry[4] = 0x02c001cc | (lo & 0xfff) << 10;
      entry[5] = 0x004501ad | (4 - LARCH_ELF_LOG_WORD_BYTES) << 10;
      entry[6] = 0x28c0018c | GOT_ENTRY_SIZE << 10;
      entry[7] = 0x4c0001e0;
    }
  else
    {
      entry[0] = 0x1c00000e | (hi & 0xfffff) << 5;
      entry[1] = 0x00113dad;
      entry[2] = 0x288001cf | (lo & 0xfff) << 10;
      entry[3] = 0x028001ad | ((-(PLT_HEADER_SIZE + 12)) & 0xfff) << 10;
      entry[4] = 0x028001cc | (lo & 0xfff) << 10;
      entry[5] = 0x004481ad | (4 - LARCH_ELF_LOG_WORD_BYTES) << 10;
      entry[6] = 0x2880018c | GOT_ENTRY_SIZE << 10;
      entry[7] = 0x4c0001e0;
    }
  return true;
}

/* Generate a PLT entry.  */

static bool
loongarch_make_plt_entry (bfd_vma got_plt_entry_addr, bfd_vma plt_entry_addr,
			  uint32_t *entry)
{
  bfd_vma pcrel = got_plt_entry_addr - plt_entry_addr;
  bfd_vma hi, lo;

  if (pcrel + 0x80000800 > 0xffffffff)
    {
      _bfd_error_handler (_("%#" PRIx64 " invaild imm"), (uint64_t) pcrel);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  hi = ((pcrel + 0x800) >> 12) & 0xfffff;
  lo = pcrel & 0xfff;

  entry[0] = 0x1c00000f | (hi & 0xfffff) << 5;
  entry[1] = ((GOT_ENTRY_SIZE == 8 ? 0x28c001ef : 0x288001ef)
	      | (lo & 0xfff) << 10);
  entry[2] = 0x4c0001ed;	/* jirl $r13, $15, 0 */
  entry[3] = 0x03400000;	/* nop */

  return true;
}

/* Create an entry in an LoongArch ELF linker hash table.  */

static struct bfd_hash_entry *
link_hash_newfunc (struct bfd_hash_entry *entry, struct bfd_hash_table *table,
		   const char *string)
{
  struct loongarch_elf_link_hash_entry *eh;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table, sizeof (*eh));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      eh = (struct loongarch_elf_link_hash_entry *) entry;
      eh->tls_type = GOT_UNKNOWN;
    }

  return entry;
}

/* Compute a hash of a local hash entry.  We use elf_link_hash_entry
  for local symbol so that we can handle local STT_GNU_IFUNC symbols
  as global symbol.  We reuse indx and dynstr_index for local symbol
  hash since they aren't used by global symbols in this backend.  */

static hashval_t
elfNN_loongarch_local_htab_hash (const void *ptr)
{
  struct elf_link_hash_entry *h = (struct elf_link_hash_entry *) ptr;
  return ELF_LOCAL_SYMBOL_HASH (h->indx, h->dynstr_index);
}

/* Compare local hash entries.  */

static int
elfNN_loongarch_local_htab_eq (const void *ptr1, const void *ptr2)
{
  struct elf_link_hash_entry *h1 = (struct elf_link_hash_entry *) ptr1;
  struct elf_link_hash_entry *h2 = (struct elf_link_hash_entry *) ptr2;

  return h1->indx == h2->indx && h1->dynstr_index == h2->dynstr_index;
}

/* Find and/or create a hash entry for local symbol.  */
static struct elf_link_hash_entry *
elfNN_loongarch_get_local_sym_hash (struct loongarch_elf_link_hash_table *htab,
				    bfd *abfd, const Elf_Internal_Rela *rel,
				    bool create)
{
  struct loongarch_elf_link_hash_entry e, *ret;
  asection *sec = abfd->sections;
  hashval_t h = ELF_LOCAL_SYMBOL_HASH (sec->id, ELFNN_R_SYM (rel->r_info));
  void **slot;

  e.elf.indx = sec->id;
  e.elf.dynstr_index = ELFNN_R_SYM (rel->r_info);
  slot = htab_find_slot_with_hash (htab->loc_hash_table, &e, h,
				   create ? INSERT : NO_INSERT);

  if (!slot)
    return NULL;

  if (*slot)
    {
      ret = (struct loongarch_elf_link_hash_entry *) *slot;
      return &ret->elf;
    }

  ret = ((struct loongarch_elf_link_hash_entry *)
	 objalloc_alloc ((struct objalloc *) htab->loc_hash_memory,
			 sizeof (struct loongarch_elf_link_hash_entry)));
  if (ret)
    {
      memset (ret, 0, sizeof (*ret));
      ret->elf.indx = sec->id;
      ret->elf.pointer_equality_needed = 0;
      ret->elf.dynstr_index = ELFNN_R_SYM (rel->r_info);
      ret->elf.dynindx = -1;
      ret->elf.needs_plt = 0;
      ret->elf.plt.refcount = -1;
      ret->elf.got.refcount = -1;
      ret->elf.def_dynamic = 0;
      ret->elf.def_regular = 1;
      ret->elf.ref_dynamic = 0; /* This should be always 0 for local.  */
      ret->elf.ref_regular = 0;
      ret->elf.forced_local = 1;
      ret->elf.root.type = bfd_link_hash_defined;
      *slot = ret;
    }
  return &ret->elf;
}

/* Destroy an LoongArch elf linker hash table.  */

static void
elfNN_loongarch_link_hash_table_free (bfd *obfd)
{
  struct loongarch_elf_link_hash_table *ret;
  ret = (struct loongarch_elf_link_hash_table *) obfd->link.hash;

  if (ret->loc_hash_table)
    htab_delete (ret->loc_hash_table);
  if (ret->loc_hash_memory)
    objalloc_free ((struct objalloc *) ret->loc_hash_memory);

  _bfd_elf_link_hash_table_free (obfd);
}

/* Create a LoongArch ELF linker hash table.  */

static struct bfd_link_hash_table *
loongarch_elf_link_hash_table_create (bfd *abfd)
{
  struct loongarch_elf_link_hash_table *ret;
  bfd_size_type amt = sizeof (struct loongarch_elf_link_hash_table);

  ret = (struct loongarch_elf_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init
      (&ret->elf, abfd, link_hash_newfunc,
       sizeof (struct loongarch_elf_link_hash_entry), LARCH_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->max_alignment = MINUS_ONE;

  ret->loc_hash_table = htab_try_create (1024, elfNN_loongarch_local_htab_hash,
					 elfNN_loongarch_local_htab_eq, NULL);
  ret->loc_hash_memory = objalloc_create ();
  if (!ret->loc_hash_table || !ret->loc_hash_memory)
    {
      elfNN_loongarch_link_hash_table_free (abfd);
      return NULL;
    }
  ret->elf.root.hash_table_free = elfNN_loongarch_link_hash_table_free;

  return &ret->elf.root;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
elfNN_loongarch_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword in_flags = elf_elfheader (ibfd)->e_flags;
  flagword out_flags = elf_elfheader (obfd)->e_flags;

  if (!is_loongarch_elf (ibfd) || !is_loongarch_elf (obfd))
    return true;

  if (strcmp (bfd_get_target (ibfd), bfd_get_target (obfd)) != 0)
    {
      _bfd_error_handler (_("%pB: ABI is incompatible with that of "
			    "the selected emulation:\n"
			    "  target emulation `%s' does not match `%s'"),
			  ibfd, bfd_get_target (ibfd), bfd_get_target (obfd));
      return false;
    }

  if (!_bfd_elf_merge_object_attributes (ibfd, info))
    return false;

  /* If the input BFD is not a dynamic object and it does not contain any
     non-data sections, do not account its ABI.  For example, various
     packages produces such data-only relocatable objects with
     `ld -r -b binary` or `objcopy`, and these objects have zero e_flags.
     But they are compatible with all ABIs.  */
  if (!(ibfd->flags & DYNAMIC))
    {
      asection *sec;
      bool have_code_sections = false;
      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	if ((bfd_section_flags (sec)
	     & (SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS))
	    == (SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS))
	  {
	    have_code_sections = true;
	    break;
	  }
      if (!have_code_sections)
	return true;
    }

  if (!elf_flags_init (obfd))
    {
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = in_flags;
      return true;
    }
  else if (out_flags != in_flags)
    {
      if ((EF_LOONGARCH_IS_OBJ_V0 (out_flags)
	   && EF_LOONGARCH_IS_OBJ_V1 (in_flags))
	  || (EF_LOONGARCH_IS_OBJ_V0 (in_flags)
	      && EF_LOONGARCH_IS_OBJ_V1 (out_flags)))
	{
	  elf_elfheader (obfd)->e_flags |= EF_LOONGARCH_OBJABI_V1;
	  out_flags = elf_elfheader (obfd)->e_flags;
	  in_flags = out_flags;
	}
    }

  /* Disallow linking different ABIs.  */
  /* Only check relocation version.
     The obj_v0 is compatible with obj_v1.  */
  if (EF_LOONGARCH_ABI(out_flags ^ in_flags) & EF_LOONGARCH_ABI_MASK)
    {
      _bfd_error_handler (_("%pB: can't link different ABI object."), ibfd);
      goto fail;
    }

  return true;

 fail:
  bfd_set_error (bfd_error_bad_value);
  return false;
}

/* Create the .got section.  */

static bool
loongarch_elf_create_got_section (bfd *abfd, struct bfd_link_info *info)
{
  flagword flags;
  char *name;
  asection *s, *s_got;
  struct elf_link_hash_entry *h;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* This function may be called more than once.  */
  if (htab->sgot != NULL)
    return true;

  flags = bed->dynamic_sec_flags;
  name = bed->rela_plts_and_copies_p ? ".rela.got" : ".rel.got";
  s = bfd_make_section_anyway_with_flags (abfd, name, flags | SEC_READONLY);

  if (s == NULL || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;
  htab->srelgot = s;

  s = s_got = bfd_make_section_anyway_with_flags (abfd, ".got", flags);
  if (s == NULL || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;
  htab->sgot = s;

  /* The first bit of the global offset table is the header.  */
  s->size += bed->got_header_size;

  if (bed->want_got_plt)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".got.plt", flags);
      if (s == NULL || !bfd_set_section_alignment (s, bed->s->log_file_align))
	return false;
      htab->sgotplt = s;

      /* Reserve room for the header.  */
      s->size = GOTPLT_HEADER_SIZE;
    }

  if (bed->want_got_sym)
    {
      /* Define the symbol _GLOBAL_OFFSET_TABLE_ at the start of the .got
	 section.  We don't do this in the linker script because we don't want
	 to define the symbol if we are not creating a global offset table.  */
      h = _bfd_elf_define_linkage_sym (abfd, info, s_got,
				       "_GLOBAL_OFFSET_TABLE_");
      elf_hash_table (info)->hgot = h;
      if (h == NULL)
	return false;
    }
  return true;
}

/* Create .plt, .rela.plt, .got, .got.plt, .rela.got, .dynbss, and
   .rela.bss sections in DYNOBJ, and set up shortcuts to them in our
   hash table.  */

static bool
loongarch_elf_create_dynamic_sections (bfd *dynobj, struct bfd_link_info *info)
{
  struct loongarch_elf_link_hash_table *htab;

  htab = loongarch_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (!loongarch_elf_create_got_section (dynobj, info))
    return false;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return false;

  if (!bfd_link_pic (info))
    htab->sdyntdata
      = bfd_make_section_anyway_with_flags (dynobj, ".tdata.dyn",
					    SEC_ALLOC | SEC_THREAD_LOCAL);

  if (!htab->elf.splt || !htab->elf.srelplt || !htab->elf.sdynbss
      || (!bfd_link_pic (info) && (!htab->elf.srelbss || !htab->sdyntdata)))
    abort ();

  return true;
}

static bool
loongarch_elf_record_tls_and_got_reference (bfd *abfd,
					    struct bfd_link_info *info,
					    struct elf_link_hash_entry *h,
					    unsigned long symndx,
					    char tls_type)
{
  struct loongarch_elf_link_hash_table *htab = loongarch_elf_hash_table (info);
  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  /* This is a global offset table entry for a local symbol.  */
  if (elf_local_got_refcounts (abfd) == NULL)
    {
      bfd_size_type size =
	symtab_hdr->sh_info * (sizeof (bfd_vma) + sizeof (tls_type));
      if (!(elf_local_got_refcounts (abfd) = bfd_zalloc (abfd, size)))
	return false;
      _bfd_loongarch_elf_local_got_tls_type (abfd) =
	(char *) (elf_local_got_refcounts (abfd) + symtab_hdr->sh_info);
    }

  switch (tls_type)
    {
    case GOT_NORMAL:
    case GOT_TLS_GD:
    case GOT_TLS_IE:
      /* Need GOT.  */
      if (htab->elf.sgot == NULL
	  && !loongarch_elf_create_got_section (htab->elf.dynobj, info))
	return false;
      if (h)
	{
	  if (h->got.refcount < 0)
	    h->got.refcount = 0;
	  h->got.refcount++;
	}
      else
	elf_local_got_refcounts (abfd)[symndx]++;
      break;
    case GOT_TLS_LE:
      /* No need for GOT.  */
      break;
    default:
      _bfd_error_handler (_("Internal error: unreachable."));
      return false;
    }

  char *new_tls_type = &_bfd_loongarch_elf_tls_type (abfd, h, symndx);
  *new_tls_type |= tls_type;
  if ((*new_tls_type & GOT_NORMAL) && (*new_tls_type & ~GOT_NORMAL))
    {
      _bfd_error_handler (_("%pB: `%s' accessed both as normal and "
			    "thread local symbol"),
			  abfd,
			  h ? h->root.root.string : "<local>");
      return false;
    }

  return true;
}

/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table or procedure linkage
   table.  */

static bool
loongarch_elf_check_relocs (bfd *abfd, struct bfd_link_info *info,
			    asection *sec, const Elf_Internal_Rela *relocs)
{
  struct loongarch_elf_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  asection *sreloc = NULL;

  if (bfd_link_relocatable (info))
    return true;

  htab = loongarch_elf_hash_table (info);
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  if (htab->elf.dynobj == NULL)
    htab->elf.dynobj = abfd;

  for (rel = relocs; rel < relocs + sec->reloc_count; rel++)
    {
      unsigned int r_type;
      unsigned int r_symndx;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *isym = NULL;

      r_symndx = ELFNN_R_SYM (rel->r_info);
      r_type = ELFNN_R_TYPE (rel->r_info);

      if (r_symndx >= NUM_SHDR_ENTRIES (symtab_hdr))
	{
	  _bfd_error_handler (_("%pB: bad symbol index: %d"), abfd, r_symndx);
	  return false;
	}

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache, abfd, r_symndx);
	  if (isym == NULL)
	    return false;

	  if (ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
	    {
	      h = elfNN_loongarch_get_local_sym_hash (htab, abfd, rel, true);
	      if (h == NULL)
		return false;

	      h->type = STT_GNU_IFUNC;
	      h->ref_regular = 1;
	    }
	  else
	    h = NULL;
	}
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      /* It is referenced by a non-shared object.  */
      if (h != NULL)
	h->ref_regular = 1;

      if (h && h->type == STT_GNU_IFUNC)
	{
	  if (htab->elf.dynobj == NULL)
	    htab->elf.dynobj = abfd;

	  /* Create 'irelifunc' in PIC object.  */
	  if (bfd_link_pic (info)
	      && !_bfd_elf_create_ifunc_sections (htab->elf.dynobj, info))
	    return false;
	  /* If '.plt' not represent, create '.iplt' to deal with ifunc.  */
	  else if (!htab->elf.splt
		   && !_bfd_elf_create_ifunc_sections (htab->elf.dynobj, info))
	    return false;
	  /* Create the ifunc sections, iplt and ipltgot, for static
	     executables.  */
	  if ((r_type == R_LARCH_64 || r_type == R_LARCH_32)
	      && !_bfd_elf_create_ifunc_sections (htab->elf.dynobj, info))
	    return false;

	  if (h->plt.refcount < 0)
	    h->plt.refcount = 0;
	  h->plt.refcount++;
	  h->needs_plt = 1;

	  elf_tdata (info->output_bfd)->has_gnu_osabi |= elf_gnu_osabi_ifunc;
	}

      int need_dynreloc = 0;
      int only_need_pcrel = 0;

      switch (r_type)
	{
	case R_LARCH_GOT_PC_HI20:
	case R_LARCH_GOT_HI20:
	case R_LARCH_SOP_PUSH_GPREL:
	  /* For la.global.  */
	  if (h)
	    h->pointer_equality_needed = 1;
	  if (!loongarch_elf_record_tls_and_got_reference (abfd, info, h,
							   r_symndx,
							   GOT_NORMAL))
	    return false;
	  break;

	case R_LARCH_TLS_LD_PC_HI20:
	case R_LARCH_TLS_LD_HI20:
	case R_LARCH_TLS_GD_PC_HI20:
	case R_LARCH_TLS_GD_HI20:
	case R_LARCH_SOP_PUSH_TLS_GD:
	  if (!loongarch_elf_record_tls_and_got_reference (abfd, info, h,
							   r_symndx,
							   GOT_TLS_GD))
	    return false;
	  break;

	case R_LARCH_TLS_IE_PC_HI20:
	case R_LARCH_TLS_IE_HI20:
	case R_LARCH_SOP_PUSH_TLS_GOT:
	  if (bfd_link_pic (info))
	    /* May fail for lazy-bind.  */
	    info->flags |= DF_STATIC_TLS;

	  if (!loongarch_elf_record_tls_and_got_reference (abfd, info, h,
							   r_symndx,
							   GOT_TLS_IE))
	    return false;
	  break;

	case R_LARCH_TLS_LE_HI20:
	case R_LARCH_SOP_PUSH_TLS_TPREL:
	  if (!bfd_link_executable (info))
	    return false;

	  info->flags |= DF_STATIC_TLS;

	  if (!loongarch_elf_record_tls_and_got_reference (abfd, info, h,
							   r_symndx,
							   GOT_TLS_LE))
	    return false;
	  break;

	case R_LARCH_ABS_HI20:
	case R_LARCH_SOP_PUSH_ABSOLUTE:
	  if (h != NULL)
	    /* If this reloc is in a read-only section, we might
	       need a copy reloc.  We can't check reliably at this
	       stage whether the section is read-only, as input
	       sections have not yet been mapped to output sections.
	       Tentatively set the flag for now, and correct in
	       adjust_dynamic_symbol.  */
	    h->non_got_ref = 1;
	  break;

	case R_LARCH_PCALA_HI20:
	  if (h != NULL)
	    {
	      /* For pcalau12i + jirl.  */
	      h->needs_plt = 1;
	      if (h->plt.refcount < 0)
		h->plt.refcount = 0;
	      h->plt.refcount++;

	      h->non_got_ref = 1;
	      h->pointer_equality_needed = 1;
	    }

	  break;

	case R_LARCH_B16:
	case R_LARCH_B21:
	case R_LARCH_B26:
	  if (h != NULL)
	    {
	      h->needs_plt = 1;
	      if (!bfd_link_pic (info))
		h->non_got_ref = 1;

	      /* We try to create PLT stub for all non-local function.  */
	      if (h->plt.refcount < 0)
		h->plt.refcount = 0;
	      h->plt.refcount++;
	    }

	  break;

	case R_LARCH_SOP_PUSH_PCREL:
	  if (h != NULL)
	    {
	      if (!bfd_link_pic (info))
		h->non_got_ref = 1;

	      /* We try to create PLT stub for all non-local function.  */
	      if (h->plt.refcount < 0)
		h->plt.refcount = 0;
	      h->plt.refcount++;
	      h->pointer_equality_needed = 1;
	    }

	  break;

	case R_LARCH_SOP_PUSH_PLT_PCREL:
	  /* This symbol requires a procedure linkage table entry.  We
	     actually build the entry in adjust_dynamic_symbol,
	     because this might be a case of linking PIC code without
	     linking in any dynamic objects, in which case we don't
	     need to generate a procedure linkage table after all.  */
	  if (h != NULL)
	    {
	      h->needs_plt = 1;
	      if (h->plt.refcount < 0)
		h->plt.refcount = 0;
	      h->plt.refcount++;
	    }
	  break;

	case R_LARCH_TLS_DTPREL32:
	case R_LARCH_TLS_DTPREL64:
	  need_dynreloc = 1;
	  only_need_pcrel = 1;
	  break;

	case R_LARCH_JUMP_SLOT:
	case R_LARCH_32:
	case R_LARCH_64:

	  need_dynreloc = 1;

	  /* If resolved symbol is defined in this object,
	     1. Under pie, the symbol is known.  We convert it
	     into R_LARCH_RELATIVE and need load-addr still.
	     2. Under pde, the symbol is known and we can discard R_LARCH_NN.
	     3. Under dll, R_LARCH_NN can't be changed normally, since
	     its defination could be covered by the one in executable.
	     For symbolic, we convert it into R_LARCH_RELATIVE.
	     Thus, only under pde, it needs pcrel only.  We discard it.  */
	  only_need_pcrel = bfd_link_pde (info);

	  if (h != NULL
	      && (!bfd_link_pic (info)
		  || h->type == STT_GNU_IFUNC))
	    {
	      /* This reloc might not bind locally.  */
	      h->non_got_ref = 1;
	      h->pointer_equality_needed = 1;

	      if (!h->def_regular
		  || (sec->flags & (SEC_CODE | SEC_READONLY)) != 0)
		{
		  /* We may need a .plt entry if the symbol is a function
		     defined in a shared lib or is a function referenced
		     from the code or read-only section.  */
		  h->plt.refcount += 1;
		}
	    }
	  break;

	case R_LARCH_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	case R_LARCH_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	default:
	  break;
	}

      /* Record some info for sizing and allocating dynamic entry.  */
      if (need_dynreloc && (sec->flags & SEC_ALLOC))
	{
	  /* When creating a shared object, we must copy these
	     relocs into the output file.  We create a reloc
	     section in dynobj and make room for the reloc.  */
	  struct elf_dyn_relocs *p;
	  struct elf_dyn_relocs **head;

	  if (sreloc == NULL)
	    {
	      sreloc
		= _bfd_elf_make_dynamic_reloc_section (sec, htab->elf.dynobj,
						       LARCH_ELF_LOG_WORD_BYTES,
						       abfd, /*rela?*/ true);
	      if (sreloc == NULL)
		return false;
	    }

	  /* If this is a global symbol, we count the number of
	     relocations we need for this symbol.  */
	  if (h != NULL)
	    head = &h->dyn_relocs;
	  else
	    {
	      /* Track dynamic relocs needed for local syms too.
		 We really need local syms available to do this
		 easily.  Oh well.  */

	      asection *s;
	      void *vpp;

	      s = bfd_section_from_elf_index (abfd, isym->st_shndx);
	      if (s == NULL)
		s = sec;

	      vpp = &elf_section_data (s)->local_dynrel;
	      head = (struct elf_dyn_relocs **) vpp;
	    }

	  p = *head;
	  if (p == NULL || p->sec != sec)
	    {
	      bfd_size_type amt = sizeof *p;
	      p = (struct elf_dyn_relocs *) bfd_alloc (htab->elf.dynobj, amt);
	      if (p == NULL)
		return false;
	      p->next = *head;
	      *head = p;
	      p->sec = sec;
	      p->count = 0;
	      p->pc_count = 0;
	    }

	  p->count++;
	  p->pc_count += only_need_pcrel;
	}
    }

  return true;
}

/* Find dynamic relocs for H that apply to read-only sections.  */

static asection *
readonly_dynrelocs (struct elf_link_hash_entry *h)
{
  struct elf_dyn_relocs *p;

  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *s = p->sec->output_section;

      if (s != NULL && (s->flags & SEC_READONLY) != 0)
	return p->sec;
    }
  return NULL;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */
static bool
loongarch_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				     struct elf_link_hash_entry *h)
{
  struct loongarch_elf_link_hash_table *htab;
  bfd *dynobj;

  htab = loongarch_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  dynobj = htab->elf.dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
	      && (h->needs_plt
		  || h->type == STT_GNU_IFUNC
		  || h->is_weakalias
		  || (h->def_dynamic
		      && h->ref_regular
		      && !h->def_regular)));

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later
     (although we could actually do it here).  */
  if (h->type == STT_FUNC || h->type == STT_GNU_IFUNC || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || (h->type != STT_GNU_IFUNC
	      && (SYMBOL_REFERENCES_LOCAL (info, h)
		  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
		      && h->root.type == bfd_link_hash_undefweak))))
	{
	  /* This case can occur if we saw a R_LARCH_SOP_PUSH_PLT_PCREL reloc
	     in an input file, but the symbol was never referred to by a
	     dynamic object, or if all references were garbage collected.
	     In such a case, we don't actually need to build a PLT entry.  */
	  h->plt.offset = MINUS_ONE;
	  h->needs_plt = 0;
	}

      return true;
    }
  else
    h->plt.offset = MINUS_ONE;

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

  /* R_LARCH_COPY is not adept glibc, not to generate.  */
  /* Can not print anything, because make check ld.  */
  return true;
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct loongarch_elf_link_hash_table *htab;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  if (h->type == STT_GNU_IFUNC
      && h->def_regular)
    return true;

  info = (struct bfd_link_info *) inf;
  htab = loongarch_elf_hash_table (info);
  bool dyn = htab->elf.dynamic_sections_created;
  BFD_ASSERT (htab != NULL);

  do
    {
      asection *plt, *gotplt, *relplt;

      if (!h->needs_plt)
	break;

      h->needs_plt = 0;

      if (htab->elf.splt)
	{
	  if (h->dynindx == -1 && !h->forced_local && dyn
	      && h->root.type == bfd_link_hash_undefweak)
	    {
	      if (!bfd_elf_link_record_dynamic_symbol (info, h))
		return false;
	    }

	  if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, bfd_link_pic (info), h)
	      && h->type != STT_GNU_IFUNC)
	    break;

	  plt = htab->elf.splt;
	  gotplt = htab->elf.sgotplt;
	  relplt = htab->elf.srelplt;
	}
      else if (htab->elf.iplt)
	{
	  /* .iplt only for IFUNC.  */
	  if (h->type != STT_GNU_IFUNC)
	    break;

	  plt = htab->elf.iplt;
	  gotplt = htab->elf.igotplt;
	  relplt = htab->elf.irelplt;
	}
      else
	break;

      if (plt->size == 0)
	plt->size = PLT_HEADER_SIZE;

      h->plt.offset = plt->size;
      plt->size += PLT_ENTRY_SIZE;
      gotplt->size += GOT_ENTRY_SIZE;
      relplt->size += sizeof (ElfNN_External_Rela);

      /* If this symbol is not defined in a regular file, and we are
	 not generating a shared library, then set the symbol to this
	 location in the .plt.  This is required to make function
	 pointers compare as equal between the normal executable and
	 the shared library.  */
      if (!bfd_link_pic (info)
	  && !h->def_regular)
	{
	  h->root.u.def.section = plt;
	  h->root.u.def.value = h->plt.offset;
	}

      h->needs_plt = 1;
    }
  while (0);

  if (!h->needs_plt)
    h->plt.offset = MINUS_ONE;

  if (0 < h->got.refcount)
    {
      asection *s;
      int tls_type = loongarch_elf_hash_entry (h)->tls_type;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1 && !h->forced_local && dyn
	  && h->root.type == bfd_link_hash_undefweak)
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      s = htab->elf.sgot;
      h->got.offset = s->size;
      if (tls_type & (GOT_TLS_GD | GOT_TLS_IE))
	{
	  /* TLS_GD needs two dynamic relocs and two GOT slots.  */
	  if (tls_type & GOT_TLS_GD)
	    {
	      s->size += 2 * GOT_ENTRY_SIZE;
	      if (bfd_link_executable (info))
		{
		  /* Link exe and not defined local.  */
		  if (!SYMBOL_REFERENCES_LOCAL (info, h))
		    htab->elf.srelgot->size += 2 * sizeof (ElfNN_External_Rela);
		}
	      else
		{
		  if (SYMBOL_REFERENCES_LOCAL (info, h))
		    htab->elf.srelgot->size += sizeof (ElfNN_External_Rela);
		  else
		    htab->elf.srelgot->size += 2 * sizeof (ElfNN_External_Rela);
		}
	    }

	  /* TLS_IE needs one dynamic reloc and one GOT slot.  */
	  if (tls_type & GOT_TLS_IE)
	    {
	      s->size += GOT_ENTRY_SIZE;

	      if (bfd_link_executable (info))
		{
		  /* Link exe and not defined local.  */
		  if (!SYMBOL_REFERENCES_LOCAL (info, h))
		    htab->elf.srelgot->size += sizeof (ElfNN_External_Rela);
		}
	      else
		{
		  htab->elf.srelgot->size += sizeof (ElfNN_External_Rela);
		}
	    }
	}
      else
	{
	  s->size += GOT_ENTRY_SIZE;
	  if ((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	       || h->root.type != bfd_link_hash_undefweak)
	      && (bfd_link_pic (info)
		  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info),
						      h))
	      && !UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
	      /* Undefined weak symbol in static PIE resolves to 0 without
		 any dynamic relocations.  */
	    htab->elf.srelgot->size += sizeof (ElfNN_External_Rela);
	}
    }
  else
    h->got.offset = MINUS_ONE;

  if (h->dyn_relocs == NULL)
    return true;

  /* Extra dynamic relocate,
   * R_LARCH_64
   * R_LARCH_TLS_DTPRELNN
   * R_LARCH_JUMP_SLOT
   * R_LARCH_NN.  */

  if (SYMBOL_CALLS_LOCAL (info, h))
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

  if (h->root.type == bfd_link_hash_undefweak)
    {
      if (UNDEFWEAK_NO_DYNAMIC_RELOC (info, h)
	  || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	  || (!bfd_link_pic (info) && h->non_got_ref))
	h->dyn_relocs = NULL;
      else if (h->dynindx == -1 && !h->forced_local)
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (!bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;

	  if (h->dynindx == -1)
	    h->dyn_relocs = NULL;
	}
    }

  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;
      sreloc->size += p->count * sizeof (ElfNN_External_Rela);
    }

  return true;
}

/* A modified version of _bfd_elf_allocate_ifunc_dyn_relocs.
   For local def and ref ifunc,
   dynamic relocations are stored in
   1.  rela.srelgot section in dynamic object (dll or exec).
   2.  rela.irelplt section in static executable.
   Unlike _bfd_elf_allocate_ifunc_dyn_relocs, rela.srelgot is used
   instead of rela.srelplt.  Glibc ELF loader will not support
   R_LARCH_IRELATIVE relocation in rela.plt.  */

static bool
local_allocate_ifunc_dyn_relocs (struct bfd_link_info *info,
				    struct elf_link_hash_entry *h,
				    struct elf_dyn_relocs **head,
				    unsigned int plt_entry_size,
				    unsigned int plt_header_size,
				    unsigned int got_entry_size,
				    bool avoid_plt)
{
  asection *plt, *gotplt, *relplt;
  struct elf_dyn_relocs *p;
  unsigned int sizeof_reloc;
  const struct elf_backend_data *bed;
  struct elf_link_hash_table *htab;
  /* If AVOID_PLT is TRUE, don't use PLT if possible.  */
  bool use_plt = !avoid_plt || h->plt.refcount > 0;
  bool need_dynreloc = !use_plt || bfd_link_pic (info);

  /* When a PIC object references a STT_GNU_IFUNC symbol defined
     in executable or it isn't referenced via PLT, the address of
     the resolved function may be used.  But in non-PIC executable,
     the address of its plt slot may be used.  Pointer equality may
     not work correctly.  PIE or non-PLT reference should be used if
     pointer equality is required here.

     If STT_GNU_IFUNC symbol is defined in position-dependent executable,
     backend should change it to the normal function and set its address
     to its PLT entry which should be resolved by R_*_IRELATIVE at
     run-time.  All external references should be resolved to its PLT in
     executable.  */
  if (!need_dynreloc
      && !(bfd_link_pde (info) && h->def_regular)
      && (h->dynindx != -1
	  || info->export_dynamic)
      && h->pointer_equality_needed)
    {
      info->callbacks->einfo
	/* xgettext:c-format.  */
	(_("%F%P: dynamic STT_GNU_IFUNC symbol `%s' with pointer "
	   "equality in `%pB' can not be used when making an "
	   "executable; recompile with -fPIE and relink with -pie\n"),
	 h->root.root.string,
	 h->root.u.def.section->owner);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  htab = elf_hash_table (info);

  /* When the symbol is marked with regular reference, if PLT isn't used
     or we are building a PIC object, we must keep dynamic relocation
     if there is non-GOT reference and use PLT if there is PC-relative
     reference.  */
  if (need_dynreloc && h->ref_regular)
    {
      bool keep = false;
      for (p = *head; p != NULL; p = p->next)
	if (p->count)
	  {
	    h->non_got_ref = 1;
	    /* Need dynamic relocations for non-GOT reference.  */
	    keep = true;
	    if (p->pc_count)
	      {
		/* Must use PLT for PC-relative reference.  */
		use_plt = true;
		need_dynreloc = bfd_link_pic (info);
		break;
	      }
	  }
      if (keep)
	goto keep;
    }

  /* Support garbage collection against STT_GNU_IFUNC symbols.  */
  if (h->plt.refcount <= 0 && h->got.refcount <= 0)
    {
      h->got = htab->init_got_offset;
      h->plt = htab->init_plt_offset;
      *head = NULL;
      return true;
    }

  /* Return and discard space for dynamic relocations against it if
     it is never referenced.  */
  if (!h->ref_regular)
    {
      if (h->plt.refcount > 0
	  || h->got.refcount > 0)
	abort ();
      h->got = htab->init_got_offset;
      h->plt = htab->init_plt_offset;
      *head = NULL;
      return true;
    }

 keep:
  bed = get_elf_backend_data (info->output_bfd);
  if (bed->rela_plts_and_copies_p)
    sizeof_reloc = bed->s->sizeof_rela;
  else
    sizeof_reloc = bed->s->sizeof_rel;

  /* When building a static executable, use iplt, igot.plt and
     rela.iplt sections for STT_GNU_IFUNC symbols.  */
  if (htab->splt != NULL)
    {
      plt = htab->splt;
      gotplt = htab->sgotplt;
      /* Change dynamic info of ifunc gotplt from srelplt to srelgot.  */
      relplt = htab->srelgot;

      /* If this is the first plt entry and PLT is used, make room for
	 the special first entry.  */
      if (plt->size == 0 && use_plt)
	plt->size += plt_header_size;
    }
  else
    {
      plt = htab->iplt;
      gotplt = htab->igotplt;
      relplt = htab->irelplt;
    }

  if (use_plt)
    {
      /* Don't update value of STT_GNU_IFUNC symbol to PLT.  We need
	 the original value for R_*_IRELATIVE.  */
      h->plt.offset = plt->size;

      /* Make room for this entry in the plt/iplt section.  */
      plt->size += plt_entry_size;

      /* We also need to make an entry in the got.plt/got.iplt section,
	 which will be placed in the got section by the linker script.  */
      gotplt->size += got_entry_size;
    }

  /* We also need to make an entry in the rela.plt/.rela.iplt
     section for GOTPLT relocation if PLT is used.  */
  if (use_plt)
    {
      relplt->size += sizeof_reloc;
      relplt->reloc_count++;
    }

  /* We need dynamic relocation for STT_GNU_IFUNC symbol only when
     there is a non-GOT reference in a PIC object or PLT isn't used.  */
  if (!need_dynreloc || !h->non_got_ref)
    *head = NULL;

  /* Finally, allocate space.  */
  p = *head;
  if (p != NULL)
    {
      bfd_size_type count = 0;
      do
	{
	  count += p->count;
	  p = p->next;
	}
      while (p != NULL);

      htab->ifunc_resolvers = count != 0;

      /* Dynamic relocations are stored in
	 1.  rela.srelgot section in PIC object.
	 2.  rela.srelgot section in dynamic executable.
	 3.  rela.irelplt section in static executable.  */
      if (htab->splt != NULL)
	htab->srelgot->size += count * sizeof_reloc;
      else
	{
	  relplt->size += count * sizeof_reloc;
	  relplt->reloc_count += count;
	}
    }

  /* For STT_GNU_IFUNC symbol, got.plt has the real function address
     and got has the PLT entry adddress.  We will load the GOT entry
     with the PLT entry in finish_dynamic_symbol if it is used.  For
     branch, it uses got.plt.  For symbol value, if PLT is used,
     1.  Use got.plt in a PIC object if it is forced local or not
     dynamic.
     2.  Use got.plt in a non-PIC object if pointer equality isn't
     needed.
     3.  Use got.plt in PIE.
     4.  Use got.plt if got isn't used.
     5.  Otherwise use got so that it can be shared among different
     objects at run-time.
     If PLT isn't used, always use got for symbol value.
     We only need to relocate got entry in PIC object or in dynamic
     executable without PLT.  */
  if (use_plt
      && (h->got.refcount <= 0
	  || (bfd_link_pic (info)
	      && (h->dynindx == -1
		  || h->forced_local))
	  || (
	      !h->pointer_equality_needed)
	  || htab->sgot == NULL))
    {
      /* Use got.plt.  */
      h->got.offset = (bfd_vma) -1;
    }
  else
    {
      if (!use_plt)
	{
	  /* PLT isn't used.  */
	  h->plt.offset = (bfd_vma) -1;
	}
      if (h->got.refcount <= 0)
	{
	  /* GOT isn't need when there are only relocations for static
	     pointers.  */
	  h->got.offset = (bfd_vma) -1;
	}
      else
	{
	  h->got.offset = htab->sgot->size;
	  htab->sgot->size += got_entry_size;
	  /* Need to relocate the GOT entry in a PIC object or PLT isn't
	     used.  Otherwise, the GOT entry will be filled with the PLT
	     entry and dynamic GOT relocation isn't needed.  */
	  if (need_dynreloc)
	    {
	      /* For non-static executable, dynamic GOT relocation is in
		 rela.got section, but for static executable, it is
		 in rela.iplt section.  */
	      if (htab->splt != NULL)
		htab->srelgot->size += sizeof_reloc;
	      else
		{
		  relplt->size += sizeof_reloc;
		  relplt->reloc_count++;
		}
	    }
	}
    }

  return true;
}

/* Allocate space in .plt, .got and associated reloc sections for
   ifunc dynamic relocs.  */

static bool
elfNN_allocate_ifunc_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  /* An example of a bfd_link_hash_indirect symbol is versioned
     symbol. For example: __gxx_personality_v0(bfd_link_hash_indirect)
     -> __gxx_personality_v0(bfd_link_hash_defined)

     There is no need to process bfd_link_hash_indirect symbols here
     because we will also be presented with the concrete instance of
     the symbol and loongarch_elf_copy_indirect_symbol () will have been
     called to copy all relevant data from the generic to the concrete
     symbol instance.  */
  if (h->root.type == bfd_link_hash_indirect)
    return true;

  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  info = (struct bfd_link_info *) inf;

  /* Since STT_GNU_IFUNC symbol must go through PLT, we handle it
     here if it is defined and referenced in a non-shared object.  */
  if (h->type == STT_GNU_IFUNC && h->def_regular)
    {
      if (SYMBOL_REFERENCES_LOCAL (info, h))
	return local_allocate_ifunc_dyn_relocs (info, h,
						&h->dyn_relocs,
						PLT_ENTRY_SIZE,
						PLT_HEADER_SIZE,
						GOT_ENTRY_SIZE,
						false);
      else
	return _bfd_elf_allocate_ifunc_dyn_relocs (info, h,
						   &h->dyn_relocs,
						   PLT_ENTRY_SIZE,
						   PLT_HEADER_SIZE,
						   GOT_ENTRY_SIZE,
						   false);
    }

  return true;
}

/* Allocate space in .plt, .got and associated reloc sections for
   ifunc dynamic relocs.  */

static bool
elfNN_allocate_local_ifunc_dynrelocs (void **slot, void *inf)
{
  struct elf_link_hash_entry *h = (struct elf_link_hash_entry *) *slot;

  if (h->type != STT_GNU_IFUNC
      || !h->def_regular
      || !h->ref_regular
      || !h->forced_local
      || h->root.type != bfd_link_hash_defined)
    abort ();

  return elfNN_allocate_ifunc_dynrelocs (h, inf);
}

/* Set DF_TEXTREL if we find any dynamic relocs that apply to
   read-only sections.  */

static bool
maybe_set_textrel (struct elf_link_hash_entry *h, void *info_p)
{
  asection *sec;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  sec = readonly_dynrelocs (h);
  if (sec != NULL)
    {
      struct bfd_link_info *info = (struct bfd_link_info *) info_p;

      info->flags |= DF_TEXTREL;
      info->callbacks->minfo (_("%pB: dynamic relocation against `%pT' in "
				"read-only section `%pA'\n"),
			      sec->owner, h->root.root.string, sec);

      /* Not an error, just cut short the traversal.  */
      return false;
    }
  return true;
}

static bool
loongarch_elf_size_dynamic_sections (bfd *output_bfd,
				     struct bfd_link_info *info)
{
  struct loongarch_elf_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd *ibfd;

  htab = loongarch_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  dynobj = htab->elf.dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (htab->elf.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  const char *interpreter;
	  s = bfd_get_linker_section (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);

	  if (elf_elfheader (output_bfd)->e_ident[EI_CLASS] == ELFCLASS32)
	    interpreter = "/lib32/ld.so.1";
	  else if (elf_elfheader (output_bfd)->e_ident[EI_CLASS] == ELFCLASS64)
	    interpreter = "/lib64/ld.so.1";
	  else
	    interpreter = "/lib/ld.so.1";

	  s->contents = (unsigned char *) interpreter;
	  s->size = strlen (interpreter) + 1;
	}
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      char *local_tls_type;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srel;

      if (!is_loongarch_elf (ibfd))
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = elf_section_data (s)->local_dynrel; p != NULL; p = p->next)
	    {
	      p->count -= p->pc_count;
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (0 < p->count)
		{
		  srel = elf_section_data (p->sec)->sreloc;
		  srel->size += p->count * sizeof (ElfNN_External_Rela);
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      local_tls_type = _bfd_loongarch_elf_local_got_tls_type (ibfd);
      s = htab->elf.sgot;
      srel = htab->elf.srelgot;
      for (; local_got < end_local_got; ++local_got, ++local_tls_type)
	{
	  if (0 < *local_got)
	    {
	      *local_got = s->size;

	      /* TLS gd use two got.  */
	      if (*local_tls_type & GOT_TLS_GD)
		s->size += GOT_ENTRY_SIZE * 2;
	      else
		/* Normal got, tls ie/ld use one got.  */
		s->size += GOT_ENTRY_SIZE;

	      if (bfd_link_executable (info)
		  && (*local_tls_type & (GOT_TLS_GD| GOT_TLS_IE)))
		;/* Do nothing.  */
	      else
		{
		  srel->size += sizeof (ElfNN_External_Rela);
		}
	    }
	  else
	    *local_got = MINUS_ONE;
	}
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->elf, allocate_dynrelocs, info);

  /* Allocate global ifunc sym .plt and .got entries, and space for global
     ifunc sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->elf, elfNN_allocate_ifunc_dynrelocs, info);

  /* Allocate .plt and .got entries, and space for local ifunc symbols.  */
  htab_traverse (htab->loc_hash_table,
		 (void *) elfNN_allocate_local_ifunc_dynrelocs, info);

  /* Don't allocate .got.plt section if there are no PLT.  */
  if (htab->elf.sgotplt && htab->elf.sgotplt->size == GOTPLT_HEADER_SIZE
      && (htab->elf.splt == NULL || htab->elf.splt->size == 0))
    htab->elf.sgotplt->size = 0;

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->elf.splt || s == htab->elf.iplt || s == htab->elf.sgot
	  || s == htab->elf.sgotplt || s == htab->elf.igotplt
	  || s == htab->elf.sdynbss || s == htab->elf.sdynrelro)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (strncmp (s->name, ".rela", 5) == 0)
	{
	  if (s->size != 0)
	    {
	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else
	{
	  /* It's not one of our sections.  */
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

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  Zero the memory
	 for the benefit of .rela.plt, which has 4 unused entries
	 at the beginning, and we don't want garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in loongarch_elf_finish_dynamic_sections, but we
	 must add the entries now so that we get the correct size for
	 the .dynamic section.  The DT_DEBUG entry is filled in by the
	 dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL) _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (bfd_link_executable (info))
	{
	  if (!add_dynamic_entry (DT_DEBUG, 0))
	    return false;
	}

      if (htab->elf.srelplt->size != 0)
	{
	  if (!add_dynamic_entry (DT_PLTGOT, 0)
	      || !add_dynamic_entry (DT_PLTRELSZ, 0)
	      || !add_dynamic_entry (DT_PLTREL, DT_RELA)
	      || !add_dynamic_entry (DT_JMPREL, 0))
	    return false;
	}

      if (!add_dynamic_entry (DT_RELA, 0)
	  || !add_dynamic_entry (DT_RELASZ, 0)
	  || !add_dynamic_entry (DT_RELAENT, sizeof (ElfNN_External_Rela)))
	return false;

      /* If any dynamic relocs apply to a read-only section,
	 then we need a DT_TEXTREL entry.  */
      if ((info->flags & DF_TEXTREL) == 0)
	elf_link_hash_traverse (&htab->elf, maybe_set_textrel, info);

      if (info->flags & DF_TEXTREL)
	{
	  if (!add_dynamic_entry (DT_TEXTREL, 0))
	    return false;
	  /* Clear the DF_TEXTREL flag.  It will be set again if we
	     write out an actual text relocation; we may not, because
	     at this point we do not know whether e.g.  any .eh_frame
	     absolute relocations have been converted to PC-relative.  */
	  info->flags &= ~DF_TEXTREL;
	}
    }
#undef add_dynamic_entry

  return true;
}

#define LARCH_LD_STACK_DEPTH 16
static int64_t larch_opc_stack[LARCH_LD_STACK_DEPTH];
static size_t larch_stack_top = 0;

static bfd_reloc_status_type
loongarch_push (int64_t val)
{
  if (LARCH_LD_STACK_DEPTH <= larch_stack_top)
    return bfd_reloc_outofrange;
  larch_opc_stack[larch_stack_top++] = val;
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
loongarch_pop (int64_t *val)
{
  if (larch_stack_top == 0)
    return bfd_reloc_outofrange;
  BFD_ASSERT (val);
  *val = larch_opc_stack[--larch_stack_top];
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
loongarch_top (int64_t *val)
{
  if (larch_stack_top == 0)
    return bfd_reloc_outofrange;
  BFD_ASSERT (val);
  *val = larch_opc_stack[larch_stack_top - 1];
  return bfd_reloc_ok;
}

static void
loongarch_elf_append_rela (bfd *abfd, asection *s, Elf_Internal_Rela *rel)
{
  BFD_ASSERT (s && s->contents);
  const struct elf_backend_data *bed;
  bfd_byte *loc;

  bed = get_elf_backend_data (abfd);
  if (!(s->size > s->reloc_count * bed->s->sizeof_rela))
    BFD_ASSERT (s->size > s->reloc_count * bed->s->sizeof_rela);
  loc = s->contents + (s->reloc_count++ * bed->s->sizeof_rela);
  bed->s->swap_reloca_out (abfd, rel, loc);
}

/* Check rel->r_offset in range of contents.  */
static bfd_reloc_status_type
loongarch_check_offset (const Elf_Internal_Rela *rel,
			const asection *input_section)
{
  if (0 == strcmp(input_section->name, ".text")
      && rel->r_offset > input_section->size)
    return bfd_reloc_overflow;

  return bfd_reloc_ok;
}

#define LARCH_RELOC_PERFORM_3OP(op1, op2, op3)	      \
  ({						      \
    bfd_reloc_status_type ret = loongarch_pop (&op2); \
    if (ret == bfd_reloc_ok)			      \
      {						      \
	ret = loongarch_pop (&op1);		      \
	if (ret == bfd_reloc_ok)		      \
	  ret = loongarch_push (op3);		      \
      }						      \
    ret;					      \
   })

static bfd_reloc_status_type
loongarch_reloc_rewrite_imm_insn (const Elf_Internal_Rela *rel,
				  const asection *input_section ATTRIBUTE_UNUSED,
				  reloc_howto_type *howto, bfd *input_bfd,
				  bfd_byte *contents, bfd_vma reloc_val)
{
  int bits = bfd_get_reloc_size (howto) * 8;
  uint32_t insn = bfd_get (bits, input_bfd, contents + rel->r_offset);

  if (!loongarch_adjust_reloc_bitsfield (input_bfd, howto, &reloc_val))
    return bfd_reloc_overflow;

  insn = (insn & (uint32_t)howto->src_mask)
    | ((insn & (~(uint32_t)howto->dst_mask)) | reloc_val);

  bfd_put (bits, input_bfd, insn, contents + rel->r_offset);

  return bfd_reloc_ok;
}

static bfd_reloc_status_type
perform_relocation (const Elf_Internal_Rela *rel, asection *input_section,
		    reloc_howto_type *howto, bfd_vma value,
		    bfd *input_bfd, bfd_byte *contents)
{
  int64_t opr1, opr2, opr3;
  bfd_reloc_status_type r = bfd_reloc_ok;
  int bits = bfd_get_reloc_size (howto) * 8;

  switch (ELFNN_R_TYPE (rel->r_info))
    {
    case R_LARCH_SOP_PUSH_PCREL:
    case R_LARCH_SOP_PUSH_ABSOLUTE:
    case R_LARCH_SOP_PUSH_GPREL:
    case R_LARCH_SOP_PUSH_TLS_TPREL:
    case R_LARCH_SOP_PUSH_TLS_GOT:
    case R_LARCH_SOP_PUSH_TLS_GD:
    case R_LARCH_SOP_PUSH_PLT_PCREL:
      r = loongarch_push (value);
      break;

    case R_LARCH_SOP_PUSH_DUP:
      r = loongarch_pop (&opr1);
      if (r == bfd_reloc_ok)
	{
	  r = loongarch_push (opr1);
	  if (r == bfd_reloc_ok)
	    r = loongarch_push (opr1);
	}
      break;

    case R_LARCH_SOP_ASSERT:
      r = loongarch_pop (&opr1);
      if (r != bfd_reloc_ok || !opr1)
	r = bfd_reloc_notsupported;
      break;

    case R_LARCH_SOP_NOT:
      r = loongarch_pop (&opr1);
      if (r == bfd_reloc_ok)
	r = loongarch_push (!opr1);
      break;

    case R_LARCH_SOP_SUB:
      r = LARCH_RELOC_PERFORM_3OP (opr1, opr2, opr1 - opr2);
      break;

    case R_LARCH_SOP_SL:
      r = LARCH_RELOC_PERFORM_3OP (opr1, opr2, opr1 << opr2);
      break;

    case R_LARCH_SOP_SR:
      r = LARCH_RELOC_PERFORM_3OP (opr1, opr2, opr1 >> opr2);
      break;

    case R_LARCH_SOP_AND:
      r = LARCH_RELOC_PERFORM_3OP (opr1, opr2, opr1 & opr2);
      break;

    case R_LARCH_SOP_ADD:
      r = LARCH_RELOC_PERFORM_3OP (opr1, opr2, opr1 + opr2);
      break;

    case R_LARCH_SOP_IF_ELSE:
      r = loongarch_pop (&opr3);
      if (r == bfd_reloc_ok)
	{
	  r = loongarch_pop (&opr2);
	  if (r == bfd_reloc_ok)
	    {
	      r = loongarch_pop (&opr1);
	      if (r == bfd_reloc_ok)
		r = loongarch_push (opr1 ? opr2 : opr3);
	    }
	}
      break;

    case R_LARCH_SOP_POP_32_S_10_5:
    case R_LARCH_SOP_POP_32_S_10_12:
    case R_LARCH_SOP_POP_32_S_10_16:
    case R_LARCH_SOP_POP_32_S_10_16_S2:
    case R_LARCH_SOP_POP_32_S_0_5_10_16_S2:
    case R_LARCH_SOP_POP_32_S_0_10_10_16_S2:
    case R_LARCH_SOP_POP_32_S_5_20:
    case R_LARCH_SOP_POP_32_U_10_12:
    case R_LARCH_SOP_POP_32_U:
      r = loongarch_pop (&opr1);
      if (r != bfd_reloc_ok)
	break;
      r = loongarch_check_offset (rel, input_section);
      if (r != bfd_reloc_ok)
	break;

      r = loongarch_reloc_rewrite_imm_insn (rel, input_section,
					    howto, input_bfd,
					    contents, (bfd_vma)opr1);
      break;

    case R_LARCH_TLS_DTPREL32:
    case R_LARCH_32:
    case R_LARCH_TLS_DTPREL64:
    case R_LARCH_64:
      r = loongarch_check_offset (rel, input_section);
      if (r != bfd_reloc_ok)
	break;

      bfd_put (bits, input_bfd, value, contents + rel->r_offset);
      break;

    /* LoongArch only has add/sub reloc pair, not has set/sub reloc pair.
       Because set/sub reloc pair not support multi-thread. While add/sub
       reloc pair process order not affect the final result.

       For add/sub reloc, the original value will be involved in the
       calculation. In order not to add/sub extra value, we write 0 to symbol
       address at assembly time.

       add/sub reloc bits determined by the value after symbol subtraction,
       not symbol value.

       add/sub reloc save part of the symbol value, so we only need to
       save howto->dst_mask bits.  */
    case R_LARCH_ADD6:
    case R_LARCH_SUB6:
      {
	bfd_vma word = bfd_get (howto->bitsize, input_bfd,
				contents + rel->r_offset);
	word = (word & ~howto->dst_mask) | (value & howto->dst_mask);
	bfd_put (howto->bitsize, input_bfd, word, contents + rel->r_offset);
	r = bfd_reloc_ok;
	break;
      }

    /* Not need to read the original value, just write the new value.  */
    case R_LARCH_ADD8:
    case R_LARCH_ADD16:
    case R_LARCH_ADD24:
    case R_LARCH_ADD32:
    case R_LARCH_ADD64:
    case R_LARCH_SUB8:
    case R_LARCH_SUB16:
    case R_LARCH_SUB24:
    case R_LARCH_SUB32:
    case R_LARCH_SUB64:
      {
	/* Because add/sub reloc is processed separately,
	   so the high bits is invalid.  */
	bfd_vma word = value & howto->dst_mask;
	bfd_put (howto->bitsize, input_bfd, word, contents + rel->r_offset);
	r = bfd_reloc_ok;
	break;
      }

    case R_LARCH_ADD_ULEB128:
    case R_LARCH_SUB_ULEB128:
      {
	unsigned int len = 0;
	/* Before write uleb128, first read it to get it's length.  */
	_bfd_read_unsigned_leb128 (input_bfd, contents + rel->r_offset, &len);
	loongarch_write_unsigned_leb128 (contents + rel->r_offset, len, value);
	r = bfd_reloc_ok;
	break;
      }

    /* For eh_frame and debug info.  */
    case R_LARCH_32_PCREL:
    case R_LARCH_64_PCREL:
      {
	value -= sec_addr (input_section) + rel->r_offset;
	value += rel->r_addend;
	bfd_vma word = bfd_get (howto->bitsize, input_bfd,
				contents + rel->r_offset);
	word = (word & ~howto->dst_mask) | (value & howto->dst_mask);
	bfd_put (howto->bitsize, input_bfd, word, contents + rel->r_offset);
	r = bfd_reloc_ok;
	break;
      }

    /* New reloc type.
       R_LARCH_B16 ~ R_LARCH_TLS_GD_HI20.  */
    case R_LARCH_B16:
    case R_LARCH_B21:
    case R_LARCH_B26:
    case R_LARCH_ABS_HI20:
    case R_LARCH_ABS_LO12:
    case R_LARCH_ABS64_LO20:
    case R_LARCH_ABS64_HI12:
    case R_LARCH_PCALA_HI20:
    case R_LARCH_PCALA_LO12:
    case R_LARCH_PCALA64_LO20:
    case R_LARCH_PCALA64_HI12:
    case R_LARCH_GOT_PC_HI20:
    case R_LARCH_GOT_PC_LO12:
    case R_LARCH_GOT64_PC_LO20:
    case R_LARCH_GOT64_PC_HI12:
    case R_LARCH_GOT_HI20:
    case R_LARCH_GOT_LO12:
    case R_LARCH_GOT64_LO20:
    case R_LARCH_GOT64_HI12:
    case R_LARCH_TLS_LE_HI20:
    case R_LARCH_TLS_LE_LO12:
    case R_LARCH_TLS_LE64_LO20:
    case R_LARCH_TLS_LE64_HI12:
    case R_LARCH_TLS_IE_PC_HI20:
    case R_LARCH_TLS_IE_PC_LO12:
    case R_LARCH_TLS_IE64_PC_LO20:
    case R_LARCH_TLS_IE64_PC_HI12:
    case R_LARCH_TLS_IE_HI20:
    case R_LARCH_TLS_IE_LO12:
    case R_LARCH_TLS_IE64_LO20:
    case R_LARCH_TLS_IE64_HI12:
    case R_LARCH_TLS_LD_PC_HI20:
    case R_LARCH_TLS_LD_HI20:
    case R_LARCH_TLS_GD_PC_HI20:
    case R_LARCH_TLS_GD_HI20:
    case R_LARCH_PCREL20_S2:
      r = loongarch_check_offset (rel, input_section);
      if (r != bfd_reloc_ok)
	break;

      r = loongarch_reloc_rewrite_imm_insn (rel, input_section,
					    howto, input_bfd,
					    contents, value);
      break;

    case R_LARCH_RELAX:
      break;

    default:
      r = bfd_reloc_notsupported;
    }
  return r;
}

#define LARCH_RECENT_RELOC_QUEUE_LENGTH 72
static struct
{
  bfd *bfd;
  asection *section;
  bfd_vma r_offset;
  int r_type;
  bfd_vma relocation;
  Elf_Internal_Sym *sym;
  struct elf_link_hash_entry *h;
  bfd_vma addend;
  int64_t top_then;
} larch_reloc_queue[LARCH_RECENT_RELOC_QUEUE_LENGTH];
static size_t larch_reloc_queue_head = 0;
static size_t larch_reloc_queue_tail = 0;

static const char *
loongarch_sym_name (bfd *input_bfd, struct elf_link_hash_entry *h,
		    Elf_Internal_Sym *sym)
{
  const char *ret = NULL;
  if (sym)
    ret = bfd_elf_string_from_elf_section (input_bfd,
					   elf_symtab_hdr (input_bfd).sh_link,
					   sym->st_name);
  else if (h)
    ret = h->root.root.string;

  if (ret == NULL || *ret == '\0')
    ret = "<nameless>";
  return ret;
}

static void
loongarch_record_one_reloc (bfd *abfd, asection *section, int r_type,
			    bfd_vma r_offset, Elf_Internal_Sym *sym,
			    struct elf_link_hash_entry *h, bfd_vma addend)
{
  if ((larch_reloc_queue_head == 0
       && larch_reloc_queue_tail == LARCH_RECENT_RELOC_QUEUE_LENGTH - 1)
      || larch_reloc_queue_head == larch_reloc_queue_tail + 1)
    larch_reloc_queue_head =
      (larch_reloc_queue_head + 1) % LARCH_RECENT_RELOC_QUEUE_LENGTH;
  larch_reloc_queue[larch_reloc_queue_tail].bfd = abfd;
  larch_reloc_queue[larch_reloc_queue_tail].section = section;
  larch_reloc_queue[larch_reloc_queue_tail].r_offset = r_offset;
  larch_reloc_queue[larch_reloc_queue_tail].r_type = r_type;
  larch_reloc_queue[larch_reloc_queue_tail].sym = sym;
  larch_reloc_queue[larch_reloc_queue_tail].h = h;
  larch_reloc_queue[larch_reloc_queue_tail].addend = addend;
  loongarch_top (&larch_reloc_queue[larch_reloc_queue_tail].top_then);
  larch_reloc_queue_tail =
    (larch_reloc_queue_tail + 1) % LARCH_RECENT_RELOC_QUEUE_LENGTH;
}

static void
loongarch_dump_reloc_record (void (*p) (const char *fmt, ...))
{
  size_t i = larch_reloc_queue_head;
  bfd *a_bfd = NULL;
  asection *section = NULL;
  bfd_vma r_offset = 0;
  int inited = 0;
  p ("Dump relocate record:\n");
  p ("stack top\t\trelocation name\t\tsymbol");
  while (i != larch_reloc_queue_tail)
    {
      if (a_bfd != larch_reloc_queue[i].bfd
	  || section != larch_reloc_queue[i].section
	  || r_offset != larch_reloc_queue[i].r_offset)
	{
	  a_bfd = larch_reloc_queue[i].bfd;
	  section = larch_reloc_queue[i].section;
	  r_offset = larch_reloc_queue[i].r_offset;
	  p ("\nat %pB(%pA+0x%v):\n", larch_reloc_queue[i].bfd,
	     larch_reloc_queue[i].section, larch_reloc_queue[i].r_offset);
	}

      if (!inited)
	inited = 1, p ("...\n");

      reloc_howto_type *howto =
	loongarch_elf_rtype_to_howto (larch_reloc_queue[i].bfd,
				      larch_reloc_queue[i].r_type);
      p ("0x%V %s\t`%s'", (bfd_vma) larch_reloc_queue[i].top_then,
	 howto ? howto->name : "<unknown reloc>",
	 loongarch_sym_name (larch_reloc_queue[i].bfd, larch_reloc_queue[i].h,
			     larch_reloc_queue[i].sym));

      long addend = larch_reloc_queue[i].addend;
      if (addend < 0)
	p (" - %ld", -addend);
      else if (0 < addend)
	p (" + %ld(0x%v)", addend, larch_reloc_queue[i].addend);

      p ("\n");
      i = (i + 1) % LARCH_RECENT_RELOC_QUEUE_LENGTH;
    }
  p ("\n"
     "-- Record dump end --\n\n");
}

static bool
loongarch_reloc_is_fatal (struct bfd_link_info *info,
			  bfd *input_bfd,
			  asection *input_section,
			  Elf_Internal_Rela *rel,
			  reloc_howto_type *howto,
			  bfd_reloc_status_type rtype,
			  bool is_undefweak,
			  const char *name,
			  const char *msg)
{
  bool fatal = true;
  switch (rtype)
    {
      /* 'dangerous' means we do it but can't promise it's ok
	 'unsupport' means out of ability of relocation type
	 'undefined' means we can't deal with the undefined symbol.  */
    case bfd_reloc_undefined:
      info->callbacks->undefined_symbol (info, name, input_bfd, input_section,
					 rel->r_offset, true);
      info->callbacks->info ("%X%pB(%pA+0x%v): error: %s against %s`%s':\n%s\n",
			     input_bfd, input_section, rel->r_offset,
			     howto->name,
			     is_undefweak ? "[undefweak] " : "", name, msg);
      break;
    case bfd_reloc_dangerous:
      info->callbacks->info ("%pB(%pA+0x%v): warning: %s against %s`%s':\n%s\n",
			     input_bfd, input_section, rel->r_offset,
			     howto->name,
			     is_undefweak ? "[undefweak] " : "", name, msg);
      fatal = false;
      break;
    case bfd_reloc_notsupported:
      info->callbacks->info ("%X%pB(%pA+0x%v): error: %s against %s`%s':\n%s\n",
			     input_bfd, input_section, rel->r_offset,
			     howto->name,
			     is_undefweak ? "[undefweak] " : "", name, msg);
      break;
    default:
      break;
    }
  return fatal;
}

/* If lo12 immediate > 0x7ff, because sign-extend caused by addi.d/ld.d,
   hi20 immediate need to add 0x1.
   For example: pc 0x120000000, symbol 0x120000812
   lo12 immediate is 0x812, 0x120000812 & 0xfff = 0x812
   hi20 immediate is 1, because lo12 imm > 0x7ff, symbol need to add 0x1000
   (((0x120000812 + 0x1000) & ~0xfff) - (0x120000000 & ~0xfff)) >> 12 = 0x1

   At run:
   pcalau12i $t0, hi20 (0x1)
      $t0 = 0x120000000 + (0x1 << 12) = 0x120001000
   addi.d $t0, $t0, lo12 (0x812)
      $t0 = 0x120001000 + 0xfffffffffffff812 (-(0x1000 - 0x812) = -0x7ee)
	  = 0x120001000 - 0x7ee (0x1000 - 0x7ee = 0x812)
	  = 0x120000812
    Without hi20 add 0x1000, the result 0x120000000 - 0x7ee = 0x11ffff812 is
    error.
    0x1000 + sign-extend-to64(0x8xx) = 0x8xx.  */
#define RELOCATE_CALC_PC32_HI20(relocation, pc) 	\
  ({							\
    bfd_vma __lo = (relocation) & ((bfd_vma)0xfff);	\
    relocation = (relocation & ~(bfd_vma)0xfff)		\
		  - (pc & ~(bfd_vma)0xfff);		\
    if (__lo > 0x7ff)					\
	relocation += 0x1000;				\
  })

/* For example: pc is 0x11000010000100, symbol is 0x1812348ffff812
   offset = (0x1812348ffff812 & ~0xfff) - (0x11000010000100 & ~0xfff)
	  = 0x712347ffff000
   lo12: 0x1812348ffff812 & 0xfff = 0x812
   hi20: 0x7ffff + 0x1(lo12 > 0x7ff) = 0x80000
   lo20: 0x71234 - 0x1(lo12 > 0x7ff) + 0x1(hi20 > 0x7ffff)
   hi12: 0x0

   pcalau12i $t1, hi20 (0x80000)
      $t1 = 0x11000010000100 + sign-extend(0x80000 << 12)
	  = 0x11000010000100 + 0xffffffff80000000
	  = 0x10ffff90000000
   addi.d $t0, $zero, lo12 (0x812)
      $t0 = 0xfffffffffffff812 (if lo12 > 0x7ff, because sign-extend,
      lo20 need to sub 0x1)
   lu32i.d $t0, lo12 (0x71234)
      $t0 = {0x71234, 0xfffff812}
	  = 0x71234fffff812
   lu52i.d $t0, hi12 (0x0)
      $t0 = {0x0, 0x71234fffff812}
	  = 0x71234fffff812
   add.d $t1, $t1, $t0
      $t1 = 0x10ffff90000000 + 0x71234fffff812
	  = 0x1812348ffff812.  */
#define RELOCATE_CALC_PC64_HI32(relocation, pc)  	\
  ({							\
    bfd_vma __lo = (relocation & (bfd_vma)0xfff);	\
    relocation = (relocation & ~(bfd_vma)0xfff)		\
		  - (pc & ~(bfd_vma)0xfff);		\
    if (__lo > 0x7ff)					\
	relocation += (0x1000 - 0x100000000);		\
    if (relocation & 0x80000000)			\
      relocation += 0x100000000;			\
  })

static int
loongarch_elf_relocate_section (bfd *output_bfd, struct bfd_link_info *info,
				bfd *input_bfd, asection *input_section,
				bfd_byte *contents, Elf_Internal_Rela *relocs,
				Elf_Internal_Sym *local_syms,
				asection **local_sections)
{
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  bool fatal = false;
  asection *sreloc = elf_section_data (input_section)->sreloc;
  struct loongarch_elf_link_hash_table *htab = loongarch_elf_hash_table (info);
  Elf_Internal_Shdr *symtab_hdr = &elf_symtab_hdr (input_bfd);
  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (input_bfd);
  bfd_vma *local_got_offsets = elf_local_got_offsets (input_bfd);
  bool is_pic = bfd_link_pic (info);
  bool is_dyn = elf_hash_table (info)->dynamic_sections_created;
  asection *plt = htab->elf.splt ? htab->elf.splt : htab->elf.iplt;
  asection *got = htab->elf.sgot;

  relend = relocs + input_section->reloc_count;
  for (rel = relocs; rel < relend; rel++)
    {
      int r_type = ELFNN_R_TYPE (rel->r_info);
      unsigned long r_symndx = ELFNN_R_SYM (rel->r_info);
      bfd_vma pc = sec_addr (input_section) + rel->r_offset;
      reloc_howto_type *howto = NULL;
      asection *sec = NULL;
      Elf_Internal_Sym *sym = NULL;
      struct elf_link_hash_entry *h = NULL;
      const char *name;
      bfd_reloc_status_type r = bfd_reloc_ok;
      bool is_ie, is_undefweak, unresolved_reloc, defined_local;
      bool resolved_local, resolved_dynly, resolved_to_const;
      char tls_type;
      bfd_vma relocation, off, ie_off;
      int i, j;

      howto = loongarch_elf_rtype_to_howto (input_bfd, r_type);
      if (howto == NULL || r_type == R_LARCH_GNU_VTINHERIT
	  || r_type == R_LARCH_GNU_VTENTRY)
	continue;

      /* This is a final link.  */
      if (r_symndx < symtab_hdr->sh_info)
	{
	  is_undefweak = false;
	  unresolved_reloc = false;
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

	  /* Relocate against local STT_GNU_IFUNC symbol.  */
	  if (!bfd_link_relocatable (info)
	      && ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
	    {
	      h = elfNN_loongarch_get_local_sym_hash (htab, input_bfd, rel,
						      false);
	      if (h == NULL)
		abort ();

	      /* Set STT_GNU_IFUNC symbol value.  */
	      h->root.u.def.value = sym->st_value;
	      h->root.u.def.section = sec;
	    }
	  defined_local = true;
	  resolved_local = true;
	  resolved_dynly = false;
	  resolved_to_const = false;

	  /* Calc in funtion elf_link_input_bfd,
	   * if #define elf_backend_rela_normal to 1.  */
	  if (bfd_link_relocatable (info)
	      && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    continue;
	}
      else
	{
	  bool warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);
	  /* Here means symbol isn't local symbol only and 'h != NULL'.  */

	  /* The 'unresolved_syms_in_objects' specify how to deal with undefined
	     symbol.  And 'dynamic_undefined_weak' specify what to do when
	     meeting undefweak.  */

	  if ((is_undefweak = h->root.type == bfd_link_hash_undefweak))
	    {
	      defined_local = false;
	      resolved_local = false;
	      resolved_to_const = (!is_dyn || h->dynindx == -1
				   || UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));
	      resolved_dynly = !resolved_local && !resolved_to_const;
	    }
	  else if (warned)
	    {
	      /* Symbol undefined offen means failed already.  I don't know why
		 'warned' here but I guess it want to continue relocating as if
		 no error occures to find other errors as more as possible.  */

	      /* To avoid generating warning messages about truncated
		 relocations, set the relocation's address to be the same as
		 the start of this section.  */
	      relocation = (input_section->output_section
			    ? input_section->output_section->vma
			    : 0);

	      defined_local = relocation != 0;
	      resolved_local = defined_local;
	      resolved_to_const = !resolved_local;
	      resolved_dynly = false;
	    }
	  else
	    {
	      defined_local = !unresolved_reloc && !ignored;
	      resolved_local =
		defined_local && SYMBOL_REFERENCES_LOCAL (info, h);
	      resolved_dynly = !resolved_local;
	      resolved_to_const = !resolved_local && !resolved_dynly;
	    }
	}

      name = loongarch_sym_name (input_bfd, h, sym);

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section, rel,
					 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      /* The r_symndx will be STN_UNDEF (zero) only for relocs against symbols
	 from removed linkonce sections, or sections discarded by a linker
	 script.  Also for R_*_SOP_PUSH_ABSOLUTE and PCREL to specify const.  */
      if (r_symndx == STN_UNDEF || bfd_is_abs_section (sec))
	{
	  defined_local = false;
	  resolved_local = false;
	  resolved_dynly = false;
	  resolved_to_const = true;
	}

      /* The ifunc reference generate plt.  */
      if (h && h->type == STT_GNU_IFUNC && h->plt.offset != MINUS_ONE)
	{
	  defined_local = true;
	  resolved_local = true;
	  resolved_dynly = false;
	  resolved_to_const = false;
	  relocation = sec_addr (plt) + h->plt.offset;
	}

      unresolved_reloc = resolved_dynly;

      BFD_ASSERT (resolved_local + resolved_dynly + resolved_to_const == 1);

      /* BFD_ASSERT (!resolved_dynly || (h && h->dynindx != -1));.  */

      BFD_ASSERT (!resolved_local || defined_local);

      is_ie = false;
      switch (r_type)
	{
	case R_LARCH_MARK_PCREL:
	case R_LARCH_MARK_LA:
	case R_LARCH_NONE:
	  r = bfd_reloc_continue;
	  unresolved_reloc = false;
	  break;

	case R_LARCH_32:
	case R_LARCH_64:
	  if (resolved_dynly || (is_pic && resolved_local))
	    {
	      Elf_Internal_Rela outrel;

	      /* When generating a shared object, these relocations are copied
		 into the output file to be resolved at run time.  */

	      outrel.r_offset = _bfd_elf_section_offset (output_bfd, info,
							 input_section,
							 rel->r_offset);

	      unresolved_reloc = (!((bfd_vma) -2 <= outrel.r_offset)
				  && (input_section->flags & SEC_ALLOC));

	      outrel.r_offset += sec_addr (input_section);

	      /* A pointer point to a ifunc symbol.  */
	      if (h && h->type == STT_GNU_IFUNC)
		{
		  if (h->dynindx == -1)
		    {
		      outrel.r_info = ELFNN_R_INFO (0, R_LARCH_IRELATIVE);
		      outrel.r_addend = (h->root.u.def.value
				  + h->root.u.def.section->output_section->vma
				  + h->root.u.def.section->output_offset);
		    }
		  else
		    {
		      outrel.r_info = ELFNN_R_INFO (h->dynindx, R_LARCH_NN);
		      outrel.r_addend = 0;
		    }

		  if (SYMBOL_REFERENCES_LOCAL (info, h))
		    {

		      if (htab->elf.splt != NULL)
			sreloc = htab->elf.srelgot;
		      else
			sreloc = htab->elf.irelplt;
		    }
		  else
		    {

		      if (bfd_link_pic (info))
			sreloc = htab->elf.irelifunc;
		      else if (htab->elf.splt != NULL)
			sreloc = htab->elf.srelgot;
		      else
			sreloc = htab->elf.irelplt;
		    }
		}
	      else if (resolved_dynly)
		{
		  if (h->dynindx == -1)
		    {
		      if (h->root.type == bfd_link_hash_undefined)
			(*info->callbacks->undefined_symbol)
			  (info, name, input_bfd, input_section,
			   rel->r_offset, true);

		      outrel.r_info = ELFNN_R_INFO (0, r_type);
		    }
		  else
		    outrel.r_info = ELFNN_R_INFO (h->dynindx, r_type);

		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  outrel.r_info = ELFNN_R_INFO (0, R_LARCH_RELATIVE);
		  outrel.r_addend = relocation + rel->r_addend;
		}

	      /* No alloc space of func allocate_dynrelocs.  */
	      if (unresolved_reloc
		  && !(h && (h->is_weakalias || !h->dyn_relocs)))
		loongarch_elf_append_rela (output_bfd, sreloc, &outrel);
	    }

	  relocation += rel->r_addend;
	  break;

	case R_LARCH_ADD6:
	case R_LARCH_ADD8:
	case R_LARCH_ADD16:
	case R_LARCH_ADD24:
	case R_LARCH_ADD32:
	case R_LARCH_ADD64:
	  {
	    bfd_vma old_value = bfd_get (howto->bitsize, input_bfd,
					 contents + rel->r_offset);
	    relocation = old_value + relocation + rel->r_addend;
	    break;
	  }

	case R_LARCH_SUB6:
	case R_LARCH_SUB8:
	case R_LARCH_SUB16:
	case R_LARCH_SUB24:
	case R_LARCH_SUB32:
	case R_LARCH_SUB64:
	  {
	    bfd_vma old_value = bfd_get (howto->bitsize, input_bfd,
					  contents + rel->r_offset);
	    relocation = old_value - relocation - rel->r_addend;
	    break;
	  }

	case R_LARCH_ADD_ULEB128:
	case R_LARCH_SUB_ULEB128:
	  {
	    /* Get the value and length of the uleb128 data.  */
	    unsigned int len = 0;
	    bfd_vma old_value = _bfd_read_unsigned_leb128 (input_bfd,
				    contents + rel->r_offset, &len);

	    if (R_LARCH_ADD_ULEB128 == ELFNN_R_TYPE (rel->r_info))
	      relocation = old_value + relocation + rel->r_addend;
	    else if (R_LARCH_SUB_ULEB128 == ELFNN_R_TYPE (rel->r_info))
	      relocation = old_value - relocation - rel->r_addend;

	    bfd_vma mask = (1 << (7 * len)) - 1;
	    relocation &= mask;
	    break;
	  }

	case R_LARCH_TLS_DTPREL32:
	case R_LARCH_TLS_DTPREL64:
	  if (resolved_dynly)
	    {
	      Elf_Internal_Rela outrel;

	      outrel.r_offset = _bfd_elf_section_offset (output_bfd, info,
							 input_section,
							 rel->r_offset);
	      unresolved_reloc = (!((bfd_vma) -2 <= outrel.r_offset)
				  && (input_section->flags & SEC_ALLOC));
	      outrel.r_info = ELFNN_R_INFO (h->dynindx, r_type);
	      outrel.r_offset += sec_addr (input_section);
	      outrel.r_addend = rel->r_addend;
	      if (unresolved_reloc)
		loongarch_elf_append_rela (output_bfd, sreloc, &outrel);
	      break;
	    }

	  if (resolved_to_const)
	    fatal = loongarch_reloc_is_fatal (info, input_bfd, input_section,
					      rel, howto,
					      bfd_reloc_notsupported,
					      is_undefweak, name,
					      "Internal:");
	  if (resolved_local)
	    {
	      if (!elf_hash_table (info)->tls_sec)
		{
		fatal = loongarch_reloc_is_fatal (info, input_bfd,
			  input_section, rel, howto, bfd_reloc_notsupported,
			  is_undefweak, name, "TLS section not be created");
		}
	      else
		relocation -= elf_hash_table (info)->tls_sec->vma;
	    }
	  else
	    {
	    fatal = loongarch_reloc_is_fatal (info, input_bfd,
		      input_section, rel, howto, bfd_reloc_undefined,
		      is_undefweak, name,
		      "TLS LE just can be resolved local only.");
	    }

	  break;

	case R_LARCH_SOP_PUSH_TLS_TPREL:
	  if (resolved_local)
	    {
	      if (!elf_hash_table (info)->tls_sec)
		fatal = (loongarch_reloc_is_fatal
			 (info, input_bfd, input_section, rel, howto,
			  bfd_reloc_notsupported, is_undefweak, name,
			  "TLS section not be created"));
	      else
		relocation -= elf_hash_table (info)->tls_sec->vma;
	    }
	  else
	    fatal = (loongarch_reloc_is_fatal
		     (info, input_bfd, input_section, rel, howto,
		      bfd_reloc_undefined, is_undefweak, name,
		      "TLS LE just can be resolved local only."));
	  break;

	case R_LARCH_SOP_PUSH_ABSOLUTE:
	  if (is_undefweak)
	    {
	      if (resolved_dynly)
		fatal = (loongarch_reloc_is_fatal
			 (info, input_bfd, input_section, rel, howto,
			  bfd_reloc_dangerous, is_undefweak, name,
			  "Someone require us to resolve undefweak "
			  "symbol dynamically.  \n"
			  "But this reloc can't be done.  "
			  "I think I can't throw error "
			  "for this\n"
			  "so I resolved it to 0.  "
			  "I suggest to re-compile with '-fpic'."));

	      relocation = 0;
	      unresolved_reloc = false;
	      break;
	    }

	  if (resolved_to_const)
	    {
	      relocation += rel->r_addend;
	      break;
	    }

	  if (is_pic)
	    {
	      fatal = (loongarch_reloc_is_fatal
		       (info, input_bfd, input_section, rel, howto,
			bfd_reloc_notsupported, is_undefweak, name,
			"Under PIC we don't know load address.  Re-compile "
			"with '-fpic'?"));
	      break;
	    }

	  if (resolved_dynly)
	    {
	      if (!(plt && h && h->plt.offset != MINUS_ONE))
		{
		  fatal = (loongarch_reloc_is_fatal
			   (info, input_bfd, input_section, rel, howto,
			    bfd_reloc_undefined, is_undefweak, name,
			    "Can't be resolved dynamically.  Try to re-compile "
			    "with '-fpic'?"));
		  break;
		}

	      if (rel->r_addend != 0)
		{
		  fatal = (loongarch_reloc_is_fatal
			   (info, input_bfd, input_section, rel, howto,
			    bfd_reloc_notsupported, is_undefweak, name,
			    "Shouldn't be with r_addend."));
		  break;
		}

	      relocation = sec_addr (plt) + h->plt.offset;
	      unresolved_reloc = false;
	      break;
	    }

	  if (resolved_local)
	    {
	      relocation += rel->r_addend;
	      break;
	    }

	  break;

	case R_LARCH_SOP_PUSH_PCREL:
	case R_LARCH_SOP_PUSH_PLT_PCREL:
	  unresolved_reloc = false;

	  if (is_undefweak)
	    {
	      i = 0, j = 0;
	      relocation = 0;
	      if (resolved_dynly)
		{
		  if (h && h->plt.offset != MINUS_ONE)
		    i = 1, j = 2;
		  else
		    fatal = (loongarch_reloc_is_fatal
			     (info, input_bfd, input_section, rel, howto,
			      bfd_reloc_dangerous, is_undefweak, name,
			      "Undefweak need to be resolved dynamically, "
			      "but PLT stub doesn't represent."));
		}
	    }
	  else
	    {
	      if (!(defined_local || (h && h->plt.offset != MINUS_ONE)))
		{
		  fatal = (loongarch_reloc_is_fatal
			   (info, input_bfd, input_section, rel, howto,
			    bfd_reloc_undefined, is_undefweak, name,
			    "PLT stub does not represent and "
			    "symbol not defined."));
		  break;
		}

	      if (resolved_local)
		i = 0, j = 2;
	      else /* if (resolved_dynly) */
		{
		  if (!(h && h->plt.offset != MINUS_ONE))
		    fatal = (loongarch_reloc_is_fatal
			     (info, input_bfd, input_section, rel, howto,
			      bfd_reloc_dangerous, is_undefweak, name,
			      "Internal: PLT stub doesn't represent.  "
			      "Resolve it with pcrel"));
		  i = 1, j = 3;
		}
	    }

	  for (; i < j; i++)
	    {
	      if ((i & 1) == 0 && defined_local)
		{
		  relocation -= pc;
		  relocation += rel->r_addend;
		  break;
		}

	      if ((i & 1) && h && h->plt.offset != MINUS_ONE)
		{
		  if (rel->r_addend != 0)
		    {
		      fatal = (loongarch_reloc_is_fatal
			       (info, input_bfd, input_section, rel, howto,
				bfd_reloc_notsupported, is_undefweak, name,
				"PLT shouldn't be with r_addend."));
		      break;
		    }
		  relocation = sec_addr (plt) + h->plt.offset - pc;
		  break;
		}
	    }
	  break;

	case R_LARCH_SOP_PUSH_GPREL:
	  unresolved_reloc = false;

	  if (rel->r_addend != 0)
	    {
	      fatal = (loongarch_reloc_is_fatal
		       (info, input_bfd, input_section, rel, howto,
			bfd_reloc_notsupported, is_undefweak, name,
			"Shouldn't be with r_addend."));
	      break;
	    }

	  if (h != NULL)
	    {
	      off = h->got.offset & (~1);

	      if (h->got.offset == MINUS_ONE && h->type != STT_GNU_IFUNC)
		{
		  fatal = (loongarch_reloc_is_fatal
			   (info, input_bfd, input_section, rel, howto,
			    bfd_reloc_notsupported, is_undefweak, name,
			    "Internal: GOT entry doesn't represent."));
		  break;
		}

	      /* Hidden symbol not has .got entry, only .got.plt entry
		 so gprel is (plt - got).  */
	      if (h->got.offset == MINUS_ONE && h->type == STT_GNU_IFUNC)
		{
		  if (h->plt.offset == (bfd_vma) -1)
		    {
		      abort();
		    }

		  bfd_vma plt_index = h->plt.offset / PLT_ENTRY_SIZE;
		  off = plt_index * GOT_ENTRY_SIZE;

		  if (htab->elf.splt != NULL)
		    {
		      /* Section .plt header is 2 times of plt entry.  */
		      off = sec_addr (htab->elf.sgotplt) + off
			- sec_addr (htab->elf.sgot);
		    }
		  else
		    {
		      /* Section iplt not has plt header.  */
		      off = sec_addr (htab->elf.igotplt) + off
			- sec_addr (htab->elf.sgot);
		    }
		}

	      if ((h->got.offset & 1) == 0)
		{
		  if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (is_dyn,
							bfd_link_pic (info), h)
		      && ((bfd_link_pic (info)
			   && SYMBOL_REFERENCES_LOCAL (info, h))))
		    {
		      /* This is actually a static link, or it is a
			 -Bsymbolic link and the symbol is defined
			 locally, or the symbol was forced to be local
			 because of a version file.  We must initialize
			 this entry in the global offset table.  Since the
			 offset must always be a multiple of the word size,
			 we use the least significant bit to record whether
			 we have initialized it already.

			 When doing a dynamic link, we create a rela.got
			 relocation entry to initialize the value.  This
			 is done in the finish_dynamic_symbol routine.  */

		      if (resolved_dynly)
			{
			  fatal = (loongarch_reloc_is_fatal
				   (info, input_bfd, input_section, rel, howto,
				    bfd_reloc_dangerous, is_undefweak, name,
				    "Internal: here shouldn't dynamic."));
			}

		      if (!(defined_local || resolved_to_const))
			{
			  fatal = (loongarch_reloc_is_fatal
				   (info, input_bfd, input_section, rel, howto,
				    bfd_reloc_undefined, is_undefweak, name,
				    "Internal: "));
			  break;
			}

		      asection *s;
		      Elf_Internal_Rela outrel;
		      /* We need to generate a R_LARCH_RELATIVE reloc
			 for the dynamic linker.  */
		      s = htab->elf.srelgot;
		      if (!s)
			{
			  fatal = loongarch_reloc_is_fatal
			    (info, input_bfd,
			     input_section, rel, howto,
			     bfd_reloc_notsupported, is_undefweak, name,
			     "Internal: '.rel.got' not represent");
			  break;
			}

		      outrel.r_offset = sec_addr (got) + off;
		      outrel.r_info = ELFNN_R_INFO (0, R_LARCH_RELATIVE);
		      outrel.r_addend = relocation; /* Link-time addr.  */
		      loongarch_elf_append_rela (output_bfd, s, &outrel);
		    }
		  bfd_put_NN (output_bfd, relocation, got->contents + off);
		  h->got.offset |= 1;
		}
	    }
	  else
	    {
	      if (!local_got_offsets)
		{
		  fatal = (loongarch_reloc_is_fatal
			   (info, input_bfd, input_section, rel, howto,
			    bfd_reloc_notsupported, is_undefweak, name,
			    "Internal: local got offsets not reporesent."));
		  break;
		}

	      off = local_got_offsets[r_symndx] & (~1);

	      if (local_got_offsets[r_symndx] == MINUS_ONE)
		{
		  fatal = (loongarch_reloc_is_fatal
			   (info, input_bfd, input_section, rel, howto,
			    bfd_reloc_notsupported, is_undefweak, name,
			    "Internal: GOT entry doesn't represent."));
		  break;
		}

	      /* The offset must always be a multiple of the word size.
		 So, we can use the least significant bit to record
		 whether we have already processed this entry.  */
	      if ((local_got_offsets[r_symndx] & 1) == 0)
		{
		  if (is_pic)
		    {
		      asection *s;
		      Elf_Internal_Rela outrel;
		      /* We need to generate a R_LARCH_RELATIVE reloc
			 for the dynamic linker.  */
		      s = htab->elf.srelgot;
		      if (!s)
			{
			  fatal = (loongarch_reloc_is_fatal
				   (info, input_bfd, input_section, rel, howto,
				    bfd_reloc_notsupported, is_undefweak, name,
				    "Internal: '.rel.got' not represent"));
			  break;
			}

		      outrel.r_offset = sec_addr (got) + off;
		      outrel.r_info = ELFNN_R_INFO (0, R_LARCH_RELATIVE);
		      outrel.r_addend = relocation; /* Link-time addr.  */
		      loongarch_elf_append_rela (output_bfd, s, &outrel);
		    }

		  bfd_put_NN (output_bfd, relocation, got->contents + off);
		  local_got_offsets[r_symndx] |= 1;
		}
	    }
	  relocation = off;

	  break;

	case R_LARCH_SOP_PUSH_TLS_GOT:
	case R_LARCH_SOP_PUSH_TLS_GD:
	  {
	    unresolved_reloc = false;
	    if (r_type == R_LARCH_SOP_PUSH_TLS_GOT)
	      is_ie = true;

	    bfd_vma got_off = 0;
	    if (h != NULL)
	      {
		got_off = h->got.offset;
		h->got.offset |= 1;
	      }
	    else
	      {
		got_off = local_got_offsets[r_symndx];
		local_got_offsets[r_symndx] |= 1;
	      }

	    BFD_ASSERT (got_off != MINUS_ONE);

	    ie_off = 0;
	    tls_type = _bfd_loongarch_elf_tls_type (input_bfd, h, r_symndx);
	    if ((tls_type & GOT_TLS_GD) && (tls_type & GOT_TLS_IE))
	      ie_off = 2 * GOT_ENTRY_SIZE;

	    if ((got_off & 1) == 0)
	      {
		Elf_Internal_Rela rela;
		asection *srel = htab->elf.srelgot;
		bfd_vma tls_block_off = 0;

		if (SYMBOL_REFERENCES_LOCAL (info, h))
		  {
		    BFD_ASSERT (elf_hash_table (info)->tls_sec);
		    tls_block_off = relocation
			- elf_hash_table (info)->tls_sec->vma;
		  }

		if (tls_type & GOT_TLS_GD)
		  {
		    rela.r_offset = sec_addr (got) + got_off;
		    rela.r_addend = 0;
		    if (SYMBOL_REFERENCES_LOCAL (info, h))
		      {
			/* Local sym, used in exec, set module id 1.  */
			if (bfd_link_executable (info))
			  bfd_put_NN (output_bfd, 1, got->contents + got_off);
			else
			  {
			    rela.r_info = ELFNN_R_INFO (0,
							R_LARCH_TLS_DTPMODNN);
			    loongarch_elf_append_rela (output_bfd, srel, &rela);
			  }

			bfd_put_NN (output_bfd, tls_block_off,
				    got->contents + got_off + GOT_ENTRY_SIZE);
		      }
		    /* Dynamic resolved.  */
		    else
		      {
			/* Dynamic relocate module id.  */
			rela.r_info = ELFNN_R_INFO (h->dynindx,
						    R_LARCH_TLS_DTPMODNN);
			loongarch_elf_append_rela (output_bfd, srel, &rela);

			/* Dynamic relocate offset of block.  */
			rela.r_offset += GOT_ENTRY_SIZE;
			rela.r_info = ELFNN_R_INFO (h->dynindx,
						    R_LARCH_TLS_DTPRELNN);
			loongarch_elf_append_rela (output_bfd, srel, &rela);
		      }
		  }
		if (tls_type & GOT_TLS_IE)
		  {
		    rela.r_offset = sec_addr (got) + got_off + ie_off;
		    if (SYMBOL_REFERENCES_LOCAL (info, h))
		      {
			/* Local sym, used in exec, set module id 1.  */
			if (!bfd_link_executable (info))
			  {
			    rela.r_info = ELFNN_R_INFO (0, R_LARCH_TLS_TPRELNN);
			    rela.r_addend = tls_block_off;
			    loongarch_elf_append_rela (output_bfd, srel, &rela);
			  }

			bfd_put_NN (output_bfd, tls_block_off,
				    got->contents + got_off + ie_off);
		      }
		    /* Dynamic resolved.  */
		    else
		      {
			/* Dynamic relocate offset of block.  */
			rela.r_info = ELFNN_R_INFO (h->dynindx,
						    R_LARCH_TLS_TPRELNN);
			rela.r_addend = 0;
			loongarch_elf_append_rela (output_bfd, srel, &rela);
		      }
		  }
	      }

	    relocation = (got_off & (~(bfd_vma)1)) + (is_ie ? ie_off : 0);
	  }
	  break;

	/* New reloc types.  */
	case R_LARCH_B21:
	case R_LARCH_B26:
	case R_LARCH_B16:
	  unresolved_reloc = false;
	  if (is_undefweak)
	    {
	      relocation = 0;
	    }

	  if (resolved_local)
	    {
	      relocation -= pc;
	      relocation += rel->r_addend;
	    }
	  else if (resolved_dynly)
	    {
	      BFD_ASSERT (h
			  && (h->plt.offset != MINUS_ONE
			      || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
			  && rel->r_addend == 0);
	      if (h && h->plt.offset == MINUS_ONE
		  && ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
		{
		  relocation -= pc;
		  relocation += rel->r_addend;
		}
	      else
		relocation = sec_addr (plt) + h->plt.offset - pc;
	    }

	  break;

	case R_LARCH_ABS_HI20:
	case R_LARCH_ABS_LO12:
	case R_LARCH_ABS64_LO20:
	case R_LARCH_ABS64_HI12:
	  BFD_ASSERT (!is_pic);

	  if (is_undefweak)
	    {
	      BFD_ASSERT (resolved_dynly);
	      relocation = 0;
	      break;
	    }
	  else if (resolved_to_const || resolved_local)
	    {
	      relocation += rel->r_addend;
	    }
	  else if (resolved_dynly)
	    {
	      unresolved_reloc = false;
	      BFD_ASSERT ((plt && h && h->plt.offset != MINUS_ONE)
			  && rel->r_addend == 0);
	      relocation = sec_addr (plt) + h->plt.offset;
	    }

	  break;

	case R_LARCH_PCREL20_S2:
	  unresolved_reloc = false;
	  if (h && h->plt.offset != MINUS_ONE)
	    relocation = sec_addr (plt) + h->plt.offset;
	  else
	    relocation += rel->r_addend;
	  relocation -= pc;
	  break;

	case R_LARCH_PCALA_HI20:
	  unresolved_reloc = false;
	  if (h && h->plt.offset != MINUS_ONE)
	    relocation = sec_addr (plt) + h->plt.offset;
	  else
	    relocation += rel->r_addend;

	  RELOCATE_CALC_PC32_HI20 (relocation, pc);

	  break;

	case R_LARCH_PCALA_LO12:
	  /* Not support if sym_addr in 2k page edge.
	     pcalau12i pc_hi20 (sym_addr)
	     ld.w/d pc_lo12 (sym_addr)
	     ld.w/d pc_lo12 (sym_addr + x)
	     ...
	     can not calc correct address
	     if sym_addr < 0x800 && sym_addr + x >= 0x800.  */

	  if (h && h->plt.offset != MINUS_ONE)
	    relocation = sec_addr (plt) + h->plt.offset;
	  else
	    relocation += rel->r_addend;

	  /* For 2G jump, generate pcalau12i, jirl.  */
	  /* If use jirl, turns to R_LARCH_B16.  */
	  uint32_t insn = bfd_get (32, input_bfd, contents + rel->r_offset);
	  if ((insn & 0x4c000000) == 0x4c000000)
	    {
	      relocation &= 0xfff;
	      /* Signed extend.  */
	      relocation = (relocation ^ 0x800) - 0x800;

	      rel->r_info = ELFNN_R_INFO (r_symndx, R_LARCH_B16);
	      howto = loongarch_elf_rtype_to_howto (input_bfd, R_LARCH_B16);
	    }
	  break;

	case R_LARCH_PCALA64_LO20:
	case R_LARCH_PCALA64_HI12:
	  if (h && h->plt.offset != MINUS_ONE)
	    relocation = sec_addr (plt) + h->plt.offset;
	  else
	    relocation += rel->r_addend;

	  RELOCATE_CALC_PC64_HI32 (relocation, pc);

	  break;

	case R_LARCH_GOT_PC_HI20:
	case R_LARCH_GOT_HI20:
	  /* Calc got offset.  */
	    {
	      unresolved_reloc = false;
	      BFD_ASSERT (rel->r_addend == 0);

	      bfd_vma got_off = 0;
	      if (h != NULL)
		{
		  /* GOT ref or ifunc.  */
		  BFD_ASSERT (h->got.offset != MINUS_ONE
			      || h->type == STT_GNU_IFUNC);

		  got_off = h->got.offset  & (~(bfd_vma)1);
		  /* Hidden symbol not has got entry,
		   * only got.plt entry so it is (plt - got).  */
		  if (h->got.offset == MINUS_ONE && h->type == STT_GNU_IFUNC)
		    {
		      bfd_vma idx;
		      if (htab->elf.splt != NULL)
			{
			  idx = (h->plt.offset - PLT_HEADER_SIZE)
			    / PLT_ENTRY_SIZE;
			  got_off = sec_addr (htab->elf.sgotplt)
			    + GOTPLT_HEADER_SIZE
			    + (idx * GOT_ENTRY_SIZE)
			    - sec_addr (htab->elf.sgot);
			}
		      else
			{
			  idx = h->plt.offset / PLT_ENTRY_SIZE;
			  got_off = sec_addr (htab->elf.sgotplt)
			    + (idx * GOT_ENTRY_SIZE)
			    - sec_addr (htab->elf.sgot);
			}
		    }

		  if ((h->got.offset & 1) == 0)
		    {
		      /* We need to generate a R_LARCH_RELATIVE reloc once
		       * in loongarch_elf_finish_dynamic_symbol or now,
		       * call finish_dyn && nopic
		       * or !call finish_dyn && pic.  */
		      if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (is_dyn,
							    bfd_link_pic (info),
							    h)
			  && bfd_link_pic (info)
			  && SYMBOL_REFERENCES_LOCAL (info, h))
			{
			  Elf_Internal_Rela rela;
			  rela.r_offset = sec_addr (got) + got_off;
			  rela.r_info = ELFNN_R_INFO (0, R_LARCH_RELATIVE);
			  rela.r_addend = relocation;
			  loongarch_elf_append_rela (output_bfd,
						     htab->elf.srelgot, &rela);
			}
		      h->got.offset |= 1;
		      bfd_put_NN (output_bfd, relocation,
				  got->contents + got_off);
		    }
		}
	      else
		{
		  BFD_ASSERT (local_got_offsets
			      && local_got_offsets[r_symndx] != MINUS_ONE);

		  got_off = local_got_offsets[r_symndx] & (~(bfd_vma)1);
		  if ((local_got_offsets[r_symndx] & 1) == 0)
		    {
		      if (bfd_link_pic (info))
			{
			  Elf_Internal_Rela rela;
			  rela.r_offset = sec_addr (got) + got_off;
			  rela.r_info = ELFNN_R_INFO (0, R_LARCH_RELATIVE);
			  rela.r_addend = relocation;
			  loongarch_elf_append_rela (output_bfd,
						     htab->elf.srelgot, &rela);
			}
		      local_got_offsets[r_symndx] |= 1;
		    }
		  bfd_put_NN (output_bfd, relocation, got->contents + got_off);
		}

	      relocation = got_off + sec_addr (got);
	    }

	  if (r_type == R_LARCH_GOT_PC_HI20)
	    RELOCATE_CALC_PC32_HI20 (relocation, pc);

	  break;

	case R_LARCH_GOT_PC_LO12:
	case R_LARCH_GOT64_PC_LO20:
	case R_LARCH_GOT64_PC_HI12:
	case R_LARCH_GOT_LO12:
	case R_LARCH_GOT64_LO20:
	case R_LARCH_GOT64_HI12:
	    {
	      unresolved_reloc = false;
	      bfd_vma got_off;
	      if (h)
		got_off = h->got.offset & (~(bfd_vma)1);
	      else
		got_off = local_got_offsets[r_symndx] & (~(bfd_vma)1);

	      if (h && h->got.offset == MINUS_ONE && h->type == STT_GNU_IFUNC)
		{
		  bfd_vma idx;
		  if (htab->elf.splt != NULL)
		    idx = (h->plt.offset - PLT_HEADER_SIZE) / PLT_ENTRY_SIZE;
		  else
		    idx = h->plt.offset / PLT_ENTRY_SIZE;

		  got_off = sec_addr (htab->elf.sgotplt)
		    + GOTPLT_HEADER_SIZE
		    + (idx * GOT_ENTRY_SIZE)
		    - sec_addr (htab->elf.sgot);
		}

	      relocation = got_off + sec_addr (got);
	    }

	  if (r_type == R_LARCH_GOT64_PC_HI12
	      || r_type == R_LARCH_GOT64_PC_LO20)
	    RELOCATE_CALC_PC64_HI32 (relocation, pc);

	  break;

	case R_LARCH_TLS_LE_HI20:
	case R_LARCH_TLS_LE_LO12:
	case R_LARCH_TLS_LE64_LO20:
	case R_LARCH_TLS_LE64_HI12:
	  BFD_ASSERT (resolved_local && elf_hash_table (info)->tls_sec);

	  relocation -= elf_hash_table (info)->tls_sec->vma;
	  break;

	/* TLS IE LD/GD process separately is troublesome.
	   When a symbol is both ie and LD/GD, h->got.off |= 1
	   make only one type be relocated.  We must use
	   h->got.offset |= 1 and h->got.offset |= 2
	   diff IE and LD/GD.  And all (got_off & (~(bfd_vma)1))
	   (IE LD/GD and reusable GOT reloc) must change to
	   (got_off & (~(bfd_vma)3)), beause we use lowest 2 bits
	   as a tag.
	   Now, LD and GD is both GOT_TLS_GD type, LD seems to
	   can be omitted.  */
	case R_LARCH_TLS_IE_PC_HI20:
	case R_LARCH_TLS_IE_HI20:
	case R_LARCH_TLS_LD_PC_HI20:
	case R_LARCH_TLS_LD_HI20:
	case R_LARCH_TLS_GD_PC_HI20:
	case R_LARCH_TLS_GD_HI20:
	  BFD_ASSERT (rel->r_addend == 0);
	  unresolved_reloc = false;

	  if (r_type == R_LARCH_TLS_IE_PC_HI20
	      || r_type == R_LARCH_TLS_IE_HI20)
	    is_ie = true;

	  bfd_vma got_off = 0;
	  if (h != NULL)
	    {
	      got_off = h->got.offset;
	      h->got.offset |= 1;
	    }
	  else
	    {
	      got_off = local_got_offsets[r_symndx];
	      local_got_offsets[r_symndx] |= 1;
	    }

	  BFD_ASSERT (got_off != MINUS_ONE);

	  ie_off = 0;
	  tls_type = _bfd_loongarch_elf_tls_type (input_bfd, h, r_symndx);
	  if ((tls_type & GOT_TLS_GD) && (tls_type & GOT_TLS_IE))
	    ie_off = 2 * GOT_ENTRY_SIZE;

	  if ((got_off & 1) == 0)
	    {
	      Elf_Internal_Rela rela;
	      asection *relgot = htab->elf.srelgot;
	      bfd_vma tls_block_off = 0;

	      if (SYMBOL_REFERENCES_LOCAL (info, h))
		{
		  BFD_ASSERT (elf_hash_table (info)->tls_sec);
		  tls_block_off = relocation
		      - elf_hash_table (info)->tls_sec->vma;
		}

	      if (tls_type & GOT_TLS_GD)
		{
		  rela.r_offset = sec_addr (got) + got_off;
		  rela.r_addend = 0;
		  if (SYMBOL_REFERENCES_LOCAL (info, h))
		    {
		      /* Local sym, used in exec, set module id 1.  */
		      if (bfd_link_executable (info))
			bfd_put_NN (output_bfd, 1, got->contents + got_off);
		      else
			{
			  rela.r_info = ELFNN_R_INFO (0, R_LARCH_TLS_DTPMODNN);
			  loongarch_elf_append_rela (output_bfd, relgot, &rela);
			}

		      bfd_put_NN (output_bfd, tls_block_off,
				  got->contents + got_off + GOT_ENTRY_SIZE);
		    }
		  /* Dynamic resolved.  */
		  else
		    {
		      /* Dynamic relocate module id.  */
		      rela.r_info = ELFNN_R_INFO (h->dynindx,
						  R_LARCH_TLS_DTPMODNN);
		      loongarch_elf_append_rela (output_bfd, relgot, &rela);

		      /* Dynamic relocate offset of block.  */
		      rela.r_offset += GOT_ENTRY_SIZE;
		      rela.r_info = ELFNN_R_INFO (h->dynindx,
						  R_LARCH_TLS_DTPRELNN);
		      loongarch_elf_append_rela (output_bfd, relgot, &rela);
		    }
		}
	      if (tls_type & GOT_TLS_IE)
		{
		  rela.r_offset = sec_addr (got) + got_off + ie_off;
		  if (SYMBOL_REFERENCES_LOCAL (info, h))
		    {
		      /* Local sym, used in exec, set module id 1.  */
		      if (!bfd_link_executable (info))
			{
			  rela.r_info = ELFNN_R_INFO (0, R_LARCH_TLS_TPRELNN);
			  rela.r_addend = tls_block_off;
			  loongarch_elf_append_rela (output_bfd, relgot, &rela);
			}

		      bfd_put_NN (output_bfd, tls_block_off,
				  got->contents + got_off + ie_off);
		    }
		  /* Dynamic resolved.  */
		  else
		    {
		      /* Dynamic relocate offset of block.  */
		      rela.r_info = ELFNN_R_INFO (h->dynindx,
						  R_LARCH_TLS_TPRELNN);
		      rela.r_addend = 0;
		      loongarch_elf_append_rela (output_bfd, relgot, &rela);
		    }
		}
	    }
	  relocation = (got_off & (~(bfd_vma)1)) + sec_addr (got)
			+ (is_ie ? ie_off : 0);

	  if (r_type == R_LARCH_TLS_LD_PC_HI20
	      || r_type == R_LARCH_TLS_GD_PC_HI20
	      || r_type == R_LARCH_TLS_IE_PC_HI20)
	    RELOCATE_CALC_PC32_HI20 (relocation, pc);

	  break;

	case R_LARCH_TLS_IE_PC_LO12:
	case R_LARCH_TLS_IE64_PC_LO20:
	case R_LARCH_TLS_IE64_PC_HI12:
	case R_LARCH_TLS_IE_LO12:
	case R_LARCH_TLS_IE64_LO20:
	case R_LARCH_TLS_IE64_HI12:
	  unresolved_reloc = false;

	  if (h)
	    relocation = sec_addr (got) + (h->got.offset & (~(bfd_vma)3));
	  else
	    relocation = sec_addr (got)
	      + (local_got_offsets[r_symndx] & (~(bfd_vma)3));

	  tls_type = _bfd_loongarch_elf_tls_type (input_bfd, h, r_symndx);
	  /* Use both TLS_GD and TLS_IE.  */
	  if ((tls_type & GOT_TLS_GD) && (tls_type & GOT_TLS_IE))
	    relocation += 2 * GOT_ENTRY_SIZE;

	  if (r_type == R_LARCH_TLS_IE64_PC_LO20
	      || r_type == R_LARCH_TLS_IE64_PC_HI12)
	    RELOCATE_CALC_PC64_HI32 (relocation, pc);

	  break;

	case R_LARCH_RELAX:
	case R_LARCH_ALIGN:
	  r = bfd_reloc_continue;
	  unresolved_reloc = false;
	  break;

	default:
	  break;
	}

      if (fatal)
	break;

      do
	{
	  /* 'unresolved_reloc' means we haven't done it yet.
	     We need help of dynamic linker to fix this memory location up.  */
	  if (!unresolved_reloc)
	    break;

	  if (_bfd_elf_section_offset (output_bfd, info, input_section,
				       rel->r_offset) == MINUS_ONE)
	    /* WHY? May because it's invalid so skip checking.
	       But why dynamic reloc a invalid section?  */
	    break;

	  if (input_section->output_section->flags & SEC_DEBUGGING)
	    {
	      fatal = (loongarch_reloc_is_fatal
		       (info, input_bfd, input_section, rel, howto,
			bfd_reloc_dangerous, is_undefweak, name,
			"Seems dynamic linker not process "
			"sections 'SEC_DEBUGGING'."));
	    }
	  if (!is_dyn)
	    break;

	  if ((info->flags & DF_TEXTREL) == 0)
	    if (input_section->output_section->flags & SEC_READONLY)
	      info->flags |= DF_TEXTREL;
	}
      while (0);

      if (fatal)
	break;

      loongarch_record_one_reloc (input_bfd, input_section, r_type,
				  rel->r_offset, sym, h, rel->r_addend);

      if (r != bfd_reloc_continue)
	r = perform_relocation (rel, input_section, howto, relocation,
				input_bfd, contents);

      switch (r)
	{
	case bfd_reloc_dangerous:
	case bfd_reloc_continue:
	case bfd_reloc_ok:
	  continue;

	case bfd_reloc_overflow:
	  /* Overflow value can't be filled in.  */
	  loongarch_dump_reloc_record (info->callbacks->info);
	  info->callbacks->reloc_overflow
	    (info, h ? &h->root : NULL, name, howto->name, rel->r_addend,
	     input_bfd, input_section, rel->r_offset);
	  break;

	case bfd_reloc_outofrange:
	  /* Stack state incorrect.  */
	  loongarch_dump_reloc_record (info->callbacks->info);
	  info->callbacks->info
	    ("%X%H: Internal stack state is incorrect.\n"
	     "Want to push to full stack or pop from empty stack?\n",
	     input_bfd, input_section, rel->r_offset);
	  break;

	case bfd_reloc_notsupported:
	  info->callbacks->info ("%X%H: Unknown relocation type.\n", input_bfd,
				 input_section, rel->r_offset);
	  break;

	default:
	  info->callbacks->info ("%X%H: Internal: unknown error.\n", input_bfd,
				 input_section, rel->r_offset);
	  break;
	}

      fatal = true;
    }

  return !fatal;
}

static bool
loongarch_relax_delete_bytes (bfd *abfd,
			  asection *sec,
			  bfd_vma addr,
			  size_t count,
			  struct bfd_link_info *link_info)
{
  unsigned int i, symcount;
  bfd_vma toaddr = sec->size;
  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (abfd);
  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  unsigned int sec_shndx = _bfd_elf_section_from_bfd_section (abfd, sec);
  struct bfd_elf_section_data *data = elf_section_data (sec);
  bfd_byte *contents = data->this_hdr.contents;

  /* Actually delete the bytes.  */
  sec->size -= count;
  memmove (contents + addr, contents + addr + count, toaddr - addr - count);

  /* Adjust the location of all of the relocs.  Note that we need not
     adjust the addends, since all PC-relative references must be against
     symbols, which we will adjust below.  */
  for (i = 0; i < sec->reloc_count; i++)
    if (data->relocs[i].r_offset > addr && data->relocs[i].r_offset < toaddr)
      data->relocs[i].r_offset -= count;

  /* Adjust the local symbols defined in this section.  */
  for (i = 0; i < symtab_hdr->sh_info; i++)
    {
      Elf_Internal_Sym *sym = (Elf_Internal_Sym *) symtab_hdr->contents + i;
      if (sym->st_shndx == sec_shndx)
	{
	  /* If the symbol is in the range of memory we just moved, we
	     have to adjust its value.  */
	  if (sym->st_value > addr && sym->st_value <= toaddr)
	    sym->st_value -= count;

	  /* If the symbol *spans* the bytes we just deleted (i.e. its
	     *end* is in the moved bytes but its *start* isn't), then we
	     must adjust its size.

	     This test needs to use the original value of st_value, otherwise
	     we might accidentally decrease size when deleting bytes right
	     before the symbol.  But since deleted relocs can't span across
	     symbols, we can't have both a st_value and a st_size decrease,
	     so it is simpler to just use an else.  */
	  else if (sym->st_value <= addr
		   && sym->st_value + sym->st_size > addr
		   && sym->st_value + sym->st_size <= toaddr)
	    sym->st_size -= count;
	}
    }

  /* Now adjust the global symbols defined in this section.  */
  symcount = ((symtab_hdr->sh_size / sizeof (ElfNN_External_Sym))
	      - symtab_hdr->sh_info);

  for (i = 0; i < symcount; i++)
    {
      struct elf_link_hash_entry *sym_hash = sym_hashes[i];

      /* The '--wrap SYMBOL' option is causing a pain when the object file,
	 containing the definition of __wrap_SYMBOL, includes a direct
	 call to SYMBOL as well. Since both __wrap_SYMBOL and SYMBOL reference
	 the same symbol (which is __wrap_SYMBOL), but still exist as two
	 different symbols in 'sym_hashes', we don't want to adjust
	 the global symbol __wrap_SYMBOL twice.

	 The same problem occurs with symbols that are versioned_hidden, as
	 foo becomes an alias for foo@BAR, and hence they need the same
	 treatment.  */
      if (link_info->wrap_hash != NULL
	  || sym_hash->versioned != unversioned)
	{
	  struct elf_link_hash_entry **cur_sym_hashes;

	  /* Loop only over the symbols which have already been checked.  */
	  for (cur_sym_hashes = sym_hashes; cur_sym_hashes < &sym_hashes[i];
	       cur_sym_hashes++)
	    {
	      /* If the current symbol is identical to 'sym_hash', that means
		 the symbol was already adjusted (or at least checked).  */
	      if (*cur_sym_hashes == sym_hash)
		break;
	    }
	  /* Don't adjust the symbol again.  */
	  if (cur_sym_hashes < &sym_hashes[i])
	    continue;
	}

      if ((sym_hash->root.type == bfd_link_hash_defined
	   || sym_hash->root.type == bfd_link_hash_defweak)
	  && sym_hash->root.u.def.section == sec)
	{
	  /* As above, adjust the value if needed.  */
	  if (sym_hash->root.u.def.value > addr
	      && sym_hash->root.u.def.value <= toaddr)
	    sym_hash->root.u.def.value -= count;

	  /* As above, adjust the size if needed.  */
	  else if (sym_hash->root.u.def.value <= addr
		   && sym_hash->root.u.def.value + sym_hash->size > addr
		   && sym_hash->root.u.def.value + sym_hash->size <= toaddr)
	    sym_hash->size -= count;
	}
    }

  return true;
}

/* Relax pcalau12i,addi.d => pcaddi.  */
static bool
loongarch_relax_pcala_addi (bfd *abfd, asection *sec,
		       Elf_Internal_Rela *rel_hi, bfd_vma symval)
{
  bfd_byte *contents = elf_section_data (sec)->this_hdr.contents;
  Elf_Internal_Rela *rel_lo = rel_hi + 2;
  uint32_t pca = bfd_get (32, abfd, contents + rel_hi->r_offset);
  uint32_t add = bfd_get (32, abfd, contents + rel_lo->r_offset);
  uint32_t rd = pca & 0x1f;
  bfd_vma pc = sec_addr (sec) + rel_hi->r_offset;
  const uint32_t addi_d = 0x02c00000;
  const uint32_t pcaddi = 0x18000000;

  /* Is pcalau12i + addi.d insns?  */
  if ((ELFNN_R_TYPE (rel_lo->r_info) != R_LARCH_PCALA_LO12)
      || (ELFNN_R_TYPE ((rel_lo + 1)->r_info) != R_LARCH_RELAX)
      || (ELFNN_R_TYPE ((rel_hi + 1)->r_info) != R_LARCH_RELAX)
      || (rel_hi->r_offset + 4 != rel_lo->r_offset)
      || ((add & addi_d) != addi_d)
      /* Is pcalau12i $rd + addi.d $rd,$rd?  */
      || ((add & 0x1f) != rd)
      || (((add >> 5) & 0x1f) != rd)
      /* Can be relaxed to pcaddi?  */
      || (symval & 0x3) /* 4 bytes align.  */
      || ((bfd_signed_vma)(symval - pc) < (bfd_signed_vma)(int32_t)0xffe00000)
      || ((bfd_signed_vma)(symval - pc) > (bfd_signed_vma)(int32_t)0x1ffffc))
    return false;

  pca = pcaddi | rd;
  bfd_put (32, abfd, pca, contents + rel_hi->r_offset);

  /* Adjust relocations.  */
  rel_hi->r_info = ELFNN_R_INFO (ELFNN_R_SYM (rel_hi->r_info),
				 R_LARCH_PCREL20_S2);
  rel_lo->r_info = ELFNN_R_INFO (ELFNN_R_SYM (rel_hi->r_info),
				 R_LARCH_DELETE);

  return true;
}

/* Relax pcalau12i,ld.d => pcalau12i,addi.d.  */
static bool
loongarch_relax_pcala_ld (bfd *abfd, asection *sec,
		Elf_Internal_Rela *rel_hi)
{
  bfd_byte *contents = elf_section_data (sec)->this_hdr.contents;
  Elf_Internal_Rela *rel_lo = rel_hi + 2;
  uint32_t pca = bfd_get (32, abfd, contents + rel_hi->r_offset);
  uint32_t ld = bfd_get (32, abfd, contents + rel_lo->r_offset);
  uint32_t rd = pca & 0x1f;
  const uint32_t ld_d = 0x28c00000;
  uint32_t addi_d = 0x02c00000;

  if ((ELFNN_R_TYPE (rel_lo->r_info) != R_LARCH_GOT_PC_LO12)
      || (ELFNN_R_TYPE ((rel_lo + 1)->r_info) != R_LARCH_RELAX)
      || (ELFNN_R_TYPE ((rel_hi + 1)->r_info) != R_LARCH_RELAX)
      || (rel_hi->r_offset + 4 != rel_lo->r_offset)
      || ((ld & 0x1f) != rd)
      || (((ld >> 5) & 0x1f) != rd)
      || ((ld & ld_d) != ld_d))
    return false;

  addi_d = addi_d | (rd << 5) | rd;
  bfd_put (32, abfd, addi_d, contents + rel_lo->r_offset);

  rel_hi->r_info = ELFNN_R_INFO (ELFNN_R_SYM (rel_hi->r_info),
				 R_LARCH_PCALA_HI20);
  rel_lo->r_info = ELFNN_R_INFO (ELFNN_R_SYM (rel_lo->r_info),
				 R_LARCH_PCALA_LO12);
  return true;
}

/* Called by after_allocation to set the information of data segment
   before relaxing.  */

void
bfd_elfNN_loongarch_set_data_segment_info (struct bfd_link_info *info,
				     int *data_segment_phase)
{
  struct loongarch_elf_link_hash_table *htab = loongarch_elf_hash_table (info);
  htab->data_segment_phase = data_segment_phase;
}

/* Implement R_LARCH_ALIGN by deleting excess alignment NOPs.
   Once we've handled an R_LARCH_ALIGN, we can't relax anything else.  */
static bool
loongarch_relax_align (bfd *abfd, asection *sec,
			asection *sym_sec,
			struct bfd_link_info *link_info,
			Elf_Internal_Rela *rel,
			bfd_vma symval)
{
  bfd_byte *contents = elf_section_data (sec)->this_hdr.contents;
  bfd_vma alignment = 1, pos;
  while (alignment <= rel->r_addend)
    alignment *= 2;

  symval -= rel->r_addend;
  bfd_vma aligned_addr = ((symval - 1) & ~(alignment - 1)) + alignment;
  bfd_vma nop_bytes = aligned_addr - symval;

  /* Once we've handled an R_LARCH_ALIGN, we can't relax anything else.  */
  sec->sec_flg0 = true;

  /* Make sure there are enough NOPs to actually achieve the alignment.  */
  if (rel->r_addend < nop_bytes)
    {
      _bfd_error_handler
	(_("%pB(%pA+%#" PRIx64 "): %" PRId64 " bytes required for alignment "
	   "to %" PRId64 "-byte boundary, but only %" PRId64 " present"),
	 abfd, sym_sec, (uint64_t) rel->r_offset,
	 (int64_t) nop_bytes, (int64_t) alignment, (int64_t) rel->r_addend);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* Delete the reloc.  */
  rel->r_info = ELFNN_R_INFO (0, R_LARCH_NONE);

  /* If the number of NOPs is already correct, there's nothing to do.  */
  if (nop_bytes == rel->r_addend)
    return true;

  /* Write as many LOONGARCH NOPs as we need.  */
  for (pos = 0; pos < (nop_bytes & -4); pos += 4)
    bfd_putl32 (LARCH_NOP, contents + rel->r_offset + pos);

  /* Delete the excess NOPs.  */
  return loongarch_relax_delete_bytes (abfd, sec, rel->r_offset + nop_bytes,
				   rel->r_addend - nop_bytes, link_info);
}

static bool
loongarch_elf_relax_section (bfd *abfd, asection *sec,
			       struct bfd_link_info *info,
			       bool *again)
{
  struct loongarch_elf_link_hash_table *htab = loongarch_elf_hash_table (info);
  Elf_Internal_Shdr *symtab_hdr = &elf_symtab_hdr (abfd);
  struct bfd_elf_section_data *data = elf_section_data (sec);
  Elf_Internal_Rela *relocs;
  *again = false;

  if (bfd_link_relocatable (info)
      || sec->sec_flg0
      || (sec->flags & SEC_RELOC) == 0
      || sec->reloc_count == 0
      || elf_seg_map (info->output_bfd) == NULL
      || (info->disable_target_specific_optimizations
	  && info->relax_pass == 0)
      /* The exp_seg_relro_adjust is enum phase_enum (0x4),
	 and defined in ld/ldexp.h.  */
      || *(htab->data_segment_phase) == 4)
    return true;

  if (data->relocs)
    relocs = data->relocs;
  else if (!(relocs = _bfd_elf_link_read_relocs (abfd, sec, NULL, NULL,
						 info->keep_memory)))
    return true;

  if (!data->this_hdr.contents
      && !bfd_malloc_and_get_section (abfd, sec, &data->this_hdr.contents))
    return true;

  if (symtab_hdr->sh_info != 0
      && !symtab_hdr->contents
      && !(symtab_hdr->contents =
	   (unsigned char *) bfd_elf_get_elf_syms (abfd, symtab_hdr,
						   symtab_hdr->sh_info,
						   0, NULL, NULL, NULL)))
    return true;

      data->relocs = relocs;

  for (unsigned int i = 0; i < sec->reloc_count; i++)
    {
      Elf_Internal_Rela *rel = relocs + i;
      asection *sym_sec;
      bfd_vma symval;
      unsigned long r_symndx = ELFNN_R_SYM (rel->r_info);
      bool local_got = false;
      char symtype;
      struct elf_link_hash_entry *h = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  Elf_Internal_Sym *sym = (Elf_Internal_Sym *)symtab_hdr->contents
				    + r_symndx;
	  if (ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
	    continue;

	  if (sym->st_shndx == SHN_UNDEF)
	    {
	      sym_sec = sec;
	      symval = rel->r_offset;
	    }
	  else
	    {
	      sym_sec = elf_elfsections (abfd)[sym->st_shndx]->bfd_section;
	      symval = sym->st_value;
	    }
	  symtype = ELF_ST_TYPE (sym->st_info);
	}
      else
	{
	  r_symndx = ELFNN_R_SYM (rel->r_info) - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[r_symndx];

	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  /* Disable the relaxation for ifunc.  */
	  if (h != NULL && h->type == STT_GNU_IFUNC)
	    continue;

	  if ((h->root.type == bfd_link_hash_defined
		    || h->root.type == bfd_link_hash_defweak)
		   && h->root.u.def.section != NULL
		   && h->root.u.def.section->output_section != NULL)
	    {
	      symval = h->root.u.def.value;
	      sym_sec = h->root.u.def.section;
	    }
	  else
	    continue;

	  if (h && bfd_link_executable (info)
	      && SYMBOL_REFERENCES_LOCAL (info, h))
	    local_got = true;
	  symtype = h->type;
	}

      if (sym_sec->sec_info_type == SEC_INFO_TYPE_MERGE
	   && (sym_sec->flags & SEC_MERGE))
	{
	   if (symtype == STT_SECTION)
	     symval += rel->r_addend;

	   symval = _bfd_merged_section_offset (abfd, &sym_sec,
				elf_section_data (sym_sec)->sec_info,
				symval);

	   if (symtype != STT_SECTION)
	     symval += rel->r_addend;
	}
      else
	symval += rel->r_addend;

      symval += sec_addr (sym_sec);

      switch (ELFNN_R_TYPE (rel->r_info))
	{
	case R_LARCH_ALIGN:
	  if (2 == info->relax_pass)
	    loongarch_relax_align (abfd, sec, sym_sec, info, rel, symval);
	  break;
	case R_LARCH_DELETE:
	  if (info->relax_pass == 1)
	    {
	      loongarch_relax_delete_bytes (abfd, sec, rel->r_offset, 4, info);
	      rel->r_info = ELFNN_R_INFO (0, R_LARCH_NONE);
	    }
	  break;
	case R_LARCH_PCALA_HI20:
	  if (info->relax_pass == 0)
	    {
	      if (i + 4 > sec->reloc_count)
		break;
	      loongarch_relax_pcala_addi (abfd, sec, rel, symval);
	    }
	  break;
	case R_LARCH_GOT_PC_HI20:
	  if (local_got)
	    {
	      if (i + 4 > sec->reloc_count)
		break;
	      if (loongarch_relax_pcala_ld (abfd, sec, rel))
		{
		  loongarch_relax_pcala_addi (abfd, sec, rel, symval);
		}
	    }
	  break;
	default:
	  break;
	}
    }

  return true;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
loongarch_elf_finish_dynamic_symbol (bfd *output_bfd,
				     struct bfd_link_info *info,
				     struct elf_link_hash_entry *h,
				     Elf_Internal_Sym *sym)
{
  struct loongarch_elf_link_hash_table *htab = loongarch_elf_hash_table (info);
  const struct elf_backend_data *bed = get_elf_backend_data (output_bfd);
  asection *rela_dyn = bfd_get_section_by_name (output_bfd, ".rela.dyn");
  struct bfd_link_order *lo = NULL;
  Elf_Internal_Rela *slot = NULL, *last_slot = NULL;

  if (rela_dyn)
    lo = rela_dyn->map_head.link_order;

  if (h->plt.offset != MINUS_ONE)
    {
      size_t i, plt_idx;
      asection *plt, *gotplt, *relplt;
      bfd_vma got_address;
      uint32_t plt_entry[PLT_ENTRY_INSNS];
      bfd_byte *loc;
      Elf_Internal_Rela rela;
      asection *rela_sec = NULL;

      if (htab->elf.splt)
	{
	  BFD_ASSERT ((h->type == STT_GNU_IFUNC
		       && SYMBOL_REFERENCES_LOCAL (info, h))
		      || h->dynindx != -1);

	  plt = htab->elf.splt;
	  gotplt = htab->elf.sgotplt;
	  if (h->type == STT_GNU_IFUNC && SYMBOL_REFERENCES_LOCAL (info, h))
	    relplt = htab->elf.srelgot;
	  else
	    relplt = htab->elf.srelplt;
	  plt_idx = (h->plt.offset - PLT_HEADER_SIZE) / PLT_ENTRY_SIZE;
	  got_address =
	    sec_addr (gotplt) + GOTPLT_HEADER_SIZE + plt_idx * GOT_ENTRY_SIZE;
	}
      else /* if (htab->elf.iplt) */
	{
	  BFD_ASSERT (h->type == STT_GNU_IFUNC
		      && SYMBOL_REFERENCES_LOCAL (info, h));

	  plt = htab->elf.iplt;
	  gotplt = htab->elf.igotplt;
	  relplt = htab->elf.irelplt;
	  plt_idx = h->plt.offset / PLT_ENTRY_SIZE;
	  got_address = sec_addr (gotplt) + plt_idx * GOT_ENTRY_SIZE;
	}

      /* Find out where the .plt entry should go.  */
      loc = plt->contents + h->plt.offset;

      /* Fill in the PLT entry itself.  */
      if (!loongarch_make_plt_entry (got_address,
				     sec_addr (plt) + h->plt.offset,
				     plt_entry))
	return false;

      for (i = 0; i < PLT_ENTRY_INSNS; i++)
	bfd_put_32 (output_bfd, plt_entry[i], loc + 4 * i);

      /* Fill in the initial value of the got.plt entry.  */
      loc = gotplt->contents + (got_address - sec_addr (gotplt));
      bfd_put_NN (output_bfd, sec_addr (plt), loc);

      rela.r_offset = got_address;

      /* TRUE if this is a PLT reference to a local IFUNC.  */
      if (PLT_LOCAL_IFUNC_P (info, h)
	  && (relplt == htab->elf.srelgot
	      || relplt == htab->elf.irelplt))
	{
	  rela.r_info = ELFNN_R_INFO (0, R_LARCH_IRELATIVE);
	  rela.r_addend = (h->root.u.def.value
			       + h->root.u.def.section->output_section->vma
			       + h->root.u.def.section->output_offset);

	  /* Find the space after dyn sort.  */
	  while (slot == last_slot || slot->r_offset != 0)
	    {
	      if (slot != last_slot)
		{
		  slot++;
		  continue;
		}

	      BFD_ASSERT (lo != NULL);
	      rela_sec = lo->u.indirect.section;
	      lo = lo->next;

	      slot = (Elf_Internal_Rela *)rela_sec->contents;
	      last_slot = (Elf_Internal_Rela *)(rela_sec->contents +
						rela_sec->size);
	    }

	  bed->s->swap_reloca_out (output_bfd, &rela, (bfd_byte *)slot);
	  rela_sec->reloc_count++;
	}
      else
	{
	  /* Fill in the entry in the rela.plt section.  */
	  rela.r_info = ELFNN_R_INFO (h->dynindx, R_LARCH_JUMP_SLOT);
	  rela.r_addend = 0;
	  loc = relplt->contents + plt_idx * sizeof (ElfNN_External_Rela);
	  bed->s->swap_reloca_out (output_bfd, &rela, loc);
	}

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	  /* If the symbol is weak, we do need to clear the value.
	     Otherwise, the PLT entry would provide a definition for
	     the symbol even if the symbol wasn't defined anywhere,
	     and so the symbol would never be NULL.  */
	  if (!h->ref_regular_nonweak)
	    sym->st_value = 0;
	}
    }

  if (h->got.offset != MINUS_ONE
      /* TLS got entry have been handled in elf_relocate_section.  */
      && !(loongarch_elf_hash_entry (h)->tls_type & (GOT_TLS_GD | GOT_TLS_IE))
      /* Have allocated got entry but not allocated rela before.  */
      && !UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
    {
      asection *sgot, *srela;
      Elf_Internal_Rela rela;
      bfd_vma off = h->got.offset & ~(bfd_vma)1;

      /* This symbol has an entry in the GOT.  Set it up.  */
      sgot = htab->elf.sgot;
      srela = htab->elf.srelgot;
      BFD_ASSERT (sgot && srela);

      rela.r_offset = sec_addr (sgot) + off;

      if (h->def_regular
	  && h->type == STT_GNU_IFUNC)
	{
	  if(h->plt.offset == MINUS_ONE)
	    {
	      if (htab->elf.splt == NULL)
		srela = htab->elf.irelplt;

	      if (SYMBOL_REFERENCES_LOCAL (info, h))
		{
		  asection *sec = h->root.u.def.section;
		  rela.r_info = ELFNN_R_INFO (0, R_LARCH_IRELATIVE);
		  rela.r_addend = h->root.u.def.value + sec->output_section->vma
		    + sec->output_offset;
		  bfd_put_NN (output_bfd, 0, sgot->contents + off);
		}
	      else
		{
		  BFD_ASSERT (h->dynindx != -1);
		  rela.r_info = ELFNN_R_INFO (h->dynindx, R_LARCH_NN);
		  rela.r_addend = 0;
		  bfd_put_NN (output_bfd, (bfd_vma) 0, sgot->contents + off);
		}
	    }
	  else if(bfd_link_pic (info))
	    {
	      rela.r_info = ELFNN_R_INFO (h->dynindx, R_LARCH_NN);
	      rela.r_addend = 0;
	      bfd_put_NN (output_bfd, rela.r_addend, sgot->contents + off);
	    }
	  else
	    {
	      asection *plt;
	      /* For non-shared object, we can't use .got.plt, which
		 contains the real function address if we need pointer
		 equality.  We load the GOT entry with the PLT entry.  */
	      plt = htab->elf.splt ? htab->elf.splt : htab->elf.iplt;
	      bfd_put_NN (output_bfd,
			  (plt->output_section->vma
			   + plt->output_offset
			   + h->plt.offset),
			  sgot->contents + off);
	      return true;
	    }
	}
      else if (bfd_link_pic (info) && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  asection *sec = h->root.u.def.section;
	  rela.r_info = ELFNN_R_INFO (0, R_LARCH_RELATIVE);
	  rela.r_addend = (h->root.u.def.value + sec->output_section->vma
			   + sec->output_offset);
	}
      else
	{
	  BFD_ASSERT (h->dynindx != -1);
	  rela.r_info = ELFNN_R_INFO (h->dynindx, R_LARCH_NN);
	  rela.r_addend = 0;
	}

      loongarch_elf_append_rela (output_bfd, srela, &rela);
    }

  /* Mark some specially defined symbols as absolute.  */
  if (h == htab->elf.hdynamic || h == htab->elf.hgot || h == htab->elf.hplt)
    sym->st_shndx = SHN_ABS;

  return true;
}

/* Finish up the dynamic sections.  */

static bool
loongarch_finish_dyn (bfd *output_bfd, struct bfd_link_info *info, bfd *dynobj,
		      asection *sdyn)
{
  struct loongarch_elf_link_hash_table *htab = loongarch_elf_hash_table (info);
  const struct elf_backend_data *bed = get_elf_backend_data (output_bfd);
  size_t dynsize = bed->s->sizeof_dyn, skipped_size = 0;
  bfd_byte *dyncon, *dynconend;

  dynconend = sdyn->contents + sdyn->size;
  for (dyncon = sdyn->contents; dyncon < dynconend; dyncon += dynsize)
    {
      Elf_Internal_Dyn dyn;
      asection *s;
      int skipped = 0;

      bed->s->swap_dyn_in (dynobj, dyncon, &dyn);

      switch (dyn.d_tag)
	{
	case DT_PLTGOT:
	  s = htab->elf.sgotplt;
	  dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	  break;
	case DT_JMPREL:
	  s = htab->elf.srelplt;
	  dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	  break;
	case DT_PLTRELSZ:
	  s = htab->elf.srelplt;
	  dyn.d_un.d_val = s->size;
	  break;
	case DT_TEXTREL:
	  if ((info->flags & DF_TEXTREL) == 0)
	    skipped = 1;
	  break;
	case DT_FLAGS:
	  if ((info->flags & DF_TEXTREL) == 0)
	    dyn.d_un.d_val &= ~DF_TEXTREL;
	  break;
	}
      if (skipped)
	skipped_size += dynsize;
      else
	bed->s->swap_dyn_out (output_bfd, &dyn, dyncon - skipped_size);
    }
  /* Wipe out any trailing entries if we shifted down a dynamic tag.  */
  memset (dyncon - skipped_size, 0, skipped_size);
  return true;
}

/* Finish up local dynamic symbol handling.  We set the contents of
   various dynamic sections here.  */

static bool
elfNN_loongarch_finish_local_dynamic_symbol (void **slot, void *inf)
{
  struct elf_link_hash_entry *h = (struct elf_link_hash_entry *) *slot;
  struct bfd_link_info *info = (struct bfd_link_info *) inf;

  return loongarch_elf_finish_dynamic_symbol (info->output_bfd, info, h, NULL);
}

static bool
loongarch_elf_finish_dynamic_sections (bfd *output_bfd,
				       struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn, *plt, *gotplt = NULL;
  struct loongarch_elf_link_hash_table *htab;

  htab = loongarch_elf_hash_table (info);
  BFD_ASSERT (htab);
  dynobj = htab->elf.dynobj;
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      BFD_ASSERT (htab->elf.splt && sdyn);

      if (!loongarch_finish_dyn (output_bfd, info, dynobj, sdyn))
	return false;
    }

  plt = htab->elf.splt;
  gotplt = htab->elf.sgotplt;

  if (plt && 0 < plt->size)
    {
      size_t i;
      uint32_t plt_header[PLT_HEADER_INSNS];
      if (!loongarch_make_plt_header (sec_addr (gotplt), sec_addr (plt),
				      plt_header))
	return false;

      for (i = 0; i < PLT_HEADER_INSNS; i++)
	bfd_put_32 (output_bfd, plt_header[i], plt->contents + 4 * i);

      elf_section_data (plt->output_section)->this_hdr.sh_entsize =
	PLT_ENTRY_SIZE;
    }

  if (htab->elf.sgotplt)
    {
      asection *output_section = htab->elf.sgotplt->output_section;

      if (bfd_is_abs_section (output_section))
	{
	  _bfd_error_handler (_("discarded output section: `%pA'"),
			      htab->elf.sgotplt);
	  return false;
	}

      if (0 < htab->elf.sgotplt->size)
	{
	  /* Write the first two entries in .got.plt, needed for the dynamic
	     linker.  */
	  bfd_put_NN (output_bfd, MINUS_ONE, htab->elf.sgotplt->contents);

	  bfd_put_NN (output_bfd, (bfd_vma) 0,
		      htab->elf.sgotplt->contents + GOT_ENTRY_SIZE);
	}

      elf_section_data (output_section)->this_hdr.sh_entsize = GOT_ENTRY_SIZE;
    }

  if (htab->elf.sgot)
    {
      asection *output_section = htab->elf.sgot->output_section;

      if (0 < htab->elf.sgot->size)
	{
	  /* Set the first entry in the global offset table to the address of
	     the dynamic section.  */
	  bfd_vma val = sdyn ? sec_addr (sdyn) : 0;
	  bfd_put_NN (output_bfd, val, htab->elf.sgot->contents);
	}

      elf_section_data (output_section)->this_hdr.sh_entsize = GOT_ENTRY_SIZE;
    }

  /* Fill PLT and GOT entries for local STT_GNU_IFUNC symbols.  */
  htab_traverse (htab->loc_hash_table,
		 (void *) elfNN_loongarch_finish_local_dynamic_symbol, info);

  return true;
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
loongarch_elf_plt_sym_val (bfd_vma i, const asection *plt,
			   const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + PLT_HEADER_SIZE + i * PLT_ENTRY_SIZE;
}

static enum elf_reloc_type_class
loongarch_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			    const asection *rel_sec ATTRIBUTE_UNUSED,
			    const Elf_Internal_Rela *rela)
{
  struct loongarch_elf_link_hash_table *htab;
  htab = loongarch_elf_hash_table (info);

  if (htab->elf.dynsym != NULL && htab->elf.dynsym->contents != NULL)
    {
      /* Check relocation against STT_GNU_IFUNC symbol if there are
	 dynamic symbols.  */
      bfd *abfd = info->output_bfd;
      const struct elf_backend_data *bed = get_elf_backend_data (abfd);
      unsigned long r_symndx = ELFNN_R_SYM (rela->r_info);
      if (r_symndx != STN_UNDEF)
	{
	  Elf_Internal_Sym sym;
	  if (!bed->s->swap_symbol_in (abfd,
				       htab->elf.dynsym->contents
				       + r_symndx * bed->s->sizeof_sym,
				       0, &sym))
	    {
	      /* xgettext:c-format  */
	      _bfd_error_handler (_("%pB symbol number %lu references"
				    " nonexistent SHT_SYMTAB_SHNDX section"),
				  abfd, r_symndx);
	      /* Ideally an error class should be returned here.  */
	    }
	  else if (ELF_ST_TYPE (sym.st_info) == STT_GNU_IFUNC)
	    return reloc_class_ifunc;
	}
    }

  switch (ELFNN_R_TYPE (rela->r_info))
    {
    case R_LARCH_IRELATIVE:
      return reloc_class_ifunc;
    case R_LARCH_RELATIVE:
      return reloc_class_relative;
    case R_LARCH_JUMP_SLOT:
      return reloc_class_plt;
    case R_LARCH_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
loongarch_elf_copy_indirect_symbol (struct bfd_link_info *info,
				    struct elf_link_hash_entry *dir,
				    struct elf_link_hash_entry *ind)
{
  struct elf_link_hash_entry *edir, *eind;

  edir = dir;
  eind = ind;

  if (eind->dyn_relocs != NULL)
    {
      if (edir->dyn_relocs != NULL)
	{
	  struct elf_dyn_relocs **pp;
	  struct elf_dyn_relocs *p;

	  /* Add reloc counts against the indirect sym to the direct sym
	     list.  Merge any entries against the same section.  */
	  for (pp = &eind->dyn_relocs; (p = *pp) != NULL;)
	    {
	      struct elf_dyn_relocs *q;

	      for (q = edir->dyn_relocs; q != NULL; q = q->next)
		if (q->sec == p->sec)
		  {
		    q->pc_count += p->pc_count;
		    q->count += p->count;
		    *pp = p->next;
		    break;
		  }
	      if (q == NULL)
		pp = &p->next;
	    }
	  *pp = edir->dyn_relocs;
	}

      edir->dyn_relocs = eind->dyn_relocs;
      eind->dyn_relocs = NULL;
    }

  if (ind->root.type == bfd_link_hash_indirect && dir->got.refcount < 0)
    {
      loongarch_elf_hash_entry(edir)->tls_type
	= loongarch_elf_hash_entry(eind)->tls_type;
      loongarch_elf_hash_entry(eind)->tls_type = GOT_UNKNOWN;
    }
  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

#define PRSTATUS_SIZE		    0x1d8
#define PRSTATUS_OFFSET_PR_CURSIG   0xc
#define PRSTATUS_OFFSET_PR_PID	    0x20
#define ELF_GREGSET_T_SIZE	    0x168
#define PRSTATUS_OFFSET_PR_REG	    0x70

/* Support for core dump NOTE sections.  */

static bool
loongarch_elf_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    default:
      return false;

    /* The sizeof (struct elf_prstatus) on Linux/LoongArch.  */
    case PRSTATUS_SIZE:
      /* pr_cursig  */
      elf_tdata (abfd)->core->signal =
	bfd_get_16 (abfd, note->descdata + PRSTATUS_OFFSET_PR_CURSIG);

      /* pr_pid  */
      elf_tdata (abfd)->core->lwpid =
	bfd_get_32 (abfd, note->descdata + PRSTATUS_OFFSET_PR_PID);
      break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg", ELF_GREGSET_T_SIZE,
					  note->descpos
					  + PRSTATUS_OFFSET_PR_REG);
}

#define PRPSINFO_SIZE		    0x88
#define PRPSINFO_OFFSET_PR_PID	    0x18
#define PRPSINFO_OFFSET_PR_FNAME    0x28
#define PRPSINFO_SIZEOF_PR_FNAME    0x10
#define PRPSINFO_OFFSET_PR_PS_ARGS  0x38
#define PRPSINFO_SIZEOF_PR_PS_ARGS  0x50

static bool
loongarch_elf_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    default:
      return false;

    /* The sizeof (prpsinfo_t) on Linux/LoongArch.  */
    case PRPSINFO_SIZE:
      /* pr_pid  */
      elf_tdata (abfd)->core->pid =
	bfd_get_32 (abfd, note->descdata + PRPSINFO_OFFSET_PR_PID);

      /* pr_fname  */
      elf_tdata (abfd)->core->program =
	_bfd_elfcore_strndup (abfd, note->descdata + PRPSINFO_OFFSET_PR_FNAME,
			      PRPSINFO_SIZEOF_PR_FNAME);

      /* pr_psargs  */
      elf_tdata (abfd)->core->command =
	_bfd_elfcore_strndup (abfd, note->descdata + PRPSINFO_OFFSET_PR_PS_ARGS,
			      PRPSINFO_SIZEOF_PR_PS_ARGS);
      break;
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

/* Set the right mach type.  */
static bool
loongarch_elf_object_p (bfd *abfd)
{
  /* There are only two mach types in LoongArch currently.  */
  if (strcmp (abfd->xvec->name, "elf64-loongarch") == 0)
    bfd_default_set_arch_mach (abfd, bfd_arch_loongarch, bfd_mach_loongarch64);
  else
    bfd_default_set_arch_mach (abfd, bfd_arch_loongarch, bfd_mach_loongarch32);
  return true;
}

static asection *
loongarch_elf_gc_mark_hook (asection *sec, struct bfd_link_info *info,
			    Elf_Internal_Rela *rel,
			    struct elf_link_hash_entry *h,
			    Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELFNN_R_TYPE (rel->r_info))
      {
      case R_LARCH_GNU_VTINHERIT:
      case R_LARCH_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Return TRUE if symbol H should be hashed in the `.gnu.hash' section.  For
   executable PLT slots where the executable never takes the address of those
   functions, the function symbols are not added to the hash table.  */

static bool
elf_loongarch64_hash_symbol (struct elf_link_hash_entry *h)
{
  if (h->plt.offset != (bfd_vma) -1
      && !h->def_regular
      && !h->pointer_equality_needed)
    return false;

  return _bfd_elf_hash_symbol (h);
}

#define TARGET_LITTLE_SYM loongarch_elfNN_vec
#define TARGET_LITTLE_NAME "elfNN-loongarch"
#define ELF_ARCH bfd_arch_loongarch
#define ELF_TARGET_ID LARCH_ELF_DATA
#define ELF_MACHINE_CODE EM_LOONGARCH
#define ELF_MAXPAGESIZE 0x4000
#define bfd_elfNN_bfd_reloc_type_lookup loongarch_reloc_type_lookup
#define bfd_elfNN_bfd_link_hash_table_create				  \
  loongarch_elf_link_hash_table_create
#define bfd_elfNN_bfd_reloc_name_lookup loongarch_reloc_name_lookup
#define elf_info_to_howto_rel NULL /* Fall through to elf_info_to_howto.  */
#define elf_info_to_howto loongarch_info_to_howto_rela
#define bfd_elfNN_bfd_merge_private_bfd_data				  \
  elfNN_loongarch_merge_private_bfd_data

#define elf_backend_reloc_type_class loongarch_reloc_type_class
#define elf_backend_copy_indirect_symbol loongarch_elf_copy_indirect_symbol
#define elf_backend_create_dynamic_sections				   \
  loongarch_elf_create_dynamic_sections
#define elf_backend_check_relocs loongarch_elf_check_relocs
#define elf_backend_adjust_dynamic_symbol loongarch_elf_adjust_dynamic_symbol
#define elf_backend_size_dynamic_sections loongarch_elf_size_dynamic_sections
#define elf_backend_relocate_section loongarch_elf_relocate_section
#define elf_backend_finish_dynamic_symbol loongarch_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections				   \
  loongarch_elf_finish_dynamic_sections
#define elf_backend_object_p loongarch_elf_object_p
#define elf_backend_gc_mark_hook loongarch_elf_gc_mark_hook
#define elf_backend_plt_sym_val loongarch_elf_plt_sym_val
#define elf_backend_grok_prstatus loongarch_elf_grok_prstatus
#define elf_backend_grok_psinfo loongarch_elf_grok_psinfo
#define elf_backend_hash_symbol elf_loongarch64_hash_symbol
#define bfd_elfNN_bfd_relax_section loongarch_elf_relax_section

#include "elfNN-target.h"
