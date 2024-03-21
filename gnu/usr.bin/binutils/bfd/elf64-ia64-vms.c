/* IA-64 support for OpenVMS
   Copyright (C) 1998-2023 Free Software Foundation, Inc.

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
#include "opcode/ia64.h"
#include "elf/ia64.h"
#include "objalloc.h"
#include "hashtab.h"
#include "elfxx-ia64.h"
#include "vms.h"
#include "bfdver.h"

/* THE RULES for all the stuff the linker creates --

  GOT		Entries created in response to LTOFF or LTOFF_FPTR
		relocations.  Dynamic relocs created for dynamic
		symbols in an application; REL relocs for locals
		in a shared library.

  FPTR		The canonical function descriptor.  Created for local
		symbols in applications.  Descriptors for dynamic symbols
		and local symbols in shared libraries are created by
		ld.so.  Thus there are no dynamic relocs against these
		objects.  The FPTR relocs for such _are_ passed through
		to the dynamic relocation tables.

  FULL_PLT	Created for a PCREL21B relocation against a dynamic symbol.
		Requires the creation of a PLTOFF entry.  This does not
		require any dynamic relocations.

  PLTOFF	Created by PLTOFF relocations.  For local symbols, this
		is an alternate function descriptor, and in shared libraries
		requires two REL relocations.  Note that this cannot be
		transformed into an FPTR relocation, since it must be in
		range of the GP.  For dynamic symbols, this is a function
		descriptor.  */

typedef struct bfd_hash_entry *(*new_hash_entry_func)
  (struct bfd_hash_entry *, struct bfd_hash_table *, const char *);

/* In dynamically (linker-) created sections, we generally need to keep track
   of the place a symbol or expression got allocated to. This is done via hash
   tables that store entries of the following type.  */

struct elf64_ia64_dyn_sym_info
{
  /* The addend for which this entry is relevant.  */
  bfd_vma addend;

  bfd_vma got_offset;
  bfd_vma fptr_offset;
  bfd_vma pltoff_offset;
  bfd_vma plt_offset;
  bfd_vma plt2_offset;

  /* The symbol table entry, if any, that this was derived from.  */
  struct elf_link_hash_entry *h;

  /* Used to count non-got, non-plt relocations for delayed sizing
     of relocation sections.  */
  struct elf64_ia64_dyn_reloc_entry
  {
    struct elf64_ia64_dyn_reloc_entry *next;
    asection *srel;
    int type;
    int count;
  } *reloc_entries;

  /* TRUE when the section contents have been updated.  */
  unsigned got_done : 1;
  unsigned fptr_done : 1;
  unsigned pltoff_done : 1;

  /* TRUE for the different kinds of linker data we want created.  */
  unsigned want_got : 1;
  unsigned want_gotx : 1;
  unsigned want_fptr : 1;
  unsigned want_ltoff_fptr : 1;
  unsigned want_plt : 1;	/* A MIN_PLT entry.  */
  unsigned want_plt2 : 1;	/* A FULL_PLT.  */
  unsigned want_pltoff : 1;
};

struct elf64_ia64_local_hash_entry
{
  int id;
  unsigned int r_sym;
  /* The number of elements in elf64_ia64_dyn_sym_info array.  */
  unsigned int count;
  /* The number of sorted elements in elf64_ia64_dyn_sym_info array.  */
  unsigned int sorted_count;
  /* The size of elf64_ia64_dyn_sym_info array.  */
  unsigned int size;
  /* The array of elf64_ia64_dyn_sym_info.  */
  struct elf64_ia64_dyn_sym_info *info;

  /* TRUE if this hash entry's addends was translated for
     SHF_MERGE optimization.  */
  unsigned sec_merge_done : 1;
};

struct elf64_ia64_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* Set if this symbol is defined in a shared library.
     We can't use root.u.def.section->owner as the symbol is an absolute
     symbol.  */
  bfd *shl;

  /* The number of elements in elf64_ia64_dyn_sym_info array.  */
  unsigned int count;
  /* The number of sorted elements in elf64_ia64_dyn_sym_info array.  */
  unsigned int sorted_count;
  /* The size of elf64_ia64_dyn_sym_info array.  */
  unsigned int size;
  /* The array of elf64_ia64_dyn_sym_info.  */
  struct elf64_ia64_dyn_sym_info *info;
};

struct elf64_ia64_link_hash_table
{
  /* The main hash table.  */
  struct elf_link_hash_table root;

  asection *fptr_sec;		/* Function descriptor table (or NULL).  */
  asection *rel_fptr_sec;	/* Dynamic relocation section for same.  */
  asection *pltoff_sec;		/* Private descriptors for plt (or NULL).  */
  asection *fixups_sec;		/* Fixups section.  */
  asection *transfer_sec;	/* Transfer vector section.  */
  asection *note_sec;		/* .note section.  */

  /* There are maybe R_IA64_GPREL22 relocations, including those
     optimized from R_IA64_LTOFF22X, against non-SHF_IA_64_SHORT
     sections.  We need to record those sections so that we can choose
     a proper GP to cover all R_IA64_GPREL22 relocations.  */
  asection *max_short_sec;	/* Maximum short output section.  */
  bfd_vma max_short_offset;	/* Maximum short offset.  */
  asection *min_short_sec;	/* Minimum short output section.  */
  bfd_vma min_short_offset;	/* Minimum short offset.  */

  htab_t loc_hash_table;
  void *loc_hash_memory;
};

struct elf64_ia64_allocate_data
{
  struct bfd_link_info *info;
  bfd_size_type ofs;
};

#define elf64_ia64_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == IA64_ELF_DATA)		\
   ? (struct elf64_ia64_link_hash_table *) (p)->hash : NULL)

struct elf64_ia64_vms_obj_tdata
{
  struct elf_obj_tdata root;

  /* Ident for shared library.  */
  uint64_t ident;

  /* Used only during link: offset in the .fixups section for this bfd.  */
  bfd_vma fixups_off;

  /* Max number of shared libraries.  */
  unsigned int needed_count;
};

#define elf_ia64_vms_tdata(abfd) \
  ((struct elf64_ia64_vms_obj_tdata *)((abfd)->tdata.any))
#define elf_ia64_vms_ident(abfd) (elf_ia64_vms_tdata(abfd)->ident)

struct elf64_vms_transfer
{
  unsigned char size[4];
  unsigned char spare[4];
  unsigned char tfradr1[8];
  unsigned char tfradr2[8];
  unsigned char tfradr3[8];
  unsigned char tfradr4[8];
  unsigned char tfradr5[8];

  /* Local function descriptor for tfr3.  */
  unsigned char tfr3_func[8];
  unsigned char tfr3_gp[8];
};

typedef struct
{
  Elf64_External_Ehdr ehdr;
  unsigned char vms_needed_count[8];
} Elf64_External_VMS_Ehdr;

static struct elf64_ia64_dyn_sym_info * get_dyn_sym_info
  (struct elf64_ia64_link_hash_table *,
   struct elf_link_hash_entry *,
   bfd *, const Elf_Internal_Rela *, bool);
static bool elf64_ia64_dynamic_symbol_p
  (struct elf_link_hash_entry *);
static bool elf64_ia64_choose_gp
  (bfd *, struct bfd_link_info *, bool);
static void elf64_ia64_dyn_sym_traverse
  (struct elf64_ia64_link_hash_table *,
   bool (*) (struct elf64_ia64_dyn_sym_info *, void *),
   void *);
static bool allocate_global_data_got
  (struct elf64_ia64_dyn_sym_info *, void *);
static bool allocate_global_fptr_got
  (struct elf64_ia64_dyn_sym_info *, void *);
static bool allocate_local_got
  (struct elf64_ia64_dyn_sym_info *, void *);
static bool allocate_dynrel_entries
  (struct elf64_ia64_dyn_sym_info *, void *);
static asection *get_pltoff
  (bfd *, struct elf64_ia64_link_hash_table *);
static asection *get_got
  (bfd *, struct elf64_ia64_link_hash_table *);


/* Given a ELF reloc, return the matching HOWTO structure.  */

static bool
elf64_ia64_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
			  arelent *bfd_reloc,
			  Elf_Internal_Rela *elf_reloc)
{
  unsigned int r_type = ELF32_R_TYPE (elf_reloc->r_info);

  bfd_reloc->howto = ia64_elf_lookup_howto (r_type);
  if (bfd_reloc->howto == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return true;
}


#define PLT_FULL_ENTRY_SIZE	(2 * 16)

static const bfd_byte plt_full_entry[PLT_FULL_ENTRY_SIZE] =
{
  0x0b, 0x78, 0x00, 0x02, 0x00, 0x24,  /*   [MMI]	addl r15=0,r1;;	  */
  0x00, 0x41, 0x3c, 0x70, 0x29, 0xc0,  /*		ld8.acq r16=[r15],8*/
  0x01, 0x08, 0x00, 0x84,	       /*		mov r14=r1;;	  */
  0x11, 0x08, 0x00, 0x1e, 0x18, 0x10,  /*   [MIB]	ld8 r1=[r15]	  */
  0x60, 0x80, 0x04, 0x80, 0x03, 0x00,  /*		mov b6=r16	  */
  0x60, 0x00, 0x80, 0x00	       /*		br.few b6;;	  */
};

static const bfd_byte oor_brl[16] =
{
  0x05, 0x00, 0x00, 0x00, 0x01, 0x00,  /*  [MLX]	nop.m 0		  */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /*		brl.sptk.few tgt;;*/
  0x00, 0x00, 0x00, 0xc0
};


/* These functions do relaxation for IA-64 ELF.  */

/* Rename some of the generic section flags to better document how they
   are used here.  */
#define skip_relax_pass_0 sec_flg0
#define skip_relax_pass_1 sec_flg1

static void
elf64_ia64_update_short_info (asection *sec, bfd_vma offset,
			      struct elf64_ia64_link_hash_table *ia64_info)
{
  /* Skip ABS and SHF_IA_64_SHORT sections.  */
  if (sec == bfd_abs_section_ptr
      || (sec->flags & SEC_SMALL_DATA) != 0)
    return;

  if (!ia64_info->min_short_sec)
    {
      ia64_info->max_short_sec = sec;
      ia64_info->max_short_offset = offset;
      ia64_info->min_short_sec = sec;
      ia64_info->min_short_offset = offset;
    }
  else if (sec == ia64_info->max_short_sec
	   && offset > ia64_info->max_short_offset)
    ia64_info->max_short_offset = offset;
  else if (sec == ia64_info->min_short_sec
	   && offset < ia64_info->min_short_offset)
    ia64_info->min_short_offset = offset;
  else if (sec->output_section->vma
	   > ia64_info->max_short_sec->vma)
    {
      ia64_info->max_short_sec = sec;
      ia64_info->max_short_offset = offset;
    }
  else if (sec->output_section->vma
	   < ia64_info->min_short_sec->vma)
    {
      ia64_info->min_short_sec = sec;
      ia64_info->min_short_offset = offset;
    }
}

/* Use a two passes algorithm.  In the first pass, branches are relaxed
   (which may increase the size of the section).  In the second pass,
   the other relaxations are done.
*/

static bool
elf64_ia64_relax_section (bfd *abfd, asection *sec,
			  struct bfd_link_info *link_info,
			  bool *again)
{
  struct one_fixup
    {
      struct one_fixup *next;
      asection *tsec;
      bfd_vma toff;
      bfd_vma trampoff;
    };

  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs;
  Elf_Internal_Rela *irel, *irelend;
  bfd_byte *contents;
  Elf_Internal_Sym *isymbuf = NULL;
  struct elf64_ia64_link_hash_table *ia64_info;
  struct one_fixup *fixups = NULL;
  bool changed_contents = false;
  bool changed_relocs = false;
  bool skip_relax_pass_0 = true;
  bool skip_relax_pass_1 = true;
  bfd_vma gp = 0;

  /* Assume we're not going to change any sizes, and we'll only need
     one pass.  */
  *again = false;

  if (bfd_link_relocatable (link_info))
    (*link_info->callbacks->einfo)
      (_("%P%F: --relax and -r may not be used together\n"));

  /* Don't even try to relax for non-ELF outputs.  */
  if (!is_elf_hash_table (link_info->hash))
    return false;

  /* Nothing to do if there are no relocations or there is no need for
     the current pass.  */
  if (sec->reloc_count == 0
      || (sec->flags & SEC_RELOC) == 0
      || (sec->flags & SEC_HAS_CONTENTS) == 0
      || (link_info->relax_pass == 0 && sec->skip_relax_pass_0)
      || (link_info->relax_pass == 1 && sec->skip_relax_pass_1))
    return true;

  ia64_info = elf64_ia64_hash_table (link_info);
  if (ia64_info == NULL)
    return false;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  /* Load the relocations for this section.  */
  internal_relocs = (_bfd_elf_link_read_relocs
		     (abfd, sec, NULL, (Elf_Internal_Rela *) NULL,
		      link_info->keep_memory));
  if (internal_relocs == NULL)
    return false;

  irelend = internal_relocs + sec->reloc_count;

  /* Get the section contents.  */
  if (elf_section_data (sec)->this_hdr.contents != NULL)
    contents = elf_section_data (sec)->this_hdr.contents;
  else
    {
      if (!bfd_malloc_and_get_section (abfd, sec, &contents))
	goto error_return;
    }

  for (irel = internal_relocs; irel < irelend; irel++)
    {
      unsigned long r_type = ELF64_R_TYPE (irel->r_info);
      bfd_vma symaddr, reladdr, trampoff, toff, roff;
      asection *tsec;
      struct one_fixup *f;
      bfd_size_type amt;
      bool is_branch;
      struct elf64_ia64_dyn_sym_info *dyn_i;

      switch (r_type)
	{
	case R_IA64_PCREL21B:
	case R_IA64_PCREL21BI:
	case R_IA64_PCREL21M:
	case R_IA64_PCREL21F:
	  /* In pass 1, all br relaxations are done. We can skip it. */
	  if (link_info->relax_pass == 1)
	    continue;
	  skip_relax_pass_0 = false;
	  is_branch = true;
	  break;

	case R_IA64_PCREL60B:
	  /* We can't optimize brl to br in pass 0 since br relaxations
	     will increase the code size. Defer it to pass 1.  */
	  if (link_info->relax_pass == 0)
	    {
	      skip_relax_pass_1 = false;
	      continue;
	    }
	  is_branch = true;
	  break;

	case R_IA64_GPREL22:
	  /* Update max_short_sec/min_short_sec.  */

	case R_IA64_LTOFF22X:
	case R_IA64_LDXMOV:
	  /* We can't relax ldx/mov in pass 0 since br relaxations will
	     increase the code size. Defer it to pass 1.  */
	  if (link_info->relax_pass == 0)
	    {
	      skip_relax_pass_1 = false;
	      continue;
	    }
	  is_branch = false;
	  break;

	default:
	  continue;
	}

      /* Get the value of the symbol referred to by the reloc.  */
      if (ELF64_R_SYM (irel->r_info) < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  Elf_Internal_Sym *isym;

	  /* Read this BFD's local symbols.  */
	  if (isymbuf == NULL)
	    {
	      isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	      if (isymbuf == NULL)
		isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr,
						symtab_hdr->sh_info, 0,
						NULL, NULL, NULL);
	      if (isymbuf == 0)
		goto error_return;
	    }

	  isym = isymbuf + ELF64_R_SYM (irel->r_info);
	  if (isym->st_shndx == SHN_UNDEF)
	    continue;	/* We can't do anything with undefined symbols.  */
	  else if (isym->st_shndx == SHN_ABS)
	    tsec = bfd_abs_section_ptr;
	  else if (isym->st_shndx == SHN_COMMON)
	    tsec = bfd_com_section_ptr;
	  else if (isym->st_shndx == SHN_IA_64_ANSI_COMMON)
	    tsec = bfd_com_section_ptr;
	  else
	    tsec = bfd_section_from_elf_index (abfd, isym->st_shndx);

	  toff = isym->st_value;
	  dyn_i = get_dyn_sym_info (ia64_info, NULL, abfd, irel, false);
	}
      else
	{
	  unsigned long indx;
	  struct elf_link_hash_entry *h;

	  indx = ELF64_R_SYM (irel->r_info) - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	  BFD_ASSERT (h != NULL);

	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  dyn_i = get_dyn_sym_info (ia64_info, h, abfd, irel, false);

	  /* For branches to dynamic symbols, we're interested instead
	     in a branch to the PLT entry.  */
	  if (is_branch && dyn_i && dyn_i->want_plt2)
	    {
	      /* Internal branches shouldn't be sent to the PLT.
		 Leave this for now and we'll give an error later.  */
	      if (r_type != R_IA64_PCREL21B)
		continue;

	      tsec = ia64_info->root.splt;
	      toff = dyn_i->plt2_offset;
	      BFD_ASSERT (irel->r_addend == 0);
	    }

	  /* Can't do anything else with dynamic symbols.  */
	  else if (elf64_ia64_dynamic_symbol_p (h))
	    continue;

	  else
	    {
	      /* We can't do anything with undefined symbols.  */
	      if (h->root.type == bfd_link_hash_undefined
		  || h->root.type == bfd_link_hash_undefweak)
		continue;

	      tsec = h->root.u.def.section;
	      toff = h->root.u.def.value;
	    }
	}

      toff += irel->r_addend;

      symaddr = tsec->output_section->vma + tsec->output_offset + toff;

      roff = irel->r_offset;

      if (is_branch)
	{
	  bfd_signed_vma offset;

	  reladdr = (sec->output_section->vma
		     + sec->output_offset
		     + roff) & (bfd_vma) -4;

	  /* The .plt section is aligned at 32byte and the .text section
	     is aligned at 64byte. The .text section is right after the
	     .plt section.  After the first relaxation pass, linker may
	     increase the gap between the .plt and .text sections up
	     to 32byte.  We assume linker will always insert 32byte
	     between the .plt and .text sections after the first
	     relaxation pass.  */
	  if (tsec == ia64_info->root.splt)
	    offset = -0x1000000 + 32;
	  else
	    offset = -0x1000000;

	  /* If the branch is in range, no need to do anything.  */
	  if ((bfd_signed_vma) (symaddr - reladdr) >= offset
	      && (bfd_signed_vma) (symaddr - reladdr) <= 0x0FFFFF0)
	    {
	      /* If the 60-bit branch is in 21-bit range, optimize it. */
	      if (r_type == R_IA64_PCREL60B)
		{
		  ia64_elf_relax_brl (contents, roff);

		  irel->r_info = ELF64_R_INFO (ELF64_R_SYM (irel->r_info),
					       R_IA64_PCREL21B);

		  /* If the original relocation offset points to slot
		     1, change it to slot 2.  */
		  if ((irel->r_offset & 3) == 1)
		    irel->r_offset += 1;
		}

	      continue;
	    }
	  else if (r_type == R_IA64_PCREL60B)
	    continue;
	  else if (ia64_elf_relax_br (contents, roff))
	    {
	      irel->r_info = ELF64_R_INFO (ELF64_R_SYM (irel->r_info),
					   R_IA64_PCREL60B);

	      /* Make the relocation offset point to slot 1.  */
	      irel->r_offset = (irel->r_offset & ~((bfd_vma) 0x3)) + 1;
	      continue;
	    }

	  /* We can't put a trampoline in a .init/.fini section. Issue
	     an error.  */
	  if (strcmp (sec->output_section->name, ".init") == 0
	      || strcmp (sec->output_section->name, ".fini") == 0)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: can't relax br at %#" PRIx64 " in section `%pA';"
		   " please use brl or indirect branch"),
		 sec->owner, (uint64_t) roff, sec);
	      bfd_set_error (bfd_error_bad_value);
	      goto error_return;
	    }

	  /* If the branch and target are in the same section, you've
	     got one honking big section and we can't help you unless
	     you are branching backwards.  You'll get an error message
	     later.  */
	  if (tsec == sec && toff > roff)
	    continue;

	  /* Look for an existing fixup to this address.  */
	  for (f = fixups; f ; f = f->next)
	    if (f->tsec == tsec && f->toff == toff)
	      break;

	  if (f == NULL)
	    {
	      /* Two alternatives: If it's a branch to a PLT entry, we can
		 make a copy of the FULL_PLT entry.  Otherwise, we'll have
		 to use a `brl' insn to get where we're going.  */

	      size_t size;

	      if (tsec == ia64_info->root.splt)
		size = sizeof (plt_full_entry);
	      else
		size = sizeof (oor_brl);

	      /* Resize the current section to make room for the new branch. */
	      trampoff = (sec->size + 15) & (bfd_vma) -16;

	      /* If trampoline is out of range, there is nothing we
		 can do.  */
	      offset = trampoff - (roff & (bfd_vma) -4);
	      if (offset < -0x1000000 || offset > 0x0FFFFF0)
		continue;

	      amt = trampoff + size;
	      contents = (bfd_byte *) bfd_realloc (contents, amt);
	      if (contents == NULL)
		goto error_return;
	      sec->size = amt;

	      if (tsec == ia64_info->root.splt)
		{
		  memcpy (contents + trampoff, plt_full_entry, size);

		  /* Hijack the old relocation for use as the PLTOFF reloc.  */
		  irel->r_info = ELF64_R_INFO (ELF64_R_SYM (irel->r_info),
					       R_IA64_PLTOFF22);
		  irel->r_offset = trampoff;
		}
	      else
		{
		  memcpy (contents + trampoff, oor_brl, size);
		  irel->r_info = ELF64_R_INFO (ELF64_R_SYM (irel->r_info),
					       R_IA64_PCREL60B);
		  irel->r_offset = trampoff + 2;
		}

	      /* Record the fixup so we don't do it again this section.  */
	      f = (struct one_fixup *)
		bfd_malloc ((bfd_size_type) sizeof (*f));
	      f->next = fixups;
	      f->tsec = tsec;
	      f->toff = toff;
	      f->trampoff = trampoff;
	      fixups = f;
	    }
	  else
	    {
	      /* If trampoline is out of range, there is nothing we
		 can do.  */
	      offset = f->trampoff - (roff & (bfd_vma) -4);
	      if (offset < -0x1000000 || offset > 0x0FFFFF0)
		continue;

	      /* Nop out the reloc, since we're finalizing things here.  */
	      irel->r_info = ELF64_R_INFO (0, R_IA64_NONE);
	    }

	  /* Fix up the existing branch to hit the trampoline.  */
	  if (ia64_elf_install_value (contents + roff, offset, r_type)
	      != bfd_reloc_ok)
	    goto error_return;

	  changed_contents = true;
	  changed_relocs = true;
	}
      else
	{
	  /* Fetch the gp.  */
	  if (gp == 0)
	    {
	      bfd *obfd = sec->output_section->owner;
	      gp = _bfd_get_gp_value (obfd);
	      if (gp == 0)
		{
		  if (!elf64_ia64_choose_gp (obfd, link_info, false))
		    goto error_return;
		  gp = _bfd_get_gp_value (obfd);
		}
	    }

	  /* If the data is out of range, do nothing.  */
	  if ((bfd_signed_vma) (symaddr - gp) >= 0x200000
	      ||(bfd_signed_vma) (symaddr - gp) < -0x200000)
	    continue;

	  if (r_type == R_IA64_GPREL22)
	    elf64_ia64_update_short_info (tsec->output_section,
					  tsec->output_offset + toff,
					  ia64_info);
	  else if (r_type == R_IA64_LTOFF22X)
	    {
	      /* Can't deal yet correctly with ABS symbols.  */
	      if (bfd_is_abs_section (tsec))
		continue;

	      irel->r_info = ELF64_R_INFO (ELF64_R_SYM (irel->r_info),
					   R_IA64_GPREL22);
	      changed_relocs = true;

	      elf64_ia64_update_short_info (tsec->output_section,
					    tsec->output_offset + toff,
					    ia64_info);
	    }
	  else
	    {
	      ia64_elf_relax_ldxmov (contents, roff);
	      irel->r_info = ELF64_R_INFO (0, R_IA64_NONE);
	      changed_contents = true;
	      changed_relocs = true;
	    }
	}
    }

  /* ??? If we created fixups, this may push the code segment large
     enough that the data segment moves, which will change the GP.
     Reset the GP so that we re-calculate next round.  We need to
     do this at the _beginning_ of the next round; now will not do.  */

  /* Clean up and go home.  */
  while (fixups)
    {
      struct one_fixup *f = fixups;
      fixups = fixups->next;
      free (f);
    }

  if (isymbuf != NULL
      && symtab_hdr->contents != (unsigned char *) isymbuf)
    {
      if (! link_info->keep_memory)
	free (isymbuf);
      else
	{
	  /* Cache the symbols for elf_link_input_bfd.  */
	  symtab_hdr->contents = (unsigned char *) isymbuf;
	}
    }

  if (contents != NULL
      && elf_section_data (sec)->this_hdr.contents != contents)
    {
      if (!changed_contents && !link_info->keep_memory)
	free (contents);
      else
	{
	  /* Cache the section contents for elf_link_input_bfd.  */
	  elf_section_data (sec)->this_hdr.contents = contents;
	}
    }

  if (elf_section_data (sec)->relocs != internal_relocs)
    {
      if (!changed_relocs)
	free (internal_relocs);
      else
	elf_section_data (sec)->relocs = internal_relocs;
    }

  if (link_info->relax_pass == 0)
    {
      /* Pass 0 is only needed to relax br.  */
      sec->skip_relax_pass_0 = skip_relax_pass_0;
      sec->skip_relax_pass_1 = skip_relax_pass_1;
    }

  *again = changed_contents || changed_relocs;
  return true;

 error_return:
  if ((unsigned char *) isymbuf != symtab_hdr->contents)
    free (isymbuf);
  if (elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);
  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);
  return false;
}
#undef skip_relax_pass_0
#undef skip_relax_pass_1

/* Return TRUE if NAME is an unwind table section name.  */

static inline bool
is_unwind_section_name (bfd *abfd ATTRIBUTE_UNUSED, const char *name)
{
  return ((startswith (name, ELF_STRING_ia64_unwind)
	   && ! startswith (name, ELF_STRING_ia64_unwind_info))
	  || startswith (name, ELF_STRING_ia64_unwind_once));
}


/* Convert IA-64 specific section flags to bfd internal section flags.  */

/* ??? There is no bfd internal flag equivalent to the SHF_IA_64_NORECOV
   flag.  */

static bool
elf64_ia64_section_flags (const Elf_Internal_Shdr *hdr)
{
  if (hdr->sh_flags & SHF_IA_64_SHORT)
    hdr->bfd_section->flags |= SEC_SMALL_DATA;

  return true;
}

/* Set the correct type for an IA-64 ELF section.  We do this by the
   section name, which is a hack, but ought to work.  */

static bool
elf64_ia64_fake_sections (bfd *abfd, Elf_Internal_Shdr *hdr,
			  asection *sec)
{
  const char *name;

  name = bfd_section_name (sec);

  if (is_unwind_section_name (abfd, name))
    {
      /* We don't have the sections numbered at this point, so sh_info
	 is set later, in elf64_ia64_final_write_processing.  */
      hdr->sh_type = SHT_IA_64_UNWIND;
      hdr->sh_flags |= SHF_LINK_ORDER;
    }
  else if (strcmp (name, ELF_STRING_ia64_archext) == 0)
    hdr->sh_type = SHT_IA_64_EXT;

  if (sec->flags & SEC_SMALL_DATA)
    hdr->sh_flags |= SHF_IA_64_SHORT;

  return true;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it to put .comm items in .sbss, and not .bss.  */

static bool
elf64_ia64_add_symbol_hook (bfd *abfd,
			    struct bfd_link_info *info,
			    Elf_Internal_Sym *sym,
			    const char **namep ATTRIBUTE_UNUSED,
			    flagword *flagsp ATTRIBUTE_UNUSED,
			    asection **secp,
			    bfd_vma *valp)
{
  if (sym->st_shndx == SHN_COMMON
      && !bfd_link_relocatable (info)
      && sym->st_size <= elf_gp_size (abfd))
    {
      /* Common symbols less than or equal to -G nn bytes are
	 automatically put into .sbss.  */

      asection *scomm = bfd_get_section_by_name (abfd, ".scommon");

      if (scomm == NULL)
	{
	  scomm = bfd_make_section_with_flags (abfd, ".scommon",
					       (SEC_ALLOC
						| SEC_IS_COMMON
						| SEC_SMALL_DATA
						| SEC_LINKER_CREATED));
	  if (scomm == NULL)
	    return false;
	}

      *secp = scomm;
      *valp = sym->st_size;
    }

  return true;
}

/* According to the Tahoe assembler spec, all labels starting with a
   '.' are local.  */

static bool
elf64_ia64_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED,
				const char *name)
{
  return name[0] == '.';
}

/* Should we do dynamic things to this symbol?  */

static bool
elf64_ia64_dynamic_symbol_p (struct elf_link_hash_entry *h)
{
  return h != NULL && h->def_dynamic;
}

static struct bfd_hash_entry*
elf64_ia64_new_elf_hash_entry (struct bfd_hash_entry *entry,
			       struct bfd_hash_table *table,
			       const char *string)
{
  struct elf64_ia64_link_hash_entry *ret;
  ret = (struct elf64_ia64_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (!ret)
    ret = bfd_hash_allocate (table, sizeof (*ret));

  if (!ret)
    return 0;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf64_ia64_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));

  ret->info = NULL;
  ret->count = 0;
  ret->sorted_count = 0;
  ret->size = 0;
  return (struct bfd_hash_entry *) ret;
}

static void
elf64_ia64_hash_hide_symbol (struct bfd_link_info *info,
			     struct elf_link_hash_entry *xh,
			     bool force_local)
{
  struct elf64_ia64_link_hash_entry *h;
  struct elf64_ia64_dyn_sym_info *dyn_i;
  unsigned int count;

  h = (struct elf64_ia64_link_hash_entry *)xh;

  _bfd_elf_link_hash_hide_symbol (info, &h->root, force_local);

  for (count = h->count, dyn_i = h->info;
       count != 0;
       count--, dyn_i++)
    {
      dyn_i->want_plt2 = 0;
      dyn_i->want_plt = 0;
    }
}

/* Compute a hash of a local hash entry.  */

static hashval_t
elf64_ia64_local_htab_hash (const void *ptr)
{
  struct elf64_ia64_local_hash_entry *entry
    = (struct elf64_ia64_local_hash_entry *) ptr;

  return ELF_LOCAL_SYMBOL_HASH (entry->id, entry->r_sym);
}

/* Compare local hash entries.  */

static int
elf64_ia64_local_htab_eq (const void *ptr1, const void *ptr2)
{
  struct elf64_ia64_local_hash_entry *entry1
    = (struct elf64_ia64_local_hash_entry *) ptr1;
  struct elf64_ia64_local_hash_entry *entry2
    = (struct elf64_ia64_local_hash_entry *) ptr2;

  return entry1->id == entry2->id && entry1->r_sym == entry2->r_sym;
}

/* Free the global elf64_ia64_dyn_sym_info array.  */

static bool
elf64_ia64_global_dyn_info_free (struct elf_link_hash_entry *xentry,
				 void * unused ATTRIBUTE_UNUSED)
{
  struct elf64_ia64_link_hash_entry *entry
    = (struct elf64_ia64_link_hash_entry *) xentry;

  if (entry->root.root.type == bfd_link_hash_warning)
    entry = (struct elf64_ia64_link_hash_entry *) entry->root.root.u.i.link;

  free (entry->info);
  entry->info = NULL;
  entry->count = 0;
  entry->sorted_count = 0;
  entry->size = 0;

  return true;
}

/* Free the local elf64_ia64_dyn_sym_info array.  */

static int
elf64_ia64_local_dyn_info_free (void **slot,
				void * unused ATTRIBUTE_UNUSED)
{
  struct elf64_ia64_local_hash_entry *entry
    = (struct elf64_ia64_local_hash_entry *) *slot;

  free (entry->info);
  entry->info = NULL;
  entry->count = 0;
  entry->sorted_count = 0;
  entry->size = 0;

  return true;
}

/* Destroy IA-64 linker hash table.  */

static void
elf64_ia64_link_hash_table_free (bfd *obfd)
{
  struct elf64_ia64_link_hash_table *ia64_info
    = (struct elf64_ia64_link_hash_table *) obfd->link.hash;
  if (ia64_info->loc_hash_table)
    {
      htab_traverse (ia64_info->loc_hash_table,
		     elf64_ia64_local_dyn_info_free, NULL);
      htab_delete (ia64_info->loc_hash_table);
    }
  if (ia64_info->loc_hash_memory)
    objalloc_free ((struct objalloc *) ia64_info->loc_hash_memory);
  elf_link_hash_traverse (&ia64_info->root,
			  elf64_ia64_global_dyn_info_free, NULL);
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create the derived linker hash table.  The IA-64 ELF port uses this
   derived hash table to keep information specific to the IA-64 ElF
   linker (without using static variables).  */

static struct bfd_link_hash_table *
elf64_ia64_hash_table_create (bfd *abfd)
{
  struct elf64_ia64_link_hash_table *ret;

  ret = bfd_zmalloc ((bfd_size_type) sizeof (*ret));
  if (!ret)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      elf64_ia64_new_elf_hash_entry,
				      sizeof (struct elf64_ia64_link_hash_entry),
				      IA64_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->loc_hash_table = htab_try_create (1024, elf64_ia64_local_htab_hash,
					 elf64_ia64_local_htab_eq, NULL);
  ret->loc_hash_memory = objalloc_create ();
  if (!ret->loc_hash_table || !ret->loc_hash_memory)
    {
      elf64_ia64_link_hash_table_free (abfd);
      return NULL;
    }
  ret->root.root.hash_table_free = elf64_ia64_link_hash_table_free;

  return &ret->root.root;
}

/* Traverse both local and global hash tables.  */

struct elf64_ia64_dyn_sym_traverse_data
{
  bool (*func) (struct elf64_ia64_dyn_sym_info *, void *);
  void * data;
};

static bool
elf64_ia64_global_dyn_sym_thunk (struct elf_link_hash_entry *xentry,
				 void * xdata)
{
  struct elf64_ia64_link_hash_entry *entry
    = (struct elf64_ia64_link_hash_entry *) xentry;
  struct elf64_ia64_dyn_sym_traverse_data *data
    = (struct elf64_ia64_dyn_sym_traverse_data *) xdata;
  struct elf64_ia64_dyn_sym_info *dyn_i;
  unsigned int count;

  if (entry->root.root.type == bfd_link_hash_warning)
    entry = (struct elf64_ia64_link_hash_entry *) entry->root.root.u.i.link;

  for (count = entry->count, dyn_i = entry->info;
       count != 0;
       count--, dyn_i++)
    if (! (*data->func) (dyn_i, data->data))
      return false;
  return true;
}

static int
elf64_ia64_local_dyn_sym_thunk (void **slot, void * xdata)
{
  struct elf64_ia64_local_hash_entry *entry
    = (struct elf64_ia64_local_hash_entry *) *slot;
  struct elf64_ia64_dyn_sym_traverse_data *data
    = (struct elf64_ia64_dyn_sym_traverse_data *) xdata;
  struct elf64_ia64_dyn_sym_info *dyn_i;
  unsigned int count;

  for (count = entry->count, dyn_i = entry->info;
       count != 0;
       count--, dyn_i++)
    if (! (*data->func) (dyn_i, data->data))
      return false;
  return true;
}

static void
elf64_ia64_dyn_sym_traverse (struct elf64_ia64_link_hash_table *ia64_info,
			     bool (*func) (struct elf64_ia64_dyn_sym_info *, void *),
			     void * data)
{
  struct elf64_ia64_dyn_sym_traverse_data xdata;

  xdata.func = func;
  xdata.data = data;

  elf_link_hash_traverse (&ia64_info->root,
			  elf64_ia64_global_dyn_sym_thunk, &xdata);
  htab_traverse (ia64_info->loc_hash_table,
		 elf64_ia64_local_dyn_sym_thunk, &xdata);
}

#define NOTE_NAME "IPF/VMS"

static bool
create_ia64_vms_notes (bfd *abfd, struct bfd_link_info *info,
		       unsigned int time_hi, unsigned int time_lo)
{
#define NBR_NOTES 7
  Elf_Internal_Note notes[NBR_NOTES];
  char *module_name;
  int module_name_len;
  unsigned char cur_time[8];
  Elf64_External_VMS_ORIG_DYN_Note *orig_dyn;
  unsigned int orig_dyn_size;
  unsigned int note_size;
  int i;
  unsigned char *noteptr;
  unsigned char *note_contents;
  struct elf64_ia64_link_hash_table *ia64_info;

  ia64_info = elf64_ia64_hash_table (info);

  module_name = vms_get_module_name (bfd_get_filename (abfd), true);
  module_name_len = strlen (module_name) + 1;

  bfd_putl32 (time_lo, cur_time + 0);
  bfd_putl32 (time_hi, cur_time + 4);

  /* Note 0: IMGNAM.  */
  notes[0].type = NT_VMS_IMGNAM;
  notes[0].descdata = module_name;
  notes[0].descsz = module_name_len;

  /* Note 1: GSTNAM.  */
  notes[1].type = NT_VMS_GSTNAM;
  notes[1].descdata = module_name;
  notes[1].descsz = module_name_len;

  /* Note 2: IMGID.  */
#define IMG_ID "V1.0"
  notes[2].type = NT_VMS_IMGID;
  notes[2].descdata = IMG_ID;
  notes[2].descsz = sizeof (IMG_ID);

  /* Note 3: Linktime.  */
  notes[3].type = NT_VMS_LINKTIME;
  notes[3].descdata = (char *)cur_time;
  notes[3].descsz = sizeof (cur_time);

  /* Note 4: Linker id.  */
  notes[4].type = NT_VMS_LINKID;
  notes[4].descdata = "GNU ld " BFD_VERSION_STRING;
  notes[4].descsz = strlen (notes[4].descdata) + 1;

  /* Note 5: Original dyn.  */
  orig_dyn_size = (sizeof (*orig_dyn) + sizeof (IMG_ID) - 1 + 7) & ~7;
  orig_dyn = bfd_zalloc (abfd, orig_dyn_size);
  if (orig_dyn == NULL)
    return false;
  bfd_putl32 (1, orig_dyn->major_id);
  bfd_putl32 (3, orig_dyn->minor_id);
  memcpy (orig_dyn->manipulation_date, cur_time, sizeof (cur_time));
  bfd_putl64 (VMS_LF_IMGSTA | VMS_LF_MAIN, orig_dyn->link_flags);
  bfd_putl32 (EF_IA_64_ABI64, orig_dyn->elf_flags);
  memcpy (orig_dyn->imgid, IMG_ID, sizeof (IMG_ID));
  notes[5].type = NT_VMS_ORIG_DYN;
  notes[5].descdata = (char *)orig_dyn;
  notes[5].descsz = orig_dyn_size;

  /* Note 3: Patchtime.  */
  notes[6].type = NT_VMS_PATCHTIME;
  notes[6].descdata = (char *)cur_time;
  notes[6].descsz = sizeof (cur_time);

  /* Compute notes size.  */
  note_size = 0;
  for (i = 0; i < NBR_NOTES; i++)
    note_size += sizeof (Elf64_External_VMS_Note) - 1
      + ((sizeof (NOTE_NAME) - 1 + 7) & ~7)
      + ((notes[i].descsz + 7) & ~7);

  /* Malloc a temporary buffer large enough for most notes */
  note_contents = (unsigned char *) bfd_zalloc (abfd, note_size);
  if (note_contents == NULL)
    return false;
  noteptr = note_contents;

  /* Fill notes.  */
  for (i = 0; i < NBR_NOTES; i++)
    {
      Elf64_External_VMS_Note *enote = (Elf64_External_VMS_Note *) noteptr;

      bfd_putl64 (sizeof (NOTE_NAME) - 1, enote->namesz);
      bfd_putl64 (notes[i].descsz, enote->descsz);
      bfd_putl64 (notes[i].type, enote->type);

      noteptr = (unsigned char *)enote->name;
      memcpy (noteptr, NOTE_NAME, sizeof (NOTE_NAME) - 1);
      noteptr += (sizeof (NOTE_NAME) - 1 + 7) & ~7;
      memcpy (noteptr, notes[i].descdata, notes[i].descsz);
      noteptr += (notes[i].descsz + 7) & ~7;
    }

  ia64_info->note_sec->contents = note_contents;
  ia64_info->note_sec->size = note_size;

  free (module_name);

  return true;
}

static bool
elf64_ia64_create_dynamic_sections (bfd *abfd,
				    struct bfd_link_info *info)
{
  struct elf64_ia64_link_hash_table *ia64_info;
  asection *s;
  flagword flags;
  const struct elf_backend_data *bed;

  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return false;

  if (elf_hash_table (info)->dynamic_sections_created)
    return true;

  abfd = elf_hash_table (info)->dynobj;
  bed = get_elf_backend_data (abfd);

  flags = bed->dynamic_sec_flags;

  s = bfd_make_section_anyway_with_flags (abfd, ".dynamic",
					  flags | SEC_READONLY);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->s->log_file_align))
    return false;

  s = bfd_make_section_anyway_with_flags (abfd, ".plt", flags | SEC_READONLY);
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->plt_alignment))
    return false;
  ia64_info->root.splt = s;

  if (!get_got (abfd, ia64_info))
    return false;

  if (!get_pltoff (abfd, ia64_info))
    return false;

  s = bfd_make_section_anyway_with_flags (abfd, ".vmsdynstr",
					  (SEC_ALLOC
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_LINKER_CREATED));
  if (s == NULL
      || !bfd_set_section_alignment (s, 0))
    return false;

  /* Create a fixup section.  */
  s = bfd_make_section_anyway_with_flags (abfd, ".fixups",
					  (SEC_ALLOC
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_LINKER_CREATED));
  if (s == NULL
      || !bfd_set_section_alignment (s, 3))
    return false;
  ia64_info->fixups_sec = s;

  /* Create the transfer fixup section.  */
  s = bfd_make_section_anyway_with_flags (abfd, ".transfer",
					  (SEC_ALLOC
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_LINKER_CREATED));
  if (s == NULL
      || !bfd_set_section_alignment (s, 3))
    return false;
  s->size = sizeof (struct elf64_vms_transfer);
  ia64_info->transfer_sec = s;

  /* Create note section.  */
  s = bfd_make_section_anyway_with_flags (abfd, ".vms.note",
					  (SEC_LINKER_CREATED
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_READONLY));
  if (s == NULL
      || !bfd_set_section_alignment (s, 3))
    return false;
  ia64_info->note_sec = s;

  elf_hash_table (info)->dynamic_sections_created = true;
  return true;
}

/* Find and/or create a hash entry for local symbol.  */
static struct elf64_ia64_local_hash_entry *
get_local_sym_hash (struct elf64_ia64_link_hash_table *ia64_info,
		    bfd *abfd, const Elf_Internal_Rela *rel,
		    bool create)
{
  struct elf64_ia64_local_hash_entry e, *ret;
  asection *sec = abfd->sections;
  hashval_t h = ELF_LOCAL_SYMBOL_HASH (sec->id,
				       ELF64_R_SYM (rel->r_info));
  void **slot;

  e.id = sec->id;
  e.r_sym = ELF64_R_SYM (rel->r_info);
  slot = htab_find_slot_with_hash (ia64_info->loc_hash_table, &e, h,
				   create ? INSERT : NO_INSERT);

  if (!slot)
    return NULL;

  if (*slot)
    return (struct elf64_ia64_local_hash_entry *) *slot;

  ret = (struct elf64_ia64_local_hash_entry *)
	objalloc_alloc ((struct objalloc *) ia64_info->loc_hash_memory,
			sizeof (struct elf64_ia64_local_hash_entry));
  if (ret)
    {
      memset (ret, 0, sizeof (*ret));
      ret->id = sec->id;
      ret->r_sym = ELF64_R_SYM (rel->r_info);
      *slot = ret;
    }
  return ret;
}

/* Used to sort elf64_ia64_dyn_sym_info array.  */

static int
addend_compare (const void *xp, const void *yp)
{
  const struct elf64_ia64_dyn_sym_info *x
    = (const struct elf64_ia64_dyn_sym_info *) xp;
  const struct elf64_ia64_dyn_sym_info *y
    = (const struct elf64_ia64_dyn_sym_info *) yp;

  return x->addend < y->addend ? -1 : x->addend > y->addend ? 1 : 0;
}

/* Sort elf64_ia64_dyn_sym_info array and remove duplicates.  */

static unsigned int
sort_dyn_sym_info (struct elf64_ia64_dyn_sym_info *info,
		   unsigned int count)
{
  bfd_vma curr, prev, got_offset;
  unsigned int i, kept, dupes, diff, dest, src, len;

  qsort (info, count, sizeof (*info), addend_compare);

  /* Find the first duplicate.  */
  prev = info [0].addend;
  got_offset = info [0].got_offset;
  for (i = 1; i < count; i++)
    {
      curr = info [i].addend;
      if (curr == prev)
	{
	  /* For duplicates, make sure that GOT_OFFSET is valid.  */
	  if (got_offset == (bfd_vma) -1)
	    got_offset = info [i].got_offset;
	  break;
	}
      got_offset = info [i].got_offset;
      prev = curr;
    }

  /* We may move a block of elements to here.  */
  dest = i++;

  /* Remove duplicates.  */
  if (i < count)
    {
      while (i < count)
	{
	  /* For duplicates, make sure that the kept one has a valid
	     got_offset.  */
	  kept = dest - 1;
	  if (got_offset != (bfd_vma) -1)
	    info [kept].got_offset = got_offset;

	  curr = info [i].addend;
	  got_offset = info [i].got_offset;

	  /* Move a block of elements whose first one is different from
	     the previous.  */
	  if (curr == prev)
	    {
	      for (src = i + 1; src < count; src++)
		{
		  if (info [src].addend != curr)
		    break;
		  /* For duplicates, make sure that GOT_OFFSET is
		     valid.  */
		  if (got_offset == (bfd_vma) -1)
		    got_offset = info [src].got_offset;
		}

	      /* Make sure that the kept one has a valid got_offset.  */
	      if (got_offset != (bfd_vma) -1)
		info [kept].got_offset = got_offset;
	    }
	  else
	    src = i;

	  if (src >= count)
	    break;

	  /* Find the next duplicate.  SRC will be kept.  */
	  prev = info [src].addend;
	  got_offset = info [src].got_offset;
	  for (dupes = src + 1; dupes < count; dupes ++)
	    {
	      curr = info [dupes].addend;
	      if (curr == prev)
		{
		  /* Make sure that got_offset is valid.  */
		  if (got_offset == (bfd_vma) -1)
		    got_offset = info [dupes].got_offset;

		  /* For duplicates, make sure that the kept one has
		     a valid got_offset.  */
		  if (got_offset != (bfd_vma) -1)
		    info [dupes - 1].got_offset = got_offset;
		  break;
		}
	      got_offset = info [dupes].got_offset;
	      prev = curr;
	    }

	  /* How much to move.  */
	  len = dupes - src;
	  i = dupes + 1;

	  if (len == 1 && dupes < count)
	    {
	      /* If we only move 1 element, we combine it with the next
		 one.  There must be at least a duplicate.  Find the
		 next different one.  */
	      for (diff = dupes + 1, src++; diff < count; diff++, src++)
		{
		  if (info [diff].addend != curr)
		    break;
		  /* Make sure that got_offset is valid.  */
		  if (got_offset == (bfd_vma) -1)
		    got_offset = info [diff].got_offset;
		}

	      /* Makre sure that the last duplicated one has an valid
		 offset.  */
	      BFD_ASSERT (curr == prev);
	      if (got_offset != (bfd_vma) -1)
		info [diff - 1].got_offset = got_offset;

	      if (diff < count)
		{
		  /* Find the next duplicate.  Track the current valid
		     offset.  */
		  prev = info [diff].addend;
		  got_offset = info [diff].got_offset;
		  for (dupes = diff + 1; dupes < count; dupes ++)
		    {
		      curr = info [dupes].addend;
		      if (curr == prev)
			{
			  /* For duplicates, make sure that GOT_OFFSET
			     is valid.  */
			  if (got_offset == (bfd_vma) -1)
			    got_offset = info [dupes].got_offset;
			  break;
			}
		      got_offset = info [dupes].got_offset;
		      prev = curr;
		      diff++;
		    }

		  len = diff - src + 1;
		  i = diff + 1;
		}
	    }

	  memmove (&info [dest], &info [src], len * sizeof (*info));

	  dest += len;
	}

      count = dest;
    }
  else
    {
      /* When we get here, either there is no duplicate at all or
	 the only duplicate is the last element.  */
      if (dest < count)
	{
	  /* If the last element is a duplicate, make sure that the
	     kept one has a valid got_offset.  We also update count.  */
	  if (got_offset != (bfd_vma) -1)
	    info [dest - 1].got_offset = got_offset;
	  count = dest;
	}
    }

  return count;
}

/* Find and/or create a descriptor for dynamic symbol info.  This will
   vary based on global or local symbol, and the addend to the reloc.

   We don't sort when inserting.  Also, we sort and eliminate
   duplicates if there is an unsorted section.  Typically, this will
   only happen once, because we do all insertions before lookups.  We
   then use bsearch to do a lookup.  This also allows lookups to be
   fast.  So we have fast insertion (O(log N) due to duplicate check),
   fast lookup (O(log N)) and one sort (O(N log N) expected time).
   Previously, all lookups were O(N) because of the use of the linked
   list and also all insertions were O(N) because of the check for
   duplicates.  There are some complications here because the array
   size grows occasionally, which may add an O(N) factor, but this
   should be rare.  Also,  we free the excess array allocation, which
   requires a copy which is O(N), but this only happens once.  */

static struct elf64_ia64_dyn_sym_info *
get_dyn_sym_info (struct elf64_ia64_link_hash_table *ia64_info,
		  struct elf_link_hash_entry *h, bfd *abfd,
		  const Elf_Internal_Rela *rel, bool create)
{
  struct elf64_ia64_dyn_sym_info **info_p, *info, *dyn_i, key;
  unsigned int *count_p, *sorted_count_p, *size_p;
  unsigned int count, sorted_count, size;
  bfd_vma addend = rel ? rel->r_addend : 0;
  bfd_size_type amt;

  if (h)
    {
      struct elf64_ia64_link_hash_entry *global_h;

      global_h = (struct elf64_ia64_link_hash_entry *) h;
      info_p = &global_h->info;
      count_p = &global_h->count;
      sorted_count_p = &global_h->sorted_count;
      size_p = &global_h->size;
    }
  else
    {
      struct elf64_ia64_local_hash_entry *loc_h;

      loc_h = get_local_sym_hash (ia64_info, abfd, rel, create);
      if (!loc_h)
	{
	  BFD_ASSERT (!create);
	  return NULL;
	}

      info_p = &loc_h->info;
      count_p = &loc_h->count;
      sorted_count_p = &loc_h->sorted_count;
      size_p = &loc_h->size;
    }

  count = *count_p;
  sorted_count = *sorted_count_p;
  size = *size_p;
  info = *info_p;
  if (create)
    {
      /* When we create the array, we don't check for duplicates,
	 except in the previously sorted section if one exists, and
	 against the last inserted entry.  This allows insertions to
	 be fast.  */
      if (info)
	{
	  if (sorted_count)
	    {
	      /* Try bsearch first on the sorted section.  */
	      key.addend = addend;
	      dyn_i = bsearch (&key, info, sorted_count,
			       sizeof (*info), addend_compare);

	      if (dyn_i)
		{
		  return dyn_i;
		}
	    }

	  /* Do a quick check for the last inserted entry.  */
	  dyn_i = info + count - 1;
	  if (dyn_i->addend == addend)
	    {
	      return dyn_i;
	    }
	}

      if (size == 0)
	{
	  /* It is the very first element. We create the array of size
	     1.  */
	  size = 1;
	  amt = size * sizeof (*info);
	  info = bfd_malloc (amt);
	}
      else if (size <= count)
	{
	  /* We double the array size every time when we reach the
	     size limit.  */
	  size += size;
	  amt = size * sizeof (*info);
	  info = bfd_realloc (info, amt);
	}
      else
	goto has_space;

      if (info == NULL)
	return NULL;
      *size_p = size;
      *info_p = info;

    has_space:
      /* Append the new one to the array.  */
      dyn_i = info + count;
      memset (dyn_i, 0, sizeof (*dyn_i));
      dyn_i->got_offset = (bfd_vma) -1;
      dyn_i->addend = addend;

      /* We increment count only since the new ones are unsorted and
	 may have duplicate.  */
      (*count_p)++;
    }
  else
    {
      /* It is a lookup without insertion.  Sort array if part of the
	 array isn't sorted.  */
      if (count != sorted_count)
	{
	  count = sort_dyn_sym_info (info, count);
	  *count_p = count;
	  *sorted_count_p = count;
	}

      /* Free unused memory.  */
      if (size != count)
	{
	  amt = count * sizeof (*info);
	  info = bfd_malloc (amt);
	  if (info != NULL)
	    {
	      memcpy (info, *info_p, amt);
	      free (*info_p);
	      *size_p = count;
	      *info_p = info;
	    }
	}

      key.addend = addend;
      dyn_i = bsearch (&key, info, count,
		       sizeof (*info), addend_compare);
    }

  return dyn_i;
}

static asection *
get_got (bfd *abfd, struct elf64_ia64_link_hash_table *ia64_info)
{
  asection *got;
  bfd *dynobj;

  got = ia64_info->root.sgot;
  if (!got)
    {
      flagword flags;

      dynobj = ia64_info->root.dynobj;
      if (!dynobj)
	ia64_info->root.dynobj = dynobj = abfd;

      /* The .got section is always aligned at 8 bytes.  */
      flags = get_elf_backend_data (dynobj)->dynamic_sec_flags;
      got = bfd_make_section_anyway_with_flags (dynobj, ".got",
						flags | SEC_SMALL_DATA);
      if (got == NULL
	  || !bfd_set_section_alignment (got, 3))
	return NULL;
      ia64_info->root.sgot = got;
    }

  return got;
}

/* Create function descriptor section (.opd).  This section is called .opd
   because it contains "official procedure descriptors".  The "official"
   refers to the fact that these descriptors are used when taking the address
   of a procedure, thus ensuring a unique address for each procedure.  */

static asection *
get_fptr (bfd *abfd, struct bfd_link_info *info,
	  struct elf64_ia64_link_hash_table *ia64_info)
{
  asection *fptr;
  bfd *dynobj;

  fptr = ia64_info->fptr_sec;
  if (!fptr)
    {
      dynobj = ia64_info->root.dynobj;
      if (!dynobj)
	ia64_info->root.dynobj = dynobj = abfd;

      fptr = bfd_make_section_anyway_with_flags (dynobj, ".opd",
						 (SEC_ALLOC
						  | SEC_LOAD
						  | SEC_HAS_CONTENTS
						  | SEC_IN_MEMORY
						  | (bfd_link_pie (info) ? 0
						     : SEC_READONLY)
						  | SEC_LINKER_CREATED));
      if (!fptr
	  || !bfd_set_section_alignment (fptr, 4))
	{
	  BFD_ASSERT (0);
	  return NULL;
	}

      ia64_info->fptr_sec = fptr;

      if (bfd_link_pie (info))
	{
	  asection *fptr_rel;
	  fptr_rel = bfd_make_section_anyway_with_flags (dynobj, ".rela.opd",
							 (SEC_ALLOC | SEC_LOAD
							  | SEC_HAS_CONTENTS
							  | SEC_IN_MEMORY
							  | SEC_LINKER_CREATED
							  | SEC_READONLY));
	  if (fptr_rel == NULL
	      || !bfd_set_section_alignment (fptr_rel, 3))
	    {
	      BFD_ASSERT (0);
	      return NULL;
	    }

	  ia64_info->rel_fptr_sec = fptr_rel;
	}
    }

  return fptr;
}

static asection *
get_pltoff (bfd *abfd, struct elf64_ia64_link_hash_table *ia64_info)
{
  asection *pltoff;
  bfd *dynobj;

  pltoff = ia64_info->pltoff_sec;
  if (!pltoff)
    {
      dynobj = ia64_info->root.dynobj;
      if (!dynobj)
	ia64_info->root.dynobj = dynobj = abfd;

      pltoff = bfd_make_section_anyway_with_flags (dynobj,
						   ELF_STRING_ia64_pltoff,
						   (SEC_ALLOC
						    | SEC_LOAD
						    | SEC_HAS_CONTENTS
						    | SEC_IN_MEMORY
						    | SEC_SMALL_DATA
						    | SEC_LINKER_CREATED));
      if (!pltoff
	  || !bfd_set_section_alignment (pltoff, 4))
	{
	  BFD_ASSERT (0);
	  return NULL;
	}

      ia64_info->pltoff_sec = pltoff;
    }

  return pltoff;
}

static asection *
get_reloc_section (bfd *abfd,
		   struct elf64_ia64_link_hash_table *ia64_info,
		   asection *sec, bool create)
{
  const char *srel_name;
  asection *srel;
  bfd *dynobj;

  srel_name = (bfd_elf_string_from_elf_section
	       (abfd, elf_elfheader(abfd)->e_shstrndx,
		_bfd_elf_single_rel_hdr (sec)->sh_name));
  if (srel_name == NULL)
    return NULL;

  BFD_ASSERT ((startswith (srel_name, ".rela")
	       && strcmp (bfd_section_name (sec), srel_name+5) == 0)
	      || (startswith (srel_name, ".rel")
		  && strcmp (bfd_section_name (sec), srel_name+4) == 0));

  dynobj = ia64_info->root.dynobj;
  if (!dynobj)
    ia64_info->root.dynobj = dynobj = abfd;

  srel = bfd_get_linker_section (dynobj, srel_name);
  if (srel == NULL && create)
    {
      srel = bfd_make_section_anyway_with_flags (dynobj, srel_name,
						 (SEC_ALLOC | SEC_LOAD
						  | SEC_HAS_CONTENTS
						  | SEC_IN_MEMORY
						  | SEC_LINKER_CREATED
						  | SEC_READONLY));
      if (srel == NULL
	  || !bfd_set_section_alignment (srel, 3))
	return NULL;
    }

  return srel;
}

static bool
count_dyn_reloc (bfd *abfd, struct elf64_ia64_dyn_sym_info *dyn_i,
		 asection *srel, int type)
{
  struct elf64_ia64_dyn_reloc_entry *rent;

  for (rent = dyn_i->reloc_entries; rent; rent = rent->next)
    if (rent->srel == srel && rent->type == type)
      break;

  if (!rent)
    {
      rent = ((struct elf64_ia64_dyn_reloc_entry *)
	      bfd_alloc (abfd, (bfd_size_type) sizeof (*rent)));
      if (!rent)
	return false;

      rent->next = dyn_i->reloc_entries;
      rent->srel = srel;
      rent->type = type;
      rent->count = 0;
      dyn_i->reloc_entries = rent;
    }
  rent->count++;

  return true;
}

static bool
elf64_ia64_check_relocs (bfd *abfd, struct bfd_link_info *info,
			 asection *sec,
			 const Elf_Internal_Rela *relocs)
{
  struct elf64_ia64_link_hash_table *ia64_info;
  const Elf_Internal_Rela *relend;
  Elf_Internal_Shdr *symtab_hdr;
  const Elf_Internal_Rela *rel;
  asection *got, *fptr, *srel, *pltoff;
  enum {
    NEED_GOT = 1,
    NEED_GOTX = 2,
    NEED_FPTR = 4,
    NEED_PLTOFF = 8,
    NEED_MIN_PLT = 16,
    NEED_FULL_PLT = 32,
    NEED_DYNREL = 64,
    NEED_LTOFF_FPTR = 128
  };
  int need_entry;
  struct elf_link_hash_entry *h;
  unsigned long r_symndx;
  bool maybe_dynamic;

  if (bfd_link_relocatable (info))
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return false;

  got = fptr = srel = pltoff = NULL;

  relend = relocs + sec->reloc_count;

  /* We scan relocations first to create dynamic relocation arrays.  We
     modified get_dyn_sym_info to allow fast insertion and support fast
     lookup in the next loop.  */
  for (rel = relocs; rel < relend; ++rel)
    {
      r_symndx = ELF64_R_SYM (rel->r_info);
      if (r_symndx >= symtab_hdr->sh_info)
	{
	  long indx = r_symndx - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}
      else
	h = NULL;

      /* We can only get preliminary data on whether a symbol is
	 locally or externally defined, as not all of the input files
	 have yet been processed.  Do something with what we know, as
	 this may help reduce memory usage and processing time later.  */
      maybe_dynamic = (h && ((!bfd_link_executable (info)
			      && (!SYMBOLIC_BIND (info, h)
				  || info->unresolved_syms_in_shared_libs == RM_IGNORE))
			     || !h->def_regular
			     || h->root.type == bfd_link_hash_defweak));

      need_entry = 0;
      switch (ELF64_R_TYPE (rel->r_info))
	{
	case R_IA64_TPREL64MSB:
	case R_IA64_TPREL64LSB:
	case R_IA64_LTOFF_TPREL22:
	case R_IA64_DTPREL32MSB:
	case R_IA64_DTPREL32LSB:
	case R_IA64_DTPREL64MSB:
	case R_IA64_DTPREL64LSB:
	case R_IA64_LTOFF_DTPREL22:
	case R_IA64_DTPMOD64MSB:
	case R_IA64_DTPMOD64LSB:
	case R_IA64_LTOFF_DTPMOD22:
	  abort ();
	  break;

	case R_IA64_IPLTMSB:
	case R_IA64_IPLTLSB:
	  break;

	case R_IA64_LTOFF_FPTR22:
	case R_IA64_LTOFF_FPTR64I:
	case R_IA64_LTOFF_FPTR32MSB:
	case R_IA64_LTOFF_FPTR32LSB:
	case R_IA64_LTOFF_FPTR64MSB:
	case R_IA64_LTOFF_FPTR64LSB:
	  need_entry = NEED_FPTR | NEED_GOT | NEED_LTOFF_FPTR;
	  break;

	case R_IA64_FPTR64I:
	case R_IA64_FPTR32MSB:
	case R_IA64_FPTR32LSB:
	case R_IA64_FPTR64MSB:
	case R_IA64_FPTR64LSB:
	  if (bfd_link_pic (info) || h)
	    need_entry = NEED_FPTR | NEED_DYNREL;
	  else
	    need_entry = NEED_FPTR;
	  break;

	case R_IA64_LTOFF22:
	case R_IA64_LTOFF64I:
	  need_entry = NEED_GOT;
	  break;

	case R_IA64_LTOFF22X:
	  need_entry = NEED_GOTX;
	  break;

	case R_IA64_PLTOFF22:
	case R_IA64_PLTOFF64I:
	case R_IA64_PLTOFF64MSB:
	case R_IA64_PLTOFF64LSB:
	  need_entry = NEED_PLTOFF;
	  if (h)
	    {
	      if (maybe_dynamic)
		need_entry |= NEED_MIN_PLT;
	    }
	  else
	    {
	      (*info->callbacks->warning)
		(info, _("@pltoff reloc against local symbol"), 0,
		 abfd, 0, (bfd_vma) 0);
	    }
	  break;

	case R_IA64_PCREL21B:
	case R_IA64_PCREL60B:
	  /* Depending on where this symbol is defined, we may or may not
	     need a full plt entry.  Only skip if we know we'll not need
	     the entry -- static or symbolic, and the symbol definition
	     has already been seen.  */
	  if (maybe_dynamic && rel->r_addend == 0)
	    need_entry = NEED_FULL_PLT;
	  break;

	case R_IA64_IMM14:
	case R_IA64_IMM22:
	case R_IA64_IMM64:
	case R_IA64_DIR32MSB:
	case R_IA64_DIR32LSB:
	case R_IA64_DIR64MSB:
	case R_IA64_DIR64LSB:
	  /* Shared objects will always need at least a REL relocation.  */
	  if (bfd_link_pic (info) || maybe_dynamic)
	    need_entry = NEED_DYNREL;
	  break;

	case R_IA64_PCREL22:
	case R_IA64_PCREL64I:
	case R_IA64_PCREL32MSB:
	case R_IA64_PCREL32LSB:
	case R_IA64_PCREL64MSB:
	case R_IA64_PCREL64LSB:
	  if (maybe_dynamic)
	    need_entry = NEED_DYNREL;
	  break;
	}

      if (!need_entry)
	continue;

      if ((need_entry & NEED_FPTR) != 0
	  && rel->r_addend)
	{
	  (*info->callbacks->warning)
	    (info, _("non-zero addend in @fptr reloc"), 0,
	     abfd, 0, (bfd_vma) 0);
	}

      if (get_dyn_sym_info (ia64_info, h, abfd, rel, true) == NULL)
	return false;
    }

  /* Now, we only do lookup without insertion, which is very fast
     with the modified get_dyn_sym_info.  */
  for (rel = relocs; rel < relend; ++rel)
    {
      struct elf64_ia64_dyn_sym_info *dyn_i;
      int dynrel_type = R_IA64_NONE;

      r_symndx = ELF64_R_SYM (rel->r_info);
      if (r_symndx >= symtab_hdr->sh_info)
	{
	  /* We're dealing with a global symbol -- find its hash entry
	     and mark it as being referenced.  */
	  long indx = r_symndx - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  /* PR15323, ref flags aren't set for references in the same
	     object.  */
	  h->ref_regular = 1;
	}
      else
	h = NULL;

      /* We can only get preliminary data on whether a symbol is
	 locally or externally defined, as not all of the input files
	 have yet been processed.  Do something with what we know, as
	 this may help reduce memory usage and processing time later.  */
      maybe_dynamic = (h && ((!bfd_link_executable (info)
			      && (!SYMBOLIC_BIND (info, h)
				  || info->unresolved_syms_in_shared_libs == RM_IGNORE))
			     || !h->def_regular
			     || h->root.type == bfd_link_hash_defweak));

      need_entry = 0;
      switch (ELF64_R_TYPE (rel->r_info))
	{
	case R_IA64_TPREL64MSB:
	case R_IA64_TPREL64LSB:
	case R_IA64_LTOFF_TPREL22:
	case R_IA64_DTPREL32MSB:
	case R_IA64_DTPREL32LSB:
	case R_IA64_DTPREL64MSB:
	case R_IA64_DTPREL64LSB:
	case R_IA64_LTOFF_DTPREL22:
	case R_IA64_DTPMOD64MSB:
	case R_IA64_DTPMOD64LSB:
	case R_IA64_LTOFF_DTPMOD22:
	  abort ();
	  break;

	case R_IA64_LTOFF_FPTR22:
	case R_IA64_LTOFF_FPTR64I:
	case R_IA64_LTOFF_FPTR32MSB:
	case R_IA64_LTOFF_FPTR32LSB:
	case R_IA64_LTOFF_FPTR64MSB:
	case R_IA64_LTOFF_FPTR64LSB:
	  need_entry = NEED_FPTR | NEED_GOT | NEED_LTOFF_FPTR;
	  break;

	case R_IA64_FPTR64I:
	case R_IA64_FPTR32MSB:
	case R_IA64_FPTR32LSB:
	case R_IA64_FPTR64MSB:
	case R_IA64_FPTR64LSB:
	  if (bfd_link_pic (info) || h)
	    need_entry = NEED_FPTR | NEED_DYNREL;
	  else
	    need_entry = NEED_FPTR;
	  dynrel_type = R_IA64_FPTR64LSB;
	  break;

	case R_IA64_LTOFF22:
	case R_IA64_LTOFF64I:
	  need_entry = NEED_GOT;
	  break;

	case R_IA64_LTOFF22X:
	  need_entry = NEED_GOTX;
	  break;

	case R_IA64_PLTOFF22:
	case R_IA64_PLTOFF64I:
	case R_IA64_PLTOFF64MSB:
	case R_IA64_PLTOFF64LSB:
	  need_entry = NEED_PLTOFF;
	  if (h)
	    {
	      if (maybe_dynamic)
		need_entry |= NEED_MIN_PLT;
	    }
	  break;

	case R_IA64_PCREL21B:
	case R_IA64_PCREL60B:
	  /* Depending on where this symbol is defined, we may or may not
	     need a full plt entry.  Only skip if we know we'll not need
	     the entry -- static or symbolic, and the symbol definition
	     has already been seen.  */
	  if (maybe_dynamic && rel->r_addend == 0)
	    need_entry = NEED_FULL_PLT;
	  break;

	case R_IA64_IMM14:
	case R_IA64_IMM22:
	case R_IA64_IMM64:
	case R_IA64_DIR32MSB:
	case R_IA64_DIR32LSB:
	case R_IA64_DIR64MSB:
	case R_IA64_DIR64LSB:
	  /* Shared objects will always need at least a REL relocation.  */
	  if (bfd_link_pic (info) || maybe_dynamic)
	    need_entry = NEED_DYNREL;
	  dynrel_type = R_IA64_DIR64LSB;
	  break;

	case R_IA64_IPLTMSB:
	case R_IA64_IPLTLSB:
	  break;

	case R_IA64_PCREL22:
	case R_IA64_PCREL64I:
	case R_IA64_PCREL32MSB:
	case R_IA64_PCREL32LSB:
	case R_IA64_PCREL64MSB:
	case R_IA64_PCREL64LSB:
	  if (maybe_dynamic)
	    need_entry = NEED_DYNREL;
	  dynrel_type = R_IA64_PCREL64LSB;
	  break;
	}

      if (!need_entry)
	continue;

      dyn_i = get_dyn_sym_info (ia64_info, h, abfd, rel, false);

      /* Record whether or not this is a local symbol.  */
      dyn_i->h = h;

      /* Create what's needed.  */
      if (need_entry & (NEED_GOT | NEED_GOTX))
	{
	  if (!got)
	    {
	      got = get_got (abfd, ia64_info);
	      if (!got)
		return false;
	    }
	  if (need_entry & NEED_GOT)
	    dyn_i->want_got = 1;
	  if (need_entry & NEED_GOTX)
	    dyn_i->want_gotx = 1;
	}
      if (need_entry & NEED_FPTR)
	{
	  /* Create the .opd section.  */
	  if (!fptr)
	    {
	      fptr = get_fptr (abfd, info, ia64_info);
	      if (!fptr)
		return false;
	    }
	  dyn_i->want_fptr = 1;
	}
      if (need_entry & NEED_LTOFF_FPTR)
	dyn_i->want_ltoff_fptr = 1;
      if (need_entry & (NEED_MIN_PLT | NEED_FULL_PLT))
	{
	  if (!ia64_info->root.dynobj)
	    ia64_info->root.dynobj = abfd;
	  h->needs_plt = 1;
	  dyn_i->want_plt = 1;
	}
      if (need_entry & NEED_FULL_PLT)
	dyn_i->want_plt2 = 1;
      if (need_entry & NEED_PLTOFF)
	{
	  /* This is needed here, in case @pltoff is used in a non-shared
	     link.  */
	  if (!pltoff)
	    {
	      pltoff = get_pltoff (abfd, ia64_info);
	      if (!pltoff)
		return false;
	    }

	  dyn_i->want_pltoff = 1;
	}
      if ((need_entry & NEED_DYNREL) && (sec->flags & SEC_ALLOC))
	{
	  if (!srel)
	    {
	      srel = get_reloc_section (abfd, ia64_info, sec, true);
	      if (!srel)
		return false;
	    }
	  if (!count_dyn_reloc (abfd, dyn_i, srel, dynrel_type))
	    return false;
	}
    }

  return true;
}

/* For cleanliness, and potentially faster dynamic loading, allocate
   external GOT entries first.  */

static bool
allocate_global_data_got (struct elf64_ia64_dyn_sym_info *dyn_i,
			  void * data)
{
  struct elf64_ia64_allocate_data *x = (struct elf64_ia64_allocate_data *)data;

  if ((dyn_i->want_got || dyn_i->want_gotx)
      && ! dyn_i->want_fptr
      && elf64_ia64_dynamic_symbol_p (dyn_i->h))
     {
       /* GOT entry with FPTR is done by allocate_global_fptr_got.  */
       dyn_i->got_offset = x->ofs;
       x->ofs += 8;
     }
  return true;
}

/* Next, allocate all the GOT entries used by LTOFF_FPTR relocs.  */

static bool
allocate_global_fptr_got (struct elf64_ia64_dyn_sym_info *dyn_i,
			  void * data)
{
  struct elf64_ia64_allocate_data *x = (struct elf64_ia64_allocate_data *)data;

  if (dyn_i->want_got
      && dyn_i->want_fptr
      && elf64_ia64_dynamic_symbol_p (dyn_i->h))
    {
      dyn_i->got_offset = x->ofs;
      x->ofs += 8;
    }
  return true;
}

/* Lastly, allocate all the GOT entries for local data.  */

static bool
allocate_local_got (struct elf64_ia64_dyn_sym_info *dyn_i,
		    void * data)
{
  struct elf64_ia64_allocate_data *x = (struct elf64_ia64_allocate_data *) data;

  if ((dyn_i->want_got || dyn_i->want_gotx)
      && !elf64_ia64_dynamic_symbol_p (dyn_i->h))
    {
      dyn_i->got_offset = x->ofs;
      x->ofs += 8;
    }
  return true;
}

/* Allocate function descriptors.  We can do these for every function
   in a main executable that is not exported.  */

static bool
allocate_fptr (struct elf64_ia64_dyn_sym_info *dyn_i, void * data)
{
  struct elf64_ia64_allocate_data *x = (struct elf64_ia64_allocate_data *) data;

  if (dyn_i->want_fptr)
    {
      struct elf_link_hash_entry *h = dyn_i->h;

      if (h)
	while (h->root.type == bfd_link_hash_indirect
	       || h->root.type == bfd_link_hash_warning)
	  h = (struct elf_link_hash_entry *) h->root.u.i.link;

      if (h == NULL || !h->def_dynamic)
	{
	  /*  A non dynamic symbol.  */
	  dyn_i->fptr_offset = x->ofs;
	  x->ofs += 16;
	}
      else
	dyn_i->want_fptr = 0;
    }
  return true;
}

/* Allocate all the minimal PLT entries.  */

static bool
allocate_plt_entries (struct elf64_ia64_dyn_sym_info *dyn_i,
		      void * data ATTRIBUTE_UNUSED)
{
  if (dyn_i->want_plt)
    {
      struct elf_link_hash_entry *h = dyn_i->h;

      if (h)
	while (h->root.type == bfd_link_hash_indirect
	       || h->root.type == bfd_link_hash_warning)
	  h = (struct elf_link_hash_entry *) h->root.u.i.link;

      /* ??? Versioned symbols seem to lose NEEDS_PLT.  */
      if (elf64_ia64_dynamic_symbol_p (h))
	{
	  dyn_i->want_pltoff = 1;
	}
      else
	{
	  dyn_i->want_plt = 0;
	  dyn_i->want_plt2 = 0;
	}
    }
  return true;
}

/* Allocate all the full PLT entries.  */

static bool
allocate_plt2_entries (struct elf64_ia64_dyn_sym_info *dyn_i,
		       void * data)
{
  struct elf64_ia64_allocate_data *x = (struct elf64_ia64_allocate_data *)data;

  if (dyn_i->want_plt2)
    {
      struct elf_link_hash_entry *h = dyn_i->h;
      bfd_size_type ofs = x->ofs;

      dyn_i->plt2_offset = ofs;
      x->ofs = ofs + PLT_FULL_ENTRY_SIZE;

      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;
      dyn_i->h->plt.offset = ofs;
    }
  return true;
}

/* Allocate all the PLTOFF entries requested by relocations and
   plt entries.  We can't share space with allocated FPTR entries,
   because the latter are not necessarily addressable by the GP.
   ??? Relaxation might be able to determine that they are.  */

static bool
allocate_pltoff_entries (struct elf64_ia64_dyn_sym_info *dyn_i,
			 void * data)
{
  struct elf64_ia64_allocate_data *x = (struct elf64_ia64_allocate_data *)data;

  if (dyn_i->want_pltoff)
    {
      dyn_i->pltoff_offset = x->ofs;
      x->ofs += 16;
    }
  return true;
}

/* Allocate dynamic relocations for those symbols that turned out
   to be dynamic.  */

static bool
allocate_dynrel_entries (struct elf64_ia64_dyn_sym_info *dyn_i,
			 void * data)
{
  struct elf64_ia64_allocate_data *x = (struct elf64_ia64_allocate_data *)data;
  struct elf64_ia64_link_hash_table *ia64_info;
  struct elf64_ia64_dyn_reloc_entry *rent;
  bool dynamic_symbol, shared, resolved_zero;
  struct elf64_ia64_link_hash_entry *h_ia64;

  ia64_info = elf64_ia64_hash_table (x->info);
  if (ia64_info == NULL)
    return false;

  /* Note that this can't be used in relation to FPTR relocs below.  */
  dynamic_symbol = elf64_ia64_dynamic_symbol_p (dyn_i->h);

  shared = bfd_link_pic (x->info);
  resolved_zero = (dyn_i->h
		   && ELF_ST_VISIBILITY (dyn_i->h->other)
		   && dyn_i->h->root.type == bfd_link_hash_undefweak);

  /* Take care of the GOT and PLT relocations.  */

  if ((!resolved_zero
       && (dynamic_symbol || shared)
       && (dyn_i->want_got || dyn_i->want_gotx))
      || (dyn_i->want_ltoff_fptr
	  && dyn_i->h
	  && dyn_i->h->def_dynamic))
    {
      /* VMS: FIX64.  */
      if (dyn_i->h != NULL && dyn_i->h->def_dynamic)
	{
	  h_ia64 = (struct elf64_ia64_link_hash_entry *) dyn_i->h;
	  elf_ia64_vms_tdata (h_ia64->shl)->fixups_off +=
	    sizeof (Elf64_External_VMS_IMAGE_FIXUP);
	  ia64_info->fixups_sec->size +=
	    sizeof (Elf64_External_VMS_IMAGE_FIXUP);
	}
    }

  if (ia64_info->rel_fptr_sec && dyn_i->want_fptr)
    {
      /* VMS: only image reloc.  */
      if (dyn_i->h == NULL || dyn_i->h->root.type != bfd_link_hash_undefweak)
	ia64_info->rel_fptr_sec->size += sizeof (Elf64_External_Rela);
    }

  if (!resolved_zero && dyn_i->want_pltoff)
    {
      /* VMS: FIXFD.  */
      if (dyn_i->h != NULL && dyn_i->h->def_dynamic)
	{
	  h_ia64 = (struct elf64_ia64_link_hash_entry *) dyn_i->h;
	  elf_ia64_vms_tdata (h_ia64->shl)->fixups_off +=
	    sizeof (Elf64_External_VMS_IMAGE_FIXUP);
	  ia64_info->fixups_sec->size +=
	    sizeof (Elf64_External_VMS_IMAGE_FIXUP);
	}
    }

  /* Take care of the normal data relocations.  */

  for (rent = dyn_i->reloc_entries; rent; rent = rent->next)
    {
      switch (rent->type)
	{
	case R_IA64_FPTR32LSB:
	case R_IA64_FPTR64LSB:
	  /* Allocate one iff !want_fptr and not PIE, which by this point
	     will be true only if we're actually allocating one statically
	     in the main executable.  Position independent executables
	     need a relative reloc.  */
	  if (dyn_i->want_fptr && !bfd_link_pie (x->info))
	    continue;
	  break;
	case R_IA64_PCREL32LSB:
	case R_IA64_PCREL64LSB:
	  if (!dynamic_symbol)
	    continue;
	  break;
	case R_IA64_DIR32LSB:
	case R_IA64_DIR64LSB:
	  if (!dynamic_symbol && !shared)
	    continue;
	  break;
	case R_IA64_IPLTLSB:
	  if (!dynamic_symbol && !shared)
	    continue;
	  break;
	case R_IA64_DTPREL32LSB:
	case R_IA64_TPREL64LSB:
	case R_IA64_DTPREL64LSB:
	case R_IA64_DTPMOD64LSB:
	  break;
	default:
	  abort ();
	}

      /* Add a fixup.  */
      if (!dynamic_symbol)
	abort ();

      h_ia64 = (struct elf64_ia64_link_hash_entry *) dyn_i->h;
      elf_ia64_vms_tdata (h_ia64->shl)->fixups_off +=
	sizeof (Elf64_External_VMS_IMAGE_FIXUP);
      ia64_info->fixups_sec->size +=
	sizeof (Elf64_External_VMS_IMAGE_FIXUP);
    }

  return true;
}

static bool
elf64_ia64_adjust_dynamic_symbol (struct bfd_link_info *info ATTRIBUTE_UNUSED,
				  struct elf_link_hash_entry *h)
{
  /* ??? Undefined symbols with PLT entries should be re-defined
     to be the PLT entry.  */

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

  /* If this is a reference to a symbol defined by a dynamic object which
     is not a function, we might allocate the symbol in our .dynbss section
     and allocate a COPY dynamic relocation.

     But IA-64 code is canonically PIC, so as a rule we can avoid this sort
     of hackery.  */

  return true;
}

static bool
elf64_ia64_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				  struct bfd_link_info *info)
{
  struct elf64_ia64_allocate_data data;
  struct elf64_ia64_link_hash_table *ia64_info;
  asection *sec;
  bfd *dynobj;
  struct elf_link_hash_table *hash_table;

  hash_table = elf_hash_table (info);
  dynobj = hash_table->dynobj;
  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return false;
  BFD_ASSERT(dynobj != NULL);
  data.info = info;

  /* Allocate the GOT entries.  */

  if (ia64_info->root.sgot)
    {
      data.ofs = 0;
      elf64_ia64_dyn_sym_traverse (ia64_info, allocate_global_data_got, &data);
      elf64_ia64_dyn_sym_traverse (ia64_info, allocate_global_fptr_got, &data);
      elf64_ia64_dyn_sym_traverse (ia64_info, allocate_local_got, &data);
      ia64_info->root.sgot->size = data.ofs;
    }

  /* Allocate the FPTR entries.  */

  if (ia64_info->fptr_sec)
    {
      data.ofs = 0;
      elf64_ia64_dyn_sym_traverse (ia64_info, allocate_fptr, &data);
      ia64_info->fptr_sec->size = data.ofs;
    }

  /* Now that we've seen all of the input files, we can decide which
     symbols need plt entries.  Allocate the minimal PLT entries first.
     We do this even though dynamic_sections_created may be FALSE, because
     this has the side-effect of clearing want_plt and want_plt2.  */

  data.ofs = 0;
  elf64_ia64_dyn_sym_traverse (ia64_info, allocate_plt_entries, &data);

  /* Align the pointer for the plt2 entries.  */
  data.ofs = (data.ofs + 31) & (bfd_vma) -32;

  elf64_ia64_dyn_sym_traverse (ia64_info, allocate_plt2_entries, &data);
  if (data.ofs != 0 || ia64_info->root.dynamic_sections_created)
    {
      /* FIXME: we always reserve the memory for dynamic linker even if
	 there are no PLT entries since dynamic linker may assume the
	 reserved memory always exists.  */

      BFD_ASSERT (ia64_info->root.dynamic_sections_created);

      ia64_info->root.splt->size = data.ofs;
    }

  /* Allocate the PLTOFF entries.  */

  if (ia64_info->pltoff_sec)
    {
      data.ofs = 0;
      elf64_ia64_dyn_sym_traverse (ia64_info, allocate_pltoff_entries, &data);
      ia64_info->pltoff_sec->size = data.ofs;
    }

  if (ia64_info->root.dynamic_sections_created)
    {
      /* Allocate space for the dynamic relocations that turned out to be
	 required.  */
      elf64_ia64_dyn_sym_traverse (ia64_info, allocate_dynrel_entries, &data);
    }

  /* We have now determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  for (sec = dynobj->sections; sec != NULL; sec = sec->next)
    {
      bool strip;

      if (!(sec->flags & SEC_LINKER_CREATED))
	continue;

      /* If we don't need this section, strip it from the output file.
	 There were several sections primarily related to dynamic
	 linking that must be create before the linker maps input
	 sections to output sections.  The linker does that before
	 bfd_elf_size_dynamic_sections is called, and it is that
	 function which decides whether anything needs to go into
	 these sections.  */

      strip = (sec->size == 0);

      if (sec == ia64_info->root.sgot)
	strip = false;
      else if (sec == ia64_info->root.srelgot)
	{
	  if (strip)
	    ia64_info->root.srelgot = NULL;
	  else
	    /* We use the reloc_count field as a counter if we need to
	       copy relocs into the output file.  */
	    sec->reloc_count = 0;
	}
      else if (sec == ia64_info->fptr_sec)
	{
	  if (strip)
	    ia64_info->fptr_sec = NULL;
	}
      else if (sec == ia64_info->rel_fptr_sec)
	{
	  if (strip)
	    ia64_info->rel_fptr_sec = NULL;
	  else
	    /* We use the reloc_count field as a counter if we need to
	       copy relocs into the output file.  */
	    sec->reloc_count = 0;
	}
      else if (sec == ia64_info->root.splt)
	{
	  if (strip)
	    ia64_info->root.splt = NULL;
	}
      else if (sec == ia64_info->pltoff_sec)
	{
	  if (strip)
	    ia64_info->pltoff_sec = NULL;
	}
      else if (sec == ia64_info->fixups_sec)
	{
	  if (strip)
	    ia64_info->fixups_sec = NULL;
	}
      else if (sec == ia64_info->transfer_sec)
	{
	  ;
	}
      else
	{
	  const char *name;

	  /* It's OK to base decisions on the section name, because none
	     of the dynobj section names depend upon the input files.  */
	  name = bfd_section_name (sec);

	  if (strcmp (name, ".got.plt") == 0)
	    strip = false;
	  else if (startswith (name, ".rel"))
	    {
	      if (!strip)
		{
		  /* We use the reloc_count field as a counter if we need to
		     copy relocs into the output file.  */
		  sec->reloc_count = 0;
		}
	    }
	  else
	    continue;
	}

      if (strip)
	sec->flags |= SEC_EXCLUDE;
      else
	{
	  /* Allocate memory for the section contents.  */
	  sec->contents = (bfd_byte *) bfd_zalloc (dynobj, sec->size);
	  if (sec->contents == NULL && sec->size != 0)
	    return false;
	}
    }

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      bfd *abfd;
      asection *dynsec;
      asection *dynstrsec;
      Elf_Internal_Dyn dyn;
      const struct elf_backend_data *bed;
      unsigned int shl_num = 0;
      bfd_vma fixups_off = 0;
      bfd_vma strdyn_off;
      unsigned int time_hi, time_lo;

      /* The .dynamic section must exist and be empty.  */
      dynsec = bfd_get_linker_section (hash_table->dynobj, ".dynamic");
      BFD_ASSERT (dynsec != NULL);
      BFD_ASSERT (dynsec->size == 0);

      dynstrsec = bfd_get_linker_section (hash_table->dynobj, ".vmsdynstr");
      BFD_ASSERT (dynstrsec != NULL);
      BFD_ASSERT (dynstrsec->size == 0);
      dynstrsec->size = 1;	/* Initial blank.  */

      /* Ident + link time.  */
      vms_get_time (&time_hi, &time_lo);

      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_IDENT, 0))
	return false;
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_LINKTIME,
				       ((uint64_t) time_hi << 32)
				       + time_lo))
	return false;

      /* Strtab.  */
      strdyn_off = dynsec->size;
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_STRTAB_OFFSET, 0))
	return false;
      if (!_bfd_elf_add_dynamic_entry (info, DT_STRSZ, 0))
	return false;

      /* PLTGOT  */
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_PLTGOT_SEG, 0))
	return false;
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_PLTGOT_OFFSET, 0))
	return false;

      /* Misc.  */
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_FPMODE, 0x9800000))
	return false;
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_LNKFLAGS,
				       VMS_LF_IMGSTA | VMS_LF_MAIN))
	return false;

      /* Add entries for shared libraries.  */
      for (abfd = info->input_bfds; abfd; abfd = abfd->link.next)
	{
	  char *soname;
	  size_t soname_len;
	  bfd_size_type strindex;
	  bfd_byte *newcontents;
	  bfd_vma fixups_shl_off;

	  if (!(abfd->flags & DYNAMIC))
	    continue;
	  BFD_ASSERT (abfd->xvec == output_bfd->xvec);

	  if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_NEEDED_IDENT,
					   elf_ia64_vms_ident (abfd)))
	    return false;

	  soname = vms_get_module_name (bfd_get_filename (abfd), true);
	  if (soname == NULL)
	    return false;
	  strindex = dynstrsec->size;
	  soname_len = strlen (soname) + 1;
	  newcontents = (bfd_byte *) bfd_realloc (dynstrsec->contents,
						  strindex + soname_len);
	  if (newcontents == NULL)
	    return false;
	  memcpy (newcontents + strindex, soname, soname_len);
	  dynstrsec->size += soname_len;
	  dynstrsec->contents = newcontents;

	  if (!_bfd_elf_add_dynamic_entry (info, DT_NEEDED, strindex))
	    return false;

	  if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_FIXUP_NEEDED,
					   shl_num))
	    return false;
	  shl_num++;

	  /* The fixups_off was in fact containing the size of the fixup
	     section.  Remap into the offset.  */
	  fixups_shl_off = elf_ia64_vms_tdata (abfd)->fixups_off;
	  elf_ia64_vms_tdata (abfd)->fixups_off = fixups_off;

	  if (!_bfd_elf_add_dynamic_entry
	      (info, DT_IA_64_VMS_FIXUP_RELA_CNT,
	       fixups_shl_off / sizeof (Elf64_External_VMS_IMAGE_FIXUP)))
	    return false;
	  if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_FIXUP_RELA_OFF,
					   fixups_off))
	    return false;
	  fixups_off += fixups_shl_off;
	}

      /* Unwind.  */
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_UNWINDSZ, 0))
	return false;
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_UNWIND_CODSEG, 0))
	return false;
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_UNWIND_INFOSEG, 0))
	return false;
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_UNWIND_OFFSET, 0))
	return false;
      if (!_bfd_elf_add_dynamic_entry (info, DT_IA_64_VMS_UNWIND_SEG, 0))
	return false;

      if (!_bfd_elf_add_dynamic_entry (info, DT_NULL, 0xdead))
	    return false;

      /* Fix the strtab entries.  */
      bed = get_elf_backend_data (hash_table->dynobj);

      if (dynstrsec->size > 1)
	dynstrsec->contents[0] = 0;
      else
	dynstrsec->size = 0;

      /* Note: one 'spare' (ie DT_NULL) entry is added by
	 bfd_elf_size_dynsym_hash_dynstr.  */
      dyn.d_tag = DT_IA_64_VMS_STRTAB_OFFSET;
      dyn.d_un.d_val = dynsec->size /* + sizeof (Elf64_External_Dyn) */;
      bed->s->swap_dyn_out (hash_table->dynobj, &dyn,
			    dynsec->contents + strdyn_off);

      dyn.d_tag = DT_STRSZ;
      dyn.d_un.d_val = dynstrsec->size;
      bed->s->swap_dyn_out (hash_table->dynobj, &dyn,
			    dynsec->contents + strdyn_off + bed->s->sizeof_dyn);

      elf_ia64_vms_tdata (output_bfd)->needed_count = shl_num;

      /* Note section.  */
      if (!create_ia64_vms_notes (output_bfd, info, time_hi, time_lo))
	return false;
    }

  /* ??? Perhaps force __gp local.  */

  return true;
}

static void
elf64_ia64_install_fixup (bfd *output_bfd,
			  struct elf64_ia64_link_hash_table *ia64_info,
			  struct elf_link_hash_entry *h,
			  unsigned int type, asection *sec, bfd_vma offset,
			  bfd_vma addend)
{
  asection *relsec;
  Elf64_External_VMS_IMAGE_FIXUP *fixup;
  struct elf64_ia64_link_hash_entry *h_ia64;
  bfd_vma fixoff;
  Elf_Internal_Phdr *phdr;

  if (h == NULL || !h->def_dynamic)
    abort ();

  h_ia64 = (struct elf64_ia64_link_hash_entry *) h;
  fixoff = elf_ia64_vms_tdata (h_ia64->shl)->fixups_off;
  elf_ia64_vms_tdata (h_ia64->shl)->fixups_off +=
    sizeof (Elf64_External_VMS_IMAGE_FIXUP);
  relsec = ia64_info->fixups_sec;

  fixup = (Elf64_External_VMS_IMAGE_FIXUP *)(relsec->contents + fixoff);
  offset += sec->output_section->vma + sec->output_offset;

  /* FIXME: this is slow.  We should cache the last one used, or create a
     map.  */
  phdr = _bfd_elf_find_segment_containing_section
    (output_bfd, sec->output_section);
  BFD_ASSERT (phdr != NULL);

  bfd_putl64 (offset - phdr->p_vaddr, fixup->fixup_offset);
  bfd_putl32 (type, fixup->type);
  bfd_putl32 (phdr - elf_tdata (output_bfd)->phdr, fixup->fixup_seg);
  bfd_putl64 (addend, fixup->addend);
  bfd_putl32 (h->root.u.def.value, fixup->symvec_index);
  bfd_putl32 (2, fixup->data_type);
}

/* Store an entry for target address TARGET_ADDR in the linkage table
   and return the gp-relative address of the linkage table entry.  */

static bfd_vma
set_got_entry (bfd *abfd, struct bfd_link_info *info,
	       struct elf64_ia64_dyn_sym_info *dyn_i,
	       bfd_vma addend, bfd_vma value, unsigned int dyn_r_type)
{
  struct elf64_ia64_link_hash_table *ia64_info;
  asection *got_sec;
  bool done;
  bfd_vma got_offset;

  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return 0;

  got_sec = ia64_info->root.sgot;

  switch (dyn_r_type)
    {
    case R_IA64_TPREL64LSB:
    case R_IA64_DTPMOD64LSB:
    case R_IA64_DTPREL32LSB:
    case R_IA64_DTPREL64LSB:
      abort ();
      break;
    default:
      done = dyn_i->got_done;
      dyn_i->got_done = true;
      got_offset = dyn_i->got_offset;
      break;
    }

  BFD_ASSERT ((got_offset & 7) == 0);

  if (! done)
    {
      /* Store the target address in the linkage table entry.  */
      bfd_put_64 (abfd, value, got_sec->contents + got_offset);

      /* Install a dynamic relocation if needed.  */
      if (((bfd_link_pic (info)
	    && (!dyn_i->h
		|| ELF_ST_VISIBILITY (dyn_i->h->other) == STV_DEFAULT
		|| dyn_i->h->root.type != bfd_link_hash_undefweak))
	   || elf64_ia64_dynamic_symbol_p (dyn_i->h))
	  && (!dyn_i->want_ltoff_fptr
	      || !bfd_link_pie (info)
	      || !dyn_i->h
	      || dyn_i->h->root.type != bfd_link_hash_undefweak))
	{
	  if (!dyn_i->h || !dyn_i->h->def_dynamic)
	    {
	      dyn_r_type = R_IA64_REL64LSB;
	      addend = value;
	    }

	  /* VMS: install a FIX32 or FIX64.  */
	  switch (dyn_r_type)
	    {
	    case R_IA64_DIR32LSB:
	    case R_IA64_FPTR32LSB:
	      dyn_r_type = R_IA64_VMS_FIX32;
	      break;
	    case R_IA64_DIR64LSB:
	    case R_IA64_FPTR64LSB:
	      dyn_r_type = R_IA64_VMS_FIX64;
	      break;
	    default:
	      BFD_ASSERT (false);
	      break;
	    }
	  elf64_ia64_install_fixup
	    (info->output_bfd, ia64_info, dyn_i->h,
	     dyn_r_type, got_sec, got_offset, addend);
	}
    }

  /* Return the address of the linkage table entry.  */
  value = (got_sec->output_section->vma
	   + got_sec->output_offset
	   + got_offset);

  return value;
}

/* Fill in a function descriptor consisting of the function's code
   address and its global pointer.  Return the descriptor's address.  */

static bfd_vma
set_fptr_entry (bfd *abfd, struct bfd_link_info *info,
		struct elf64_ia64_dyn_sym_info *dyn_i,
		bfd_vma value)
{
  struct elf64_ia64_link_hash_table *ia64_info;
  asection *fptr_sec;

  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return 0;

  fptr_sec = ia64_info->fptr_sec;

  if (!dyn_i->fptr_done)
    {
      dyn_i->fptr_done = 1;

      /* Fill in the function descriptor.  */
      bfd_put_64 (abfd, value, fptr_sec->contents + dyn_i->fptr_offset);
      bfd_put_64 (abfd, _bfd_get_gp_value (abfd),
		  fptr_sec->contents + dyn_i->fptr_offset + 8);
    }

  /* Return the descriptor's address.  */
  value = (fptr_sec->output_section->vma
	   + fptr_sec->output_offset
	   + dyn_i->fptr_offset);

  return value;
}

/* Fill in a PLTOFF entry consisting of the function's code address
   and its global pointer.  Return the descriptor's address.  */

static bfd_vma
set_pltoff_entry (bfd *abfd, struct bfd_link_info *info,
		  struct elf64_ia64_dyn_sym_info *dyn_i,
		  bfd_vma value, bool is_plt)
{
  struct elf64_ia64_link_hash_table *ia64_info;
  asection *pltoff_sec;

  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return 0;

  pltoff_sec = ia64_info->pltoff_sec;

  /* Don't do anything if this symbol uses a real PLT entry.  In
     that case, we'll fill this in during finish_dynamic_symbol.  */
  if ((! dyn_i->want_plt || is_plt)
      && !dyn_i->pltoff_done)
    {
      bfd_vma gp = _bfd_get_gp_value (abfd);

      /* Fill in the function descriptor.  */
      bfd_put_64 (abfd, value, pltoff_sec->contents + dyn_i->pltoff_offset);
      bfd_put_64 (abfd, gp, pltoff_sec->contents + dyn_i->pltoff_offset + 8);

      /* Install dynamic relocations if needed.  */
      if (!is_plt
	  && bfd_link_pic (info)
	  && (!dyn_i->h
	      || ELF_ST_VISIBILITY (dyn_i->h->other) == STV_DEFAULT
	      || dyn_i->h->root.type != bfd_link_hash_undefweak))
	{
	  /* VMS:  */
	  abort ();
	}

      dyn_i->pltoff_done = 1;
    }

  /* Return the descriptor's address.  */
  value = (pltoff_sec->output_section->vma
	   + pltoff_sec->output_offset
	   + dyn_i->pltoff_offset);

  return value;
}

/* Called through qsort to sort the .IA_64.unwind section during a
   non-relocatable link.  Set elf64_ia64_unwind_entry_compare_bfd
   to the output bfd so we can do proper endianness frobbing.  */

static bfd *elf64_ia64_unwind_entry_compare_bfd;

static int
elf64_ia64_unwind_entry_compare (const void * a, const void * b)
{
  bfd_vma av, bv;

  av = bfd_get_64 (elf64_ia64_unwind_entry_compare_bfd, a);
  bv = bfd_get_64 (elf64_ia64_unwind_entry_compare_bfd, b);

  return (av < bv ? -1 : av > bv ? 1 : 0);
}

/* Make sure we've got ourselves a nice fat __gp value.  */
static bool
elf64_ia64_choose_gp (bfd *abfd, struct bfd_link_info *info, bool final)
{
  bfd_vma min_vma = (bfd_vma) -1, max_vma = 0;
  bfd_vma min_short_vma = min_vma, max_short_vma = 0;
  struct elf_link_hash_entry *gp;
  bfd_vma gp_val;
  asection *os;
  struct elf64_ia64_link_hash_table *ia64_info;

  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return false;

  /* Find the min and max vma of all sections marked short.  Also collect
     min and max vma of any type, for use in selecting a nice gp.  */
  for (os = abfd->sections; os ; os = os->next)
    {
      bfd_vma lo, hi;

      if ((os->flags & SEC_ALLOC) == 0)
	continue;

      lo = os->vma;
      /* When this function is called from elfNN_ia64_final_link
	 the correct value to use is os->size.  When called from
	 elfNN_ia64_relax_section we are in the middle of section
	 sizing; some sections will already have os->size set, others
	 will have os->size zero and os->rawsize the previous size.  */
      hi = os->vma + (!final && os->rawsize ? os->rawsize : os->size);
      if (hi < lo)
	hi = (bfd_vma) -1;

      if (min_vma > lo)
	min_vma = lo;
      if (max_vma < hi)
	max_vma = hi;
      if (os->flags & SEC_SMALL_DATA)
	{
	  if (min_short_vma > lo)
	    min_short_vma = lo;
	  if (max_short_vma < hi)
	    max_short_vma = hi;
	}
    }

  if (ia64_info->min_short_sec)
    {
      if (min_short_vma
	  > (ia64_info->min_short_sec->vma
	     + ia64_info->min_short_offset))
	min_short_vma = (ia64_info->min_short_sec->vma
			 + ia64_info->min_short_offset);
      if (max_short_vma
	  < (ia64_info->max_short_sec->vma
	     + ia64_info->max_short_offset))
	max_short_vma = (ia64_info->max_short_sec->vma
			 + ia64_info->max_short_offset);
    }

  /* See if the user wants to force a value.  */
  gp = elf_link_hash_lookup (elf_hash_table (info), "__gp", false,
			     false, false);

  if (gp
      && (gp->root.type == bfd_link_hash_defined
	  || gp->root.type == bfd_link_hash_defweak))
    {
      asection *gp_sec = gp->root.u.def.section;
      gp_val = (gp->root.u.def.value
		+ gp_sec->output_section->vma
		+ gp_sec->output_offset);
    }
  else
    {
      /* Pick a sensible value.  */

      if (ia64_info->min_short_sec)
	{
	  bfd_vma short_range = max_short_vma - min_short_vma;

	  /* If min_short_sec is set, pick one in the middle bewteen
	     min_short_vma and max_short_vma.  */
	  if (short_range >= 0x400000)
	    goto overflow;
	  gp_val = min_short_vma + short_range / 2;
	}
      else
	{
	  asection *got_sec = ia64_info->root.sgot;

	  /* Start with just the address of the .got.  */
	  if (got_sec)
	    gp_val = got_sec->output_section->vma;
	  else if (max_short_vma != 0)
	    gp_val = min_short_vma;
	  else if (max_vma - min_vma < 0x200000)
	    gp_val = min_vma;
	  else
	    gp_val = max_vma - 0x200000 + 8;
	}

      /* If it is possible to address the entire image, but we
	 don't with the choice above, adjust.  */
      if (max_vma - min_vma < 0x400000
	  && (max_vma - gp_val >= 0x200000
	      || gp_val - min_vma > 0x200000))
	gp_val = min_vma + 0x200000;
      else if (max_short_vma != 0)
	{
	  /* If we don't cover all the short data, adjust.  */
	  if (max_short_vma - gp_val >= 0x200000)
	    gp_val = min_short_vma + 0x200000;

	  /* If we're addressing stuff past the end, adjust back.  */
	  if (gp_val > max_vma)
	    gp_val = max_vma - 0x200000 + 8;
	}
    }

  /* Validate whether all SHF_IA_64_SHORT sections are within
     range of the chosen GP.  */

  if (max_short_vma != 0)
    {
      if (max_short_vma - min_short_vma >= 0x400000)
	{
	overflow:
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: short data segment overflowed (%#" PRIx64 " >= 0x400000)"),
	     abfd, (uint64_t) (max_short_vma - min_short_vma));
	  return false;
	}
      else if ((gp_val > min_short_vma
		&& gp_val - min_short_vma > 0x200000)
	       || (gp_val < max_short_vma
		   && max_short_vma - gp_val >= 0x200000))
	{
	  _bfd_error_handler
	    (_("%pB: __gp does not cover short data segment"), abfd);
	  return false;
	}
    }

  _bfd_set_gp_value (abfd, gp_val);

  return true;
}

static bool
elf64_ia64_final_link (bfd *abfd, struct bfd_link_info *info)
{
  struct elf64_ia64_link_hash_table *ia64_info;
  asection *unwind_output_sec;

  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return false;

  /* Make sure we've got ourselves a nice fat __gp value.  */
  if (!bfd_link_relocatable (info))
    {
      bfd_vma gp_val;
      struct elf_link_hash_entry *gp;

      /* We assume after gp is set, section size will only decrease. We
	 need to adjust gp for it.  */
      _bfd_set_gp_value (abfd, 0);
      if (! elf64_ia64_choose_gp (abfd, info, true))
	return false;
      gp_val = _bfd_get_gp_value (abfd);

      gp = elf_link_hash_lookup (elf_hash_table (info), "__gp", false,
				 false, false);
      if (gp)
	{
	  gp->root.type = bfd_link_hash_defined;
	  gp->root.u.def.value = gp_val;
	  gp->root.u.def.section = bfd_abs_section_ptr;
	}
    }

  /* If we're producing a final executable, we need to sort the contents
     of the .IA_64.unwind section.  Force this section to be relocated
     into memory rather than written immediately to the output file.  */
  unwind_output_sec = NULL;
  if (!bfd_link_relocatable (info))
    {
      asection *s = bfd_get_section_by_name (abfd, ELF_STRING_ia64_unwind);
      if (s)
	{
	  unwind_output_sec = s->output_section;
	  unwind_output_sec->contents
	    = bfd_malloc (unwind_output_sec->size);
	  if (unwind_output_sec->contents == NULL)
	    return false;
	}
    }

  /* Invoke the regular ELF backend linker to do all the work.  */
  if (!bfd_elf_final_link (abfd, info))
    return false;

  if (unwind_output_sec)
    {
      elf64_ia64_unwind_entry_compare_bfd = abfd;
      qsort (unwind_output_sec->contents,
	     (size_t) (unwind_output_sec->size / 24),
	     24,
	     elf64_ia64_unwind_entry_compare);

      if (! bfd_set_section_contents (abfd, unwind_output_sec,
				      unwind_output_sec->contents, (bfd_vma) 0,
				      unwind_output_sec->size))
	return false;
    }

  return true;
}

static int
elf64_ia64_relocate_section (bfd *output_bfd,
			     struct bfd_link_info *info,
			     bfd *input_bfd,
			     asection *input_section,
			     bfd_byte *contents,
			     Elf_Internal_Rela *relocs,
			     Elf_Internal_Sym *local_syms,
			     asection **local_sections)
{
  struct elf64_ia64_link_hash_table *ia64_info;
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  bool ret_val = true;	/* for non-fatal errors */
  bfd_vma gp_val;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return false;

  /* Infect various flags from the input section to the output section.  */
  if (bfd_link_relocatable (info))
    {
      bfd_vma flags;

      flags = elf_section_data(input_section)->this_hdr.sh_flags;
      flags &= SHF_IA_64_NORECOV;

      elf_section_data(input_section->output_section)
	->this_hdr.sh_flags |= flags;
    }

  gp_val = _bfd_get_gp_value (output_bfd);

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; ++rel)
    {
      struct elf_link_hash_entry *h;
      struct elf64_ia64_dyn_sym_info *dyn_i;
      bfd_reloc_status_type r;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      unsigned int r_type;
      bfd_vma value;
      asection *sym_sec;
      bfd_byte *hit_addr;
      bool dynamic_symbol_p;
      bool undef_weak_ref;

      r_type = ELF64_R_TYPE (rel->r_info);
      if (r_type > R_IA64_MAX_RELOC_CODE)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      input_bfd, (int) r_type);
	  bfd_set_error (bfd_error_bad_value);
	  ret_val = false;
	  continue;
	}

      howto = ia64_elf_lookup_howto (r_type);
      if (howto == NULL)
	{
	  ret_val = false;
	  continue;
	}
      r_symndx = ELF64_R_SYM (rel->r_info);
      h = NULL;
      sym = NULL;
      sym_sec = NULL;
      undef_weak_ref = false;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* Reloc against local symbol.  */
	  asection *msec;
	  sym = local_syms + r_symndx;
	  sym_sec = local_sections[r_symndx];
	  msec = sym_sec;
	  value = _bfd_elf_rela_local_sym (output_bfd, sym, &msec, rel);
	  if (!bfd_link_relocatable (info)
	      && (sym_sec->flags & SEC_MERGE) != 0
	      && ELF_ST_TYPE (sym->st_info) == STT_SECTION
	      && sym_sec->sec_info_type == SEC_INFO_TYPE_MERGE)
	    {
	      struct elf64_ia64_local_hash_entry *loc_h;

	      loc_h = get_local_sym_hash (ia64_info, input_bfd, rel, false);
	      if (loc_h && ! loc_h->sec_merge_done)
		{
		  struct elf64_ia64_dyn_sym_info *dynent;
		  unsigned int count;

		  for (count = loc_h->count, dynent = loc_h->info;
		       count != 0;
		       count--, dynent++)
		    {
		      msec = sym_sec;
		      dynent->addend =
			_bfd_merged_section_offset (output_bfd, &msec,
						    elf_section_data (msec)->
						    sec_info,
						    sym->st_value
						    + dynent->addend);
		      dynent->addend -= sym->st_value;
		      dynent->addend += msec->output_section->vma
					+ msec->output_offset
					- sym_sec->output_section->vma
					- sym_sec->output_offset;
		    }

		  /* We may have introduced duplicated entries. We need
		     to remove them properly.  */
		  count = sort_dyn_sym_info (loc_h->info, loc_h->count);
		  if (count != loc_h->count)
		    {
		      loc_h->count = count;
		      loc_h->sorted_count = count;
		    }

		  loc_h->sec_merge_done = 1;
		}
	    }
	}
      else
	{
	  bool unresolved_reloc;
	  bool warned, ignored;
	  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (input_bfd);

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sym_sec, value,
				   unresolved_reloc, warned, ignored);

	  if (h->root.type == bfd_link_hash_undefweak)
	    undef_weak_ref = true;
	  else if (warned)
	    continue;
	}

      /* For relocs against symbols from removed linkonce sections,
	 or sections discarded by a linker script, we just want the
	 section contents zeroed.  Avoid any special processing.  */
      if (sym_sec != NULL && discarded_section (sym_sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      hit_addr = contents + rel->r_offset;
      value += rel->r_addend;
      dynamic_symbol_p = elf64_ia64_dynamic_symbol_p (h);

      switch (r_type)
	{
	case R_IA64_NONE:
	case R_IA64_LDXMOV:
	  continue;

	case R_IA64_IMM14:
	case R_IA64_IMM22:
	case R_IA64_IMM64:
	case R_IA64_DIR32MSB:
	case R_IA64_DIR32LSB:
	case R_IA64_DIR64MSB:
	case R_IA64_DIR64LSB:
	  /* Install a dynamic relocation for this reloc.  */
	  if ((dynamic_symbol_p || bfd_link_pic (info))
	      && r_symndx != 0
	      && (input_section->flags & SEC_ALLOC) != 0)
	    {
	      unsigned int dyn_r_type;
	      bfd_vma addend;

	      switch (r_type)
		{
		case R_IA64_IMM14:
		case R_IA64_IMM22:
		case R_IA64_IMM64:
		  /* ??? People shouldn't be doing non-pic code in
		     shared libraries nor dynamic executables.  */
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: non-pic code with imm relocation against"
		       " dynamic symbol `%s'"),
		     input_bfd,
		     h ? h->root.root.string
		       : bfd_elf_sym_name (input_bfd, symtab_hdr, sym,
					   sym_sec));
		  ret_val = false;
		  continue;

		default:
		  break;
		}

	      /* If we don't need dynamic symbol lookup, find a
		 matching RELATIVE relocation.  */
	      dyn_r_type = r_type;
	      if (dynamic_symbol_p)
		{
		  addend = rel->r_addend;
		  value = 0;
		}
	      else
		{
		  addend = value;
		}

	      /* VMS: install a FIX64.  */
	      switch (dyn_r_type)
		{
		case R_IA64_DIR32LSB:
		  dyn_r_type = R_IA64_VMS_FIX32;
		  break;
		case R_IA64_DIR64LSB:
		  dyn_r_type = R_IA64_VMS_FIX64;
		  break;
		default:
		  BFD_ASSERT (false);
		  break;
		}
	      elf64_ia64_install_fixup
		(output_bfd, ia64_info, h,
		 dyn_r_type, input_section, rel->r_offset, addend);
	      r = bfd_reloc_ok;
	      break;
	    }
	  /* Fall through.  */

	case R_IA64_LTV32MSB:
	case R_IA64_LTV32LSB:
	case R_IA64_LTV64MSB:
	case R_IA64_LTV64LSB:
	  r = ia64_elf_install_value (hit_addr, value, r_type);
	  break;

	case R_IA64_GPREL22:
	case R_IA64_GPREL64I:
	case R_IA64_GPREL32MSB:
	case R_IA64_GPREL32LSB:
	case R_IA64_GPREL64MSB:
	case R_IA64_GPREL64LSB:
	  if (dynamic_symbol_p)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: @gprel relocation against dynamic symbol %s"),
		 input_bfd,
		 h ? h->root.root.string
		   : bfd_elf_sym_name (input_bfd, symtab_hdr, sym,
				       sym_sec));
	      ret_val = false;
	      continue;
	    }
	  value -= gp_val;
	  r = ia64_elf_install_value (hit_addr, value, r_type);
	  break;

	case R_IA64_LTOFF22:
	case R_IA64_LTOFF22X:
	case R_IA64_LTOFF64I:
	  dyn_i = get_dyn_sym_info (ia64_info, h, input_bfd, rel, false);
	  value = set_got_entry (input_bfd, info, dyn_i,
				 rel->r_addend, value, R_IA64_DIR64LSB);
	  value -= gp_val;
	  r = ia64_elf_install_value (hit_addr, value, r_type);
	  break;

	case R_IA64_PLTOFF22:
	case R_IA64_PLTOFF64I:
	case R_IA64_PLTOFF64MSB:
	case R_IA64_PLTOFF64LSB:
	  dyn_i = get_dyn_sym_info (ia64_info, h, input_bfd, rel, false);
	  value = set_pltoff_entry (output_bfd, info, dyn_i, value, false);
	  value -= gp_val;
	  r = ia64_elf_install_value (hit_addr, value, r_type);
	  break;

	case R_IA64_FPTR64I:
	case R_IA64_FPTR32MSB:
	case R_IA64_FPTR32LSB:
	case R_IA64_FPTR64MSB:
	case R_IA64_FPTR64LSB:
	  dyn_i = get_dyn_sym_info (ia64_info, h, input_bfd, rel, false);
	  if (dyn_i->want_fptr)
	    {
	      if (!undef_weak_ref)
		value = set_fptr_entry (output_bfd, info, dyn_i, value);
	    }
	  if (!dyn_i->want_fptr || bfd_link_pie (info))
	    {
	      /* Otherwise, we expect the dynamic linker to create
		 the entry.  */

	      if (dyn_i->want_fptr)
		{
		  if (r_type == R_IA64_FPTR64I)
		    {
		      /* We can't represent this without a dynamic symbol.
			 Adjust the relocation to be against an output
			 section symbol, which are always present in the
			 dynamic symbol table.  */
		      /* ??? People shouldn't be doing non-pic code in
			 shared libraries.  Hork.  */
		      _bfd_error_handler
			(_("%pB: linking non-pic code in a position independent executable"),
			 input_bfd);
		      ret_val = false;
		      continue;
		    }
		}
	      else
		{
		  value = 0;
		}

	      /* VMS: FIXFD.  */
	      elf64_ia64_install_fixup
		(output_bfd, ia64_info, h, R_IA64_VMS_FIXFD,
		 input_section, rel->r_offset, 0);
	      r = bfd_reloc_ok;
	      break;
	    }

	  r = ia64_elf_install_value (hit_addr, value, r_type);
	  break;

	case R_IA64_LTOFF_FPTR22:
	case R_IA64_LTOFF_FPTR64I:
	case R_IA64_LTOFF_FPTR32MSB:
	case R_IA64_LTOFF_FPTR32LSB:
	case R_IA64_LTOFF_FPTR64MSB:
	case R_IA64_LTOFF_FPTR64LSB:
	  dyn_i = get_dyn_sym_info (ia64_info, h, input_bfd, rel, false);
	  if (dyn_i->want_fptr)
	    {
	      BFD_ASSERT (h == NULL || !h->def_dynamic);
	      if (!undef_weak_ref)
		value = set_fptr_entry (output_bfd, info, dyn_i, value);
	    }
	  else
	    value = 0;

	  value = set_got_entry (output_bfd, info, dyn_i,
				 rel->r_addend, value, R_IA64_FPTR64LSB);
	  value -= gp_val;
	  r = ia64_elf_install_value (hit_addr, value, r_type);
	  break;

	case R_IA64_PCREL32MSB:
	case R_IA64_PCREL32LSB:
	case R_IA64_PCREL64MSB:
	case R_IA64_PCREL64LSB:
	  /* Install a dynamic relocation for this reloc.  */
	  if (dynamic_symbol_p && r_symndx != 0)
	    {
	      /* VMS: doesn't exist ???  */
	      abort ();
	    }
	  goto finish_pcrel;

	case R_IA64_PCREL21B:
	case R_IA64_PCREL60B:
	  /* We should have created a PLT entry for any dynamic symbol.  */
	  dyn_i = NULL;
	  if (h)
	    dyn_i = get_dyn_sym_info (ia64_info, h, NULL, NULL, false);

	  if (dyn_i && dyn_i->want_plt2)
	    {
	      /* Should have caught this earlier.  */
	      BFD_ASSERT (rel->r_addend == 0);

	      value = (ia64_info->root.splt->output_section->vma
		       + ia64_info->root.splt->output_offset
		       + dyn_i->plt2_offset);
	    }
	  else
	    {
	      /* Since there's no PLT entry, Validate that this is
		 locally defined.  */
	      BFD_ASSERT (undef_weak_ref || sym_sec->output_section != NULL);

	      /* If the symbol is undef_weak, we shouldn't be trying
		 to call it.  There's every chance that we'd wind up
		 with an out-of-range fixup here.  Don't bother setting
		 any value at all.  */
	      if (undef_weak_ref)
		continue;
	    }
	  goto finish_pcrel;

	case R_IA64_PCREL21BI:
	case R_IA64_PCREL21F:
	case R_IA64_PCREL21M:
	case R_IA64_PCREL22:
	case R_IA64_PCREL64I:
	  /* The PCREL21BI reloc is specifically not intended for use with
	     dynamic relocs.  PCREL21F and PCREL21M are used for speculation
	     fixup code, and thus probably ought not be dynamic.  The
	     PCREL22 and PCREL64I relocs aren't emitted as dynamic relocs.  */
	  if (dynamic_symbol_p)
	    {
	      const char *msg;

	      if (r_type == R_IA64_PCREL21BI)
		/* xgettext:c-format */
		msg = _("%pB: @internal branch to dynamic symbol %s");
	      else if (r_type == R_IA64_PCREL21F || r_type == R_IA64_PCREL21M)
		/* xgettext:c-format */
		msg = _("%pB: speculation fixup to dynamic symbol %s");
	      else
		/* xgettext:c-format */
		msg = _("%pB: @pcrel relocation against dynamic symbol %s");
	      _bfd_error_handler (msg, input_bfd,
				  h ? h->root.root.string
				  : bfd_elf_sym_name (input_bfd,
						      symtab_hdr,
						      sym,
						      sym_sec));
	      ret_val = false;
	      continue;
	    }
	  goto finish_pcrel;

	finish_pcrel:
	  /* Make pc-relative.  */
	  value -= (input_section->output_section->vma
		    + input_section->output_offset
		    + rel->r_offset) & ~ (bfd_vma) 0x3;
	  r = ia64_elf_install_value (hit_addr, value, r_type);
	  break;

	case R_IA64_SEGREL32MSB:
	case R_IA64_SEGREL32LSB:
	case R_IA64_SEGREL64MSB:
	case R_IA64_SEGREL64LSB:
	    {
	      /* Find the segment that contains the output_section.  */
	      Elf_Internal_Phdr *p = _bfd_elf_find_segment_containing_section
		(output_bfd, sym_sec->output_section);

	      if (p == NULL)
		{
		  r = bfd_reloc_notsupported;
		}
	      else
		{
		  /* The VMA of the segment is the vaddr of the associated
		     program header.  */
		  if (value > p->p_vaddr)
		    value -= p->p_vaddr;
		  else
		    value = 0;
		  r = ia64_elf_install_value (hit_addr, value, r_type);
		}
	      break;
	    }

	case R_IA64_SECREL32MSB:
	case R_IA64_SECREL32LSB:
	case R_IA64_SECREL64MSB:
	case R_IA64_SECREL64LSB:
	  /* Make output-section relative to section where the symbol
	     is defined. PR 475  */
	  if (sym_sec)
	    value -= sym_sec->output_section->vma;
	  r = ia64_elf_install_value (hit_addr, value, r_type);
	  break;

	case R_IA64_IPLTMSB:
	case R_IA64_IPLTLSB:
	  /* Install a dynamic relocation for this reloc.  */
	  if ((dynamic_symbol_p || bfd_link_pic (info))
	      && (input_section->flags & SEC_ALLOC) != 0)
	    {
	      /* VMS: FIXFD ??  */
	      abort ();
	    }

	  if (r_type == R_IA64_IPLTMSB)
	    r_type = R_IA64_DIR64MSB;
	  else
	    r_type = R_IA64_DIR64LSB;
	  ia64_elf_install_value (hit_addr, value, r_type);
	  r = ia64_elf_install_value (hit_addr + 8, gp_val, r_type);
	  break;

	case R_IA64_TPREL14:
	case R_IA64_TPREL22:
	case R_IA64_TPREL64I:
	  r = bfd_reloc_notsupported;
	  break;

	case R_IA64_DTPREL14:
	case R_IA64_DTPREL22:
	case R_IA64_DTPREL64I:
	case R_IA64_DTPREL32LSB:
	case R_IA64_DTPREL32MSB:
	case R_IA64_DTPREL64LSB:
	case R_IA64_DTPREL64MSB:
	  r = bfd_reloc_notsupported;
	  break;

	case R_IA64_LTOFF_TPREL22:
	case R_IA64_LTOFF_DTPMOD22:
	case R_IA64_LTOFF_DTPREL22:
	  r = bfd_reloc_notsupported;
	  break;

	default:
	  r = bfd_reloc_notsupported;
	  break;
	}

      switch (r)
	{
	case bfd_reloc_ok:
	  break;

	case bfd_reloc_undefined:
	  /* This can happen for global table relative relocs if
	     __gp is undefined.  This is a panic situation so we
	     don't try to continue.  */
	  (*info->callbacks->undefined_symbol)
	    (info, "__gp", input_bfd, input_section, rel->r_offset, 1);
	  return false;

	case bfd_reloc_notsupported:
	  {
	    const char *name;

	    if (h)
	      name = h->root.root.string;
	    else
	      name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym,
				       sym_sec);
	    (*info->callbacks->warning) (info, _("unsupported reloc"),
					 name, input_bfd,
					 input_section, rel->r_offset);
	    ret_val = false;
	  }
	  break;

	case bfd_reloc_dangerous:
	case bfd_reloc_outofrange:
	case bfd_reloc_overflow:
	default:
	  {
	    const char *name;

	    if (h)
	      name = h->root.root.string;
	    else
	      name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym,
				       sym_sec);

	    switch (r_type)
	      {
	      case R_IA64_TPREL14:
	      case R_IA64_TPREL22:
	      case R_IA64_TPREL64I:
	      case R_IA64_DTPREL14:
	      case R_IA64_DTPREL22:
	      case R_IA64_DTPREL64I:
	      case R_IA64_DTPREL32LSB:
	      case R_IA64_DTPREL32MSB:
	      case R_IA64_DTPREL64LSB:
	      case R_IA64_DTPREL64MSB:
	      case R_IA64_LTOFF_TPREL22:
	      case R_IA64_LTOFF_DTPMOD22:
	      case R_IA64_LTOFF_DTPREL22:
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("%pB: missing TLS section for relocation %s against `%s'"
		     " at %#" PRIx64 " in section `%pA'."),
		   input_bfd, howto->name, name,
		   (uint64_t) rel->r_offset, input_section);
		break;

	      case R_IA64_PCREL21B:
	      case R_IA64_PCREL21BI:
	      case R_IA64_PCREL21M:
	      case R_IA64_PCREL21F:
		if (is_elf_hash_table (info->hash))
		  {
		    /* Relaxtion is always performed for ELF output.
		       Overflow failures for those relocations mean
		       that the section is too big to relax.  */
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: Can't relax br (%s) to `%s' "
			 "at %#" PRIx64 " in section `%pA' "
			 "with size %#" PRIx64 " (> 0x1000000)."),
		       input_bfd, howto->name, name, (uint64_t) rel->r_offset,
		       input_section, (uint64_t) input_section->size);
		    break;
		  }
		/* Fall through.  */
	      default:
		(*info->callbacks->reloc_overflow) (info,
						    &h->root,
						    name,
						    howto->name,
						    (bfd_vma) 0,
						    input_bfd,
						    input_section,
						    rel->r_offset);
		break;
	      }

	    ret_val = false;
	  }
	  break;
	}
    }

  return ret_val;
}

static bool
elf64_ia64_finish_dynamic_symbol (bfd *output_bfd,
				  struct bfd_link_info *info,
				  struct elf_link_hash_entry *h,
				  Elf_Internal_Sym *sym)
{
  struct elf64_ia64_link_hash_table *ia64_info;
  struct elf64_ia64_dyn_sym_info *dyn_i;

  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return false;

  dyn_i = get_dyn_sym_info (ia64_info, h, NULL, NULL, false);

  /* Fill in the PLT data, if required.  */
  if (dyn_i && dyn_i->want_plt)
    {
      bfd_byte *loc;
      asection *plt_sec;
      bfd_vma plt_addr, pltoff_addr, gp_val;

      gp_val = _bfd_get_gp_value (output_bfd);

      plt_sec = ia64_info->root.splt;
      plt_addr = 0;  /* Not used as overriden by FIXUPs.  */
      pltoff_addr = set_pltoff_entry (output_bfd, info, dyn_i, plt_addr, true);

      /* Initialize the FULL PLT entry, if needed.  */
      if (dyn_i->want_plt2)
	{
	  loc = plt_sec->contents + dyn_i->plt2_offset;

	  memcpy (loc, plt_full_entry, PLT_FULL_ENTRY_SIZE);
	  ia64_elf_install_value (loc, pltoff_addr - gp_val, R_IA64_IMM22);

	  /* Mark the symbol as undefined, rather than as defined in the
	     plt section.  Leave the value alone.  */
	  /* ??? We didn't redefine it in adjust_dynamic_symbol in the
	     first place.  But perhaps elflink.c did some for us.  */
	  if (!h->def_regular)
	    sym->st_shndx = SHN_UNDEF;
	}

      /* VMS: FIXFD.  */
      elf64_ia64_install_fixup
	(output_bfd, ia64_info, h, R_IA64_VMS_FIXFD, ia64_info->pltoff_sec,
	 pltoff_addr - (ia64_info->pltoff_sec->output_section->vma
			+ ia64_info->pltoff_sec->output_offset), 0);
    }

  /* Mark some specially defined symbols as absolute.  */
  if (h == ia64_info->root.hdynamic
      || h == ia64_info->root.hgot
      || h == ia64_info->root.hplt)
    sym->st_shndx = SHN_ABS;

  return true;
}

static bool
elf64_ia64_finish_dynamic_sections (bfd *abfd,
				    struct bfd_link_info *info)
{
  struct elf64_ia64_link_hash_table *ia64_info;
  bfd *dynobj;

  ia64_info = elf64_ia64_hash_table (info);
  if (ia64_info == NULL)
    return false;

  dynobj = ia64_info->root.dynobj;

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      Elf64_External_Dyn *dyncon, *dynconend;
      asection *sdyn;
      asection *unwind_sec;
      bfd_vma gp_val;
      unsigned int gp_seg;
      bfd_vma gp_off;
      Elf_Internal_Phdr *phdr;
      Elf_Internal_Phdr *base_phdr;
      unsigned int unwind_seg = 0;
      unsigned int code_seg = 0;

      sdyn = bfd_get_linker_section (dynobj, ".dynamic");
      BFD_ASSERT (sdyn != NULL);
      dyncon = (Elf64_External_Dyn *) sdyn->contents;
      dynconend = (Elf64_External_Dyn *) (sdyn->contents + sdyn->size);

      gp_val = _bfd_get_gp_value (abfd);
      phdr = _bfd_elf_find_segment_containing_section
	(info->output_bfd, ia64_info->pltoff_sec->output_section);
      BFD_ASSERT (phdr != NULL);
      base_phdr = elf_tdata (info->output_bfd)->phdr;
      gp_seg = phdr - base_phdr;
      gp_off = gp_val - phdr->p_vaddr;

      unwind_sec = bfd_get_section_by_name (abfd, ELF_STRING_ia64_unwind);
      if (unwind_sec != NULL)
	{
	  asection *code_sec;

	  phdr = _bfd_elf_find_segment_containing_section (abfd, unwind_sec);
	  BFD_ASSERT (phdr != NULL);
	  unwind_seg = phdr - base_phdr;

	  code_sec = bfd_get_section_by_name (abfd, "$CODE$");
	  phdr = _bfd_elf_find_segment_containing_section (abfd, code_sec);
	  BFD_ASSERT (phdr != NULL);
	  code_seg = phdr - base_phdr;
	}

      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;

	  bfd_elf64_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    case DT_IA_64_VMS_FIXUP_RELA_OFF:
	      dyn.d_un.d_val +=
		(ia64_info->fixups_sec->output_section->vma
		 + ia64_info->fixups_sec->output_offset)
		- (sdyn->output_section->vma + sdyn->output_offset);
	      break;

	    case DT_IA_64_VMS_PLTGOT_OFFSET:
	      dyn.d_un.d_val = gp_off;
	      break;

	    case DT_IA_64_VMS_PLTGOT_SEG:
	      dyn.d_un.d_val = gp_seg;
	      break;

	    case DT_IA_64_VMS_UNWINDSZ:
	      if (unwind_sec == NULL)
		{
		  dyn.d_tag = DT_NULL;
		  dyn.d_un.d_val = 0xdead;
		}
	      else
		dyn.d_un.d_val = unwind_sec->size;
	      break;

	    case DT_IA_64_VMS_UNWIND_CODSEG:
	      dyn.d_un.d_val = code_seg;
	      break;

	    case DT_IA_64_VMS_UNWIND_INFOSEG:
	    case DT_IA_64_VMS_UNWIND_SEG:
	      dyn.d_un.d_val = unwind_seg;
	      break;

	    case DT_IA_64_VMS_UNWIND_OFFSET:
	      break;

	    default:
	      /* No need to rewrite the entry.  */
	      continue;
	    }

	  bfd_elf64_swap_dyn_out (abfd, &dyn, dyncon);
	}
    }

  /* Handle transfer addresses.  */
  {
    asection *tfr_sec = ia64_info->transfer_sec;
    struct elf64_vms_transfer *tfr;
    struct elf_link_hash_entry *tfr3;

    tfr = (struct elf64_vms_transfer *)tfr_sec->contents;
    bfd_putl32 (6 * 8, tfr->size);
    bfd_putl64 (tfr_sec->output_section->vma
		+ tfr_sec->output_offset
		+ 6 * 8, tfr->tfradr3);

    tfr3 = elf_link_hash_lookup (elf_hash_table (info), "ELF$TFRADR", false,
				 false, false);

    if (tfr3
	&& (tfr3->root.type == bfd_link_hash_defined
	    || tfr3->root.type == bfd_link_hash_defweak))
      {
	asection *tfr3_sec = tfr3->root.u.def.section;
	bfd_vma tfr3_val;

	tfr3_val = (tfr3->root.u.def.value
		    + tfr3_sec->output_section->vma
		    + tfr3_sec->output_offset);

	bfd_putl64 (tfr3_val, tfr->tfr3_func);
	bfd_putl64 (_bfd_get_gp_value (info->output_bfd), tfr->tfr3_gp);
      }

    /* FIXME: set linker flags,
       handle lib$initialize.  */
  }

  return true;
}

/* ELF file flag handling:  */

/* Function to keep IA-64 specific file flags.  */
static bool
elf64_ia64_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */
static bool
elf64_ia64_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword out_flags;
  flagword in_flags;
  bool ok = true;

  /* FIXME: What should be checked when linking shared libraries?  */
  if ((ibfd->flags & DYNAMIC) != 0)
    return true;

  /* Don't even pretend to support mixed-format linking.  */
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return false;

  in_flags  = elf_elfheader (ibfd)->e_flags;
  out_flags = elf_elfheader (obfd)->e_flags;

  if (! elf_flags_init (obfd))
    {
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = in_flags;

      if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
	  && bfd_get_arch_info (obfd)->the_default)
	{
	  return bfd_set_arch_mach (obfd, bfd_get_arch (ibfd),
				    bfd_get_mach (ibfd));
	}

      return true;
    }

  /* Check flag compatibility.  */
  if (in_flags == out_flags)
    return true;

  /* Output has EF_IA_64_REDUCEDFP set only if all inputs have it set.  */
  if (!(in_flags & EF_IA_64_REDUCEDFP) && (out_flags & EF_IA_64_REDUCEDFP))
    elf_elfheader (obfd)->e_flags &= ~EF_IA_64_REDUCEDFP;

  if ((in_flags & EF_IA_64_TRAPNIL) != (out_flags & EF_IA_64_TRAPNIL))
    {
      _bfd_error_handler
	(_("%pB: linking trap-on-NULL-dereference with non-trapping files"),
	 ibfd);

      bfd_set_error (bfd_error_bad_value);
      ok = false;
    }
  if ((in_flags & EF_IA_64_BE) != (out_flags & EF_IA_64_BE))
    {
      _bfd_error_handler
	(_("%pB: linking big-endian files with little-endian files"),
	 ibfd);

      bfd_set_error (bfd_error_bad_value);
      ok = false;
    }
  if ((in_flags & EF_IA_64_ABI64) != (out_flags & EF_IA_64_ABI64))
    {
      _bfd_error_handler
	(_("%pB: linking 64-bit files with 32-bit files"),
	 ibfd);

      bfd_set_error (bfd_error_bad_value);
      ok = false;
    }
  if ((in_flags & EF_IA_64_CONS_GP) != (out_flags & EF_IA_64_CONS_GP))
    {
      _bfd_error_handler
	(_("%pB: linking constant-gp files with non-constant-gp files"),
	 ibfd);

      bfd_set_error (bfd_error_bad_value);
      ok = false;
    }
  if ((in_flags & EF_IA_64_NOFUNCDESC_CONS_GP)
      != (out_flags & EF_IA_64_NOFUNCDESC_CONS_GP))
    {
      _bfd_error_handler
	(_("%pB: linking auto-pic files with non-auto-pic files"),
	 ibfd);

      bfd_set_error (bfd_error_bad_value);
      ok = false;
    }

  return ok;
}

static bool
elf64_ia64_print_private_bfd_data (bfd *abfd, void * ptr)
{
  FILE *file = (FILE *) ptr;
  flagword flags = elf_elfheader (abfd)->e_flags;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  fprintf (file, "private flags = %s%s%s%s%s%s%s%s\n",
	   (flags & EF_IA_64_TRAPNIL) ? "TRAPNIL, " : "",
	   (flags & EF_IA_64_EXT) ? "EXT, " : "",
	   (flags & EF_IA_64_BE) ? "BE, " : "LE, ",
	   (flags & EF_IA_64_REDUCEDFP) ? "REDUCEDFP, " : "",
	   (flags & EF_IA_64_CONS_GP) ? "CONS_GP, " : "",
	   (flags & EF_IA_64_NOFUNCDESC_CONS_GP) ? "NOFUNCDESC_CONS_GP, " : "",
	   (flags & EF_IA_64_ABSOLUTE) ? "ABSOLUTE, " : "",
	   (flags & EF_IA_64_ABI64) ? "ABI64" : "ABI32");

  _bfd_elf_print_private_bfd_data (abfd, ptr);
  return true;
}

static enum elf_reloc_type_class
elf64_ia64_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			     const asection *rel_sec ATTRIBUTE_UNUSED,
			     const Elf_Internal_Rela *rela)
{
  switch ((int) ELF64_R_TYPE (rela->r_info))
    {
    case R_IA64_REL32MSB:
    case R_IA64_REL32LSB:
    case R_IA64_REL64MSB:
    case R_IA64_REL64LSB:
      return reloc_class_relative;
    case R_IA64_IPLTMSB:
    case R_IA64_IPLTLSB:
      return reloc_class_plt;
    case R_IA64_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

static const struct bfd_elf_special_section elf64_ia64_special_sections[] =
{
  { STRING_COMMA_LEN (".sbss"),	 -1, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE + SHF_IA_64_SHORT },
  { STRING_COMMA_LEN (".sdata"), -1, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE + SHF_IA_64_SHORT },
  { NULL,		     0,	  0, 0,		   0 }
};

static bool
elf64_ia64_object_p (bfd *abfd)
{
  asection *sec;
  asection *group, *unwi, *unw;
  flagword flags;
  const char *name;
  char *unwi_name, *unw_name;
  size_t amt;

  if (abfd->flags & DYNAMIC)
    return true;

  /* Flags for fake group section.  */
  flags = (SEC_LINKER_CREATED | SEC_GROUP | SEC_LINK_ONCE
	   | SEC_EXCLUDE);

  /* We add a fake section group for each .gnu.linkonce.t.* section,
     which isn't in a section group, and its unwind sections.  */
  for (sec = abfd->sections; sec != NULL; sec = sec->next)
    {
      if (elf_sec_group (sec) == NULL
	  && ((sec->flags & (SEC_LINK_ONCE | SEC_CODE | SEC_GROUP))
	      == (SEC_LINK_ONCE | SEC_CODE))
	  && startswith (sec->name, ".gnu.linkonce.t."))
	{
	  name = sec->name + 16;

	  amt = strlen (name) + sizeof (".gnu.linkonce.ia64unwi.");
	  unwi_name = bfd_alloc (abfd, amt);
	  if (!unwi_name)
	    return false;

	  strcpy (stpcpy (unwi_name, ".gnu.linkonce.ia64unwi."), name);
	  unwi = bfd_get_section_by_name (abfd, unwi_name);

	  amt = strlen (name) + sizeof (".gnu.linkonce.ia64unw.");
	  unw_name = bfd_alloc (abfd, amt);
	  if (!unw_name)
	    return false;

	  strcpy (stpcpy (unw_name, ".gnu.linkonce.ia64unw."), name);
	  unw = bfd_get_section_by_name (abfd, unw_name);

	  /* We need to create a fake group section for it and its
	     unwind sections.  */
	  group = bfd_make_section_anyway_with_flags (abfd, name,
						      flags);
	  if (group == NULL)
	    return false;

	  /* Move the fake group section to the beginning.  */
	  bfd_section_list_remove (abfd, group);
	  bfd_section_list_prepend (abfd, group);

	  elf_next_in_group (group) = sec;

	  elf_group_name (sec) = name;
	  elf_next_in_group (sec) = sec;
	  elf_sec_group (sec) = group;

	  if (unwi)
	    {
	      elf_group_name (unwi) = name;
	      elf_next_in_group (unwi) = sec;
	      elf_next_in_group (sec) = unwi;
	      elf_sec_group (unwi) = group;
	    }

	   if (unw)
	     {
	       elf_group_name (unw) = name;
	       if (unwi)
		 {
		   elf_next_in_group (unw) = elf_next_in_group (unwi);
		   elf_next_in_group (unwi) = unw;
		 }
	       else
		 {
		   elf_next_in_group (unw) = sec;
		   elf_next_in_group (sec) = unw;
		 }
	       elf_sec_group (unw) = group;
	     }

	   /* Fake SHT_GROUP section header.  */
	  elf_section_data (group)->this_hdr.bfd_section = group;
	  elf_section_data (group)->this_hdr.sh_type = SHT_GROUP;
	}
    }
  return true;
}

/* Handle an IA-64 specific section when reading an object file.  This
   is called when bfd_section_from_shdr finds a section with an unknown
   type.  */

static bool
elf64_vms_section_from_shdr (bfd *abfd,
			     Elf_Internal_Shdr *hdr,
			     const char *name,
			     int shindex)
{
  flagword secflags = 0;

  switch (hdr->sh_type)
    {
    case SHT_IA_64_VMS_TRACE:
    case SHT_IA_64_VMS_DEBUG:
    case SHT_IA_64_VMS_DEBUG_STR:
      secflags = SEC_DEBUGGING;
      break;

    case SHT_IA_64_UNWIND:
    case SHT_IA_64_HP_OPT_ANOT:
      break;

    case SHT_IA_64_EXT:
      if (strcmp (name, ELF_STRING_ia64_archext) != 0)
	return false;
      break;

    default:
      return false;
    }

  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return false;

  if (secflags != 0)
    {
      asection *newsect = hdr->bfd_section;

      if (!bfd_set_section_flags (newsect,
				  bfd_section_flags (newsect) | secflags))
	return false;
    }

  return true;
}

static bool
elf64_vms_object_p (bfd *abfd)
{
  Elf_Internal_Ehdr *i_ehdrp = elf_elfheader (abfd);
  Elf_Internal_Phdr *i_phdr = elf_tdata (abfd)->phdr;
  unsigned int i;
  unsigned int num_text = 0;
  unsigned int num_data = 0;
  unsigned int num_rodata = 0;
  char name[16];

  if (!elf64_ia64_object_p (abfd))
    return false;

  /* Many VMS compilers do not generate sections for the corresponding
     segment.  This is boring as binutils tools won't be able to disassemble
     the code.  So we simply create all the missing sections.  */
  for (i = 0; i < i_ehdrp->e_phnum; i++, i_phdr++)
    {
      /* Is there a section for this segment?  */
      bfd_vma base_vma = i_phdr->p_vaddr;
      bfd_vma limit_vma = base_vma + i_phdr->p_filesz;

      if (i_phdr->p_type != PT_LOAD)
	continue;

      /* We need to cover from base_vms to limit_vma.  */
    again:
      while (base_vma < limit_vma)
	{
	  bfd_vma next_vma = limit_vma;
	  asection *nsec;
	  asection *sec;
	  flagword flags;
	  char *nname = NULL;

	  /* Find a section covering [base_vma;limit_vma)  */
	  for (sec = abfd->sections; sec != NULL; sec = sec->next)
	    {
	      /* Skip uninteresting sections (either not in memory or
		 below base_vma.  */
	      if ((sec->flags & (SEC_ALLOC | SEC_LOAD)) == 0
		  || sec->vma + sec->size <= base_vma)
		continue;
	      if (sec->vma <= base_vma)
		{
		  /* This section covers (maybe partially) the beginning
		     of the range.  */
		  base_vma = sec->vma + sec->size;
		  goto again;
		}
	      if (sec->vma < next_vma)
		{
		  /* This section partially covers the end of the range.
		     Used to compute the size of the hole.  */
		  next_vma = sec->vma;
		}
	    }

	  /* No section covering [base_vma; next_vma).  Create a fake one.  */
	  flags = SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS;
	  if (i_phdr->p_flags & PF_X)
	    {
	      flags |= SEC_CODE;
	      if (num_text++ == 0)
		nname = ".text";
	      else
		sprintf (name, ".text$%u", num_text);
	    }
	  else if ((i_phdr->p_flags & (PF_R | PF_W)) == PF_R)
	    {
	      flags |= SEC_READONLY;
	      sprintf (name, ".rodata$%u", num_rodata++);
	    }
	  else
	    {
	      flags |= SEC_DATA;
	      sprintf (name, ".data$%u", num_data++);
	    }

	  /* Allocate name.  */
	  if (nname == NULL)
	    {
	      size_t name_len = strlen (name) + 1;
	      nname = bfd_alloc (abfd, name_len);
	      if (nname == NULL)
		return false;
	      memcpy (nname, name, name_len);
	    }

	  /* Create and fill new section.  */
	  nsec = bfd_make_section_anyway_with_flags (abfd, nname, flags);
	  if (nsec == NULL)
	    return false;
	  nsec->vma = base_vma;
	  nsec->size = next_vma - base_vma;
	  nsec->filepos = i_phdr->p_offset + (base_vma - i_phdr->p_vaddr);

	  base_vma = next_vma;
	}
    }
  return true;
}

static bool
elf64_vms_init_file_header (bfd *abfd, struct bfd_link_info *info)
{
  Elf_Internal_Ehdr *i_ehdrp;

  if (!_bfd_elf_init_file_header (abfd, info))
    return false;

  i_ehdrp = elf_elfheader (abfd);
  i_ehdrp->e_ident[EI_OSABI] = ELFOSABI_OPENVMS;
  i_ehdrp->e_ident[EI_ABIVERSION] = 2;
  return true;
}

static bool
elf64_vms_section_processing (bfd *abfd ATTRIBUTE_UNUSED,
			      Elf_Internal_Shdr *hdr)
{
  if (hdr->bfd_section != NULL)
    {
      const char *name = bfd_section_name (hdr->bfd_section);

      if (strcmp (name, ".text") == 0)
	hdr->sh_flags |= SHF_IA_64_VMS_SHARED;
      else if ((strcmp (name, ".debug") == 0)
	    || (strcmp (name, ".debug_abbrev") == 0)
	    || (strcmp (name, ".debug_aranges") == 0)
	    || (strcmp (name, ".debug_frame") == 0)
	    || (strcmp (name, ".debug_info") == 0)
	    || (strcmp (name, ".debug_loc") == 0)
	    || (strcmp (name, ".debug_macinfo") == 0)
	    || (strcmp (name, ".debug_pubnames") == 0)
	    || (strcmp (name, ".debug_pubtypes") == 0))
	hdr->sh_type = SHT_IA_64_VMS_DEBUG;
      else if ((strcmp (name, ".debug_line") == 0)
	    || (strcmp (name, ".debug_ranges") == 0)
	    || (strcmp (name, ".trace_info") == 0)
	    || (strcmp (name, ".trace_abbrev") == 0)
	    || (strcmp (name, ".trace_aranges") == 0))
	hdr->sh_type = SHT_IA_64_VMS_TRACE;
      else if (strcmp (name, ".debug_str") == 0)
	hdr->sh_type = SHT_IA_64_VMS_DEBUG_STR;
    }

  return true;
}

/* The final processing done just before writing out a VMS IA-64 ELF
   object file.  */

static bool
elf64_vms_final_write_processing (bfd *abfd)
{
  Elf_Internal_Shdr *hdr;
  asection *s;
  int unwind_info_sect_idx = 0;

  for (s = abfd->sections; s; s = s->next)
    {
      hdr = &elf_section_data (s)->this_hdr;

      if (strcmp (bfd_section_name (hdr->bfd_section),
		  ".IA_64.unwind_info") == 0)
	unwind_info_sect_idx = elf_section_data (s)->this_idx;

      switch (hdr->sh_type)
	{
	case SHT_IA_64_UNWIND:
	  /* VMS requires sh_info to point to the unwind info section.  */
	  hdr->sh_info = unwind_info_sect_idx;
	  break;
	}
    }

  if (! elf_flags_init (abfd))
    {
      unsigned long flags = 0;

      if (abfd->xvec->byteorder == BFD_ENDIAN_BIG)
	flags |= EF_IA_64_BE;
      if (bfd_get_mach (abfd) == bfd_mach_ia64_elf64)
	flags |= EF_IA_64_ABI64;

      elf_elfheader (abfd)->e_flags = flags;
      elf_flags_init (abfd) = true;
    }
  return _bfd_elf_final_write_processing (abfd);
}

static bool
elf64_vms_write_shdrs_and_ehdr (bfd *abfd)
{
  unsigned char needed_count[8];

  if (!bfd_elf64_write_shdrs_and_ehdr (abfd))
    return false;

  bfd_putl64 (elf_ia64_vms_tdata (abfd)->needed_count, needed_count);

  if (bfd_seek (abfd, sizeof (Elf64_External_Ehdr), SEEK_SET) != 0
      || bfd_bwrite (needed_count, 8, abfd) != 8)
    return false;

  return true;
}

static bool
elf64_vms_close_and_cleanup (bfd *abfd)
{
  if (bfd_get_format (abfd) == bfd_object)
    {
      long isize;

      /* Pad to 8 byte boundary for IPF/VMS.  */
      isize = bfd_get_size (abfd);
      if ((isize & 7) != 0)
	{
	  int ishort = 8 - (isize & 7);
	  uint64_t pad = 0;

	  bfd_seek (abfd, isize, SEEK_SET);
	  bfd_bwrite (&pad, ishort, abfd);
	}
    }

  return _bfd_generic_close_and_cleanup (abfd);
}

/* Add symbols from an ELF object file to the linker hash table.  */

static bool
elf64_vms_link_add_object_symbols (bfd *abfd, struct bfd_link_info *info)
{
  Elf_Internal_Shdr *hdr;
  bfd_size_type symcount;
  bfd_size_type extsymcount;
  bfd_size_type extsymoff;
  struct elf_link_hash_entry **sym_hash;
  bool dynamic;
  Elf_Internal_Sym *isymbuf = NULL;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  const struct elf_backend_data *bed;
  struct elf_link_hash_table *htab;
  bfd_size_type amt;

  htab = elf_hash_table (info);
  bed = get_elf_backend_data (abfd);

  if ((abfd->flags & DYNAMIC) == 0)
    dynamic = false;
  else
    {
      dynamic = true;

      /* You can't use -r against a dynamic object.  Also, there's no
	 hope of using a dynamic object which does not exactly match
	 the format of the output file.  */
      if (bfd_link_relocatable (info)
	  || !is_elf_hash_table (&htab->root)
	  || info->output_bfd->xvec != abfd->xvec)
	{
	  if (bfd_link_relocatable (info))
	    bfd_set_error (bfd_error_invalid_operation);
	  else
	    bfd_set_error (bfd_error_wrong_format);
	  goto error_return;
	}
    }

  if (! dynamic)
    {
      /* If we are creating a shared library, create all the dynamic
	 sections immediately.  We need to attach them to something,
	 so we attach them to this BFD, provided it is the right
	 format.  FIXME: If there are no input BFD's of the same
	 format as the output, we can't make a shared library.  */
      if (bfd_link_pic (info)
	  && is_elf_hash_table (&htab->root)
	  && info->output_bfd->xvec == abfd->xvec
	  && !htab->dynamic_sections_created)
	{
	  if (! elf64_ia64_create_dynamic_sections (abfd, info))
	    goto error_return;
	}
    }
  else if (!is_elf_hash_table (&htab->root))
    goto error_return;
  else
    {
      asection *s;
      bfd_byte *dynbuf;
      bfd_byte *extdyn;

      /* ld --just-symbols and dynamic objects don't mix very well.
	 ld shouldn't allow it.  */
      if ((s = abfd->sections) != NULL
	  && s->sec_info_type == SEC_INFO_TYPE_JUST_SYMS)
	abort ();

      /* Be sure there are dynamic sections.  */
      if (! elf64_ia64_create_dynamic_sections (htab->dynobj, info))
	goto error_return;

      s = bfd_get_section_by_name (abfd, ".dynamic");
      if (s == NULL)
	{
	  /* VMS libraries do not have dynamic sections.  Create one from
	     the segment.  */
	  Elf_Internal_Phdr *phdr;
	  unsigned int i, phnum;

	  phdr = elf_tdata (abfd)->phdr;
	  if (phdr == NULL)
	    goto error_return;
	  phnum = elf_elfheader (abfd)->e_phnum;
	  for (i = 0; i < phnum; phdr++)
	    if (phdr->p_type == PT_DYNAMIC)
	      {
		s = bfd_make_section (abfd, ".dynamic");
		if (s == NULL)
		  goto error_return;
		s->vma = phdr->p_vaddr;
		s->lma = phdr->p_paddr;
		s->size = phdr->p_filesz;
		s->filepos = phdr->p_offset;
		s->flags |= SEC_HAS_CONTENTS;
		s->alignment_power = bfd_log2 (phdr->p_align);
		break;
	      }
	  if (s == NULL)
	    goto error_return;
	}

      /* Extract IDENT.  */
      if (!bfd_malloc_and_get_section (abfd, s, &dynbuf))
	{
	error_free_dyn:
	  free (dynbuf);
	  goto error_return;
	}

      for (extdyn = dynbuf;
	   (size_t) (dynbuf + s->size - extdyn) >= bed->s->sizeof_dyn;
	   extdyn += bed->s->sizeof_dyn)
	{
	  Elf_Internal_Dyn dyn;

	  bed->s->swap_dyn_in (abfd, extdyn, &dyn);
	  if (dyn.d_tag == DT_IA_64_VMS_IDENT)
	    {
	      uint64_t tagv = dyn.d_un.d_val;
	      elf_ia64_vms_ident (abfd) = tagv;
	      break;
	    }
	}
      if (extdyn >= dynbuf + s->size)
	{
	  /* Ident not found.  */
	  goto error_free_dyn;
	}
      free (dynbuf);

      /* We do not want to include any of the sections in a dynamic
	 object in the output file.  We hack by simply clobbering the
	 list of sections in the BFD.  This could be handled more
	 cleanly by, say, a new section flag; the existing
	 SEC_NEVER_LOAD flag is not the one we want, because that one
	 still implies that the section takes up space in the output
	 file.  */
      bfd_section_list_clear (abfd);

      /* FIXME: should we detect if this library is already included ?
	 This should be harmless and shouldn't happen in practice.  */
    }

  hdr = &elf_tdata (abfd)->symtab_hdr;
  symcount = hdr->sh_size / bed->s->sizeof_sym;

  /* The sh_info field of the symtab header tells us where the
     external symbols start.  We don't care about the local symbols at
     this point.  */
  extsymcount = symcount - hdr->sh_info;
  extsymoff = hdr->sh_info;

  sym_hash = NULL;
  if (extsymcount != 0)
    {
      isymbuf = bfd_elf_get_elf_syms (abfd, hdr, extsymcount, extsymoff,
				      NULL, NULL, NULL);
      if (isymbuf == NULL)
	goto error_return;

      /* We store a pointer to the hash table entry for each external
	 symbol.  */
      amt = extsymcount * sizeof (struct elf_link_hash_entry *);
      sym_hash = (struct elf_link_hash_entry **) bfd_alloc (abfd, amt);
      if (sym_hash == NULL)
	goto error_free_sym;
      elf_sym_hashes (abfd) = sym_hash;
    }

  for (isym = isymbuf, isymend = isymbuf + extsymcount;
       isym < isymend;
       isym++, sym_hash++)
    {
      int bind;
      bfd_vma value;
      asection *sec, *new_sec;
      flagword flags;
      const char *name;
      struct elf_link_hash_entry *h;
      bool definition;
      bool size_change_ok;
      bool type_change_ok;
      bool common;
      unsigned int old_alignment;
      bfd *old_bfd;

      flags = BSF_NO_FLAGS;
      sec = NULL;
      value = isym->st_value;
      *sym_hash = NULL;
      common = bed->common_definition (isym);

      bind = ELF_ST_BIND (isym->st_info);
      switch (bind)
	{
	case STB_LOCAL:
	  /* This should be impossible, since ELF requires that all
	     global symbols follow all local symbols, and that sh_info
	     point to the first global symbol.  Unfortunately, Irix 5
	     screws this up.  */
	  continue;

	case STB_GLOBAL:
	  if (isym->st_shndx != SHN_UNDEF && !common)
	    flags = BSF_GLOBAL;
	  break;

	case STB_WEAK:
	  flags = BSF_WEAK;
	  break;

	case STB_GNU_UNIQUE:
	  flags = BSF_GNU_UNIQUE;
	  break;

	default:
	  /* Leave it up to the processor backend.  */
	  break;
	}

      if (isym->st_shndx == SHN_UNDEF)
	sec = bfd_und_section_ptr;
      else if (isym->st_shndx == SHN_ABS)
	sec = bfd_abs_section_ptr;
      else if (isym->st_shndx == SHN_COMMON)
	{
	  sec = bfd_com_section_ptr;
	  /* What ELF calls the size we call the value.  What ELF
	     calls the value we call the alignment.  */
	  value = isym->st_size;
	}
      else
	{
	  sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
	  if (sec == NULL)
	    sec = bfd_abs_section_ptr;
	  else if (sec->kept_section)
	    {
	      /* Symbols from discarded section are undefined.  We keep
		 its visibility.  */
	      sec = bfd_und_section_ptr;
	      isym->st_shndx = SHN_UNDEF;
	    }
	  else if ((abfd->flags & (EXEC_P | DYNAMIC)) != 0)
	    value -= sec->vma;
	}

      name = bfd_elf_string_from_elf_section (abfd, hdr->sh_link,
					      isym->st_name);
      if (name == NULL)
	goto error_free_vers;

      if (bed->elf_add_symbol_hook)
	{
	  if (! (*bed->elf_add_symbol_hook) (abfd, info, isym, &name, &flags,
					     &sec, &value))
	    goto error_free_vers;

	  /* The hook function sets the name to NULL if this symbol
	     should be skipped for some reason.  */
	  if (name == NULL)
	    continue;
	}

      /* Sanity check that all possibilities were handled.  */
      if (sec == NULL)
	{
	  bfd_set_error (bfd_error_bad_value);
	  goto error_free_vers;
	}

      if (bfd_is_und_section (sec)
	  || bfd_is_com_section (sec))
	definition = false;
      else
	definition = true;

      size_change_ok = false;
      type_change_ok = bed->type_change_ok;
      old_alignment = 0;
      old_bfd = NULL;
      new_sec = sec;

      if (! bfd_is_und_section (sec))
	h = elf_link_hash_lookup (htab, name, true, false, false);
      else
	h = ((struct elf_link_hash_entry *) bfd_wrapped_link_hash_lookup
	     (abfd, info, name, true, false, false));
      if (h == NULL)
	goto error_free_sym;

      *sym_hash = h;

      if (is_elf_hash_table (&htab->root))
	{
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  /* Remember the old alignment if this is a common symbol, so
	     that we don't reduce the alignment later on.  We can't
	     check later, because _bfd_generic_link_add_one_symbol
	     will set a default for the alignment which we want to
	     override. We also remember the old bfd where the existing
	     definition comes from.  */
	  switch (h->root.type)
	    {
	    default:
	      break;

	    case bfd_link_hash_defined:
	      if (abfd->selective_search)
		continue;
	      /* Fall-through.  */
	    case bfd_link_hash_defweak:
	      old_bfd = h->root.u.def.section->owner;
	      break;

	    case bfd_link_hash_common:
	      old_bfd = h->root.u.c.p->section->owner;
	      old_alignment = h->root.u.c.p->alignment_power;
	      break;
	    }
	}

      if (! (_bfd_generic_link_add_one_symbol
	     (info, abfd, name, flags, sec, value, NULL, false, bed->collect,
	      (struct bfd_link_hash_entry **) sym_hash)))
	goto error_free_vers;

      h = *sym_hash;
      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      *sym_hash = h;
      if (definition)
	h->unique_global = (flags & BSF_GNU_UNIQUE) != 0;

      /* Set the alignment of a common symbol.  */
      if ((common || bfd_is_com_section (sec))
	  && h->root.type == bfd_link_hash_common)
	{
	  unsigned int align;

	  if (common)
	    align = bfd_log2 (isym->st_value);
	  else
	    {
	      /* The new symbol is a common symbol in a shared object.
		 We need to get the alignment from the section.  */
	      align = new_sec->alignment_power;
	    }
	  if (align > old_alignment
	      /* Permit an alignment power of zero if an alignment of one
		 is specified and no other alignments have been specified.  */
	      || (isym->st_value == 1 && old_alignment == 0))
	    h->root.u.c.p->alignment_power = align;
	  else
	    h->root.u.c.p->alignment_power = old_alignment;
	}

      if (is_elf_hash_table (&htab->root))
	{
	  /* Check the alignment when a common symbol is involved. This
	     can change when a common symbol is overridden by a normal
	     definition or a common symbol is ignored due to the old
	     normal definition. We need to make sure the maximum
	     alignment is maintained.  */
	  if ((old_alignment || common)
	      && h->root.type != bfd_link_hash_common)
	    {
	      unsigned int common_align;
	      unsigned int normal_align;
	      unsigned int symbol_align;
	      bfd *normal_bfd;
	      bfd *common_bfd;

	      symbol_align = ffs (h->root.u.def.value) - 1;
	      if (h->root.u.def.section->owner != NULL
		  && (h->root.u.def.section->owner->flags & DYNAMIC) == 0)
		{
		  normal_align = h->root.u.def.section->alignment_power;
		  if (normal_align > symbol_align)
		    normal_align = symbol_align;
		}
	      else
		normal_align = symbol_align;

	      if (old_alignment)
		{
		  common_align = old_alignment;
		  common_bfd = old_bfd;
		  normal_bfd = abfd;
		}
	      else
		{
		  common_align = bfd_log2 (isym->st_value);
		  common_bfd = abfd;
		  normal_bfd = old_bfd;
		}

	      if (normal_align < common_align)
		{
		  /* PR binutils/2735 */
		  if (normal_bfd == NULL)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("warning: alignment %u of common symbol `%s' in %pB"
			 " is greater than the alignment (%u) of its section %pA"),
		       1 << common_align, name, common_bfd,
		       1 << normal_align, h->root.u.def.section);
		  else
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("warning: alignment %u of symbol `%s' in %pB"
			 " is smaller than %u in %pB"),
		       1 << normal_align, name, normal_bfd,
		       1 << common_align, common_bfd);
		}
	    }

	  /* Remember the symbol size if it isn't undefined.  */
	  if ((isym->st_size != 0 && isym->st_shndx != SHN_UNDEF)
	      && (definition || h->size == 0))
	    {
	      if (h->size != 0
		  && h->size != isym->st_size
		  && ! size_change_ok)
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("warning: size of symbol `%s' changed"
		     " from %" PRIu64 " in %pB to %" PRIu64 " in %pB"),
		   name, (uint64_t) h->size, old_bfd,
		   (uint64_t) isym->st_size, abfd);

	      h->size = isym->st_size;
	    }

	  /* If this is a common symbol, then we always want H->SIZE
	     to be the size of the common symbol.  The code just above
	     won't fix the size if a common symbol becomes larger.  We
	     don't warn about a size change here, because that is
	     covered by --warn-common.  Allow changed between different
	     function types.  */
	  if (h->root.type == bfd_link_hash_common)
	    h->size = h->root.u.c.size;

	  if (ELF_ST_TYPE (isym->st_info) != STT_NOTYPE
	      && (definition || h->type == STT_NOTYPE))
	    {
	      unsigned int type = ELF_ST_TYPE (isym->st_info);

	      if (h->type != type)
		{
		  if (h->type != STT_NOTYPE && ! type_change_ok)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("warning: type of symbol `%s' changed"
			 " from %d to %d in %pB"),
		       name, h->type, type, abfd);

		  h->type = type;
		}
	    }

	  /* Set a flag in the hash table entry indicating the type of
	     reference or definition we just found.  Keep a count of
	     the number of dynamic symbols we find.  A dynamic symbol
	     is one which is referenced or defined by both a regular
	     object and a shared object.  */
	  if (! dynamic)
	    {
	      if (! definition)
		{
		  h->ref_regular = 1;
		  if (bind != STB_WEAK)
		    h->ref_regular_nonweak = 1;
		}
	      else
		{
		  BFD_ASSERT (!h->def_dynamic);
		  h->def_regular = 1;
		}
	    }
	  else
	    {
	      BFD_ASSERT (definition);
	      h->def_dynamic = 1;
	      h->dynindx = -2;
	      ((struct elf64_ia64_link_hash_entry *)h)->shl = abfd;
	    }
	}
    }

  free (isymbuf);
  isymbuf = NULL;

  /* If this object is the same format as the output object, and it is
     not a shared library, then let the backend look through the
     relocs.

     This is required to build global offset table entries and to
     arrange for dynamic relocs.  It is not required for the
     particular common case of linking non PIC code, even when linking
     against shared libraries, but unfortunately there is no way of
     knowing whether an object file has been compiled PIC or not.
     Looking through the relocs is not particularly time consuming.
     The problem is that we must either (1) keep the relocs in memory,
     which causes the linker to require additional runtime memory or
     (2) read the relocs twice from the input file, which wastes time.
     This would be a good case for using mmap.

     I have no idea how to handle linking PIC code into a file of a
     different format.  It probably can't be done.  */
  if (! dynamic
      && is_elf_hash_table (&htab->root)
      && bed->check_relocs != NULL
      && (*bed->relocs_compatible) (abfd->xvec, info->output_bfd->xvec))
    {
      asection *o;

      for (o = abfd->sections; o != NULL; o = o->next)
	{
	  Elf_Internal_Rela *internal_relocs;
	  bool ok;

	  if ((o->flags & SEC_RELOC) == 0
	      || o->reloc_count == 0
	      || ((info->strip == strip_all || info->strip == strip_debugger)
		  && (o->flags & SEC_DEBUGGING) != 0)
	      || bfd_is_abs_section (o->output_section))
	    continue;

	  internal_relocs = _bfd_elf_link_read_relocs (abfd, o, NULL, NULL,
						       info->keep_memory);
	  if (internal_relocs == NULL)
	    goto error_return;

	  ok = (*bed->check_relocs) (abfd, info, o, internal_relocs);

	  if (elf_section_data (o)->relocs != internal_relocs)
	    free (internal_relocs);

	  if (! ok)
	    goto error_return;
	}
    }

  return true;

 error_free_vers:
 error_free_sym:
  free (isymbuf);
 error_return:
  return false;
}

static bool
elf64_vms_link_add_archive_symbols (bfd *abfd, struct bfd_link_info *info)
{
  int pass;
  struct bfd_link_hash_entry **pundef;
  struct bfd_link_hash_entry **next_pundef;

  /* We only accept VMS libraries.  */
  if (info->output_bfd->xvec != abfd->xvec)
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  /* The archive_pass field in the archive itself is used to
     initialize PASS, since we may search the same archive multiple
     times.  */
  pass = ++abfd->archive_pass;

  /* Look through the list of undefined symbols.  */
  for (pundef = &info->hash->undefs; *pundef != NULL; pundef = next_pundef)
    {
      struct bfd_link_hash_entry *h;
      symindex symidx;
      bfd *element;
      bfd *orig_element;

      h = *pundef;
      next_pundef = &(*pundef)->u.undef.next;

      /* When a symbol is defined, it is not necessarily removed from
	 the list.  */
      if (h->type != bfd_link_hash_undefined
	  && h->type != bfd_link_hash_common)
	{
	  /* Remove this entry from the list, for general cleanliness
	     and because we are going to look through the list again
	     if we search any more libraries.  We can't remove the
	     entry if it is the tail, because that would lose any
	     entries we add to the list later on.  */
	  if (*pundef != info->hash->undefs_tail)
	    {
	      *pundef = *next_pundef;
	      next_pundef = pundef;
	    }
	  continue;
	}

      /* Look for this symbol in the archive hash table.  */
      symidx = _bfd_vms_lib_find_symbol (abfd, h->root.string);
      if (symidx == BFD_NO_MORE_SYMBOLS)
	{
	  /* Nothing in this slot.  */
	  continue;
	}

      element = bfd_get_elt_at_index (abfd, symidx);
      if (element == NULL)
	return false;

      if (element->archive_pass == -1 || element->archive_pass == pass)
	{
	  /* Next symbol if this archive is wrong or already handled.  */
	  continue;
	}

      orig_element = element;
      if (bfd_is_thin_archive (abfd))
	{
	  element = _bfd_vms_lib_get_imagelib_file (element);
	  if (element == NULL || !bfd_check_format (element, bfd_object))
	    {
	      orig_element->archive_pass = -1;
	      return false;
	    }
	}
      else if (! bfd_check_format (element, bfd_object))
	{
	  element->archive_pass = -1;
	  return false;
	}

      /* Unlike the generic linker, we know that this element provides
	 a definition for an undefined symbol and we know that we want
	 to include it.  We don't need to check anything.  */
      if (! (*info->callbacks->add_archive_element) (info, element,
						     h->root.string, &element))
	continue;
      if (! elf64_vms_link_add_object_symbols (element, info))
	return false;

      orig_element->archive_pass = pass;
    }

  return true;
}

static bool
elf64_vms_bfd_link_add_symbols (bfd *abfd, struct bfd_link_info *info)
{
  switch (bfd_get_format (abfd))
    {
    case bfd_object:
      return elf64_vms_link_add_object_symbols (abfd, info);
      break;
    case bfd_archive:
      return elf64_vms_link_add_archive_symbols (abfd, info);
      break;
    default:
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }
}

static bool
elf64_ia64_vms_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object
    (abfd, sizeof (struct elf64_ia64_vms_obj_tdata), IA64_ELF_DATA);
}


/* Size-dependent data and functions.  */
static const struct elf_size_info elf64_ia64_vms_size_info = {
  sizeof (Elf64_External_VMS_Ehdr),
  sizeof (Elf64_External_Phdr),
  sizeof (Elf64_External_Shdr),
  sizeof (Elf64_External_Rel),
  sizeof (Elf64_External_Rela),
  sizeof (Elf64_External_Sym),
  sizeof (Elf64_External_Dyn),
  sizeof (Elf_External_Note),
  4,
  1,
  64, 3, /* ARCH_SIZE, LOG_FILE_ALIGN */
  ELFCLASS64, EV_CURRENT,
  bfd_elf64_write_out_phdrs,
  elf64_vms_write_shdrs_and_ehdr,
  bfd_elf64_checksum_contents,
  bfd_elf64_write_relocs,
  bfd_elf64_swap_symbol_in,
  bfd_elf64_swap_symbol_out,
  bfd_elf64_slurp_reloc_table,
  bfd_elf64_slurp_symbol_table,
  bfd_elf64_swap_dyn_in,
  bfd_elf64_swap_dyn_out,
  bfd_elf64_swap_reloc_in,
  bfd_elf64_swap_reloc_out,
  bfd_elf64_swap_reloca_in,
  bfd_elf64_swap_reloca_out
};

#define ELF_ARCH			bfd_arch_ia64
#define ELF_MACHINE_CODE		EM_IA_64
#define ELF_MAXPAGESIZE			0x10000	/* 64KB */
#define ELF_COMMONPAGESIZE		0x200	/* 16KB */

#define elf_backend_section_from_shdr \
	elf64_ia64_section_from_shdr
#define elf_backend_section_flags \
	elf64_ia64_section_flags
#define elf_backend_fake_sections \
	elf64_ia64_fake_sections
#define elf_backend_final_write_processing \
	elf64_ia64_final_write_processing
#define elf_backend_add_symbol_hook \
	elf64_ia64_add_symbol_hook
#define elf_info_to_howto \
	elf64_ia64_info_to_howto

#define bfd_elf64_bfd_reloc_type_lookup \
	ia64_elf_reloc_type_lookup
#define bfd_elf64_bfd_reloc_name_lookup \
	ia64_elf_reloc_name_lookup
#define bfd_elf64_bfd_is_local_label_name \
	elf64_ia64_is_local_label_name
#define bfd_elf64_bfd_relax_section \
	elf64_ia64_relax_section

#define elf_backend_object_p \
	elf64_ia64_object_p

/* Stuff for the BFD linker: */
#define bfd_elf64_bfd_link_hash_table_create \
	elf64_ia64_hash_table_create
#define elf_backend_create_dynamic_sections \
	elf64_ia64_create_dynamic_sections
#define elf_backend_check_relocs \
	elf64_ia64_check_relocs
#define elf_backend_adjust_dynamic_symbol \
	elf64_ia64_adjust_dynamic_symbol
#define elf_backend_size_dynamic_sections \
	elf64_ia64_size_dynamic_sections
#define elf_backend_omit_section_dynsym \
	_bfd_elf_omit_section_dynsym_all
#define elf_backend_relocate_section \
	elf64_ia64_relocate_section
#define elf_backend_finish_dynamic_symbol \
	elf64_ia64_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
	elf64_ia64_finish_dynamic_sections
#define bfd_elf64_bfd_final_link \
	elf64_ia64_final_link

#define bfd_elf64_bfd_merge_private_bfd_data \
	elf64_ia64_merge_private_bfd_data
#define bfd_elf64_bfd_set_private_flags \
	elf64_ia64_set_private_flags
#define bfd_elf64_bfd_print_private_bfd_data \
	elf64_ia64_print_private_bfd_data

#define elf_backend_plt_readonly	1
#define elf_backend_want_plt_sym	0
#define elf_backend_plt_alignment	5
#define elf_backend_got_header_size	0
#define elf_backend_want_got_plt	1
#define elf_backend_may_use_rel_p	1
#define elf_backend_may_use_rela_p	1
#define elf_backend_default_use_rela_p	1
#define elf_backend_want_dynbss		0
#define elf_backend_hide_symbol		elf64_ia64_hash_hide_symbol
#define elf_backend_fixup_symbol	_bfd_elf_link_hash_fixup_symbol
#define elf_backend_reloc_type_class	elf64_ia64_reloc_type_class
#define elf_backend_rela_normal		1
#define elf_backend_special_sections	elf64_ia64_special_sections
#define elf_backend_default_execstack	0

/* FIXME: PR 290: The Intel C compiler generates SHT_IA_64_UNWIND with
   SHF_LINK_ORDER. But it doesn't set the sh_link or sh_info fields.
   We don't want to flood users with so many error messages. We turn
   off the warning for now. It will be turned on later when the Intel
   compiler is fixed.   */
#define elf_backend_link_order_error_handler NULL

/* VMS-specific vectors.  */

#undef  TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		ia64_elf64_vms_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		"elf64-ia64-vms"
#undef  TARGET_BIG_SYM
#undef  TARGET_BIG_NAME

/* These are VMS specific functions.  */

#undef  elf_backend_object_p
#define elf_backend_object_p elf64_vms_object_p

#undef  elf_backend_section_from_shdr
#define elf_backend_section_from_shdr elf64_vms_section_from_shdr

#undef  elf_backend_init_file_header
#define elf_backend_init_file_header elf64_vms_init_file_header

#undef  elf_backend_section_processing
#define elf_backend_section_processing elf64_vms_section_processing

#undef  elf_backend_final_write_processing
#define elf_backend_final_write_processing elf64_vms_final_write_processing

#undef  bfd_elf64_close_and_cleanup
#define bfd_elf64_close_and_cleanup elf64_vms_close_and_cleanup

#undef  elf_backend_section_from_bfd_section

#undef  elf_backend_symbol_processing

#undef  elf_backend_want_p_paddr_set_to_zero

#undef  ELF_OSABI
#define ELF_OSABI			ELFOSABI_OPENVMS

#undef  ELF_MAXPAGESIZE
#define ELF_MAXPAGESIZE			0x10000	/* 64KB */

#undef  elf64_bed
#define elf64_bed elf64_ia64_vms_bed

#define elf_backend_size_info elf64_ia64_vms_size_info

/* Use VMS-style archives (in particular, don't use the standard coff
   archive format).  */
#define bfd_elf64_archive_functions

#undef bfd_elf64_archive_p
#define bfd_elf64_archive_p _bfd_vms_lib_ia64_archive_p
#undef bfd_elf64_write_archive_contents
#define bfd_elf64_write_archive_contents _bfd_vms_lib_write_archive_contents
#undef bfd_elf64_mkarchive
#define bfd_elf64_mkarchive _bfd_vms_lib_ia64_mkarchive

#define bfd_elf64_archive_slurp_armap \
  _bfd_vms_lib_slurp_armap
#define bfd_elf64_archive_slurp_extended_name_table \
  _bfd_vms_lib_slurp_extended_name_table
#define bfd_elf64_archive_construct_extended_name_table \
  _bfd_vms_lib_construct_extended_name_table
#define bfd_elf64_archive_truncate_arname \
  _bfd_vms_lib_truncate_arname
#define bfd_elf64_archive_write_armap \
  _bfd_vms_lib_write_armap
#define bfd_elf64_archive_read_ar_hdr \
  _bfd_vms_lib_read_ar_hdr
#define bfd_elf64_archive_write_ar_hdr \
  _bfd_vms_lib_write_ar_hdr
#define bfd_elf64_archive_openr_next_archived_file \
  _bfd_vms_lib_openr_next_archived_file
#define bfd_elf64_archive_get_elt_at_index \
  _bfd_vms_lib_get_elt_at_index
#define bfd_elf64_archive_generic_stat_arch_elt \
  _bfd_vms_lib_generic_stat_arch_elt
#define bfd_elf64_archive_update_armap_timestamp \
  _bfd_vms_lib_update_armap_timestamp

/* VMS link methods.  */
#undef  bfd_elf64_bfd_link_add_symbols
#define bfd_elf64_bfd_link_add_symbols	elf64_vms_bfd_link_add_symbols

#undef  elf_backend_want_got_sym
#define elf_backend_want_got_sym	0

#undef  bfd_elf64_mkobject
#define bfd_elf64_mkobject		elf64_ia64_vms_mkobject

/* Redefine to align segments on block size.  */
#undef  ELF_MAXPAGESIZE
#define ELF_MAXPAGESIZE			0x200 /* 512B  */

#undef  elf_backend_want_got_plt
#define elf_backend_want_got_plt	0

#include "elf64-target.h"
