/* ELF executable support for BFD.

   Copyright (C) 1993-2023 Free Software Foundation, Inc.

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


/*
SECTION
	ELF backends

	BFD support for ELF formats is being worked on.
	Currently, the best supported back ends are for sparc and i386
	(running svr4 or Solaris 2).

	Documentation of the internals of the support code still needs
	to be written.  The code is changing quickly enough that we
	haven't bothered yet.  */

/* For sparc64-cross-sparc32.  */
#define _SYSCALL32
#include "sysdep.h"
#include <limits.h>
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#define ARCH_SIZE 0
#include "elf-bfd.h"
#include "libiberty.h"
#include "safe-ctype.h"
#include "elf-linux-core.h"

#ifdef CORE_HEADER
#include CORE_HEADER
#endif

static int elf_sort_sections (const void *, const void *);
static bool assign_file_positions_except_relocs (bfd *, struct bfd_link_info *);
static bool swap_out_syms (bfd *, struct elf_strtab_hash **, int,
			   struct bfd_link_info *);
static bool elf_parse_notes (bfd *abfd, char *buf, size_t size,
			     file_ptr offset, size_t align);

/* Swap version information in and out.  The version information is
   currently size independent.  If that ever changes, this code will
   need to move into elfcode.h.  */

/* Swap in a Verdef structure.  */

void
_bfd_elf_swap_verdef_in (bfd *abfd,
			 const Elf_External_Verdef *src,
			 Elf_Internal_Verdef *dst)
{
  dst->vd_version = H_GET_16 (abfd, src->vd_version);
  dst->vd_flags   = H_GET_16 (abfd, src->vd_flags);
  dst->vd_ndx     = H_GET_16 (abfd, src->vd_ndx);
  dst->vd_cnt     = H_GET_16 (abfd, src->vd_cnt);
  dst->vd_hash    = H_GET_32 (abfd, src->vd_hash);
  dst->vd_aux     = H_GET_32 (abfd, src->vd_aux);
  dst->vd_next    = H_GET_32 (abfd, src->vd_next);
}

/* Swap out a Verdef structure.  */

void
_bfd_elf_swap_verdef_out (bfd *abfd,
			  const Elf_Internal_Verdef *src,
			  Elf_External_Verdef *dst)
{
  H_PUT_16 (abfd, src->vd_version, dst->vd_version);
  H_PUT_16 (abfd, src->vd_flags, dst->vd_flags);
  H_PUT_16 (abfd, src->vd_ndx, dst->vd_ndx);
  H_PUT_16 (abfd, src->vd_cnt, dst->vd_cnt);
  H_PUT_32 (abfd, src->vd_hash, dst->vd_hash);
  H_PUT_32 (abfd, src->vd_aux, dst->vd_aux);
  H_PUT_32 (abfd, src->vd_next, dst->vd_next);
}

/* Swap in a Verdaux structure.  */

void
_bfd_elf_swap_verdaux_in (bfd *abfd,
			  const Elf_External_Verdaux *src,
			  Elf_Internal_Verdaux *dst)
{
  dst->vda_name = H_GET_32 (abfd, src->vda_name);
  dst->vda_next = H_GET_32 (abfd, src->vda_next);
}

/* Swap out a Verdaux structure.  */

void
_bfd_elf_swap_verdaux_out (bfd *abfd,
			   const Elf_Internal_Verdaux *src,
			   Elf_External_Verdaux *dst)
{
  H_PUT_32 (abfd, src->vda_name, dst->vda_name);
  H_PUT_32 (abfd, src->vda_next, dst->vda_next);
}

/* Swap in a Verneed structure.  */

void
_bfd_elf_swap_verneed_in (bfd *abfd,
			  const Elf_External_Verneed *src,
			  Elf_Internal_Verneed *dst)
{
  dst->vn_version = H_GET_16 (abfd, src->vn_version);
  dst->vn_cnt     = H_GET_16 (abfd, src->vn_cnt);
  dst->vn_file    = H_GET_32 (abfd, src->vn_file);
  dst->vn_aux     = H_GET_32 (abfd, src->vn_aux);
  dst->vn_next    = H_GET_32 (abfd, src->vn_next);
}

/* Swap out a Verneed structure.  */

void
_bfd_elf_swap_verneed_out (bfd *abfd,
			   const Elf_Internal_Verneed *src,
			   Elf_External_Verneed *dst)
{
  H_PUT_16 (abfd, src->vn_version, dst->vn_version);
  H_PUT_16 (abfd, src->vn_cnt, dst->vn_cnt);
  H_PUT_32 (abfd, src->vn_file, dst->vn_file);
  H_PUT_32 (abfd, src->vn_aux, dst->vn_aux);
  H_PUT_32 (abfd, src->vn_next, dst->vn_next);
}

/* Swap in a Vernaux structure.  */

void
_bfd_elf_swap_vernaux_in (bfd *abfd,
			  const Elf_External_Vernaux *src,
			  Elf_Internal_Vernaux *dst)
{
  dst->vna_hash  = H_GET_32 (abfd, src->vna_hash);
  dst->vna_flags = H_GET_16 (abfd, src->vna_flags);
  dst->vna_other = H_GET_16 (abfd, src->vna_other);
  dst->vna_name  = H_GET_32 (abfd, src->vna_name);
  dst->vna_next  = H_GET_32 (abfd, src->vna_next);
}

/* Swap out a Vernaux structure.  */

void
_bfd_elf_swap_vernaux_out (bfd *abfd,
			   const Elf_Internal_Vernaux *src,
			   Elf_External_Vernaux *dst)
{
  H_PUT_32 (abfd, src->vna_hash, dst->vna_hash);
  H_PUT_16 (abfd, src->vna_flags, dst->vna_flags);
  H_PUT_16 (abfd, src->vna_other, dst->vna_other);
  H_PUT_32 (abfd, src->vna_name, dst->vna_name);
  H_PUT_32 (abfd, src->vna_next, dst->vna_next);
}

/* Swap in a Versym structure.  */

void
_bfd_elf_swap_versym_in (bfd *abfd,
			 const Elf_External_Versym *src,
			 Elf_Internal_Versym *dst)
{
  dst->vs_vers = H_GET_16 (abfd, src->vs_vers);
}

/* Swap out a Versym structure.  */

void
_bfd_elf_swap_versym_out (bfd *abfd,
			  const Elf_Internal_Versym *src,
			  Elf_External_Versym *dst)
{
  H_PUT_16 (abfd, src->vs_vers, dst->vs_vers);
}

/* Standard ELF hash function.  Do not change this function; you will
   cause invalid hash tables to be generated.  */

unsigned long
bfd_elf_hash (const char *namearg)
{
  uint32_t h = 0;

  for (const unsigned char *name = (const unsigned char *) namearg;
       *name; name++)
    {
      h = (h << 4) + *name;
      h ^= (h >> 24) & 0xf0;
    }
  return h & 0x0fffffff;
}

/* DT_GNU_HASH hash function.  Do not change this function; you will
   cause invalid hash tables to be generated.  */

unsigned long
bfd_elf_gnu_hash (const char *namearg)
{
  uint32_t h = 5381;

  for (const unsigned char *name = (const unsigned char *) namearg;
       *name; name++)
    h = (h << 5) + h + *name;
  return h;
}

/* Create a tdata field OBJECT_SIZE bytes in length, zeroed out and with
   the object_id field of an elf_obj_tdata field set to OBJECT_ID.  */
bool
bfd_elf_allocate_object (bfd *abfd,
			 size_t object_size,
			 enum elf_target_id object_id)
{
  BFD_ASSERT (object_size >= sizeof (struct elf_obj_tdata));
  abfd->tdata.any = bfd_zalloc (abfd, object_size);
  if (abfd->tdata.any == NULL)
    return false;

  elf_object_id (abfd) = object_id;
  if (abfd->direction != read_direction)
    {
      struct output_elf_obj_tdata *o = bfd_zalloc (abfd, sizeof *o);
      if (o == NULL)
	return false;
      elf_tdata (abfd)->o = o;
      elf_program_header_size (abfd) = (bfd_size_type) -1;
    }
  return true;
}


bool
bfd_elf_make_object (bfd *abfd)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_obj_tdata),
				  bed->target_id);
}

bool
bfd_elf_mkcorefile (bfd *abfd)
{
  /* I think this can be done just like an object file.  */
  if (!abfd->xvec->_bfd_set_format[(int) bfd_object] (abfd))
    return false;
  elf_tdata (abfd)->core = bfd_zalloc (abfd, sizeof (*elf_tdata (abfd)->core));
  return elf_tdata (abfd)->core != NULL;
}

char *
bfd_elf_get_str_section (bfd *abfd, unsigned int shindex)
{
  Elf_Internal_Shdr **i_shdrp;
  bfd_byte *shstrtab = NULL;
  file_ptr offset;
  bfd_size_type shstrtabsize;

  i_shdrp = elf_elfsections (abfd);
  if (i_shdrp == 0
      || shindex >= elf_numsections (abfd)
      || i_shdrp[shindex] == 0)
    return NULL;

  shstrtab = i_shdrp[shindex]->contents;
  if (shstrtab == NULL)
    {
      /* No cached one, attempt to read, and cache what we read.  */
      offset = i_shdrp[shindex]->sh_offset;
      shstrtabsize = i_shdrp[shindex]->sh_size;

      /* Allocate and clear an extra byte at the end, to prevent crashes
	 in case the string table is not terminated.  */
      if (shstrtabsize + 1 <= 1
	  || bfd_seek (abfd, offset, SEEK_SET) != 0
	  || (shstrtab = _bfd_alloc_and_read (abfd, shstrtabsize + 1,
					      shstrtabsize)) == NULL)
	{
	  /* Once we've failed to read it, make sure we don't keep
	     trying.  Otherwise, we'll keep allocating space for
	     the string table over and over.  */
	  i_shdrp[shindex]->sh_size = 0;
	}
      else
	shstrtab[shstrtabsize] = '\0';
      i_shdrp[shindex]->contents = shstrtab;
    }
  return (char *) shstrtab;
}

char *
bfd_elf_string_from_elf_section (bfd *abfd,
				 unsigned int shindex,
				 unsigned int strindex)
{
  Elf_Internal_Shdr *hdr;

  if (strindex == 0)
    return "";

  if (elf_elfsections (abfd) == NULL || shindex >= elf_numsections (abfd))
    return NULL;

  hdr = elf_elfsections (abfd)[shindex];

  if (hdr->contents == NULL)
    {
      if (hdr->sh_type != SHT_STRTAB && hdr->sh_type < SHT_LOOS)
	{
	  /* PR 17512: file: f057ec89.  */
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: attempt to load strings from"
				" a non-string section (number %d)"),
			      abfd, shindex);
	  return NULL;
	}

      if (bfd_elf_get_str_section (abfd, shindex) == NULL)
	return NULL;
    }
  else
    {
      /* PR 24273: The string section's contents may have already
	 been loaded elsewhere, eg because a corrupt file has the
	 string section index in the ELF header pointing at a group
	 section.  So be paranoid, and test that the last byte of
	 the section is zero.  */
      if (hdr->sh_size == 0 || hdr->contents[hdr->sh_size - 1] != 0)
	return NULL;
    }

  if (strindex >= hdr->sh_size)
    {
      unsigned int shstrndx = elf_elfheader(abfd)->e_shstrndx;
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: invalid string offset %u >= %" PRIu64 " for section `%s'"),
	 abfd, strindex, (uint64_t) hdr->sh_size,
	 (shindex == shstrndx && strindex == hdr->sh_name
	  ? ".shstrtab"
	  : bfd_elf_string_from_elf_section (abfd, shstrndx, hdr->sh_name)));
      return NULL;
    }

  return ((char *) hdr->contents) + strindex;
}

/* Read and convert symbols to internal format.
   SYMCOUNT specifies the number of symbols to read, starting from
   symbol SYMOFFSET.  If any of INTSYM_BUF, EXTSYM_BUF or EXTSHNDX_BUF
   are non-NULL, they are used to store the internal symbols, external
   symbols, and symbol section index extensions, respectively.
   Returns a pointer to the internal symbol buffer (malloced if necessary)
   or NULL if there were no symbols or some kind of problem.  */

Elf_Internal_Sym *
bfd_elf_get_elf_syms (bfd *ibfd,
		      Elf_Internal_Shdr *symtab_hdr,
		      size_t symcount,
		      size_t symoffset,
		      Elf_Internal_Sym *intsym_buf,
		      void *extsym_buf,
		      Elf_External_Sym_Shndx *extshndx_buf)
{
  Elf_Internal_Shdr *shndx_hdr;
  void *alloc_ext;
  const bfd_byte *esym;
  Elf_External_Sym_Shndx *alloc_extshndx;
  Elf_External_Sym_Shndx *shndx;
  Elf_Internal_Sym *alloc_intsym;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  const struct elf_backend_data *bed;
  size_t extsym_size;
  size_t amt;
  file_ptr pos;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
    abort ();

  if (symcount == 0)
    return intsym_buf;

  if (elf_use_dt_symtab_p (ibfd))
    {
      /* Use dynamic symbol table.  */
      if (elf_tdata (ibfd)->dt_symtab_count != symcount + symoffset)
	{
	  bfd_set_error (bfd_error_invalid_operation);
	  return NULL;
	}
      return elf_tdata (ibfd)->dt_symtab + symoffset;
    }

  /* Normal syms might have section extension entries.  */
  shndx_hdr = NULL;
  if (elf_symtab_shndx_list (ibfd) != NULL)
    {
      elf_section_list * entry;
      Elf_Internal_Shdr **sections = elf_elfsections (ibfd);

      /* Find an index section that is linked to this symtab section.  */
      for (entry = elf_symtab_shndx_list (ibfd); entry != NULL; entry = entry->next)
	{
	  /* PR 20063.  */
	  if (entry->hdr.sh_link >= elf_numsections (ibfd))
	    continue;

	  if (sections[entry->hdr.sh_link] == symtab_hdr)
	    {
	      shndx_hdr = & entry->hdr;
	      break;
	    };
	}

      if (shndx_hdr == NULL)
	{
	  if (symtab_hdr == &elf_symtab_hdr (ibfd))
	    /* Not really accurate, but this was how the old code used
	       to work.  */
	    shndx_hdr = &elf_symtab_shndx_list (ibfd)->hdr;
	  /* Otherwise we do nothing.  The assumption is that
	     the index table will not be needed.  */
	}
    }

  /* Read the symbols.  */
  alloc_ext = NULL;
  alloc_extshndx = NULL;
  alloc_intsym = NULL;
  bed = get_elf_backend_data (ibfd);
  extsym_size = bed->s->sizeof_sym;
  if (_bfd_mul_overflow (symcount, extsym_size, &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      intsym_buf = NULL;
      goto out;
    }
  pos = symtab_hdr->sh_offset + symoffset * extsym_size;
  if (extsym_buf == NULL)
    {
      alloc_ext = bfd_malloc (amt);
      extsym_buf = alloc_ext;
    }
  if (extsym_buf == NULL
      || bfd_seek (ibfd, pos, SEEK_SET) != 0
      || bfd_bread (extsym_buf, amt, ibfd) != amt)
    {
      intsym_buf = NULL;
      goto out;
    }

  if (shndx_hdr == NULL || shndx_hdr->sh_size == 0)
    extshndx_buf = NULL;
  else
    {
      if (_bfd_mul_overflow (symcount, sizeof (Elf_External_Sym_Shndx), &amt))
	{
	  bfd_set_error (bfd_error_file_too_big);
	  intsym_buf = NULL;
	  goto out;
	}
      pos = shndx_hdr->sh_offset + symoffset * sizeof (Elf_External_Sym_Shndx);
      if (extshndx_buf == NULL)
	{
	  alloc_extshndx = (Elf_External_Sym_Shndx *) bfd_malloc (amt);
	  extshndx_buf = alloc_extshndx;
	}
      if (extshndx_buf == NULL
	  || bfd_seek (ibfd, pos, SEEK_SET) != 0
	  || bfd_bread (extshndx_buf, amt, ibfd) != amt)
	{
	  intsym_buf = NULL;
	  goto out;
	}
    }

  if (intsym_buf == NULL)
    {
      if (_bfd_mul_overflow (symcount, sizeof (Elf_Internal_Sym), &amt))
	{
	  bfd_set_error (bfd_error_file_too_big);
	  goto out;
	}
      alloc_intsym = (Elf_Internal_Sym *) bfd_malloc (amt);
      intsym_buf = alloc_intsym;
      if (intsym_buf == NULL)
	goto out;
    }

  /* Convert the symbols to internal form.  */
  isymend = intsym_buf + symcount;
  for (esym = (const bfd_byte *) extsym_buf, isym = intsym_buf,
	   shndx = extshndx_buf;
       isym < isymend;
       esym += extsym_size, isym++, shndx = shndx != NULL ? shndx + 1 : NULL)
    if (!(*bed->s->swap_symbol_in) (ibfd, esym, shndx, isym))
      {
	symoffset += (esym - (bfd_byte *) extsym_buf) / extsym_size;
	/* xgettext:c-format */
	_bfd_error_handler (_("%pB symbol number %lu references"
			      " nonexistent SHT_SYMTAB_SHNDX section"),
			    ibfd, (unsigned long) symoffset);
	free (alloc_intsym);
	intsym_buf = NULL;
	goto out;
      }

 out:
  free (alloc_ext);
  free (alloc_extshndx);

  return intsym_buf;
}

/* Look up a symbol name.  */
const char *
bfd_elf_sym_name (bfd *abfd,
		  Elf_Internal_Shdr *symtab_hdr,
		  Elf_Internal_Sym *isym,
		  asection *sym_sec)
{
  const char *name;
  unsigned int iname = isym->st_name;
  unsigned int shindex = symtab_hdr->sh_link;

  if (iname == 0 && ELF_ST_TYPE (isym->st_info) == STT_SECTION
      /* Check for a bogus st_shndx to avoid crashing.  */
      && isym->st_shndx < elf_numsections (abfd))
    {
      iname = elf_elfsections (abfd)[isym->st_shndx]->sh_name;
      shindex = elf_elfheader (abfd)->e_shstrndx;
    }

  name = bfd_elf_string_from_elf_section (abfd, shindex, iname);
  if (name == NULL)
    name = "(null)";
  else if (sym_sec && *name == '\0')
    name = bfd_section_name (sym_sec);

  return name;
}

/* Elf_Internal_Shdr->contents is an array of these for SHT_GROUP
   sections.  The first element is the flags, the rest are section
   pointers.  */

typedef union elf_internal_group {
  Elf_Internal_Shdr *shdr;
  unsigned int flags;
} Elf_Internal_Group;

/* Return the name of the group signature symbol.  Why isn't the
   signature just a string?  */

static const char *
group_signature (bfd *abfd, Elf_Internal_Shdr *ghdr)
{
  Elf_Internal_Shdr *hdr;
  unsigned char esym[sizeof (Elf64_External_Sym)];
  Elf_External_Sym_Shndx eshndx;
  Elf_Internal_Sym isym;

  /* First we need to ensure the symbol table is available.  Make sure
     that it is a symbol table section.  */
  if (ghdr->sh_link >= elf_numsections (abfd))
    return NULL;
  hdr = elf_elfsections (abfd) [ghdr->sh_link];
  if (hdr->sh_type != SHT_SYMTAB
      || ! bfd_section_from_shdr (abfd, ghdr->sh_link))
    return NULL;

  /* Go read the symbol.  */
  hdr = &elf_tdata (abfd)->symtab_hdr;
  if (bfd_elf_get_elf_syms (abfd, hdr, 1, ghdr->sh_info,
			    &isym, esym, &eshndx) == NULL)
    return NULL;

  return bfd_elf_sym_name (abfd, hdr, &isym, NULL);
}

/* Set next_in_group list pointer, and group name for NEWSECT.  */

static bool
setup_group (bfd *abfd, Elf_Internal_Shdr *hdr, asection *newsect)
{
  unsigned int num_group = elf_tdata (abfd)->num_group;

  /* If num_group is zero, read in all SHT_GROUP sections.  The count
     is set to -1 if there are no SHT_GROUP sections.  */
  if (num_group == 0)
    {
      unsigned int i, shnum;

      /* First count the number of groups.  If we have a SHT_GROUP
	 section with just a flag word (ie. sh_size is 4), ignore it.  */
      shnum = elf_numsections (abfd);
      num_group = 0;

#define IS_VALID_GROUP_SECTION_HEADER(shdr, minsize)	\
	(   (shdr)->sh_type == SHT_GROUP		\
	 && (shdr)->sh_size >= minsize			\
	 && (shdr)->sh_entsize == GRP_ENTRY_SIZE	\
	 && ((shdr)->sh_size % GRP_ENTRY_SIZE) == 0)

      for (i = 0; i < shnum; i++)
	{
	  Elf_Internal_Shdr *shdr = elf_elfsections (abfd)[i];

	  if (IS_VALID_GROUP_SECTION_HEADER (shdr, 2 * GRP_ENTRY_SIZE))
	    num_group += 1;
	}

      if (num_group == 0)
	{
	  num_group = (unsigned) -1;
	  elf_tdata (abfd)->num_group = num_group;
	  elf_tdata (abfd)->group_sect_ptr = NULL;
	}
      else
	{
	  /* We keep a list of elf section headers for group sections,
	     so we can find them quickly.  */
	  size_t amt;

	  elf_tdata (abfd)->num_group = num_group;
	  amt = num_group * sizeof (Elf_Internal_Shdr *);
	  elf_tdata (abfd)->group_sect_ptr
	    = (Elf_Internal_Shdr **) bfd_zalloc (abfd, amt);
	  if (elf_tdata (abfd)->group_sect_ptr == NULL)
	    return false;
	  num_group = 0;

	  for (i = 0; i < shnum; i++)
	    {
	      Elf_Internal_Shdr *shdr = elf_elfsections (abfd)[i];

	      if (IS_VALID_GROUP_SECTION_HEADER (shdr, 2 * GRP_ENTRY_SIZE))
		{
		  unsigned char *src;
		  Elf_Internal_Group *dest;

		  /* Make sure the group section has a BFD section
		     attached to it.  */
		  if (!bfd_section_from_shdr (abfd, i))
		    return false;

		  /* Add to list of sections.  */
		  elf_tdata (abfd)->group_sect_ptr[num_group] = shdr;
		  num_group += 1;

		  /* Read the raw contents.  */
		  BFD_ASSERT (sizeof (*dest) >= 4 && sizeof (*dest) % 4 == 0);
		  shdr->contents = NULL;
		  if (_bfd_mul_overflow (shdr->sh_size,
					 sizeof (*dest) / 4, &amt)
		      || bfd_seek (abfd, shdr->sh_offset, SEEK_SET) != 0
		      || !(shdr->contents
			   = _bfd_alloc_and_read (abfd, amt, shdr->sh_size)))
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB: invalid size field in group section"
			   " header: %#" PRIx64 ""),
			 abfd, (uint64_t) shdr->sh_size);
		      bfd_set_error (bfd_error_bad_value);
		      -- num_group;
		      continue;
		    }

		  /* Translate raw contents, a flag word followed by an
		     array of elf section indices all in target byte order,
		     to the flag word followed by an array of elf section
		     pointers.  */
		  src = shdr->contents + shdr->sh_size;
		  dest = (Elf_Internal_Group *) (shdr->contents + amt);

		  while (1)
		    {
		      unsigned int idx;

		      src -= 4;
		      --dest;
		      idx = H_GET_32 (abfd, src);
		      if (src == shdr->contents)
			{
			  dest->shdr = NULL;
			  dest->flags = idx;
			  if (shdr->bfd_section != NULL && (idx & GRP_COMDAT))
			    shdr->bfd_section->flags
			      |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;
			  break;
			}
		      if (idx < shnum)
			{
			  dest->shdr = elf_elfsections (abfd)[idx];
			  /* PR binutils/23199: All sections in a
			     section group should be marked with
			     SHF_GROUP.  But some tools generate
			     broken objects without SHF_GROUP.  Fix
			     them up here.  */
			  dest->shdr->sh_flags |= SHF_GROUP;
			}
		      if (idx >= shnum
			  || dest->shdr->sh_type == SHT_GROUP)
			{
			  _bfd_error_handler
			    (_("%pB: invalid entry in SHT_GROUP section [%u]"),
			       abfd, i);
			  dest->shdr = NULL;
			}
		    }
		}
	    }

	  /* PR 17510: Corrupt binaries might contain invalid groups.  */
	  if (num_group != (unsigned) elf_tdata (abfd)->num_group)
	    {
	      elf_tdata (abfd)->num_group = num_group;

	      /* If all groups are invalid then fail.  */
	      if (num_group == 0)
		{
		  elf_tdata (abfd)->group_sect_ptr = NULL;
		  elf_tdata (abfd)->num_group = num_group = -1;
		  _bfd_error_handler
		    (_("%pB: no valid group sections found"), abfd);
		  bfd_set_error (bfd_error_bad_value);
		}
	    }
	}
    }

  if (num_group != (unsigned) -1)
    {
      unsigned int search_offset = elf_tdata (abfd)->group_search_offset;
      unsigned int j;

      for (j = 0; j < num_group; j++)
	{
	  /* Begin search from previous found group.  */
	  unsigned i = (j + search_offset) % num_group;

	  Elf_Internal_Shdr *shdr = elf_tdata (abfd)->group_sect_ptr[i];
	  Elf_Internal_Group *idx;
	  bfd_size_type n_elt;

	  if (shdr == NULL)
	    continue;

	  idx = (Elf_Internal_Group *) shdr->contents;
	  if (idx == NULL || shdr->sh_size < 4)
	    {
	      /* See PR 21957 for a reproducer.  */
	      /* xgettext:c-format */
	      _bfd_error_handler (_("%pB: group section '%pA' has no contents"),
				  abfd, shdr->bfd_section);
	      elf_tdata (abfd)->group_sect_ptr[i] = NULL;
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  n_elt = shdr->sh_size / 4;

	  /* Look through this group's sections to see if current
	     section is a member.  */
	  while (--n_elt != 0)
	    if ((++idx)->shdr == hdr)
	      {
		asection *s = NULL;

		/* We are a member of this group.  Go looking through
		   other members to see if any others are linked via
		   next_in_group.  */
		idx = (Elf_Internal_Group *) shdr->contents;
		n_elt = shdr->sh_size / 4;
		while (--n_elt != 0)
		  if ((++idx)->shdr != NULL
		      && (s = idx->shdr->bfd_section) != NULL
		      && elf_next_in_group (s) != NULL)
		    break;
		if (n_elt != 0)
		  {
		    /* Snarf the group name from other member, and
		       insert current section in circular list.  */
		    elf_group_name (newsect) = elf_group_name (s);
		    elf_next_in_group (newsect) = elf_next_in_group (s);
		    elf_next_in_group (s) = newsect;
		  }
		else
		  {
		    const char *gname;

		    gname = group_signature (abfd, shdr);
		    if (gname == NULL)
		      return false;
		    elf_group_name (newsect) = gname;

		    /* Start a circular list with one element.  */
		    elf_next_in_group (newsect) = newsect;
		  }

		/* If the group section has been created, point to the
		   new member.  */
		if (shdr->bfd_section != NULL)
		  elf_next_in_group (shdr->bfd_section) = newsect;

		elf_tdata (abfd)->group_search_offset = i;
		j = num_group - 1;
		break;
	      }
	}
    }

  if (elf_group_name (newsect) == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: no group info for section '%pA'"),
			  abfd, newsect);
      /* PR 29532: Return true here, even though the group info has not been
	 read.  Separate debug info files can have empty group sections, but
	 we do not want this to prevent them from being loaded as otherwise
	 GDB will not be able to use them.  */
      return true;
    }
  return true;
}

bool
_bfd_elf_setup_sections (bfd *abfd)
{
  unsigned int i;
  unsigned int num_group = elf_tdata (abfd)->num_group;
  bool result = true;
  asection *s;

  /* Process SHF_LINK_ORDER.  */
  for (s = abfd->sections; s != NULL; s = s->next)
    {
      Elf_Internal_Shdr *this_hdr = &elf_section_data (s)->this_hdr;
      if ((this_hdr->sh_flags & SHF_LINK_ORDER) != 0)
	{
	  unsigned int elfsec = this_hdr->sh_link;
	  /* An sh_link value of 0 is now allowed.  It indicates that linked
	     to section has already been discarded, but that the current
	     section has been retained for some other reason.  This linking
	     section is still a candidate for later garbage collection
	     however.  */
	  if (elfsec == 0)
	    {
	      elf_linked_to_section (s) = NULL;
	    }
	  else
	    {
	      asection *linksec = NULL;

	      if (elfsec < elf_numsections (abfd))
		{
		  this_hdr = elf_elfsections (abfd)[elfsec];
		  linksec = this_hdr->bfd_section;
		}

	      /* PR 1991, 2008:
		 Some strip/objcopy may leave an incorrect value in
		 sh_link.  We don't want to proceed.  */
	      if (linksec == NULL)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: sh_link [%d] in section `%pA' is incorrect"),
		     s->owner, elfsec, s);
		  result = false;
		}

	      elf_linked_to_section (s) = linksec;
	    }
	}
      else if (this_hdr->sh_type == SHT_GROUP
	       && elf_next_in_group (s) == NULL)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: SHT_GROUP section [index %d] has no SHF_GROUP sections"),
	     abfd, elf_section_data (s)->this_idx);
	  result = false;
	}
    }

  /* Process section groups.  */
  if (num_group == (unsigned) -1)
    return result;

  for (i = 0; i < num_group; i++)
    {
      Elf_Internal_Shdr *shdr = elf_tdata (abfd)->group_sect_ptr[i];
      Elf_Internal_Group *idx;
      unsigned int n_elt;

      /* PR binutils/18758: Beware of corrupt binaries with invalid
	 group data.  */
      if (shdr == NULL || shdr->bfd_section == NULL || shdr->contents == NULL)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: section group entry number %u is corrupt"),
	     abfd, i);
	  result = false;
	  continue;
	}

      idx = (Elf_Internal_Group *) shdr->contents;
      n_elt = shdr->sh_size / 4;

      while (--n_elt != 0)
	{
	  ++ idx;

	  if (idx->shdr == NULL)
	    continue;
	  else if (idx->shdr->bfd_section)
	    elf_sec_group (idx->shdr->bfd_section) = shdr->bfd_section;
	  else if (idx->shdr->sh_type != SHT_RELA
		   && idx->shdr->sh_type != SHT_REL)
	    {
	      /* There are some unknown sections in the group.  */
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: unknown type [%#x] section `%s' in group [%pA]"),
		 abfd,
		 idx->shdr->sh_type,
		 bfd_elf_string_from_elf_section (abfd,
						  (elf_elfheader (abfd)
						   ->e_shstrndx),
						  idx->shdr->sh_name),
		 shdr->bfd_section);
	      result = false;
	    }
	}
    }

  return result;
}

bool
bfd_elf_is_group_section (bfd *abfd ATTRIBUTE_UNUSED, const asection *sec)
{
  return elf_next_in_group (sec) != NULL;
}

const char *
bfd_elf_group_name (bfd *abfd ATTRIBUTE_UNUSED, const asection *sec)
{
  if (elf_sec_group (sec) != NULL)
    return elf_group_name (sec);
  return NULL;
}

/* This a copy of lto_section defined in GCC (lto-streamer.h).  */

struct lto_section
{
  int16_t major_version;
  int16_t minor_version;
  unsigned char slim_object;

  /* Flags is a private field that is not defined publicly.  */
  uint16_t flags;
};

/* Make a BFD section from an ELF section.  We store a pointer to the
   BFD section in the bfd_section field of the header.  */

bool
_bfd_elf_make_section_from_shdr (bfd *abfd,
				 Elf_Internal_Shdr *hdr,
				 const char *name,
				 int shindex)
{
  asection *newsect;
  flagword flags;
  const struct elf_backend_data *bed;
  unsigned int opb = bfd_octets_per_byte (abfd, NULL);

  if (hdr->bfd_section != NULL)
    return true;

  newsect = bfd_make_section_anyway (abfd, name);
  if (newsect == NULL)
    return false;

  hdr->bfd_section = newsect;
  elf_section_data (newsect)->this_hdr = *hdr;
  elf_section_data (newsect)->this_idx = shindex;

  /* Always use the real type/flags.  */
  elf_section_type (newsect) = hdr->sh_type;
  elf_section_flags (newsect) = hdr->sh_flags;

  newsect->filepos = hdr->sh_offset;

  flags = SEC_NO_FLAGS;
  if (hdr->sh_type != SHT_NOBITS)
    flags |= SEC_HAS_CONTENTS;
  if (hdr->sh_type == SHT_GROUP)
    flags |= SEC_GROUP;
  if ((hdr->sh_flags & SHF_ALLOC) != 0)
    {
      flags |= SEC_ALLOC;
      if (hdr->sh_type != SHT_NOBITS)
	flags |= SEC_LOAD;
    }
  if ((hdr->sh_flags & SHF_WRITE) == 0)
    flags |= SEC_READONLY;
  if ((hdr->sh_flags & SHF_EXECINSTR) != 0)
    flags |= SEC_CODE;
  else if ((flags & SEC_LOAD) != 0)
    flags |= SEC_DATA;
  if ((hdr->sh_flags & SHF_MERGE) != 0)
    {
      flags |= SEC_MERGE;
      newsect->entsize = hdr->sh_entsize;
    }
  if ((hdr->sh_flags & SHF_STRINGS) != 0)
    flags |= SEC_STRINGS;
  if (hdr->sh_flags & SHF_GROUP)
    if (!setup_group (abfd, hdr, newsect))
      return false;
  if ((hdr->sh_flags & SHF_TLS) != 0)
    flags |= SEC_THREAD_LOCAL;
  if ((hdr->sh_flags & SHF_EXCLUDE) != 0)
    flags |= SEC_EXCLUDE;

  switch (elf_elfheader (abfd)->e_ident[EI_OSABI])
    {
      /* FIXME: We should not recognize SHF_GNU_MBIND for ELFOSABI_NONE,
	 but binutils as of 2019-07-23 did not set the EI_OSABI header
	 byte.  */
    case ELFOSABI_GNU:
    case ELFOSABI_FREEBSD:
      if ((hdr->sh_flags & SHF_GNU_RETAIN) != 0)
	elf_tdata (abfd)->has_gnu_osabi |= elf_gnu_osabi_retain;
      /* Fall through */
    case ELFOSABI_NONE:
      if ((hdr->sh_flags & SHF_GNU_MBIND) != 0)
	elf_tdata (abfd)->has_gnu_osabi |= elf_gnu_osabi_mbind;
      break;
    }

  if ((flags & SEC_ALLOC) == 0)
    {
      /* The debugging sections appear to be recognized only by name,
	 not any sort of flag.  Their SEC_ALLOC bits are cleared.  */
      if (name [0] == '.')
	{
	  if (startswith (name, ".debug")
	      || startswith (name, ".gnu.debuglto_.debug_")
	      || startswith (name, ".gnu.linkonce.wi.")
	      || startswith (name, ".zdebug"))
	    flags |= SEC_DEBUGGING | SEC_ELF_OCTETS;
	  else if (startswith (name, GNU_BUILD_ATTRS_SECTION_NAME)
		   || startswith (name, ".note.gnu"))
	    {
	      flags |= SEC_ELF_OCTETS;
	      opb = 1;
	    }
	  else if (startswith (name, ".line")
		   || startswith (name, ".stab")
		   || strcmp (name, ".gdb_index") == 0)
	    flags |= SEC_DEBUGGING;
	}
    }

  if (!bfd_set_section_vma (newsect, hdr->sh_addr / opb)
      || !bfd_set_section_size (newsect, hdr->sh_size)
      || !bfd_set_section_alignment (newsect, bfd_log2 (hdr->sh_addralign
							& -hdr->sh_addralign)))
    return false;

  /* As a GNU extension, if the name begins with .gnu.linkonce, we
     only link a single copy of the section.  This is used to support
     g++.  g++ will emit each template expansion in its own section.
     The symbols will be defined as weak, so that multiple definitions
     are permitted.  The GNU linker extension is to actually discard
     all but one of the sections.  */
  if (startswith (name, ".gnu.linkonce")
      && elf_next_in_group (newsect) == NULL)
    flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;

  if (!bfd_set_section_flags (newsect, flags))
    return false;

  bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_section_flags)
    if (!bed->elf_backend_section_flags (hdr))
      return false;

  /* We do not parse the PT_NOTE segments as we are interested even in the
     separate debug info files which may have the segments offsets corrupted.
     PT_NOTEs from the core files are currently not parsed using BFD.  */
  if (hdr->sh_type == SHT_NOTE && hdr->sh_size != 0)
    {
      bfd_byte *contents;

      if (!bfd_malloc_and_get_section (abfd, newsect, &contents))
	return false;

      elf_parse_notes (abfd, (char *) contents, hdr->sh_size,
		       hdr->sh_offset, hdr->sh_addralign);
      free (contents);
    }

  if ((newsect->flags & SEC_ALLOC) != 0)
    {
      Elf_Internal_Phdr *phdr;
      unsigned int i, nload;

      /* Some ELF linkers produce binaries with all the program header
	 p_paddr fields zero.  If we have such a binary with more than
	 one PT_LOAD header, then leave the section lma equal to vma
	 so that we don't create sections with overlapping lma.  */
      phdr = elf_tdata (abfd)->phdr;
      for (nload = 0, i = 0; i < elf_elfheader (abfd)->e_phnum; i++, phdr++)
	if (phdr->p_paddr != 0)
	  break;
	else if (phdr->p_type == PT_LOAD && phdr->p_memsz != 0)
	  ++nload;
      if (i >= elf_elfheader (abfd)->e_phnum && nload > 1)
	return true;

      phdr = elf_tdata (abfd)->phdr;
      for (i = 0; i < elf_elfheader (abfd)->e_phnum; i++, phdr++)
	{
	  if (((phdr->p_type == PT_LOAD
		&& (hdr->sh_flags & SHF_TLS) == 0)
	       || phdr->p_type == PT_TLS)
	      && ELF_SECTION_IN_SEGMENT (hdr, phdr))
	    {
	      if ((newsect->flags & SEC_LOAD) == 0)
		newsect->lma = (phdr->p_paddr
				+ hdr->sh_addr - phdr->p_vaddr) / opb;
	      else
		/* We used to use the same adjustment for SEC_LOAD
		   sections, but that doesn't work if the segment
		   is packed with code from multiple VMAs.
		   Instead we calculate the section LMA based on
		   the segment LMA.  It is assumed that the
		   segment will contain sections with contiguous
		   LMAs, even if the VMAs are not.  */
		newsect->lma = (phdr->p_paddr
				+ hdr->sh_offset - phdr->p_offset) / opb;

	      /* With contiguous segments, we can't tell from file
		 offsets whether a section with zero size should
		 be placed at the end of one segment or the
		 beginning of the next.  Decide based on vaddr.  */
	      if (hdr->sh_addr >= phdr->p_vaddr
		  && (hdr->sh_addr + hdr->sh_size
		      <= phdr->p_vaddr + phdr->p_memsz))
		break;
	    }
	}
    }

  /* Compress/decompress DWARF debug sections with names: .debug_*,
     .zdebug_*, .gnu.debuglto_.debug_, after the section flags is set.  */
  if ((newsect->flags & SEC_DEBUGGING) != 0
      && (newsect->flags & SEC_HAS_CONTENTS) != 0
      && (newsect->flags & SEC_ELF_OCTETS) != 0)
    {
      enum { nothing, compress, decompress } action = nothing;
      int compression_header_size;
      bfd_size_type uncompressed_size;
      unsigned int uncompressed_align_power;
      enum compression_type ch_type = ch_none;
      bool compressed
	= bfd_is_section_compressed_info (abfd, newsect,
					  &compression_header_size,
					  &uncompressed_size,
					  &uncompressed_align_power,
					  &ch_type);

      /* Should we decompress?  */
      if ((abfd->flags & BFD_DECOMPRESS) != 0 && compressed)
	action = decompress;

      /* Should we compress?  Or convert to a different compression?  */
      else if ((abfd->flags & BFD_COMPRESS) != 0
	       && newsect->size != 0
	       && compression_header_size >= 0
	       && uncompressed_size > 0)
	{
	  if (!compressed)
	    action = compress;
	  else
	    {
	      enum compression_type new_ch_type = ch_none;
	      if ((abfd->flags & BFD_COMPRESS_GABI) != 0)
		new_ch_type = ((abfd->flags & BFD_COMPRESS_ZSTD) != 0
			       ? ch_compress_zstd : ch_compress_zlib);
	      if (new_ch_type != ch_type)
		action = compress;
	    }
	}

      if (action == compress)
	{
	  if (!bfd_init_section_compress_status (abfd, newsect))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: unable to compress section %s"), abfd, name);
	      return false;
	    }
	}
      else if (action == decompress)
	{
	  if (!bfd_init_section_decompress_status (abfd, newsect))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: unable to decompress section %s"), abfd, name);
	      return false;
	    }
#ifndef HAVE_ZSTD
	  if (newsect->compress_status == DECOMPRESS_SECTION_ZSTD)
	    {
	      _bfd_error_handler
		  /* xgettext:c-format */
		  (_ ("%pB: section %s is compressed with zstd, but BFD "
		      "is not built with zstd support"),
		   abfd, name);
	      newsect->compress_status = COMPRESS_SECTION_NONE;
	      return false;
	    }
#endif
	  if (abfd->is_linker_input
	      && name[1] == 'z')
	    {
	      /* Rename section from .zdebug_* to .debug_* so that ld
		 scripts will see this section as a debug section.  */
	      char *new_name = bfd_zdebug_name_to_debug (abfd, name);
	      if (new_name == NULL)
		return false;
	      bfd_rename_section (newsect, new_name);
	    }
	}
    }

  /* GCC uses .gnu.lto_.lto.<some_hash> as a LTO bytecode information
     section.  */
  if (startswith (name, ".gnu.lto_.lto."))
    {
      struct lto_section lsection;
      if (bfd_get_section_contents (abfd, newsect, &lsection, 0,
				    sizeof (struct lto_section)))
	abfd->lto_slim_object = lsection.slim_object;
    }

  return true;
}

const char *const bfd_elf_section_type_names[] =
{
  "SHT_NULL", "SHT_PROGBITS", "SHT_SYMTAB", "SHT_STRTAB",
  "SHT_RELA", "SHT_HASH", "SHT_DYNAMIC", "SHT_NOTE",
  "SHT_NOBITS", "SHT_REL", "SHT_SHLIB", "SHT_DYNSYM",
};

/* ELF relocs are against symbols.  If we are producing relocatable
   output, and the reloc is against an external symbol, and nothing
   has given us any additional addend, the resulting reloc will also
   be against the same symbol.  In such a case, we don't want to
   change anything about the way the reloc is handled, since it will
   all be done at final link time.  Rather than put special case code
   into bfd_perform_relocation, all the reloc types use this howto
   function, or should call this function for relocatable output.  */

bfd_reloc_status_type
bfd_elf_generic_reloc (bfd *abfd ATTRIBUTE_UNUSED,
		       arelent *reloc_entry,
		       asymbol *symbol,
		       void *data ATTRIBUTE_UNUSED,
		       asection *input_section,
		       bfd *output_bfd,
		       char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! reloc_entry->howto->partial_inplace
	  || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  /* In some cases the relocation should be treated as output section
     relative, as when linking ELF DWARF into PE COFF.  Many ELF
     targets lack section relative relocations and instead use
     ordinary absolute relocations for references between DWARF
     sections.  That is arguably a bug in those targets but it happens
     to work for the usual case of linking to non-loaded ELF debug
     sections with VMAs forced to zero.  PE COFF on the other hand
     doesn't allow a section VMA of zero.  */
  if (output_bfd == NULL
      && !reloc_entry->howto->pc_relative
      && (symbol->section->flags & SEC_DEBUGGING) != 0
      && (input_section->flags & SEC_DEBUGGING) != 0)
    reloc_entry->addend -= symbol->section->output_section->vma;

  return bfd_reloc_continue;
}

/* Returns TRUE if section A matches section B.
   Names, addresses and links may be different, but everything else
   should be the same.  */

static bool
section_match (const Elf_Internal_Shdr * a,
	       const Elf_Internal_Shdr * b)
{
  if (a->sh_type != b->sh_type
      || ((a->sh_flags ^ b->sh_flags) & ~SHF_INFO_LINK) != 0
      || a->sh_addralign != b->sh_addralign
      || a->sh_entsize != b->sh_entsize)
    return false;
  if (a->sh_type == SHT_SYMTAB
      || a->sh_type == SHT_STRTAB)
    return true;
  return a->sh_size == b->sh_size;
}

/* Find a section in OBFD that has the same characteristics
   as IHEADER.  Return the index of this section or SHN_UNDEF if
   none can be found.  Check's section HINT first, as this is likely
   to be the correct section.  */

static unsigned int
find_link (const bfd *obfd, const Elf_Internal_Shdr *iheader,
	   const unsigned int hint)
{
  Elf_Internal_Shdr ** oheaders = elf_elfsections (obfd);
  unsigned int i;

  BFD_ASSERT (iheader != NULL);

  /* See PR 20922 for a reproducer of the NULL test.  */
  if (hint < elf_numsections (obfd)
      && oheaders[hint] != NULL
      && section_match (oheaders[hint], iheader))
    return hint;

  for (i = 1; i < elf_numsections (obfd); i++)
    {
      Elf_Internal_Shdr * oheader = oheaders[i];

      if (oheader == NULL)
	continue;
      if (section_match (oheader, iheader))
	/* FIXME: Do we care if there is a potential for
	   multiple matches ?  */
	return i;
    }

  return SHN_UNDEF;
}

/* PR 19938: Attempt to set the ELF section header fields of an OS or
   Processor specific section, based upon a matching input section.
   Returns TRUE upon success, FALSE otherwise.  */

static bool
copy_special_section_fields (const bfd *ibfd,
			     bfd *obfd,
			     const Elf_Internal_Shdr *iheader,
			     Elf_Internal_Shdr *oheader,
			     const unsigned int secnum)
{
  const struct elf_backend_data *bed = get_elf_backend_data (obfd);
  const Elf_Internal_Shdr **iheaders
    = (const Elf_Internal_Shdr **) elf_elfsections (ibfd);
  bool changed = false;
  unsigned int sh_link;

  if (oheader->sh_type == SHT_NOBITS)
    {
      /* This is a feature for objcopy --only-keep-debug:
	 When a section's type is changed to NOBITS, we preserve
	 the sh_link and sh_info fields so that they can be
	 matched up with the original.

	 Note: Strictly speaking these assignments are wrong.
	 The sh_link and sh_info fields should point to the
	 relevent sections in the output BFD, which may not be in
	 the same location as they were in the input BFD.  But
	 the whole point of this action is to preserve the
	 original values of the sh_link and sh_info fields, so
	 that they can be matched up with the section headers in
	 the original file.  So strictly speaking we may be
	 creating an invalid ELF file, but it is only for a file
	 that just contains debug info and only for sections
	 without any contents.  */
      if (oheader->sh_link == 0)
	oheader->sh_link = iheader->sh_link;
      if (oheader->sh_info == 0)
	oheader->sh_info = iheader->sh_info;
      return true;
    }

  /* Allow the target a chance to decide how these fields should be set.  */
  if (bed->elf_backend_copy_special_section_fields (ibfd, obfd,
						    iheader, oheader))
    return true;

  /* We have an iheader which might match oheader, and which has non-zero
     sh_info and/or sh_link fields.  Attempt to follow those links and find
     the section in the output bfd which corresponds to the linked section
     in the input bfd.  */
  if (iheader->sh_link != SHN_UNDEF)
    {
      /* See PR 20931 for a reproducer.  */
      if (iheader->sh_link >= elf_numsections (ibfd))
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: invalid sh_link field (%d) in section number %d"),
	     ibfd, iheader->sh_link, secnum);
	  return false;
	}

      sh_link = find_link (obfd, iheaders[iheader->sh_link], iheader->sh_link);
      if (sh_link != SHN_UNDEF)
	{
	  oheader->sh_link = sh_link;
	  changed = true;
	}
      else
	/* FIXME: Should we install iheader->sh_link
	   if we could not find a match ?  */
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB: failed to find link section for section %d"), obfd, secnum);
    }

  if (iheader->sh_info)
    {
      /* The sh_info field can hold arbitrary information, but if the
	 SHF_LINK_INFO flag is set then it should be interpreted as a
	 section index.  */
      if (iheader->sh_flags & SHF_INFO_LINK)
	{
	  sh_link = find_link (obfd, iheaders[iheader->sh_info],
			       iheader->sh_info);
	  if (sh_link != SHN_UNDEF)
	    oheader->sh_flags |= SHF_INFO_LINK;
	}
      else
	/* No idea what it means - just copy it.  */
	sh_link = iheader->sh_info;

      if (sh_link != SHN_UNDEF)
	{
	  oheader->sh_info = sh_link;
	  changed = true;
	}
      else
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB: failed to find info section for section %d"), obfd, secnum);
    }

  return changed;
}

/* Copy the program header and other data from one object module to
   another.  */

bool
_bfd_elf_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  const Elf_Internal_Shdr **iheaders
    = (const Elf_Internal_Shdr **) elf_elfsections (ibfd);
  Elf_Internal_Shdr **oheaders = elf_elfsections (obfd);
  const struct elf_backend_data *bed;
  unsigned int i;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
    || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  if (!elf_flags_init (obfd))
    {
      elf_elfheader (obfd)->e_flags = elf_elfheader (ibfd)->e_flags;
      elf_flags_init (obfd) = true;
    }

  elf_gp (obfd) = elf_gp (ibfd);

  /* Also copy the EI_OSABI field.  */
  elf_elfheader (obfd)->e_ident[EI_OSABI] =
    elf_elfheader (ibfd)->e_ident[EI_OSABI];

  /* If set, copy the EI_ABIVERSION field.  */
  if (elf_elfheader (ibfd)->e_ident[EI_ABIVERSION])
    elf_elfheader (obfd)->e_ident[EI_ABIVERSION]
      = elf_elfheader (ibfd)->e_ident[EI_ABIVERSION];

  /* Copy object attributes.  */
  _bfd_elf_copy_obj_attributes (ibfd, obfd);

  if (iheaders == NULL || oheaders == NULL)
    return true;

  bed = get_elf_backend_data (obfd);

  /* Possibly copy other fields in the section header.  */
  for (i = 1; i < elf_numsections (obfd); i++)
    {
      unsigned int j;
      Elf_Internal_Shdr * oheader = oheaders[i];

      /* Ignore ordinary sections.  SHT_NOBITS sections are considered however
	 because of a special case need for generating separate debug info
	 files.  See below for more details.  */
      if (oheader == NULL
	  || (oheader->sh_type != SHT_NOBITS
	      && oheader->sh_type < SHT_LOOS))
	continue;

      /* Ignore empty sections, and sections whose
	 fields have already been initialised.  */
      if (oheader->sh_size == 0
	  || (oheader->sh_info != 0 && oheader->sh_link != 0))
	continue;

      /* Scan for the matching section in the input bfd.
	 First we try for a direct mapping between the input and
	 output sections.  */
      for (j = 1; j < elf_numsections (ibfd); j++)
	{
	  const Elf_Internal_Shdr * iheader = iheaders[j];

	  if (iheader == NULL)
	    continue;

	  if (oheader->bfd_section != NULL
	      && iheader->bfd_section != NULL
	      && iheader->bfd_section->output_section != NULL
	      && iheader->bfd_section->output_section == oheader->bfd_section)
	    {
	      /* We have found a connection from the input section to
		 the output section.  Attempt to copy the header fields.
		 If this fails then do not try any further sections -
		 there should only be a one-to-one mapping between
		 input and output.  */
	      if (!copy_special_section_fields (ibfd, obfd,
						iheader, oheader, i))
		j = elf_numsections (ibfd);
	      break;
	    }
	}

      if (j < elf_numsections (ibfd))
	continue;

      /* That failed.  So try to deduce the corresponding input section.
	 Unfortunately we cannot compare names as the output string table
	 is empty, so instead we check size, address and type.  */
      for (j = 1; j < elf_numsections (ibfd); j++)
	{
	  const Elf_Internal_Shdr * iheader = iheaders[j];

	  if (iheader == NULL)
	    continue;

	  /* Try matching fields in the input section's header.
	     Since --only-keep-debug turns all non-debug sections into
	     SHT_NOBITS sections, the output SHT_NOBITS type matches any
	     input type.  */
	  if ((oheader->sh_type == SHT_NOBITS
	       || iheader->sh_type == oheader->sh_type)
	      && (iheader->sh_flags & ~ SHF_INFO_LINK)
	      == (oheader->sh_flags & ~ SHF_INFO_LINK)
	      && iheader->sh_addralign == oheader->sh_addralign
	      && iheader->sh_entsize == oheader->sh_entsize
	      && iheader->sh_size == oheader->sh_size
	      && iheader->sh_addr == oheader->sh_addr
	      && (iheader->sh_info != oheader->sh_info
		  || iheader->sh_link != oheader->sh_link))
	    {
	      if (copy_special_section_fields (ibfd, obfd, iheader, oheader, i))
		break;
	    }
	}

      if (j == elf_numsections (ibfd) && oheader->sh_type >= SHT_LOOS)
	{
	  /* Final attempt.  Call the backend copy function
	     with a NULL input section.  */
	  (void) bed->elf_backend_copy_special_section_fields (ibfd, obfd,
							       NULL, oheader);
	}
    }

  return true;
}

static const char *
get_segment_type (unsigned int p_type)
{
  const char *pt;
  switch (p_type)
    {
    case PT_NULL: pt = "NULL"; break;
    case PT_LOAD: pt = "LOAD"; break;
    case PT_DYNAMIC: pt = "DYNAMIC"; break;
    case PT_INTERP: pt = "INTERP"; break;
    case PT_NOTE: pt = "NOTE"; break;
    case PT_SHLIB: pt = "SHLIB"; break;
    case PT_PHDR: pt = "PHDR"; break;
    case PT_TLS: pt = "TLS"; break;
    case PT_GNU_EH_FRAME: pt = "EH_FRAME"; break;
    case PT_GNU_STACK: pt = "STACK"; break;
    case PT_GNU_RELRO: pt = "RELRO"; break;
    case PT_GNU_SFRAME: pt = "SFRAME"; break;
    default: pt = NULL; break;
    }
  return pt;
}

/* Print out the program headers.  */

bool
_bfd_elf_print_private_bfd_data (bfd *abfd, void *farg)
{
  FILE *f = (FILE *) farg;
  Elf_Internal_Phdr *p;
  asection *s;
  bfd_byte *dynbuf = NULL;

  p = elf_tdata (abfd)->phdr;
  if (p != NULL)
    {
      unsigned int i, c;

      fprintf (f, _("\nProgram Header:\n"));
      c = elf_elfheader (abfd)->e_phnum;
      for (i = 0; i < c; i++, p++)
	{
	  const char *pt = get_segment_type (p->p_type);
	  char buf[20];

	  if (pt == NULL)
	    {
	      sprintf (buf, "0x%lx", p->p_type);
	      pt = buf;
	    }
	  fprintf (f, "%8s off    0x", pt);
	  bfd_fprintf_vma (abfd, f, p->p_offset);
	  fprintf (f, " vaddr 0x");
	  bfd_fprintf_vma (abfd, f, p->p_vaddr);
	  fprintf (f, " paddr 0x");
	  bfd_fprintf_vma (abfd, f, p->p_paddr);
	  fprintf (f, " align 2**%u\n", bfd_log2 (p->p_align));
	  fprintf (f, "         filesz 0x");
	  bfd_fprintf_vma (abfd, f, p->p_filesz);
	  fprintf (f, " memsz 0x");
	  bfd_fprintf_vma (abfd, f, p->p_memsz);
	  fprintf (f, " flags %c%c%c",
		   (p->p_flags & PF_R) != 0 ? 'r' : '-',
		   (p->p_flags & PF_W) != 0 ? 'w' : '-',
		   (p->p_flags & PF_X) != 0 ? 'x' : '-');
	  if ((p->p_flags &~ (unsigned) (PF_R | PF_W | PF_X)) != 0)
	    fprintf (f, " %lx", p->p_flags &~ (unsigned) (PF_R | PF_W | PF_X));
	  fprintf (f, "\n");
	}
    }

  s = bfd_get_section_by_name (abfd, ".dynamic");
  if (s != NULL && (s->flags & SEC_HAS_CONTENTS) != 0)
    {
      unsigned int elfsec;
      unsigned long shlink;
      bfd_byte *extdyn, *extdynend;
      size_t extdynsize;
      void (*swap_dyn_in) (bfd *, const void *, Elf_Internal_Dyn *);

      fprintf (f, _("\nDynamic Section:\n"));

      if (!bfd_malloc_and_get_section (abfd, s, &dynbuf))
	goto error_return;

      elfsec = _bfd_elf_section_from_bfd_section (abfd, s);
      if (elfsec == SHN_BAD)
	goto error_return;
      shlink = elf_elfsections (abfd)[elfsec]->sh_link;

      extdynsize = get_elf_backend_data (abfd)->s->sizeof_dyn;
      swap_dyn_in = get_elf_backend_data (abfd)->s->swap_dyn_in;

      for (extdyn = dynbuf, extdynend = dynbuf + s->size;
	   (size_t) (extdynend - extdyn) >= extdynsize;
	   extdyn += extdynsize)
	{
	  Elf_Internal_Dyn dyn;
	  const char *name = "";
	  char ab[20];
	  bool stringp;
	  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

	  (*swap_dyn_in) (abfd, extdyn, &dyn);

	  if (dyn.d_tag == DT_NULL)
	    break;

	  stringp = false;
	  switch (dyn.d_tag)
	    {
	    default:
	      if (bed->elf_backend_get_target_dtag)
		name = (*bed->elf_backend_get_target_dtag) (dyn.d_tag);

	      if (!strcmp (name, ""))
		{
		  sprintf (ab, "%#" PRIx64, (uint64_t) dyn.d_tag);
		  name = ab;
		}
	      break;

	    case DT_NEEDED: name = "NEEDED"; stringp = true; break;
	    case DT_PLTRELSZ: name = "PLTRELSZ"; break;
	    case DT_PLTGOT: name = "PLTGOT"; break;
	    case DT_HASH: name = "HASH"; break;
	    case DT_STRTAB: name = "STRTAB"; break;
	    case DT_SYMTAB: name = "SYMTAB"; break;
	    case DT_RELA: name = "RELA"; break;
	    case DT_RELASZ: name = "RELASZ"; break;
	    case DT_RELAENT: name = "RELAENT"; break;
	    case DT_STRSZ: name = "STRSZ"; break;
	    case DT_SYMENT: name = "SYMENT"; break;
	    case DT_INIT: name = "INIT"; break;
	    case DT_FINI: name = "FINI"; break;
	    case DT_SONAME: name = "SONAME"; stringp = true; break;
	    case DT_RPATH: name = "RPATH"; stringp = true; break;
	    case DT_SYMBOLIC: name = "SYMBOLIC"; break;
	    case DT_REL: name = "REL"; break;
	    case DT_RELSZ: name = "RELSZ"; break;
	    case DT_RELENT: name = "RELENT"; break;
	    case DT_RELR: name = "RELR"; break;
	    case DT_RELRSZ: name = "RELRSZ"; break;
	    case DT_RELRENT: name = "RELRENT"; break;
	    case DT_PLTREL: name = "PLTREL"; break;
	    case DT_DEBUG: name = "DEBUG"; break;
	    case DT_TEXTREL: name = "TEXTREL"; break;
	    case DT_JMPREL: name = "JMPREL"; break;
	    case DT_BIND_NOW: name = "BIND_NOW"; break;
	    case DT_INIT_ARRAY: name = "INIT_ARRAY"; break;
	    case DT_FINI_ARRAY: name = "FINI_ARRAY"; break;
	    case DT_INIT_ARRAYSZ: name = "INIT_ARRAYSZ"; break;
	    case DT_FINI_ARRAYSZ: name = "FINI_ARRAYSZ"; break;
	    case DT_RUNPATH: name = "RUNPATH"; stringp = true; break;
	    case DT_FLAGS: name = "FLAGS"; break;
	    case DT_PREINIT_ARRAY: name = "PREINIT_ARRAY"; break;
	    case DT_PREINIT_ARRAYSZ: name = "PREINIT_ARRAYSZ"; break;
	    case DT_CHECKSUM: name = "CHECKSUM"; break;
	    case DT_PLTPADSZ: name = "PLTPADSZ"; break;
	    case DT_MOVEENT: name = "MOVEENT"; break;
	    case DT_MOVESZ: name = "MOVESZ"; break;
	    case DT_FEATURE: name = "FEATURE"; break;
	    case DT_POSFLAG_1: name = "POSFLAG_1"; break;
	    case DT_SYMINSZ: name = "SYMINSZ"; break;
	    case DT_SYMINENT: name = "SYMINENT"; break;
	    case DT_CONFIG: name = "CONFIG"; stringp = true; break;
	    case DT_DEPAUDIT: name = "DEPAUDIT"; stringp = true; break;
	    case DT_AUDIT: name = "AUDIT"; stringp = true; break;
	    case DT_PLTPAD: name = "PLTPAD"; break;
	    case DT_MOVETAB: name = "MOVETAB"; break;
	    case DT_SYMINFO: name = "SYMINFO"; break;
	    case DT_RELACOUNT: name = "RELACOUNT"; break;
	    case DT_RELCOUNT: name = "RELCOUNT"; break;
	    case DT_FLAGS_1: name = "FLAGS_1"; break;
	    case DT_VERSYM: name = "VERSYM"; break;
	    case DT_VERDEF: name = "VERDEF"; break;
	    case DT_VERDEFNUM: name = "VERDEFNUM"; break;
	    case DT_VERNEED: name = "VERNEED"; break;
	    case DT_VERNEEDNUM: name = "VERNEEDNUM"; break;
	    case DT_AUXILIARY: name = "AUXILIARY"; stringp = true; break;
	    case DT_USED: name = "USED"; break;
	    case DT_FILTER: name = "FILTER"; stringp = true; break;
	    case DT_GNU_HASH: name = "GNU_HASH"; break;
	    }

	  fprintf (f, "  %-20s ", name);
	  if (! stringp)
	    {
	      fprintf (f, "0x");
	      bfd_fprintf_vma (abfd, f, dyn.d_un.d_val);
	    }
	  else
	    {
	      const char *string;
	      unsigned int tagv = dyn.d_un.d_val;

	      string = bfd_elf_string_from_elf_section (abfd, shlink, tagv);
	      if (string == NULL)
		goto error_return;
	      fprintf (f, "%s", string);
	    }
	  fprintf (f, "\n");
	}

      free (dynbuf);
      dynbuf = NULL;
    }

  if ((elf_dynverdef (abfd) != 0 && elf_tdata (abfd)->verdef == NULL)
      || (elf_dynverref (abfd) != 0 && elf_tdata (abfd)->verref == NULL))
    {
      if (! _bfd_elf_slurp_version_tables (abfd, false))
	return false;
    }

  if (elf_dynverdef (abfd) != 0)
    {
      Elf_Internal_Verdef *t;

      fprintf (f, _("\nVersion definitions:\n"));
      for (t = elf_tdata (abfd)->verdef; t != NULL; t = t->vd_nextdef)
	{
	  fprintf (f, "%d 0x%2.2x 0x%8.8lx %s\n", t->vd_ndx,
		   t->vd_flags, t->vd_hash,
		   t->vd_nodename ? t->vd_nodename : "<corrupt>");
	  if (t->vd_auxptr != NULL && t->vd_auxptr->vda_nextptr != NULL)
	    {
	      Elf_Internal_Verdaux *a;

	      fprintf (f, "\t");
	      for (a = t->vd_auxptr->vda_nextptr;
		   a != NULL;
		   a = a->vda_nextptr)
		fprintf (f, "%s ",
			 a->vda_nodename ? a->vda_nodename : "<corrupt>");
	      fprintf (f, "\n");
	    }
	}
    }

  if (elf_dynverref (abfd) != 0)
    {
      Elf_Internal_Verneed *t;

      fprintf (f, _("\nVersion References:\n"));
      for (t = elf_tdata (abfd)->verref; t != NULL; t = t->vn_nextref)
	{
	  Elf_Internal_Vernaux *a;

	  fprintf (f, _("  required from %s:\n"),
		   t->vn_filename ? t->vn_filename : "<corrupt>");
	  for (a = t->vn_auxptr; a != NULL; a = a->vna_nextptr)
	    fprintf (f, "    0x%8.8lx 0x%2.2x %2.2d %s\n", a->vna_hash,
		     a->vna_flags, a->vna_other,
		     a->vna_nodename ? a->vna_nodename : "<corrupt>");
	}
    }

  return true;

 error_return:
  free (dynbuf);
  return false;
}

/* Find the file offset corresponding to VMA by using the program
   headers.  */

static file_ptr
offset_from_vma (Elf_Internal_Phdr *phdrs, size_t phnum, bfd_vma vma,
		 size_t size, size_t *max_size_p)
{
  Elf_Internal_Phdr *seg;
  size_t i;

  for (seg = phdrs, i = 0; i < phnum; ++seg, ++i)
    if (seg->p_type == PT_LOAD
	&& vma >= (seg->p_vaddr & -seg->p_align)
	&& vma + size <= seg->p_vaddr + seg->p_filesz)
      {
	if (max_size_p)
	  *max_size_p = seg->p_vaddr + seg->p_filesz - vma;
	return vma - seg->p_vaddr + seg->p_offset;
      }

  if (max_size_p)
    *max_size_p = 0;
  bfd_set_error (bfd_error_invalid_operation);
  return (file_ptr) -1;
}

/* Convert hash table to internal form.  */

static bfd_vma *
get_hash_table_data (bfd *abfd, bfd_size_type number,
		     unsigned int ent_size, bfd_size_type filesize)
{
  unsigned char *e_data = NULL;
  bfd_vma *i_data = NULL;
  bfd_size_type size;

  if (ent_size != 4 && ent_size != 8)
    return NULL;

  if ((size_t) number != number)
    {
      bfd_set_error (bfd_error_file_too_big);
      return NULL;
    }

  size = ent_size * number;
  /* Be kind to memory checkers (eg valgrind, address sanitizer) by not
     attempting to allocate memory when the read is bound to fail.  */
  if (size > filesize
      || number >= ~(size_t) 0 / ent_size
      || number >= ~(size_t) 0 / sizeof (*i_data))
    {
      bfd_set_error (bfd_error_file_too_big);
      return NULL;
    }

  e_data = _bfd_malloc_and_read (abfd, size, size);
  if (e_data == NULL)
    return NULL;

  i_data = (bfd_vma *) bfd_malloc (number * sizeof (*i_data));
  if (i_data == NULL)
    {
      free (e_data);
      return NULL;
    }

  if (ent_size == 4)
    while (number--)
      i_data[number] = bfd_get_32 (abfd, e_data + number * ent_size);
  else
    while (number--)
      i_data[number] = bfd_get_64 (abfd, e_data + number * ent_size);

  free (e_data);
  return i_data;
}

/* Address of .MIPS.xhash section.  FIXME: What is the best way to
   support DT_MIPS_XHASH?  */
#define DT_MIPS_XHASH	       0x70000036

/* Reconstruct dynamic symbol table from PT_DYNAMIC segment.  */

bool
_bfd_elf_get_dynamic_symbols (bfd *abfd, Elf_Internal_Phdr *phdr,
			      Elf_Internal_Phdr *phdrs, size_t phnum,
			      bfd_size_type filesize)
{
  bfd_byte *extdyn, *extdynend;
  size_t extdynsize;
  void (*swap_dyn_in) (bfd *, const void *, Elf_Internal_Dyn *);
  bool (*swap_symbol_in) (bfd *, const void *, const void *,
			  Elf_Internal_Sym *);
  Elf_Internal_Dyn dyn;
  bfd_vma dt_hash = 0;
  bfd_vma dt_gnu_hash = 0;
  bfd_vma dt_mips_xhash = 0;
  bfd_vma dt_strtab = 0;
  bfd_vma dt_symtab = 0;
  size_t dt_strsz = 0;
  bfd_vma dt_versym = 0;
  bfd_vma dt_verdef = 0;
  bfd_vma dt_verneed = 0;
  bfd_byte *dynbuf = NULL;
  char *strbuf = NULL;
  bfd_vma *gnubuckets = NULL;
  bfd_vma *gnuchains = NULL;
  bfd_vma *mipsxlat = NULL;
  file_ptr saved_filepos, filepos;
  bool res = false;
  size_t amt;
  bfd_byte *esymbuf = NULL, *esym;
  bfd_size_type symcount;
  Elf_Internal_Sym *isymbuf = NULL;
  Elf_Internal_Sym *isym, *isymend;
  bfd_byte *versym = NULL;
  bfd_byte *verdef = NULL;
  bfd_byte *verneed = NULL;
  size_t verdef_size = 0;
  size_t verneed_size = 0;
  size_t extsym_size;
  const struct elf_backend_data *bed;

  /* Return TRUE if symbol table is bad.  */
  if (elf_bad_symtab (abfd))
    return true;

  /* Return TRUE if DT_HASH/DT_GNU_HASH have bee processed before.  */
  if (elf_tdata (abfd)->dt_strtab != NULL)
    return true;

  bed = get_elf_backend_data (abfd);

  /* Save file position for elf_object_p.  */
  saved_filepos = bfd_tell (abfd);

  if (bfd_seek (abfd, phdr->p_offset, SEEK_SET) != 0)
    goto error_return;

  dynbuf = _bfd_malloc_and_read (abfd, phdr->p_filesz, phdr->p_filesz);
  if (dynbuf == NULL)
    goto error_return;

  extsym_size = bed->s->sizeof_sym;
  extdynsize = bed->s->sizeof_dyn;
  swap_dyn_in = bed->s->swap_dyn_in;

  extdyn = dynbuf;
  if (phdr->p_filesz < extdynsize)
    goto error_return;
  extdynend = extdyn + phdr->p_filesz;
  for (; extdyn <= (extdynend - extdynsize); extdyn += extdynsize)
    {
      swap_dyn_in (abfd, extdyn, &dyn);

      if (dyn.d_tag == DT_NULL)
	break;

      switch (dyn.d_tag)
	{
	case DT_HASH:
	  dt_hash = dyn.d_un.d_val;
	  break;
	case DT_GNU_HASH:
	  if (bed->elf_machine_code != EM_MIPS
	      && bed->elf_machine_code != EM_MIPS_RS3_LE)
	    dt_gnu_hash = dyn.d_un.d_val;
	  break;
	case DT_STRTAB:
	  dt_strtab = dyn.d_un.d_val;
	  break;
	case DT_SYMTAB:
	  dt_symtab = dyn.d_un.d_val;
	  break;
	case DT_STRSZ:
	  dt_strsz = dyn.d_un.d_val;
	  break;
	case DT_SYMENT:
	  if (dyn.d_un.d_val != extsym_size)
	    goto error_return;
	  break;
	case DT_VERSYM:
	  dt_versym = dyn.d_un.d_val;
	  break;
	case DT_VERDEF:
	  dt_verdef = dyn.d_un.d_val;
	  break;
	case DT_VERNEED:
	  dt_verneed = dyn.d_un.d_val;
	  break;
	default:
	  if (dyn.d_tag == DT_MIPS_XHASH
	      && (bed->elf_machine_code == EM_MIPS
		  || bed->elf_machine_code == EM_MIPS_RS3_LE))
	    {
	      dt_gnu_hash = dyn.d_un.d_val;
	      dt_mips_xhash = dyn.d_un.d_val;
	    }
	  break;
	}
    }

  /* Check if we can reconstruct dynamic symbol table from PT_DYNAMIC
     segment.  */
  if ((!dt_hash && !dt_gnu_hash)
      || !dt_strtab
      || !dt_symtab
      || !dt_strsz)
    goto error_return;

  /* Get dynamic string table.  */
  filepos = offset_from_vma (phdrs, phnum, dt_strtab, dt_strsz, NULL);
  if (filepos == (file_ptr) -1
      || bfd_seek (abfd, filepos, SEEK_SET) != 0)
    goto error_return;

  /* Dynamic string table must be valid until ABFD is closed.  */
  strbuf = (char *) _bfd_alloc_and_read (abfd, dt_strsz, dt_strsz);
  if (strbuf == NULL)
    goto error_return;

  /* Get the real symbol count from DT_HASH or DT_GNU_HASH.  Prefer
     DT_HASH since it is simpler than DT_GNU_HASH.  */
  if (dt_hash)
    {
      unsigned char nb[16];
      unsigned int hash_ent_size;

      switch (bed->elf_machine_code)
	{
	case EM_ALPHA:
	case EM_S390:
	case EM_S390_OLD:
	  if (bed->s->elfclass == ELFCLASS64)
	    {
	      hash_ent_size = 8;
	      break;
	    }
	  /* FALLTHROUGH */
	default:
	  hash_ent_size = 4;
	  break;
	}

      filepos = offset_from_vma (phdrs, phnum, dt_hash, sizeof (nb),
				 NULL);
      if (filepos == (file_ptr) -1
	  || bfd_seek (abfd, filepos, SEEK_SET) != 0
	  || (bfd_bread (nb, 2 * hash_ent_size, abfd)
	      != (2 * hash_ent_size)))
	goto error_return;

      /* The number of dynamic symbol table entries equals the number
	 of chains.  */
      if (hash_ent_size == 8)
	symcount = bfd_get_64 (abfd, nb + hash_ent_size);
      else
	symcount = bfd_get_32 (abfd, nb + hash_ent_size);
    }
  else
    {
      /* For DT_GNU_HASH, only defined symbols with non-STB_LOCAL
	 bindings are in hash table.  Since in dynamic symbol table,
	 all symbols with STB_LOCAL binding are placed before symbols
	 with other bindings and all undefined symbols are placed
	 before defined ones, the highest symbol index in DT_GNU_HASH
	 is the highest dynamic symbol table index.  */
      unsigned char nb[16];
      bfd_vma ngnubuckets;
      bfd_vma gnusymidx;
      size_t i, ngnuchains;
      bfd_vma maxchain = 0xffffffff, bitmaskwords;
      bfd_vma buckets_vma;

      filepos = offset_from_vma (phdrs, phnum, dt_gnu_hash,
				 sizeof (nb), NULL);
      if (filepos == (file_ptr) -1
	  || bfd_seek (abfd, filepos, SEEK_SET) != 0
	  || bfd_bread (nb, sizeof (nb), abfd) != sizeof (nb))
	goto error_return;

      ngnubuckets = bfd_get_32 (abfd, nb);
      gnusymidx = bfd_get_32 (abfd, nb + 4);
      bitmaskwords = bfd_get_32 (abfd, nb + 8);
      buckets_vma = dt_gnu_hash + 16;
      if (bed->s->elfclass == ELFCLASS32)
	buckets_vma += bitmaskwords * 4;
      else
	buckets_vma += bitmaskwords * 8;
      filepos = offset_from_vma (phdrs, phnum, buckets_vma, 4, NULL);
      if (filepos == (file_ptr) -1
	  || bfd_seek (abfd, filepos, SEEK_SET) != 0)
	goto error_return;

      gnubuckets = get_hash_table_data (abfd, ngnubuckets, 4, filesize);
      if (gnubuckets == NULL)
	goto error_return;

      for (i = 0; i < ngnubuckets; i++)
	if (gnubuckets[i] != 0)
	  {
	    if (gnubuckets[i] < gnusymidx)
	      goto error_return;

	    if (maxchain == 0xffffffff || gnubuckets[i] > maxchain)
	      maxchain = gnubuckets[i];
	  }

      if (maxchain == 0xffffffff)
	{
	  symcount = 0;
	  goto empty_gnu_hash;
	}

      maxchain -= gnusymidx;
      filepos = offset_from_vma (phdrs, phnum,
				 (buckets_vma +
				  4 * (ngnubuckets + maxchain)),
				 4, NULL);
      if (filepos == (file_ptr) -1
	  || bfd_seek (abfd, filepos, SEEK_SET) != 0)
	goto error_return;

      do
	{
	  if (bfd_bread (nb, 4, abfd) != 4)
	    goto error_return;
	  ++maxchain;
	  if (maxchain == 0)
	    goto error_return;
	}
      while ((bfd_get_32 (abfd, nb) & 1) == 0);

      filepos = offset_from_vma (phdrs, phnum,
				 (buckets_vma + 4 * ngnubuckets),
				 4, NULL);
      if (filepos == (file_ptr) -1
	  || bfd_seek (abfd, filepos, SEEK_SET) != 0)
	goto error_return;

      gnuchains = get_hash_table_data (abfd, maxchain, 4, filesize);
      if (gnubuckets == NULL)
	goto error_return;
      ngnuchains = maxchain;

      if (dt_mips_xhash)
	{
	  filepos = offset_from_vma (phdrs, phnum,
				     (buckets_vma
				      + 4 * (ngnubuckets + maxchain)),
				     4, NULL);
	  if (filepos == (file_ptr) -1
	      || bfd_seek (abfd, filepos, SEEK_SET) != 0)
	    goto error_return;

	  mipsxlat = get_hash_table_data (abfd, maxchain, 4, filesize);
	  if (mipsxlat == NULL)
	    goto error_return;
	}

      symcount = 0;
      for (i = 0; i < ngnubuckets; ++i)
	if (gnubuckets[i] != 0)
	  {
	    bfd_vma si = gnubuckets[i];
	    bfd_vma off = si - gnusymidx;
	    do
	      {
		if (mipsxlat)
		  {
		    if (mipsxlat[off] >= symcount)
		      symcount = mipsxlat[off] + 1;
		  }
		else
		  {
		    if (si >= symcount)
		      symcount = si + 1;
		  }
		si++;
	      }
	    while (off < ngnuchains && (gnuchains[off++] & 1) == 0);
	  }
    }

  /* Swap in dynamic symbol table.  */
  if (_bfd_mul_overflow (symcount, extsym_size, &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      goto error_return;
    }

  filepos = offset_from_vma (phdrs, phnum, dt_symtab, amt, NULL);
  if (filepos == (file_ptr) -1
      || bfd_seek (abfd, filepos, SEEK_SET) != 0)
    goto error_return;
  esymbuf = _bfd_malloc_and_read (abfd, amt, amt);
  if (esymbuf == NULL)
    goto error_return;

  if (_bfd_mul_overflow (symcount, sizeof (Elf_Internal_Sym), &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      goto error_return;
    }

  /* Dynamic symbol table must be valid until ABFD is closed.  */
  isymbuf = (Elf_Internal_Sym *) bfd_alloc (abfd, amt);
  if (isymbuf == NULL)
    goto error_return;

  swap_symbol_in = bed->s->swap_symbol_in;

  /* Convert the symbols to internal form.  */
  isymend = isymbuf + symcount;
  for (esym = esymbuf, isym = isymbuf;
       isym < isymend;
       esym += extsym_size, isym++)
    if (!swap_symbol_in (abfd, esym, NULL, isym)
	|| isym->st_name >= dt_strsz)
      {
	bfd_set_error (bfd_error_invalid_operation);
	goto error_return;
      }

  if (dt_versym)
    {
      /* Swap in DT_VERSYM.  */
      if (_bfd_mul_overflow (symcount, 2, &amt))
	{
	  bfd_set_error (bfd_error_file_too_big);
	  goto error_return;
	}

      filepos = offset_from_vma (phdrs, phnum, dt_versym, amt, NULL);
      if (filepos == (file_ptr) -1
	  || bfd_seek (abfd, filepos, SEEK_SET) != 0)
	goto error_return;

      /* DT_VERSYM info must be valid until ABFD is closed.  */
      versym = _bfd_alloc_and_read (abfd, amt, amt);

      if (dt_verdef)
	{
	  /* Read in DT_VERDEF.  */
	  filepos = offset_from_vma (phdrs, phnum, dt_verdef,
				     0, &verdef_size);
	  if (filepos == (file_ptr) -1
	      || bfd_seek (abfd, filepos, SEEK_SET) != 0)
	    goto error_return;

	  /* DT_VERDEF info must be valid until ABFD is closed.  */
	  verdef = _bfd_alloc_and_read (abfd, verdef_size,
					verdef_size);
	}

      if (dt_verneed)
	{
	  /* Read in DT_VERNEED.  */
	  filepos = offset_from_vma (phdrs, phnum, dt_verneed,
				     0, &verneed_size);
	  if (filepos == (file_ptr) -1
	      || bfd_seek (abfd, filepos, SEEK_SET) != 0)
	    goto error_return;

	  /* DT_VERNEED info must be valid until ABFD is closed.  */
	  verneed = _bfd_alloc_and_read (abfd, verneed_size,
					 verneed_size);
	}
    }

 empty_gnu_hash:
  elf_tdata (abfd)->dt_strtab = strbuf;
  elf_tdata (abfd)->dt_symtab = isymbuf;
  elf_tdata (abfd)->dt_symtab_count = symcount;
  elf_tdata (abfd)->dt_versym = versym;
  elf_tdata (abfd)->dt_verdef = verdef;
  elf_tdata (abfd)->dt_verneed = verneed;
  elf_tdata (abfd)->dt_verdef_count
    = verdef_size / sizeof (Elf_External_Verdef);
  elf_tdata (abfd)->dt_verneed_count
    = verneed_size / sizeof (Elf_External_Verneed);

  res = true;

 error_return:
  /* Restore file position for elf_object_p.  */
  if (bfd_seek (abfd, saved_filepos, SEEK_SET) != 0)
    res = false;
  free (dynbuf);
  free (esymbuf);
  free (gnubuckets);
  free (gnuchains);
  free (mipsxlat);
  return res;
}

/* Reconstruct section from dynamic symbol.  */

asection *
_bfd_elf_get_section_from_dynamic_symbol (bfd *abfd,
					  Elf_Internal_Sym *isym)
{
  asection *sec;
  flagword flags;

  if (!elf_use_dt_symtab_p (abfd))
    return NULL;

  flags = SEC_ALLOC | SEC_LOAD;
  switch (ELF_ST_TYPE (isym->st_info))
    {
    case STT_FUNC:
    case STT_GNU_IFUNC:
      sec = bfd_get_section_by_name (abfd, ".text");
      if (sec == NULL)
	sec = bfd_make_section_with_flags (abfd,
					   ".text",
					   flags | SEC_CODE);
      break;
    case STT_COMMON:
      sec = bfd_com_section_ptr;
      break;
    case STT_OBJECT:
      sec = bfd_get_section_by_name (abfd, ".data");
      if (sec == NULL)
	sec = bfd_make_section_with_flags (abfd,
					   ".data",
					   flags | SEC_DATA);
      break;
    case STT_TLS:
      sec = bfd_get_section_by_name (abfd, ".tdata");
      if (sec == NULL)
	sec = bfd_make_section_with_flags (abfd,
					   ".tdata",
					   (flags
					    | SEC_DATA
					    | SEC_THREAD_LOCAL));
      break;
    default:
      sec = bfd_abs_section_ptr;
      break;
    }

  return sec;
}

/* Get version name.  If BASE_P is TRUE, return "Base" for VER_FLG_BASE
   and return symbol version for symbol version itself.   */

const char *
_bfd_elf_get_symbol_version_string (bfd *abfd, asymbol *symbol,
				    bool base_p,
				    bool *hidden)
{
  const char *version_string = NULL;
  if ((elf_dynversym (abfd) != 0
       && (elf_dynverdef (abfd) != 0 || elf_dynverref (abfd) != 0))
      || (elf_tdata (abfd)->dt_versym != NULL
	  && (elf_tdata (abfd)->dt_verdef != NULL
	      || elf_tdata (abfd)->dt_verneed != NULL)))
    {
      unsigned int vernum = ((elf_symbol_type *) symbol)->version;

      *hidden = (vernum & VERSYM_HIDDEN) != 0;
      vernum &= VERSYM_VERSION;

      if (vernum == 0)
	version_string = "";
      else if (vernum == 1
	       && (vernum > elf_tdata (abfd)->cverdefs
		   || (elf_tdata (abfd)->verdef[0].vd_flags
		       == VER_FLG_BASE)))
	version_string = base_p ? "Base" : "";
      else if (vernum <= elf_tdata (abfd)->cverdefs)
	{
	  const char *nodename
	    = elf_tdata (abfd)->verdef[vernum - 1].vd_nodename;
	  version_string = "";
	  if (base_p
	      || nodename == NULL
	      || symbol->name == NULL
	      || strcmp (symbol->name, nodename) != 0)
	    version_string = nodename;
	}
      else
	{
	  Elf_Internal_Verneed *t;

	  version_string = _("<corrupt>");
	  for (t = elf_tdata (abfd)->verref;
	       t != NULL;
	       t = t->vn_nextref)
	    {
	      Elf_Internal_Vernaux *a;

	      for (a = t->vn_auxptr; a != NULL; a = a->vna_nextptr)
		{
		  if (a->vna_other == vernum)
		    {
		      *hidden = true;
		      version_string = a->vna_nodename;
		      break;
		    }
		}
	    }
	}
    }
  return version_string;
}

/* Display ELF-specific fields of a symbol.  */

void
bfd_elf_print_symbol (bfd *abfd,
		      void *filep,
		      asymbol *symbol,
		      bfd_print_symbol_type how)
{
  FILE *file = (FILE *) filep;
  switch (how)
    {
    case bfd_print_symbol_name:
      fprintf (file, "%s", symbol->name);
      break;
    case bfd_print_symbol_more:
      fprintf (file, "elf ");
      bfd_fprintf_vma (abfd, file, symbol->value);
      fprintf (file, " %x", symbol->flags);
      break;
    case bfd_print_symbol_all:
      {
	const char *section_name;
	const char *name = NULL;
	const struct elf_backend_data *bed;
	unsigned char st_other;
	bfd_vma val;
	const char *version_string;
	bool hidden;

	section_name = symbol->section ? symbol->section->name : "(*none*)";

	bed = get_elf_backend_data (abfd);
	if (bed->elf_backend_print_symbol_all)
	  name = (*bed->elf_backend_print_symbol_all) (abfd, filep, symbol);

	if (name == NULL)
	  {
	    name = symbol->name;
	    bfd_print_symbol_vandf (abfd, file, symbol);
	  }

	fprintf (file, " %s\t", section_name);
	/* Print the "other" value for a symbol.  For common symbols,
	   we've already printed the size; now print the alignment.
	   For other symbols, we have no specified alignment, and
	   we've printed the address; now print the size.  */
	if (symbol->section && bfd_is_com_section (symbol->section))
	  val = ((elf_symbol_type *) symbol)->internal_elf_sym.st_value;
	else
	  val = ((elf_symbol_type *) symbol)->internal_elf_sym.st_size;
	bfd_fprintf_vma (abfd, file, val);

	/* If we have version information, print it.  */
	version_string = _bfd_elf_get_symbol_version_string (abfd,
							     symbol,
							     true,
							     &hidden);
	if (version_string)
	  {
	    if (!hidden)
	      fprintf (file, "  %-11s", version_string);
	    else
	      {
		int i;

		fprintf (file, " (%s)", version_string);
		for (i = 10 - strlen (version_string); i > 0; --i)
		  putc (' ', file);
	      }
	  }

	/* If the st_other field is not zero, print it.  */
	st_other = ((elf_symbol_type *) symbol)->internal_elf_sym.st_other;

	switch (st_other)
	  {
	  case 0: break;
	  case STV_INTERNAL:  fprintf (file, " .internal");  break;
	  case STV_HIDDEN:    fprintf (file, " .hidden");    break;
	  case STV_PROTECTED: fprintf (file, " .protected"); break;
	  default:
	    /* Some other non-defined flags are also present, so print
	       everything hex.  */
	    fprintf (file, " 0x%02x", (unsigned int) st_other);
	  }

	fprintf (file, " %s", name);
      }
      break;
    }
}

/* ELF .o/exec file reading */

/* Create a new bfd section from an ELF section header.  */

bool
bfd_section_from_shdr (bfd *abfd, unsigned int shindex)
{
  Elf_Internal_Shdr *hdr;
  Elf_Internal_Ehdr *ehdr;
  const struct elf_backend_data *bed;
  const char *name;
  bool ret = true;

  if (shindex >= elf_numsections (abfd))
    return false;

  /* PR17512: A corrupt ELF binary might contain a loop of sections via
     sh_link or sh_info.  Detect this here, by refusing to load a
     section that we are already in the process of loading.  */
  if (elf_tdata (abfd)->being_created[shindex])
    {
      _bfd_error_handler
	(_("%pB: warning: loop in section dependencies detected"), abfd);
      return false;
    }
  elf_tdata (abfd)->being_created[shindex] = true;

  hdr = elf_elfsections (abfd)[shindex];
  ehdr = elf_elfheader (abfd);
  name = bfd_elf_string_from_elf_section (abfd, ehdr->e_shstrndx,
					  hdr->sh_name);
  if (name == NULL)
    goto fail;

  bed = get_elf_backend_data (abfd);
  switch (hdr->sh_type)
    {
    case SHT_NULL:
      /* Inactive section. Throw it away.  */
      goto success;

    case SHT_PROGBITS:		/* Normal section with contents.  */
    case SHT_NOBITS:		/* .bss section.  */
    case SHT_HASH:		/* .hash section.  */
    case SHT_NOTE:		/* .note section.  */
    case SHT_INIT_ARRAY:	/* .init_array section.  */
    case SHT_FINI_ARRAY:	/* .fini_array section.  */
    case SHT_PREINIT_ARRAY:	/* .preinit_array section.  */
    case SHT_GNU_LIBLIST:	/* .gnu.liblist section.  */
    case SHT_GNU_HASH:		/* .gnu.hash section.  */
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_DYNAMIC:	/* Dynamic linking information.  */
      if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
	goto fail;

      if (hdr->sh_link > elf_numsections (abfd))
	{
	  /* PR 10478: Accept Solaris binaries with a sh_link field
	     set to SHN_BEFORE (LORESERVE) or SHN_AFTER (LORESERVE+1).  */
	  switch (bfd_get_arch (abfd))
	    {
	    case bfd_arch_i386:
	    case bfd_arch_sparc:
	      if (hdr->sh_link == (SHN_LORESERVE & 0xffff)
		  || hdr->sh_link == ((SHN_LORESERVE + 1) & 0xffff))
		break;
	      /* Otherwise fall through.  */
	    default:
	      goto fail;
	    }
	}
      else if (elf_elfsections (abfd)[hdr->sh_link] == NULL)
	goto fail;
      else if (elf_elfsections (abfd)[hdr->sh_link]->sh_type != SHT_STRTAB)
	{
	  Elf_Internal_Shdr *dynsymhdr;

	  /* The shared libraries distributed with hpux11 have a bogus
	     sh_link field for the ".dynamic" section.  Find the
	     string table for the ".dynsym" section instead.  */
	  if (elf_dynsymtab (abfd) != 0)
	    {
	      dynsymhdr = elf_elfsections (abfd)[elf_dynsymtab (abfd)];
	      hdr->sh_link = dynsymhdr->sh_link;
	    }
	  else
	    {
	      unsigned int i, num_sec;

	      num_sec = elf_numsections (abfd);
	      for (i = 1; i < num_sec; i++)
		{
		  dynsymhdr = elf_elfsections (abfd)[i];
		  if (dynsymhdr->sh_type == SHT_DYNSYM)
		    {
		      hdr->sh_link = dynsymhdr->sh_link;
		      break;
		    }
		}
	    }
	}
      goto success;

    case SHT_SYMTAB:		/* A symbol table.  */
      if (elf_onesymtab (abfd) == shindex)
	goto success;

      if (hdr->sh_entsize != bed->s->sizeof_sym)
	goto fail;

      if (hdr->sh_info * hdr->sh_entsize > hdr->sh_size)
	{
	  if (hdr->sh_size != 0)
	    goto fail;
	  /* Some assemblers erroneously set sh_info to one with a
	     zero sh_size.  ld sees this as a global symbol count
	     of (unsigned) -1.  Fix it here.  */
	  hdr->sh_info = 0;
	  goto success;
	}

      /* PR 18854: A binary might contain more than one symbol table.
	 Unusual, but possible.  Warn, but continue.  */
      if (elf_onesymtab (abfd) != 0)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: warning: multiple symbol tables detected"
	       " - ignoring the table in section %u"),
	     abfd, shindex);
	  goto success;
	}
      elf_onesymtab (abfd) = shindex;
      elf_symtab_hdr (abfd) = *hdr;
      elf_elfsections (abfd)[shindex] = hdr = & elf_symtab_hdr (abfd);
      abfd->flags |= HAS_SYMS;

      /* Sometimes a shared object will map in the symbol table.  If
	 SHF_ALLOC is set, and this is a shared object, then we also
	 treat this section as a BFD section.  We can not base the
	 decision purely on SHF_ALLOC, because that flag is sometimes
	 set in a relocatable object file, which would confuse the
	 linker.  */
      if ((hdr->sh_flags & SHF_ALLOC) != 0
	  && (abfd->flags & DYNAMIC) != 0
	  && ! _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						shindex))
	goto fail;

      /* Go looking for SHT_SYMTAB_SHNDX too, since if there is one we
	 can't read symbols without that section loaded as well.  It
	 is most likely specified by the next section header.  */
      {
	elf_section_list * entry;
	unsigned int i, num_sec;

	for (entry = elf_symtab_shndx_list (abfd); entry; entry = entry->next)
	  if (entry->hdr.sh_link == shindex)
	    goto success;

	num_sec = elf_numsections (abfd);
	for (i = shindex + 1; i < num_sec; i++)
	  {
	    Elf_Internal_Shdr *hdr2 = elf_elfsections (abfd)[i];

	    if (hdr2->sh_type == SHT_SYMTAB_SHNDX
		&& hdr2->sh_link == shindex)
	      break;
	  }

	if (i == num_sec)
	  for (i = 1; i < shindex; i++)
	    {
	      Elf_Internal_Shdr *hdr2 = elf_elfsections (abfd)[i];

	      if (hdr2->sh_type == SHT_SYMTAB_SHNDX
		  && hdr2->sh_link == shindex)
		break;
	    }

	if (i != shindex)
	  ret = bfd_section_from_shdr (abfd, i);
	/* else FIXME: we have failed to find the symbol table.
	   Should we issue an error?  */
	goto success;
      }

    case SHT_DYNSYM:		/* A dynamic symbol table.  */
      if (elf_dynsymtab (abfd) == shindex)
	goto success;

      if (hdr->sh_entsize != bed->s->sizeof_sym)
	goto fail;

      if (hdr->sh_info * hdr->sh_entsize > hdr->sh_size)
	{
	  if (hdr->sh_size != 0)
	    goto fail;

	  /* Some linkers erroneously set sh_info to one with a
	     zero sh_size.  ld sees this as a global symbol count
	     of (unsigned) -1.  Fix it here.  */
	  hdr->sh_info = 0;
	  goto success;
	}

      /* PR 18854: A binary might contain more than one dynamic symbol table.
	 Unusual, but possible.  Warn, but continue.  */
      if (elf_dynsymtab (abfd) != 0)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: warning: multiple dynamic symbol tables detected"
	       " - ignoring the table in section %u"),
	     abfd, shindex);
	  goto success;
	}
      elf_dynsymtab (abfd) = shindex;
      elf_tdata (abfd)->dynsymtab_hdr = *hdr;
      elf_elfsections (abfd)[shindex] = hdr = &elf_tdata (abfd)->dynsymtab_hdr;
      abfd->flags |= HAS_SYMS;

      /* Besides being a symbol table, we also treat this as a regular
	 section, so that objcopy can handle it.  */
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_SYMTAB_SHNDX: /* Symbol section indices when >64k sections.  */
      {
	elf_section_list * entry;

	for (entry = elf_symtab_shndx_list (abfd); entry; entry = entry->next)
	  if (entry->ndx == shindex)
	    goto success;

	entry = bfd_alloc (abfd, sizeof (*entry));
	if (entry == NULL)
	  goto fail;
	entry->ndx = shindex;
	entry->hdr = * hdr;
	entry->next = elf_symtab_shndx_list (abfd);
	elf_symtab_shndx_list (abfd) = entry;
	elf_elfsections (abfd)[shindex] = & entry->hdr;
	goto success;
      }

    case SHT_STRTAB:		/* A string table.  */
      if (hdr->bfd_section != NULL)
	goto success;

      if (ehdr->e_shstrndx == shindex)
	{
	  elf_tdata (abfd)->shstrtab_hdr = *hdr;
	  elf_elfsections (abfd)[shindex] = &elf_tdata (abfd)->shstrtab_hdr;
	  goto success;
	}

      if (elf_elfsections (abfd)[elf_onesymtab (abfd)]->sh_link == shindex)
	{
	symtab_strtab:
	  elf_tdata (abfd)->strtab_hdr = *hdr;
	  elf_elfsections (abfd)[shindex] = &elf_tdata (abfd)->strtab_hdr;
	  goto success;
	}

      if (elf_elfsections (abfd)[elf_dynsymtab (abfd)]->sh_link == shindex)
	{
	dynsymtab_strtab:
	  elf_tdata (abfd)->dynstrtab_hdr = *hdr;
	  hdr = &elf_tdata (abfd)->dynstrtab_hdr;
	  elf_elfsections (abfd)[shindex] = hdr;
	  /* We also treat this as a regular section, so that objcopy
	     can handle it.  */
	  ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						 shindex);
	  goto success;
	}

      /* If the string table isn't one of the above, then treat it as a
	 regular section.  We need to scan all the headers to be sure,
	 just in case this strtab section appeared before the above.  */
      if (elf_onesymtab (abfd) == 0 || elf_dynsymtab (abfd) == 0)
	{
	  unsigned int i, num_sec;

	  num_sec = elf_numsections (abfd);
	  for (i = 1; i < num_sec; i++)
	    {
	      Elf_Internal_Shdr *hdr2 = elf_elfsections (abfd)[i];
	      if (hdr2->sh_link == shindex)
		{
		  /* Prevent endless recursion on broken objects.  */
		  if (i == shindex)
		    goto fail;
		  if (! bfd_section_from_shdr (abfd, i))
		    goto fail;
		  if (elf_onesymtab (abfd) == i)
		    goto symtab_strtab;
		  if (elf_dynsymtab (abfd) == i)
		    goto dynsymtab_strtab;
		}
	    }
	}
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_REL:
    case SHT_RELA:
    case SHT_RELR:
      /* *These* do a lot of work -- but build no sections!  */
      {
	asection *target_sect;
	Elf_Internal_Shdr *hdr2, **p_hdr;
	unsigned int num_sec = elf_numsections (abfd);
	struct bfd_elf_section_data *esdt;
	bfd_size_type size;

	if (hdr->sh_type == SHT_REL)
	  size = bed->s->sizeof_rel;
	else if (hdr->sh_type == SHT_RELA)
	  size = bed->s->sizeof_rela;
	else
	  size = bed->s->arch_size / 8;
	if (hdr->sh_entsize != size)
	  goto fail;

	/* Check for a bogus link to avoid crashing.  */
	if (hdr->sh_link >= num_sec)
	  {
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: invalid link %u for reloc section %s (index %u)"),
	       abfd, hdr->sh_link, name, shindex);
	    ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
	    goto success;
	  }

	/* Get the symbol table.  */
	if ((elf_elfsections (abfd)[hdr->sh_link]->sh_type == SHT_SYMTAB
	     || elf_elfsections (abfd)[hdr->sh_link]->sh_type == SHT_DYNSYM)
	    && ! bfd_section_from_shdr (abfd, hdr->sh_link))
	  goto fail;

	/* If this is an alloc section in an executable or shared
	   library, or the reloc section does not use the main symbol
	   table we don't treat it as a reloc section.  BFD can't
	   adequately represent such a section, so at least for now,
	   we don't try.  We just present it as a normal section.  We
	   also can't use it as a reloc section if it points to the
	   null section, an invalid section, another reloc section, or
	   its sh_link points to the null section.  */
	if (((abfd->flags & (DYNAMIC | EXEC_P)) != 0
	     && (hdr->sh_flags & SHF_ALLOC) != 0)
	    || (hdr->sh_flags & SHF_COMPRESSED) != 0
	    || hdr->sh_type == SHT_RELR
	    || hdr->sh_link == SHN_UNDEF
	    || hdr->sh_link != elf_onesymtab (abfd)
	    || hdr->sh_info == SHN_UNDEF
	    || hdr->sh_info >= num_sec
	    || elf_elfsections (abfd)[hdr->sh_info]->sh_type == SHT_REL
	    || elf_elfsections (abfd)[hdr->sh_info]->sh_type == SHT_RELA)
	  {
	    ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
	    goto success;
	  }

	if (! bfd_section_from_shdr (abfd, hdr->sh_info))
	  goto fail;

	target_sect = bfd_section_from_elf_index (abfd, hdr->sh_info);
	if (target_sect == NULL)
	  goto fail;

	esdt = elf_section_data (target_sect);
	if (hdr->sh_type == SHT_RELA)
	  p_hdr = &esdt->rela.hdr;
	else
	  p_hdr = &esdt->rel.hdr;

	/* PR 17512: file: 0b4f81b7.
	   Also see PR 24456, for a file which deliberately has two reloc
	   sections.  */
	if (*p_hdr != NULL)
	  {
	    if (!bed->init_secondary_reloc_section (abfd, hdr, name, shindex))
	      {
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("%pB: warning: secondary relocation section '%s' "
		     "for section %pA found - ignoring"),
		   abfd, name, target_sect);
	      }
	    else
	      esdt->has_secondary_relocs = true;
	    goto success;
	  }

	hdr2 = (Elf_Internal_Shdr *) bfd_alloc (abfd, sizeof (*hdr2));
	if (hdr2 == NULL)
	  goto fail;
	*hdr2 = *hdr;
	*p_hdr = hdr2;
	elf_elfsections (abfd)[shindex] = hdr2;
	target_sect->reloc_count += (NUM_SHDR_ENTRIES (hdr)
				     * bed->s->int_rels_per_ext_rel);
	target_sect->flags |= SEC_RELOC;
	target_sect->relocation = NULL;
	target_sect->rel_filepos = hdr->sh_offset;
	/* In the section to which the relocations apply, mark whether
	   its relocations are of the REL or RELA variety.  */
	if (hdr->sh_size != 0)
	  {
	    if (hdr->sh_type == SHT_RELA)
	      target_sect->use_rela_p = 1;
	  }
	abfd->flags |= HAS_RELOC;
	goto success;
      }

    case SHT_GNU_verdef:
      if (hdr->sh_info != 0)
	elf_dynverdef (abfd) = shindex;
      elf_tdata (abfd)->dynverdef_hdr = *hdr;
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_GNU_versym:
      if (hdr->sh_entsize != sizeof (Elf_External_Versym))
	goto fail;

      elf_dynversym (abfd) = shindex;
      elf_tdata (abfd)->dynversym_hdr = *hdr;
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_GNU_verneed:
      if (hdr->sh_info != 0)
	elf_dynverref (abfd) = shindex;
      elf_tdata (abfd)->dynverref_hdr = *hdr;
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_SHLIB:
      goto success;

    case SHT_GROUP:
      if (! IS_VALID_GROUP_SECTION_HEADER (hdr, GRP_ENTRY_SIZE))
	goto fail;

      if (!_bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
	goto fail;

      goto success;

    default:
      /* Possibly an attributes section.  */
      if (hdr->sh_type == SHT_GNU_ATTRIBUTES
	  || hdr->sh_type == bed->obj_attrs_section_type)
	{
	  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
	    goto fail;
	  _bfd_elf_parse_attributes (abfd, hdr);
	  goto success;
	}

      /* Check for any processor-specific section types.  */
      if (bed->elf_backend_section_from_shdr (abfd, hdr, name, shindex))
	goto success;

      if (hdr->sh_type >= SHT_LOUSER && hdr->sh_type <= SHT_HIUSER)
	{
	  if ((hdr->sh_flags & SHF_ALLOC) != 0)
	    /* FIXME: How to properly handle allocated section reserved
	       for applications?  */
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: unknown type [%#x] section `%s'"),
	       abfd, hdr->sh_type, name);
	  else
	    {
	      /* Allow sections reserved for applications.  */
	      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
	      goto success;
	    }
	}
      else if (hdr->sh_type >= SHT_LOPROC
	       && hdr->sh_type <= SHT_HIPROC)
	/* FIXME: We should handle this section.  */
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB: unknown type [%#x] section `%s'"),
	   abfd, hdr->sh_type, name);
      else if (hdr->sh_type >= SHT_LOOS && hdr->sh_type <= SHT_HIOS)
	{
	  /* Unrecognised OS-specific sections.  */
	  if ((hdr->sh_flags & SHF_OS_NONCONFORMING) != 0)
	    /* SHF_OS_NONCONFORMING indicates that special knowledge is
	       required to correctly process the section and the file should
	       be rejected with an error message.  */
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: unknown type [%#x] section `%s'"),
	       abfd, hdr->sh_type, name);
	  else
	    {
	      /* Otherwise it should be processed.  */
	      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
	      goto success;
	    }
	}
      else
	/* FIXME: We should handle this section.  */
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB: unknown type [%#x] section `%s'"),
	   abfd, hdr->sh_type, name);

      goto fail;
    }

 fail:
  ret = false;
 success:
  elf_tdata (abfd)->being_created[shindex] = false;
  return ret;
}

/* Return the local symbol specified by ABFD, R_SYMNDX.  */

Elf_Internal_Sym *
bfd_sym_from_r_symndx (struct sym_cache *cache,
		       bfd *abfd,
		       unsigned long r_symndx)
{
  unsigned int ent = r_symndx % LOCAL_SYM_CACHE_SIZE;

  if (cache->abfd != abfd || cache->indx[ent] != r_symndx)
    {
      Elf_Internal_Shdr *symtab_hdr;
      unsigned char esym[sizeof (Elf64_External_Sym)];
      Elf_External_Sym_Shndx eshndx;

      symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
      if (bfd_elf_get_elf_syms (abfd, symtab_hdr, 1, r_symndx,
				&cache->sym[ent], esym, &eshndx) == NULL)
	return NULL;

      if (cache->abfd != abfd)
	{
	  memset (cache->indx, -1, sizeof (cache->indx));
	  cache->abfd = abfd;
	}
      cache->indx[ent] = r_symndx;
    }

  return &cache->sym[ent];
}

/* Given an ELF section number, retrieve the corresponding BFD
   section.  */

asection *
bfd_section_from_elf_index (bfd *abfd, unsigned int sec_index)
{
  if (sec_index >= elf_numsections (abfd))
    return NULL;
  return elf_elfsections (abfd)[sec_index]->bfd_section;
}

static const struct bfd_elf_special_section special_sections_b[] =
{
  { STRING_COMMA_LEN (".bss"), -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE },
  { NULL,		    0,	0, 0,		 0 }
};

static const struct bfd_elf_special_section special_sections_c[] =
{
  { STRING_COMMA_LEN (".comment"), 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".ctf"),	0, SHT_PROGBITS,    0 },
  { NULL,			0, 0, 0,	    0 }
};

static const struct bfd_elf_special_section special_sections_d[] =
{
  { STRING_COMMA_LEN (".data"),		-2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".data1"),	 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  /* There are more DWARF sections than these, but they needn't be added here
     unless you have to cope with broken compilers that don't emit section
     attributes or you want to help the user writing assembler.  */
  { STRING_COMMA_LEN (".debug"),	 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_line"),	 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_info"),	 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_abbrev"),	 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_aranges"), 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".dynamic"),	 0, SHT_DYNAMIC,  SHF_ALLOC },
  { STRING_COMMA_LEN (".dynstr"),	 0, SHT_STRTAB,	  SHF_ALLOC },
  { STRING_COMMA_LEN (".dynsym"),	 0, SHT_DYNSYM,	  SHF_ALLOC },
  { NULL,		       0,	 0, 0,		  0 }
};

static const struct bfd_elf_special_section special_sections_f[] =
{
  { STRING_COMMA_LEN (".fini"),	       0, SHT_PROGBITS,	  SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".fini_array"), -2, SHT_FINI_ARRAY, SHF_ALLOC + SHF_WRITE },
  { NULL,			   0 , 0, 0,		  0 }
};

static const struct bfd_elf_special_section special_sections_g[] =
{
  { STRING_COMMA_LEN (".gnu.linkonce.b"), -2, SHT_NOBITS,      SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".gnu.linkonce.n"), -2, SHT_NOBITS,      SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".gnu.linkonce.p"), -2, SHT_PROGBITS,    SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".gnu.lto_"),	  -1, SHT_PROGBITS,    SHF_EXCLUDE },
  { STRING_COMMA_LEN (".got"),		   0, SHT_PROGBITS,    SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".gnu.version"),	   0, SHT_GNU_versym,  0 },
  { STRING_COMMA_LEN (".gnu.version_d"),   0, SHT_GNU_verdef,  0 },
  { STRING_COMMA_LEN (".gnu.version_r"),   0, SHT_GNU_verneed, 0 },
  { STRING_COMMA_LEN (".gnu.liblist"),	   0, SHT_GNU_LIBLIST, SHF_ALLOC },
  { STRING_COMMA_LEN (".gnu.conflict"),	   0, SHT_RELA,	       SHF_ALLOC },
  { STRING_COMMA_LEN (".gnu.hash"),	   0, SHT_GNU_HASH,    SHF_ALLOC },
  { NULL,			 0,	   0, 0,	       0 }
};

static const struct bfd_elf_special_section special_sections_h[] =
{
  { STRING_COMMA_LEN (".hash"), 0, SHT_HASH,	 SHF_ALLOC },
  { NULL,		     0, 0, 0,		 0 }
};

static const struct bfd_elf_special_section special_sections_i[] =
{
  { STRING_COMMA_LEN (".init"),	       0, SHT_PROGBITS,	  SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".init_array"), -2, SHT_INIT_ARRAY, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".interp"),      0, SHT_PROGBITS,	  0 },
  { NULL,		       0,      0, 0,		  0 }
};

static const struct bfd_elf_special_section special_sections_l[] =
{
  { STRING_COMMA_LEN (".line"), 0, SHT_PROGBITS, 0 },
  { NULL,		     0, 0, 0,		 0 }
};

static const struct bfd_elf_special_section special_sections_n[] =
{
  { STRING_COMMA_LEN (".noinit"),	 -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".note.GNU-stack"), 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".note"),		 -1, SHT_NOTE,	   0 },
  { NULL,		     0,		  0, 0,		   0 }
};

static const struct bfd_elf_special_section special_sections_p[] =
{
  { STRING_COMMA_LEN (".persistent.bss"), 0, SHT_NOBITS,	SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".persistent"),	 -2, SHT_PROGBITS,	SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".preinit_array"), -2, SHT_PREINIT_ARRAY, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".plt"),		  0, SHT_PROGBITS,	SHF_ALLOC + SHF_EXECINSTR },
  { NULL,		    0,		  0, 0,			0 }
};

static const struct bfd_elf_special_section special_sections_r[] =
{
  { STRING_COMMA_LEN (".rodata"), -2, SHT_PROGBITS, SHF_ALLOC },
  { STRING_COMMA_LEN (".rodata1"), 0, SHT_PROGBITS, SHF_ALLOC },
  { STRING_COMMA_LEN (".relr.dyn"), 0, SHT_RELR, SHF_ALLOC },
  { STRING_COMMA_LEN (".rela"),	  -1, SHT_RELA,	    0 },
  { STRING_COMMA_LEN (".rel"),	  -1, SHT_REL,	    0 },
  { NULL,		    0,	   0, 0,	    0 }
};

static const struct bfd_elf_special_section special_sections_s[] =
{
  { STRING_COMMA_LEN (".shstrtab"), 0, SHT_STRTAB, 0 },
  { STRING_COMMA_LEN (".strtab"),   0, SHT_STRTAB, 0 },
  { STRING_COMMA_LEN (".symtab"),   0, SHT_SYMTAB, 0 },
  /* See struct bfd_elf_special_section declaration for the semantics of
     this special case where .prefix_length != strlen (.prefix).  */
  { ".stabstr",			5,  3, SHT_STRTAB, 0 },
  { NULL,			0,  0, 0,	   0 }
};

static const struct bfd_elf_special_section special_sections_t[] =
{
  { STRING_COMMA_LEN (".text"),	 -2, SHT_PROGBITS, SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".tbss"),	 -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE + SHF_TLS },
  { STRING_COMMA_LEN (".tdata"), -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE + SHF_TLS },
  { NULL,		      0,  0, 0,		   0 }
};

static const struct bfd_elf_special_section special_sections_z[] =
{
  { STRING_COMMA_LEN (".zdebug_line"),	  0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".zdebug_info"),	  0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".zdebug_abbrev"),  0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".zdebug_aranges"), 0, SHT_PROGBITS, 0 },
  { NULL,		      0,  0, 0,		   0 }
};

static const struct bfd_elf_special_section * const special_sections[] =
{
  special_sections_b,		/* 'b' */
  special_sections_c,		/* 'c' */
  special_sections_d,		/* 'd' */
  NULL,				/* 'e' */
  special_sections_f,		/* 'f' */
  special_sections_g,		/* 'g' */
  special_sections_h,		/* 'h' */
  special_sections_i,		/* 'i' */
  NULL,				/* 'j' */
  NULL,				/* 'k' */
  special_sections_l,		/* 'l' */
  NULL,				/* 'm' */
  special_sections_n,		/* 'n' */
  NULL,				/* 'o' */
  special_sections_p,		/* 'p' */
  NULL,				/* 'q' */
  special_sections_r,		/* 'r' */
  special_sections_s,		/* 's' */
  special_sections_t,		/* 't' */
  NULL,				/* 'u' */
  NULL,				/* 'v' */
  NULL,				/* 'w' */
  NULL,				/* 'x' */
  NULL,				/* 'y' */
  special_sections_z		/* 'z' */
};

const struct bfd_elf_special_section *
_bfd_elf_get_special_section (const char *name,
			      const struct bfd_elf_special_section *spec,
			      unsigned int rela)
{
  int i;
  int len;

  len = strlen (name);

  for (i = 0; spec[i].prefix != NULL; i++)
    {
      int suffix_len;
      int prefix_len = spec[i].prefix_length;

      if (len < prefix_len)
	continue;
      if (memcmp (name, spec[i].prefix, prefix_len) != 0)
	continue;

      suffix_len = spec[i].suffix_length;
      if (suffix_len <= 0)
	{
	  if (name[prefix_len] != 0)
	    {
	      if (suffix_len == 0)
		continue;
	      if (name[prefix_len] != '.'
		  && (suffix_len == -2
		      || (rela && spec[i].type == SHT_REL)))
		continue;
	    }
	}
      else
	{
	  if (len < prefix_len + suffix_len)
	    continue;
	  if (memcmp (name + len - suffix_len,
		      spec[i].prefix + prefix_len,
		      suffix_len) != 0)
	    continue;
	}
      return &spec[i];
    }

  return NULL;
}

const struct bfd_elf_special_section *
_bfd_elf_get_sec_type_attr (bfd *abfd, asection *sec)
{
  int i;
  const struct bfd_elf_special_section *spec;
  const struct elf_backend_data *bed;

  /* See if this is one of the special sections.  */
  if (sec->name == NULL)
    return NULL;

  bed = get_elf_backend_data (abfd);
  spec = bed->special_sections;
  if (spec)
    {
      spec = _bfd_elf_get_special_section (sec->name,
					   bed->special_sections,
					   sec->use_rela_p);
      if (spec != NULL)
	return spec;
    }

  if (sec->name[0] != '.')
    return NULL;

  i = sec->name[1] - 'b';
  if (i < 0 || i > 'z' - 'b')
    return NULL;

  spec = special_sections[i];

  if (spec == NULL)
    return NULL;

  return _bfd_elf_get_special_section (sec->name, spec, sec->use_rela_p);
}

bool
_bfd_elf_new_section_hook (bfd *abfd, asection *sec)
{
  struct bfd_elf_section_data *sdata;
  const struct elf_backend_data *bed;
  const struct bfd_elf_special_section *ssect;

  sdata = (struct bfd_elf_section_data *) sec->used_by_bfd;
  if (sdata == NULL)
    {
      sdata = (struct bfd_elf_section_data *) bfd_zalloc (abfd,
							  sizeof (*sdata));
      if (sdata == NULL)
	return false;
      sec->used_by_bfd = sdata;
    }

  /* Indicate whether or not this section should use RELA relocations.  */
  bed = get_elf_backend_data (abfd);
  sec->use_rela_p = bed->default_use_rela_p;

  /* Set up ELF section type and flags for newly created sections, if
     there is an ABI mandated section.  */
  ssect = (*bed->get_sec_type_attr) (abfd, sec);
  if (ssect != NULL)
    {
      elf_section_type (sec) = ssect->type;
      elf_section_flags (sec) = ssect->attr;
    }

  return _bfd_generic_new_section_hook (abfd, sec);
}

/* Create a new bfd section from an ELF program header.

   Since program segments have no names, we generate a synthetic name
   of the form segment<NUM>, where NUM is generally the index in the
   program header table.  For segments that are split (see below) we
   generate the names segment<NUM>a and segment<NUM>b.

   Note that some program segments may have a file size that is different than
   (less than) the memory size.  All this means is that at execution the
   system must allocate the amount of memory specified by the memory size,
   but only initialize it with the first "file size" bytes read from the
   file.  This would occur for example, with program segments consisting
   of combined data+bss.

   To handle the above situation, this routine generates TWO bfd sections
   for the single program segment.  The first has the length specified by
   the file size of the segment, and the second has the length specified
   by the difference between the two sizes.  In effect, the segment is split
   into its initialized and uninitialized parts.  */

bool
_bfd_elf_make_section_from_phdr (bfd *abfd,
				 Elf_Internal_Phdr *hdr,
				 int hdr_index,
				 const char *type_name)
{
  asection *newsect;
  char *name;
  char namebuf[64];
  size_t len;
  int split;
  unsigned int opb = bfd_octets_per_byte (abfd, NULL);

  split = ((hdr->p_memsz > 0)
	    && (hdr->p_filesz > 0)
	    && (hdr->p_memsz > hdr->p_filesz));

  if (hdr->p_filesz > 0)
    {
      sprintf (namebuf, "%s%d%s", type_name, hdr_index, split ? "a" : "");
      len = strlen (namebuf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (!name)
	return false;
      memcpy (name, namebuf, len);
      newsect = bfd_make_section (abfd, name);
      if (newsect == NULL)
	return false;
      newsect->vma = hdr->p_vaddr / opb;
      newsect->lma = hdr->p_paddr / opb;
      newsect->size = hdr->p_filesz;
      newsect->filepos = hdr->p_offset;
      newsect->flags |= SEC_HAS_CONTENTS;
      newsect->alignment_power = bfd_log2 (hdr->p_align);
      if (hdr->p_type == PT_LOAD)
	{
	  newsect->flags |= SEC_ALLOC;
	  newsect->flags |= SEC_LOAD;
	  if (hdr->p_flags & PF_X)
	    {
	      /* FIXME: all we known is that it has execute PERMISSION,
		 may be data.  */
	      newsect->flags |= SEC_CODE;
	    }
	}
      if (!(hdr->p_flags & PF_W))
	{
	  newsect->flags |= SEC_READONLY;
	}
    }

  if (hdr->p_memsz > hdr->p_filesz)
    {
      bfd_vma align;

      sprintf (namebuf, "%s%d%s", type_name, hdr_index, split ? "b" : "");
      len = strlen (namebuf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (!name)
	return false;
      memcpy (name, namebuf, len);
      newsect = bfd_make_section (abfd, name);
      if (newsect == NULL)
	return false;
      newsect->vma = (hdr->p_vaddr + hdr->p_filesz) / opb;
      newsect->lma = (hdr->p_paddr + hdr->p_filesz) / opb;
      newsect->size = hdr->p_memsz - hdr->p_filesz;
      newsect->filepos = hdr->p_offset + hdr->p_filesz;
      align = newsect->vma & -newsect->vma;
      if (align == 0 || align > hdr->p_align)
	align = hdr->p_align;
      newsect->alignment_power = bfd_log2 (align);
      if (hdr->p_type == PT_LOAD)
	{
	  newsect->flags |= SEC_ALLOC;
	  if (hdr->p_flags & PF_X)
	    newsect->flags |= SEC_CODE;
	}
      if (!(hdr->p_flags & PF_W))
	newsect->flags |= SEC_READONLY;
    }

  return true;
}

static bool
_bfd_elf_core_find_build_id (bfd *templ, bfd_vma offset)
{
  /* The return value is ignored.  Build-ids are considered optional.  */
  if (templ->xvec->flavour == bfd_target_elf_flavour)
    return (*get_elf_backend_data (templ)->elf_backend_core_find_build_id)
      (templ, offset);
  return false;
}

bool
bfd_section_from_phdr (bfd *abfd, Elf_Internal_Phdr *hdr, int hdr_index)
{
  const struct elf_backend_data *bed;

  switch (hdr->p_type)
    {
    case PT_NULL:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "null");

    case PT_LOAD:
      if (! _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "load"))
	return false;
      if (bfd_get_format (abfd) == bfd_core && abfd->build_id == NULL)
	_bfd_elf_core_find_build_id (abfd, hdr->p_offset);
      return true;

    case PT_DYNAMIC:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "dynamic");

    case PT_INTERP:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "interp");

    case PT_NOTE:
      if (! _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "note"))
	return false;
      if (! elf_read_notes (abfd, hdr->p_offset, hdr->p_filesz,
			    hdr->p_align))
	return false;
      return true;

    case PT_SHLIB:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "shlib");

    case PT_PHDR:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "phdr");

    case PT_GNU_EH_FRAME:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index,
					      "eh_frame_hdr");

    case PT_GNU_STACK:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "stack");

    case PT_GNU_RELRO:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "relro");

    case PT_GNU_SFRAME:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index,
					      "sframe");

    default:
      /* Check for any processor-specific program segment types.  */
      bed = get_elf_backend_data (abfd);
      return bed->elf_backend_section_from_phdr (abfd, hdr, hdr_index, "proc");
    }
}

/* Return the REL_HDR for SEC, assuming there is only a single one, either
   REL or RELA.  */

Elf_Internal_Shdr *
_bfd_elf_single_rel_hdr (asection *sec)
{
  if (elf_section_data (sec)->rel.hdr)
    {
      BFD_ASSERT (elf_section_data (sec)->rela.hdr == NULL);
      return elf_section_data (sec)->rel.hdr;
    }
  else
    return elf_section_data (sec)->rela.hdr;
}

static bool
_bfd_elf_set_reloc_sh_name (bfd *abfd,
			    Elf_Internal_Shdr *rel_hdr,
			    const char *sec_name,
			    bool use_rela_p)
{
  char *name = (char *) bfd_alloc (abfd,
				   sizeof ".rela" + strlen (sec_name));
  if (name == NULL)
    return false;

  sprintf (name, "%s%s", use_rela_p ? ".rela" : ".rel", sec_name);
  rel_hdr->sh_name =
    (unsigned int) _bfd_elf_strtab_add (elf_shstrtab (abfd), name,
					false);
  if (rel_hdr->sh_name == (unsigned int) -1)
    return false;

  return true;
}

/* Allocate and initialize a section-header for a new reloc section,
   containing relocations against ASECT.  It is stored in RELDATA.  If
   USE_RELA_P is TRUE, we use RELA relocations; otherwise, we use REL
   relocations.  */

static bool
_bfd_elf_init_reloc_shdr (bfd *abfd,
			  struct bfd_elf_section_reloc_data *reldata,
			  const char *sec_name,
			  bool use_rela_p,
			  bool delay_st_name_p)
{
  Elf_Internal_Shdr *rel_hdr;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  BFD_ASSERT (reldata->hdr == NULL);
  rel_hdr = bfd_zalloc (abfd, sizeof (*rel_hdr));
  reldata->hdr = rel_hdr;

  if (delay_st_name_p)
    rel_hdr->sh_name = (unsigned int) -1;
  else if (!_bfd_elf_set_reloc_sh_name (abfd, rel_hdr, sec_name,
					use_rela_p))
    return false;
  rel_hdr->sh_type = use_rela_p ? SHT_RELA : SHT_REL;
  rel_hdr->sh_entsize = (use_rela_p
			 ? bed->s->sizeof_rela
			 : bed->s->sizeof_rel);
  rel_hdr->sh_addralign = (bfd_vma) 1 << bed->s->log_file_align;
  rel_hdr->sh_flags = 0;
  rel_hdr->sh_addr = 0;
  rel_hdr->sh_size = 0;
  rel_hdr->sh_offset = 0;

  return true;
}

/* Return the default section type based on the passed in section flags.  */

int
bfd_elf_get_default_section_type (flagword flags)
{
  if ((flags & (SEC_ALLOC | SEC_IS_COMMON)) != 0
      && (flags & (SEC_LOAD | SEC_HAS_CONTENTS)) == 0)
    return SHT_NOBITS;
  return SHT_PROGBITS;
}

struct fake_section_arg
{
  struct bfd_link_info *link_info;
  bool failed;
};

/* Set up an ELF internal section header for a section.  */

static void
elf_fake_sections (bfd *abfd, asection *asect, void *fsarg)
{
  struct fake_section_arg *arg = (struct fake_section_arg *)fsarg;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct bfd_elf_section_data *esd = elf_section_data (asect);
  Elf_Internal_Shdr *this_hdr;
  unsigned int sh_type;
  const char *name = asect->name;
  bool delay_st_name_p = false;
  bfd_vma mask;

  if (arg->failed)
    {
      /* We already failed; just get out of the bfd_map_over_sections
	 loop.  */
      return;
    }

  this_hdr = &esd->this_hdr;

  /* ld: compress DWARF debug sections with names: .debug_*.  */
  if (arg->link_info
      && (abfd->flags & BFD_COMPRESS) != 0
      && (asect->flags & SEC_DEBUGGING) != 0
      && name[1] == 'd'
      && name[6] == '_')
    {
      /* If this section will be compressed, delay adding section
	 name to section name section after it is compressed in
	 _bfd_elf_assign_file_positions_for_non_load.  */
      delay_st_name_p = true;
    }

  if (delay_st_name_p)
    this_hdr->sh_name = (unsigned int) -1;
  else
    {
      this_hdr->sh_name
	= (unsigned int) _bfd_elf_strtab_add (elf_shstrtab (abfd),
					      name, false);
      if (this_hdr->sh_name == (unsigned int) -1)
	{
	  arg->failed = true;
	  return;
	}
    }

  /* Don't clear sh_flags. Assembler may set additional bits.  */

  if ((asect->flags & SEC_ALLOC) != 0
      || asect->user_set_vma)
    this_hdr->sh_addr = asect->vma * bfd_octets_per_byte (abfd, asect);
  else
    this_hdr->sh_addr = 0;

  this_hdr->sh_offset = 0;
  this_hdr->sh_size = asect->size;
  this_hdr->sh_link = 0;
  /* PR 17512: file: 0eb809fe, 8b0535ee.  */
  if (asect->alignment_power >= (sizeof (bfd_vma) * 8) - 1)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: error: alignment power %d of section `%pA' is too big"),
	 abfd, asect->alignment_power, asect);
      arg->failed = true;
      return;
    }
  /* Set sh_addralign to the highest power of two given by alignment
     consistent with the section VMA.  Linker scripts can force VMA.  */
  mask = ((bfd_vma) 1 << asect->alignment_power) | this_hdr->sh_addr;
  this_hdr->sh_addralign = mask & -mask;
  /* The sh_entsize and sh_info fields may have been set already by
     copy_private_section_data.  */

  this_hdr->bfd_section = asect;
  this_hdr->contents = NULL;

  /* If the section type is unspecified, we set it based on
     asect->flags.  */
  if (asect->type != 0)
    sh_type = asect->type;
  else if ((asect->flags & SEC_GROUP) != 0)
    sh_type = SHT_GROUP;
  else
    sh_type = bfd_elf_get_default_section_type (asect->flags);

  if (this_hdr->sh_type == SHT_NULL)
    this_hdr->sh_type = sh_type;
  else if (this_hdr->sh_type == SHT_NOBITS
	   && sh_type == SHT_PROGBITS
	   && (asect->flags & SEC_ALLOC) != 0)
    {
      /* Warn if we are changing a NOBITS section to PROGBITS, but
	 allow the link to proceed.  This can happen when users link
	 non-bss input sections to bss output sections, or emit data
	 to a bss output section via a linker script.  */
      _bfd_error_handler
	(_("warning: section `%pA' type changed to PROGBITS"), asect);
      this_hdr->sh_type = sh_type;
    }

  switch (this_hdr->sh_type)
    {
    default:
      break;

    case SHT_STRTAB:
    case SHT_NOTE:
    case SHT_NOBITS:
    case SHT_PROGBITS:
      break;

    case SHT_INIT_ARRAY:
    case SHT_FINI_ARRAY:
    case SHT_PREINIT_ARRAY:
      this_hdr->sh_entsize = bed->s->arch_size / 8;
      break;

    case SHT_HASH:
      this_hdr->sh_entsize = bed->s->sizeof_hash_entry;
      break;

    case SHT_DYNSYM:
      this_hdr->sh_entsize = bed->s->sizeof_sym;
      break;

    case SHT_DYNAMIC:
      this_hdr->sh_entsize = bed->s->sizeof_dyn;
      break;

    case SHT_RELA:
      if (get_elf_backend_data (abfd)->may_use_rela_p)
	this_hdr->sh_entsize = bed->s->sizeof_rela;
      break;

     case SHT_REL:
      if (get_elf_backend_data (abfd)->may_use_rel_p)
	this_hdr->sh_entsize = bed->s->sizeof_rel;
      break;

     case SHT_GNU_versym:
      this_hdr->sh_entsize = sizeof (Elf_External_Versym);
      break;

     case SHT_GNU_verdef:
      this_hdr->sh_entsize = 0;
      /* objcopy or strip will copy over sh_info, but may not set
	 cverdefs.  The linker will set cverdefs, but sh_info will be
	 zero.  */
      if (this_hdr->sh_info == 0)
	this_hdr->sh_info = elf_tdata (abfd)->cverdefs;
      else
	BFD_ASSERT (elf_tdata (abfd)->cverdefs == 0
		    || this_hdr->sh_info == elf_tdata (abfd)->cverdefs);
      break;

    case SHT_GNU_verneed:
      this_hdr->sh_entsize = 0;
      /* objcopy or strip will copy over sh_info, but may not set
	 cverrefs.  The linker will set cverrefs, but sh_info will be
	 zero.  */
      if (this_hdr->sh_info == 0)
	this_hdr->sh_info = elf_tdata (abfd)->cverrefs;
      else
	BFD_ASSERT (elf_tdata (abfd)->cverrefs == 0
		    || this_hdr->sh_info == elf_tdata (abfd)->cverrefs);
      break;

    case SHT_GROUP:
      this_hdr->sh_entsize = GRP_ENTRY_SIZE;
      break;

    case SHT_GNU_HASH:
      this_hdr->sh_entsize = bed->s->arch_size == 64 ? 0 : 4;
      break;
    }

  if ((asect->flags & SEC_ALLOC) != 0)
    this_hdr->sh_flags |= SHF_ALLOC;
  if ((asect->flags & SEC_READONLY) == 0)
    this_hdr->sh_flags |= SHF_WRITE;
  if ((asect->flags & SEC_CODE) != 0)
    this_hdr->sh_flags |= SHF_EXECINSTR;
  if ((asect->flags & SEC_MERGE) != 0)
    {
      this_hdr->sh_flags |= SHF_MERGE;
      this_hdr->sh_entsize = asect->entsize;
    }
  if ((asect->flags & SEC_STRINGS) != 0)
    this_hdr->sh_flags |= SHF_STRINGS;
  if ((asect->flags & SEC_GROUP) == 0 && elf_group_name (asect) != NULL)
    this_hdr->sh_flags |= SHF_GROUP;
  if ((asect->flags & SEC_THREAD_LOCAL) != 0)
    {
      this_hdr->sh_flags |= SHF_TLS;
      if (asect->size == 0
	  && (asect->flags & SEC_HAS_CONTENTS) == 0)
	{
	  struct bfd_link_order *o = asect->map_tail.link_order;

	  this_hdr->sh_size = 0;
	  if (o != NULL)
	    {
	      this_hdr->sh_size = o->offset + o->size;
	      if (this_hdr->sh_size != 0)
		this_hdr->sh_type = SHT_NOBITS;
	    }
	}
    }
  if ((asect->flags & (SEC_GROUP | SEC_EXCLUDE)) == SEC_EXCLUDE)
    this_hdr->sh_flags |= SHF_EXCLUDE;

  /* If the section has relocs, set up a section header for the
     SHT_REL[A] section.  If two relocation sections are required for
     this section, it is up to the processor-specific back-end to
     create the other.  */
  if ((asect->flags & SEC_RELOC) != 0)
    {
      /* When doing a relocatable link, create both REL and RELA sections if
	 needed.  */
      if (arg->link_info
	  /* Do the normal setup if we wouldn't create any sections here.  */
	  && esd->rel.count + esd->rela.count > 0
	  && (bfd_link_relocatable (arg->link_info)
	      || arg->link_info->emitrelocations))
	{
	  if (esd->rel.count && esd->rel.hdr == NULL
	      && !_bfd_elf_init_reloc_shdr (abfd, &esd->rel, name,
					    false, delay_st_name_p))
	    {
	      arg->failed = true;
	      return;
	    }
	  if (esd->rela.count && esd->rela.hdr == NULL
	      && !_bfd_elf_init_reloc_shdr (abfd, &esd->rela, name,
					    true, delay_st_name_p))
	    {
	      arg->failed = true;
	      return;
	    }
	}
      else if (!_bfd_elf_init_reloc_shdr (abfd,
					  (asect->use_rela_p
					   ? &esd->rela : &esd->rel),
					  name,
					  asect->use_rela_p,
					  delay_st_name_p))
	{
	  arg->failed = true;
	  return;
	}
    }

  /* Check for processor-specific section types.  */
  sh_type = this_hdr->sh_type;
  if (bed->elf_backend_fake_sections
      && !(*bed->elf_backend_fake_sections) (abfd, this_hdr, asect))
    {
      arg->failed = true;
      return;
    }

  if (sh_type == SHT_NOBITS && asect->size != 0)
    {
      /* Don't change the header type from NOBITS if we are being
	 called for objcopy --only-keep-debug.  */
      this_hdr->sh_type = sh_type;
    }
}

/* Fill in the contents of a SHT_GROUP section.  Called from
   _bfd_elf_compute_section_file_positions for gas, objcopy, and
   when ELF targets use the generic linker, ld.  Called for ld -r
   from bfd_elf_final_link.  */

void
bfd_elf_set_group_contents (bfd *abfd, asection *sec, void *failedptrarg)
{
  bool *failedptr = (bool *) failedptrarg;
  asection *elt, *first;
  unsigned char *loc;
  bool gas;

  /* Ignore linker created group section.  See elfNN_ia64_object_p in
     elfxx-ia64.c.  */
  if ((sec->flags & (SEC_GROUP | SEC_LINKER_CREATED)) != SEC_GROUP
      || sec->size == 0
      || *failedptr)
    return;

  if (elf_section_data (sec)->this_hdr.sh_info == 0)
    {
      unsigned long symindx = 0;

      /* elf_group_id will have been set up by objcopy and the
	 generic linker.  */
      if (elf_group_id (sec) != NULL)
	symindx = elf_group_id (sec)->udata.i;

      if (symindx == 0)
	{
	  /* If called from the assembler, swap_out_syms will have set up
	     elf_section_syms.
	     PR 25699: A corrupt input file could contain bogus group info.  */
	  if (sec->index >= elf_num_section_syms (abfd)
	      || elf_section_syms (abfd)[sec->index] == NULL)
	    {
	      *failedptr = true;
	      return;
	    }
	  symindx = elf_section_syms (abfd)[sec->index]->udata.i;
	}
      elf_section_data (sec)->this_hdr.sh_info = symindx;
    }
  else if (elf_section_data (sec)->this_hdr.sh_info == (unsigned int) -2)
    {
      /* The ELF backend linker sets sh_info to -2 when the group
	 signature symbol is global, and thus the index can't be
	 set until all local symbols are output.  */
      asection *igroup;
      struct bfd_elf_section_data *sec_data;
      unsigned long symndx;
      unsigned long extsymoff;
      struct elf_link_hash_entry *h;

      /* The point of this little dance to the first SHF_GROUP section
	 then back to the SHT_GROUP section is that this gets us to
	 the SHT_GROUP in the input object.  */
      igroup = elf_sec_group (elf_next_in_group (sec));
      sec_data = elf_section_data (igroup);
      symndx = sec_data->this_hdr.sh_info;
      extsymoff = 0;
      if (!elf_bad_symtab (igroup->owner))
	{
	  Elf_Internal_Shdr *symtab_hdr;

	  symtab_hdr = &elf_tdata (igroup->owner)->symtab_hdr;
	  extsymoff = symtab_hdr->sh_info;
	}
      h = elf_sym_hashes (igroup->owner)[symndx - extsymoff];
      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      elf_section_data (sec)->this_hdr.sh_info = h->indx;
    }

  /* The contents won't be allocated for "ld -r" or objcopy.  */
  gas = true;
  if (sec->contents == NULL)
    {
      gas = false;
      sec->contents = (unsigned char *) bfd_alloc (abfd, sec->size);

      /* Arrange for the section to be written out.  */
      elf_section_data (sec)->this_hdr.contents = sec->contents;
      if (sec->contents == NULL)
	{
	  *failedptr = true;
	  return;
	}
    }

  loc = sec->contents + sec->size;

  /* Get the pointer to the first section in the group that gas
     squirreled away here.  objcopy arranges for this to be set to the
     start of the input section group.  */
  first = elt = elf_next_in_group (sec);

  /* First element is a flag word.  Rest of section is elf section
     indices for all the sections of the group.  Write them backwards
     just to keep the group in the same order as given in .section
     directives, not that it matters.  */
  while (elt != NULL)
    {
      asection *s;

      s = elt;
      if (!gas)
	s = s->output_section;
      if (s != NULL
	  && !bfd_is_abs_section (s))
	{
	  struct bfd_elf_section_data *elf_sec = elf_section_data (s);
	  struct bfd_elf_section_data *input_elf_sec = elf_section_data (elt);

	  if (elf_sec->rel.hdr != NULL
	      && (gas
		  || (input_elf_sec->rel.hdr != NULL
		      && input_elf_sec->rel.hdr->sh_flags & SHF_GROUP) != 0))
	    {
	      elf_sec->rel.hdr->sh_flags |= SHF_GROUP;
	      loc -= 4;
	      if (loc == sec->contents)
		break;
	      H_PUT_32 (abfd, elf_sec->rel.idx, loc);
	    }
	  if (elf_sec->rela.hdr != NULL
	      && (gas
		  || (input_elf_sec->rela.hdr != NULL
		      && input_elf_sec->rela.hdr->sh_flags & SHF_GROUP) != 0))
	    {
	      elf_sec->rela.hdr->sh_flags |= SHF_GROUP;
	      loc -= 4;
	      if (loc == sec->contents)
		break;
	      H_PUT_32 (abfd, elf_sec->rela.idx, loc);
	    }
	  loc -= 4;
	  if (loc == sec->contents)
	    break;
	  H_PUT_32 (abfd, elf_sec->this_idx, loc);
	}
      elt = elf_next_in_group (elt);
      if (elt == first)
	break;
    }

  /* We should always get here with loc == sec->contents + 4, but it is
     possible to craft bogus SHT_GROUP sections that will cause segfaults
     in objcopy without checking loc here and in the loop above.  */
  if (loc == sec->contents)
    BFD_ASSERT (0);
  else
    {
      loc -= 4;
      if (loc != sec->contents)
	{
	  BFD_ASSERT (0);
	  memset (sec->contents + 4, 0, loc - sec->contents);
	  loc = sec->contents;
	}
    }

  H_PUT_32 (abfd, sec->flags & SEC_LINK_ONCE ? GRP_COMDAT : 0, loc);
}

/* Given NAME, the name of a relocation section stripped of its
   .rel/.rela prefix, return the section in ABFD to which the
   relocations apply.  */

asection *
_bfd_elf_plt_get_reloc_section (bfd *abfd, const char *name)
{
  /* If a target needs .got.plt section, relocations in rela.plt/rel.plt
     section likely apply to .got.plt or .got section.  */
  if (get_elf_backend_data (abfd)->want_got_plt
      && strcmp (name, ".plt") == 0)
    {
      asection *sec;

      name = ".got.plt";
      sec = bfd_get_section_by_name (abfd, name);
      if (sec != NULL)
	return sec;
      name = ".got";
    }

  return bfd_get_section_by_name (abfd, name);
}

/* Return the section to which RELOC_SEC applies.  */

static asection *
elf_get_reloc_section (asection *reloc_sec)
{
  const char *name;
  unsigned int type;
  bfd *abfd;
  const struct elf_backend_data *bed;

  type = elf_section_data (reloc_sec)->this_hdr.sh_type;
  if (type != SHT_REL && type != SHT_RELA)
    return NULL;

  /* We look up the section the relocs apply to by name.  */
  name = reloc_sec->name;
  if (!startswith (name, ".rel"))
    return NULL;
  name += 4;
  if (type == SHT_RELA && *name++ != 'a')
    return NULL;

  abfd = reloc_sec->owner;
  bed = get_elf_backend_data (abfd);
  return bed->get_reloc_section (abfd, name);
}

/* Assign all ELF section numbers.  The dummy first section is handled here
   too.  The link/info pointers for the standard section types are filled
   in here too, while we're at it.  LINK_INFO will be 0 when arriving
   here for gas, objcopy, and when using the generic ELF linker.  */

static bool
assign_section_numbers (bfd *abfd, struct bfd_link_info *link_info)
{
  struct elf_obj_tdata *t = elf_tdata (abfd);
  asection *sec;
  unsigned int section_number;
  Elf_Internal_Shdr **i_shdrp;
  struct bfd_elf_section_data *d;
  bool need_symtab;
  size_t amt;

  section_number = 1;

  _bfd_elf_strtab_clear_all_refs (elf_shstrtab (abfd));

  /* SHT_GROUP sections are in relocatable files only.  */
  if (link_info == NULL || !link_info->resolve_section_groups)
    {
      size_t reloc_count = 0;

      /* Put SHT_GROUP sections first.  */
      for (sec = abfd->sections; sec != NULL; sec = sec->next)
	{
	  d = elf_section_data (sec);

	  if (d->this_hdr.sh_type == SHT_GROUP)
	    {
	      if (sec->flags & SEC_LINKER_CREATED)
		{
		  /* Remove the linker created SHT_GROUP sections.  */
		  bfd_section_list_remove (abfd, sec);
		  abfd->section_count--;
		}
	      else
		d->this_idx = section_number++;
	    }

	  /* Count relocations.  */
	  reloc_count += sec->reloc_count;
	}

      /* Set/clear HAS_RELOC depending on whether there are relocations.  */
      if (reloc_count == 0)
	abfd->flags &= ~HAS_RELOC;
      else
	abfd->flags |= HAS_RELOC;
    }

  for (sec = abfd->sections; sec; sec = sec->next)
    {
      d = elf_section_data (sec);

      if (d->this_hdr.sh_type != SHT_GROUP)
	d->this_idx = section_number++;
      if (d->this_hdr.sh_name != (unsigned int) -1)
	_bfd_elf_strtab_addref (elf_shstrtab (abfd), d->this_hdr.sh_name);
      if (d->rel.hdr)
	{
	  d->rel.idx = section_number++;
	  if (d->rel.hdr->sh_name != (unsigned int) -1)
	    _bfd_elf_strtab_addref (elf_shstrtab (abfd), d->rel.hdr->sh_name);
	}
      else
	d->rel.idx = 0;

      if (d->rela.hdr)
	{
	  d->rela.idx = section_number++;
	  if (d->rela.hdr->sh_name != (unsigned int) -1)
	    _bfd_elf_strtab_addref (elf_shstrtab (abfd), d->rela.hdr->sh_name);
	}
      else
	d->rela.idx = 0;
    }

  need_symtab = (bfd_get_symcount (abfd) > 0
		 || (link_info == NULL
		     && ((abfd->flags & (EXEC_P | DYNAMIC | HAS_RELOC))
			 == HAS_RELOC)));
  if (need_symtab)
    {
      elf_onesymtab (abfd) = section_number++;
      _bfd_elf_strtab_addref (elf_shstrtab (abfd), t->symtab_hdr.sh_name);
      if (section_number > ((SHN_LORESERVE - 2) & 0xFFFF))
	{
	  elf_section_list *entry;

	  BFD_ASSERT (elf_symtab_shndx_list (abfd) == NULL);

	  entry = bfd_zalloc (abfd, sizeof (*entry));
	  entry->ndx = section_number++;
	  elf_symtab_shndx_list (abfd) = entry;
	  entry->hdr.sh_name
	    = (unsigned int) _bfd_elf_strtab_add (elf_shstrtab (abfd),
						  ".symtab_shndx", false);
	  if (entry->hdr.sh_name == (unsigned int) -1)
	    return false;
	}
      elf_strtab_sec (abfd) = section_number++;
      _bfd_elf_strtab_addref (elf_shstrtab (abfd), t->strtab_hdr.sh_name);
    }

  elf_shstrtab_sec (abfd) = section_number++;
  _bfd_elf_strtab_addref (elf_shstrtab (abfd), t->shstrtab_hdr.sh_name);
  elf_elfheader (abfd)->e_shstrndx = elf_shstrtab_sec (abfd);

  if (section_number >= SHN_LORESERVE)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: too many sections: %u"),
			  abfd, section_number);
      return false;
    }

  elf_numsections (abfd) = section_number;
  elf_elfheader (abfd)->e_shnum = section_number;

  /* Set up the list of section header pointers, in agreement with the
     indices.  */
  amt = section_number * sizeof (Elf_Internal_Shdr *);
  i_shdrp = (Elf_Internal_Shdr **) bfd_zalloc (abfd, amt);
  if (i_shdrp == NULL)
    return false;

  i_shdrp[0] = (Elf_Internal_Shdr *) bfd_zalloc (abfd,
						 sizeof (Elf_Internal_Shdr));
  if (i_shdrp[0] == NULL)
    {
      bfd_release (abfd, i_shdrp);
      return false;
    }

  elf_elfsections (abfd) = i_shdrp;

  i_shdrp[elf_shstrtab_sec (abfd)] = &t->shstrtab_hdr;
  if (need_symtab)
    {
      i_shdrp[elf_onesymtab (abfd)] = &t->symtab_hdr;
      if (elf_numsections (abfd) > (SHN_LORESERVE & 0xFFFF))
	{
	  elf_section_list * entry = elf_symtab_shndx_list (abfd);
	  BFD_ASSERT (entry != NULL);
	  i_shdrp[entry->ndx] = & entry->hdr;
	  entry->hdr.sh_link = elf_onesymtab (abfd);
	}
      i_shdrp[elf_strtab_sec (abfd)] = &t->strtab_hdr;
      t->symtab_hdr.sh_link = elf_strtab_sec (abfd);
    }

  for (sec = abfd->sections; sec; sec = sec->next)
    {
      asection *s;

      d = elf_section_data (sec);

      i_shdrp[d->this_idx] = &d->this_hdr;
      if (d->rel.idx != 0)
	i_shdrp[d->rel.idx] = d->rel.hdr;
      if (d->rela.idx != 0)
	i_shdrp[d->rela.idx] = d->rela.hdr;

      /* Fill in the sh_link and sh_info fields while we're at it.  */

      /* sh_link of a reloc section is the section index of the symbol
	 table.  sh_info is the section index of the section to which
	 the relocation entries apply.  */
      if (d->rel.idx != 0)
	{
	  d->rel.hdr->sh_link = elf_onesymtab (abfd);
	  d->rel.hdr->sh_info = d->this_idx;
	  d->rel.hdr->sh_flags |= SHF_INFO_LINK;
	}
      if (d->rela.idx != 0)
	{
	  d->rela.hdr->sh_link = elf_onesymtab (abfd);
	  d->rela.hdr->sh_info = d->this_idx;
	  d->rela.hdr->sh_flags |= SHF_INFO_LINK;
	}

      /* We need to set up sh_link for SHF_LINK_ORDER.  */
      if ((d->this_hdr.sh_flags & SHF_LINK_ORDER) != 0)
	{
	  s = elf_linked_to_section (sec);
	  /* We can now have a NULL linked section pointer.
	     This happens when the sh_link field is 0, which is done
	     when a linked to section is discarded but the linking
	     section has been retained for some reason.  */
	  if (s)
	    {
	      /* Check discarded linkonce section.  */
	      if (discarded_section (s))
		{
		  asection *kept;
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: sh_link of section `%pA' points to"
		       " discarded section `%pA' of `%pB'"),
		     abfd, d->this_hdr.bfd_section, s, s->owner);
		  /* Point to the kept section if it has the same
		     size as the discarded one.  */
		  kept = _bfd_elf_check_kept_section (s, link_info);
		  if (kept == NULL)
		    {
		      bfd_set_error (bfd_error_bad_value);
		      return false;
		    }
		  s = kept;
		}
	      /* Handle objcopy. */
	      else if (s->output_section == NULL)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: sh_link of section `%pA' points to"
		       " removed section `%pA' of `%pB'"),
		     abfd, d->this_hdr.bfd_section, s, s->owner);
		  bfd_set_error (bfd_error_bad_value);
		  return false;
		}
	      s = s->output_section;
	      d->this_hdr.sh_link = elf_section_data (s)->this_idx;
	    }
	}

      switch (d->this_hdr.sh_type)
	{
	case SHT_REL:
	case SHT_RELA:
	  /* sh_link is the section index of the symbol table.
	     sh_info is the section index of the section to which the
	     relocation entries apply.  */
	  if (d->this_hdr.sh_link == 0)
	    {
	      /* FIXME maybe: If this is a reloc section which we are
		 treating as a normal section then we likely should
		 not be assuming its sh_link is .dynsym or .symtab.  */
	      if ((sec->flags & SEC_ALLOC) != 0)
		{
		  s = bfd_get_section_by_name (abfd, ".dynsym");
		  if (s != NULL)
		    d->this_hdr.sh_link = elf_section_data (s)->this_idx;
		}
	      else
		d->this_hdr.sh_link = elf_onesymtab (abfd);
	    }

	  s = elf_get_reloc_section (sec);
	  if (s != NULL)
	    {
	      d->this_hdr.sh_info = elf_section_data (s)->this_idx;
	      d->this_hdr.sh_flags |= SHF_INFO_LINK;
	    }
	  break;

	case SHT_STRTAB:
	  /* We assume that a section named .stab*str is a stabs
	     string section.  We look for a section with the same name
	     but without the trailing ``str'', and set its sh_link
	     field to point to this section.  */
	  if (startswith (sec->name, ".stab")
	      && strcmp (sec->name + strlen (sec->name) - 3, "str") == 0)
	    {
	      size_t len;
	      char *alc;

	      len = strlen (sec->name);
	      alc = (char *) bfd_malloc (len - 2);
	      if (alc == NULL)
		return false;
	      memcpy (alc, sec->name, len - 3);
	      alc[len - 3] = '\0';
	      s = bfd_get_section_by_name (abfd, alc);
	      free (alc);
	      if (s != NULL)
		{
		  elf_section_data (s)->this_hdr.sh_link = d->this_idx;

		  /* This is a .stab section.  */
		  elf_section_data (s)->this_hdr.sh_entsize = 12;
		}
	    }
	  break;

	case SHT_DYNAMIC:
	case SHT_DYNSYM:
	case SHT_GNU_verneed:
	case SHT_GNU_verdef:
	  /* sh_link is the section header index of the string table
	     used for the dynamic entries, or the symbol table, or the
	     version strings.  */
	  s = bfd_get_section_by_name (abfd, ".dynstr");
	  if (s != NULL)
	    d->this_hdr.sh_link = elf_section_data (s)->this_idx;
	  break;

	case SHT_GNU_LIBLIST:
	  /* sh_link is the section header index of the prelink library
	     list used for the dynamic entries, or the symbol table, or
	     the version strings.  */
	  s = bfd_get_section_by_name (abfd, ((sec->flags & SEC_ALLOC)
					      ? ".dynstr" : ".gnu.libstr"));
	  if (s != NULL)
	    d->this_hdr.sh_link = elf_section_data (s)->this_idx;
	  break;

	case SHT_HASH:
	case SHT_GNU_HASH:
	case SHT_GNU_versym:
	  /* sh_link is the section header index of the symbol table
	     this hash table or version table is for.  */
	  s = bfd_get_section_by_name (abfd, ".dynsym");
	  if (s != NULL)
	    d->this_hdr.sh_link = elf_section_data (s)->this_idx;
	  break;

	case SHT_GROUP:
	  d->this_hdr.sh_link = elf_onesymtab (abfd);
	}
    }

  /* Delay setting sh_name to _bfd_elf_write_object_contents so that
     _bfd_elf_assign_file_positions_for_non_load can convert DWARF
     debug section name from .debug_* to .zdebug_* if needed.  */

  return true;
}

static bool
sym_is_global (bfd *abfd, asymbol *sym)
{
  /* If the backend has a special mapping, use it.  */
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_sym_is_global)
    return (*bed->elf_backend_sym_is_global) (abfd, sym);

  return ((sym->flags & (BSF_GLOBAL | BSF_WEAK | BSF_GNU_UNIQUE)) != 0
	  || bfd_is_und_section (bfd_asymbol_section (sym))
	  || bfd_is_com_section (bfd_asymbol_section (sym)));
}

/* Filter global symbols of ABFD to include in the import library.  All
   SYMCOUNT symbols of ABFD can be examined from their pointers in
   SYMS.  Pointers of symbols to keep should be stored contiguously at
   the beginning of that array.

   Returns the number of symbols to keep.  */

unsigned int
_bfd_elf_filter_global_symbols (bfd *abfd, struct bfd_link_info *info,
				asymbol **syms, long symcount)
{
  long src_count, dst_count = 0;

  for (src_count = 0; src_count < symcount; src_count++)
    {
      asymbol *sym = syms[src_count];
      char *name = (char *) bfd_asymbol_name (sym);
      struct bfd_link_hash_entry *h;

      if (!sym_is_global (abfd, sym))
	continue;

      h = bfd_link_hash_lookup (info->hash, name, false, false, false);
      if (h == NULL)
	continue;
      if (h->type != bfd_link_hash_defined && h->type != bfd_link_hash_defweak)
	continue;
      if (h->linker_def || h->ldscript_def)
	continue;

      syms[dst_count++] = sym;
    }

  syms[dst_count] = NULL;

  return dst_count;
}

/* Don't output section symbols for sections that are not going to be
   output, that are duplicates or there is no BFD section.  */

static bool
ignore_section_sym (bfd *abfd, asymbol *sym)
{
  elf_symbol_type *type_ptr;

  if (sym == NULL)
    return false;

  if ((sym->flags & BSF_SECTION_SYM) == 0)
    return false;

  /* Ignore the section symbol if it isn't used.  */
  if ((sym->flags & BSF_SECTION_SYM_USED) == 0)
    return true;

  if (sym->section == NULL)
    return true;

  type_ptr = elf_symbol_from (sym);
  return ((type_ptr != NULL
	   && type_ptr->internal_elf_sym.st_shndx != 0
	   && bfd_is_abs_section (sym->section))
	  || !(sym->section->owner == abfd
	       || (sym->section->output_section != NULL
		   && sym->section->output_section->owner == abfd
		   && sym->section->output_offset == 0)
	       || bfd_is_abs_section (sym->section)));
}

/* Map symbol from it's internal number to the external number, moving
   all local symbols to be at the head of the list.  */

static bool
elf_map_symbols (bfd *abfd, unsigned int *pnum_locals)
{
  unsigned int symcount = bfd_get_symcount (abfd);
  asymbol **syms = bfd_get_outsymbols (abfd);
  asymbol **sect_syms;
  unsigned int num_locals = 0;
  unsigned int num_globals = 0;
  unsigned int num_locals2 = 0;
  unsigned int num_globals2 = 0;
  unsigned int max_index = 0;
  unsigned int idx;
  asection *asect;
  asymbol **new_syms;
  size_t amt;

#ifdef DEBUG
  fprintf (stderr, "elf_map_symbols\n");
  fflush (stderr);
#endif

  for (asect = abfd->sections; asect; asect = asect->next)
    {
      if (max_index < asect->index)
	max_index = asect->index;
    }

  max_index++;
  amt = max_index * sizeof (asymbol *);
  sect_syms = (asymbol **) bfd_zalloc (abfd, amt);
  if (sect_syms == NULL)
    return false;
  elf_section_syms (abfd) = sect_syms;
  elf_num_section_syms (abfd) = max_index;

  /* Init sect_syms entries for any section symbols we have already
     decided to output.  */
  for (idx = 0; idx < symcount; idx++)
    {
      asymbol *sym = syms[idx];

      if ((sym->flags & BSF_SECTION_SYM) != 0
	  && sym->value == 0
	  && !ignore_section_sym (abfd, sym)
	  && !bfd_is_abs_section (sym->section))
	{
	  asection *sec = sym->section;

	  if (sec->owner != abfd)
	    sec = sec->output_section;

	  sect_syms[sec->index] = syms[idx];
	}
    }

  /* Classify all of the symbols.  */
  for (idx = 0; idx < symcount; idx++)
    {
      if (sym_is_global (abfd, syms[idx]))
	num_globals++;
      else if (!ignore_section_sym (abfd, syms[idx]))
	num_locals++;
    }

  /* We will be adding a section symbol for each normal BFD section.  Most
     sections will already have a section symbol in outsymbols, but
     eg. SHT_GROUP sections will not, and we need the section symbol mapped
     at least in that case.  */
  for (asect = abfd->sections; asect; asect = asect->next)
    {
      asymbol *sym = asect->symbol;
      /* Don't include ignored section symbols.  */
      if (!ignore_section_sym (abfd, sym)
	  && sect_syms[asect->index] == NULL)
	{
	  if (!sym_is_global (abfd, asect->symbol))
	    num_locals++;
	  else
	    num_globals++;
	}
    }

  /* Now sort the symbols so the local symbols are first.  */
  amt = (num_locals + num_globals) * sizeof (asymbol *);
  new_syms = (asymbol **) bfd_alloc (abfd, amt);
  if (new_syms == NULL)
    return false;

  for (idx = 0; idx < symcount; idx++)
    {
      asymbol *sym = syms[idx];
      unsigned int i;

      if (sym_is_global (abfd, sym))
	i = num_locals + num_globals2++;
      /* Don't include ignored section symbols.  */
      else if (!ignore_section_sym (abfd, sym))
	i = num_locals2++;
      else
	continue;
      new_syms[i] = sym;
      sym->udata.i = i + 1;
    }
  for (asect = abfd->sections; asect; asect = asect->next)
    {
      asymbol *sym = asect->symbol;
      if (!ignore_section_sym (abfd, sym)
	  && sect_syms[asect->index] == NULL)
	{
	  unsigned int i;

	  sect_syms[asect->index] = sym;
	  if (!sym_is_global (abfd, sym))
	    i = num_locals2++;
	  else
	    i = num_locals + num_globals2++;
	  new_syms[i] = sym;
	  sym->udata.i = i + 1;
	}
    }

  bfd_set_symtab (abfd, new_syms, num_locals + num_globals);

  *pnum_locals = num_locals;
  return true;
}

/* Align to the maximum file alignment that could be required for any
   ELF data structure.  */

static inline file_ptr
align_file_position (file_ptr off, int align)
{
  return (off + align - 1) & ~(align - 1);
}

/* Assign a file position to a section, optionally aligning to the
   required section alignment.  */

file_ptr
_bfd_elf_assign_file_position_for_section (Elf_Internal_Shdr *i_shdrp,
					   file_ptr offset,
					   bool align)
{
  if (align && i_shdrp->sh_addralign > 1)
    offset = BFD_ALIGN (offset, i_shdrp->sh_addralign & -i_shdrp->sh_addralign);
  i_shdrp->sh_offset = offset;
  if (i_shdrp->bfd_section != NULL)
    i_shdrp->bfd_section->filepos = offset;
  if (i_shdrp->sh_type != SHT_NOBITS)
    offset += i_shdrp->sh_size;
  return offset;
}

/* Compute the file positions we are going to put the sections at, and
   otherwise prepare to begin writing out the ELF file.  If LINK_INFO
   is not NULL, this is being called by the ELF backend linker.  */

bool
_bfd_elf_compute_section_file_positions (bfd *abfd,
					 struct bfd_link_info *link_info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct fake_section_arg fsargs;
  bool failed;
  struct elf_strtab_hash *strtab = NULL;
  Elf_Internal_Shdr *shstrtab_hdr;
  bool need_symtab;

  if (abfd->output_has_begun)
    return true;

  /* Do any elf backend specific processing first.  */
  if (bed->elf_backend_begin_write_processing)
    (*bed->elf_backend_begin_write_processing) (abfd, link_info);

  if (!(*bed->elf_backend_init_file_header) (abfd, link_info))
    return false;

  fsargs.failed = false;
  fsargs.link_info = link_info;
  bfd_map_over_sections (abfd, elf_fake_sections, &fsargs);
  if (fsargs.failed)
    return false;

  if (!assign_section_numbers (abfd, link_info))
    return false;

  /* The backend linker builds symbol table information itself.  */
  need_symtab = (link_info == NULL
		 && (bfd_get_symcount (abfd) > 0
		     || ((abfd->flags & (EXEC_P | DYNAMIC | HAS_RELOC))
			 == HAS_RELOC)));
  if (need_symtab)
    {
      /* Non-zero if doing a relocatable link.  */
      int relocatable_p = ! (abfd->flags & (EXEC_P | DYNAMIC));

      if (! swap_out_syms (abfd, &strtab, relocatable_p, link_info))
	return false;
    }

  failed = false;
  if (link_info == NULL)
    {
      bfd_map_over_sections (abfd, bfd_elf_set_group_contents, &failed);
      if (failed)
	goto err_free_strtab;
    }

  shstrtab_hdr = &elf_tdata (abfd)->shstrtab_hdr;
  /* sh_name was set in init_file_header.  */
  shstrtab_hdr->sh_type = SHT_STRTAB;
  shstrtab_hdr->sh_flags = bed->elf_strtab_flags;
  shstrtab_hdr->sh_addr = 0;
  /* sh_size is set in _bfd_elf_assign_file_positions_for_non_load.  */
  shstrtab_hdr->sh_entsize = 0;
  shstrtab_hdr->sh_link = 0;
  shstrtab_hdr->sh_info = 0;
  /* sh_offset is set in _bfd_elf_assign_file_positions_for_non_load.  */
  shstrtab_hdr->sh_addralign = 1;

  if (!assign_file_positions_except_relocs (abfd, link_info))
    goto err_free_strtab;

  if (strtab != NULL)
    {
      file_ptr off;
      Elf_Internal_Shdr *hdr;

      off = elf_next_file_pos (abfd);

      hdr = & elf_symtab_hdr (abfd);
      off = _bfd_elf_assign_file_position_for_section (hdr, off, true);

      if (elf_symtab_shndx_list (abfd) != NULL)
	{
	  hdr = & elf_symtab_shndx_list (abfd)->hdr;
	  if (hdr->sh_size != 0)
	    off = _bfd_elf_assign_file_position_for_section (hdr, off, true);
	  /* FIXME: What about other symtab_shndx sections in the list ?  */
	}

      hdr = &elf_tdata (abfd)->strtab_hdr;
      off = _bfd_elf_assign_file_position_for_section (hdr, off, true);

      elf_next_file_pos (abfd) = off;

      /* Now that we know where the .strtab section goes, write it
	 out.  */
      if (bfd_seek (abfd, hdr->sh_offset, SEEK_SET) != 0
	  || ! _bfd_elf_strtab_emit (abfd, strtab))
	goto err_free_strtab;
      _bfd_elf_strtab_free (strtab);
    }

  abfd->output_has_begun = true;
  return true;

 err_free_strtab:
  if (strtab != NULL)
    _bfd_elf_strtab_free (strtab);
  return false;
}

/* Retrieve .eh_frame_hdr.  Prior to size_dynamic_sections the
   function effectively returns whether --eh-frame-hdr is given on the
   command line.  After size_dynamic_sections the result reflects
   whether .eh_frame_hdr will actually be output (sizing isn't done
   until ldemul_after_allocation).  */

static asection *
elf_eh_frame_hdr (const struct bfd_link_info *info)
{
  if (info != NULL && is_elf_hash_table (info->hash))
    return elf_hash_table (info)->eh_info.hdr_sec;
  return NULL;
}

/* Make an initial estimate of the size of the program header.  If we
   get the number wrong here, we'll redo section placement.  */

static bfd_size_type
get_program_header_size (bfd *abfd, struct bfd_link_info *info)
{
  size_t segs;
  asection *s;
  const struct elf_backend_data *bed;

  /* Assume we will need exactly two PT_LOAD segments: one for text
     and one for data.  */
  segs = 2;

  s = bfd_get_section_by_name (abfd, ".interp");
  if (s != NULL && (s->flags & SEC_LOAD) != 0 && s->size != 0)
    {
      /* If we have a loadable interpreter section, we need a
	 PT_INTERP segment.  In this case, assume we also need a
	 PT_PHDR segment, although that may not be true for all
	 targets.  */
      segs += 2;
    }

  if (bfd_get_section_by_name (abfd, ".dynamic") != NULL)
    {
      /* We need a PT_DYNAMIC segment.  */
      ++segs;
    }

  if (info != NULL && info->relro)
    {
      /* We need a PT_GNU_RELRO segment.  */
      ++segs;
    }

  if (elf_eh_frame_hdr (info))
    {
      /* We need a PT_GNU_EH_FRAME segment.  */
      ++segs;
    }

  if (elf_stack_flags (abfd))
    {
      /* We need a PT_GNU_STACK segment.  */
      ++segs;
    }

  if (elf_sframe (abfd))
    {
      /* We need a PT_GNU_SFRAME segment.  */
      ++segs;
    }

  s = bfd_get_section_by_name (abfd,
			       NOTE_GNU_PROPERTY_SECTION_NAME);
  if (s != NULL && s->size != 0)
    {
      /* We need a PT_GNU_PROPERTY segment.  */
      ++segs;
    }

  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LOAD) != 0
	  && elf_section_type (s) == SHT_NOTE)
	{
	  unsigned int alignment_power;
	  /* We need a PT_NOTE segment.  */
	  ++segs;
	  /* Try to create just one PT_NOTE segment for all adjacent
	     loadable SHT_NOTE sections.  gABI requires that within a
	     PT_NOTE segment (and also inside of each SHT_NOTE section)
	     each note should have the same alignment.  So we check
	     whether the sections are correctly aligned.  */
	  alignment_power = s->alignment_power;
	  while (s->next != NULL
		 && s->next->alignment_power == alignment_power
		 && (s->next->flags & SEC_LOAD) != 0
		 && elf_section_type (s->next) == SHT_NOTE)
	    s = s->next;
	}
    }

  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if (s->flags & SEC_THREAD_LOCAL)
	{
	  /* We need a PT_TLS segment.  */
	  ++segs;
	  break;
	}
    }

  bed = get_elf_backend_data (abfd);

  if ((abfd->flags & D_PAGED) != 0
      && (elf_tdata (abfd)->has_gnu_osabi & elf_gnu_osabi_mbind) != 0)
    {
      /* Add a PT_GNU_MBIND segment for each mbind section.  */
      bfd_vma commonpagesize;
      unsigned int page_align_power;

      if (info != NULL)
	commonpagesize = info->commonpagesize;
      else
	commonpagesize = bed->commonpagesize;
      page_align_power = bfd_log2 (commonpagesize);
      for (s = abfd->sections; s != NULL; s = s->next)
	if (elf_section_flags (s) & SHF_GNU_MBIND)
	  {
	    if (elf_section_data (s)->this_hdr.sh_info > PT_GNU_MBIND_NUM)
	      {
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("%pB: GNU_MBIND section `%pA' has invalid "
		     "sh_info field: %d"),
		   abfd, s, elf_section_data (s)->this_hdr.sh_info);
		continue;
	      }
	    /* Align mbind section to page size.  */
	    if (s->alignment_power < page_align_power)
	      s->alignment_power = page_align_power;
	    segs ++;
	  }
    }

  /* Let the backend count up any program headers it might need.  */
  if (bed->elf_backend_additional_program_headers)
    {
      int a;

      a = (*bed->elf_backend_additional_program_headers) (abfd, info);
      if (a == -1)
	abort ();
      segs += a;
    }

  return segs * bed->s->sizeof_phdr;
}

/* Find the segment that contains the output_section of section.  */

Elf_Internal_Phdr *
_bfd_elf_find_segment_containing_section (bfd * abfd, asection * section)
{
  struct elf_segment_map *m;
  Elf_Internal_Phdr *p;

  for (m = elf_seg_map (abfd), p = elf_tdata (abfd)->phdr;
       m != NULL;
       m = m->next, p++)
    {
      int i;

      for (i = m->count - 1; i >= 0; i--)
	if (m->sections[i] == section)
	  return p;
    }

  return NULL;
}

/* Create a mapping from a set of sections to a program segment.  */

static struct elf_segment_map *
make_mapping (bfd *abfd,
	      asection **sections,
	      unsigned int from,
	      unsigned int to,
	      bool phdr)
{
  struct elf_segment_map *m;
  unsigned int i;
  asection **hdrpp;
  size_t amt;

  amt = sizeof (struct elf_segment_map) - sizeof (asection *);
  amt += (to - from) * sizeof (asection *);
  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
  if (m == NULL)
    return NULL;
  m->next = NULL;
  m->p_type = PT_LOAD;
  for (i = from, hdrpp = sections + from; i < to; i++, hdrpp++)
    m->sections[i - from] = *hdrpp;
  m->count = to - from;

  if (from == 0 && phdr)
    {
      /* Include the headers in the first PT_LOAD segment.  */
      m->includes_filehdr = 1;
      m->includes_phdrs = 1;
    }

  return m;
}

/* Create the PT_DYNAMIC segment, which includes DYNSEC.  Returns NULL
   on failure.  */

struct elf_segment_map *
_bfd_elf_make_dynamic_segment (bfd *abfd, asection *dynsec)
{
  struct elf_segment_map *m;

  m = (struct elf_segment_map *) bfd_zalloc (abfd,
					     sizeof (struct elf_segment_map));
  if (m == NULL)
    return NULL;
  m->next = NULL;
  m->p_type = PT_DYNAMIC;
  m->count = 1;
  m->sections[0] = dynsec;

  return m;
}

/* Possibly add or remove segments from the segment map.  */

static bool
elf_modify_segment_map (bfd *abfd,
			struct bfd_link_info *info,
			bool remove_empty_load)
{
  struct elf_segment_map **m;
  const struct elf_backend_data *bed;

  /* The placement algorithm assumes that non allocated sections are
     not in PT_LOAD segments.  We ensure this here by removing such
     sections from the segment map.  We also remove excluded
     sections.  Finally, any PT_LOAD segment without sections is
     removed.  */
  m = &elf_seg_map (abfd);
  while (*m)
    {
      unsigned int i, new_count;

      for (new_count = 0, i = 0; i < (*m)->count; i++)
	{
	  if (((*m)->sections[i]->flags & SEC_EXCLUDE) == 0
	      && (((*m)->sections[i]->flags & SEC_ALLOC) != 0
		  || (*m)->p_type != PT_LOAD))
	    {
	      (*m)->sections[new_count] = (*m)->sections[i];
	      new_count++;
	    }
	}
      (*m)->count = new_count;

      if (remove_empty_load
	  && (*m)->p_type == PT_LOAD
	  && (*m)->count == 0
	  && !(*m)->includes_phdrs)
	*m = (*m)->next;
      else
	m = &(*m)->next;
    }

  bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_modify_segment_map != NULL)
    {
      if (!(*bed->elf_backend_modify_segment_map) (abfd, info))
	return false;
    }

  return true;
}

#define IS_TBSS(s) \
  ((s->flags & (SEC_THREAD_LOCAL | SEC_LOAD)) == SEC_THREAD_LOCAL)

/* Set up a mapping from BFD sections to program segments.  Update
   NEED_LAYOUT if the section layout is changed.  */

bool
_bfd_elf_map_sections_to_segments (bfd *abfd,
				   struct bfd_link_info *info,
				   bool *need_layout)
{
  unsigned int count;
  struct elf_segment_map *m;
  asection **sections = NULL;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  bool no_user_phdrs;

  no_user_phdrs = elf_seg_map (abfd) == NULL;

  if (info != NULL)
    {
      info->user_phdrs = !no_user_phdrs;

      /* Size the relative relocations if DT_RELR is enabled.  */
      if (info->enable_dt_relr
	  && need_layout != NULL
	  && bed->size_relative_relocs
	  && !bed->size_relative_relocs (info, need_layout))
	info->callbacks->einfo
	  (_("%F%P: failed to size relative relocations\n"));
    }

  if (no_user_phdrs && bfd_count_sections (abfd) != 0)
    {
      asection *s;
      unsigned int i;
      struct elf_segment_map *mfirst;
      struct elf_segment_map **pm;
      asection *last_hdr;
      bfd_vma last_size;
      unsigned int hdr_index;
      bfd_vma maxpagesize;
      asection **hdrpp;
      bool phdr_in_segment;
      bool writable;
      bool executable;
      unsigned int tls_count = 0;
      asection *first_tls = NULL;
      asection *first_mbind = NULL;
      asection *dynsec, *eh_frame_hdr;
      asection *sframe;
      size_t amt;
      bfd_vma addr_mask, wrap_to = 0;  /* Bytes.  */
      bfd_size_type phdr_size;  /* Octets/bytes.  */
      unsigned int opb = bfd_octets_per_byte (abfd, NULL);

      /* Select the allocated sections, and sort them.  */

      amt = bfd_count_sections (abfd) * sizeof (asection *);
      sections = (asection **) bfd_malloc (amt);
      if (sections == NULL)
	goto error_return;

      /* Calculate top address, avoiding undefined behaviour of shift
	 left operator when shift count is equal to size of type
	 being shifted.  */
      addr_mask = ((bfd_vma) 1 << (bfd_arch_bits_per_address (abfd) - 1)) - 1;
      addr_mask = (addr_mask << 1) + 1;

      i = 0;
      for (s = abfd->sections; s != NULL; s = s->next)
	{
	  if ((s->flags & SEC_ALLOC) != 0)
	    {
	      /* target_index is unused until bfd_elf_final_link
		 starts output of section symbols.  Use it to make
		 qsort stable.  */
	      s->target_index = i;
	      sections[i] = s;
	      ++i;
	      /* A wrapping section potentially clashes with header.  */
	      if (((s->lma + s->size / opb) & addr_mask) < (s->lma & addr_mask))
		wrap_to = (s->lma + s->size / opb) & addr_mask;
	    }
	}
      BFD_ASSERT (i <= bfd_count_sections (abfd));
      count = i;

      qsort (sections, (size_t) count, sizeof (asection *), elf_sort_sections);

      phdr_size = elf_program_header_size (abfd);
      if (phdr_size == (bfd_size_type) -1)
	phdr_size = get_program_header_size (abfd, info);
      phdr_size += bed->s->sizeof_ehdr;
      /* phdr_size is compared to LMA values which are in bytes.  */
      phdr_size /= opb;
      if (info != NULL)
	maxpagesize = info->maxpagesize;
      else
	maxpagesize = bed->maxpagesize;
      if (maxpagesize == 0)
	maxpagesize = 1;
      phdr_in_segment = info != NULL && info->load_phdrs;
      if (count != 0
	  && (((sections[0]->lma & addr_mask) & (maxpagesize - 1))
	      >= (phdr_size & (maxpagesize - 1))))
	/* For compatibility with old scripts that may not be using
	   SIZEOF_HEADERS, add headers when it looks like space has
	   been left for them.  */
	phdr_in_segment = true;

      /* Build the mapping.  */
      mfirst = NULL;
      pm = &mfirst;

      /* If we have a .interp section, then create a PT_PHDR segment for
	 the program headers and a PT_INTERP segment for the .interp
	 section.  */
      s = bfd_get_section_by_name (abfd, ".interp");
      if (s != NULL && (s->flags & SEC_LOAD) != 0 && s->size != 0)
	{
	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_PHDR;
	  m->p_flags = PF_R;
	  m->p_flags_valid = 1;
	  m->includes_phdrs = 1;
	  phdr_in_segment = true;
	  *pm = m;
	  pm = &m->next;

	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_INTERP;
	  m->count = 1;
	  m->sections[0] = s;

	  *pm = m;
	  pm = &m->next;
	}

      /* Look through the sections.  We put sections in the same program
	 segment when the start of the second section can be placed within
	 a few bytes of the end of the first section.  */
      last_hdr = NULL;
      last_size = 0;
      hdr_index = 0;
      writable = false;
      executable = false;
      dynsec = bfd_get_section_by_name (abfd, ".dynamic");
      if (dynsec != NULL
	  && (dynsec->flags & SEC_LOAD) == 0)
	dynsec = NULL;

      if ((abfd->flags & D_PAGED) == 0)
	phdr_in_segment = false;

      /* Deal with -Ttext or something similar such that the first section
	 is not adjacent to the program headers.  This is an
	 approximation, since at this point we don't know exactly how many
	 program headers we will need.  */
      if (phdr_in_segment && count > 0)
	{
	  bfd_vma phdr_lma;  /* Bytes.  */
	  bool separate_phdr = false;

	  phdr_lma = (sections[0]->lma - phdr_size) & addr_mask & -maxpagesize;
	  if (info != NULL
	      && info->separate_code
	      && (sections[0]->flags & SEC_CODE) != 0)
	    {
	      /* If data sections should be separate from code and
		 thus not executable, and the first section is
		 executable then put the file and program headers in
		 their own PT_LOAD.  */
	      separate_phdr = true;
	      if ((((phdr_lma + phdr_size - 1) & addr_mask & -maxpagesize)
		   == (sections[0]->lma & addr_mask & -maxpagesize)))
		{
		  /* The file and program headers are currently on the
		     same page as the first section.  Put them on the
		     previous page if we can.  */
		  if (phdr_lma >= maxpagesize)
		    phdr_lma -= maxpagesize;
		  else
		    separate_phdr = false;
		}
	    }
	  if ((sections[0]->lma & addr_mask) < phdr_lma
	      || (sections[0]->lma & addr_mask) < phdr_size)
	    /* If file and program headers would be placed at the end
	       of memory then it's probably better to omit them.  */
	    phdr_in_segment = false;
	  else if (phdr_lma < wrap_to)
	    /* If a section wraps around to where we'll be placing
	       file and program headers, then the headers will be
	       overwritten.  */
	    phdr_in_segment = false;
	  else if (separate_phdr)
	    {
	      m = make_mapping (abfd, sections, 0, 0, phdr_in_segment);
	      if (m == NULL)
		goto error_return;
	      m->p_paddr = phdr_lma * opb;
	      m->p_vaddr_offset
		= (sections[0]->vma - phdr_size) & addr_mask & -maxpagesize;
	      m->p_paddr_valid = 1;
	      *pm = m;
	      pm = &m->next;
	      phdr_in_segment = false;
	    }
	}

      for (i = 0, hdrpp = sections; i < count; i++, hdrpp++)
	{
	  asection *hdr;
	  bool new_segment;

	  hdr = *hdrpp;

	  /* See if this section and the last one will fit in the same
	     segment.  */

	  if (last_hdr == NULL)
	    {
	      /* If we don't have a segment yet, then we don't need a new
		 one (we build the last one after this loop).  */
	      new_segment = false;
	    }
	  else if (last_hdr->lma - last_hdr->vma != hdr->lma - hdr->vma)
	    {
	      /* If this section has a different relation between the
		 virtual address and the load address, then we need a new
		 segment.  */
	      new_segment = true;
	    }
	  else if (hdr->lma < last_hdr->lma + last_size
		   || last_hdr->lma + last_size < last_hdr->lma)
	    {
	      /* If this section has a load address that makes it overlap
		 the previous section, then we need a new segment.  */
	      new_segment = true;
	    }
	  else if ((abfd->flags & D_PAGED) != 0
		   && (((last_hdr->lma + last_size - 1) & -maxpagesize)
		       == (hdr->lma & -maxpagesize)))
	    {
	      /* If we are demand paged then we can't map two disk
		 pages onto the same memory page.  */
	      new_segment = false;
	    }
	  /* In the next test we have to be careful when last_hdr->lma is close
	     to the end of the address space.  If the aligned address wraps
	     around to the start of the address space, then there are no more
	     pages left in memory and it is OK to assume that the current
	     section can be included in the current segment.  */
	  else if ((BFD_ALIGN (last_hdr->lma + last_size, maxpagesize)
		    + maxpagesize > last_hdr->lma)
		   && (BFD_ALIGN (last_hdr->lma + last_size, maxpagesize)
		       + maxpagesize <= hdr->lma))
	    {
	      /* If putting this section in this segment would force us to
		 skip a page in the segment, then we need a new segment.  */
	      new_segment = true;
	    }
	  else if ((last_hdr->flags & (SEC_LOAD | SEC_THREAD_LOCAL)) == 0
		   && (hdr->flags & (SEC_LOAD | SEC_THREAD_LOCAL)) != 0)
	    {
	      /* We don't want to put a loaded section after a
		 nonloaded (ie. bss style) section in the same segment
		 as that will force the non-loaded section to be loaded.
		 Consider .tbss sections as loaded for this purpose.  */
	      new_segment = true;
	    }
	  else if ((abfd->flags & D_PAGED) == 0)
	    {
	      /* If the file is not demand paged, which means that we
		 don't require the sections to be correctly aligned in the
		 file, then there is no other reason for a new segment.  */
	      new_segment = false;
	    }
	  else if (info != NULL
		   && info->separate_code
		   && executable != ((hdr->flags & SEC_CODE) != 0))
	    {
	      new_segment = true;
	    }
	  else if (! writable
		   && (hdr->flags & SEC_READONLY) == 0)
	    {
	      /* We don't want to put a writable section in a read only
		 segment.  */
	      new_segment = true;
	    }
	  else
	    {
	      /* Otherwise, we can use the same segment.  */
	      new_segment = false;
	    }

	  /* Allow interested parties a chance to override our decision.  */
	  if (last_hdr != NULL
	      && info != NULL
	      && info->callbacks->override_segment_assignment != NULL)
	    new_segment
	      = info->callbacks->override_segment_assignment (info, abfd, hdr,
							      last_hdr,
							      new_segment);

	  if (! new_segment)
	    {
	      if ((hdr->flags & SEC_READONLY) == 0)
		writable = true;
	      if ((hdr->flags & SEC_CODE) != 0)
		executable = true;
	      last_hdr = hdr;
	      /* .tbss sections effectively have zero size.  */
	      last_size = (!IS_TBSS (hdr) ? hdr->size : 0) / opb;
	      continue;
	    }

	  /* We need a new program segment.  We must create a new program
	     header holding all the sections from hdr_index until hdr.  */

	  m = make_mapping (abfd, sections, hdr_index, i, phdr_in_segment);
	  if (m == NULL)
	    goto error_return;

	  *pm = m;
	  pm = &m->next;

	  if ((hdr->flags & SEC_READONLY) == 0)
	    writable = true;
	  else
	    writable = false;

	  if ((hdr->flags & SEC_CODE) == 0)
	    executable = false;
	  else
	    executable = true;

	  last_hdr = hdr;
	  /* .tbss sections effectively have zero size.  */
	  last_size = (!IS_TBSS (hdr) ? hdr->size : 0) / opb;
	  hdr_index = i;
	  phdr_in_segment = false;
	}

      /* Create a final PT_LOAD program segment, but not if it's just
	 for .tbss.  */
      if (last_hdr != NULL
	  && (i - hdr_index != 1
	      || !IS_TBSS (last_hdr)))
	{
	  m = make_mapping (abfd, sections, hdr_index, i, phdr_in_segment);
	  if (m == NULL)
	    goto error_return;

	  *pm = m;
	  pm = &m->next;
	}

      /* If there is a .dynamic section, throw in a PT_DYNAMIC segment.  */
      if (dynsec != NULL)
	{
	  m = _bfd_elf_make_dynamic_segment (abfd, dynsec);
	  if (m == NULL)
	    goto error_return;
	  *pm = m;
	  pm = &m->next;
	}

      /* For each batch of consecutive loadable SHT_NOTE  sections,
	 add a PT_NOTE segment.  We don't use bfd_get_section_by_name,
	 because if we link together nonloadable .note sections and
	 loadable .note sections, we will generate two .note sections
	 in the output file.  */
      for (s = abfd->sections; s != NULL; s = s->next)
	{
	  if ((s->flags & SEC_LOAD) != 0
	      && elf_section_type (s) == SHT_NOTE)
	    {
	      asection *s2;
	      unsigned int alignment_power = s->alignment_power;

	      count = 1;
	      for (s2 = s; s2->next != NULL; s2 = s2->next)
		{
		  if (s2->next->alignment_power == alignment_power
		      && (s2->next->flags & SEC_LOAD) != 0
		      && elf_section_type (s2->next) == SHT_NOTE
		      && align_power (s2->lma + s2->size / opb,
				      alignment_power)
		      == s2->next->lma)
		    count++;
		  else
		    break;
		}
	      amt = sizeof (struct elf_segment_map) - sizeof (asection *);
	      amt += count * sizeof (asection *);
	      m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	      if (m == NULL)
		goto error_return;
	      m->next = NULL;
	      m->p_type = PT_NOTE;
	      m->count = count;
	      while (count > 1)
		{
		  m->sections[m->count - count--] = s;
		  BFD_ASSERT ((s->flags & SEC_THREAD_LOCAL) == 0);
		  s = s->next;
		}
	      m->sections[m->count - 1] = s;
	      BFD_ASSERT ((s->flags & SEC_THREAD_LOCAL) == 0);
	      *pm = m;
	      pm = &m->next;
	    }
	  if (s->flags & SEC_THREAD_LOCAL)
	    {
	      if (! tls_count)
		first_tls = s;
	      tls_count++;
	    }
	  if (first_mbind == NULL
	      && (elf_section_flags (s) & SHF_GNU_MBIND) != 0)
	    first_mbind = s;
	}

      /* If there are any SHF_TLS output sections, add PT_TLS segment.  */
      if (tls_count > 0)
	{
	  amt = sizeof (struct elf_segment_map) - sizeof (asection *);
	  amt += tls_count * sizeof (asection *);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_TLS;
	  m->count = tls_count;
	  /* Mandated PF_R.  */
	  m->p_flags = PF_R;
	  m->p_flags_valid = 1;
	  s = first_tls;
	  for (i = 0; i < tls_count; ++i)
	    {
	      if ((s->flags & SEC_THREAD_LOCAL) == 0)
		{
		  _bfd_error_handler
		    (_("%pB: TLS sections are not adjacent:"), abfd);
		  s = first_tls;
		  i = 0;
		  while (i < tls_count)
		    {
		      if ((s->flags & SEC_THREAD_LOCAL) != 0)
			{
			  _bfd_error_handler (_("	    TLS: %pA"), s);
			  i++;
			}
		      else
			_bfd_error_handler (_("	non-TLS: %pA"), s);
		      s = s->next;
		    }
		  bfd_set_error (bfd_error_bad_value);
		  goto error_return;
		}
	      m->sections[i] = s;
	      s = s->next;
	    }

	  *pm = m;
	  pm = &m->next;
	}

      if (first_mbind
	  && (abfd->flags & D_PAGED) != 0
	  && (elf_tdata (abfd)->has_gnu_osabi & elf_gnu_osabi_mbind) != 0)
	for (s = first_mbind; s != NULL; s = s->next)
	  if ((elf_section_flags (s) & SHF_GNU_MBIND) != 0
	      && elf_section_data (s)->this_hdr.sh_info <= PT_GNU_MBIND_NUM)
	    {
	      /* Mandated PF_R.  */
	      unsigned long p_flags = PF_R;
	      if ((s->flags & SEC_READONLY) == 0)
		p_flags |= PF_W;
	      if ((s->flags & SEC_CODE) != 0)
		p_flags |= PF_X;

	      amt = sizeof (struct elf_segment_map) + sizeof (asection *);
	      m = bfd_zalloc (abfd, amt);
	      if (m == NULL)
		goto error_return;
	      m->next = NULL;
	      m->p_type = (PT_GNU_MBIND_LO
			   + elf_section_data (s)->this_hdr.sh_info);
	      m->count = 1;
	      m->p_flags_valid = 1;
	      m->sections[0] = s;
	      m->p_flags = p_flags;

	      *pm = m;
	      pm = &m->next;
	    }

      s = bfd_get_section_by_name (abfd,
				   NOTE_GNU_PROPERTY_SECTION_NAME);
      if (s != NULL && s->size != 0)
	{
	  amt = sizeof (struct elf_segment_map) + sizeof (asection *);
	  m = bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_GNU_PROPERTY;
	  m->count = 1;
	  m->p_flags_valid = 1;
	  m->sections[0] = s;
	  m->p_flags = PF_R;
	  *pm = m;
	  pm = &m->next;
	}

      /* If there is a .eh_frame_hdr section, throw in a PT_GNU_EH_FRAME
	 segment.  */
      eh_frame_hdr = elf_eh_frame_hdr (info);
      if (eh_frame_hdr != NULL
	  && (eh_frame_hdr->output_section->flags & SEC_LOAD) != 0)
	{
	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_GNU_EH_FRAME;
	  m->count = 1;
	  m->sections[0] = eh_frame_hdr->output_section;

	  *pm = m;
	  pm = &m->next;
	}

      /* If there is a .sframe section, throw in a PT_GNU_SFRAME
	 segment.  */
      sframe = elf_sframe (abfd);
      if (sframe != NULL
	  && (sframe->output_section->flags & SEC_LOAD) != 0
	  && sframe->size != 0)
	{
	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_GNU_SFRAME;
	  m->count = 1;
	  m->sections[0] = sframe->output_section;

	  *pm = m;
	  pm = &m->next;
	}

      if (elf_stack_flags (abfd))
	{
	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_GNU_STACK;
	  m->p_flags = elf_stack_flags (abfd);
	  m->p_align = bed->stack_align;
	  m->p_flags_valid = 1;
	  m->p_align_valid = m->p_align != 0;
	  if (info->stacksize > 0)
	    {
	      m->p_size = info->stacksize;
	      m->p_size_valid = 1;
	    }

	  *pm = m;
	  pm = &m->next;
	}

      if (info != NULL && info->relro)
	{
	  for (m = mfirst; m != NULL; m = m->next)
	    {
	      if (m->p_type == PT_LOAD
		  && m->count != 0
		  && m->sections[0]->vma >= info->relro_start
		  && m->sections[0]->vma < info->relro_end)
		{
		  i = m->count;
		  while (--i != (unsigned) -1)
		    {
		      if (m->sections[i]->size > 0
			  && (m->sections[i]->flags & SEC_LOAD) != 0
			  && (m->sections[i]->flags & SEC_HAS_CONTENTS) != 0)
			break;
		    }

		  if (i != (unsigned) -1)
		    break;
		}
	    }

	  /* Make a PT_GNU_RELRO segment only when it isn't empty.  */
	  if (m != NULL)
	    {
	      amt = sizeof (struct elf_segment_map);
	      m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	      if (m == NULL)
		goto error_return;
	      m->next = NULL;
	      m->p_type = PT_GNU_RELRO;
	      *pm = m;
	      pm = &m->next;
	    }
	}

      free (sections);
      elf_seg_map (abfd) = mfirst;
    }

  if (!elf_modify_segment_map (abfd, info, no_user_phdrs))
    return false;

  for (count = 0, m = elf_seg_map (abfd); m != NULL; m = m->next)
    ++count;
  elf_program_header_size (abfd) = count * bed->s->sizeof_phdr;

  return true;

 error_return:
  free (sections);
  return false;
}

/* Sort sections by address.  */

static int
elf_sort_sections (const void *arg1, const void *arg2)
{
  const asection *sec1 = *(const asection **) arg1;
  const asection *sec2 = *(const asection **) arg2;
  bfd_size_type size1, size2;

  /* Sort by LMA first, since this is the address used to
     place the section into a segment.  */
  if (sec1->lma < sec2->lma)
    return -1;
  else if (sec1->lma > sec2->lma)
    return 1;

  /* Then sort by VMA.  Normally the LMA and the VMA will be
     the same, and this will do nothing.  */
  if (sec1->vma < sec2->vma)
    return -1;
  else if (sec1->vma > sec2->vma)
    return 1;

  /* Put !SEC_LOAD sections after SEC_LOAD ones.  */

#define TOEND(x) (((x)->flags & (SEC_LOAD | SEC_THREAD_LOCAL)) == 0 \
		  && (x)->size != 0)

  if (TOEND (sec1))
    {
      if (!TOEND (sec2))
	return 1;
    }
  else if (TOEND (sec2))
    return -1;

#undef TOEND

  /* Sort by size, to put zero sized sections
     before others at the same address.  */

  size1 = (sec1->flags & SEC_LOAD) ? sec1->size : 0;
  size2 = (sec2->flags & SEC_LOAD) ? sec2->size : 0;

  if (size1 < size2)
    return -1;
  if (size1 > size2)
    return 1;

  return sec1->target_index - sec2->target_index;
}

/* This qsort comparison functions sorts PT_LOAD segments first and
   by p_paddr, for assign_file_positions_for_load_sections.  */

static int
elf_sort_segments (const void *arg1, const void *arg2)
{
  const struct elf_segment_map *m1 = *(const struct elf_segment_map **) arg1;
  const struct elf_segment_map *m2 = *(const struct elf_segment_map **) arg2;

  if (m1->p_type != m2->p_type)
    {
      if (m1->p_type == PT_NULL)
	return 1;
      if (m2->p_type == PT_NULL)
	return -1;
      return m1->p_type < m2->p_type ? -1 : 1;
    }
  if (m1->includes_filehdr != m2->includes_filehdr)
    return m1->includes_filehdr ? -1 : 1;
  if (m1->no_sort_lma != m2->no_sort_lma)
    return m1->no_sort_lma ? -1 : 1;
  if (m1->p_type == PT_LOAD && !m1->no_sort_lma)
    {
      bfd_vma lma1, lma2;  /* Octets.  */
      lma1 = 0;
      if (m1->p_paddr_valid)
	lma1 = m1->p_paddr;
      else if (m1->count != 0)
	{
	  unsigned int opb = bfd_octets_per_byte (m1->sections[0]->owner,
						  m1->sections[0]);
	  lma1 = (m1->sections[0]->lma + m1->p_vaddr_offset) * opb;
	}
      lma2 = 0;
      if (m2->p_paddr_valid)
	lma2 = m2->p_paddr;
      else if (m2->count != 0)
	{
	  unsigned int opb = bfd_octets_per_byte (m2->sections[0]->owner,
						  m2->sections[0]);
	  lma2 = (m2->sections[0]->lma + m2->p_vaddr_offset) * opb;
	}
      if (lma1 != lma2)
	return lma1 < lma2 ? -1 : 1;
    }
  if (m1->idx != m2->idx)
    return m1->idx < m2->idx ? -1 : 1;
  return 0;
}

/* Ian Lance Taylor writes:

   We shouldn't be using % with a negative signed number.  That's just
   not good.  We have to make sure either that the number is not
   negative, or that the number has an unsigned type.  When the types
   are all the same size they wind up as unsigned.  When file_ptr is a
   larger signed type, the arithmetic winds up as signed long long,
   which is wrong.

   What we're trying to say here is something like ``increase OFF by
   the least amount that will cause it to be equal to the VMA modulo
   the page size.''  */
/* In other words, something like:

   vma_offset = m->sections[0]->vma % bed->maxpagesize;
   off_offset = off % bed->maxpagesize;
   if (vma_offset < off_offset)
     adjustment = vma_offset + bed->maxpagesize - off_offset;
   else
     adjustment = vma_offset - off_offset;

   which can be collapsed into the expression below.  */

static file_ptr
vma_page_aligned_bias (bfd_vma vma, ufile_ptr off, bfd_vma maxpagesize)
{
  /* PR binutils/16199: Handle an alignment of zero.  */
  if (maxpagesize == 0)
    maxpagesize = 1;
  return ((vma - off) % maxpagesize);
}

static void
print_segment_map (const struct elf_segment_map *m)
{
  unsigned int j;
  const char *pt = get_segment_type (m->p_type);
  char buf[32];

  if (pt == NULL)
    {
      if (m->p_type >= PT_LOPROC && m->p_type <= PT_HIPROC)
	sprintf (buf, "LOPROC+%7.7x",
		 (unsigned int) (m->p_type - PT_LOPROC));
      else if (m->p_type >= PT_LOOS && m->p_type <= PT_HIOS)
	sprintf (buf, "LOOS+%7.7x",
		 (unsigned int) (m->p_type - PT_LOOS));
      else
	snprintf (buf, sizeof (buf), "%8.8x",
		  (unsigned int) m->p_type);
      pt = buf;
    }
  fflush (stdout);
  fprintf (stderr, "%s:", pt);
  for (j = 0; j < m->count; j++)
    fprintf (stderr, " %s", m->sections [j]->name);
  putc ('\n',stderr);
  fflush (stderr);
}

/* Assign file positions to the sections based on the mapping from
   sections to segments.  This function also sets up some fields in
   the file header.  */

static bool
assign_file_positions_for_load_sections (bfd *abfd,
					 struct bfd_link_info *link_info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_segment_map *m;
  struct elf_segment_map *phdr_load_seg;
  Elf_Internal_Phdr *phdrs;
  Elf_Internal_Phdr *p;
  file_ptr off;  /* Octets.  */
  bfd_size_type maxpagesize;
  unsigned int alloc, actual;
  unsigned int i, j;
  struct elf_segment_map **sorted_seg_map;
  unsigned int opb = bfd_octets_per_byte (abfd, NULL);

  if (link_info == NULL
      && !_bfd_elf_map_sections_to_segments (abfd, link_info, NULL))
    return false;

  alloc = 0;
  for (m = elf_seg_map (abfd); m != NULL; m = m->next)
    m->idx = alloc++;

  if (alloc)
    {
      elf_elfheader (abfd)->e_phoff = bed->s->sizeof_ehdr;
      elf_elfheader (abfd)->e_phentsize = bed->s->sizeof_phdr;
    }
  else
    {
      /* PR binutils/12467.  */
      elf_elfheader (abfd)->e_phoff = 0;
      elf_elfheader (abfd)->e_phentsize = 0;
    }

  elf_elfheader (abfd)->e_phnum = alloc;

  if (elf_program_header_size (abfd) == (bfd_size_type) -1)
    {
      actual = alloc;
      elf_program_header_size (abfd) = alloc * bed->s->sizeof_phdr;
    }
  else
    {
      actual = elf_program_header_size (abfd) / bed->s->sizeof_phdr;
      BFD_ASSERT (elf_program_header_size (abfd)
		  == actual * bed->s->sizeof_phdr);
      BFD_ASSERT (actual >= alloc);
    }

  if (alloc == 0)
    {
      elf_next_file_pos (abfd) = bed->s->sizeof_ehdr;
      return true;
    }

  /* We're writing the size in elf_program_header_size (abfd),
     see assign_file_positions_except_relocs, so make sure we have
     that amount allocated, with trailing space cleared.
     The variable alloc contains the computed need, while
     elf_program_header_size (abfd) contains the size used for the
     layout.
     See ld/emultempl/elf-generic.em:gld${EMULATION_NAME}_map_segments
     where the layout is forced to according to a larger size in the
     last iterations for the testcase ld-elf/header.  */
  phdrs = bfd_zalloc (abfd, (actual * sizeof (*phdrs)
			     + alloc * sizeof (*sorted_seg_map)));
  sorted_seg_map = (struct elf_segment_map **) (phdrs + actual);
  elf_tdata (abfd)->phdr = phdrs;
  if (phdrs == NULL)
    return false;

  for (m = elf_seg_map (abfd), j = 0; m != NULL; m = m->next, j++)
    {
      sorted_seg_map[j] = m;
      /* If elf_segment_map is not from map_sections_to_segments, the
	 sections may not be correctly ordered.  NOTE: sorting should
	 not be done to the PT_NOTE section of a corefile, which may
	 contain several pseudo-sections artificially created by bfd.
	 Sorting these pseudo-sections breaks things badly.  */
      if (m->count > 1
	  && !(elf_elfheader (abfd)->e_type == ET_CORE
	       && m->p_type == PT_NOTE))
	{
	  for (i = 0; i < m->count; i++)
	    m->sections[i]->target_index = i;
	  qsort (m->sections, (size_t) m->count, sizeof (asection *),
		 elf_sort_sections);
	}
    }
  if (alloc > 1)
    qsort (sorted_seg_map, alloc, sizeof (*sorted_seg_map),
	   elf_sort_segments);

  maxpagesize = 1;
  if ((abfd->flags & D_PAGED) != 0)
    {
      if (link_info != NULL)
	maxpagesize = link_info->maxpagesize;
      else
	maxpagesize = bed->maxpagesize;
    }

  /* Sections must map to file offsets past the ELF file header.  */
  off = bed->s->sizeof_ehdr;
  /* And if one of the PT_LOAD headers doesn't include the program
     headers then we'll be mapping program headers in the usual
     position after the ELF file header.  */
  phdr_load_seg = NULL;
  for (j = 0; j < alloc; j++)
    {
      m = sorted_seg_map[j];
      if (m->p_type != PT_LOAD)
	break;
      if (m->includes_phdrs)
	{
	  phdr_load_seg = m;
	  break;
	}
    }
  if (phdr_load_seg == NULL)
    off += actual * bed->s->sizeof_phdr;

  for (j = 0; j < alloc; j++)
    {
      asection **secpp;
      bfd_vma off_adjust;  /* Octets.  */
      bool no_contents;
      bfd_size_type p_align;
      bool p_align_p;

      /* An ELF segment (described by Elf_Internal_Phdr) may contain a
	 number of sections with contents contributing to both p_filesz
	 and p_memsz, followed by a number of sections with no contents
	 that just contribute to p_memsz.  In this loop, OFF tracks next
	 available file offset for PT_LOAD and PT_NOTE segments.  */
      m = sorted_seg_map[j];
      p = phdrs + m->idx;
      p->p_type = m->p_type;
      p->p_flags = m->p_flags;
      p_align = bed->p_align;
      p_align_p = false;

      if (m->count == 0)
	p->p_vaddr = m->p_vaddr_offset * opb;
      else
	p->p_vaddr = (m->sections[0]->vma + m->p_vaddr_offset) * opb;

      if (m->p_paddr_valid)
	p->p_paddr = m->p_paddr;
      else if (m->count == 0)
	p->p_paddr = 0;
      else
	p->p_paddr = (m->sections[0]->lma + m->p_vaddr_offset) * opb;

      if (p->p_type == PT_LOAD
	  && (abfd->flags & D_PAGED) != 0)
	{
	  /* p_align in demand paged PT_LOAD segments effectively stores
	     the maximum page size.  When copying an executable with
	     objcopy, we set m->p_align from the input file.  Use this
	     value for maxpagesize rather than bed->maxpagesize, which
	     may be different.  Note that we use maxpagesize for PT_TLS
	     segment alignment later in this function, so we are relying
	     on at least one PT_LOAD segment appearing before a PT_TLS
	     segment.  */
	  if (m->p_align_valid)
	    maxpagesize = m->p_align;
	  else if (p_align != 0
		   && (link_info == NULL
		       || !link_info->maxpagesize_is_set))
	    /* Set p_align to the default p_align value while laying
	       out segments aligning to the maximum page size or the
	       largest section alignment.  The run-time loader can
	       align segments to the default p_align value or the
	       maximum page size, depending on system page size.  */
	    p_align_p = true;

	  p->p_align = maxpagesize;
	}
      else if (m->p_align_valid)
	p->p_align = m->p_align;
      else if (m->count == 0)
	p->p_align = 1 << bed->s->log_file_align;

      if (m == phdr_load_seg)
	{
	  if (!m->includes_filehdr)
	    p->p_offset = off;
	  off += actual * bed->s->sizeof_phdr;
	}

      no_contents = false;
      off_adjust = 0;
      if (p->p_type == PT_LOAD
	  && m->count > 0)
	{
	  bfd_size_type align;  /* Bytes.  */
	  unsigned int align_power = 0;

	  if (m->p_align_valid)
	    align = p->p_align;
	  else
	    {
	      for (i = 0, secpp = m->sections; i < m->count; i++, secpp++)
		{
		  unsigned int secalign;

		  secalign = bfd_section_alignment (*secpp);
		  if (secalign > align_power)
		    align_power = secalign;
		}
	      align = (bfd_size_type) 1 << align_power;
	      if (align < maxpagesize)
		{
		  /* If a section requires alignment higher than the
		     default p_align value, don't set p_align to the
		     default p_align value.  */
		  if (align > p_align)
		    p_align_p = false;
		  align = maxpagesize;
		}
	      else
		{
		  /* If a section requires alignment higher than the
		     maximum page size, set p_align to the section
		     alignment.  */
		  p_align_p = true;
		  p_align = align;
		}
	    }

	  for (i = 0; i < m->count; i++)
	    if ((m->sections[i]->flags & (SEC_LOAD | SEC_HAS_CONTENTS)) == 0)
	      /* If we aren't making room for this section, then
		 it must be SHT_NOBITS regardless of what we've
		 set via struct bfd_elf_special_section.  */
	      elf_section_type (m->sections[i]) = SHT_NOBITS;

	  /* Find out whether this segment contains any loadable
	     sections.  */
	  no_contents = true;
	  for (i = 0; i < m->count; i++)
	    if (elf_section_type (m->sections[i]) != SHT_NOBITS)
	      {
		no_contents = false;
		break;
	      }

	  off_adjust = vma_page_aligned_bias (p->p_vaddr, off, align * opb);

	  /* Broken hardware and/or kernel require that files do not
	     map the same page with different permissions on some hppa
	     processors.  */
	  if (j != 0
	      && (abfd->flags & D_PAGED) != 0
	      && bed->no_page_alias
	      && (off & (maxpagesize - 1)) != 0
	      && ((off & -maxpagesize)
		  == ((off + off_adjust) & -maxpagesize)))
	    off_adjust += maxpagesize;
	  off += off_adjust;
	  if (no_contents)
	    {
	      /* We shouldn't need to align the segment on disk since
		 the segment doesn't need file space, but the gABI
		 arguably requires the alignment and glibc ld.so
		 checks it.  So to comply with the alignment
		 requirement but not waste file space, we adjust
		 p_offset for just this segment.  (OFF_ADJUST is
		 subtracted from OFF later.)  This may put p_offset
		 past the end of file, but that shouldn't matter.  */
	    }
	  else
	    off_adjust = 0;
	}
      /* Make sure the .dynamic section is the first section in the
	 PT_DYNAMIC segment.  */
      else if (p->p_type == PT_DYNAMIC
	       && m->count > 1
	       && strcmp (m->sections[0]->name, ".dynamic") != 0)
	{
	  _bfd_error_handler
	    (_("%pB: The first section in the PT_DYNAMIC segment"
	       " is not the .dynamic section"),
	     abfd);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      /* Set the note section type to SHT_NOTE.  */
      else if (p->p_type == PT_NOTE)
	for (i = 0; i < m->count; i++)
	  elf_section_type (m->sections[i]) = SHT_NOTE;

      if (m->includes_filehdr)
	{
	  if (!m->p_flags_valid)
	    p->p_flags |= PF_R;
	  p->p_filesz = bed->s->sizeof_ehdr;
	  p->p_memsz = bed->s->sizeof_ehdr;
	  if (p->p_type == PT_LOAD)
	    {
	      if (m->count > 0)
		{
		  if (p->p_vaddr < (bfd_vma) off
		      || (!m->p_paddr_valid
			  && p->p_paddr < (bfd_vma) off))
		    {
		      _bfd_error_handler
			(_("%pB: not enough room for program headers,"
			   " try linking with -N"),
			 abfd);
		      bfd_set_error (bfd_error_bad_value);
		      return false;
		    }
		  p->p_vaddr -= off;
		  if (!m->p_paddr_valid)
		    p->p_paddr -= off;
		}
	    }
	  else if (sorted_seg_map[0]->includes_filehdr)
	    {
	      Elf_Internal_Phdr *filehdr = phdrs + sorted_seg_map[0]->idx;
	      p->p_vaddr = filehdr->p_vaddr;
	      if (!m->p_paddr_valid)
		p->p_paddr = filehdr->p_paddr;
	    }
	}

      if (m->includes_phdrs)
	{
	  if (!m->p_flags_valid)
	    p->p_flags |= PF_R;
	  p->p_filesz += actual * bed->s->sizeof_phdr;
	  p->p_memsz += actual * bed->s->sizeof_phdr;
	  if (!m->includes_filehdr)
	    {
	      if (p->p_type == PT_LOAD)
		{
		  elf_elfheader (abfd)->e_phoff = p->p_offset;
		  if (m->count > 0)
		    {
		      p->p_vaddr -= off - p->p_offset;
		      if (!m->p_paddr_valid)
			p->p_paddr -= off - p->p_offset;
		    }
		}
	      else if (phdr_load_seg != NULL)
		{
		  Elf_Internal_Phdr *phdr = phdrs + phdr_load_seg->idx;
		  bfd_vma phdr_off = 0;  /* Octets.  */
		  if (phdr_load_seg->includes_filehdr)
		    phdr_off = bed->s->sizeof_ehdr;
		  p->p_vaddr = phdr->p_vaddr + phdr_off;
		  if (!m->p_paddr_valid)
		    p->p_paddr = phdr->p_paddr + phdr_off;
		  p->p_offset = phdr->p_offset + phdr_off;
		}
	      else
		p->p_offset = bed->s->sizeof_ehdr;
	    }
	}

      if (p->p_type == PT_LOAD
	  || (p->p_type == PT_NOTE && bfd_get_format (abfd) == bfd_core))
	{
	  if (!m->includes_filehdr && !m->includes_phdrs)
	    {
	      p->p_offset = off;
	      if (no_contents)
		{
		  /* Put meaningless p_offset for PT_LOAD segments
		     without file contents somewhere within the first
		     page, in an attempt to not point past EOF.  */
		  bfd_size_type align = maxpagesize;
		  if (align < p->p_align)
		    align = p->p_align;
		  if (align < 1)
		    align = 1;
		  p->p_offset = off % align;
		}
	    }
	  else
	    {
	      file_ptr adjust;  /* Octets.  */

	      adjust = off - (p->p_offset + p->p_filesz);
	      if (!no_contents)
		p->p_filesz += adjust;
	      p->p_memsz += adjust;
	    }
	}

      /* Set up p_filesz, p_memsz, p_align and p_flags from the section
	 maps.  Set filepos for sections in PT_LOAD segments, and in
	 core files, for sections in PT_NOTE segments.
	 assign_file_positions_for_non_load_sections will set filepos
	 for other sections and update p_filesz for other segments.  */
      for (i = 0, secpp = m->sections; i < m->count; i++, secpp++)
	{
	  asection *sec;
	  bfd_size_type align;
	  Elf_Internal_Shdr *this_hdr;

	  sec = *secpp;
	  this_hdr = &elf_section_data (sec)->this_hdr;
	  align = (bfd_size_type) 1 << bfd_section_alignment (sec);

	  if ((p->p_type == PT_LOAD
	       || p->p_type == PT_TLS)
	      && (this_hdr->sh_type != SHT_NOBITS
		  || ((this_hdr->sh_flags & SHF_ALLOC) != 0
		      && ((this_hdr->sh_flags & SHF_TLS) == 0
			  || p->p_type == PT_TLS))))
	    {
	      bfd_vma p_start = p->p_paddr;		/* Octets.  */
	      bfd_vma p_end = p_start + p->p_memsz;	/* Octets.  */
	      bfd_vma s_start = sec->lma * opb;		/* Octets.  */
	      bfd_vma adjust = s_start - p_end;		/* Octets.  */

	      if (adjust != 0
		  && (s_start < p_end
		      || p_end < p_start))
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: section %pA lma %#" PRIx64
		       " adjusted to %#" PRIx64),
		     abfd, sec, (uint64_t) s_start / opb,
		     (uint64_t) p_end / opb);
		  adjust = 0;
		  sec->lma = p_end / opb;
		}
	      p->p_memsz += adjust;

	      if (p->p_type == PT_LOAD)
		{
		  if (this_hdr->sh_type != SHT_NOBITS)
		    {
		      off_adjust = 0;
		      if (p->p_filesz + adjust < p->p_memsz)
			{
			  /* We have a PROGBITS section following NOBITS ones.
			     Allocate file space for the NOBITS section(s).
			     We don't need to write out the zeros, posix
			     fseek past the end of data already written
			     followed by a write at that location is
			     guaranteed to result in zeros being read
			     from the gap.  */
			  adjust = p->p_memsz - p->p_filesz;
			}
		    }
		  /* We only adjust sh_offset in SHT_NOBITS sections
		     as would seem proper for their address when the
		     section is first in the segment.  sh_offset
		     doesn't really have any significance for
		     SHT_NOBITS anyway, apart from a notional position
		     relative to other sections.  Historically we
		     didn't bother with adjusting sh_offset and some
		     programs depend on it not being adjusted.  See
		     pr12921 and pr25662.  */
		  if (this_hdr->sh_type != SHT_NOBITS || i == 0)
		    {
		      off += adjust;
		      if (this_hdr->sh_type == SHT_NOBITS)
			off_adjust += adjust;
		    }
		}
	      if (this_hdr->sh_type != SHT_NOBITS)
		p->p_filesz += adjust;
	    }

	  if (p->p_type == PT_NOTE && bfd_get_format (abfd) == bfd_core)
	    {
	      /* The section at i == 0 is the one that actually contains
		 everything.  */
	      if (i == 0)
		{
		  this_hdr->sh_offset = sec->filepos = off;
		  off += this_hdr->sh_size;
		  p->p_filesz = this_hdr->sh_size;
		  p->p_memsz = 0;
		  p->p_align = 1;
		}
	      else
		{
		  /* The rest are fake sections that shouldn't be written.  */
		  sec->filepos = 0;
		  sec->size = 0;
		  sec->flags = 0;
		  continue;
		}
	    }
	  else
	    {
	      if (p->p_type == PT_LOAD)
		{
		  this_hdr->sh_offset = sec->filepos = off;
		  if (this_hdr->sh_type != SHT_NOBITS)
		    off += this_hdr->sh_size;
		}
	      else if (this_hdr->sh_type == SHT_NOBITS
		       && (this_hdr->sh_flags & SHF_TLS) != 0
		       && this_hdr->sh_offset == 0)
		{
		  /* This is a .tbss section that didn't get a PT_LOAD.
		     (See _bfd_elf_map_sections_to_segments "Create a
		     final PT_LOAD".)  Set sh_offset to the value it
		     would have if we had created a zero p_filesz and
		     p_memsz PT_LOAD header for the section.  This
		     also makes the PT_TLS header have the same
		     p_offset value.  */
		  bfd_vma adjust = vma_page_aligned_bias (this_hdr->sh_addr,
							  off, align);
		  this_hdr->sh_offset = sec->filepos = off + adjust;
		}

	      if (this_hdr->sh_type != SHT_NOBITS)
		{
		  p->p_filesz += this_hdr->sh_size;
		  /* A load section without SHF_ALLOC is something like
		     a note section in a PT_NOTE segment.  These take
		     file space but are not loaded into memory.  */
		  if ((this_hdr->sh_flags & SHF_ALLOC) != 0)
		    p->p_memsz += this_hdr->sh_size;
		}
	      else if ((this_hdr->sh_flags & SHF_ALLOC) != 0)
		{
		  if (p->p_type == PT_TLS)
		    p->p_memsz += this_hdr->sh_size;

		  /* .tbss is special.  It doesn't contribute to p_memsz of
		     normal segments.  */
		  else if ((this_hdr->sh_flags & SHF_TLS) == 0)
		    p->p_memsz += this_hdr->sh_size;
		}

	      if (align > p->p_align
		  && !m->p_align_valid
		  && (p->p_type != PT_LOAD
		      || (abfd->flags & D_PAGED) == 0))
		p->p_align = align;
	    }

	  if (!m->p_flags_valid)
	    {
	      p->p_flags |= PF_R;
	      if ((this_hdr->sh_flags & SHF_EXECINSTR) != 0)
		p->p_flags |= PF_X;
	      if ((this_hdr->sh_flags & SHF_WRITE) != 0)
		p->p_flags |= PF_W;
	    }
	}

      off -= off_adjust;

      /* PR ld/20815 - Check that the program header segment, if
	 present, will be loaded into memory.  */
      if (p->p_type == PT_PHDR
	  && phdr_load_seg == NULL
	  && !(bed->elf_backend_allow_non_load_phdr != NULL
	       && bed->elf_backend_allow_non_load_phdr (abfd, phdrs, alloc)))
	{
	  /* The fix for this error is usually to edit the linker script being
	     used and set up the program headers manually.  Either that or
	     leave room for the headers at the start of the SECTIONS.  */
	  _bfd_error_handler (_("%pB: error: PHDR segment not covered"
				" by LOAD segment"),
			      abfd);
	  if (link_info == NULL)
	    return false;
	  /* Arrange for the linker to exit with an error, deleting
	     the output file unless --noinhibit-exec is given.  */
	  link_info->callbacks->info ("%X");
	}

      /* Check that all sections are in a PT_LOAD segment.
	 Don't check funky gdb generated core files.  */
      if (p->p_type == PT_LOAD && bfd_get_format (abfd) != bfd_core)
	{
	  bool check_vma = true;

	  for (i = 1; i < m->count; i++)
	    if (m->sections[i]->vma == m->sections[i - 1]->vma
		&& ELF_SECTION_SIZE (&(elf_section_data (m->sections[i])
				       ->this_hdr), p) != 0
		&& ELF_SECTION_SIZE (&(elf_section_data (m->sections[i - 1])
				       ->this_hdr), p) != 0)
	      {
		/* Looks like we have overlays packed into the segment.  */
		check_vma = false;
		break;
	      }

	  for (i = 0; i < m->count; i++)
	    {
	      Elf_Internal_Shdr *this_hdr;
	      asection *sec;

	      sec = m->sections[i];
	      this_hdr = &(elf_section_data(sec)->this_hdr);
	      if (!ELF_SECTION_IN_SEGMENT_1 (this_hdr, p, check_vma, 0)
		  && !ELF_TBSS_SPECIAL (this_hdr, p))
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: section `%pA' can't be allocated in segment %d"),
		     abfd, sec, j);
		  print_segment_map (m);
		}
	    }

	  if (p_align_p)
	    p->p_align = p_align;
	}
    }

  elf_next_file_pos (abfd) = off;

  if (link_info != NULL
      && phdr_load_seg != NULL
      && phdr_load_seg->includes_filehdr)
    {
      /* There is a segment that contains both the file headers and the
	 program headers, so provide a symbol __ehdr_start pointing there.
	 A program can use this to examine itself robustly.  */

      struct elf_link_hash_entry *hash
	= elf_link_hash_lookup (elf_hash_table (link_info), "__ehdr_start",
				false, false, true);
      /* If the symbol was referenced and not defined, define it.  */
      if (hash != NULL
	  && (hash->root.type == bfd_link_hash_new
	      || hash->root.type == bfd_link_hash_undefined
	      || hash->root.type == bfd_link_hash_undefweak
	      || hash->root.type == bfd_link_hash_common))
	{
	  asection *s = NULL;
	  bfd_vma filehdr_vaddr = phdrs[phdr_load_seg->idx].p_vaddr / opb;

	  if (phdr_load_seg->count != 0)
	    /* The segment contains sections, so use the first one.  */
	    s = phdr_load_seg->sections[0];
	  else
	    /* Use the first (i.e. lowest-addressed) section in any segment.  */
	    for (m = elf_seg_map (abfd); m != NULL; m = m->next)
	      if (m->p_type == PT_LOAD && m->count != 0)
		{
		  s = m->sections[0];
		  break;
		}

	  if (s != NULL)
	    {
	      hash->root.u.def.value = filehdr_vaddr - s->vma;
	      hash->root.u.def.section = s;
	    }
	  else
	    {
	      hash->root.u.def.value = filehdr_vaddr;
	      hash->root.u.def.section = bfd_abs_section_ptr;
	    }

	  hash->root.type = bfd_link_hash_defined;
	  hash->def_regular = 1;
	  hash->non_elf = 0;
	}
    }

  return true;
}

/* Determine if a bfd is a debuginfo file.  Unfortunately there
   is no defined method for detecting such files, so we have to
   use heuristics instead.  */

bool
is_debuginfo_file (bfd *abfd)
{
  if (abfd == NULL || bfd_get_flavour (abfd) != bfd_target_elf_flavour)
    return false;

  Elf_Internal_Shdr **start_headers = elf_elfsections (abfd);
  Elf_Internal_Shdr **end_headers = start_headers + elf_numsections (abfd);
  Elf_Internal_Shdr **headerp;

  for (headerp = start_headers; headerp < end_headers; headerp ++)
    {
      Elf_Internal_Shdr *header = * headerp;

      /* Debuginfo files do not have any allocated SHT_PROGBITS sections.
	 The only allocated sections are SHT_NOBITS or SHT_NOTES.  */
      if ((header->sh_flags & SHF_ALLOC) == SHF_ALLOC
	  && header->sh_type != SHT_NOBITS
	  && header->sh_type != SHT_NOTE)
	return false;
    }

  return true;
}

/* Assign file positions for other sections, except for compressed debug
   and sections assigned in _bfd_elf_assign_file_positions_for_non_load.  */

static bool
assign_file_positions_for_non_load_sections (bfd *abfd,
					     struct bfd_link_info *link_info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  Elf_Internal_Shdr **i_shdrpp;
  Elf_Internal_Shdr **hdrpp, **end_hdrpp;
  Elf_Internal_Phdr *phdrs;
  Elf_Internal_Phdr *p;
  struct elf_segment_map *m;
  file_ptr off;
  unsigned int opb = bfd_octets_per_byte (abfd, NULL);
  bfd_vma maxpagesize;

  if (link_info != NULL)
    maxpagesize = link_info->maxpagesize;
  else
    maxpagesize = bed->maxpagesize;
  i_shdrpp = elf_elfsections (abfd);
  end_hdrpp = i_shdrpp + elf_numsections (abfd);
  off = elf_next_file_pos (abfd);
  for (hdrpp = i_shdrpp + 1; hdrpp < end_hdrpp; hdrpp++)
    {
      Elf_Internal_Shdr *hdr;
      bfd_vma align;

      hdr = *hdrpp;
      if (hdr->bfd_section != NULL
	  && (hdr->bfd_section->filepos != 0
	      || (hdr->sh_type == SHT_NOBITS
		  && hdr->contents == NULL)))
	BFD_ASSERT (hdr->sh_offset == hdr->bfd_section->filepos);
      else if ((hdr->sh_flags & SHF_ALLOC) != 0)
	{
	  if (hdr->sh_size != 0
	      /* PR 24717 - debuginfo files are known to be not strictly
		 compliant with the ELF standard.  In particular they often
		 have .note.gnu.property sections that are outside of any
		 loadable segment.  This is not a problem for such files,
		 so do not warn about them.  */
	      && ! is_debuginfo_file (abfd))
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: warning: allocated section `%s' not in segment"),
	       abfd,
	       (hdr->bfd_section == NULL
		? "*unknown*"
		: hdr->bfd_section->name));
	  /* We don't need to page align empty sections.  */
	  if ((abfd->flags & D_PAGED) != 0 && hdr->sh_size != 0)
	    align = maxpagesize;
	  else
	    align = hdr->sh_addralign & -hdr->sh_addralign;
	  off += vma_page_aligned_bias (hdr->sh_addr, off, align);
	  off = _bfd_elf_assign_file_position_for_section (hdr, off,
							   false);
	}
      else if (((hdr->sh_type == SHT_REL || hdr->sh_type == SHT_RELA)
		&& hdr->bfd_section == NULL)
	       /* We don't know the offset of these sections yet:
		  their size has not been decided.  */
	       || (abfd->is_linker_output
		   && hdr->bfd_section != NULL
		   && (hdr->sh_name == -1u
		       || bfd_section_is_ctf (hdr->bfd_section)))
	       || hdr == i_shdrpp[elf_onesymtab (abfd)]
	       || (elf_symtab_shndx_list (abfd) != NULL
		   && hdr == i_shdrpp[elf_symtab_shndx_list (abfd)->ndx])
	       || hdr == i_shdrpp[elf_strtab_sec (abfd)]
	       || hdr == i_shdrpp[elf_shstrtab_sec (abfd)])
	hdr->sh_offset = -1;
      else
	off = _bfd_elf_assign_file_position_for_section (hdr, off, true);
    }
  elf_next_file_pos (abfd) = off;

  /* Now that we have set the section file positions, we can set up
     the file positions for the non PT_LOAD segments.  */
  phdrs = elf_tdata (abfd)->phdr;
  for (m = elf_seg_map (abfd), p = phdrs; m != NULL; m = m->next, p++)
    {
      if (p->p_type == PT_GNU_RELRO)
	{
	  bfd_vma start, end;  /* Bytes.  */
	  bool ok;

	  if (link_info != NULL)
	    {
	      /* During linking the range of the RELRO segment is passed
		 in link_info.  Note that there may be padding between
		 relro_start and the first RELRO section.  */
	      start = link_info->relro_start;
	      end = link_info->relro_end;
	    }
	  else if (m->count != 0)
	    {
	      if (!m->p_size_valid)
		abort ();
	      start = m->sections[0]->vma;
	      end = start + m->p_size / opb;
	    }
	  else
	    {
	      start = 0;
	      end = 0;
	    }

	  ok = false;
	  if (start < end)
	    {
	      struct elf_segment_map *lm;
	      const Elf_Internal_Phdr *lp;
	      unsigned int i;

	      /* Find a LOAD segment containing a section in the RELRO
		 segment.  */
	      for (lm = elf_seg_map (abfd), lp = phdrs;
		   lm != NULL;
		   lm = lm->next, lp++)
		{
		  if (lp->p_type == PT_LOAD
		      && lm->count != 0
		      && (lm->sections[lm->count - 1]->vma
			  + (!IS_TBSS (lm->sections[lm->count - 1])
			     ? lm->sections[lm->count - 1]->size / opb
			     : 0)) > start
		      && lm->sections[0]->vma < end)
		    break;
		}

	      if (lm != NULL)
		{
		  /* Find the section starting the RELRO segment.  */
		  for (i = 0; i < lm->count; i++)
		    {
		      asection *s = lm->sections[i];
		      if (s->vma >= start
			  && s->vma < end
			  && s->size != 0)
			break;
		    }

		  if (i < lm->count)
		    {
		      p->p_vaddr = lm->sections[i]->vma * opb;
		      p->p_paddr = lm->sections[i]->lma * opb;
		      p->p_offset = lm->sections[i]->filepos;
		      p->p_memsz = end * opb - p->p_vaddr;
		      p->p_filesz = p->p_memsz;

		      /* The RELRO segment typically ends a few bytes
			 into .got.plt but other layouts are possible.
			 In cases where the end does not match any
			 loaded section (for instance is in file
			 padding), trim p_filesz back to correspond to
			 the end of loaded section contents.  */
		      if (p->p_filesz > lp->p_vaddr + lp->p_filesz - p->p_vaddr)
			p->p_filesz = lp->p_vaddr + lp->p_filesz - p->p_vaddr;

		      /* Preserve the alignment and flags if they are
			 valid.  The gold linker generates RW/4 for
			 the PT_GNU_RELRO section.  It is better for
			 objcopy/strip to honor these attributes
			 otherwise gdb will choke when using separate
			 debug files.  */
		      if (!m->p_align_valid)
			p->p_align = 1;
		      if (!m->p_flags_valid)
			p->p_flags = PF_R;
		      ok = true;
		    }
		}
	    }

	  if (!ok)
	    {
	      if (link_info != NULL)
		_bfd_error_handler
		  (_("%pB: warning: unable to allocate any sections"
		     " to PT_GNU_RELRO segment"),
		   abfd);
	      memset (p, 0, sizeof *p);
	    }
	}
      else if (p->p_type == PT_GNU_STACK)
	{
	  if (m->p_size_valid)
	    p->p_memsz = m->p_size;
	}
      else if (m->count != 0)
	{
	  unsigned int i;

	  if (p->p_type != PT_LOAD
	      && (p->p_type != PT_NOTE
		  || bfd_get_format (abfd) != bfd_core))
	    {
	      /* A user specified segment layout may include a PHDR
		 segment that overlaps with a LOAD segment...  */
	      if (p->p_type == PT_PHDR)
		{
		  m->count = 0;
		  continue;
		}

	      if (m->includes_filehdr || m->includes_phdrs)
		{
		  /* PR 17512: file: 2195325e.  */
		  _bfd_error_handler
		    (_("%pB: error: non-load segment %d includes file header "
		       "and/or program header"),
		     abfd, (int) (p - phdrs));
		  return false;
		}

	      p->p_filesz = 0;
	      p->p_offset = m->sections[0]->filepos;
	      for (i = m->count; i-- != 0;)
		{
		  asection *sect = m->sections[i];
		  Elf_Internal_Shdr *hdr = &elf_section_data (sect)->this_hdr;
		  if (hdr->sh_type != SHT_NOBITS)
		    {
		      p->p_filesz = sect->filepos - p->p_offset + hdr->sh_size;
		      /* NB: p_memsz of the loadable PT_NOTE segment
			 should be the same as p_filesz.  */
		      if (p->p_type == PT_NOTE
			  && (hdr->sh_flags & SHF_ALLOC) != 0)
			p->p_memsz = p->p_filesz;
		      break;
		    }
		}
	    }
	}
    }

  return true;
}

static elf_section_list *
find_section_in_list (unsigned int i, elf_section_list * list)
{
  for (;list != NULL; list = list->next)
    if (list->ndx == i)
      break;
  return list;
}

/* Work out the file positions of all the sections.  This is called by
   _bfd_elf_compute_section_file_positions.  All the section sizes and
   VMAs must be known before this is called.

   Reloc sections come in two flavours: Those processed specially as
   "side-channel" data attached to a section to which they apply, and
   those that bfd doesn't process as relocations.  The latter sort are
   stored in a normal bfd section by bfd_section_from_shdr.  We don't
   consider the former sort here, unless they form part of the loadable
   image.  Reloc sections not assigned here (and compressed debugging
   sections and CTF sections which nothing else in the file can rely
   upon) will be handled later by assign_file_positions_for_relocs.

   We also don't set the positions of the .symtab and .strtab here.  */

static bool
assign_file_positions_except_relocs (bfd *abfd,
				     struct bfd_link_info *link_info)
{
  struct elf_obj_tdata *tdata = elf_tdata (abfd);
  Elf_Internal_Ehdr *i_ehdrp = elf_elfheader (abfd);
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  unsigned int alloc;

  if ((abfd->flags & (EXEC_P | DYNAMIC)) == 0
      && bfd_get_format (abfd) != bfd_core)
    {
      Elf_Internal_Shdr ** const i_shdrpp = elf_elfsections (abfd);
      unsigned int num_sec = elf_numsections (abfd);
      Elf_Internal_Shdr **hdrpp;
      unsigned int i;
      file_ptr off;

      /* Start after the ELF header.  */
      off = i_ehdrp->e_ehsize;

      /* We are not creating an executable, which means that we are
	 not creating a program header, and that the actual order of
	 the sections in the file is unimportant.  */
      for (i = 1, hdrpp = i_shdrpp + 1; i < num_sec; i++, hdrpp++)
	{
	  Elf_Internal_Shdr *hdr;

	  hdr = *hdrpp;
	  if (((hdr->sh_type == SHT_REL || hdr->sh_type == SHT_RELA)
	       && hdr->bfd_section == NULL)
	      /* Do not assign offsets for these sections yet: we don't know
		 their sizes.  */
	      || (abfd->is_linker_output
		  && hdr->bfd_section != NULL
		  && (hdr->sh_name == -1u
		      || bfd_section_is_ctf (hdr->bfd_section)))
	      || i == elf_onesymtab (abfd)
	      || (elf_symtab_shndx_list (abfd) != NULL
		  && hdr == i_shdrpp[elf_symtab_shndx_list (abfd)->ndx])
	      || i == elf_strtab_sec (abfd)
	      || i == elf_shstrtab_sec (abfd))
	    {
	      hdr->sh_offset = -1;
	    }
	  else
	    off = _bfd_elf_assign_file_position_for_section (hdr, off, true);
	}

      elf_next_file_pos (abfd) = off;
      elf_program_header_size (abfd) = 0;
    }
  else
    {
      /* Assign file positions for the loaded sections based on the
	 assignment of sections to segments.  */
      if (!assign_file_positions_for_load_sections (abfd, link_info))
	return false;

      /* And for non-load sections.  */
      if (!assign_file_positions_for_non_load_sections (abfd, link_info))
	return false;
    }

  if (!(*bed->elf_backend_modify_headers) (abfd, link_info))
    return false;

  /* Write out the program headers.  */
  alloc = i_ehdrp->e_phnum;
  if (alloc != 0)
    {
      if (link_info != NULL && ! link_info->no_warn_rwx_segments)
	{
	  /* Memory resident segments with non-zero size and RWX
	     permissions are a security risk, so we generate a warning
	     here if we are creating any.  */
	  unsigned int i;

	  for (i = 0; i < alloc; i++)
	    {
	      const Elf_Internal_Phdr * phdr = tdata->phdr + i;

	      if (phdr->p_memsz == 0)
		continue;

	      if (phdr->p_type == PT_TLS && (phdr->p_flags & PF_X))
		_bfd_error_handler (_("warning: %pB has a TLS segment"
				      " with execute permission"),
				    abfd);
	      else if (phdr->p_type == PT_LOAD
		       && ((phdr->p_flags & (PF_R | PF_W | PF_X))
			   == (PF_R | PF_W | PF_X)))
		_bfd_error_handler (_("warning: %pB has a LOAD segment"
				      " with RWX permissions"),
				    abfd);
	    }
	}

      if (bfd_seek (abfd, i_ehdrp->e_phoff, SEEK_SET) != 0
	  || bed->s->write_out_phdrs (abfd, tdata->phdr, alloc) != 0)
	return false;
    }

  return true;
}

bool
_bfd_elf_init_file_header (bfd *abfd,
			   struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  Elf_Internal_Ehdr *i_ehdrp;	/* Elf file header, internal form.  */
  struct elf_strtab_hash *shstrtab;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  i_ehdrp = elf_elfheader (abfd);

  shstrtab = _bfd_elf_strtab_init ();
  if (shstrtab == NULL)
    return false;

  elf_shstrtab (abfd) = shstrtab;

  i_ehdrp->e_ident[EI_MAG0] = ELFMAG0;
  i_ehdrp->e_ident[EI_MAG1] = ELFMAG1;
  i_ehdrp->e_ident[EI_MAG2] = ELFMAG2;
  i_ehdrp->e_ident[EI_MAG3] = ELFMAG3;

  i_ehdrp->e_ident[EI_CLASS] = bed->s->elfclass;
  i_ehdrp->e_ident[EI_DATA] =
    bfd_big_endian (abfd) ? ELFDATA2MSB : ELFDATA2LSB;
  i_ehdrp->e_ident[EI_VERSION] = bed->s->ev_current;

  if ((abfd->flags & DYNAMIC) != 0)
    i_ehdrp->e_type = ET_DYN;
  else if ((abfd->flags & EXEC_P) != 0)
    i_ehdrp->e_type = ET_EXEC;
  else if (bfd_get_format (abfd) == bfd_core)
    i_ehdrp->e_type = ET_CORE;
  else
    i_ehdrp->e_type = ET_REL;

  switch (bfd_get_arch (abfd))
    {
    case bfd_arch_unknown:
      i_ehdrp->e_machine = EM_NONE;
      break;

      /* There used to be a long list of cases here, each one setting
	 e_machine to the same EM_* macro #defined as ELF_MACHINE_CODE
	 in the corresponding bfd definition.  To avoid duplication,
	 the switch was removed.  Machines that need special handling
	 can generally do it in elf_backend_final_write_processing(),
	 unless they need the information earlier than the final write.
	 Such need can generally be supplied by replacing the tests for
	 e_machine with the conditions used to determine it.  */
    default:
      i_ehdrp->e_machine = bed->elf_machine_code;
    }

  i_ehdrp->e_version = bed->s->ev_current;
  i_ehdrp->e_ehsize = bed->s->sizeof_ehdr;

  /* No program header, for now.  */
  i_ehdrp->e_phoff = 0;
  i_ehdrp->e_phentsize = 0;
  i_ehdrp->e_phnum = 0;

  /* Each bfd section is section header entry.  */
  i_ehdrp->e_entry = bfd_get_start_address (abfd);
  i_ehdrp->e_shentsize = bed->s->sizeof_shdr;

  elf_tdata (abfd)->symtab_hdr.sh_name =
    (unsigned int) _bfd_elf_strtab_add (shstrtab, ".symtab", false);
  elf_tdata (abfd)->strtab_hdr.sh_name =
    (unsigned int) _bfd_elf_strtab_add (shstrtab, ".strtab", false);
  elf_tdata (abfd)->shstrtab_hdr.sh_name =
    (unsigned int) _bfd_elf_strtab_add (shstrtab, ".shstrtab", false);
  if (elf_tdata (abfd)->symtab_hdr.sh_name == (unsigned int) -1
      || elf_tdata (abfd)->strtab_hdr.sh_name == (unsigned int) -1
      || elf_tdata (abfd)->shstrtab_hdr.sh_name == (unsigned int) -1)
    return false;

  return true;
}

/* Set e_type in ELF header to ET_EXEC for -pie -Ttext-segment=.

   FIXME: We used to have code here to sort the PT_LOAD segments into
   ascending order, as per the ELF spec.  But this breaks some programs,
   including the Linux kernel.  But really either the spec should be
   changed or the programs updated.  */

bool
_bfd_elf_modify_headers (bfd *obfd, struct bfd_link_info *link_info)
{
  if (link_info != NULL && bfd_link_pie (link_info))
    {
      Elf_Internal_Ehdr *i_ehdrp = elf_elfheader (obfd);
      unsigned int num_segments = i_ehdrp->e_phnum;
      struct elf_obj_tdata *tdata = elf_tdata (obfd);
      Elf_Internal_Phdr *segment = tdata->phdr;
      Elf_Internal_Phdr *end_segment = &segment[num_segments];

      /* Find the lowest p_vaddr in PT_LOAD segments.  */
      bfd_vma p_vaddr = (bfd_vma) -1;
      for (; segment < end_segment; segment++)
	if (segment->p_type == PT_LOAD && p_vaddr > segment->p_vaddr)
	  p_vaddr = segment->p_vaddr;

      /* Set e_type to ET_EXEC if the lowest p_vaddr in PT_LOAD
	 segments is non-zero.  */
      if (p_vaddr)
	i_ehdrp->e_type = ET_EXEC;
    }
  return true;
}

/* Assign file positions for all the reloc sections which are not part
   of the loadable file image, and the file position of section headers.  */

static bool
_bfd_elf_assign_file_positions_for_non_load (bfd *abfd)
{
  file_ptr off;
  Elf_Internal_Shdr **shdrpp, **end_shdrpp;
  Elf_Internal_Shdr *shdrp;
  Elf_Internal_Ehdr *i_ehdrp;
  const struct elf_backend_data *bed;

  /* Skip non-load sections without section header.  */
  if ((abfd->flags & BFD_NO_SECTION_HEADER) != 0)
    return true;

  off = elf_next_file_pos (abfd);

  shdrpp = elf_elfsections (abfd);
  end_shdrpp = shdrpp + elf_numsections (abfd);
  for (shdrpp++; shdrpp < end_shdrpp; shdrpp++)
    {
      shdrp = *shdrpp;
      if (shdrp->sh_offset == -1)
	{
	  asection *sec = shdrp->bfd_section;
	  if (sec == NULL
	      || shdrp->sh_type == SHT_REL
	      || shdrp->sh_type == SHT_RELA)
	    ;
	  else if (bfd_section_is_ctf (sec))
	    {
	      /* Update section size and contents.	*/
	      shdrp->sh_size = sec->size;
	      shdrp->contents = sec->contents;
	    }
	  else if (shdrp->sh_name == -1u)
	    {
	      const char *name = sec->name;
	      struct bfd_elf_section_data *d;

	      /* Compress DWARF debug sections.  */
	      if (!bfd_compress_section (abfd, sec, shdrp->contents))
		return false;

	      if (sec->compress_status == COMPRESS_SECTION_DONE
		  && (abfd->flags & BFD_COMPRESS_GABI) == 0
		  && name[1] == 'd')
		{
		  /* If section is compressed with zlib-gnu, convert
		     section name from .debug_* to .zdebug_*.  */
		  char *new_name = bfd_debug_name_to_zdebug (abfd, name);
		  if (new_name == NULL)
		    return false;
		  name = new_name;
		}
	      /* Add section name to section name section.  */
	      shdrp->sh_name
		= (unsigned int) _bfd_elf_strtab_add (elf_shstrtab (abfd),
						      name, false);
	      d = elf_section_data (sec);

	      /* Add reloc section name to section name section.  */
	      if (d->rel.hdr
		  && !_bfd_elf_set_reloc_sh_name (abfd, d->rel.hdr,
						  name, false))
		return false;
	      if (d->rela.hdr
		  && !_bfd_elf_set_reloc_sh_name (abfd, d->rela.hdr,
						  name, true))
		return false;

	      /* Update section size and contents.  */
	      shdrp->sh_size = sec->size;
	      shdrp->contents = sec->contents;
	      sec->contents = NULL;
	    }

	  off = _bfd_elf_assign_file_position_for_section (shdrp, off, true);
	}
    }

  /* Place section name section after DWARF debug sections have been
     compressed.  */
  _bfd_elf_strtab_finalize (elf_shstrtab (abfd));
  shdrp = &elf_tdata (abfd)->shstrtab_hdr;
  shdrp->sh_size = _bfd_elf_strtab_size (elf_shstrtab (abfd));
  off = _bfd_elf_assign_file_position_for_section (shdrp, off, true);

  /* Place the section headers.  */
  i_ehdrp = elf_elfheader (abfd);
  bed = get_elf_backend_data (abfd);
  off = align_file_position (off, 1 << bed->s->log_file_align);
  i_ehdrp->e_shoff = off;
  off += i_ehdrp->e_shnum * i_ehdrp->e_shentsize;
  elf_next_file_pos (abfd) = off;

  return true;
}

bool
_bfd_elf_write_object_contents (bfd *abfd)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  Elf_Internal_Shdr **i_shdrp;
  bool failed;
  unsigned int count, num_sec;
  struct elf_obj_tdata *t;

  if (! abfd->output_has_begun
      && ! _bfd_elf_compute_section_file_positions (abfd, NULL))
    return false;
  /* Do not rewrite ELF data when the BFD has been opened for update.
     abfd->output_has_begun was set to TRUE on opening, so creation of
     new sections, and modification of existing section sizes was
     restricted.  This means the ELF header, program headers and
     section headers can't have changed.  If the contents of any
     sections has been modified, then those changes have already been
     written to the BFD.  */
  else if (abfd->direction == both_direction)
    {
      BFD_ASSERT (abfd->output_has_begun);
      return true;
    }

  i_shdrp = elf_elfsections (abfd);

  failed = false;
  bfd_map_over_sections (abfd, bed->s->write_relocs, &failed);
  if (failed)
    return false;

  if (!_bfd_elf_assign_file_positions_for_non_load (abfd))
    return false;

  /* After writing the headers, we need to write the sections too...  */
  num_sec = elf_numsections (abfd);
  for (count = 1; count < num_sec; count++)
    {
      /* Don't set the sh_name field without section header.  */
      if ((abfd->flags & BFD_NO_SECTION_HEADER) == 0)
	i_shdrp[count]->sh_name
	  = _bfd_elf_strtab_offset (elf_shstrtab (abfd),
				    i_shdrp[count]->sh_name);
      if (bed->elf_backend_section_processing)
	if (!(*bed->elf_backend_section_processing) (abfd, i_shdrp[count]))
	  return false;
      if (i_shdrp[count]->contents)
	{
	  bfd_size_type amt = i_shdrp[count]->sh_size;

	  if (bfd_seek (abfd, i_shdrp[count]->sh_offset, SEEK_SET) != 0
	      || bfd_bwrite (i_shdrp[count]->contents, amt, abfd) != amt)
	    return false;
	}
    }

  /* Write out the section header names.  */
  t = elf_tdata (abfd);
  if (elf_shstrtab (abfd) != NULL
      && t->shstrtab_hdr.sh_offset != -1
      && (bfd_seek (abfd, t->shstrtab_hdr.sh_offset, SEEK_SET) != 0
	  || !_bfd_elf_strtab_emit (abfd, elf_shstrtab (abfd))))
    return false;

  if (!(*bed->elf_backend_final_write_processing) (abfd))
    return false;

  if (!bed->s->write_shdrs_and_ehdr (abfd))
    return false;

  /* This is last since write_shdrs_and_ehdr can touch i_shdrp[0].  */
  if (t->o->build_id.after_write_object_contents != NULL
      && !(*t->o->build_id.after_write_object_contents) (abfd))
    return false;
  if (t->o->package_metadata.after_write_object_contents != NULL
      && !(*t->o->package_metadata.after_write_object_contents) (abfd))
    return false;

  return true;
}

bool
_bfd_elf_write_corefile_contents (bfd *abfd)
{
  /* Hopefully this can be done just like an object file.  */
  return _bfd_elf_write_object_contents (abfd);
}

/* Given a section, search the header to find them.  */

unsigned int
_bfd_elf_section_from_bfd_section (bfd *abfd, struct bfd_section *asect)
{
  const struct elf_backend_data *bed;
  unsigned int sec_index;

  if (elf_section_data (asect) != NULL
      && elf_section_data (asect)->this_idx != 0)
    return elf_section_data (asect)->this_idx;

  if (bfd_is_abs_section (asect))
    sec_index = SHN_ABS;
  else if (bfd_is_com_section (asect))
    sec_index = SHN_COMMON;
  else if (bfd_is_und_section (asect))
    sec_index = SHN_UNDEF;
  else
    sec_index = SHN_BAD;

  bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_section_from_bfd_section)
    {
      int retval = sec_index;

      if ((*bed->elf_backend_section_from_bfd_section) (abfd, asect, &retval))
	return retval;
    }

  if (sec_index == SHN_BAD)
    bfd_set_error (bfd_error_nonrepresentable_section);

  return sec_index;
}

/* Given a BFD symbol, return the index in the ELF symbol table, or -1
   on error.  */

int
_bfd_elf_symbol_from_bfd_symbol (bfd *abfd, asymbol **asym_ptr_ptr)
{
  asymbol *asym_ptr = *asym_ptr_ptr;
  int idx;
  flagword flags = asym_ptr->flags;

  /* When gas creates relocations against local labels, it creates its
     own symbol for the section, but does put the symbol into the
     symbol chain, so udata is 0.  When the linker is generating
     relocatable output, this section symbol may be for one of the
     input sections rather than the output section.  */
  if (asym_ptr->udata.i == 0
      && (flags & BSF_SECTION_SYM)
      && asym_ptr->section)
    {
      asection *sec;

      sec = asym_ptr->section;
      if (sec->owner != abfd && sec->output_section != NULL)
	sec = sec->output_section;
      if (sec->owner == abfd
	  && sec->index < elf_num_section_syms (abfd)
	  && elf_section_syms (abfd)[sec->index] != NULL)
	asym_ptr->udata.i = elf_section_syms (abfd)[sec->index]->udata.i;
    }

  idx = asym_ptr->udata.i;

  if (idx == 0)
    {
      /* This case can occur when using --strip-symbol on a symbol
	 which is used in a relocation entry.  */
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: symbol `%s' required but not present"),
	 abfd, bfd_asymbol_name (asym_ptr));
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

#if DEBUG & 4
  {
    fprintf (stderr,
	     "elf_symbol_from_bfd_symbol 0x%.8lx, name = %s, sym num = %d,"
	     " flags = 0x%.8x\n",
	     (long) asym_ptr, asym_ptr->name, idx, flags);
    fflush (stderr);
  }
#endif

  return idx;
}

static inline bfd_vma
segment_size (Elf_Internal_Phdr *segment)
{
  return (segment->p_memsz > segment->p_filesz
	  ? segment->p_memsz : segment->p_filesz);
}


/* Returns the end address of the segment + 1.  */
static inline bfd_vma
segment_end (Elf_Internal_Phdr *segment, bfd_vma start)
{
  return start + segment_size (segment);
}

static inline bfd_size_type
section_size (asection *section, Elf_Internal_Phdr *segment)
{
  if ((section->flags & SEC_HAS_CONTENTS) != 0
      || (section->flags & SEC_THREAD_LOCAL) == 0
      || segment->p_type == PT_TLS)
    return section->size;
  return 0;
}

/* Returns TRUE if the given section is contained within the given
   segment.  LMA addresses are compared against PADDR when
   USE_VADDR is false, VMA against VADDR when true.  */
static bool
is_contained_by (asection *section, Elf_Internal_Phdr *segment,
		 bfd_vma paddr, bfd_vma vaddr, unsigned int opb,
		 bool use_vaddr)
{
  bfd_vma seg_addr = !use_vaddr ? paddr : vaddr;
  bfd_vma addr = !use_vaddr ? section->lma : section->vma;
  bfd_vma octet;
  if (_bfd_mul_overflow (addr, opb, &octet))
    return false;
  /* The third and fourth lines below are testing that the section end
     address is within the segment.  It's written this way to avoid
     overflow.  Add seg_addr + section_size to both sides of the
     inequality to make it obvious.  */
  return (octet >= seg_addr
	  && segment_size (segment) >= section_size (section, segment)
	  && (octet - seg_addr
	      <= segment_size (segment) - section_size (section, segment)));
}

/* Handle PT_NOTE segment.  */
static bool
is_note (asection *s, Elf_Internal_Phdr *p)
{
  return (p->p_type == PT_NOTE
	  && elf_section_type (s) == SHT_NOTE
	  && (ufile_ptr) s->filepos >= p->p_offset
	  && p->p_filesz >= s->size
	  && (ufile_ptr) s->filepos - p->p_offset <= p->p_filesz - s->size);
}

/* Rewrite program header information.  */

static bool
rewrite_elf_program_header (bfd *ibfd, bfd *obfd, bfd_vma maxpagesize)
{
  Elf_Internal_Ehdr *iehdr;
  struct elf_segment_map *map;
  struct elf_segment_map *map_first;
  struct elf_segment_map **pointer_to_map;
  Elf_Internal_Phdr *segment;
  asection *section;
  unsigned int i;
  unsigned int num_segments;
  bool phdr_included = false;
  bool p_paddr_valid;
  struct elf_segment_map *phdr_adjust_seg = NULL;
  unsigned int phdr_adjust_num = 0;
  const struct elf_backend_data *bed;
  unsigned int opb = bfd_octets_per_byte (ibfd, NULL);

  bed = get_elf_backend_data (ibfd);
  iehdr = elf_elfheader (ibfd);

  map_first = NULL;
  pointer_to_map = &map_first;

  num_segments = elf_elfheader (ibfd)->e_phnum;

  /* The complicated case when p_vaddr is 0 is to handle the Solaris
     linker, which generates a PT_INTERP section with p_vaddr and
     p_memsz set to 0.  */
#define IS_SOLARIS_PT_INTERP(p, s)					\
  (p->p_vaddr == 0							\
   && p->p_paddr == 0							\
   && p->p_memsz == 0							\
   && p->p_filesz > 0							\
   && (s->flags & SEC_HAS_CONTENTS) != 0				\
   && s->size > 0							\
   && (bfd_vma) s->filepos >= p->p_offset				\
   && ((bfd_vma) s->filepos + s->size					\
       <= p->p_offset + p->p_filesz))

  /* Decide if the given section should be included in the given segment.
     A section will be included if:
       1. It is within the address space of the segment -- we use the LMA
	  if that is set for the segment and the VMA otherwise,
       2. It is an allocated section or a NOTE section in a PT_NOTE
	  segment.
       3. There is an output section associated with it,
       4. The section has not already been allocated to a previous segment.
       5. PT_GNU_STACK segments do not include any sections.
       6. PT_TLS segment includes only SHF_TLS sections.
       7. SHF_TLS sections are only in PT_TLS or PT_LOAD segments.
       8. PT_DYNAMIC should not contain empty sections at the beginning
	  (with the possible exception of .dynamic).  */
#define IS_SECTION_IN_INPUT_SEGMENT(section, segment, bed, opb)		\
  (((is_contained_by (section, segment, segment->p_paddr,		\
		      segment->p_vaddr, opb,				\
		      bed->want_p_paddr_set_to_zero)			\
     && (section->flags & SEC_ALLOC) != 0)				\
    || is_note (section, segment))					\
   && segment->p_type != PT_GNU_STACK					\
   && (segment->p_type != PT_TLS					\
       || (section->flags & SEC_THREAD_LOCAL))				\
   && (segment->p_type == PT_LOAD					\
       || segment->p_type == PT_TLS					\
       || (section->flags & SEC_THREAD_LOCAL) == 0)			\
   && (segment->p_type != PT_DYNAMIC					\
       || section_size (section, segment) > 0				\
       || (segment->p_paddr						\
	   ? segment->p_paddr != section->lma * (opb)			\
	   : segment->p_vaddr != section->vma * (opb))			\
       || (strcmp (bfd_section_name (section), ".dynamic") == 0))	\
   && (segment->p_type != PT_LOAD || !section->segment_mark))

/* If the output section of a section in the input segment is NULL,
   it is removed from the corresponding output segment.   */
#define INCLUDE_SECTION_IN_SEGMENT(section, segment, bed, opb)		\
  (IS_SECTION_IN_INPUT_SEGMENT (section, segment, bed, opb)		\
   && section->output_section != NULL)

  /* Returns TRUE iff seg1 starts after the end of seg2.  */
#define SEGMENT_AFTER_SEGMENT(seg1, seg2, field)			\
  (seg1->field >= segment_end (seg2, seg2->field))

  /* Returns TRUE iff seg1 and seg2 overlap. Segments overlap iff both
     their VMA address ranges and their LMA address ranges overlap.
     It is possible to have overlapping VMA ranges without overlapping LMA
     ranges.  RedBoot images for example can have both .data and .bss mapped
     to the same VMA range, but with the .data section mapped to a different
     LMA.  */
#define SEGMENT_OVERLAPS(seg1, seg2)					\
  (   !(SEGMENT_AFTER_SEGMENT (seg1, seg2, p_vaddr)			\
	|| SEGMENT_AFTER_SEGMENT (seg2, seg1, p_vaddr))			\
   && !(SEGMENT_AFTER_SEGMENT (seg1, seg2, p_paddr)			\
	|| SEGMENT_AFTER_SEGMENT (seg2, seg1, p_paddr)))

  /* Initialise the segment mark field, and discard stupid alignment.  */
  for (section = ibfd->sections; section != NULL; section = section->next)
    {
      asection *o = section->output_section;
      if (o != NULL && o->alignment_power >= (sizeof (bfd_vma) * 8) - 1)
	o->alignment_power = 0;
      section->segment_mark = false;
    }

  /* The Solaris linker creates program headers in which all the
     p_paddr fields are zero.  When we try to objcopy or strip such a
     file, we get confused.  Check for this case, and if we find it
     don't set the p_paddr_valid fields.  */
  p_paddr_valid = false;
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    if (segment->p_paddr != 0)
      {
	p_paddr_valid = true;
	break;
      }

  /* Scan through the segments specified in the program header
     of the input BFD.  For this first scan we look for overlaps
     in the loadable segments.  These can be created by weird
     parameters to objcopy.  Also, fix some solaris weirdness.  */
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    {
      unsigned int j;
      Elf_Internal_Phdr *segment2;

      if (segment->p_type == PT_INTERP)
	for (section = ibfd->sections; section; section = section->next)
	  if (IS_SOLARIS_PT_INTERP (segment, section))
	    {
	      /* Mininal change so that the normal section to segment
		 assignment code will work.  */
	      segment->p_vaddr = section->vma * opb;
	      break;
	    }

      if (segment->p_type != PT_LOAD)
	{
	  /* Remove PT_GNU_RELRO segment.  */
	  if (segment->p_type == PT_GNU_RELRO)
	    segment->p_type = PT_NULL;
	  continue;
	}

      /* Determine if this segment overlaps any previous segments.  */
      for (j = 0, segment2 = elf_tdata (ibfd)->phdr; j < i; j++, segment2++)
	{
	  bfd_signed_vma extra_length;

	  if (segment2->p_type != PT_LOAD
	      || !SEGMENT_OVERLAPS (segment, segment2))
	    continue;

	  /* Merge the two segments together.  */
	  if (segment2->p_vaddr < segment->p_vaddr)
	    {
	      /* Extend SEGMENT2 to include SEGMENT and then delete
		 SEGMENT.  */
	      extra_length = (segment_end (segment, segment->p_vaddr)
			      - segment_end (segment2, segment2->p_vaddr));

	      if (extra_length > 0)
		{
		  segment2->p_memsz += extra_length;
		  segment2->p_filesz += extra_length;
		}

	      segment->p_type = PT_NULL;

	      /* Since we have deleted P we must restart the outer loop.  */
	      i = 0;
	      segment = elf_tdata (ibfd)->phdr;
	      break;
	    }
	  else
	    {
	      /* Extend SEGMENT to include SEGMENT2 and then delete
		 SEGMENT2.  */
	      extra_length = (segment_end (segment2, segment2->p_vaddr)
			      - segment_end (segment, segment->p_vaddr));

	      if (extra_length > 0)
		{
		  segment->p_memsz += extra_length;
		  segment->p_filesz += extra_length;
		}

	      segment2->p_type = PT_NULL;
	    }
	}
    }

  /* The second scan attempts to assign sections to segments.  */
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    {
      unsigned int section_count;
      asection **sections;
      asection *output_section;
      unsigned int isec;
      asection *matching_lma;
      asection *suggested_lma;
      unsigned int j;
      size_t amt;
      asection *first_section;

      if (segment->p_type == PT_NULL)
	continue;

      first_section = NULL;
      /* Compute how many sections might be placed into this segment.  */
      for (section = ibfd->sections, section_count = 0;
	   section != NULL;
	   section = section->next)
	{
	  /* Find the first section in the input segment, which may be
	     removed from the corresponding output segment.   */
	  if (IS_SECTION_IN_INPUT_SEGMENT (section, segment, bed, opb))
	    {
	      if (first_section == NULL)
		first_section = section;
	      if (section->output_section != NULL)
		++section_count;
	    }
	}

      /* Allocate a segment map big enough to contain
	 all of the sections we have selected.  */
      amt = sizeof (struct elf_segment_map) - sizeof (asection *);
      amt += section_count * sizeof (asection *);
      map = (struct elf_segment_map *) bfd_zalloc (obfd, amt);
      if (map == NULL)
	return false;

      /* Initialise the fields of the segment map.  Default to
	 using the physical address of the segment in the input BFD.  */
      map->next = NULL;
      map->p_type = segment->p_type;
      map->p_flags = segment->p_flags;
      map->p_flags_valid = 1;

      if (map->p_type == PT_LOAD
	  && (ibfd->flags & D_PAGED) != 0
	  && maxpagesize > 1
	  && segment->p_align > 1)
	{
	  map->p_align = segment->p_align;
	  if (segment->p_align > maxpagesize)
	    map->p_align = maxpagesize;
	  map->p_align_valid = 1;
	}

      /* If the first section in the input segment is removed, there is
	 no need to preserve segment physical address in the corresponding
	 output segment.  */
      if (!first_section || first_section->output_section != NULL)
	{
	  map->p_paddr = segment->p_paddr;
	  map->p_paddr_valid = p_paddr_valid;
	}

      /* Determine if this segment contains the ELF file header
	 and if it contains the program headers themselves.  */
      map->includes_filehdr = (segment->p_offset == 0
			       && segment->p_filesz >= iehdr->e_ehsize);
      map->includes_phdrs = 0;

      if (!phdr_included || segment->p_type != PT_LOAD)
	{
	  map->includes_phdrs =
	    (segment->p_offset <= (bfd_vma) iehdr->e_phoff
	     && (segment->p_offset + segment->p_filesz
		 >= ((bfd_vma) iehdr->e_phoff
		     + iehdr->e_phnum * iehdr->e_phentsize)));

	  if (segment->p_type == PT_LOAD && map->includes_phdrs)
	    phdr_included = true;
	}

      if (section_count == 0)
	{
	  /* Special segments, such as the PT_PHDR segment, may contain
	     no sections, but ordinary, loadable segments should contain
	     something.  They are allowed by the ELF spec however, so only
	     a warning is produced.
	     Don't warn if an empty PT_LOAD contains the program headers.
	     There is however the valid use case of embedded systems which
	     have segments with p_filesz of 0 and a p_memsz > 0 to initialize
	     flash memory with zeros.  No warning is shown for that case.  */
	  if (segment->p_type == PT_LOAD
	      && !map->includes_phdrs
	      && (segment->p_filesz > 0 || segment->p_memsz == 0))
	    /* xgettext:c-format */
	    _bfd_error_handler
	      (_("%pB: warning: empty loadable segment detected"
		 " at vaddr=%#" PRIx64 ", is this intentional?"),
	       ibfd, (uint64_t) segment->p_vaddr);

	  map->p_vaddr_offset = segment->p_vaddr / opb;
	  map->count = 0;
	  *pointer_to_map = map;
	  pointer_to_map = &map->next;

	  continue;
	}

      /* Now scan the sections in the input BFD again and attempt
	 to add their corresponding output sections to the segment map.
	 The problem here is how to handle an output section which has
	 been moved (ie had its LMA changed).  There are four possibilities:

	 1. None of the sections have been moved.
	    In this case we can continue to use the segment LMA from the
	    input BFD.

	 2. All of the sections have been moved by the same amount.
	    In this case we can change the segment's LMA to match the LMA
	    of the first section.

	 3. Some of the sections have been moved, others have not.
	    In this case those sections which have not been moved can be
	    placed in the current segment which will have to have its size,
	    and possibly its LMA changed, and a new segment or segments will
	    have to be created to contain the other sections.

	 4. The sections have been moved, but not by the same amount.
	    In this case we can change the segment's LMA to match the LMA
	    of the first section and we will have to create a new segment
	    or segments to contain the other sections.

	 In order to save time, we allocate an array to hold the section
	 pointers that we are interested in.  As these sections get assigned
	 to a segment, they are removed from this array.  */

      amt = section_count * sizeof (asection *);
      sections = (asection **) bfd_malloc (amt);
      if (sections == NULL)
	return false;

      /* Step One: Scan for segment vs section LMA conflicts.
	 Also add the sections to the section array allocated above.
	 Also add the sections to the current segment.  In the common
	 case, where the sections have not been moved, this means that
	 we have completely filled the segment, and there is nothing
	 more to do.  */
      isec = 0;
      matching_lma = NULL;
      suggested_lma = NULL;

      for (section = first_section, j = 0;
	   section != NULL;
	   section = section->next)
	{
	  if (INCLUDE_SECTION_IN_SEGMENT (section, segment, bed, opb))
	    {
	      output_section = section->output_section;

	      sections[j++] = section;

	      /* The Solaris native linker always sets p_paddr to 0.
		 We try to catch that case here, and set it to the
		 correct value.  Note - some backends require that
		 p_paddr be left as zero.  */
	      if (!p_paddr_valid
		  && segment->p_vaddr != 0
		  && !bed->want_p_paddr_set_to_zero
		  && isec == 0
		  && output_section->lma != 0
		  && (align_power (segment->p_vaddr
				   + (map->includes_filehdr
				      ? iehdr->e_ehsize : 0)
				   + (map->includes_phdrs
				      ? iehdr->e_phnum * iehdr->e_phentsize
				      : 0),
				   output_section->alignment_power * opb)
		      == (output_section->vma * opb)))
		map->p_paddr = segment->p_vaddr;

	      /* Match up the physical address of the segment with the
		 LMA address of the output section.  */
	      if (is_contained_by (output_section, segment, map->p_paddr,
				   0, opb, false)
		  || is_note (section, segment))
		{
		  if (matching_lma == NULL
		      || output_section->lma < matching_lma->lma)
		    matching_lma = output_section;

		  /* We assume that if the section fits within the segment
		     then it does not overlap any other section within that
		     segment.  */
		  map->sections[isec++] = output_section;
		}
	      else if (suggested_lma == NULL)
		suggested_lma = output_section;

	      if (j == section_count)
		break;
	    }
	}

      BFD_ASSERT (j == section_count);

      /* Step Two: Adjust the physical address of the current segment,
	 if necessary.  */
      if (isec == section_count)
	{
	  /* All of the sections fitted within the segment as currently
	     specified.  This is the default case.  Add the segment to
	     the list of built segments and carry on to process the next
	     program header in the input BFD.  */
	  map->count = section_count;
	  *pointer_to_map = map;
	  pointer_to_map = &map->next;

	  if (p_paddr_valid
	      && !bed->want_p_paddr_set_to_zero)
	    {
	      bfd_vma hdr_size = 0;
	      if (map->includes_filehdr)
		hdr_size = iehdr->e_ehsize;
	      if (map->includes_phdrs)
		hdr_size += iehdr->e_phnum * iehdr->e_phentsize;

	      /* Account for padding before the first section in the
		 segment.  */
	      map->p_vaddr_offset = ((map->p_paddr + hdr_size) / opb
				     - matching_lma->lma);
	    }

	  free (sections);
	  continue;
	}
      else
	{
	  /* Change the current segment's physical address to match
	     the LMA of the first section that fitted, or if no
	     section fitted, the first section.  */
	  if (matching_lma == NULL)
	    matching_lma = suggested_lma;

	  map->p_paddr = matching_lma->lma * opb;

	  /* Offset the segment physical address from the lma
	     to allow for space taken up by elf headers.  */
	  if (map->includes_phdrs)
	    {
	      map->p_paddr -= iehdr->e_phnum * iehdr->e_phentsize;

	      /* iehdr->e_phnum is just an estimate of the number
		 of program headers that we will need.  Make a note
		 here of the number we used and the segment we chose
		 to hold these headers, so that we can adjust the
		 offset when we know the correct value.  */
	      phdr_adjust_num = iehdr->e_phnum;
	      phdr_adjust_seg = map;
	    }

	  if (map->includes_filehdr)
	    {
	      bfd_vma align = (bfd_vma) 1 << matching_lma->alignment_power;
	      map->p_paddr -= iehdr->e_ehsize;
	      /* We've subtracted off the size of headers from the
		 first section lma, but there may have been some
		 alignment padding before that section too.  Try to
		 account for that by adjusting the segment lma down to
		 the same alignment.  */
	      if (segment->p_align != 0 && segment->p_align < align)
		align = segment->p_align;
	      map->p_paddr &= -(align * opb);
	    }
	}

      /* Step Three: Loop over the sections again, this time assigning
	 those that fit to the current segment and removing them from the
	 sections array; but making sure not to leave large gaps.  Once all
	 possible sections have been assigned to the current segment it is
	 added to the list of built segments and if sections still remain
	 to be assigned, a new segment is constructed before repeating
	 the loop.  */
      isec = 0;
      do
	{
	  map->count = 0;
	  suggested_lma = NULL;

	  /* Fill the current segment with sections that fit.  */
	  for (j = 0; j < section_count; j++)
	    {
	      section = sections[j];

	      if (section == NULL)
		continue;

	      output_section = section->output_section;

	      BFD_ASSERT (output_section != NULL);

	      if (is_contained_by (output_section, segment, map->p_paddr,
				   0, opb, false)
		  || is_note (section, segment))
		{
		  if (map->count == 0)
		    {
		      /* If the first section in a segment does not start at
			 the beginning of the segment, then something is
			 wrong.  */
		      if (align_power (map->p_paddr
				       + (map->includes_filehdr
					  ? iehdr->e_ehsize : 0)
				       + (map->includes_phdrs
					  ? iehdr->e_phnum * iehdr->e_phentsize
					  : 0),
				       output_section->alignment_power * opb)
			  != output_section->lma * opb)
			goto sorry;
		    }
		  else
		    {
		      asection *prev_sec;

		      prev_sec = map->sections[map->count - 1];

		      /* If the gap between the end of the previous section
			 and the start of this section is more than
			 maxpagesize then we need to start a new segment.  */
		      if ((BFD_ALIGN (prev_sec->lma + prev_sec->size,
				      maxpagesize)
			   < BFD_ALIGN (output_section->lma, maxpagesize))
			  || (prev_sec->lma + prev_sec->size
			      > output_section->lma))
			{
			  if (suggested_lma == NULL)
			    suggested_lma = output_section;

			  continue;
			}
		    }

		  map->sections[map->count++] = output_section;
		  ++isec;
		  sections[j] = NULL;
		  if (segment->p_type == PT_LOAD)
		    section->segment_mark = true;
		}
	      else if (suggested_lma == NULL)
		suggested_lma = output_section;
	    }

	  /* PR 23932.  A corrupt input file may contain sections that cannot
	     be assigned to any segment - because for example they have a
	     negative size - or segments that do not contain any sections.
	     But there are also valid reasons why a segment can be empty.
	     So allow a count of zero.  */

	  /* Add the current segment to the list of built segments.  */
	  *pointer_to_map = map;
	  pointer_to_map = &map->next;

	  if (isec < section_count)
	    {
	      /* We still have not allocated all of the sections to
		 segments.  Create a new segment here, initialise it
		 and carry on looping.  */
	      amt = sizeof (struct elf_segment_map) - sizeof (asection *);
	      amt += section_count * sizeof (asection *);
	      map = (struct elf_segment_map *) bfd_zalloc (obfd, amt);
	      if (map == NULL)
		{
		  free (sections);
		  return false;
		}

	      /* Initialise the fields of the segment map.  Set the physical
		 physical address to the LMA of the first section that has
		 not yet been assigned.  */
	      map->next = NULL;
	      map->p_type = segment->p_type;
	      map->p_flags = segment->p_flags;
	      map->p_flags_valid = 1;
	      map->p_paddr = suggested_lma->lma * opb;
	      map->p_paddr_valid = p_paddr_valid;
	      map->includes_filehdr = 0;
	      map->includes_phdrs = 0;
	    }

	  continue;
	sorry:
	  bfd_set_error (bfd_error_sorry);
	  free (sections);
	  return false;
	}
      while (isec < section_count);

      free (sections);
    }

  elf_seg_map (obfd) = map_first;

  /* If we had to estimate the number of program headers that were
     going to be needed, then check our estimate now and adjust
     the offset if necessary.  */
  if (phdr_adjust_seg != NULL)
    {
      unsigned int count;

      for (count = 0, map = map_first; map != NULL; map = map->next)
	count++;

      if (count > phdr_adjust_num)
	phdr_adjust_seg->p_paddr
	  -= (count - phdr_adjust_num) * iehdr->e_phentsize;

      for (map = map_first; map != NULL; map = map->next)
	if (map->p_type == PT_PHDR)
	  {
	    bfd_vma adjust
	      = phdr_adjust_seg->includes_filehdr ? iehdr->e_ehsize : 0;
	    map->p_paddr = phdr_adjust_seg->p_paddr + adjust;
	    break;
	  }
    }

#undef IS_SOLARIS_PT_INTERP
#undef IS_SECTION_IN_INPUT_SEGMENT
#undef INCLUDE_SECTION_IN_SEGMENT
#undef SEGMENT_AFTER_SEGMENT
#undef SEGMENT_OVERLAPS
  return true;
}

/* Return true if p_align in the ELF program header in ABFD is valid.  */

static bool
elf_is_p_align_valid (bfd *abfd)
{
  unsigned int i;
  Elf_Internal_Phdr *segment;
  unsigned int num_segments;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  bfd_size_type maxpagesize = bed->maxpagesize;
  bfd_size_type p_align = bed->p_align;

  /* Return true if the default p_align value isn't set or the maximum
     page size is the same as the minimum page size.  */
  if (p_align == 0 || maxpagesize == bed->minpagesize)
    return true;

  /* When the default p_align value is set, p_align may be set to the
     default p_align value while segments are aligned to the maximum
     page size.  In this case, the input p_align will be ignored and
     the maximum page size will be used to align the output segments.  */
  segment = elf_tdata (abfd)->phdr;
  num_segments = elf_elfheader (abfd)->e_phnum;
  for (i = 0; i < num_segments; i++, segment++)
    if (segment->p_type == PT_LOAD
	&& (segment->p_align != p_align
	    || vma_page_aligned_bias (segment->p_vaddr,
				      segment->p_offset,
				      maxpagesize) != 0))
      return true;

  return false;
}

/* Copy ELF program header information.  */

static bool
copy_elf_program_header (bfd *ibfd, bfd *obfd)
{
  Elf_Internal_Ehdr *iehdr;
  struct elf_segment_map *map;
  struct elf_segment_map *map_first;
  struct elf_segment_map **pointer_to_map;
  Elf_Internal_Phdr *segment;
  unsigned int i;
  unsigned int num_segments;
  bool phdr_included = false;
  bool p_paddr_valid;
  bool p_palign_valid;
  unsigned int opb = bfd_octets_per_byte (ibfd, NULL);

  iehdr = elf_elfheader (ibfd);

  map_first = NULL;
  pointer_to_map = &map_first;

  /* If all the segment p_paddr fields are zero, don't set
     map->p_paddr_valid.  */
  p_paddr_valid = false;
  num_segments = elf_elfheader (ibfd)->e_phnum;
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    if (segment->p_paddr != 0)
      {
	p_paddr_valid = true;
	break;
      }

  p_palign_valid = elf_is_p_align_valid (ibfd);

  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    {
      asection *section;
      unsigned int section_count;
      size_t amt;
      Elf_Internal_Shdr *this_hdr;
      asection *first_section = NULL;
      asection *lowest_section;

      /* Compute how many sections are in this segment.  */
      for (section = ibfd->sections, section_count = 0;
	   section != NULL;
	   section = section->next)
	{
	  this_hdr = &(elf_section_data(section)->this_hdr);
	  if (ELF_SECTION_IN_SEGMENT (this_hdr, segment))
	    {
	      if (first_section == NULL)
		first_section = section;
	      section_count++;
	    }
	}

      /* Allocate a segment map big enough to contain
	 all of the sections we have selected.  */
      amt = sizeof (struct elf_segment_map) - sizeof (asection *);
      amt += section_count * sizeof (asection *);
      map = (struct elf_segment_map *) bfd_zalloc (obfd, amt);
      if (map == NULL)
	return false;

      /* Initialize the fields of the output segment map with the
	 input segment.  */
      map->next = NULL;
      map->p_type = segment->p_type;
      map->p_flags = segment->p_flags;
      map->p_flags_valid = 1;
      map->p_paddr = segment->p_paddr;
      map->p_paddr_valid = p_paddr_valid;
      map->p_align = segment->p_align;
      /* Keep p_align of PT_GNU_STACK for stack alignment.  */
      map->p_align_valid = (map->p_type == PT_GNU_STACK
			    || p_palign_valid);
      map->p_vaddr_offset = 0;

      if (map->p_type == PT_GNU_RELRO
	  || map->p_type == PT_GNU_STACK)
	{
	  /* The PT_GNU_RELRO segment may contain the first a few
	     bytes in the .got.plt section even if the whole .got.plt
	     section isn't in the PT_GNU_RELRO segment.  We won't
	     change the size of the PT_GNU_RELRO segment.
	     Similarly, PT_GNU_STACK size is significant on uclinux
	     systems.    */
	  map->p_size = segment->p_memsz;
	  map->p_size_valid = 1;
	}

      /* Determine if this segment contains the ELF file header
	 and if it contains the program headers themselves.  */
      map->includes_filehdr = (segment->p_offset == 0
			       && segment->p_filesz >= iehdr->e_ehsize);

      map->includes_phdrs = 0;
      if (! phdr_included || segment->p_type != PT_LOAD)
	{
	  map->includes_phdrs =
	    (segment->p_offset <= (bfd_vma) iehdr->e_phoff
	     && (segment->p_offset + segment->p_filesz
		 >= ((bfd_vma) iehdr->e_phoff
		     + iehdr->e_phnum * iehdr->e_phentsize)));

	  if (segment->p_type == PT_LOAD && map->includes_phdrs)
	    phdr_included = true;
	}

      lowest_section = NULL;
      if (section_count != 0)
	{
	  unsigned int isec = 0;

	  for (section = first_section;
	       section != NULL;
	       section = section->next)
	    {
	      this_hdr = &(elf_section_data(section)->this_hdr);
	      if (ELF_SECTION_IN_SEGMENT (this_hdr, segment))
		{
		  map->sections[isec++] = section->output_section;
		  if ((section->flags & SEC_ALLOC) != 0)
		    {
		      bfd_vma seg_off;

		      if (lowest_section == NULL
			  || section->lma < lowest_section->lma)
			lowest_section = section;

		      /* Section lmas are set up from PT_LOAD header
			 p_paddr in _bfd_elf_make_section_from_shdr.
			 If this header has a p_paddr that disagrees
			 with the section lma, flag the p_paddr as
			 invalid.  */
		      if ((section->flags & SEC_LOAD) != 0)
			seg_off = this_hdr->sh_offset - segment->p_offset;
		      else
			seg_off = this_hdr->sh_addr - segment->p_vaddr;
		      if (section->lma * opb - segment->p_paddr != seg_off)
			map->p_paddr_valid = false;
		    }
		  if (isec == section_count)
		    break;
		}
	    }
	}

      if (section_count == 0)
	map->p_vaddr_offset = segment->p_vaddr / opb;
      else if (map->p_paddr_valid)
	{
	  /* Account for padding before the first section in the segment.  */
	  bfd_vma hdr_size = 0;
	  if (map->includes_filehdr)
	    hdr_size = iehdr->e_ehsize;
	  if (map->includes_phdrs)
	    hdr_size += iehdr->e_phnum * iehdr->e_phentsize;

	  map->p_vaddr_offset = ((map->p_paddr + hdr_size) / opb
				 - (lowest_section ? lowest_section->lma : 0));
	}

      map->count = section_count;
      *pointer_to_map = map;
      pointer_to_map = &map->next;
    }

  elf_seg_map (obfd) = map_first;
  return true;
}

/* Copy private BFD data.  This copies or rewrites ELF program header
   information.  */

static bool
copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  bfd_vma maxpagesize;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  if (elf_tdata (ibfd)->phdr == NULL)
    return true;

  if (ibfd->xvec == obfd->xvec)
    {
      /* Check to see if any sections in the input BFD
	 covered by ELF program header have changed.  */
      Elf_Internal_Phdr *segment;
      asection *section, *osec;
      unsigned int i, num_segments;
      Elf_Internal_Shdr *this_hdr;
      const struct elf_backend_data *bed;

      bed = get_elf_backend_data (ibfd);

      /* Regenerate the segment map if p_paddr is set to 0.  */
      if (bed->want_p_paddr_set_to_zero)
	goto rewrite;

      /* Initialize the segment mark field.  */
      for (section = obfd->sections; section != NULL;
	   section = section->next)
	section->segment_mark = false;

      num_segments = elf_elfheader (ibfd)->e_phnum;
      for (i = 0, segment = elf_tdata (ibfd)->phdr;
	   i < num_segments;
	   i++, segment++)
	{
	  /* PR binutils/3535.  The Solaris linker always sets the p_paddr
	     and p_memsz fields of special segments (DYNAMIC, INTERP) to 0
	     which severly confuses things, so always regenerate the segment
	     map in this case.  */
	  if (segment->p_paddr == 0
	      && segment->p_memsz == 0
	      && (segment->p_type == PT_INTERP
		  || segment->p_type == PT_DYNAMIC))
	    goto rewrite;

	  for (section = ibfd->sections;
	       section != NULL; section = section->next)
	    {
	      /* We mark the output section so that we know it comes
		 from the input BFD.  */
	      osec = section->output_section;
	      if (osec)
		osec->segment_mark = true;

	      /* Check if this section is covered by the segment.  */
	      this_hdr = &(elf_section_data(section)->this_hdr);
	      if (ELF_SECTION_IN_SEGMENT (this_hdr, segment))
		{
		  /* FIXME: Check if its output section is changed or
		     removed.  What else do we need to check?  */
		  if (osec == NULL
		      || section->flags != osec->flags
		      || section->lma != osec->lma
		      || section->vma != osec->vma
		      || section->size != osec->size
		      || section->rawsize != osec->rawsize
		      || section->alignment_power != osec->alignment_power)
		    goto rewrite;
		}
	    }
	}

      /* Check to see if any output section do not come from the
	 input BFD.  */
      for (section = obfd->sections; section != NULL;
	   section = section->next)
	{
	  if (!section->segment_mark)
	    goto rewrite;
	  else
	    section->segment_mark = false;
	}

      return copy_elf_program_header (ibfd, obfd);
    }

 rewrite:
  maxpagesize = 0;
  if (ibfd->xvec == obfd->xvec)
    {
      /* When rewriting program header, set the output maxpagesize to
	 the maximum alignment of input PT_LOAD segments.  */
      Elf_Internal_Phdr *segment;
      unsigned int i;
      unsigned int num_segments = elf_elfheader (ibfd)->e_phnum;

      for (i = 0, segment = elf_tdata (ibfd)->phdr;
	   i < num_segments;
	   i++, segment++)
	if (segment->p_type == PT_LOAD
	    && maxpagesize < segment->p_align)
	  {
	    /* PR 17512: file: f17299af.  */
	    if (segment->p_align > (bfd_vma) 1 << ((sizeof (bfd_vma) * 8) - 2))
	      /* xgettext:c-format */
	      _bfd_error_handler (_("%pB: warning: segment alignment of %#"
				    PRIx64 " is too large"),
				  ibfd, (uint64_t) segment->p_align);
	    else
	      maxpagesize = segment->p_align;
	  }
    }
  if (maxpagesize == 0)
    maxpagesize = get_elf_backend_data (obfd)->maxpagesize;

  return rewrite_elf_program_header (ibfd, obfd, maxpagesize);
}

/* Initialize private output section information from input section.  */

bool
_bfd_elf_init_private_section_data (bfd *ibfd,
				    asection *isec,
				    bfd *obfd,
				    asection *osec,
				    struct bfd_link_info *link_info)

{
  Elf_Internal_Shdr *ihdr, *ohdr;
  bool final_link = (link_info != NULL
		     && !bfd_link_relocatable (link_info));

  if (ibfd->xvec->flavour != bfd_target_elf_flavour
      || obfd->xvec->flavour != bfd_target_elf_flavour)
    return true;

  BFD_ASSERT (elf_section_data (osec) != NULL);

  /* If this is a known ABI section, ELF section type and flags may
     have been set up when OSEC was created.  For normal sections we
     allow the user to override the type and flags other than
     SHF_MASKOS and SHF_MASKPROC.  */
  if (elf_section_type (osec) == SHT_PROGBITS
      || elf_section_type (osec) == SHT_NOTE
      || elf_section_type (osec) == SHT_NOBITS)
    elf_section_type (osec) = SHT_NULL;
  /* For objcopy and relocatable link, copy the ELF section type from
     the input file if the BFD section flags are the same.  (If they
     are different the user may be doing something like
     "objcopy --set-section-flags .text=alloc,data".)  For a final
     link allow some flags that the linker clears to differ.  */
  if (elf_section_type (osec) == SHT_NULL
      && (osec->flags == isec->flags
	  || (final_link
	      && ((osec->flags ^ isec->flags)
		  & ~(SEC_LINK_ONCE | SEC_LINK_DUPLICATES | SEC_RELOC)) == 0)))
    elf_section_type (osec) = elf_section_type (isec);

  /* FIXME: Is this correct for all OS/PROC specific flags?  */
  elf_section_flags (osec) = (elf_section_flags (isec)
			      & (SHF_MASKOS | SHF_MASKPROC));

  /* Copy sh_info from input for mbind section.  */
  if ((elf_tdata (ibfd)->has_gnu_osabi & elf_gnu_osabi_mbind) != 0
      && elf_section_flags (isec) & SHF_GNU_MBIND)
    elf_section_data (osec)->this_hdr.sh_info
      = elf_section_data (isec)->this_hdr.sh_info;

  /* Set things up for objcopy and relocatable link.  The output
     SHT_GROUP section will have its elf_next_in_group pointing back
     to the input group members.  Ignore linker created group section.
     See elfNN_ia64_object_p in elfxx-ia64.c.  */
  if ((link_info == NULL
       || !link_info->resolve_section_groups)
      && (elf_sec_group (isec) == NULL
	  || (elf_sec_group (isec)->flags & SEC_LINKER_CREATED) == 0))
    {
      if (elf_section_flags (isec) & SHF_GROUP)
	elf_section_flags (osec) |= SHF_GROUP;
      elf_next_in_group (osec) = elf_next_in_group (isec);
      elf_section_data (osec)->group = elf_section_data (isec)->group;
    }

  /* If not decompress, preserve SHF_COMPRESSED.  */
  if (!final_link && (ibfd->flags & BFD_DECOMPRESS) == 0)
    elf_section_flags (osec) |= (elf_section_flags (isec)
				 & SHF_COMPRESSED);

  ihdr = &elf_section_data (isec)->this_hdr;

  /* We need to handle elf_linked_to_section for SHF_LINK_ORDER. We
     don't use the output section of the linked-to section since it
     may be NULL at this point.  */
  if ((ihdr->sh_flags & SHF_LINK_ORDER) != 0)
    {
      ohdr = &elf_section_data (osec)->this_hdr;
      ohdr->sh_flags |= SHF_LINK_ORDER;
      elf_linked_to_section (osec) = elf_linked_to_section (isec);
    }

  osec->use_rela_p = isec->use_rela_p;

  return true;
}

/* Copy private section information.  This copies over the entsize
   field, and sometimes the info field.  */

bool
_bfd_elf_copy_private_section_data (bfd *ibfd,
				    asection *isec,
				    bfd *obfd,
				    asection *osec)
{
  Elf_Internal_Shdr *ihdr, *ohdr;

  if (ibfd->xvec->flavour != bfd_target_elf_flavour
      || obfd->xvec->flavour != bfd_target_elf_flavour)
    return true;

  ihdr = &elf_section_data (isec)->this_hdr;
  ohdr = &elf_section_data (osec)->this_hdr;

  ohdr->sh_entsize = ihdr->sh_entsize;

  if (ihdr->sh_type == SHT_SYMTAB
      || ihdr->sh_type == SHT_DYNSYM
      || ihdr->sh_type == SHT_GNU_verneed
      || ihdr->sh_type == SHT_GNU_verdef)
    ohdr->sh_info = ihdr->sh_info;

  return _bfd_elf_init_private_section_data (ibfd, isec, obfd, osec,
					     NULL);
}

/* Look at all the SHT_GROUP sections in IBFD, making any adjustments
   necessary if we are removing either the SHT_GROUP section or any of
   the group member sections.  DISCARDED is the value that a section's
   output_section has if the section will be discarded, NULL when this
   function is called from objcopy, bfd_abs_section_ptr when called
   from the linker.  */

bool
_bfd_elf_fixup_group_sections (bfd *ibfd, asection *discarded)
{
  asection *isec;

  for (isec = ibfd->sections; isec != NULL; isec = isec->next)
    if (elf_section_type (isec) == SHT_GROUP)
      {
	asection *first = elf_next_in_group (isec);
	asection *s = first;
	bfd_size_type removed = 0;

	while (s != NULL)
	  {
	    /* If this member section is being output but the
	       SHT_GROUP section is not, then clear the group info
	       set up by _bfd_elf_copy_private_section_data.  */
	    if (s->output_section != discarded
		&& isec->output_section == discarded)
	      {
		elf_section_flags (s->output_section) &= ~SHF_GROUP;
		elf_group_name (s->output_section) = NULL;
	      }
	    else
	      {
		struct bfd_elf_section_data *elf_sec = elf_section_data (s);
		if (s->output_section == discarded
		    && isec->output_section != discarded)
		  {
		    /* Conversely, if the member section is not being
		       output but the SHT_GROUP section is, then adjust
		       its size.  */
		    removed += 4;
		    if (elf_sec->rel.hdr != NULL
			&& (elf_sec->rel.hdr->sh_flags & SHF_GROUP) != 0)
		      removed += 4;
		    if (elf_sec->rela.hdr != NULL
			&& (elf_sec->rela.hdr->sh_flags & SHF_GROUP) != 0)
		      removed += 4;
		  }
		else
		  {
		    /* Also adjust for zero-sized relocation member
		       section.  */
		    if (elf_sec->rel.hdr != NULL
			&& elf_sec->rel.hdr->sh_size == 0)
		      removed += 4;
		    if (elf_sec->rela.hdr != NULL
			&& elf_sec->rela.hdr->sh_size == 0)
		      removed += 4;
		  }
	      }
	    s = elf_next_in_group (s);
	    if (s == first)
	      break;
	  }
	if (removed != 0)
	  {
	    if (discarded != NULL)
	      {
		/* If we've been called for ld -r, then we need to
		   adjust the input section size.  */
		if (isec->rawsize == 0)
		  isec->rawsize = isec->size;
		isec->size = isec->rawsize - removed;
		if (isec->size <= 4)
		  {
		    isec->size = 0;
		    isec->flags |= SEC_EXCLUDE;
		  }
	      }
	    else if (isec->output_section != NULL)
	      {
		/* Adjust the output section size when called from
		   objcopy. */
		isec->output_section->size -= removed;
		if (isec->output_section->size <= 4)
		  {
		    isec->output_section->size = 0;
		    isec->output_section->flags |= SEC_EXCLUDE;
		  }
	      }
	  }
      }

  return true;
}

/* Copy private header information.  */

bool
_bfd_elf_copy_private_header_data (bfd *ibfd, bfd *obfd)
{
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  /* Copy over private BFD data if it has not already been copied.
     This must be done here, rather than in the copy_private_bfd_data
     entry point, because the latter is called after the section
     contents have been set, which means that the program headers have
     already been worked out.  */
  if (elf_seg_map (obfd) == NULL && elf_tdata (ibfd)->phdr != NULL)
    {
      if (! copy_private_bfd_data (ibfd, obfd))
	return false;
    }

  return _bfd_elf_fixup_group_sections (ibfd, NULL);
}

/* Copy private symbol information.  If this symbol is in a section
   which we did not map into a BFD section, try to map the section
   index correctly.  We use special macro definitions for the mapped
   section indices; these definitions are interpreted by the
   swap_out_syms function.  */

#define MAP_ONESYMTAB (SHN_HIOS + 1)
#define MAP_DYNSYMTAB (SHN_HIOS + 2)
#define MAP_STRTAB    (SHN_HIOS + 3)
#define MAP_SHSTRTAB  (SHN_HIOS + 4)
#define MAP_SYM_SHNDX (SHN_HIOS + 5)

bool
_bfd_elf_copy_private_symbol_data (bfd *ibfd,
				   asymbol *isymarg,
				   bfd *obfd,
				   asymbol *osymarg)
{
  elf_symbol_type *isym, *osym;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  isym = elf_symbol_from (isymarg);
  osym = elf_symbol_from (osymarg);

  if (isym != NULL
      && isym->internal_elf_sym.st_shndx != 0
      && osym != NULL
      && bfd_is_abs_section (isym->symbol.section))
    {
      unsigned int shndx;

      shndx = isym->internal_elf_sym.st_shndx;
      if (shndx == elf_onesymtab (ibfd))
	shndx = MAP_ONESYMTAB;
      else if (shndx == elf_dynsymtab (ibfd))
	shndx = MAP_DYNSYMTAB;
      else if (shndx == elf_strtab_sec (ibfd))
	shndx = MAP_STRTAB;
      else if (shndx == elf_shstrtab_sec (ibfd))
	shndx = MAP_SHSTRTAB;
      else if (find_section_in_list (shndx, elf_symtab_shndx_list (ibfd)))
	shndx = MAP_SYM_SHNDX;
      osym->internal_elf_sym.st_shndx = shndx;
    }

  return true;
}

/* Swap out the symbols.  */

static bool
swap_out_syms (bfd *abfd,
	       struct elf_strtab_hash **sttp,
	       int relocatable_p,
	       struct bfd_link_info *info)
{
  const struct elf_backend_data *bed;
  unsigned int symcount;
  asymbol **syms;
  struct elf_strtab_hash *stt;
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Shdr *symtab_shndx_hdr;
  Elf_Internal_Shdr *symstrtab_hdr;
  struct elf_sym_strtab *symstrtab;
  bfd_byte *outbound_syms;
  bfd_byte *outbound_shndx;
  unsigned long outbound_syms_index;
  unsigned int idx;
  unsigned int num_locals;
  size_t amt;
  bool name_local_sections;

  if (!elf_map_symbols (abfd, &num_locals))
    return false;

  /* Dump out the symtabs.  */
  stt = _bfd_elf_strtab_init ();
  if (stt == NULL)
    return false;

  bed = get_elf_backend_data (abfd);
  symcount = bfd_get_symcount (abfd);
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  symtab_hdr->sh_type = SHT_SYMTAB;
  symtab_hdr->sh_entsize = bed->s->sizeof_sym;
  symtab_hdr->sh_size = symtab_hdr->sh_entsize * (symcount + 1);
  symtab_hdr->sh_info = num_locals + 1;
  symtab_hdr->sh_addralign = (bfd_vma) 1 << bed->s->log_file_align;

  symstrtab_hdr = &elf_tdata (abfd)->strtab_hdr;
  symstrtab_hdr->sh_type = SHT_STRTAB;

  /* Allocate buffer to swap out the .strtab section.  */
  if (_bfd_mul_overflow (symcount + 1, sizeof (*symstrtab), &amt)
      || (symstrtab = (struct elf_sym_strtab *) bfd_malloc (amt)) == NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      _bfd_elf_strtab_free (stt);
      return false;
    }

  if (_bfd_mul_overflow (symcount + 1, bed->s->sizeof_sym, &amt)
      || (outbound_syms = (bfd_byte *) bfd_alloc (abfd, amt)) == NULL)
    {
    error_no_mem:
      bfd_set_error (bfd_error_no_memory);
    error_return:
      free (symstrtab);
      _bfd_elf_strtab_free (stt);
      return false;
    }
  symtab_hdr->contents = outbound_syms;
  outbound_syms_index = 0;

  outbound_shndx = NULL;

  if (elf_symtab_shndx_list (abfd))
    {
      symtab_shndx_hdr = & elf_symtab_shndx_list (abfd)->hdr;
      if (symtab_shndx_hdr->sh_name != 0)
	{
	  if (_bfd_mul_overflow (symcount + 1,
				 sizeof (Elf_External_Sym_Shndx), &amt))
	    goto error_no_mem;
	  outbound_shndx =  (bfd_byte *) bfd_zalloc (abfd, amt);
	  if (outbound_shndx == NULL)
	    goto error_return;

	  symtab_shndx_hdr->contents = outbound_shndx;
	  symtab_shndx_hdr->sh_type = SHT_SYMTAB_SHNDX;
	  symtab_shndx_hdr->sh_size = amt;
	  symtab_shndx_hdr->sh_addralign = sizeof (Elf_External_Sym_Shndx);
	  symtab_shndx_hdr->sh_entsize = sizeof (Elf_External_Sym_Shndx);
	}
      /* FIXME: What about any other headers in the list ?  */
    }

  /* Now generate the data (for "contents").  */
  {
    /* Fill in zeroth symbol and swap it out.  */
    Elf_Internal_Sym sym;
    sym.st_name = 0;
    sym.st_value = 0;
    sym.st_size = 0;
    sym.st_info = 0;
    sym.st_other = 0;
    sym.st_shndx = SHN_UNDEF;
    sym.st_target_internal = 0;
    symstrtab[0].sym = sym;
    symstrtab[0].dest_index = outbound_syms_index;
    outbound_syms_index++;
  }

  name_local_sections
    = (bed->elf_backend_name_local_section_symbols
       && bed->elf_backend_name_local_section_symbols (abfd));

  syms = bfd_get_outsymbols (abfd);
  for (idx = 0; idx < symcount;)
    {
      Elf_Internal_Sym sym;
      bfd_vma value = syms[idx]->value;
      elf_symbol_type *type_ptr;
      flagword flags = syms[idx]->flags;
      int type;

      if (!name_local_sections
	  && (flags & (BSF_SECTION_SYM | BSF_GLOBAL)) == BSF_SECTION_SYM)
	{
	  /* Local section symbols have no name.  */
	  sym.st_name = (unsigned long) -1;
	}
      else
	{
	  /* Call _bfd_elf_strtab_offset after _bfd_elf_strtab_finalize
	     to get the final offset for st_name.  */
	  sym.st_name
	    = (unsigned long) _bfd_elf_strtab_add (stt, syms[idx]->name,
						   false);
	  if (sym.st_name == (unsigned long) -1)
	    goto error_return;
	}

      type_ptr = elf_symbol_from (syms[idx]);

      if ((flags & BSF_SECTION_SYM) == 0
	  && bfd_is_com_section (syms[idx]->section))
	{
	  /* ELF common symbols put the alignment into the `value' field,
	     and the size into the `size' field.  This is backwards from
	     how BFD handles it, so reverse it here.  */
	  sym.st_size = value;
	  if (type_ptr == NULL
	      || type_ptr->internal_elf_sym.st_value == 0)
	    sym.st_value = value >= 16 ? 16 : (1 << bfd_log2 (value));
	  else
	    sym.st_value = type_ptr->internal_elf_sym.st_value;
	  sym.st_shndx = _bfd_elf_section_from_bfd_section
	    (abfd, syms[idx]->section);
	}
      else
	{
	  asection *sec = syms[idx]->section;
	  unsigned int shndx;

	  if (sec->output_section)
	    {
	      value += sec->output_offset;
	      sec = sec->output_section;
	    }

	  /* Don't add in the section vma for relocatable output.  */
	  if (! relocatable_p)
	    value += sec->vma;
	  sym.st_value = value;
	  sym.st_size = type_ptr ? type_ptr->internal_elf_sym.st_size : 0;

	  if (bfd_is_abs_section (sec)
	      && type_ptr != NULL
	      && type_ptr->internal_elf_sym.st_shndx != 0)
	    {
	      /* This symbol is in a real ELF section which we did
		 not create as a BFD section.  Undo the mapping done
		 by copy_private_symbol_data.  */
	      shndx = type_ptr->internal_elf_sym.st_shndx;
	      switch (shndx)
		{
		case MAP_ONESYMTAB:
		  shndx = elf_onesymtab (abfd);
		  break;
		case MAP_DYNSYMTAB:
		  shndx = elf_dynsymtab (abfd);
		  break;
		case MAP_STRTAB:
		  shndx = elf_strtab_sec (abfd);
		  break;
		case MAP_SHSTRTAB:
		  shndx = elf_shstrtab_sec (abfd);
		  break;
		case MAP_SYM_SHNDX:
		  if (elf_symtab_shndx_list (abfd))
		    shndx = elf_symtab_shndx_list (abfd)->ndx;
		  break;
		case SHN_COMMON:
		case SHN_ABS:
		  shndx = SHN_ABS;
		  break;
		default:
		  if (shndx >= SHN_LOPROC && shndx <= SHN_HIOS)
		    {
		      if (bed->symbol_section_index)
			shndx = bed->symbol_section_index (abfd, type_ptr);
		      /* Otherwise just leave the index alone.  */
		    }
		  else
		    {
		      if (shndx > SHN_HIOS && shndx < SHN_HIRESERVE)
			_bfd_error_handler (_("%pB: \
Unable to handle section index %x in ELF symbol.  Using ABS instead."),
					  abfd, shndx);
		      shndx = SHN_ABS;
		    }
		  break;
		}
	    }
	  else
	    {
	      shndx = _bfd_elf_section_from_bfd_section (abfd, sec);

	      if (shndx == SHN_BAD)
		{
		  asection *sec2;

		  /* Writing this would be a hell of a lot easier if
		     we had some decent documentation on bfd, and
		     knew what to expect of the library, and what to
		     demand of applications.  For example, it
		     appears that `objcopy' might not set the
		     section of a symbol to be a section that is
		     actually in the output file.  */
		  sec2 = bfd_get_section_by_name (abfd, sec->name);
		  if (sec2 != NULL)
		    shndx = _bfd_elf_section_from_bfd_section (abfd, sec2);
		  if (shndx == SHN_BAD)
		    {
		      /* xgettext:c-format */
		      _bfd_error_handler
			(_("unable to find equivalent output section"
			   " for symbol '%s' from section '%s'"),
			 syms[idx]->name ? syms[idx]->name : "<Local sym>",
			 sec->name);
		      bfd_set_error (bfd_error_invalid_operation);
		      goto error_return;
		    }
		}
	    }

	  sym.st_shndx = shndx;
	}

      if ((flags & BSF_THREAD_LOCAL) != 0)
	type = STT_TLS;
      else if ((flags & BSF_GNU_INDIRECT_FUNCTION) != 0)
	type = STT_GNU_IFUNC;
      else if ((flags & BSF_FUNCTION) != 0)
	type = STT_FUNC;
      else if ((flags & BSF_OBJECT) != 0)
	type = STT_OBJECT;
      else if ((flags & BSF_RELC) != 0)
	type = STT_RELC;
      else if ((flags & BSF_SRELC) != 0)
	type = STT_SRELC;
      else
	type = STT_NOTYPE;

      if (syms[idx]->section->flags & SEC_THREAD_LOCAL)
	type = STT_TLS;

      /* Processor-specific types.  */
      if (type_ptr != NULL
	  && bed->elf_backend_get_symbol_type)
	type = ((*bed->elf_backend_get_symbol_type)
		(&type_ptr->internal_elf_sym, type));

      if (flags & BSF_SECTION_SYM)
	{
	  if (flags & BSF_GLOBAL)
	    sym.st_info = ELF_ST_INFO (STB_GLOBAL, STT_SECTION);
	  else
	    sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_SECTION);
	}
      else if (bfd_is_com_section (syms[idx]->section))
	{
	  if (type != STT_TLS)
	    {
	      if ((abfd->flags & BFD_CONVERT_ELF_COMMON))
		type = ((abfd->flags & BFD_USE_ELF_STT_COMMON)
			? STT_COMMON : STT_OBJECT);
	      else
		type = ((flags & BSF_ELF_COMMON) != 0
			? STT_COMMON : STT_OBJECT);
	    }
	  sym.st_info = ELF_ST_INFO (STB_GLOBAL, type);
	}
      else if (bfd_is_und_section (syms[idx]->section))
	sym.st_info = ELF_ST_INFO (((flags & BSF_WEAK)
				    ? STB_WEAK
				    : STB_GLOBAL),
				   type);
      else if (flags & BSF_FILE)
	sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_FILE);
      else
	{
	  int bind = STB_LOCAL;

	  if (flags & BSF_LOCAL)
	    bind = STB_LOCAL;
	  else if (flags & BSF_GNU_UNIQUE)
	    bind = STB_GNU_UNIQUE;
	  else if (flags & BSF_WEAK)
	    bind = STB_WEAK;
	  else if (flags & BSF_GLOBAL)
	    bind = STB_GLOBAL;

	  sym.st_info = ELF_ST_INFO (bind, type);
	}

      if (type_ptr != NULL)
	{
	  sym.st_other = type_ptr->internal_elf_sym.st_other;
	  sym.st_target_internal
	    = type_ptr->internal_elf_sym.st_target_internal;
	}
      else
	{
	  sym.st_other = 0;
	  sym.st_target_internal = 0;
	}

      idx++;
      symstrtab[idx].sym = sym;
      symstrtab[idx].dest_index = outbound_syms_index;

      outbound_syms_index++;
    }

  /* Finalize the .strtab section.  */
  _bfd_elf_strtab_finalize (stt);

  /* Swap out the .strtab section.  */
  for (idx = 0; idx <= symcount; idx++)
    {
      struct elf_sym_strtab *elfsym = &symstrtab[idx];
      if (elfsym->sym.st_name == (unsigned long) -1)
	elfsym->sym.st_name = 0;
      else
	elfsym->sym.st_name = _bfd_elf_strtab_offset (stt,
						      elfsym->sym.st_name);
      if (info && info->callbacks->ctf_new_symbol)
	info->callbacks->ctf_new_symbol (elfsym->dest_index,
					 &elfsym->sym);

      /* Inform the linker of the addition of this symbol.  */

      bed->s->swap_symbol_out (abfd, &elfsym->sym,
			       (outbound_syms
				+ (elfsym->dest_index
				   * bed->s->sizeof_sym)),
			       NPTR_ADD (outbound_shndx,
					 (elfsym->dest_index
					  * sizeof (Elf_External_Sym_Shndx))));
    }
  free (symstrtab);

  *sttp = stt;
  symstrtab_hdr->sh_size = _bfd_elf_strtab_size (stt);
  symstrtab_hdr->sh_type = SHT_STRTAB;
  symstrtab_hdr->sh_flags = bed->elf_strtab_flags;
  symstrtab_hdr->sh_addr = 0;
  symstrtab_hdr->sh_entsize = 0;
  symstrtab_hdr->sh_link = 0;
  symstrtab_hdr->sh_info = 0;
  symstrtab_hdr->sh_addralign = 1;

  return true;
}

/* Return the number of bytes required to hold the symtab vector.

   Note that we base it on the count plus 1, since we will null terminate
   the vector allocated based on this size.  However, the ELF symbol table
   always has a dummy entry as symbol #0, so it ends up even.  */

long
_bfd_elf_get_symtab_upper_bound (bfd *abfd)
{
  bfd_size_type symcount;
  long symtab_size;
  Elf_Internal_Shdr *hdr = &elf_tdata (abfd)->symtab_hdr;

  symcount = hdr->sh_size / get_elf_backend_data (abfd)->s->sizeof_sym;
  if (symcount > LONG_MAX / sizeof (asymbol *))
    {
      bfd_set_error (bfd_error_file_too_big);
      return -1;
    }
  symtab_size = symcount * (sizeof (asymbol *));
  if (symcount == 0)
    symtab_size = sizeof (asymbol *);
  else if (!bfd_write_p (abfd))
    {
      ufile_ptr filesize = bfd_get_file_size (abfd);

      if (filesize != 0 && (unsigned long) symtab_size > filesize)
	{
	  bfd_set_error (bfd_error_file_truncated);
	  return -1;
	}
    }

  return symtab_size;
}

long
_bfd_elf_get_dynamic_symtab_upper_bound (bfd *abfd)
{
  bfd_size_type symcount;
  long symtab_size;
  Elf_Internal_Shdr *hdr = &elf_tdata (abfd)->dynsymtab_hdr;

  if (elf_dynsymtab (abfd) == 0)
    {
      /* Check if there is dynamic symbol table.  */
      symcount = elf_tdata (abfd)->dt_symtab_count;
      if (symcount)
	goto compute_symtab_size;

      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  symcount = hdr->sh_size / get_elf_backend_data (abfd)->s->sizeof_sym;
  if (symcount > LONG_MAX / sizeof (asymbol *))
    {
      bfd_set_error (bfd_error_file_too_big);
      return -1;
    }

 compute_symtab_size:
  symtab_size = symcount * (sizeof (asymbol *));
  if (symcount == 0)
    symtab_size = sizeof (asymbol *);
  else if (!bfd_write_p (abfd))
    {
      ufile_ptr filesize = bfd_get_file_size (abfd);

      if (filesize != 0 && (unsigned long) symtab_size > filesize)
	{
	  bfd_set_error (bfd_error_file_truncated);
	  return -1;
	}
    }

  return symtab_size;
}

long
_bfd_elf_get_reloc_upper_bound (bfd *abfd, sec_ptr asect)
{
  if (asect->reloc_count != 0 && !bfd_write_p (abfd))
    {
      /* Sanity check reloc section size.  */
      ufile_ptr filesize = bfd_get_file_size (abfd);

      if (filesize != 0)
	{
	  struct bfd_elf_section_data *d = elf_section_data (asect);
	  bfd_size_type rel_size = d->rel.hdr ? d->rel.hdr->sh_size : 0;
	  bfd_size_type rela_size = d->rela.hdr ? d->rela.hdr->sh_size : 0;

	  if (rel_size + rela_size > filesize
	      || rel_size + rela_size < rel_size)
	    {
	      bfd_set_error (bfd_error_file_truncated);
	      return -1;
	    }
	}
    }

#if SIZEOF_LONG == SIZEOF_INT
  if (asect->reloc_count >= LONG_MAX / sizeof (arelent *))
    {
      bfd_set_error (bfd_error_file_too_big);
      return -1;
    }
#endif
  return (asect->reloc_count + 1L) * sizeof (arelent *);
}

/* Canonicalize the relocs.  */

long
_bfd_elf_canonicalize_reloc (bfd *abfd,
			     sec_ptr section,
			     arelent **relptr,
			     asymbol **symbols)
{
  arelent *tblptr;
  unsigned int i;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (! bed->s->slurp_reloc_table (abfd, section, symbols, false))
    return -1;

  tblptr = section->relocation;
  for (i = 0; i < section->reloc_count; i++)
    *relptr++ = tblptr++;

  *relptr = NULL;

  return section->reloc_count;
}

long
_bfd_elf_canonicalize_symtab (bfd *abfd, asymbol **allocation)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  long symcount = bed->s->slurp_symbol_table (abfd, allocation, false);

  if (symcount >= 0)
    abfd->symcount = symcount;
  return symcount;
}

long
_bfd_elf_canonicalize_dynamic_symtab (bfd *abfd,
				      asymbol **allocation)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  long symcount = bed->s->slurp_symbol_table (abfd, allocation, true);

  if (symcount >= 0)
    abfd->dynsymcount = symcount;
  return symcount;
}

/* Return the size required for the dynamic reloc entries.  Any loadable
   section that was actually installed in the BFD, and has type SHT_REL
   or SHT_RELA, and uses the dynamic symbol table, is considered to be a
   dynamic reloc section.  */

long
_bfd_elf_get_dynamic_reloc_upper_bound (bfd *abfd)
{
  bfd_size_type count, ext_rel_size;
  asection *s;

  if (elf_dynsymtab (abfd) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  count = 1;
  ext_rel_size = 0;
  for (s = abfd->sections; s != NULL; s = s->next)
    if (elf_section_data (s)->this_hdr.sh_link == elf_dynsymtab (abfd)
	&& (elf_section_data (s)->this_hdr.sh_type == SHT_REL
	    || elf_section_data (s)->this_hdr.sh_type == SHT_RELA)
	&& (elf_section_data (s)->this_hdr.sh_flags & SHF_COMPRESSED) == 0)
      {
	ext_rel_size += elf_section_data (s)->this_hdr.sh_size;
	if (ext_rel_size < elf_section_data (s)->this_hdr.sh_size)
	  {
	    bfd_set_error (bfd_error_file_truncated);
	    return -1;
	  }
	count += NUM_SHDR_ENTRIES (&elf_section_data (s)->this_hdr);
	if (count > LONG_MAX / sizeof (arelent *))
	  {
	    bfd_set_error (bfd_error_file_too_big);
	    return -1;
	  }
      }
  if (count > 1 && !bfd_write_p (abfd))
    {
      /* Sanity check reloc section sizes.  */
      ufile_ptr filesize = bfd_get_file_size (abfd);
      if (filesize != 0 && ext_rel_size > filesize)
	{
	  bfd_set_error (bfd_error_file_truncated);
	  return -1;
	}
    }
  return count * sizeof (arelent *);
}

/* Canonicalize the dynamic relocation entries.  Note that we return the
   dynamic relocations as a single block, although they are actually
   associated with particular sections; the interface, which was
   designed for SunOS style shared libraries, expects that there is only
   one set of dynamic relocs.  Any loadable section that was actually
   installed in the BFD, and has type SHT_REL or SHT_RELA, and uses the
   dynamic symbol table, is considered to be a dynamic reloc section.  */

long
_bfd_elf_canonicalize_dynamic_reloc (bfd *abfd,
				     arelent **storage,
				     asymbol **syms)
{
  bool (*slurp_relocs) (bfd *, asection *, asymbol **, bool);
  asection *s;
  long ret;

  if (elf_dynsymtab (abfd) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  slurp_relocs = get_elf_backend_data (abfd)->s->slurp_reloc_table;
  ret = 0;
  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if (elf_section_data (s)->this_hdr.sh_link == elf_dynsymtab (abfd)
	  && (elf_section_data (s)->this_hdr.sh_type == SHT_REL
	      || elf_section_data (s)->this_hdr.sh_type == SHT_RELA)
	  && (elf_section_data (s)->this_hdr.sh_flags & SHF_COMPRESSED) == 0)
	{
	  arelent *p;
	  long count, i;

	  if (! (*slurp_relocs) (abfd, s, syms, true))
	    return -1;
	  count = NUM_SHDR_ENTRIES (&elf_section_data (s)->this_hdr);
	  p = s->relocation;
	  for (i = 0; i < count; i++)
	    *storage++ = p++;
	  ret += count;
	}
    }

  *storage = NULL;

  return ret;
}

/* Read in the version information.  */

bool
_bfd_elf_slurp_version_tables (bfd *abfd, bool default_imported_symver)
{
  bfd_byte *contents = NULL;
  unsigned int freeidx = 0;
  size_t amt;

  if (elf_dynverref (abfd) != 0 || elf_tdata (abfd)->dt_verneed != NULL)
    {
      Elf_Internal_Shdr *hdr;
      Elf_External_Verneed *everneed;
      Elf_Internal_Verneed *iverneed;
      unsigned int i;
      bfd_byte *contents_end;
      size_t verneed_count;
      size_t verneed_size;

      if (elf_tdata (abfd)->dt_verneed != NULL)
	{
	  hdr = NULL;
	  contents = elf_tdata (abfd)->dt_verneed;
	  verneed_count = elf_tdata (abfd)->dt_verneed_count;
	  verneed_size = verneed_count * sizeof (Elf_External_Verneed);
	}
      else
	{
	  hdr = &elf_tdata (abfd)->dynverref_hdr;

	  if (hdr->sh_info > hdr->sh_size / sizeof (Elf_External_Verneed))
	    {
	    error_return_bad_verref:
	      _bfd_error_handler
		(_("%pB: .gnu.version_r invalid entry"), abfd);
	      bfd_set_error (bfd_error_bad_value);
	    error_return_verref:
	      elf_tdata (abfd)->verref = NULL;
	      elf_tdata (abfd)->cverrefs = 0;
	      goto error_return;
	    }

	  if (bfd_seek (abfd, hdr->sh_offset, SEEK_SET) != 0)
	    goto error_return_verref;
	  contents = _bfd_malloc_and_read (abfd, hdr->sh_size, hdr->sh_size);
	  if (contents == NULL)
	    goto error_return_verref;

	  verneed_size = hdr->sh_size;
	  verneed_count = hdr->sh_info;
	}

      if (_bfd_mul_overflow (verneed_count,
			     sizeof (Elf_Internal_Verneed), &amt))
	{
	  bfd_set_error (bfd_error_file_too_big);
	  goto error_return_verref;
	}
      if (amt == 0)
	goto error_return_verref;
      elf_tdata (abfd)->verref = (Elf_Internal_Verneed *) bfd_zalloc (abfd, amt);
      if (elf_tdata (abfd)->verref == NULL)
	goto error_return_verref;

      BFD_ASSERT (sizeof (Elf_External_Verneed)
		  == sizeof (Elf_External_Vernaux));
      contents_end = (contents + verneed_size
		      - sizeof (Elf_External_Verneed));
      everneed = (Elf_External_Verneed *) contents;
      iverneed = elf_tdata (abfd)->verref;
      for (i = 0; i < verneed_count; i++, iverneed++)
	{
	  Elf_External_Vernaux *evernaux;
	  Elf_Internal_Vernaux *ivernaux;
	  unsigned int j;

	  _bfd_elf_swap_verneed_in (abfd, everneed, iverneed);

	  iverneed->vn_bfd = abfd;

	  if (elf_use_dt_symtab_p (abfd))
	    iverneed->vn_filename
	      = elf_tdata (abfd)->dt_strtab + iverneed->vn_file;
	  else
	    iverneed->vn_filename
	      = bfd_elf_string_from_elf_section (abfd, hdr->sh_link,
						 iverneed->vn_file);
	  if (iverneed->vn_filename == NULL)
	    goto error_return_bad_verref;

	  if (iverneed->vn_cnt == 0)
	    iverneed->vn_auxptr = NULL;
	  else
	    {
	      if (_bfd_mul_overflow (iverneed->vn_cnt,
				     sizeof (Elf_Internal_Vernaux), &amt))
		{
		  bfd_set_error (bfd_error_file_too_big);
		  goto error_return_verref;
		}
	      iverneed->vn_auxptr = (struct elf_internal_vernaux *)
		bfd_alloc (abfd, amt);
	      if (iverneed->vn_auxptr == NULL)
		goto error_return_verref;
	    }

	  if (iverneed->vn_aux
	      > (size_t) (contents_end - (bfd_byte *) everneed))
	    goto error_return_bad_verref;

	  evernaux = ((Elf_External_Vernaux *)
		      ((bfd_byte *) everneed + iverneed->vn_aux));
	  ivernaux = iverneed->vn_auxptr;
	  for (j = 0; j < iverneed->vn_cnt; j++, ivernaux++)
	    {
	      _bfd_elf_swap_vernaux_in (abfd, evernaux, ivernaux);

	      if (elf_use_dt_symtab_p (abfd))
		ivernaux->vna_nodename
		  = elf_tdata (abfd)->dt_strtab + ivernaux->vna_name;
	      else
		ivernaux->vna_nodename
		  = bfd_elf_string_from_elf_section (abfd, hdr->sh_link,
						     ivernaux->vna_name);
	      if (ivernaux->vna_nodename == NULL)
		goto error_return_bad_verref;

	      if (ivernaux->vna_other > freeidx)
		freeidx = ivernaux->vna_other;

	      ivernaux->vna_nextptr = NULL;
	      if (ivernaux->vna_next == 0)
		{
		  iverneed->vn_cnt = j + 1;
		  break;
		}
	      if (j + 1 < iverneed->vn_cnt)
		ivernaux->vna_nextptr = ivernaux + 1;

	      if (ivernaux->vna_next
		  > (size_t) (contents_end - (bfd_byte *) evernaux))
		goto error_return_bad_verref;

	      evernaux = ((Elf_External_Vernaux *)
			  ((bfd_byte *) evernaux + ivernaux->vna_next));
	    }

	  iverneed->vn_nextref = NULL;
	  if (iverneed->vn_next == 0)
	    break;
	  if (i + 1 < hdr->sh_info)
	    iverneed->vn_nextref = iverneed + 1;

	  if (iverneed->vn_next
	      > (size_t) (contents_end - (bfd_byte *) everneed))
	    goto error_return_bad_verref;

	  everneed = ((Elf_External_Verneed *)
		      ((bfd_byte *) everneed + iverneed->vn_next));
	}
      elf_tdata (abfd)->cverrefs = i;

      if (elf_tdata (abfd)->dt_verneed == NULL)
	free (contents);
      contents = NULL;
    }

  if (elf_dynverdef (abfd) != 0 || elf_tdata (abfd)->dt_verdef != NULL)
    {
      Elf_Internal_Shdr *hdr;
      Elf_External_Verdef *everdef;
      Elf_Internal_Verdef *iverdef;
      Elf_Internal_Verdef *iverdefarr;
      Elf_Internal_Verdef iverdefmem;
      unsigned int i;
      unsigned int maxidx;
      bfd_byte *contents_end_def, *contents_end_aux;
      size_t verdef_count;
      size_t verdef_size;

      if (elf_tdata (abfd)->dt_verdef != NULL)
	{
	  hdr = NULL;
	  contents = elf_tdata (abfd)->dt_verdef;
	  verdef_count = elf_tdata (abfd)->dt_verdef_count;
	  verdef_size = verdef_count * sizeof (Elf_External_Verdef);
	}
      else
	{
	  hdr = &elf_tdata (abfd)->dynverdef_hdr;

	  if (hdr->sh_size < sizeof (Elf_External_Verdef))
	    {
	    error_return_bad_verdef:
	      _bfd_error_handler
		(_("%pB: .gnu.version_d invalid entry"), abfd);
	      bfd_set_error (bfd_error_bad_value);
	    error_return_verdef:
	      elf_tdata (abfd)->verdef = NULL;
	      elf_tdata (abfd)->cverdefs = 0;
	      goto error_return;
	    }

	  if (bfd_seek (abfd, hdr->sh_offset, SEEK_SET) != 0)
	    goto error_return_verdef;
	  contents = _bfd_malloc_and_read (abfd, hdr->sh_size, hdr->sh_size);
	  if (contents == NULL)
	    goto error_return_verdef;

	  BFD_ASSERT (sizeof (Elf_External_Verdef)
		      >= sizeof (Elf_External_Verdaux));

	  verdef_count = hdr->sh_info;
	  verdef_size = hdr->sh_size;
	}

      contents_end_def = (contents + verdef_size
			  - sizeof (Elf_External_Verdef));
      contents_end_aux = (contents + verdef_size
			  - sizeof (Elf_External_Verdaux));

      /* We know the number of entries in the section but not the maximum
	 index.  Therefore we have to run through all entries and find
	 the maximum.  */
      everdef = (Elf_External_Verdef *) contents;
      maxidx = 0;
      for (i = 0; i < verdef_count; ++i)
	{
	  _bfd_elf_swap_verdef_in (abfd, everdef, &iverdefmem);

	  if ((iverdefmem.vd_ndx & ((unsigned) VERSYM_VERSION)) == 0)
	    goto error_return_bad_verdef;
	  if ((iverdefmem.vd_ndx & ((unsigned) VERSYM_VERSION)) > maxidx)
	    maxidx = iverdefmem.vd_ndx & ((unsigned) VERSYM_VERSION);

	  if (iverdefmem.vd_next == 0)
	    break;

	  if (iverdefmem.vd_next
	      > (size_t) (contents_end_def - (bfd_byte *) everdef))
	    goto error_return_bad_verdef;

	  everdef = ((Elf_External_Verdef *)
		     ((bfd_byte *) everdef + iverdefmem.vd_next));
	}

      if (default_imported_symver)
	{
	  if (freeidx > maxidx)
	    maxidx = ++freeidx;
	  else
	    freeidx = ++maxidx;
	}
      if (_bfd_mul_overflow (maxidx, sizeof (Elf_Internal_Verdef), &amt))
	{
	  bfd_set_error (bfd_error_file_too_big);
	  goto error_return_verdef;
	}

      if (amt == 0)
	goto error_return_verdef;
      elf_tdata (abfd)->verdef = (Elf_Internal_Verdef *) bfd_zalloc (abfd, amt);
      if (elf_tdata (abfd)->verdef == NULL)
	goto error_return_verdef;

      elf_tdata (abfd)->cverdefs = maxidx;

      everdef = (Elf_External_Verdef *) contents;
      iverdefarr = elf_tdata (abfd)->verdef;
      for (i = 0; i < verdef_count; ++i)
	{
	  Elf_External_Verdaux *everdaux;
	  Elf_Internal_Verdaux *iverdaux;
	  unsigned int j;

	  _bfd_elf_swap_verdef_in (abfd, everdef, &iverdefmem);

	  if ((iverdefmem.vd_ndx & VERSYM_VERSION) == 0)
	    goto error_return_bad_verdef;

	  iverdef = &iverdefarr[(iverdefmem.vd_ndx & VERSYM_VERSION) - 1];
	  memcpy (iverdef, &iverdefmem, offsetof (Elf_Internal_Verdef, vd_bfd));

	  iverdef->vd_bfd = abfd;

	  if (iverdef->vd_cnt == 0)
	    iverdef->vd_auxptr = NULL;
	  else
	    {
	      if (_bfd_mul_overflow (iverdef->vd_cnt,
				     sizeof (Elf_Internal_Verdaux), &amt))
		{
		  bfd_set_error (bfd_error_file_too_big);
		  goto error_return_verdef;
		}
	      iverdef->vd_auxptr = (struct elf_internal_verdaux *)
		bfd_alloc (abfd, amt);
	      if (iverdef->vd_auxptr == NULL)
		goto error_return_verdef;
	    }

	  if (iverdef->vd_aux
	      > (size_t) (contents_end_aux - (bfd_byte *) everdef))
	    goto error_return_bad_verdef;

	  everdaux = ((Elf_External_Verdaux *)
		      ((bfd_byte *) everdef + iverdef->vd_aux));
	  iverdaux = iverdef->vd_auxptr;
	  for (j = 0; j < iverdef->vd_cnt; j++, iverdaux++)
	    {
	      _bfd_elf_swap_verdaux_in (abfd, everdaux, iverdaux);

	      if (elf_use_dt_symtab_p (abfd))
		iverdaux->vda_nodename
		  = elf_tdata (abfd)->dt_strtab + iverdaux->vda_name;
	      else
		iverdaux->vda_nodename
		  = bfd_elf_string_from_elf_section (abfd, hdr->sh_link,
						     iverdaux->vda_name);
	      if (iverdaux->vda_nodename == NULL)
		goto error_return_bad_verdef;

	      iverdaux->vda_nextptr = NULL;
	      if (iverdaux->vda_next == 0)
		{
		  iverdef->vd_cnt = j + 1;
		  break;
		}
	      if (j + 1 < iverdef->vd_cnt)
		iverdaux->vda_nextptr = iverdaux + 1;

	      if (iverdaux->vda_next
		  > (size_t) (contents_end_aux - (bfd_byte *) everdaux))
		goto error_return_bad_verdef;

	      everdaux = ((Elf_External_Verdaux *)
			  ((bfd_byte *) everdaux + iverdaux->vda_next));
	    }

	  iverdef->vd_nodename = NULL;
	  if (iverdef->vd_cnt)
	    iverdef->vd_nodename = iverdef->vd_auxptr->vda_nodename;

	  iverdef->vd_nextdef = NULL;
	  if (iverdef->vd_next == 0)
	    break;
	  if ((size_t) (iverdef - iverdefarr) + 1 < maxidx)
	    iverdef->vd_nextdef = iverdef + 1;

	  everdef = ((Elf_External_Verdef *)
		     ((bfd_byte *) everdef + iverdef->vd_next));
	}

      if (elf_tdata (abfd)->dt_verdef == NULL)
	free (contents);
      contents = NULL;
    }
  else if (default_imported_symver)
    {
      if (freeidx < 3)
	freeidx = 3;
      else
	freeidx++;

      if (_bfd_mul_overflow (freeidx, sizeof (Elf_Internal_Verdef), &amt))
	{
	  bfd_set_error (bfd_error_file_too_big);
	  goto error_return;
	}
      if (amt == 0)
	goto error_return;
      elf_tdata (abfd)->verdef = (Elf_Internal_Verdef *) bfd_zalloc (abfd, amt);
      if (elf_tdata (abfd)->verdef == NULL)
	goto error_return;

      elf_tdata (abfd)->cverdefs = freeidx;
    }

  /* Create a default version based on the soname.  */
  if (default_imported_symver)
    {
      Elf_Internal_Verdef *iverdef;
      Elf_Internal_Verdaux *iverdaux;

      iverdef = &elf_tdata (abfd)->verdef[freeidx - 1];

      iverdef->vd_version = VER_DEF_CURRENT;
      iverdef->vd_flags = 0;
      iverdef->vd_ndx = freeidx;
      iverdef->vd_cnt = 1;

      iverdef->vd_bfd = abfd;

      iverdef->vd_nodename = bfd_elf_get_dt_soname (abfd);
      if (iverdef->vd_nodename == NULL)
	goto error_return_verdef;
      iverdef->vd_nextdef = NULL;
      iverdef->vd_auxptr = ((struct elf_internal_verdaux *)
			    bfd_zalloc (abfd, sizeof (Elf_Internal_Verdaux)));
      if (iverdef->vd_auxptr == NULL)
	goto error_return_verdef;

      iverdaux = iverdef->vd_auxptr;
      iverdaux->vda_nodename = iverdef->vd_nodename;
    }

  return true;

 error_return:
  free (contents);
  return false;
}

asymbol *
_bfd_elf_make_empty_symbol (bfd *abfd)
{
  elf_symbol_type *newsym;

  newsym = (elf_symbol_type *) bfd_zalloc (abfd, sizeof (*newsym));
  if (!newsym)
    return NULL;
  newsym->symbol.the_bfd = abfd;
  return &newsym->symbol;
}

void
_bfd_elf_get_symbol_info (bfd *abfd ATTRIBUTE_UNUSED,
			  asymbol *symbol,
			  symbol_info *ret)
{
  bfd_symbol_info (symbol, ret);
}

/* Return whether a symbol name implies a local symbol.  Most targets
   use this function for the is_local_label_name entry point, but some
   override it.  */

bool
_bfd_elf_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED,
			      const char *name)
{
  /* Normal local symbols start with ``.L''.  */
  if (name[0] == '.' && name[1] == 'L')
    return true;

  /* At least some SVR4 compilers (e.g., UnixWare 2.1 cc) generate
     DWARF debugging symbols starting with ``..''.  */
  if (name[0] == '.' && name[1] == '.')
    return true;

  /* gcc will sometimes generate symbols beginning with ``_.L_'' when
     emitting DWARF debugging output.  I suspect this is actually a
     small bug in gcc (it calls ASM_OUTPUT_LABEL when it should call
     ASM_GENERATE_INTERNAL_LABEL, and this causes the leading
     underscore to be emitted on some ELF targets).  For ease of use,
     we treat such symbols as local.  */
  if (name[0] == '_' && name[1] == '.' && name[2] == 'L' && name[3] == '_')
    return true;

  /* Treat assembler generated fake symbols, dollar local labels and
     forward-backward labels (aka local labels) as locals.
     These labels have the form:

       L0^A.*				       (fake symbols)

       [.]?L[0123456789]+{^A|^B}[0123456789]*  (local labels)

     Versions which start with .L will have already been matched above,
     so we only need to match the rest.  */
  if (name[0] == 'L' && ISDIGIT (name[1]))
    {
      bool ret = false;
      const char * p;
      char c;

      for (p = name + 2; (c = *p); p++)
	{
	  if (c == 1 || c == 2)
	    {
	      if (c == 1 && p == name + 2)
		/* A fake symbol.  */
		return true;

	      /* FIXME: We are being paranoid here and treating symbols like
		 L0^Bfoo as if there were non-local, on the grounds that the
		 assembler will never generate them.  But can any symbol
		 containing an ASCII value in the range 1-31 ever be anything
		 other than some kind of local ?  */
	      ret = true;
	    }

	  if (! ISDIGIT (c))
	    {
	      ret = false;
	      break;
	    }
	}
      return ret;
    }

  return false;
}

alent *
_bfd_elf_get_lineno (bfd *abfd ATTRIBUTE_UNUSED,
		     asymbol *symbol ATTRIBUTE_UNUSED)
{
  abort ();
  return NULL;
}

bool
_bfd_elf_set_arch_mach (bfd *abfd,
			enum bfd_architecture arch,
			unsigned long machine)
{
  /* If this isn't the right architecture for this backend, and this
     isn't the generic backend, fail.  */
  if (arch != get_elf_backend_data (abfd)->arch
      && arch != bfd_arch_unknown
      && get_elf_backend_data (abfd)->arch != bfd_arch_unknown)
    return false;

  return bfd_default_set_arch_mach (abfd, arch, machine);
}

/* Find the nearest line to a particular section and offset,
   for error reporting.  */

bool
_bfd_elf_find_nearest_line (bfd *abfd,
			    asymbol **symbols,
			    asection *section,
			    bfd_vma offset,
			    const char **filename_ptr,
			    const char **functionname_ptr,
			    unsigned int *line_ptr,
			    unsigned int *discriminator_ptr)
{
  return _bfd_elf_find_nearest_line_with_alt (abfd, NULL, symbols, section,
					      offset, filename_ptr,
					      functionname_ptr, line_ptr,
					      discriminator_ptr);
}

/* Find the nearest line to a particular section and offset,
   for error reporting.  ALT_BFD representing a .gnu_debugaltlink file
   can be optionally specified.  */

bool
_bfd_elf_find_nearest_line_with_alt (bfd *abfd,
				     const char *alt_filename,
				     asymbol **symbols,
				     asection *section,
				     bfd_vma offset,
				     const char **filename_ptr,
				     const char **functionname_ptr,
				     unsigned int *line_ptr,
				     unsigned int *discriminator_ptr)
{
  bool found;

  if (_bfd_dwarf2_find_nearest_line_with_alt (abfd, alt_filename, symbols, NULL,
					      section, offset, filename_ptr,
					      functionname_ptr, line_ptr,
					      discriminator_ptr,
					      dwarf_debug_sections,
					      &elf_tdata (abfd)->dwarf2_find_line_info))
    return true;

  if (_bfd_dwarf1_find_nearest_line (abfd, symbols, section, offset,
				     filename_ptr, functionname_ptr, line_ptr))
    {
      if (!*functionname_ptr)
	_bfd_elf_find_function (abfd, symbols, section, offset,
				*filename_ptr ? NULL : filename_ptr,
				functionname_ptr);
      return true;
    }

  if (! _bfd_stab_section_find_nearest_line (abfd, symbols, section, offset,
					     &found, filename_ptr,
					     functionname_ptr, line_ptr,
					     &elf_tdata (abfd)->line_info))
    return false;
  if (found && (*functionname_ptr || *line_ptr))
    return true;

  if (symbols == NULL)
    return false;

  if (! _bfd_elf_find_function (abfd, symbols, section, offset,
				filename_ptr, functionname_ptr))
    return false;

  *line_ptr = 0;
  return true;
}

/* Find the line for a symbol.  */

bool
_bfd_elf_find_line (bfd *abfd, asymbol **symbols, asymbol *symbol,
		    const char **filename_ptr, unsigned int *line_ptr)
{
  struct elf_obj_tdata *tdata = elf_tdata (abfd);
  return _bfd_dwarf2_find_nearest_line (abfd, symbols, symbol, NULL, 0,
					filename_ptr, NULL, line_ptr, NULL,
					dwarf_debug_sections,
					&tdata->dwarf2_find_line_info);
}

/* After a call to bfd_find_nearest_line, successive calls to
   bfd_find_inliner_info can be used to get source information about
   each level of function inlining that terminated at the address
   passed to bfd_find_nearest_line.  Currently this is only supported
   for DWARF2 with appropriate DWARF3 extensions. */

bool
_bfd_elf_find_inliner_info (bfd *abfd,
			    const char **filename_ptr,
			    const char **functionname_ptr,
			    unsigned int *line_ptr)
{
  struct elf_obj_tdata *tdata = elf_tdata (abfd);
  return _bfd_dwarf2_find_inliner_info (abfd, filename_ptr,
					functionname_ptr, line_ptr,
					&tdata->dwarf2_find_line_info);
}

int
_bfd_elf_sizeof_headers (bfd *abfd, struct bfd_link_info *info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  int ret = bed->s->sizeof_ehdr;

  if (!bfd_link_relocatable (info))
    {
      bfd_size_type phdr_size = elf_program_header_size (abfd);

      if (phdr_size == (bfd_size_type) -1)
	{
	  struct elf_segment_map *m;

	  phdr_size = 0;
	  for (m = elf_seg_map (abfd); m != NULL; m = m->next)
	    phdr_size += bed->s->sizeof_phdr;

	  if (phdr_size == 0)
	    phdr_size = get_program_header_size (abfd, info);
	}

      elf_program_header_size (abfd) = phdr_size;
      ret += phdr_size;
    }

  return ret;
}

bool
_bfd_elf_set_section_contents (bfd *abfd,
			       sec_ptr section,
			       const void *location,
			       file_ptr offset,
			       bfd_size_type count)
{
  Elf_Internal_Shdr *hdr;

  if (! abfd->output_has_begun
      && ! _bfd_elf_compute_section_file_positions (abfd, NULL))
    return false;

  if (!count)
    return true;

  hdr = &elf_section_data (section)->this_hdr;
  if (hdr->sh_offset == (file_ptr) -1)
    {
      unsigned char *contents;

      if (bfd_section_is_ctf (section))
	/* Nothing to do with this section: the contents are generated
	   later.  */
	return true;

      if ((offset + count) > hdr->sh_size)
	{
	  _bfd_error_handler
	    (_("%pB:%pA: error: attempting to write"
	       " over the end of the section"),
	     abfd, section);

	  bfd_set_error (bfd_error_invalid_operation);
	  return false;
	}

      contents = hdr->contents;
      if (contents == NULL)
	{
	  _bfd_error_handler
	    (_("%pB:%pA: error: attempting to write"
	       " section into an empty buffer"),
	     abfd, section);

	  bfd_set_error (bfd_error_invalid_operation);
	  return false;
	}

      memcpy (contents + offset, location, count);
      return true;
    }

  return _bfd_generic_set_section_contents (abfd, section,
					    location, offset, count);
}

bool
_bfd_elf_no_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
			   arelent *cache_ptr ATTRIBUTE_UNUSED,
			   Elf_Internal_Rela *dst ATTRIBUTE_UNUSED)
{
  abort ();
  return false;
}

/* Try to convert a non-ELF reloc into an ELF one.  */

bool
_bfd_elf_validate_reloc (bfd *abfd, arelent *areloc)
{
  /* Check whether we really have an ELF howto.  */

  if ((*areloc->sym_ptr_ptr)->the_bfd->xvec != abfd->xvec)
    {
      bfd_reloc_code_real_type code;
      reloc_howto_type *howto;

      /* Alien reloc: Try to determine its type to replace it with an
	 equivalent ELF reloc.  */

      if (areloc->howto->pc_relative)
	{
	  switch (areloc->howto->bitsize)
	    {
	    case 8:
	      code = BFD_RELOC_8_PCREL;
	      break;
	    case 12:
	      code = BFD_RELOC_12_PCREL;
	      break;
	    case 16:
	      code = BFD_RELOC_16_PCREL;
	      break;
	    case 24:
	      code = BFD_RELOC_24_PCREL;
	      break;
	    case 32:
	      code = BFD_RELOC_32_PCREL;
	      break;
	    case 64:
	      code = BFD_RELOC_64_PCREL;
	      break;
	    default:
	      goto fail;
	    }

	  howto = bfd_reloc_type_lookup (abfd, code);

	  if (howto && areloc->howto->pcrel_offset != howto->pcrel_offset)
	    {
	      if (howto->pcrel_offset)
		areloc->addend += areloc->address;
	      else
		areloc->addend -= areloc->address; /* addend is unsigned!! */
	    }
	}
      else
	{
	  switch (areloc->howto->bitsize)
	    {
	    case 8:
	      code = BFD_RELOC_8;
	      break;
	    case 14:
	      code = BFD_RELOC_14;
	      break;
	    case 16:
	      code = BFD_RELOC_16;
	      break;
	    case 26:
	      code = BFD_RELOC_26;
	      break;
	    case 32:
	      code = BFD_RELOC_32;
	      break;
	    case 64:
	      code = BFD_RELOC_64;
	      break;
	    default:
	      goto fail;
	    }

	  howto = bfd_reloc_type_lookup (abfd, code);
	}

      if (howto)
	areloc->howto = howto;
      else
	goto fail;
    }

  return true;

 fail:
  /* xgettext:c-format */
  _bfd_error_handler (_("%pB: %s unsupported"),
		      abfd, areloc->howto->name);
  bfd_set_error (bfd_error_sorry);
  return false;
}

bool
_bfd_elf_free_cached_info (bfd *abfd)
{
  struct elf_obj_tdata *tdata;

  if ((bfd_get_format (abfd) == bfd_object
       || bfd_get_format (abfd) == bfd_core)
      && (tdata = elf_tdata (abfd)) != NULL)
    {
      if (tdata->o != NULL && elf_shstrtab (abfd) != NULL)
	_bfd_elf_strtab_free (elf_shstrtab (abfd));
      _bfd_dwarf2_cleanup_debug_info (abfd, &tdata->dwarf2_find_line_info);
      _bfd_dwarf1_cleanup_debug_info (abfd, &tdata->dwarf1_find_line_info);
      _bfd_stab_cleanup (abfd, &tdata->line_info);
    }

  return _bfd_generic_bfd_free_cached_info (abfd);
}

/* For Rel targets, we encode meaningful data for BFD_RELOC_VTABLE_ENTRY
   in the relocation's offset.  Thus we cannot allow any sort of sanity
   range-checking to interfere.  There is nothing else to do in processing
   this reloc.  */

bfd_reloc_status_type
_bfd_elf_rel_vtable_reloc_fn
  (bfd *abfd ATTRIBUTE_UNUSED, arelent *re ATTRIBUTE_UNUSED,
   struct bfd_symbol *symbol ATTRIBUTE_UNUSED,
   void *data ATTRIBUTE_UNUSED, asection *is ATTRIBUTE_UNUSED,
   bfd *obfd ATTRIBUTE_UNUSED, char **errmsg ATTRIBUTE_UNUSED)
{
  return bfd_reloc_ok;
}

/* Elf core file support.  Much of this only works on native
   toolchains, since we rely on knowing the
   machine-dependent procfs structure in order to pick
   out details about the corefile.  */

#ifdef HAVE_SYS_PROCFS_H
# include <sys/procfs.h>
#endif

/* Return a PID that identifies a "thread" for threaded cores, or the
   PID of the main process for non-threaded cores.  */

static int
elfcore_make_pid (bfd *abfd)
{
  int pid;

  pid = elf_tdata (abfd)->core->lwpid;
  if (pid == 0)
    pid = elf_tdata (abfd)->core->pid;

  return pid;
}

/* If there isn't a section called NAME, make one, using data from
   SECT.  Note, this function will generate a reference to NAME, so
   you shouldn't deallocate or overwrite it.  */

static bool
elfcore_maybe_make_sect (bfd *abfd, char *name, asection *sect)
{
  asection *sect2;

  if (bfd_get_section_by_name (abfd, name) != NULL)
    return true;

  sect2 = bfd_make_section_with_flags (abfd, name, sect->flags);
  if (sect2 == NULL)
    return false;

  sect2->size = sect->size;
  sect2->filepos = sect->filepos;
  sect2->alignment_power = sect->alignment_power;
  return true;
}

/* Create a pseudosection containing SIZE bytes at FILEPOS.  This
   actually creates up to two pseudosections:
   - For the single-threaded case, a section named NAME, unless
     such a section already exists.
   - For the multi-threaded case, a section named "NAME/PID", where
     PID is elfcore_make_pid (abfd).
   Both pseudosections have identical contents.  */
bool
_bfd_elfcore_make_pseudosection (bfd *abfd,
				 char *name,
				 size_t size,
				 ufile_ptr filepos)
{
  char buf[100];
  char *threaded_name;
  size_t len;
  asection *sect;

  /* Build the section name.  */

  sprintf (buf, "%s/%d", name, elfcore_make_pid (abfd));
  len = strlen (buf) + 1;
  threaded_name = (char *) bfd_alloc (abfd, len);
  if (threaded_name == NULL)
    return false;
  memcpy (threaded_name, buf, len);

  sect = bfd_make_section_anyway_with_flags (abfd, threaded_name,
					     SEC_HAS_CONTENTS);
  if (sect == NULL)
    return false;
  sect->size = size;
  sect->filepos = filepos;
  sect->alignment_power = 2;

  return elfcore_maybe_make_sect (abfd, name, sect);
}

static bool
elfcore_make_auxv_note_section (bfd *abfd, Elf_Internal_Note *note,
				size_t offs)
{
  asection *sect = bfd_make_section_anyway_with_flags (abfd, ".auxv",
						       SEC_HAS_CONTENTS);

  if (sect == NULL)
    return false;

  sect->size = note->descsz - offs;
  sect->filepos = note->descpos + offs;
  sect->alignment_power = 1 + bfd_get_arch_size (abfd) / 32;

  return true;
}

/* prstatus_t exists on:
     solaris 2.5+
     linux 2.[01] + glibc
     unixware 4.2
*/

#if defined (HAVE_PRSTATUS_T)

static bool
elfcore_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  size_t size;
  int offset;

  if (note->descsz == sizeof (prstatus_t))
    {
      prstatus_t prstat;

      size = sizeof (prstat.pr_reg);
      offset   = offsetof (prstatus_t, pr_reg);
      memcpy (&prstat, note->descdata, sizeof (prstat));

      /* Do not overwrite the core signal if it
	 has already been set by another thread.  */
      if (elf_tdata (abfd)->core->signal == 0)
	elf_tdata (abfd)->core->signal = prstat.pr_cursig;
      if (elf_tdata (abfd)->core->pid == 0)
	elf_tdata (abfd)->core->pid = prstat.pr_pid;

      /* pr_who exists on:
	 solaris 2.5+
	 unixware 4.2
	 pr_who doesn't exist on:
	 linux 2.[01]
	 */
#if defined (HAVE_PRSTATUS_T_PR_WHO)
      elf_tdata (abfd)->core->lwpid = prstat.pr_who;
#else
      elf_tdata (abfd)->core->lwpid = prstat.pr_pid;
#endif
    }
#if defined (HAVE_PRSTATUS32_T)
  else if (note->descsz == sizeof (prstatus32_t))
    {
      /* 64-bit host, 32-bit corefile */
      prstatus32_t prstat;

      size = sizeof (prstat.pr_reg);
      offset   = offsetof (prstatus32_t, pr_reg);
      memcpy (&prstat, note->descdata, sizeof (prstat));

      /* Do not overwrite the core signal if it
	 has already been set by another thread.  */
      if (elf_tdata (abfd)->core->signal == 0)
	elf_tdata (abfd)->core->signal = prstat.pr_cursig;
      if (elf_tdata (abfd)->core->pid == 0)
	elf_tdata (abfd)->core->pid = prstat.pr_pid;

      /* pr_who exists on:
	 solaris 2.5+
	 unixware 4.2
	 pr_who doesn't exist on:
	 linux 2.[01]
	 */
#if defined (HAVE_PRSTATUS32_T_PR_WHO)
      elf_tdata (abfd)->core->lwpid = prstat.pr_who;
#else
      elf_tdata (abfd)->core->lwpid = prstat.pr_pid;
#endif
    }
#endif /* HAVE_PRSTATUS32_T */
  else
    {
      /* Fail - we don't know how to handle any other
	 note size (ie. data object type).  */
      return true;
    }

  /* Make a ".reg/999" section and a ".reg" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}
#endif /* defined (HAVE_PRSTATUS_T) */

/* Create a pseudosection containing the exact contents of NOTE.  */
static bool
elfcore_make_note_pseudosection (bfd *abfd,
				 char *name,
				 Elf_Internal_Note *note)
{
  return _bfd_elfcore_make_pseudosection (abfd, name,
					  note->descsz, note->descpos);
}

/* There isn't a consistent prfpregset_t across platforms,
   but it doesn't matter, because we don't have to pick this
   data structure apart.  */

static bool
elfcore_grok_prfpreg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg2", note);
}

/* Linux dumps the Intel SSE regs in a note named "LINUX" with a note
   type of NT_PRXFPREG.  Just include the whole note's contents
   literally.  */

static bool
elfcore_grok_prxfpreg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-xfp", note);
}

/* Linux dumps the Intel XSAVE extended state in a note named "LINUX"
   with a note type of NT_X86_XSTATE.  Just include the whole note's
   contents literally.  */

static bool
elfcore_grok_xstatereg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-xstate", note);
}

static bool
elfcore_grok_ppc_vmx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-vmx", note);
}

static bool
elfcore_grok_ppc_vsx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-vsx", note);
}

static bool
elfcore_grok_ppc_tar (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tar", note);
}

static bool
elfcore_grok_ppc_ppr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-ppr", note);
}

static bool
elfcore_grok_ppc_dscr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-dscr", note);
}

static bool
elfcore_grok_ppc_ebb (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-ebb", note);
}

static bool
elfcore_grok_ppc_pmu (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-pmu", note);
}

static bool
elfcore_grok_ppc_tm_cgpr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cgpr", note);
}

static bool
elfcore_grok_ppc_tm_cfpr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cfpr", note);
}

static bool
elfcore_grok_ppc_tm_cvmx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cvmx", note);
}

static bool
elfcore_grok_ppc_tm_cvsx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cvsx", note);
}

static bool
elfcore_grok_ppc_tm_spr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-spr", note);
}

static bool
elfcore_grok_ppc_tm_ctar (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-ctar", note);
}

static bool
elfcore_grok_ppc_tm_cppr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cppr", note);
}

static bool
elfcore_grok_ppc_tm_cdscr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cdscr", note);
}

static bool
elfcore_grok_s390_high_gprs (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-high-gprs", note);
}

static bool
elfcore_grok_s390_timer (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-timer", note);
}

static bool
elfcore_grok_s390_todcmp (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-todcmp", note);
}

static bool
elfcore_grok_s390_todpreg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-todpreg", note);
}

static bool
elfcore_grok_s390_ctrs (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-ctrs", note);
}

static bool
elfcore_grok_s390_prefix (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-prefix", note);
}

static bool
elfcore_grok_s390_last_break (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-last-break", note);
}

static bool
elfcore_grok_s390_system_call (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-system-call", note);
}

static bool
elfcore_grok_s390_tdb (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-tdb", note);
}

static bool
elfcore_grok_s390_vxrs_low (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-vxrs-low", note);
}

static bool
elfcore_grok_s390_vxrs_high (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-vxrs-high", note);
}

static bool
elfcore_grok_s390_gs_cb (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-gs-cb", note);
}

static bool
elfcore_grok_s390_gs_bc (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-gs-bc", note);
}

static bool
elfcore_grok_arm_vfp (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-arm-vfp", note);
}

static bool
elfcore_grok_aarch_tls (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-tls", note);
}

static bool
elfcore_grok_aarch_hw_break (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-hw-break", note);
}

static bool
elfcore_grok_aarch_hw_watch (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-hw-watch", note);
}

static bool
elfcore_grok_aarch_sve (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-sve", note);
}

static bool
elfcore_grok_aarch_pauth (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-pauth", note);
}

static bool
elfcore_grok_aarch_mte (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-mte",
					  note);
}

static bool
elfcore_grok_arc_v2 (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-arc-v2", note);
}

/* Convert NOTE into a bfd_section called ".reg-riscv-csr".  Return TRUE if
   successful otherwise, return FALSE.  */

static bool
elfcore_grok_riscv_csr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-riscv-csr", note);
}

/* Convert NOTE into a bfd_section called ".gdb-tdesc".  Return TRUE if
   successful otherwise, return FALSE.  */

static bool
elfcore_grok_gdb_tdesc (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".gdb-tdesc", note);
}

static bool
elfcore_grok_loongarch_cpucfg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-loongarch-cpucfg", note);
}

static bool
elfcore_grok_loongarch_lbt (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-loongarch-lbt", note);
}

static bool
elfcore_grok_loongarch_lsx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-loongarch-lsx", note);
}

static bool
elfcore_grok_loongarch_lasx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-loongarch-lasx", note);
}

#if defined (HAVE_PRPSINFO_T)
typedef prpsinfo_t   elfcore_psinfo_t;
#if defined (HAVE_PRPSINFO32_T)		/* Sparc64 cross Sparc32 */
typedef prpsinfo32_t elfcore_psinfo32_t;
#endif
#endif

#if defined (HAVE_PSINFO_T)
typedef psinfo_t   elfcore_psinfo_t;
#if defined (HAVE_PSINFO32_T)		/* Sparc64 cross Sparc32 */
typedef psinfo32_t elfcore_psinfo32_t;
#endif
#endif

/* return a malloc'ed copy of a string at START which is at
   most MAX bytes long, possibly without a terminating '\0'.
   the copy will always have a terminating '\0'.  */

char *
_bfd_elfcore_strndup (bfd *abfd, char *start, size_t max)
{
  char *dups;
  char *end = (char *) memchr (start, '\0', max);
  size_t len;

  if (end == NULL)
    len = max;
  else
    len = end - start;

  dups = (char *) bfd_alloc (abfd, len + 1);
  if (dups == NULL)
    return NULL;

  memcpy (dups, start, len);
  dups[len] = '\0';

  return dups;
}

#if defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T)
static bool
elfcore_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz == sizeof (elfcore_psinfo_t))
    {
      elfcore_psinfo_t psinfo;

      memcpy (&psinfo, note->descdata, sizeof (psinfo));

#if defined (HAVE_PSINFO_T_PR_PID) || defined (HAVE_PRPSINFO_T_PR_PID)
      elf_tdata (abfd)->core->pid = psinfo.pr_pid;
#endif
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, psinfo.pr_fname,
				sizeof (psinfo.pr_fname));

      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, psinfo.pr_psargs,
				sizeof (psinfo.pr_psargs));
    }
#if defined (HAVE_PRPSINFO32_T) || defined (HAVE_PSINFO32_T)
  else if (note->descsz == sizeof (elfcore_psinfo32_t))
    {
      /* 64-bit host, 32-bit corefile */
      elfcore_psinfo32_t psinfo;

      memcpy (&psinfo, note->descdata, sizeof (psinfo));

#if defined (HAVE_PSINFO32_T_PR_PID) || defined (HAVE_PRPSINFO32_T_PR_PID)
      elf_tdata (abfd)->core->pid = psinfo.pr_pid;
#endif
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, psinfo.pr_fname,
				sizeof (psinfo.pr_fname));

      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, psinfo.pr_psargs,
				sizeof (psinfo.pr_psargs));
    }
#endif

  else
    {
      /* Fail - we don't know how to handle any other
	 note size (ie. data object type).  */
      return true;
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
#endif /* defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T) */

#if defined (HAVE_PSTATUS_T)
static bool
elfcore_grok_pstatus (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz == sizeof (pstatus_t)
#if defined (HAVE_PXSTATUS_T)
      || note->descsz == sizeof (pxstatus_t)
#endif
      )
    {
      pstatus_t pstat;

      memcpy (&pstat, note->descdata, sizeof (pstat));

      elf_tdata (abfd)->core->pid = pstat.pr_pid;
    }
#if defined (HAVE_PSTATUS32_T)
  else if (note->descsz == sizeof (pstatus32_t))
    {
      /* 64-bit host, 32-bit corefile */
      pstatus32_t pstat;

      memcpy (&pstat, note->descdata, sizeof (pstat));

      elf_tdata (abfd)->core->pid = pstat.pr_pid;
    }
#endif
  /* Could grab some more details from the "representative"
     lwpstatus_t in pstat.pr_lwp, but we'll catch it all in an
     NT_LWPSTATUS note, presumably.  */

  return true;
}
#endif /* defined (HAVE_PSTATUS_T) */

#if defined (HAVE_LWPSTATUS_T)
static bool
elfcore_grok_lwpstatus (bfd *abfd, Elf_Internal_Note *note)
{
  lwpstatus_t lwpstat;
  char buf[100];
  char *name;
  size_t len;
  asection *sect;

  if (note->descsz != sizeof (lwpstat)
#if defined (HAVE_LWPXSTATUS_T)
      && note->descsz != sizeof (lwpxstatus_t)
#endif
      )
    return true;

  memcpy (&lwpstat, note->descdata, sizeof (lwpstat));

  elf_tdata (abfd)->core->lwpid = lwpstat.pr_lwpid;
  /* Do not overwrite the core signal if it has already been set by
     another thread.  */
  if (elf_tdata (abfd)->core->signal == 0)
    elf_tdata (abfd)->core->signal = lwpstat.pr_cursig;

  /* Make a ".reg/999" section.  */

  sprintf (buf, ".reg/%d", elfcore_make_pid (abfd));
  len = strlen (buf) + 1;
  name = bfd_alloc (abfd, len);
  if (name == NULL)
    return false;
  memcpy (name, buf, len);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return false;

#if defined (HAVE_LWPSTATUS_T_PR_CONTEXT)
  sect->size = sizeof (lwpstat.pr_context.uc_mcontext.gregs);
  sect->filepos = note->descpos
    + offsetof (lwpstatus_t, pr_context.uc_mcontext.gregs);
#endif

#if defined (HAVE_LWPSTATUS_T_PR_REG)
  sect->size = sizeof (lwpstat.pr_reg);
  sect->filepos = note->descpos + offsetof (lwpstatus_t, pr_reg);
#endif

  sect->alignment_power = 2;

  if (!elfcore_maybe_make_sect (abfd, ".reg", sect))
    return false;

  /* Make a ".reg2/999" section */

  sprintf (buf, ".reg2/%d", elfcore_make_pid (abfd));
  len = strlen (buf) + 1;
  name = bfd_alloc (abfd, len);
  if (name == NULL)
    return false;
  memcpy (name, buf, len);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return false;

#if defined (HAVE_LWPSTATUS_T_PR_CONTEXT)
  sect->size = sizeof (lwpstat.pr_context.uc_mcontext.fpregs);
  sect->filepos = note->descpos
    + offsetof (lwpstatus_t, pr_context.uc_mcontext.fpregs);
#endif

#if defined (HAVE_LWPSTATUS_T_PR_FPREG)
  sect->size = sizeof (lwpstat.pr_fpreg);
  sect->filepos = note->descpos + offsetof (lwpstatus_t, pr_fpreg);
#endif

  sect->alignment_power = 2;

  return elfcore_maybe_make_sect (abfd, ".reg2", sect);
}
#endif /* defined (HAVE_LWPSTATUS_T) */

/* These constants, and the structure offsets used below, are defined by
   Cygwin's core_dump.h */
#define NOTE_INFO_PROCESS  1
#define NOTE_INFO_THREAD   2
#define NOTE_INFO_MODULE   3
#define NOTE_INFO_MODULE64 4

static bool
elfcore_grok_win32pstatus (bfd *abfd, Elf_Internal_Note *note)
{
  char buf[30];
  char *name;
  size_t len;
  unsigned int name_size;
  asection *sect;
  unsigned int type;
  int is_active_thread;
  bfd_vma base_addr;

  if (note->descsz < 4)
    return true;

  if (! startswith (note->namedata, "win32"))
    return true;

  type = bfd_get_32 (abfd, note->descdata);

  struct
  {
    const char *type_name;
    unsigned long min_size;
  } size_check[] =
      {
       { "NOTE_INFO_PROCESS", 12 },
       { "NOTE_INFO_THREAD", 12 },
       { "NOTE_INFO_MODULE", 12 },
       { "NOTE_INFO_MODULE64", 16 },
      };

  if (type == 0 || type > (sizeof(size_check)/sizeof(size_check[0])))
      return true;

  if (note->descsz < size_check[type - 1].min_size)
    {
      _bfd_error_handler (_("%pB: warning: win32pstatus %s of size %lu bytes"
			    " is too small"),
			  abfd, size_check[type - 1].type_name, note->descsz);
      return true;
    }

  switch (type)
    {
    case NOTE_INFO_PROCESS:
      /* FIXME: need to add ->core->command.  */
      elf_tdata (abfd)->core->pid = bfd_get_32 (abfd, note->descdata + 4);
      elf_tdata (abfd)->core->signal = bfd_get_32 (abfd, note->descdata + 8);
      break;

    case NOTE_INFO_THREAD:
      /* Make a ".reg/<tid>" section containing the Win32 API thread CONTEXT
	 structure. */
      /* thread_info.tid */
      sprintf (buf, ".reg/%ld", (long) bfd_get_32 (abfd, note->descdata + 4));

      len = strlen (buf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (name == NULL)
	return false;

      memcpy (name, buf, len);

      sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
      if (sect == NULL)
	return false;

      /* sizeof (thread_info.thread_context) */
      sect->size = note->descsz - 12;
      /* offsetof (thread_info.thread_context) */
      sect->filepos = note->descpos + 12;
      sect->alignment_power = 2;

      /* thread_info.is_active_thread */
      is_active_thread = bfd_get_32 (abfd, note->descdata + 8);

      if (is_active_thread)
	if (! elfcore_maybe_make_sect (abfd, ".reg", sect))
	  return false;
      break;

    case NOTE_INFO_MODULE:
    case NOTE_INFO_MODULE64:
      /* Make a ".module/xxxxxxxx" section.  */
      if (type == NOTE_INFO_MODULE)
	{
	  /* module_info.base_address */
	  base_addr = bfd_get_32 (abfd, note->descdata + 4);
	  sprintf (buf, ".module/%08lx", (unsigned long) base_addr);
	  /* module_info.module_name_size */
	  name_size = bfd_get_32 (abfd, note->descdata + 8);
	}
      else /* NOTE_INFO_MODULE64 */
	{
	  /* module_info.base_address */
	  base_addr = bfd_get_64 (abfd, note->descdata + 4);
	  sprintf (buf, ".module/%016lx", (unsigned long) base_addr);
	  /* module_info.module_name_size */
	  name_size = bfd_get_32 (abfd, note->descdata + 12);
	}

      len = strlen (buf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (name == NULL)
	return false;

      memcpy (name, buf, len);

      sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);

      if (sect == NULL)
	return false;

      if (note->descsz < 12 + name_size)
	{
	  _bfd_error_handler (_("%pB: win32pstatus NOTE_INFO_MODULE of size %lu"
				" is too small to contain a name of size %u"),
			      abfd, note->descsz, name_size);
	  return true;
	}

      sect->size = note->descsz;
      sect->filepos = note->descpos;
      sect->alignment_power = 2;
      break;

    default:
      return true;
    }

  return true;
}

static bool
elfcore_grok_note (bfd *abfd, Elf_Internal_Note *note)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  switch (note->type)
    {
    default:
      return true;

    case NT_PRSTATUS:
      if (bed->elf_backend_grok_prstatus)
	if ((*bed->elf_backend_grok_prstatus) (abfd, note))
	  return true;
#if defined (HAVE_PRSTATUS_T)
      return elfcore_grok_prstatus (abfd, note);
#else
      return true;
#endif

#if defined (HAVE_PSTATUS_T)
    case NT_PSTATUS:
      return elfcore_grok_pstatus (abfd, note);
#endif

#if defined (HAVE_LWPSTATUS_T)
    case NT_LWPSTATUS:
      return elfcore_grok_lwpstatus (abfd, note);
#endif

    case NT_FPREGSET:		/* FIXME: rename to NT_PRFPREG */
      return elfcore_grok_prfpreg (abfd, note);

    case NT_WIN32PSTATUS:
      return elfcore_grok_win32pstatus (abfd, note);

    case NT_PRXFPREG:		/* Linux SSE extension */
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_prxfpreg (abfd, note);
      else
	return true;

    case NT_X86_XSTATE:		/* Linux XSAVE extension */
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_xstatereg (abfd, note);
      else
	return true;

    case NT_PPC_VMX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_vmx (abfd, note);
      else
	return true;

    case NT_PPC_VSX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_vsx (abfd, note);
      else
	return true;

    case NT_PPC_TAR:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_tar (abfd, note);
      else
	return true;

    case NT_PPC_PPR:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_ppr (abfd, note);
      else
	return true;

    case NT_PPC_DSCR:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_dscr (abfd, note);
      else
	return true;

    case NT_PPC_EBB:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_ebb (abfd, note);
      else
	return true;

    case NT_PPC_PMU:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_pmu (abfd, note);
      else
	return true;

    case NT_PPC_TM_CGPR:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_tm_cgpr (abfd, note);
      else
	return true;

    case NT_PPC_TM_CFPR:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_tm_cfpr (abfd, note);
      else
	return true;

    case NT_PPC_TM_CVMX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_tm_cvmx (abfd, note);
      else
	return true;

    case NT_PPC_TM_CVSX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_tm_cvsx (abfd, note);
      else
	return true;

    case NT_PPC_TM_SPR:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_tm_spr (abfd, note);
      else
	return true;

    case NT_PPC_TM_CTAR:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_tm_ctar (abfd, note);
      else
	return true;

    case NT_PPC_TM_CPPR:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_tm_cppr (abfd, note);
      else
	return true;

    case NT_PPC_TM_CDSCR:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_tm_cdscr (abfd, note);
      else
	return true;

    case NT_S390_HIGH_GPRS:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_high_gprs (abfd, note);
      else
	return true;

    case NT_S390_TIMER:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_timer (abfd, note);
      else
	return true;

    case NT_S390_TODCMP:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_todcmp (abfd, note);
      else
	return true;

    case NT_S390_TODPREG:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_todpreg (abfd, note);
      else
	return true;

    case NT_S390_CTRS:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_ctrs (abfd, note);
      else
	return true;

    case NT_S390_PREFIX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_prefix (abfd, note);
      else
	return true;

    case NT_S390_LAST_BREAK:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_last_break (abfd, note);
      else
	return true;

    case NT_S390_SYSTEM_CALL:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_system_call (abfd, note);
      else
	return true;

    case NT_S390_TDB:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_tdb (abfd, note);
      else
	return true;

    case NT_S390_VXRS_LOW:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_vxrs_low (abfd, note);
      else
	return true;

    case NT_S390_VXRS_HIGH:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_vxrs_high (abfd, note);
      else
	return true;

    case NT_S390_GS_CB:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_gs_cb (abfd, note);
      else
	return true;

    case NT_S390_GS_BC:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_gs_bc (abfd, note);
      else
	return true;

    case NT_ARC_V2:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_arc_v2 (abfd, note);
      else
	return true;

    case NT_ARM_VFP:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_arm_vfp (abfd, note);
      else
	return true;

    case NT_ARM_TLS:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_tls (abfd, note);
      else
	return true;

    case NT_ARM_HW_BREAK:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_hw_break (abfd, note);
      else
	return true;

    case NT_ARM_HW_WATCH:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_hw_watch (abfd, note);
      else
	return true;

    case NT_ARM_SVE:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_sve (abfd, note);
      else
	return true;

    case NT_ARM_PAC_MASK:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_pauth (abfd, note);
      else
	return true;

    case NT_ARM_TAGGED_ADDR_CTRL:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_mte (abfd, note);
      else
	return true;

    case NT_GDB_TDESC:
      if (note->namesz == 4
	  && strcmp (note->namedata, "GDB") == 0)
	return elfcore_grok_gdb_tdesc (abfd, note);
      else
	return true;

    case NT_RISCV_CSR:
      if (note->namesz == 4
	  && strcmp (note->namedata, "GDB") == 0)
	return elfcore_grok_riscv_csr (abfd, note);
      else
	return true;

    case NT_LARCH_CPUCFG:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_loongarch_cpucfg (abfd, note);
      else
	return true;

    case NT_LARCH_LBT:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_loongarch_lbt (abfd, note);
      else
	return true;

    case NT_LARCH_LSX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_loongarch_lsx (abfd, note);
      else
	return true;

    case NT_LARCH_LASX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_loongarch_lasx (abfd, note);
      else
	return true;

    case NT_PRPSINFO:
    case NT_PSINFO:
      if (bed->elf_backend_grok_psinfo)
	if ((*bed->elf_backend_grok_psinfo) (abfd, note))
	  return true;
#if defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T)
      return elfcore_grok_psinfo (abfd, note);
#else
      return true;
#endif

    case NT_AUXV:
      return elfcore_make_auxv_note_section (abfd, note, 0);

    case NT_FILE:
      return elfcore_make_note_pseudosection (abfd, ".note.linuxcore.file",
					      note);

    case NT_SIGINFO:
      return elfcore_make_note_pseudosection (abfd, ".note.linuxcore.siginfo",
					      note);

    }
}

static bool
elfobj_grok_gnu_build_id (bfd *abfd, Elf_Internal_Note *note)
{
  struct bfd_build_id* build_id;

  if (note->descsz == 0)
    return false;

  build_id = bfd_alloc (abfd, sizeof (struct bfd_build_id) - 1 + note->descsz);
  if (build_id == NULL)
    return false;

  build_id->size = note->descsz;
  memcpy (build_id->data, note->descdata, note->descsz);
  abfd->build_id = build_id;

  return true;
}

static bool
elfobj_grok_gnu_note (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->type)
    {
    default:
      return true;

    case NT_GNU_PROPERTY_TYPE_0:
      return _bfd_elf_parse_gnu_properties (abfd, note);

    case NT_GNU_BUILD_ID:
      return elfobj_grok_gnu_build_id (abfd, note);
    }
}

static bool
elfobj_grok_stapsdt_note_1 (bfd *abfd, Elf_Internal_Note *note)
{
  struct sdt_note *cur =
    (struct sdt_note *) bfd_alloc (abfd,
				   sizeof (struct sdt_note) + note->descsz);

  cur->next = (struct sdt_note *) (elf_tdata (abfd))->sdt_note_head;
  cur->size = (bfd_size_type) note->descsz;
  memcpy (cur->data, note->descdata, note->descsz);

  elf_tdata (abfd)->sdt_note_head = cur;

  return true;
}

static bool
elfobj_grok_stapsdt_note (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->type)
    {
    case NT_STAPSDT:
      return elfobj_grok_stapsdt_note_1 (abfd, note);

    default:
      return true;
    }
}

static bool
elfcore_grok_freebsd_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  size_t offset;

  switch (elf_elfheader (abfd)->e_ident[EI_CLASS])
    {
    case ELFCLASS32:
      if (note->descsz < 108)
	return false;
      break;

    case ELFCLASS64:
      if (note->descsz < 120)
	return false;
      break;

    default:
      return false;
    }

  /* Check for version 1 in pr_version.  */
  if (bfd_h_get_32 (abfd, (bfd_byte *) note->descdata) != 1)
    return false;

  offset = 4;

  /* Skip over pr_psinfosz. */
  if (elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS32)
    offset += 4;
  else
    {
      offset += 4;	/* Padding before pr_psinfosz. */
      offset += 8;
    }

  /* pr_fname is PRFNAMESZ (16) + 1 bytes in size.  */
  elf_tdata (abfd)->core->program
    = _bfd_elfcore_strndup (abfd, note->descdata + offset, 17);
  offset += 17;

  /* pr_psargs is PRARGSZ (80) + 1 bytes in size.  */
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + offset, 81);
  offset += 81;

  /* Padding before pr_pid.  */
  offset += 2;

  /* The pr_pid field was added in version "1a".  */
  if (note->descsz < offset + 4)
    return true;

  elf_tdata (abfd)->core->pid
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + offset);

  return true;
}

static bool
elfcore_grok_freebsd_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  size_t offset;
  size_t size;
  size_t min_size;

  /* Compute offset of pr_getregsz, skipping over pr_statussz.
     Also compute minimum size of this note.  */
  switch (elf_elfheader (abfd)->e_ident[EI_CLASS])
    {
    case ELFCLASS32:
      offset = 4 + 4;
      min_size = offset + (4 * 2) + 4 + 4 + 4;
      break;

    case ELFCLASS64:
      offset = 4 + 4 + 8;	/* Includes padding before pr_statussz.  */
      min_size = offset + (8 * 2) + 4 + 4 + 4 + 4;
      break;

    default:
      return false;
    }

  if (note->descsz < min_size)
    return false;

  /* Check for version 1 in pr_version.  */
  if (bfd_h_get_32 (abfd, (bfd_byte *) note->descdata) != 1)
    return false;

  /* Extract size of pr_reg from pr_gregsetsz.  */
  /* Skip over pr_gregsetsz and pr_fpregsetsz.  */
  if (elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS32)
    {
      size = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + offset);
      offset += 4 * 2;
    }
  else
    {
      size = bfd_h_get_64 (abfd, (bfd_byte *) note->descdata + offset);
      offset += 8 * 2;
    }

  /* Skip over pr_osreldate.  */
  offset += 4;

  /* Read signal from pr_cursig.  */
  if (elf_tdata (abfd)->core->signal == 0)
    elf_tdata (abfd)->core->signal
      = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + offset);
  offset += 4;

  /* Read TID from pr_pid.  */
  elf_tdata (abfd)->core->lwpid
      = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + offset);
  offset += 4;

  /* Padding before pr_reg.  */
  if (elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS64)
    offset += 4;

  /* Make sure that there is enough data remaining in the note.  */
  if ((note->descsz - offset) < size)
    return false;

  /* Make a ".reg/999" section and a ".reg" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
elfcore_grok_freebsd_note (bfd *abfd, Elf_Internal_Note *note)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  switch (note->type)
    {
    case NT_PRSTATUS:
      if (bed->elf_backend_grok_freebsd_prstatus)
	if ((*bed->elf_backend_grok_freebsd_prstatus) (abfd, note))
	  return true;
      return elfcore_grok_freebsd_prstatus (abfd, note);

    case NT_FPREGSET:
      return elfcore_grok_prfpreg (abfd, note);

    case NT_PRPSINFO:
      return elfcore_grok_freebsd_psinfo (abfd, note);

    case NT_FREEBSD_THRMISC:
      return elfcore_make_note_pseudosection (abfd, ".thrmisc", note);

    case NT_FREEBSD_PROCSTAT_PROC:
      return elfcore_make_note_pseudosection (abfd, ".note.freebsdcore.proc",
					      note);

    case NT_FREEBSD_PROCSTAT_FILES:
      return elfcore_make_note_pseudosection (abfd, ".note.freebsdcore.files",
					      note);

    case NT_FREEBSD_PROCSTAT_VMMAP:
      return elfcore_make_note_pseudosection (abfd, ".note.freebsdcore.vmmap",
					      note);

    case NT_FREEBSD_PROCSTAT_AUXV:
      return elfcore_make_auxv_note_section (abfd, note, 4);

    case NT_FREEBSD_X86_SEGBASES:
      return elfcore_make_note_pseudosection (abfd, ".reg-x86-segbases", note);

    case NT_X86_XSTATE:
      return elfcore_grok_xstatereg (abfd, note);

    case NT_FREEBSD_PTLWPINFO:
      return elfcore_make_note_pseudosection (abfd, ".note.freebsdcore.lwpinfo",
					      note);

    case NT_ARM_TLS:
      return elfcore_grok_aarch_tls (abfd, note);

    case NT_ARM_VFP:
      return elfcore_grok_arm_vfp (abfd, note);

    default:
      return true;
    }
}

static bool
elfcore_netbsd_get_lwpid (Elf_Internal_Note *note, int *lwpidp)
{
  char *cp;

  cp = strchr (note->namedata, '@');
  if (cp != NULL)
    {
      *lwpidp = atoi(cp + 1);
      return true;
    }
  return false;
}

static bool
elfcore_grok_netbsd_procinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz <= 0x7c + 31)
    return false;

  /* Signal number at offset 0x08. */
  elf_tdata (abfd)->core->signal
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x08);

  /* Process ID at offset 0x50. */
  elf_tdata (abfd)->core->pid
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x50);

  /* Command name at 0x7c (max 32 bytes, including nul). */
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + 0x7c, 31);

  return elfcore_make_note_pseudosection (abfd, ".note.netbsdcore.procinfo",
					  note);
}

static bool
elfcore_grok_netbsd_note (bfd *abfd, Elf_Internal_Note *note)
{
  int lwp;

  if (elfcore_netbsd_get_lwpid (note, &lwp))
    elf_tdata (abfd)->core->lwpid = lwp;

  switch (note->type)
    {
    case NT_NETBSDCORE_PROCINFO:
      /* NetBSD-specific core "procinfo".  Note that we expect to
	 find this note before any of the others, which is fine,
	 since the kernel writes this note out first when it
	 creates a core file.  */
      return elfcore_grok_netbsd_procinfo (abfd, note);
    case NT_NETBSDCORE_AUXV:
      /* NetBSD-specific Elf Auxiliary Vector data. */
      return elfcore_make_auxv_note_section (abfd, note, 4);
    case NT_NETBSDCORE_LWPSTATUS:
      return elfcore_make_note_pseudosection (abfd,
					      ".note.netbsdcore.lwpstatus",
					      note);
    default:
      break;
    }

  /* As of March 2020 there are no other machine-independent notes
     defined for NetBSD core files.  If the note type is less
     than the start of the machine-dependent note types, we don't
     understand it.  */

  if (note->type < NT_NETBSDCORE_FIRSTMACH)
    return true;


  switch (bfd_get_arch (abfd))
    {
      /* On the Alpha, SPARC (32-bit and 64-bit), PT_GETREGS == mach+0 and
	 PT_GETFPREGS == mach+2.  */

    case bfd_arch_aarch64:
    case bfd_arch_alpha:
    case bfd_arch_sparc:
      switch (note->type)
	{
	case NT_NETBSDCORE_FIRSTMACH+0:
	  return elfcore_make_note_pseudosection (abfd, ".reg", note);

	case NT_NETBSDCORE_FIRSTMACH+2:
	  return elfcore_make_note_pseudosection (abfd, ".reg2", note);

	default:
	  return true;
	}

      /* On SuperH, PT_GETREGS == mach+3 and PT_GETFPREGS == mach+5.
	 There's also old PT___GETREGS40 == mach + 1 for old reg
	 structure which lacks GBR.  */

    case bfd_arch_sh:
      switch (note->type)
	{
	case NT_NETBSDCORE_FIRSTMACH+3:
	  return elfcore_make_note_pseudosection (abfd, ".reg", note);

	case NT_NETBSDCORE_FIRSTMACH+5:
	  return elfcore_make_note_pseudosection (abfd, ".reg2", note);

	default:
	  return true;
	}

      /* On all other arch's, PT_GETREGS == mach+1 and
	 PT_GETFPREGS == mach+3.  */

    default:
      switch (note->type)
	{
	case NT_NETBSDCORE_FIRSTMACH+1:
	  return elfcore_make_note_pseudosection (abfd, ".reg", note);

	case NT_NETBSDCORE_FIRSTMACH+3:
	  return elfcore_make_note_pseudosection (abfd, ".reg2", note);

	default:
	  return true;
	}
    }
    /* NOTREACHED */
}

static bool
elfcore_grok_openbsd_procinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz <= 0x48 + 31)
    return false;

  /* Signal number at offset 0x08. */
  elf_tdata (abfd)->core->signal
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x08);

  /* Process ID at offset 0x20. */
  elf_tdata (abfd)->core->pid
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x20);

  /* Command name at 0x48 (max 32 bytes, including nul). */
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + 0x48, 31);

  return true;
}

/* Processes Solaris's process status note.
   sig_off ~ offsetof(prstatus_t, pr_cursig)
   pid_off ~ offsetof(prstatus_t, pr_pid)
   lwpid_off ~ offsetof(prstatus_t, pr_who)
   gregset_size ~ sizeof(gregset_t)
   gregset_offset ~ offsetof(prstatus_t, pr_reg)  */

static bool
elfcore_grok_solaris_prstatus (bfd *abfd, Elf_Internal_Note* note, int sig_off,
			       int pid_off, int lwpid_off, size_t gregset_size,
			       size_t gregset_offset)
{
  asection *sect = NULL;
  elf_tdata (abfd)->core->signal
    = bfd_get_16 (abfd, note->descdata + sig_off);
  elf_tdata (abfd)->core->pid
    = bfd_get_32 (abfd, note->descdata + pid_off);
  elf_tdata (abfd)->core->lwpid
    = bfd_get_32 (abfd, note->descdata + lwpid_off);

  sect = bfd_get_section_by_name (abfd, ".reg");
  if (sect != NULL)
    sect->size = gregset_size;

  return _bfd_elfcore_make_pseudosection (abfd, ".reg", gregset_size,
					  note->descpos + gregset_offset);
}

/* Gets program and arguments from a core.
   prog_off ~ offsetof(prpsinfo | psinfo_t, pr_fname)
   comm_off ~ offsetof(prpsinfo | psinfo_t, pr_psargs)  */

static bool
elfcore_grok_solaris_info(bfd *abfd, Elf_Internal_Note* note,
			  int prog_off, int comm_off)
{
  elf_tdata (abfd)->core->program
    = _bfd_elfcore_strndup (abfd, note->descdata + prog_off, 16);
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + comm_off, 80);

  return true;
}

/* Processes Solaris's LWP status note.
   gregset_size ~ sizeof(gregset_t)
   gregset_off ~ offsetof(lwpstatus_t, pr_reg)
   fpregset_size ~ sizeof(fpregset_t)
   fpregset_off ~ offsetof(lwpstatus_t, pr_fpreg)  */

static bool
elfcore_grok_solaris_lwpstatus (bfd *abfd, Elf_Internal_Note* note,
				size_t gregset_size, int gregset_off,
				size_t fpregset_size, int fpregset_off)
{
  asection *sect = NULL;
  char reg2_section_name[16] = { 0 };

  (void) snprintf (reg2_section_name, 16, "%s/%i", ".reg2",
		   elf_tdata (abfd)->core->lwpid);

  /* offsetof(lwpstatus_t, pr_lwpid) */
  elf_tdata (abfd)->core->lwpid
    = bfd_get_32 (abfd, note->descdata + 4);
  /* offsetof(lwpstatus_t, pr_cursig) */
  elf_tdata (abfd)->core->signal
    = bfd_get_16 (abfd, note->descdata + 12);

  sect = bfd_get_section_by_name (abfd, ".reg");
  if (sect != NULL)
    sect->size = gregset_size;
  else if (!_bfd_elfcore_make_pseudosection (abfd, ".reg", gregset_size,
					     note->descpos + gregset_off))
    return false;

  sect = bfd_get_section_by_name (abfd, reg2_section_name);
  if (sect != NULL)
    {
      sect->size = fpregset_size;
      sect->filepos = note->descpos + fpregset_off;
      sect->alignment_power = 2;
    }
  else if (!_bfd_elfcore_make_pseudosection (abfd, ".reg2", fpregset_size,
					     note->descpos + fpregset_off))
    return false;

  return true;
}

static bool
elfcore_grok_solaris_note_impl (bfd *abfd, Elf_Internal_Note *note)
{
  if (note == NULL)
    return false;

  /* core files are identified as 32- or 64-bit, SPARC or x86,
     by the size of the descsz which matches the sizeof()
     the type appropriate for that note type (e.g., prstatus_t for
     SOLARIS_NT_PRSTATUS) for the corresponding architecture
     on Solaris. The core file bitness may differ from the bitness of
     gdb itself, so fixed values are used instead of sizeof().
     Appropriate fixed offsets are also used to obtain data from
     the note.  */

  switch ((int) note->type)
    {
    case SOLARIS_NT_PRSTATUS:
      switch (note->descsz)
	{
	case 508: /* sizeof(prstatus_t) SPARC 32-bit */
	  return elfcore_grok_solaris_prstatus(abfd, note,
					       136, 216, 308, 152, 356);
	case 904: /* sizeof(prstatus_t) SPARC 64-bit */
	  return elfcore_grok_solaris_prstatus(abfd, note,
					       264, 360, 520, 304, 600);
	case 432: /* sizeof(prstatus_t) Intel 32-bit */
	  return elfcore_grok_solaris_prstatus(abfd, note,
					       136, 216, 308, 76, 356);
	case 824: /* sizeof(prstatus_t) Intel 64-bit */
	  return elfcore_grok_solaris_prstatus(abfd, note,
					       264, 360, 520, 224, 600);
	default:
	  return true;
	}

    case SOLARIS_NT_PSINFO:
    case SOLARIS_NT_PRPSINFO:
      switch (note->descsz)
	{
	case 260: /* sizeof(prpsinfo_t) SPARC and Intel 32-bit */
	  return elfcore_grok_solaris_info(abfd, note, 84, 100);
	case 328: /* sizeof(prpsinfo_t) SPARC and Intel 64-bit */
	  return elfcore_grok_solaris_info(abfd, note, 120, 136);
	case 360: /* sizeof(psinfo_t) SPARC and Intel 32-bit */
	  return elfcore_grok_solaris_info(abfd, note, 88, 104);
	case 440: /* sizeof(psinfo_t) SPARC and Intel 64-bit */
	  return elfcore_grok_solaris_info(abfd, note, 136, 152);
	default:
	  return true;
	}

    case SOLARIS_NT_LWPSTATUS:
      switch (note->descsz)
	{
	case 896: /* sizeof(lwpstatus_t) SPARC 32-bit */
	  return elfcore_grok_solaris_lwpstatus(abfd, note,
						152, 344, 400, 496);
	case 1392: /* sizeof(lwpstatus_t) SPARC 64-bit */
	  return elfcore_grok_solaris_lwpstatus(abfd, note,
						304, 544, 544, 848);
	case 800: /* sizeof(lwpstatus_t) Intel 32-bit */
	  return elfcore_grok_solaris_lwpstatus(abfd, note,
						76, 344, 380, 420);
	case 1296: /* sizeof(lwpstatus_t) Intel 64-bit */
	  return elfcore_grok_solaris_lwpstatus(abfd, note,
						224, 544, 528, 768);
	default:
	  return true;
	}

    case SOLARIS_NT_LWPSINFO:
      /* sizeof(lwpsinfo_t) on 32- and 64-bit, respectively */
      if (note->descsz == 128 || note->descsz == 152)
	elf_tdata (abfd)->core->lwpid =
	  bfd_get_32 (abfd, note->descdata + 4);
      break;

    default:
      break;
    }

  return true;
}

/* For name starting with "CORE" this may be either a Solaris
   core file or a gdb-generated core file.  Do Solaris-specific
   processing on selected note types first with
   elfcore_grok_solaris_note(), then process the note
   in elfcore_grok_note().  */

static bool
elfcore_grok_solaris_note (bfd *abfd, Elf_Internal_Note *note)
{
  if (!elfcore_grok_solaris_note_impl (abfd, note))
    return false;

  return elfcore_grok_note (abfd, note);
}

static bool
elfcore_grok_openbsd_note (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->type == NT_OPENBSD_PROCINFO)
    return elfcore_grok_openbsd_procinfo (abfd, note);

  if (note->type == NT_OPENBSD_REGS)
    return elfcore_make_note_pseudosection (abfd, ".reg", note);

  if (note->type == NT_OPENBSD_FPREGS)
    return elfcore_make_note_pseudosection (abfd, ".reg2", note);

  if (note->type == NT_OPENBSD_XFPREGS)
    return elfcore_make_note_pseudosection (abfd, ".reg-xfp", note);

  if (note->type == NT_OPENBSD_AUXV)
    return elfcore_make_auxv_note_section (abfd, note, 0);

  if (note->type == NT_OPENBSD_WCOOKIE)
    {
      asection *sect = bfd_make_section_anyway_with_flags (abfd, ".wcookie",
							   SEC_HAS_CONTENTS);

      if (sect == NULL)
	return false;
      sect->size = note->descsz;
      sect->filepos = note->descpos;
      sect->alignment_power = 1 + bfd_get_arch_size (abfd) / 32;

      return true;
    }

  return true;
}

static bool
elfcore_grok_nto_status (bfd *abfd, Elf_Internal_Note *note, long *tid)
{
  void *ddata = note->descdata;
  char buf[100];
  char *name;
  asection *sect;
  short sig;
  unsigned flags;

  if (note->descsz < 16)
    return false;

  /* nto_procfs_status 'pid' field is at offset 0.  */
  elf_tdata (abfd)->core->pid = bfd_get_32 (abfd, (bfd_byte *) ddata);

  /* nto_procfs_status 'tid' field is at offset 4.  Pass it back.  */
  *tid = bfd_get_32 (abfd, (bfd_byte *) ddata + 4);

  /* nto_procfs_status 'flags' field is at offset 8.  */
  flags = bfd_get_32 (abfd, (bfd_byte *) ddata + 8);

  /* nto_procfs_status 'what' field is at offset 14.  */
  if ((sig = bfd_get_16 (abfd, (bfd_byte *) ddata + 14)) > 0)
    {
      elf_tdata (abfd)->core->signal = sig;
      elf_tdata (abfd)->core->lwpid = *tid;
    }

  /* _DEBUG_FLAG_CURTID (current thread) is 0x80.  Some cores
     do not come from signals so we make sure we set the current
     thread just in case.  */
  if (flags & 0x00000080)
    elf_tdata (abfd)->core->lwpid = *tid;

  /* Make a ".qnx_core_status/%d" section.  */
  sprintf (buf, ".qnx_core_status/%ld", *tid);

  name = (char *) bfd_alloc (abfd, strlen (buf) + 1);
  if (name == NULL)
    return false;
  strcpy (name, buf);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return false;

  sect->size		= note->descsz;
  sect->filepos		= note->descpos;
  sect->alignment_power = 2;

  return (elfcore_maybe_make_sect (abfd, ".qnx_core_status", sect));
}

static bool
elfcore_grok_nto_regs (bfd *abfd,
		       Elf_Internal_Note *note,
		       long tid,
		       char *base)
{
  char buf[100];
  char *name;
  asection *sect;

  /* Make a "(base)/%d" section.  */
  sprintf (buf, "%s/%ld", base, tid);

  name = (char *) bfd_alloc (abfd, strlen (buf) + 1);
  if (name == NULL)
    return false;
  strcpy (name, buf);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return false;

  sect->size		= note->descsz;
  sect->filepos		= note->descpos;
  sect->alignment_power = 2;

  /* This is the current thread.  */
  if (elf_tdata (abfd)->core->lwpid == tid)
    return elfcore_maybe_make_sect (abfd, base, sect);

  return true;
}

static bool
elfcore_grok_nto_note (bfd *abfd, Elf_Internal_Note *note)
{
  /* Every GREG section has a STATUS section before it.  Store the
     tid from the previous call to pass down to the next gregs
     function.  */
  static long tid = 1;

  switch (note->type)
    {
    case QNT_CORE_INFO:
      return elfcore_make_note_pseudosection (abfd, ".qnx_core_info", note);
    case QNT_CORE_STATUS:
      return elfcore_grok_nto_status (abfd, note, &tid);
    case QNT_CORE_GREG:
      return elfcore_grok_nto_regs (abfd, note, tid, ".reg");
    case QNT_CORE_FPREG:
      return elfcore_grok_nto_regs (abfd, note, tid, ".reg2");
    default:
      return true;
    }
}

static bool
elfcore_grok_spu_note (bfd *abfd, Elf_Internal_Note *note)
{
  char *name;
  asection *sect;
  size_t len;

  /* Use note name as section name.  */
  len = note->namesz;
  name = (char *) bfd_alloc (abfd, len);
  if (name == NULL)
    return false;
  memcpy (name, note->namedata, len);
  name[len - 1] = '\0';

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return false;

  sect->size		= note->descsz;
  sect->filepos		= note->descpos;
  sect->alignment_power = 1;

  return true;
}

/* Function: elfcore_write_note

   Inputs:
     buffer to hold note, and current size of buffer
     name of note
     type of note
     data for note
     size of data for note

   Writes note to end of buffer.  ELF64 notes are written exactly as
   for ELF32, despite the current (as of 2006) ELF gabi specifying
   that they ought to have 8-byte namesz and descsz field, and have
   8-byte alignment.  Other writers, eg. Linux kernel, do the same.

   Return:
   Pointer to realloc'd buffer, *BUFSIZ updated.  */

char *
elfcore_write_note (bfd *abfd,
		    char *buf,
		    int *bufsiz,
		    const char *name,
		    int type,
		    const void *input,
		    int size)
{
  Elf_External_Note *xnp;
  size_t namesz;
  size_t newspace;
  char *dest;

  namesz = 0;
  if (name != NULL)
    namesz = strlen (name) + 1;

  newspace = 12 + ((namesz + 3) & -4) + ((size + 3) & -4);

  buf = (char *) realloc (buf, *bufsiz + newspace);
  if (buf == NULL)
    return buf;
  dest = buf + *bufsiz;
  *bufsiz += newspace;
  xnp = (Elf_External_Note *) dest;
  H_PUT_32 (abfd, namesz, xnp->namesz);
  H_PUT_32 (abfd, size, xnp->descsz);
  H_PUT_32 (abfd, type, xnp->type);
  dest = xnp->name;
  if (name != NULL)
    {
      memcpy (dest, name, namesz);
      dest += namesz;
      while (namesz & 3)
	{
	  *dest++ = '\0';
	  ++namesz;
	}
    }
  memcpy (dest, input, size);
  dest += size;
  while (size & 3)
    {
      *dest++ = '\0';
      ++size;
    }
  return buf;
}

/* gcc-8 warns (*) on all the strncpy calls in this function about
   possible string truncation.  The "truncation" is not a bug.  We
   have an external representation of structs with fields that are not
   necessarily NULL terminated and corresponding internal
   representation fields that are one larger so that they can always
   be NULL terminated.
   gcc versions between 4.2 and 4.6 do not allow pragma control of
   diagnostics inside functions, giving a hard error if you try to use
   the finer control available with later versions.
   gcc prior to 4.2 warns about diagnostic push and pop.
   gcc-5, gcc-6 and gcc-7 warn that -Wstringop-truncation is unknown,
   unless you also add #pragma GCC diagnostic ignored "-Wpragma".
   (*) Depending on your system header files!  */
#if GCC_VERSION >= 8000
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
char *
elfcore_write_prpsinfo (bfd  *abfd,
			char *buf,
			int  *bufsiz,
			const char *fname,
			const char *psargs)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (bed->elf_backend_write_core_note != NULL)
    {
      char *ret;
      ret = (*bed->elf_backend_write_core_note) (abfd, buf, bufsiz,
						 NT_PRPSINFO, fname, psargs);
      if (ret != NULL)
	return ret;
    }

#if defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T)
# if defined (HAVE_PRPSINFO32_T) || defined (HAVE_PSINFO32_T)
  if (bed->s->elfclass == ELFCLASS32)
    {
#  if defined (HAVE_PSINFO32_T)
      psinfo32_t data;
      int note_type = NT_PSINFO;
#  else
      prpsinfo32_t data;
      int note_type = NT_PRPSINFO;
#  endif

      memset (&data, 0, sizeof (data));
      strncpy (data.pr_fname, fname, sizeof (data.pr_fname));
      strncpy (data.pr_psargs, psargs, sizeof (data.pr_psargs));
      return elfcore_write_note (abfd, buf, bufsiz,
				 "CORE", note_type, &data, sizeof (data));
    }
  else
# endif
    {
# if defined (HAVE_PSINFO_T)
      psinfo_t data;
      int note_type = NT_PSINFO;
# else
      prpsinfo_t data;
      int note_type = NT_PRPSINFO;
# endif

      memset (&data, 0, sizeof (data));
      strncpy (data.pr_fname, fname, sizeof (data.pr_fname));
      strncpy (data.pr_psargs, psargs, sizeof (data.pr_psargs));
      return elfcore_write_note (abfd, buf, bufsiz,
				 "CORE", note_type, &data, sizeof (data));
    }
#endif	/* PSINFO_T or PRPSINFO_T */

  free (buf);
  return NULL;
}
#if GCC_VERSION >= 8000
# pragma GCC diagnostic pop
#endif

char *
elfcore_write_linux_prpsinfo32
  (bfd *abfd, char *buf, int *bufsiz,
   const struct elf_internal_linux_prpsinfo *prpsinfo)
{
  if (get_elf_backend_data (abfd)->linux_prpsinfo32_ugid16)
    {
      struct elf_external_linux_prpsinfo32_ugid16 data;

      swap_linux_prpsinfo32_ugid16_out (abfd, prpsinfo, &data);
      return elfcore_write_note (abfd, buf, bufsiz, "CORE", NT_PRPSINFO,
				 &data, sizeof (data));
    }
  else
    {
      struct elf_external_linux_prpsinfo32_ugid32 data;

      swap_linux_prpsinfo32_ugid32_out (abfd, prpsinfo, &data);
      return elfcore_write_note (abfd, buf, bufsiz, "CORE", NT_PRPSINFO,
				 &data, sizeof (data));
    }
}

char *
elfcore_write_linux_prpsinfo64
  (bfd *abfd, char *buf, int *bufsiz,
   const struct elf_internal_linux_prpsinfo *prpsinfo)
{
  if (get_elf_backend_data (abfd)->linux_prpsinfo64_ugid16)
    {
      struct elf_external_linux_prpsinfo64_ugid16 data;

      swap_linux_prpsinfo64_ugid16_out (abfd, prpsinfo, &data);
      return elfcore_write_note (abfd, buf, bufsiz,
				 "CORE", NT_PRPSINFO, &data, sizeof (data));
    }
  else
    {
      struct elf_external_linux_prpsinfo64_ugid32 data;

      swap_linux_prpsinfo64_ugid32_out (abfd, prpsinfo, &data);
      return elfcore_write_note (abfd, buf, bufsiz,
				 "CORE", NT_PRPSINFO, &data, sizeof (data));
    }
}

char *
elfcore_write_prstatus (bfd *abfd,
			char *buf,
			int *bufsiz,
			long pid,
			int cursig,
			const void *gregs)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (bed->elf_backend_write_core_note != NULL)
    {
      char *ret;
      ret = (*bed->elf_backend_write_core_note) (abfd, buf, bufsiz,
						 NT_PRSTATUS,
						 pid, cursig, gregs);
      if (ret != NULL)
	return ret;
    }

#if defined (HAVE_PRSTATUS_T)
#if defined (HAVE_PRSTATUS32_T)
  if (bed->s->elfclass == ELFCLASS32)
    {
      prstatus32_t prstat;

      memset (&prstat, 0, sizeof (prstat));
      prstat.pr_pid = pid;
      prstat.pr_cursig = cursig;
      memcpy (&prstat.pr_reg, gregs, sizeof (prstat.pr_reg));
      return elfcore_write_note (abfd, buf, bufsiz, "CORE",
				 NT_PRSTATUS, &prstat, sizeof (prstat));
    }
  else
#endif
    {
      prstatus_t prstat;

      memset (&prstat, 0, sizeof (prstat));
      prstat.pr_pid = pid;
      prstat.pr_cursig = cursig;
      memcpy (&prstat.pr_reg, gregs, sizeof (prstat.pr_reg));
      return elfcore_write_note (abfd, buf, bufsiz, "CORE",
				 NT_PRSTATUS, &prstat, sizeof (prstat));
    }
#endif /* HAVE_PRSTATUS_T */

  free (buf);
  return NULL;
}

#if defined (HAVE_LWPSTATUS_T)
char *
elfcore_write_lwpstatus (bfd *abfd,
			 char *buf,
			 int *bufsiz,
			 long pid,
			 int cursig,
			 const void *gregs)
{
  lwpstatus_t lwpstat;
  const char *note_name = "CORE";

  memset (&lwpstat, 0, sizeof (lwpstat));
  lwpstat.pr_lwpid  = pid >> 16;
  lwpstat.pr_cursig = cursig;
#if defined (HAVE_LWPSTATUS_T_PR_REG)
  memcpy (&lwpstat.pr_reg, gregs, sizeof (lwpstat.pr_reg));
#elif defined (HAVE_LWPSTATUS_T_PR_CONTEXT)
#if !defined(gregs)
  memcpy (lwpstat.pr_context.uc_mcontext.gregs,
	  gregs, sizeof (lwpstat.pr_context.uc_mcontext.gregs));
#else
  memcpy (lwpstat.pr_context.uc_mcontext.__gregs,
	  gregs, sizeof (lwpstat.pr_context.uc_mcontext.__gregs));
#endif
#endif
  return elfcore_write_note (abfd, buf, bufsiz, note_name,
			     NT_LWPSTATUS, &lwpstat, sizeof (lwpstat));
}
#endif /* HAVE_LWPSTATUS_T */

#if defined (HAVE_PSTATUS_T)
char *
elfcore_write_pstatus (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       long pid,
		       int cursig ATTRIBUTE_UNUSED,
		       const void *gregs ATTRIBUTE_UNUSED)
{
  const char *note_name = "CORE";
#if defined (HAVE_PSTATUS32_T)
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (bed->s->elfclass == ELFCLASS32)
    {
      pstatus32_t pstat;

      memset (&pstat, 0, sizeof (pstat));
      pstat.pr_pid = pid & 0xffff;
      buf = elfcore_write_note (abfd, buf, bufsiz, note_name,
				NT_PSTATUS, &pstat, sizeof (pstat));
      return buf;
    }
  else
#endif
    {
      pstatus_t pstat;

      memset (&pstat, 0, sizeof (pstat));
      pstat.pr_pid = pid & 0xffff;
      buf = elfcore_write_note (abfd, buf, bufsiz, note_name,
				NT_PSTATUS, &pstat, sizeof (pstat));
      return buf;
    }
}
#endif /* HAVE_PSTATUS_T */

char *
elfcore_write_prfpreg (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *fpregs,
		       int size)
{
  const char *note_name = "CORE";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_FPREGSET, fpregs, size);
}

char *
elfcore_write_prxfpreg (bfd *abfd,
			char *buf,
			int *bufsiz,
			const void *xfpregs,
			int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PRXFPREG, xfpregs, size);
}

char *
elfcore_write_xstatereg (bfd *abfd, char *buf, int *bufsiz,
			 const void *xfpregs, int size)
{
  char *note_name;
  if (get_elf_backend_data (abfd)->elf_osabi == ELFOSABI_FREEBSD)
    note_name = "FreeBSD";
  else
    note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_X86_XSTATE, xfpregs, size);
}

char *
elfcore_write_x86_segbases (bfd *abfd, char *buf, int *bufsiz,
			    const void *regs, int size)
{
  char *note_name = "FreeBSD";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_FREEBSD_X86_SEGBASES, regs, size);
}

char *
elfcore_write_ppc_vmx (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *ppc_vmx,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_VMX, ppc_vmx, size);
}

char *
elfcore_write_ppc_vsx (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *ppc_vsx,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_VSX, ppc_vsx, size);
}

char *
elfcore_write_ppc_tar (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *ppc_tar,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_TAR, ppc_tar, size);
}

char *
elfcore_write_ppc_ppr (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *ppc_ppr,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_PPR, ppc_ppr, size);
}

char *
elfcore_write_ppc_dscr (bfd *abfd,
			char *buf,
			int *bufsiz,
			const void *ppc_dscr,
			int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_DSCR, ppc_dscr, size);
}

char *
elfcore_write_ppc_ebb (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *ppc_ebb,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_EBB, ppc_ebb, size);
}

char *
elfcore_write_ppc_pmu (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *ppc_pmu,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_PMU, ppc_pmu, size);
}

char *
elfcore_write_ppc_tm_cgpr (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *ppc_tm_cgpr,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_TM_CGPR, ppc_tm_cgpr, size);
}

char *
elfcore_write_ppc_tm_cfpr (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *ppc_tm_cfpr,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_TM_CFPR, ppc_tm_cfpr, size);
}

char *
elfcore_write_ppc_tm_cvmx (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *ppc_tm_cvmx,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_TM_CVMX, ppc_tm_cvmx, size);
}

char *
elfcore_write_ppc_tm_cvsx (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *ppc_tm_cvsx,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_TM_CVSX, ppc_tm_cvsx, size);
}

char *
elfcore_write_ppc_tm_spr (bfd *abfd,
			  char *buf,
			  int *bufsiz,
			  const void *ppc_tm_spr,
			  int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_TM_SPR, ppc_tm_spr, size);
}

char *
elfcore_write_ppc_tm_ctar (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *ppc_tm_ctar,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_TM_CTAR, ppc_tm_ctar, size);
}

char *
elfcore_write_ppc_tm_cppr (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *ppc_tm_cppr,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_TM_CPPR, ppc_tm_cppr, size);
}

char *
elfcore_write_ppc_tm_cdscr (bfd *abfd,
			    char *buf,
			    int *bufsiz,
			    const void *ppc_tm_cdscr,
			    int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_TM_CDSCR, ppc_tm_cdscr, size);
}

static char *
elfcore_write_s390_high_gprs (bfd *abfd,
			      char *buf,
			      int *bufsiz,
			      const void *s390_high_gprs,
			      int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_HIGH_GPRS,
			     s390_high_gprs, size);
}

char *
elfcore_write_s390_timer (bfd *abfd,
			  char *buf,
			  int *bufsiz,
			  const void *s390_timer,
			  int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_TIMER, s390_timer, size);
}

char *
elfcore_write_s390_todcmp (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *s390_todcmp,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_TODCMP, s390_todcmp, size);
}

char *
elfcore_write_s390_todpreg (bfd *abfd,
			    char *buf,
			    int *bufsiz,
			    const void *s390_todpreg,
			    int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_TODPREG, s390_todpreg, size);
}

char *
elfcore_write_s390_ctrs (bfd *abfd,
			 char *buf,
			 int *bufsiz,
			 const void *s390_ctrs,
			 int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_CTRS, s390_ctrs, size);
}

char *
elfcore_write_s390_prefix (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *s390_prefix,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_PREFIX, s390_prefix, size);
}

char *
elfcore_write_s390_last_break (bfd *abfd,
			       char *buf,
			       int *bufsiz,
			       const void *s390_last_break,
			       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_LAST_BREAK,
			     s390_last_break, size);
}

char *
elfcore_write_s390_system_call (bfd *abfd,
				char *buf,
				int *bufsiz,
				const void *s390_system_call,
				int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_SYSTEM_CALL,
			     s390_system_call, size);
}

char *
elfcore_write_s390_tdb (bfd *abfd,
			char *buf,
			int *bufsiz,
			const void *s390_tdb,
			int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_TDB, s390_tdb, size);
}

char *
elfcore_write_s390_vxrs_low (bfd *abfd,
			     char *buf,
			     int *bufsiz,
			     const void *s390_vxrs_low,
			     int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_VXRS_LOW, s390_vxrs_low, size);
}

char *
elfcore_write_s390_vxrs_high (bfd *abfd,
			     char *buf,
			     int *bufsiz,
			     const void *s390_vxrs_high,
			     int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_VXRS_HIGH,
			     s390_vxrs_high, size);
}

char *
elfcore_write_s390_gs_cb (bfd *abfd,
			  char *buf,
			  int *bufsiz,
			  const void *s390_gs_cb,
			  int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_GS_CB,
			     s390_gs_cb, size);
}

char *
elfcore_write_s390_gs_bc (bfd *abfd,
			  char *buf,
			  int *bufsiz,
			  const void *s390_gs_bc,
			  int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_GS_BC,
			     s390_gs_bc, size);
}

char *
elfcore_write_arm_vfp (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *arm_vfp,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_VFP, arm_vfp, size);
}

char *
elfcore_write_aarch_tls (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *aarch_tls,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_TLS, aarch_tls, size);
}

char *
elfcore_write_aarch_hw_break (bfd *abfd,
			    char *buf,
			    int *bufsiz,
			    const void *aarch_hw_break,
			    int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_HW_BREAK, aarch_hw_break, size);
}

char *
elfcore_write_aarch_hw_watch (bfd *abfd,
			    char *buf,
			    int *bufsiz,
			    const void *aarch_hw_watch,
			    int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_HW_WATCH, aarch_hw_watch, size);
}

char *
elfcore_write_aarch_sve (bfd *abfd,
			 char *buf,
			 int *bufsiz,
			 const void *aarch_sve,
			 int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_SVE, aarch_sve, size);
}

char *
elfcore_write_aarch_pauth (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *aarch_pauth,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_PAC_MASK, aarch_pauth, size);
}

char *
elfcore_write_aarch_mte (bfd *abfd,
				      char *buf,
				      int *bufsiz,
				      const void *aarch_mte,
				      int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_TAGGED_ADDR_CTRL,
			     aarch_mte,
			     size);
}

char *
elfcore_write_arc_v2 (bfd *abfd,
		      char *buf,
		      int *bufsiz,
		      const void *arc_v2,
		      int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARC_V2, arc_v2, size);
}

char *
elfcore_write_loongarch_cpucfg (bfd *abfd,
				char *buf,
				int *bufsiz,
				const void *loongarch_cpucfg,
				int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_LARCH_CPUCFG,
			     loongarch_cpucfg, size);
}

char *
elfcore_write_loongarch_lbt (bfd *abfd,
			     char *buf,
			     int *bufsiz,
			     const void *loongarch_lbt,
			     int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_LARCH_LBT, loongarch_lbt, size);
}

char *
elfcore_write_loongarch_lsx (bfd *abfd,
			     char *buf,
			     int *bufsiz,
			     const void *loongarch_lsx,
			     int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_LARCH_LSX, loongarch_lsx, size);
}

char *
elfcore_write_loongarch_lasx (bfd *abfd,
			      char *buf,
			      int *bufsiz,
			      const void *loongarch_lasx,
			      int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_LARCH_LASX, loongarch_lasx, size);
}

/* Write the buffer of csr values in CSRS (length SIZE) into the note
   buffer BUF and update *BUFSIZ.  ABFD is the bfd the note is being
   written into.  Return a pointer to the new start of the note buffer, to
   replace BUF which may no longer be valid.  */

char *
elfcore_write_riscv_csr (bfd *abfd,
			 char *buf,
			 int *bufsiz,
			 const void *csrs,
			 int size)
{
  const char *note_name = "GDB";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_RISCV_CSR, csrs, size);
}

/* Write the target description (a string) pointed to by TDESC, length
   SIZE, into the note buffer BUF, and update *BUFSIZ.  ABFD is the bfd the
   note is being written into.  Return a pointer to the new start of the
   note buffer, to replace BUF which may no longer be valid.  */

char *
elfcore_write_gdb_tdesc (bfd *abfd,
			 char *buf,
			 int *bufsiz,
			 const void *tdesc,
			 int size)
{
  const char *note_name = "GDB";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_GDB_TDESC, tdesc, size);
}

char *
elfcore_write_register_note (bfd *abfd,
			     char *buf,
			     int *bufsiz,
			     const char *section,
			     const void *data,
			     int size)
{
  if (strcmp (section, ".reg2") == 0)
    return elfcore_write_prfpreg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-xfp") == 0)
    return elfcore_write_prxfpreg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-xstate") == 0)
    return elfcore_write_xstatereg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-x86-segbases") == 0)
    return elfcore_write_x86_segbases (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-vmx") == 0)
    return elfcore_write_ppc_vmx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-vsx") == 0)
    return elfcore_write_ppc_vsx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tar") == 0)
    return elfcore_write_ppc_tar (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-ppr") == 0)
    return elfcore_write_ppc_ppr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-dscr") == 0)
    return elfcore_write_ppc_dscr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-ebb") == 0)
    return elfcore_write_ppc_ebb (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-pmu") == 0)
    return elfcore_write_ppc_pmu (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cgpr") == 0)
    return elfcore_write_ppc_tm_cgpr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cfpr") == 0)
    return elfcore_write_ppc_tm_cfpr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cvmx") == 0)
    return elfcore_write_ppc_tm_cvmx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cvsx") == 0)
    return elfcore_write_ppc_tm_cvsx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-spr") == 0)
    return elfcore_write_ppc_tm_spr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-ctar") == 0)
    return elfcore_write_ppc_tm_ctar (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cppr") == 0)
    return elfcore_write_ppc_tm_cppr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cdscr") == 0)
    return elfcore_write_ppc_tm_cdscr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-high-gprs") == 0)
    return elfcore_write_s390_high_gprs (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-timer") == 0)
    return elfcore_write_s390_timer (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-todcmp") == 0)
    return elfcore_write_s390_todcmp (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-todpreg") == 0)
    return elfcore_write_s390_todpreg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-ctrs") == 0)
    return elfcore_write_s390_ctrs (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-prefix") == 0)
    return elfcore_write_s390_prefix (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-last-break") == 0)
    return elfcore_write_s390_last_break (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-system-call") == 0)
    return elfcore_write_s390_system_call (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-tdb") == 0)
    return elfcore_write_s390_tdb (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-vxrs-low") == 0)
    return elfcore_write_s390_vxrs_low (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-vxrs-high") == 0)
    return elfcore_write_s390_vxrs_high (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-gs-cb") == 0)
    return elfcore_write_s390_gs_cb (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-gs-bc") == 0)
    return elfcore_write_s390_gs_bc (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-arm-vfp") == 0)
    return elfcore_write_arm_vfp (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-tls") == 0)
    return elfcore_write_aarch_tls (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-hw-break") == 0)
    return elfcore_write_aarch_hw_break (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-hw-watch") == 0)
    return elfcore_write_aarch_hw_watch (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-sve") == 0)
    return elfcore_write_aarch_sve (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-pauth") == 0)
    return elfcore_write_aarch_pauth (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-mte") == 0)
    return elfcore_write_aarch_mte (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-arc-v2") == 0)
    return elfcore_write_arc_v2 (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".gdb-tdesc") == 0)
    return elfcore_write_gdb_tdesc (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-riscv-csr") == 0)
    return elfcore_write_riscv_csr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-loongarch-cpucfg") == 0)
    return elfcore_write_loongarch_cpucfg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-loongarch-lbt") == 0)
    return elfcore_write_loongarch_lbt (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-loongarch-lsx") == 0)
    return elfcore_write_loongarch_lsx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-loongarch-lasx") == 0)
    return elfcore_write_loongarch_lasx (abfd, buf, bufsiz, data, size);
  return NULL;
}

char *
elfcore_write_file_note (bfd *obfd, char *note_data, int *note_size,
			 const void *buf, int bufsiz)
{
  return elfcore_write_note (obfd, note_data, note_size,
			     "CORE", NT_FILE, buf, bufsiz);
}

static bool
elf_parse_notes (bfd *abfd, char *buf, size_t size, file_ptr offset,
		 size_t align)
{
  char *p;

  /* NB: CORE PT_NOTE segments may have p_align values of 0 or 1.
     gABI specifies that PT_NOTE alignment should be aligned to 4
     bytes for 32-bit objects and to 8 bytes for 64-bit objects.  If
     align is less than 4, we use 4 byte alignment.   */
  if (align < 4)
    align = 4;
  if (align != 4 && align != 8)
    return false;

  p = buf;
  while (p < buf + size)
    {
      Elf_External_Note *xnp = (Elf_External_Note *) p;
      Elf_Internal_Note in;

      if (offsetof (Elf_External_Note, name) > buf - p + size)
	return false;

      in.type = H_GET_32 (abfd, xnp->type);

      in.namesz = H_GET_32 (abfd, xnp->namesz);
      in.namedata = xnp->name;
      if (in.namesz > buf - in.namedata + size)
	return false;

      in.descsz = H_GET_32 (abfd, xnp->descsz);
      in.descdata = p + ELF_NOTE_DESC_OFFSET (in.namesz, align);
      in.descpos = offset + (in.descdata - buf);
      if (in.descsz != 0
	  && (in.descdata >= buf + size
	      || in.descsz > buf - in.descdata + size))
	return false;

      switch (bfd_get_format (abfd))
	{
	default:
	  return true;

	case bfd_core:
	  {
#define GROKER_ELEMENT(S,F) {S, sizeof (S) - 1, F}
	    struct
	    {
	      const char * string;
	      size_t len;
	      bool (*func) (bfd *, Elf_Internal_Note *);
	    }
	    grokers[] =
	    {
	      GROKER_ELEMENT ("", elfcore_grok_note),
	      GROKER_ELEMENT ("FreeBSD", elfcore_grok_freebsd_note),
	      GROKER_ELEMENT ("NetBSD-CORE", elfcore_grok_netbsd_note),
	      GROKER_ELEMENT ("OpenBSD", elfcore_grok_openbsd_note),
	      GROKER_ELEMENT ("QNX", elfcore_grok_nto_note),
	      GROKER_ELEMENT ("SPU/", elfcore_grok_spu_note),
	      GROKER_ELEMENT ("GNU", elfobj_grok_gnu_note),
	      GROKER_ELEMENT ("CORE", elfcore_grok_solaris_note)
	    };
#undef GROKER_ELEMENT
	    int i;

	    for (i = ARRAY_SIZE (grokers); i--;)
	      {
		if (in.namesz >= grokers[i].len
		    && strncmp (in.namedata, grokers[i].string,
				grokers[i].len) == 0)
		  {
		    if (! grokers[i].func (abfd, & in))
		      return false;
		    break;
		  }
	      }
	    break;
	  }

	case bfd_object:
	  if (in.namesz == sizeof "GNU" && strcmp (in.namedata, "GNU") == 0)
	    {
	      if (! elfobj_grok_gnu_note (abfd, &in))
		return false;
	    }
	  else if (in.namesz == sizeof "stapsdt"
		   && strcmp (in.namedata, "stapsdt") == 0)
	    {
	      if (! elfobj_grok_stapsdt_note (abfd, &in))
		return false;
	    }
	  break;
	}

      p += ELF_NOTE_NEXT_OFFSET (in.namesz, in.descsz, align);
    }

  return true;
}

bool
elf_read_notes (bfd *abfd, file_ptr offset, bfd_size_type size,
		size_t align)
{
  char *buf;

  if (size == 0 || (size + 1) == 0)
    return true;

  if (bfd_seek (abfd, offset, SEEK_SET) != 0)
    return false;

  buf = (char *) _bfd_malloc_and_read (abfd, size + 1, size);
  if (buf == NULL)
    return false;

  /* PR 17512: file: ec08f814
     0-termintate the buffer so that string searches will not overflow.  */
  buf[size] = 0;

  if (!elf_parse_notes (abfd, buf, size, offset, align))
    {
      free (buf);
      return false;
    }

  free (buf);
  return true;
}

/* Providing external access to the ELF program header table.  */

/* Return an upper bound on the number of bytes required to store a
   copy of ABFD's program header table entries.  Return -1 if an error
   occurs; bfd_get_error will return an appropriate code.  */

long
bfd_get_elf_phdr_upper_bound (bfd *abfd)
{
  if (abfd->xvec->flavour != bfd_target_elf_flavour)
    {
      bfd_set_error (bfd_error_wrong_format);
      return -1;
    }

  return elf_elfheader (abfd)->e_phnum * sizeof (Elf_Internal_Phdr);
}

/* Copy ABFD's program header table entries to *PHDRS.  The entries
   will be stored as an array of Elf_Internal_Phdr structures, as
   defined in include/elf/internal.h.  To find out how large the
   buffer needs to be, call bfd_get_elf_phdr_upper_bound.

   Return the number of program header table entries read, or -1 if an
   error occurs; bfd_get_error will return an appropriate code.  */

int
bfd_get_elf_phdrs (bfd *abfd, void *phdrs)
{
  int num_phdrs;

  if (abfd->xvec->flavour != bfd_target_elf_flavour)
    {
      bfd_set_error (bfd_error_wrong_format);
      return -1;
    }

  num_phdrs = elf_elfheader (abfd)->e_phnum;
  if (num_phdrs != 0)
    memcpy (phdrs, elf_tdata (abfd)->phdr,
	    num_phdrs * sizeof (Elf_Internal_Phdr));

  return num_phdrs;
}

enum elf_reloc_type_class
_bfd_elf_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			   const asection *rel_sec ATTRIBUTE_UNUSED,
			   const Elf_Internal_Rela *rela ATTRIBUTE_UNUSED)
{
  return reloc_class_normal;
}

/* For RELA architectures, return the relocation value for a
   relocation against a local symbol.  */

bfd_vma
_bfd_elf_rela_local_sym (bfd *abfd,
			 Elf_Internal_Sym *sym,
			 asection **psec,
			 Elf_Internal_Rela *rel)
{
  asection *sec = *psec;
  bfd_vma relocation;

  relocation = (sec->output_section->vma
		+ sec->output_offset
		+ sym->st_value);
  if ((sec->flags & SEC_MERGE)
      && ELF_ST_TYPE (sym->st_info) == STT_SECTION
      && sec->sec_info_type == SEC_INFO_TYPE_MERGE)
    {
      rel->r_addend =
	_bfd_merged_section_offset (abfd, psec,
				    elf_section_data (sec)->sec_info,
				    sym->st_value + rel->r_addend);
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

bfd_vma
_bfd_elf_rel_local_sym (bfd *abfd,
			Elf_Internal_Sym *sym,
			asection **psec,
			bfd_vma addend)
{
  asection *sec = *psec;

  if (sec->sec_info_type != SEC_INFO_TYPE_MERGE)
    return sym->st_value + addend;

  return _bfd_merged_section_offset (abfd, psec,
				     elf_section_data (sec)->sec_info,
				     sym->st_value + addend);
}

/* Adjust an address within a section.  Given OFFSET within SEC, return
   the new offset within the section, based upon changes made to the
   section.  Returns -1 if the offset is now invalid.
   The offset (in abnd out) is in target sized bytes, however big a
   byte may be.  */

bfd_vma
_bfd_elf_section_offset (bfd *abfd,
			 struct bfd_link_info *info,
			 asection *sec,
			 bfd_vma offset)
{
  switch (sec->sec_info_type)
    {
    case SEC_INFO_TYPE_STABS:
      return _bfd_stab_section_offset (sec, elf_section_data (sec)->sec_info,
				       offset);
    case SEC_INFO_TYPE_EH_FRAME:
      return _bfd_elf_eh_frame_section_offset (abfd, info, sec, offset);

    default:
      if ((sec->flags & SEC_ELF_REVERSE_COPY) != 0)
	{
	  /* Reverse the offset.  */
	  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
	  bfd_size_type address_size = bed->s->arch_size / 8;

	  /* address_size and sec->size are in octets.  Convert
	     to bytes before subtracting the original offset.  */
	  offset = ((sec->size - address_size)
		    / bfd_octets_per_byte (abfd, sec) - offset);
	}
      return offset;
    }
}

long
_bfd_elf_get_synthetic_symtab (bfd *abfd,
			       long symcount ATTRIBUTE_UNUSED,
			       asymbol **syms ATTRIBUTE_UNUSED,
			       long dynsymcount,
			       asymbol **dynsyms,
			       asymbol **ret)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  asection *relplt;
  asymbol *s;
  const char *relplt_name;
  bool (*slurp_relocs) (bfd *, asection *, asymbol **, bool);
  arelent *p;
  long count, i, n;
  size_t size;
  Elf_Internal_Shdr *hdr;
  char *names;
  asection *plt;

  *ret = NULL;

  if ((abfd->flags & (DYNAMIC | EXEC_P)) == 0)
    return 0;

  if (dynsymcount <= 0)
    return 0;

  if (!bed->plt_sym_val)
    return 0;

  relplt_name = bed->relplt_name;
  if (relplt_name == NULL)
    relplt_name = bed->rela_plts_and_copies_p ? ".rela.plt" : ".rel.plt";
  relplt = bfd_get_section_by_name (abfd, relplt_name);
  if (relplt == NULL)
    return 0;

  hdr = &elf_section_data (relplt)->this_hdr;
  if (hdr->sh_link != elf_dynsymtab (abfd)
      || (hdr->sh_type != SHT_REL && hdr->sh_type != SHT_RELA))
    return 0;

  plt = bfd_get_section_by_name (abfd, ".plt");
  if (plt == NULL)
    return 0;

  slurp_relocs = get_elf_backend_data (abfd)->s->slurp_reloc_table;
  if (! (*slurp_relocs) (abfd, relplt, dynsyms, true))
    return -1;

  count = NUM_SHDR_ENTRIES (hdr);
  size = count * sizeof (asymbol);
  p = relplt->relocation;
  for (i = 0; i < count; i++, p += bed->s->int_rels_per_ext_rel)
    {
      size += strlen ((*p->sym_ptr_ptr)->name) + sizeof ("@plt");
      if (p->addend != 0)
	{
#ifdef BFD64
	  size += sizeof ("+0x") - 1 + 8 + 8 * (bed->s->elfclass == ELFCLASS64);
#else
	  size += sizeof ("+0x") - 1 + 8;
#endif
	}
    }

  s = *ret = (asymbol *) bfd_malloc (size);
  if (s == NULL)
    return -1;

  names = (char *) (s + count);
  p = relplt->relocation;
  n = 0;
  for (i = 0; i < count; i++, p += bed->s->int_rels_per_ext_rel)
    {
      size_t len;
      bfd_vma addr;

      addr = bed->plt_sym_val (i, plt, p);
      if (addr == (bfd_vma) -1)
	continue;

      *s = **p->sym_ptr_ptr;
      /* Undefined syms won't have BSF_LOCAL or BSF_GLOBAL set.  Since
	 we are defining a symbol, ensure one of them is set.  */
      if ((s->flags & BSF_LOCAL) == 0)
	s->flags |= BSF_GLOBAL;
      s->flags |= BSF_SYNTHETIC;
      s->section = plt;
      s->value = addr - plt->vma;
      s->name = names;
      s->udata.p = NULL;
      len = strlen ((*p->sym_ptr_ptr)->name);
      memcpy (names, (*p->sym_ptr_ptr)->name, len);
      names += len;
      if (p->addend != 0)
	{
	  char buf[30], *a;

	  memcpy (names, "+0x", sizeof ("+0x") - 1);
	  names += sizeof ("+0x") - 1;
	  bfd_sprintf_vma (abfd, buf, p->addend);
	  for (a = buf; *a == '0'; ++a)
	    ;
	  len = strlen (a);
	  memcpy (names, a, len);
	  names += len;
	}
      memcpy (names, "@plt", sizeof ("@plt"));
      names += sizeof ("@plt");
      ++s, ++n;
    }

  return n;
}

/* It is only used by x86-64 so far.
   ??? This repeats *COM* id of zero.  sec->id is supposed to be unique,
   but current usage would allow all of _bfd_std_section to be zero.  */
static const asymbol lcomm_sym
  = GLOBAL_SYM_INIT ("LARGE_COMMON", &_bfd_elf_large_com_section);
asection _bfd_elf_large_com_section
  = BFD_FAKE_SECTION (_bfd_elf_large_com_section, &lcomm_sym,
		      "LARGE_COMMON", 0, SEC_IS_COMMON);

bool
_bfd_elf_final_write_processing (bfd *abfd)
{
  Elf_Internal_Ehdr *i_ehdrp;	/* ELF file header, internal form.  */

  i_ehdrp = elf_elfheader (abfd);

  if (i_ehdrp->e_ident[EI_OSABI] == ELFOSABI_NONE)
    i_ehdrp->e_ident[EI_OSABI] = get_elf_backend_data (abfd)->elf_osabi;

  /* Set the osabi field to ELFOSABI_GNU if the binary contains
     SHF_GNU_MBIND or SHF_GNU_RETAIN sections or symbols of STT_GNU_IFUNC type
     or STB_GNU_UNIQUE binding.  */
  if (elf_tdata (abfd)->has_gnu_osabi != 0)
    {
      if (i_ehdrp->e_ident[EI_OSABI] == ELFOSABI_NONE)
	i_ehdrp->e_ident[EI_OSABI] = ELFOSABI_GNU;
      else if (i_ehdrp->e_ident[EI_OSABI] != ELFOSABI_GNU
	       && i_ehdrp->e_ident[EI_OSABI] != ELFOSABI_FREEBSD)
	{
	  if (elf_tdata (abfd)->has_gnu_osabi & elf_gnu_osabi_mbind)
	    _bfd_error_handler (_("GNU_MBIND section is supported only by GNU "
				  "and FreeBSD targets"));
	  if (elf_tdata (abfd)->has_gnu_osabi & elf_gnu_osabi_ifunc)
	    _bfd_error_handler (_("symbol type STT_GNU_IFUNC is supported "
				  "only by GNU and FreeBSD targets"));
	  if (elf_tdata (abfd)->has_gnu_osabi & elf_gnu_osabi_unique)
	    _bfd_error_handler (_("symbol binding STB_GNU_UNIQUE is supported "
				  "only by GNU and FreeBSD targets"));
	  if (elf_tdata (abfd)->has_gnu_osabi & elf_gnu_osabi_retain)
	    _bfd_error_handler (_("GNU_RETAIN section is supported "
				  "only by GNU and FreeBSD targets"));
	  bfd_set_error (bfd_error_sorry);
	  return false;
	}
    }
  return true;
}


/* Return TRUE for ELF symbol types that represent functions.
   This is the default version of this function, which is sufficient for
   most targets.  It returns true if TYPE is STT_FUNC or STT_GNU_IFUNC.  */

bool
_bfd_elf_is_function_type (unsigned int type)
{
  return (type == STT_FUNC
	  || type == STT_GNU_IFUNC);
}

/* If the ELF symbol SYM might be a function in SEC, return the
   function size and set *CODE_OFF to the function's entry point,
   otherwise return zero.  */

bfd_size_type
_bfd_elf_maybe_function_sym (const asymbol *sym, asection *sec,
			     bfd_vma *code_off)
{
  bfd_size_type size;
  elf_symbol_type * elf_sym = (elf_symbol_type *) sym;

  if ((sym->flags & (BSF_SECTION_SYM | BSF_FILE | BSF_OBJECT
		     | BSF_THREAD_LOCAL | BSF_RELC | BSF_SRELC)) != 0
      || sym->section != sec)
    return 0;

  size = (sym->flags & BSF_SYNTHETIC) ? 0 : elf_sym->internal_elf_sym.st_size;

  /* In theory we should check that the symbol's type satisfies
     _bfd_elf_is_function_type(), but there are some function-like
     symbols which would fail this test.  (eg _start).  Instead
     we check for hidden, local, notype symbols with zero size.
     This type of symbol is generated by the annobin plugin for gcc
     and clang, and should not be considered to be a function symbol.  */
  if (size == 0
      && ((sym->flags & (BSF_SYNTHETIC | BSF_LOCAL)) == BSF_LOCAL)
      && ELF_ST_TYPE (elf_sym->internal_elf_sym.st_info) == STT_NOTYPE
      && ELF_ST_VISIBILITY (elf_sym->internal_elf_sym.st_other) == STV_HIDDEN)
    return 0;

  *code_off = sym->value;
  /* Do not return 0 for the function's size.  */
  return size ? size : 1;
}

/* Set to non-zero to enable some debug messages.  */
#define DEBUG_SECONDARY_RELOCS	 0

/* An internal-to-the-bfd-library only section type
   used to indicate a cached secondary reloc section.  */
#define SHT_SECONDARY_RELOC	 (SHT_LOOS + SHT_RELA)

/* Create a BFD section to hold a secondary reloc section.  */

bool
_bfd_elf_init_secondary_reloc_section (bfd * abfd,
				       Elf_Internal_Shdr *hdr,
				       const char * name,
				       unsigned int shindex)
{
  /* We only support RELA secondary relocs.  */
  if (hdr->sh_type != SHT_RELA)
    return false;

#if DEBUG_SECONDARY_RELOCS
  fprintf (stderr, "secondary reloc section %s encountered\n", name);
#endif
  hdr->sh_type = SHT_SECONDARY_RELOC;
  return _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
}

/* Read in any secondary relocs associated with SEC.  */

bool
_bfd_elf_slurp_secondary_reloc_section (bfd *       abfd,
					asection *  sec,
					asymbol **  symbols,
					bool dynamic)
{
  const struct elf_backend_data * const ebd = get_elf_backend_data (abfd);
  asection * relsec;
  bool result = true;
  bfd_vma (*r_sym) (bfd_vma);
  ufile_ptr filesize;

#if BFD_DEFAULT_TARGET_SIZE > 32
  if (bfd_arch_bits_per_address (abfd) != 32)
    r_sym = elf64_r_sym;
  else
#endif
    r_sym = elf32_r_sym;

  if (!elf_section_data (sec)->has_secondary_relocs)
    return true;

  /* Discover if there are any secondary reloc sections
     associated with SEC.  */
  filesize = bfd_get_file_size (abfd);
  for (relsec = abfd->sections; relsec != NULL; relsec = relsec->next)
    {
      Elf_Internal_Shdr * hdr = & elf_section_data (relsec)->this_hdr;

      if (hdr->sh_type == SHT_SECONDARY_RELOC
	  && hdr->sh_info == (unsigned) elf_section_data (sec)->this_idx
	  && (hdr->sh_entsize == ebd->s->sizeof_rel
	      || hdr->sh_entsize == ebd->s->sizeof_rela))
	{
	  bfd_byte * native_relocs;
	  bfd_byte * native_reloc;
	  arelent * internal_relocs;
	  arelent * internal_reloc;
	  size_t i;
	  unsigned int entsize;
	  unsigned int symcount;
	  bfd_size_type reloc_count;
	  size_t amt;

	  if (ebd->elf_info_to_howto == NULL)
	    return false;

#if DEBUG_SECONDARY_RELOCS
	  fprintf (stderr, "read secondary relocs for %s from %s\n",
		   sec->name, relsec->name);
#endif
	  entsize = hdr->sh_entsize;

	  if (filesize != 0
	      && ((ufile_ptr) hdr->sh_offset > filesize
		  || hdr->sh_size > filesize - hdr->sh_offset))
	    {
	      bfd_set_error (bfd_error_file_truncated);
	      result = false;
	      continue;
	    }

	  native_relocs = bfd_malloc (hdr->sh_size);
	  if (native_relocs == NULL)
	    {
	      result = false;
	      continue;
	    }

	  reloc_count = NUM_SHDR_ENTRIES (hdr);
	  if (_bfd_mul_overflow (reloc_count, sizeof (arelent), & amt))
	    {
	      free (native_relocs);
	      bfd_set_error (bfd_error_file_too_big);
	      result = false;
	      continue;
	    }

	  internal_relocs = (arelent *) bfd_alloc (abfd, amt);
	  if (internal_relocs == NULL)
	    {
	      free (native_relocs);
	      result = false;
	      continue;
	    }

	  if (bfd_seek (abfd, hdr->sh_offset, SEEK_SET) != 0
	      || (bfd_bread (native_relocs, hdr->sh_size, abfd)
		  != hdr->sh_size))
	    {
	      free (native_relocs);
	      /* The internal_relocs will be freed when
		 the memory for the bfd is released.  */
	      result = false;
	      continue;
	    }

	  if (dynamic)
	    symcount = bfd_get_dynamic_symcount (abfd);
	  else
	    symcount = bfd_get_symcount (abfd);

	  for (i = 0, internal_reloc = internal_relocs,
		 native_reloc = native_relocs;
	       i < reloc_count;
	       i++, internal_reloc++, native_reloc += entsize)
	    {
	      bool res;
	      Elf_Internal_Rela rela;

	      if (entsize == ebd->s->sizeof_rel)
		ebd->s->swap_reloc_in (abfd, native_reloc, & rela);
	      else /* entsize == ebd->s->sizeof_rela */
		ebd->s->swap_reloca_in (abfd, native_reloc, & rela);

	      /* The address of an ELF reloc is section relative for an object
		 file, and absolute for an executable file or shared library.
		 The address of a normal BFD reloc is always section relative,
		 and the address of a dynamic reloc is absolute..  */
	      if ((abfd->flags & (EXEC_P | DYNAMIC)) == 0)
		internal_reloc->address = rela.r_offset;
	      else
		internal_reloc->address = rela.r_offset - sec->vma;

	      if (r_sym (rela.r_info) == STN_UNDEF)
		{
		  /* FIXME: This and the error case below mean that we
		     have a symbol on relocs that is not elf_symbol_type.  */
		  internal_reloc->sym_ptr_ptr =
		    bfd_abs_section_ptr->symbol_ptr_ptr;
		}
	      else if (r_sym (rela.r_info) > symcount)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB(%pA): relocation %zu has invalid symbol index %lu"),
		     abfd, sec, i, (long) r_sym (rela.r_info));
		  bfd_set_error (bfd_error_bad_value);
		  internal_reloc->sym_ptr_ptr =
		    bfd_abs_section_ptr->symbol_ptr_ptr;
		  result = false;
		}
	      else
		{
		  asymbol **ps;

		  ps = symbols + r_sym (rela.r_info) - 1;
		  internal_reloc->sym_ptr_ptr = ps;
		  /* Make sure that this symbol is not removed by strip.  */
		  (*ps)->flags |= BSF_KEEP;
		}

	      internal_reloc->addend = rela.r_addend;

	      res = ebd->elf_info_to_howto (abfd, internal_reloc, & rela);
	      if (! res || internal_reloc->howto == NULL)
		{
#if DEBUG_SECONDARY_RELOCS
		  fprintf (stderr,
			   "there is no howto associated with reloc %lx\n",
			   rela.r_info);
#endif
		  result = false;
		}
	    }

	  free (native_relocs);
	  /* Store the internal relocs.  */
	  elf_section_data (relsec)->sec_info = internal_relocs;
	}
    }

  return result;
}

/* Set the ELF section header fields of an output secondary reloc section.  */

bool
_bfd_elf_copy_special_section_fields (const bfd *ibfd ATTRIBUTE_UNUSED,
				      bfd *obfd ATTRIBUTE_UNUSED,
				      const Elf_Internal_Shdr *isection,
				      Elf_Internal_Shdr *osection)
{
  asection * isec;
  asection * osec;
  struct bfd_elf_section_data * esd;

  if (isection == NULL)
    return false;

  if (isection->sh_type != SHT_SECONDARY_RELOC)
    return true;

  isec = isection->bfd_section;
  if (isec == NULL)
    return false;

  osec = osection->bfd_section;
  if (osec == NULL)
    return false;

  esd = elf_section_data (osec);
  BFD_ASSERT (esd->sec_info == NULL);
  esd->sec_info = elf_section_data (isec)->sec_info;
  osection->sh_type = SHT_RELA;
  osection->sh_link = elf_onesymtab (obfd);
  if (osection->sh_link == 0)
    {
      /* There is no symbol table - we are hosed...  */
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB(%pA): link section cannot be set"
	   " because the output file does not have a symbol table"),
	obfd, osec);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* Find the output section that corresponds to the isection's
     sh_info link.  */
  if (isection->sh_info == 0
      || isection->sh_info >= elf_numsections (ibfd))
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB(%pA): info section index is invalid"),
	obfd, osec);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  isection = elf_elfsections (ibfd)[isection->sh_info];

  if (isection == NULL
      || isection->bfd_section == NULL
      || isection->bfd_section->output_section == NULL)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB(%pA): info section index cannot be set"
	   " because the section is not in the output"),
	obfd, osec);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  esd = elf_section_data (isection->bfd_section->output_section);
  BFD_ASSERT (esd != NULL);
  osection->sh_info = esd->this_idx;
  esd->has_secondary_relocs = true;
#if DEBUG_SECONDARY_RELOCS
  fprintf (stderr, "update header of %s, sh_link = %u, sh_info = %u\n",
	   osec->name, osection->sh_link, osection->sh_info);
  fprintf (stderr, "mark section %s as having secondary relocs\n",
	   bfd_section_name (isection->bfd_section->output_section));
#endif

  return true;
}

/* Write out a secondary reloc section.

   FIXME: Currently this function can result in a serious performance penalty
   for files with secondary relocs and lots of sections.  The proper way to
   fix this is for _bfd_elf_copy_special_section_fields() to chain secondary
   relocs together and then to have this function just walk that chain.  */

bool
_bfd_elf_write_secondary_reloc_section (bfd *abfd, asection *sec)
{
  const struct elf_backend_data * const ebd = get_elf_backend_data (abfd);
  bfd_vma addr_offset;
  asection * relsec;
  bfd_vma (*r_info) (bfd_vma, bfd_vma);
  bool result = true;

  if (sec == NULL)
    return false;

#if BFD_DEFAULT_TARGET_SIZE > 32
  if (bfd_arch_bits_per_address (abfd) != 32)
    r_info = elf64_r_info;
  else
#endif
    r_info = elf32_r_info;

  /* The address of an ELF reloc is section relative for an object
     file, and absolute for an executable file or shared library.
     The address of a BFD reloc is always section relative.  */
  addr_offset = 0;
  if ((abfd->flags & (EXEC_P | DYNAMIC)) != 0)
    addr_offset = sec->vma;

  /* Discover if there are any secondary reloc sections
     associated with SEC.  */
  for (relsec = abfd->sections; relsec != NULL; relsec = relsec->next)
    {
      const struct bfd_elf_section_data * const esd = elf_section_data (relsec);
      Elf_Internal_Shdr * const hdr = (Elf_Internal_Shdr *) & esd->this_hdr;

      if (hdr->sh_type == SHT_RELA
	  && hdr->sh_info == (unsigned) elf_section_data (sec)->this_idx)
	{
	  asymbol *    last_sym;
	  int          last_sym_idx;
	  size_t       reloc_count;
	  size_t       idx;
	  bfd_size_type entsize;
	  arelent *    src_irel;
	  bfd_byte *   dst_rela;

	  if (hdr->contents != NULL)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA): error: secondary reloc section processed twice"),
		 abfd, relsec);
	      bfd_set_error (bfd_error_bad_value);
	      result = false;
	      continue;
	    }

	  entsize = hdr->sh_entsize;
	  if (entsize == 0)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA): error: secondary reloc section"
		   " has zero sized entries"),
		 abfd, relsec);
	      bfd_set_error (bfd_error_bad_value);
	      result = false;
	      continue;
	    }
	  else if (entsize != ebd->s->sizeof_rel
		   && entsize != ebd->s->sizeof_rela)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA): error: secondary reloc section"
		   " has non-standard sized entries"),
		 abfd, relsec);
	      bfd_set_error (bfd_error_bad_value);
	      result = false;
	      continue;
	    }

	  reloc_count = hdr->sh_size / entsize;
	  hdr->sh_size = entsize * reloc_count;
	  if (reloc_count == 0)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA): error: secondary reloc section is empty!"),
		 abfd, relsec);
	      bfd_set_error (bfd_error_bad_value);
	      result = false;
	      continue;
	    }

	  hdr->contents = bfd_alloc (abfd, hdr->sh_size);
	  if (hdr->contents == NULL)
	    continue;

#if DEBUG_SECONDARY_RELOCS
	  fprintf (stderr, "write %u secondary relocs for %s from %s\n",
		   reloc_count, sec->name, relsec->name);
#endif
	  last_sym = NULL;
	  last_sym_idx = 0;
	  dst_rela = hdr->contents;
	  src_irel = (arelent *) esd->sec_info;
	  if (src_irel == NULL)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA): error: internal relocs missing"
		   " for secondary reloc section"),
		 abfd, relsec);
	      bfd_set_error (bfd_error_bad_value);
	      result = false;
	      continue;
	    }

	  for (idx = 0; idx < reloc_count; idx++, dst_rela += entsize)
	    {
	      Elf_Internal_Rela src_rela;
	      arelent *ptr;
	      asymbol *sym;
	      int n;

	      ptr = src_irel + idx;
	      if (ptr == NULL)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB(%pA): error: reloc table entry %zu is empty"),
		     abfd, relsec, idx);
		  bfd_set_error (bfd_error_bad_value);
		  result = false;
		  break;
		}

	      if (ptr->sym_ptr_ptr == NULL)
		{
		  /* FIXME: Is this an error ? */
		  n = 0;
		}
	      else
		{
		  sym = *ptr->sym_ptr_ptr;

		  if (sym == last_sym)
		    n = last_sym_idx;
		  else
		    {
		      n = _bfd_elf_symbol_from_bfd_symbol (abfd, & sym);
		      if (n < 0)
			{
			  _bfd_error_handler
			    /* xgettext:c-format */
			    (_("%pB(%pA): error: secondary reloc %zu"
			       " references a missing symbol"),
			     abfd, relsec, idx);
			  bfd_set_error (bfd_error_bad_value);
			  result = false;
			  n = 0;
			}

		      last_sym = sym;
		      last_sym_idx = n;
		    }

		  if (sym->the_bfd != NULL
		      && sym->the_bfd->xvec != abfd->xvec
		      && ! _bfd_elf_validate_reloc (abfd, ptr))
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB(%pA): error: secondary reloc %zu"
			   " references a deleted symbol"),
			 abfd, relsec, idx);
		      bfd_set_error (bfd_error_bad_value);
		      result = false;
		      n = 0;
		    }
		}

	      src_rela.r_offset = ptr->address + addr_offset;
	      if (ptr->howto == NULL)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB(%pA): error: secondary reloc %zu"
		       " is of an unknown type"),
		     abfd, relsec, idx);
		  bfd_set_error (bfd_error_bad_value);
		  result = false;
		  src_rela.r_info = r_info (0, 0);
		}
	      else
		src_rela.r_info = r_info (n, ptr->howto->type);
	      src_rela.r_addend = ptr->addend;

	      if (entsize == ebd->s->sizeof_rel)
		ebd->s->swap_reloc_out (abfd, &src_rela, dst_rela);
	      else /* entsize == ebd->s->sizeof_rela */
		ebd->s->swap_reloca_out (abfd, &src_rela, dst_rela);
	    }
	}
    }

  return result;
}
