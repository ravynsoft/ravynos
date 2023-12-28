/* Support for the generic parts of PE/PEI, for BFD.
   Copyright (C) 1995-2023 Free Software Foundation, Inc.
   Written by Cygnus Solutions.

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


/* Most of this hacked by  Steve Chamberlain,
			sac@cygnus.com

   PE/PEI rearrangement (and code added): Donn Terry
				       Softway Systems, Inc.  */

/* Hey look, some documentation [and in a place you expect to find it]!

   The main reference for the pei format is "Microsoft Portable Executable
   and Common Object File Format Specification 4.1".  Get it if you need to
   do some serious hacking on this code.

   Another reference:
   "Peering Inside the PE: A Tour of the Win32 Portable Executable
   File Format", MSJ 1994, Volume 9.

   The *sole* difference between the pe format and the pei format is that the
   latter has an MSDOS 2.0 .exe header on the front that prints the message
   "This app must be run under Windows." (or some such).
   (FIXME: Whether that statement is *really* true or not is unknown.
   Are there more subtle differences between pe and pei formats?
   For now assume there aren't.  If you find one, then for God sakes
   document it here!)

   The Microsoft docs use the word "image" instead of "executable" because
   the former can also refer to a DLL (shared library).  Confusion can arise
   because the `i' in `pei' also refers to "image".  The `pe' format can
   also create images (i.e. executables), it's just that to run on a win32
   system you need to use the pei format.

   FIXME: Please add more docs here so the next poor fool that has to hack
   on this code has a chance of getting something accomplished without
   wasting too much time.  */

#include "libpei.h"

static bool (*pe_saved_coff_bfd_print_private_bfd_data) (bfd *, void *) =
#ifndef coff_bfd_print_private_bfd_data
     NULL;
#else
     coff_bfd_print_private_bfd_data;
#undef coff_bfd_print_private_bfd_data
#endif

static bool pe_print_private_bfd_data (bfd *, void *);
#define coff_bfd_print_private_bfd_data pe_print_private_bfd_data

static bool (*pe_saved_coff_bfd_copy_private_bfd_data) (bfd *, bfd *) =
#ifndef coff_bfd_copy_private_bfd_data
     NULL;
#else
     coff_bfd_copy_private_bfd_data;
#undef coff_bfd_copy_private_bfd_data
#endif

static bool pe_bfd_copy_private_bfd_data (bfd *, bfd *);
#define coff_bfd_copy_private_bfd_data pe_bfd_copy_private_bfd_data

#define coff_mkobject	   pe_mkobject
#define coff_mkobject_hook pe_mkobject_hook

#ifdef COFF_IMAGE_WITH_PE
/* This structure contains static variables used by the ILF code.  */
typedef asection * asection_ptr;

typedef struct
{
  bfd *			abfd;
  bfd_byte *		data;
  struct bfd_in_memory * bim;
  unsigned short	magic;

  arelent *		reltab;
  unsigned int		relcount;

  coff_symbol_type *	sym_cache;
  coff_symbol_type *	sym_ptr;
  unsigned int		sym_index;

  unsigned int *	sym_table;
  unsigned int *	table_ptr;

  combined_entry_type * native_syms;
  combined_entry_type * native_ptr;

  coff_symbol_type **	sym_ptr_table;
  coff_symbol_type **	sym_ptr_ptr;

  unsigned int		sec_index;

  char *		string_table;
  char *		string_ptr;
  char *		end_string_ptr;

  SYMENT *		esym_table;
  SYMENT *		esym_ptr;

  struct internal_reloc * int_reltab;
}
pe_ILF_vars;
#endif /* COFF_IMAGE_WITH_PE */

bfd_cleanup coff_real_object_p
  (bfd *, unsigned, struct internal_filehdr *, struct internal_aouthdr *);

#ifndef NO_COFF_RELOCS
static void
coff_swap_reloc_in (bfd * abfd, void * src, void * dst)
{
  RELOC *reloc_src = (RELOC *) src;
  struct internal_reloc *reloc_dst = (struct internal_reloc *) dst;

  reloc_dst->r_vaddr  = H_GET_32 (abfd, reloc_src->r_vaddr);
  reloc_dst->r_symndx = H_GET_S32 (abfd, reloc_src->r_symndx);
  reloc_dst->r_type   = H_GET_16 (abfd, reloc_src->r_type);
#ifdef SWAP_IN_RELOC_OFFSET
  reloc_dst->r_offset = SWAP_IN_RELOC_OFFSET (abfd, reloc_src->r_offset);
#endif
}

static unsigned int
coff_swap_reloc_out (bfd * abfd, void * src, void * dst)
{
  struct internal_reloc *reloc_src = (struct internal_reloc *) src;
  struct external_reloc *reloc_dst = (struct external_reloc *) dst;

  H_PUT_32 (abfd, reloc_src->r_vaddr, reloc_dst->r_vaddr);
  H_PUT_32 (abfd, reloc_src->r_symndx, reloc_dst->r_symndx);
  H_PUT_16 (abfd, reloc_src->r_type, reloc_dst->r_type);

#ifdef SWAP_OUT_RELOC_OFFSET
  SWAP_OUT_RELOC_OFFSET (abfd, reloc_src->r_offset, reloc_dst->r_offset);
#endif
#ifdef SWAP_OUT_RELOC_EXTRA
  SWAP_OUT_RELOC_EXTRA (abfd, reloc_src, reloc_dst);
#endif
  return RELSZ;
}
#endif /* not NO_COFF_RELOCS */

#ifdef COFF_IMAGE_WITH_PE
#undef FILHDR
#define FILHDR struct external_PEI_IMAGE_hdr
#endif

static void
coff_swap_filehdr_in (bfd * abfd, void * src, void * dst)
{
  FILHDR *filehdr_src = (FILHDR *) src;
  struct internal_filehdr *filehdr_dst = (struct internal_filehdr *) dst;

  filehdr_dst->f_magic  = H_GET_16 (abfd, filehdr_src->f_magic);
  filehdr_dst->f_nscns  = H_GET_16 (abfd, filehdr_src->f_nscns);
  filehdr_dst->f_timdat = H_GET_32 (abfd, filehdr_src->f_timdat);
  filehdr_dst->f_nsyms  = H_GET_32 (abfd, filehdr_src->f_nsyms);
  filehdr_dst->f_flags  = H_GET_16 (abfd, filehdr_src->f_flags);
  filehdr_dst->f_symptr = H_GET_32 (abfd, filehdr_src->f_symptr);

  /* Other people's tools sometimes generate headers with an nsyms but
     a zero symptr.  */
  if (filehdr_dst->f_nsyms != 0 && filehdr_dst->f_symptr == 0)
    {
      filehdr_dst->f_nsyms = 0;
      filehdr_dst->f_flags |= F_LSYMS;
    }

  filehdr_dst->f_opthdr = H_GET_16 (abfd, filehdr_src-> f_opthdr);
}

#ifdef COFF_IMAGE_WITH_PE
# define coff_swap_filehdr_out _bfd_XXi_only_swap_filehdr_out
#elif defined COFF_WITH_peAArch64
# define coff_swap_filehdr_out _bfd_XX_only_swap_filehdr_out
#elif defined COFF_WITH_pex64
# define coff_swap_filehdr_out _bfd_pex64_only_swap_filehdr_out
#elif defined COFF_WITH_pep
# define coff_swap_filehdr_out _bfd_pep_only_swap_filehdr_out
#else
# define coff_swap_filehdr_out _bfd_pe_only_swap_filehdr_out
#endif

static void
coff_swap_scnhdr_in (bfd * abfd, void * ext, void * in)
{
  SCNHDR *scnhdr_ext = (SCNHDR *) ext;
  struct internal_scnhdr *scnhdr_int = (struct internal_scnhdr *) in;

  memcpy (scnhdr_int->s_name, scnhdr_ext->s_name, sizeof (scnhdr_int->s_name));

  scnhdr_int->s_vaddr   = GET_SCNHDR_VADDR (abfd, scnhdr_ext->s_vaddr);
  scnhdr_int->s_paddr   = GET_SCNHDR_PADDR (abfd, scnhdr_ext->s_paddr);
  scnhdr_int->s_size    = GET_SCNHDR_SIZE (abfd, scnhdr_ext->s_size);
  scnhdr_int->s_scnptr  = GET_SCNHDR_SCNPTR (abfd, scnhdr_ext->s_scnptr);
  scnhdr_int->s_relptr  = GET_SCNHDR_RELPTR (abfd, scnhdr_ext->s_relptr);
  scnhdr_int->s_lnnoptr = GET_SCNHDR_LNNOPTR (abfd, scnhdr_ext->s_lnnoptr);
  scnhdr_int->s_flags   = H_GET_32 (abfd, scnhdr_ext->s_flags);

  /* MS handles overflow of line numbers by carrying into the reloc
     field (it appears).  Since it's supposed to be zero for PE
     *IMAGE* format, that's safe.  This is still a bit iffy.  */
#ifdef COFF_IMAGE_WITH_PE
  scnhdr_int->s_nlnno = (H_GET_16 (abfd, scnhdr_ext->s_nlnno)
			 + (H_GET_16 (abfd, scnhdr_ext->s_nreloc) << 16));
  scnhdr_int->s_nreloc = 0;
#else
  scnhdr_int->s_nreloc = H_GET_16 (abfd, scnhdr_ext->s_nreloc);
  scnhdr_int->s_nlnno = H_GET_16 (abfd, scnhdr_ext->s_nlnno);
#endif

  if (scnhdr_int->s_vaddr != 0)
    {
      scnhdr_int->s_vaddr += pe_data (abfd)->pe_opthdr.ImageBase;
      /* Do not cut upper 32-bits for 64-bit vma.  */
#if !defined(COFF_WITH_pex64) && !defined(COFF_WITH_peAArch64) && !defined(COFF_WITH_peLoongArch64)
      scnhdr_int->s_vaddr &= 0xffffffff;
#endif
    }

#ifndef COFF_NO_HACK_SCNHDR_SIZE
  /* If this section holds uninitialized data and is from an object file
     or from an executable image that has not initialized the field,
     or if the image is an executable file and the physical size is padded,
     use the virtual size (stored in s_paddr) instead.  */
  if (scnhdr_int->s_paddr > 0
      && (((scnhdr_int->s_flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) != 0
	   && (! bfd_pei_p (abfd) || scnhdr_int->s_size == 0))
	  || (bfd_pei_p (abfd) && (scnhdr_int->s_size > scnhdr_int->s_paddr))))
  /* This code used to set scnhdr_int->s_paddr to 0.  However,
     coff_set_alignment_hook stores s_paddr in virt_size, which
     only works if it correctly holds the virtual size of the
     section.  */
    scnhdr_int->s_size = scnhdr_int->s_paddr;
#endif
}

static bool
pe_mkobject (bfd * abfd)
{
  pe_data_type *pe;
  size_t amt = sizeof (pe_data_type);

  abfd->tdata.pe_obj_data = (struct pe_tdata *) bfd_zalloc (abfd, amt);

  if (abfd->tdata.pe_obj_data == 0)
    return false;

  pe = pe_data (abfd);

  pe->coff.pe = 1;

  /* in_reloc_p is architecture dependent.  */
  pe->in_reloc_p = in_reloc_p;

  /* Default DOS message string.  */
  pe->dos_message[0]  = 0x0eba1f0e;
  pe->dos_message[1]  = 0xcd09b400;
  pe->dos_message[2]  = 0x4c01b821;
  pe->dos_message[3]  = 0x685421cd;
  pe->dos_message[4]  = 0x70207369;
  pe->dos_message[5]  = 0x72676f72;
  pe->dos_message[6]  = 0x63206d61;
  pe->dos_message[7]  = 0x6f6e6e61;
  pe->dos_message[8]  = 0x65622074;
  pe->dos_message[9]  = 0x6e757220;
  pe->dos_message[10] = 0x206e6920;
  pe->dos_message[11] = 0x20534f44;
  pe->dos_message[12] = 0x65646f6d;
  pe->dos_message[13] = 0x0a0d0d2e;
  pe->dos_message[14] = 0x24;
  pe->dos_message[15] = 0x0;

  memset (& pe->pe_opthdr, 0, sizeof pe->pe_opthdr);

  bfd_coff_long_section_names (abfd)
    = coff_backend_info (abfd)->_bfd_coff_long_section_names;

  return true;
}

/* Create the COFF backend specific information.  */

static void *
pe_mkobject_hook (bfd * abfd,
		  void * filehdr,
		  void * aouthdr ATTRIBUTE_UNUSED)
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;
  pe_data_type *pe;

  if (! pe_mkobject (abfd))
    return NULL;

  pe = pe_data (abfd);
  pe->coff.sym_filepos = internal_f->f_symptr;
  /* These members communicate important constants about the symbol
     table to GDB's symbol-reading code.  These `constants'
     unfortunately vary among coff implementations...  */
  pe->coff.local_n_btmask = N_BTMASK;
  pe->coff.local_n_btshft = N_BTSHFT;
  pe->coff.local_n_tmask = N_TMASK;
  pe->coff.local_n_tshift = N_TSHIFT;
  pe->coff.local_symesz = SYMESZ;
  pe->coff.local_auxesz = AUXESZ;
  pe->coff.local_linesz = LINESZ;

  pe->coff.timestamp = internal_f->f_timdat;

  obj_raw_syment_count (abfd) =
    obj_conv_table_size (abfd) =
      internal_f->f_nsyms;

  pe->real_flags = internal_f->f_flags;

  if ((internal_f->f_flags & F_DLL) != 0)
    pe->dll = 1;

  if ((internal_f->f_flags & IMAGE_FILE_DEBUG_STRIPPED) == 0)
    abfd->flags |= HAS_DEBUG;

#ifdef COFF_IMAGE_WITH_PE
  if (aouthdr)
    pe->pe_opthdr = ((struct internal_aouthdr *) aouthdr)->pe;
#endif

#ifdef ARM
  if (! _bfd_coff_arm_set_private_flags (abfd, internal_f->f_flags))
    coff_data (abfd) ->flags = 0;
#endif

  memcpy (pe->dos_message, internal_f->pe.dos_message,
	  sizeof (pe->dos_message));

  return (void *) pe;
}

static bool
pe_print_private_bfd_data (bfd *abfd, void * vfile)
{
  FILE *file = (FILE *) vfile;

  if (!_bfd_XX_print_private_bfd_data_common (abfd, vfile))
    return false;

  if (pe_saved_coff_bfd_print_private_bfd_data == NULL)
    return true;

  fputc ('\n', file);

  return pe_saved_coff_bfd_print_private_bfd_data (abfd, vfile);
}

/* Copy any private info we understand from the input bfd
   to the output bfd.  */

static bool
pe_bfd_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  /* PR binutils/716: Copy the large address aware flag.
     XXX: Should we be copying other flags or other fields in the pe_data()
     structure ?  */
  if (pe_data (obfd) != NULL
      && pe_data (ibfd) != NULL
      && pe_data (ibfd)->real_flags & IMAGE_FILE_LARGE_ADDRESS_AWARE)
    pe_data (obfd)->real_flags |= IMAGE_FILE_LARGE_ADDRESS_AWARE;

  if (!_bfd_XX_bfd_copy_private_bfd_data_common (ibfd, obfd))
    return false;

  if (pe_saved_coff_bfd_copy_private_bfd_data)
    return pe_saved_coff_bfd_copy_private_bfd_data (ibfd, obfd);

  return true;
}

#define coff_bfd_copy_private_section_data \
  _bfd_XX_bfd_copy_private_section_data

#define coff_get_symbol_info _bfd_XX_get_symbol_info

#ifdef COFF_IMAGE_WITH_PE

/* Code to handle Microsoft's Import Library Format.
   Also known as LINK6 format.
   Documentation about this format can be found at:

   https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#import-library-format  */

/* The following constants specify the sizes of the various data
   structures that we have to create in order to build a bfd describing
   an ILF object file.  The final "+ 1" in the definitions of SIZEOF_IDATA6
   and SIZEOF_IDATA7 below is to allow for the possibility that we might
   need a padding byte in order to ensure 16 bit alignment for the section's
   contents.

   The value for SIZEOF_ILF_STRINGS is computed as follows:

      There will be NUM_ILF_SECTIONS section symbols.  Allow 9 characters
      per symbol for their names (longest section name is .idata$x).

      There will be two symbols for the imported value, one the symbol name
      and one with _imp__ prefixed.  Allowing for the terminating nul's this
      is strlen (symbol_name) * 2 + 8 + 21 + strlen (source_dll).

      The strings in the string table must start STRING__SIZE_SIZE bytes into
      the table in order to for the string lookup code in coffgen/coffcode to
      work.  */
#define NUM_ILF_RELOCS		8
#define NUM_ILF_SECTIONS	6
#define NUM_ILF_SYMS		(2 + NUM_ILF_SECTIONS)

#define SIZEOF_ILF_SYMS		 (NUM_ILF_SYMS * sizeof (* vars.sym_cache))
#define SIZEOF_ILF_SYM_TABLE	 (NUM_ILF_SYMS * sizeof (* vars.sym_table))
#define SIZEOF_ILF_NATIVE_SYMS	 (NUM_ILF_SYMS * sizeof (* vars.native_syms))
#define SIZEOF_ILF_SYM_PTR_TABLE (NUM_ILF_SYMS * sizeof (* vars.sym_ptr_table))
#define SIZEOF_ILF_EXT_SYMS	 (NUM_ILF_SYMS * sizeof (* vars.esym_table))
#define SIZEOF_ILF_RELOCS	 (NUM_ILF_RELOCS * sizeof (* vars.reltab))
#define SIZEOF_ILF_INT_RELOCS	 (NUM_ILF_RELOCS * sizeof (* vars.int_reltab))
#define SIZEOF_ILF_STRINGS	 (strlen (symbol_name) * 2 + 8 \
					+ 21 + strlen (source_dll) \
					+ NUM_ILF_SECTIONS * 9 \
					+ STRING_SIZE_SIZE)
#define SIZEOF_IDATA2		(5 * 4)

/* For PEx64 idata4 & 5 have thumb size of 8 bytes.  */
#if defined(COFF_WITH_pex64) || defined(COFF_WITH_peAArch64)
#define SIZEOF_IDATA4		(2 * 4)
#define SIZEOF_IDATA5		(2 * 4)
#else
#define SIZEOF_IDATA4		(1 * 4)
#define SIZEOF_IDATA5		(1 * 4)
#endif

#define SIZEOF_IDATA6		(2 + strlen (symbol_name) + 1 + 1)
#define SIZEOF_IDATA7		(strlen (source_dll) + 1 + 1)
#define SIZEOF_ILF_SECTIONS	(NUM_ILF_SECTIONS * sizeof (struct coff_section_tdata))

#define ILF_DATA_SIZE				\
    + SIZEOF_ILF_SYMS				\
    + SIZEOF_ILF_SYM_TABLE			\
    + SIZEOF_ILF_NATIVE_SYMS			\
    + SIZEOF_ILF_SYM_PTR_TABLE			\
    + SIZEOF_ILF_EXT_SYMS			\
    + SIZEOF_ILF_RELOCS				\
    + SIZEOF_ILF_INT_RELOCS			\
    + SIZEOF_ILF_STRINGS			\
    + SIZEOF_IDATA2				\
    + SIZEOF_IDATA4				\
    + SIZEOF_IDATA5				\
    + SIZEOF_IDATA6				\
    + SIZEOF_IDATA7				\
    + SIZEOF_ILF_SECTIONS			\
    + MAX_TEXT_SECTION_SIZE

/* Create an empty relocation against the given symbol.  */

static void
pe_ILF_make_a_symbol_reloc (pe_ILF_vars *		vars,
			    bfd_vma			address,
			    bfd_reloc_code_real_type	reloc,
			    struct bfd_symbol **	sym,
			    unsigned int		sym_index)
{
  arelent * entry;
  struct internal_reloc * internal;

  entry = vars->reltab + vars->relcount;
  internal = vars->int_reltab + vars->relcount;

  entry->address     = address;
  entry->addend      = 0;
  entry->howto       = bfd_reloc_type_lookup (vars->abfd, reloc);
  entry->sym_ptr_ptr = sym;

  internal->r_vaddr  = address;
  internal->r_symndx = sym_index;
  internal->r_type   = entry->howto ? entry->howto->type : 0;

  vars->relcount ++;

  BFD_ASSERT (vars->relcount <= NUM_ILF_RELOCS);
}

/* Create an empty relocation against the given section.  */

static void
pe_ILF_make_a_reloc (pe_ILF_vars *	       vars,
		     bfd_vma		       address,
		     bfd_reloc_code_real_type  reloc,
		     asection_ptr	       sec)
{
  pe_ILF_make_a_symbol_reloc (vars, address, reloc, sec->symbol_ptr_ptr,
			      coff_section_data (vars->abfd, sec)->i);
}

/* Move the queued relocs into the given section.  */

static void
pe_ILF_save_relocs (pe_ILF_vars * vars,
		    asection_ptr  sec)
{
  /* Make sure that there is somewhere to store the internal relocs.  */
  if (coff_section_data (vars->abfd, sec) == NULL)
    /* We should probably return an error indication here.  */
    abort ();

  coff_section_data (vars->abfd, sec)->relocs = vars->int_reltab;

  sec->relocation  = vars->reltab;
  sec->reloc_count = vars->relcount;
  sec->flags      |= SEC_RELOC;

  vars->reltab     += vars->relcount;
  vars->int_reltab += vars->relcount;
  vars->relcount   = 0;

  BFD_ASSERT ((bfd_byte *) vars->int_reltab < (bfd_byte *) vars->string_table);
}

/* Create a global symbol and add it to the relevant tables.  */

static void
pe_ILF_make_a_symbol (pe_ILF_vars *  vars,
		      const char *   prefix,
		      const char *   symbol_name,
		      asection_ptr   section,
		      flagword       extra_flags)
{
  coff_symbol_type * sym;
  combined_entry_type * ent;
  SYMENT * esym;
  unsigned short sclass;

  if (extra_flags & BSF_LOCAL)
    sclass = C_STAT;
  else
    sclass = C_EXT;

#ifdef THUMBPEMAGIC
  if (vars->magic == THUMBPEMAGIC)
    {
      if (extra_flags & BSF_FUNCTION)
	sclass = C_THUMBEXTFUNC;
      else if (extra_flags & BSF_LOCAL)
	sclass = C_THUMBSTAT;
      else
	sclass = C_THUMBEXT;
    }
#endif

  BFD_ASSERT (vars->sym_index < NUM_ILF_SYMS);

  sym = vars->sym_ptr;
  ent = vars->native_ptr;
  esym = vars->esym_ptr;

  /* Copy the symbol's name into the string table.  */
  int len = sprintf (vars->string_ptr, "%s%s", prefix, symbol_name);

  if (section == NULL)
    section = bfd_und_section_ptr;

  /* Initialise the external symbol.  */
  H_PUT_32 (vars->abfd, vars->string_ptr - vars->string_table,
	    esym->e.e.e_offset);
  H_PUT_16 (vars->abfd, section->target_index, esym->e_scnum);
  esym->e_sclass[0] = sclass;

  /* The following initialisations are unnecessary - the memory is
     zero initialised.  They are just kept here as reminders.  */

  /* Initialise the internal symbol structure.  */
  ent->u.syment.n_sclass	  = sclass;
  ent->u.syment.n_scnum		  = section->target_index;
  ent->u.syment._n._n_n._n_offset = (uintptr_t) sym;
  ent->is_sym = true;

  sym->symbol.the_bfd = vars->abfd;
  sym->symbol.name    = vars->string_ptr;
  sym->symbol.flags   = BSF_EXPORT | BSF_GLOBAL | extra_flags;
  sym->symbol.section = section;
  sym->native	      = ent;

  * vars->table_ptr = vars->sym_index;
  * vars->sym_ptr_ptr = sym;

  /* Adjust pointers for the next symbol.  */
  vars->sym_index ++;
  vars->sym_ptr ++;
  vars->sym_ptr_ptr ++;
  vars->table_ptr ++;
  vars->native_ptr ++;
  vars->esym_ptr ++;
  vars->string_ptr += len + 1;

  BFD_ASSERT (vars->string_ptr < vars->end_string_ptr);
}

/* Create a section.  */

static asection_ptr
pe_ILF_make_a_section (pe_ILF_vars * vars,
		       const char *  name,
		       unsigned int  size,
		       flagword      extra_flags)
{
  asection_ptr sec;
  flagword     flags;
  intptr_t alignment;

  sec = bfd_make_section_old_way (vars->abfd, name);
  if (sec == NULL)
    return NULL;

  flags = SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_KEEP | SEC_IN_MEMORY;

  bfd_set_section_flags (sec, flags | extra_flags);

  bfd_set_section_alignment (sec, 2);

  /* Check that we will not run out of space.  */
  BFD_ASSERT (vars->data + size < vars->bim->buffer + vars->bim->size);

  /* Set the section size and contents.  The actual
     contents are filled in by our parent.  */
  bfd_set_section_size (sec, (bfd_size_type) size);
  sec->contents = vars->data;
  sec->target_index = vars->sec_index ++;

  /* Advance data pointer in the vars structure.  */
  vars->data += size;

  /* Skip the padding byte if it was not needed.
     The logic here is that if the string length is odd,
     then the entire string length, including the null byte,
     is even and so the extra, padding byte, is not needed.  */
  if (size & 1)
    vars->data --;

  /* PR 18758: See note in pe_ILF_buid_a_bfd.  We must make sure that we
     preserve host alignment requirements.  The BFD_ASSERTs in this
     functions will warn us if we run out of room, but we should
     already have enough padding built in to ILF_DATA_SIZE.  */
#if GCC_VERSION >= 3000
  alignment = __alignof__ (struct coff_section_tdata);
#else
  alignment = 8;
#endif
  vars->data
    = (bfd_byte *) (((intptr_t) vars->data + alignment - 1) & -alignment);

  /* Create a coff_section_tdata structure for our use.  */
  sec->used_by_bfd = (struct coff_section_tdata *) vars->data;
  vars->data += sizeof (struct coff_section_tdata);

  BFD_ASSERT (vars->data <= vars->bim->buffer + vars->bim->size);

  /* Create a symbol to refer to this section.  */
  pe_ILF_make_a_symbol (vars, "", name, sec, BSF_LOCAL);

  /* Cache the index to the symbol in the coff_section_data structure.  */
  coff_section_data (vars->abfd, sec)->i = vars->sym_index - 1;

  return sec;
}

/* This structure contains the code that goes into the .text section
   in order to perform a jump into the DLL lookup table.  The entries
   in the table are index by the magic number used to represent the
   machine type in the PE file.  The contents of the data[] arrays in
   these entries are stolen from the jtab[] arrays in ld/pe-dll.c.
   The SIZE field says how many bytes in the DATA array are actually
   used.  The OFFSET field says where in the data array the address
   of the .idata$5 section should be placed.  */
#define MAX_TEXT_SECTION_SIZE 32

typedef struct
{
  unsigned short magic;
  unsigned char  data[MAX_TEXT_SECTION_SIZE];
  unsigned int   size;
  unsigned int   offset;
}
jump_table;

static const jump_table jtab[] =
{
#ifdef I386MAGIC
  { I386MAGIC,
    { 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90 },
    8, 2
  },
#endif

#ifdef AMD64MAGIC
  { AMD64MAGIC,
    { 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90 },
    8, 2
  },
#endif

#ifdef  MC68MAGIC
  { MC68MAGIC,
    { /* XXX fill me in */ },
    0, 0
  },
#endif

#ifdef  MIPS_ARCH_MAGIC_WINCE
  { MIPS_ARCH_MAGIC_WINCE,
    { 0x00, 0x00, 0x08, 0x3c, 0x00, 0x00, 0x08, 0x8d,
      0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 },
    16, 0
  },
#endif

#ifdef  SH_ARCH_MAGIC_WINCE
  { SH_ARCH_MAGIC_WINCE,
    { 0x01, 0xd0, 0x02, 0x60, 0x2b, 0x40,
      0x09, 0x00, 0x00, 0x00, 0x00, 0x00 },
    12, 8
  },
#endif

#ifdef AARCH64MAGIC
/* We don't currently support jumping to DLLs, so if
   someone does try emit a runtime trap.  Through UDF #0.  */
  { AARCH64MAGIC,
    { 0x00, 0x00, 0x00, 0x00 },
    4, 0
  },

#endif

#ifdef  ARMPEMAGIC
  { ARMPEMAGIC,
    { 0x00, 0xc0, 0x9f, 0xe5, 0x00, 0xf0,
      0x9c, 0xe5, 0x00, 0x00, 0x00, 0x00},
    12, 8
  },
#endif

#ifdef  THUMBPEMAGIC
  { THUMBPEMAGIC,
    { 0x40, 0xb4, 0x02, 0x4e, 0x36, 0x68, 0xb4, 0x46,
      0x40, 0xbc, 0x60, 0x47, 0x00, 0x00, 0x00, 0x00 },
    16, 12
  },
#endif

#ifdef LOONGARCH64MAGIC
/* We don't currently support jumping to DLLs, so if
   someone does try emit a runtime trap.  Through BREAK 0.  */
  { LOONGARCH64MAGIC,
    { 0x00, 0x00, 0x2a, 0x00 },
    4, 0
  },

#endif

  { 0, { 0 }, 0, 0 }
};

#ifndef NUM_ENTRIES
#define NUM_ENTRIES(a) (sizeof (a) / sizeof (a)[0])
#endif

/* Build a full BFD from the information supplied in a ILF object.  */

static bool
pe_ILF_build_a_bfd (bfd *	    abfd,
		    unsigned int    magic,
		    char *	    symbol_name,
		    char *	    source_dll,
		    unsigned int    ordinal,
		    unsigned int    types)
{
  bfd_byte *		   ptr;
  pe_ILF_vars		   vars;
  struct internal_filehdr  internal_f;
  unsigned int		   import_type;
  unsigned int		   import_name_type;
  asection_ptr		   id4, id5, id6 = NULL, text = NULL;
  coff_symbol_type **	   imp_sym;
  unsigned int		   imp_index;
  intptr_t alignment;

  /* Decode and verify the types field of the ILF structure.  */
  import_type = types & 0x3;
  import_name_type = (types & 0x1c) >> 2;

  switch (import_type)
    {
    case IMPORT_CODE:
    case IMPORT_DATA:
      break;

    case IMPORT_CONST:
      /* XXX code yet to be written.  */
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unhandled import type; %x"),
			  abfd, import_type);
      return false;

    default:
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unrecognized import type; %x"),
			  abfd, import_type);
      return false;
    }

  switch (import_name_type)
    {
    case IMPORT_ORDINAL:
    case IMPORT_NAME:
    case IMPORT_NAME_NOPREFIX:
    case IMPORT_NAME_UNDECORATE:
      break;

    default:
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unrecognized import name type; %x"),
			  abfd, import_name_type);
      return false;
    }

  /* Initialise local variables.

     Note these are kept in a structure rather than being
     declared as statics since bfd frowns on global variables.

     We are going to construct the contents of the BFD in memory,
     so allocate all the space that we will need right now.  */
  vars.bim
    = (struct bfd_in_memory *) bfd_malloc ((bfd_size_type) sizeof (*vars.bim));
  if (vars.bim == NULL)
    return false;

  ptr = (bfd_byte *) bfd_zmalloc ((bfd_size_type) ILF_DATA_SIZE);
  vars.bim->buffer = ptr;
  vars.bim->size   = ILF_DATA_SIZE;
  if (ptr == NULL)
    goto error_return;

  /* Initialise the pointers to regions of the memory and the
     other contents of the pe_ILF_vars structure as well.  */
  vars.sym_cache = (coff_symbol_type *) ptr;
  vars.sym_ptr   = (coff_symbol_type *) ptr;
  vars.sym_index = 0;
  ptr += SIZEOF_ILF_SYMS;

  vars.sym_table = (unsigned int *) ptr;
  vars.table_ptr = (unsigned int *) ptr;
  ptr += SIZEOF_ILF_SYM_TABLE;

  vars.native_syms = (combined_entry_type *) ptr;
  vars.native_ptr  = (combined_entry_type *) ptr;
  ptr += SIZEOF_ILF_NATIVE_SYMS;

  vars.sym_ptr_table = (coff_symbol_type **) ptr;
  vars.sym_ptr_ptr   = (coff_symbol_type **) ptr;
  ptr += SIZEOF_ILF_SYM_PTR_TABLE;

  vars.esym_table = (SYMENT *) ptr;
  vars.esym_ptr   = (SYMENT *) ptr;
  ptr += SIZEOF_ILF_EXT_SYMS;

  vars.reltab   = (arelent *) ptr;
  vars.relcount = 0;
  ptr += SIZEOF_ILF_RELOCS;

  vars.int_reltab  = (struct internal_reloc *) ptr;
  ptr += SIZEOF_ILF_INT_RELOCS;

  vars.string_table = (char *) ptr;
  vars.string_ptr   = (char *) ptr + STRING_SIZE_SIZE;
  ptr += SIZEOF_ILF_STRINGS;
  vars.end_string_ptr = (char *) ptr;

  /* The remaining space in bim->buffer is used
     by the pe_ILF_make_a_section() function.  */

  /* PR 18758: Make sure that the data area is sufficiently aligned for
     struct coff_section_tdata.  __alignof__ is a gcc extension, hence
     the test of GCC_VERSION.  For other compilers we assume 8 byte
     alignment.  */
#if GCC_VERSION >= 3000
  alignment = __alignof__ (struct coff_section_tdata);
#else
  alignment = 8;
#endif
  ptr = (bfd_byte *) (((intptr_t) ptr + alignment - 1) & -alignment);

  vars.data = ptr;
  vars.abfd = abfd;
  vars.sec_index = 0;
  vars.magic = magic;

  /* Create the initial .idata$<n> sections:
     [.idata$2:  Import Directory Table -- not needed]
     .idata$4:  Import Lookup Table
     .idata$5:  Import Address Table

     Note we do not create a .idata$3 section as this is
     created for us by the linker script.  */
  id4 = pe_ILF_make_a_section (& vars, ".idata$4", SIZEOF_IDATA4, 0);
  id5 = pe_ILF_make_a_section (& vars, ".idata$5", SIZEOF_IDATA5, 0);
  if (id4 == NULL || id5 == NULL)
    goto error_return;

  /* Fill in the contents of these sections.  */
  if (import_name_type == IMPORT_ORDINAL)
    {
      if (ordinal == 0)
	/* See PR 20907 for a reproducer.  */
	goto error_return;

#if defined(COFF_WITH_pex64) || defined(COFF_WITH_peAArch64) || defined(COFF_WITH_peLoongArch64)
      ((unsigned int *) id4->contents)[0] = ordinal;
      ((unsigned int *) id4->contents)[1] = 0x80000000;
      ((unsigned int *) id5->contents)[0] = ordinal;
      ((unsigned int *) id5->contents)[1] = 0x80000000;
#else
      * (unsigned int *) id4->contents = ordinal | 0x80000000;
      * (unsigned int *) id5->contents = ordinal | 0x80000000;
#endif
    }
  else
    {
      char * symbol;
      unsigned int len;

      /* Create .idata$6 - the Hint Name Table.  */
      id6 = pe_ILF_make_a_section (& vars, ".idata$6", SIZEOF_IDATA6, 0);
      if (id6 == NULL)
	goto error_return;

      /* If necessary, trim the import symbol name.  */
      symbol = symbol_name;

      /* As used by MS compiler, '_', '@', and '?' are alternative
	 forms of USER_LABEL_PREFIX, with '?' for c++ mangled names,
	 '@' used for fastcall (in C),  '_' everywhere else.  Only one
	 of these is used for a symbol.  We strip this leading char for
	 IMPORT_NAME_NOPREFIX and IMPORT_NAME_UNDECORATE as per the
	 PE COFF 6.0 spec (section 8.3, Import Name Type).  */

      if (import_name_type != IMPORT_NAME)
	{
	  char c = symbol[0];

	  /* Check that we don't remove for targets with empty
	     USER_LABEL_PREFIX the leading underscore.  */
	  if ((c == '_' && abfd->xvec->symbol_leading_char != 0)
	      || c == '@' || c == '?')
	    symbol++;
	}

      len = strlen (symbol);
      if (import_name_type == IMPORT_NAME_UNDECORATE)
	{
	  /* Truncate at the first '@'.  */
	  char *at = strchr (symbol, '@');

	  if (at != NULL)
	    len = at - symbol;
	}

      id6->contents[0] = ordinal & 0xff;
      id6->contents[1] = ordinal >> 8;

      memcpy ((char *) id6->contents + 2, symbol, len);
      id6->contents[len + 2] = '\0';
    }

  if (import_name_type != IMPORT_ORDINAL)
    {
      pe_ILF_make_a_reloc (&vars, (bfd_vma) 0, BFD_RELOC_RVA, id6);
      pe_ILF_save_relocs (&vars, id4);

      pe_ILF_make_a_reloc (&vars, (bfd_vma) 0, BFD_RELOC_RVA, id6);
      pe_ILF_save_relocs (&vars, id5);
    }

  /* Create an import symbol.  */
  pe_ILF_make_a_symbol (& vars, "__imp_", symbol_name, id5, 0);
  imp_sym   = vars.sym_ptr_ptr - 1;
  imp_index = vars.sym_index - 1;

  /* Create extra sections depending upon the type of import we are dealing with.  */
  switch (import_type)
    {
      int i;

    case IMPORT_CODE:
      /* CODE functions are special, in that they get a trampoline that
	 jumps to the main import symbol.  Create a .text section to hold it.
	 First we need to look up its contents in the jump table.  */
      for (i = NUM_ENTRIES (jtab); i--;)
	{
	  if (jtab[i].size == 0)
	    continue;
	  if (jtab[i].magic == magic)
	    break;
	}
      /* If we did not find a matching entry something is wrong.  */
      if (i < 0)
	abort ();

      /* Create the .text section.  */
      text = pe_ILF_make_a_section (& vars, ".text", jtab[i].size, SEC_CODE);
      if (text == NULL)
	goto error_return;

      /* Copy in the jump code.  */
      memcpy (text->contents, jtab[i].data, jtab[i].size);

      /* Create a reloc for the data in the text section.  */
#ifdef MIPS_ARCH_MAGIC_WINCE
      if (magic == MIPS_ARCH_MAGIC_WINCE)
	{
	  pe_ILF_make_a_symbol_reloc (&vars, (bfd_vma) 0, BFD_RELOC_HI16_S,
				      (struct bfd_symbol **) imp_sym,
				      imp_index);
	  pe_ILF_make_a_reloc (&vars, (bfd_vma) 0, BFD_RELOC_LO16, text);
	  pe_ILF_make_a_symbol_reloc (&vars, (bfd_vma) 4, BFD_RELOC_LO16,
				      (struct bfd_symbol **) imp_sym,
				      imp_index);
	}
      else
#endif
#ifdef AMD64MAGIC
      if (magic == AMD64MAGIC)
	{
	  pe_ILF_make_a_symbol_reloc (&vars, (bfd_vma) jtab[i].offset,
				      BFD_RELOC_32_PCREL, (asymbol **) imp_sym,
				      imp_index);
	}
      else
#endif
	pe_ILF_make_a_symbol_reloc (&vars, (bfd_vma) jtab[i].offset,
				    BFD_RELOC_32, (asymbol **) imp_sym,
				    imp_index);

      pe_ILF_save_relocs (& vars, text);
      break;

    case IMPORT_DATA:
      break;

    default:
      /* XXX code not yet written.  */
      abort ();
    }

  /* Now create a symbol describing the imported value.  */
  switch (import_type)
    {
    case IMPORT_CODE:
      pe_ILF_make_a_symbol (& vars, "", symbol_name, text,
			    BSF_NOT_AT_END | BSF_FUNCTION);

      break;

    case IMPORT_DATA:
      /* Nothing to do here.  */
      break;

    default:
      /* XXX code not yet written.  */
      abort ();
    }

  /* Create an import symbol for the DLL, without the .dll suffix.  */
  ptr = (bfd_byte *) strrchr (source_dll, '.');
  if (ptr)
    * ptr = 0;
  pe_ILF_make_a_symbol (& vars, "__IMPORT_DESCRIPTOR_", source_dll, NULL, 0);
  if (ptr)
    * ptr = '.';

  /* Initialise the bfd.  */
  memset (& internal_f, 0, sizeof (internal_f));

  internal_f.f_magic  = magic;
  internal_f.f_symptr = 0;
  internal_f.f_nsyms  = 0;
  internal_f.f_flags  = F_AR32WR | F_LNNO; /* XXX is this correct ?  */

  if (   ! bfd_set_start_address (abfd, (bfd_vma) 0)
      || ! bfd_coff_set_arch_mach_hook (abfd, & internal_f))
    goto error_return;

  if (bfd_coff_mkobject_hook (abfd, (void *) & internal_f, NULL) == NULL)
    goto error_return;

  obj_pe (abfd) = true;
#ifdef THUMBPEMAGIC
  if (vars.magic == THUMBPEMAGIC)
    /* Stop some linker warnings about thumb code not supporting interworking.  */
    coff_data (abfd)->flags |= F_INTERWORK | F_INTERWORK_SET;
#endif

  /* Switch from file contents to memory contents.  */
  bfd_cache_close (abfd);

  abfd->iostream = (void *) vars.bim;
  abfd->flags |= BFD_IN_MEMORY | HAS_SYMS;
  abfd->iovec = &_bfd_memory_iovec;
  abfd->where = 0;
  abfd->origin = 0;
  abfd->size = 0;
  obj_sym_filepos (abfd) = 0;

  /* Point the bfd at the symbol table.  */
  obj_symbols (abfd) = vars.sym_cache;
  abfd->symcount = vars.sym_index;

  obj_raw_syments (abfd) = vars.native_syms;
  obj_raw_syment_count (abfd) = vars.sym_index;

  obj_coff_external_syms (abfd) = (void *) vars.esym_table;
  obj_coff_keep_syms (abfd) = true;

  obj_convert (abfd) = vars.sym_table;
  obj_conv_table_size (abfd) = vars.sym_index;

  obj_coff_strings (abfd) = vars.string_table;
  obj_coff_strings_len (abfd) = vars.string_ptr - vars.string_table;
  obj_coff_keep_strings (abfd) = true;

  return true;

 error_return:
  free (vars.bim->buffer);
  free (vars.bim);
  return false;
}

/* Cleanup function, returned from check_format hook.  */

static void
pe_ILF_cleanup (bfd *abfd)
{
  struct bfd_in_memory *bim = abfd->iostream;
  free (bim->buffer);
  free (bim);
  abfd->iostream = NULL;
}

/* We have detected an Import Library Format archive element.
   Decode the element and return the appropriate target.  */

static bfd_cleanup
pe_ILF_object_p (bfd * abfd)
{
  bfd_byte	  buffer[14];
  bfd_byte *	  ptr;
  char *	  symbol_name;
  char *	  source_dll;
  unsigned int	  machine;
  bfd_size_type	  size;
  unsigned int	  ordinal;
  unsigned int	  types;
  unsigned int	  magic;

  /* Upon entry the first six bytes of the ILF header have
     already been read.  Now read the rest of the header.  */
  if (bfd_bread (buffer, (bfd_size_type) 14, abfd) != 14)
    return NULL;

  ptr = buffer;

  machine = H_GET_16 (abfd, ptr);
  ptr += 2;

  /* Check that the machine type is recognised.  */
  magic = 0;

  switch (machine)
    {
    case IMAGE_FILE_MACHINE_UNKNOWN:
    case IMAGE_FILE_MACHINE_ALPHA:
    case IMAGE_FILE_MACHINE_ALPHA64:
    case IMAGE_FILE_MACHINE_IA64:
      break;

    case IMAGE_FILE_MACHINE_I386:
#ifdef I386MAGIC
      magic = I386MAGIC;
#endif
      break;

    case IMAGE_FILE_MACHINE_AMD64:
#ifdef AMD64MAGIC
      magic = AMD64MAGIC;
#endif
      break;

    case IMAGE_FILE_MACHINE_R3000:
    case IMAGE_FILE_MACHINE_R4000:
    case IMAGE_FILE_MACHINE_R10000:

    case IMAGE_FILE_MACHINE_MIPS16:
    case IMAGE_FILE_MACHINE_MIPSFPU:
    case IMAGE_FILE_MACHINE_MIPSFPU16:
#ifdef MIPS_ARCH_MAGIC_WINCE
      magic = MIPS_ARCH_MAGIC_WINCE;
#endif
      break;

    case IMAGE_FILE_MACHINE_SH3:
    case IMAGE_FILE_MACHINE_SH4:
#ifdef SH_ARCH_MAGIC_WINCE
      magic = SH_ARCH_MAGIC_WINCE;
#endif
      break;

    case IMAGE_FILE_MACHINE_ARM:
#ifdef ARMPEMAGIC
      magic = ARMPEMAGIC;
#endif
      break;

    case IMAGE_FILE_MACHINE_ARM64:
#ifdef AARCH64MAGIC
      magic = AARCH64MAGIC;
#endif
      break;

    case IMAGE_FILE_MACHINE_LOONGARCH64:
#ifdef LOONGARCH64MAGIC
      magic = LOONGARCH64MAGIC;
#endif
      break;

    case IMAGE_FILE_MACHINE_THUMB:
#ifdef THUMBPEMAGIC
      {
	extern const bfd_target TARGET_LITTLE_SYM;

	if (abfd->xvec == & TARGET_LITTLE_SYM)
	  magic = THUMBPEMAGIC;
      }
#endif
      break;

    case IMAGE_FILE_MACHINE_POWERPC:
      /* We no longer support PowerPC.  */
    default:
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: unrecognised machine type (0x%x)"
	   " in Import Library Format archive"),
	 abfd, machine);
      bfd_set_error (bfd_error_malformed_archive);

      return NULL;
      break;
    }

  if (magic == 0)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: recognised but unhandled machine type (0x%x)"
	   " in Import Library Format archive"),
	 abfd, machine);
      bfd_set_error (bfd_error_wrong_format);

      return NULL;
    }

  /* We do not bother to check the date.
     date = H_GET_32 (abfd, ptr);  */
  ptr += 4;

  size = H_GET_32 (abfd, ptr);
  ptr += 4;

  if (size == 0)
    {
      _bfd_error_handler
	(_("%pB: size field is zero in Import Library Format header"), abfd);
      bfd_set_error (bfd_error_malformed_archive);

      return NULL;
    }

  ordinal = H_GET_16 (abfd, ptr);
  ptr += 2;

  types = H_GET_16 (abfd, ptr);
  /* ptr += 2; */

  /* Now read in the two strings that follow.  */
  ptr = (bfd_byte *) _bfd_alloc_and_read (abfd, size, size);
  if (ptr == NULL)
    return NULL;

  symbol_name = (char *) ptr;
  /* See PR 20905 for an example of where the strnlen is necessary.  */
  source_dll  = symbol_name + strnlen (symbol_name, size - 1) + 1;

  /* Verify that the strings are null terminated.  */
  if (ptr[size - 1] != 0
      || (bfd_size_type) ((bfd_byte *) source_dll - ptr) >= size)
    {
      _bfd_error_handler
	(_("%pB: string not null terminated in ILF object file"), abfd);
      bfd_set_error (bfd_error_malformed_archive);
      bfd_release (abfd, ptr);
      return NULL;
    }

  /* Now construct the bfd.  */
  if (! pe_ILF_build_a_bfd (abfd, magic, symbol_name,
			    source_dll, ordinal, types))
    {
      bfd_release (abfd, ptr);
      return NULL;
    }

  return pe_ILF_cleanup;
}

static void
pe_bfd_read_buildid (bfd *abfd)
{
  pe_data_type *pe = pe_data (abfd);
  struct internal_extra_pe_aouthdr *extra = &pe->pe_opthdr;
  asection *section;
  bfd_byte *data = 0;
  bfd_size_type dataoff;
  unsigned int i;
  bfd_vma addr = extra->DataDirectory[PE_DEBUG_DATA].VirtualAddress;
  bfd_size_type size = extra->DataDirectory[PE_DEBUG_DATA].Size;

  if (size == 0)
    return;

  addr += extra->ImageBase;

  /* Search for the section containing the DebugDirectory.  */
  for (section = abfd->sections; section != NULL; section = section->next)
    {
      if ((addr >= section->vma) && (addr < (section->vma + section->size)))
	break;
    }

  if (section == NULL)
    return;

  if (!(section->flags & SEC_HAS_CONTENTS))
    return;

  dataoff = addr - section->vma;

  /* PR 20605 and 22373: Make sure that the data is really there.
     Note - since we are dealing with unsigned quantities we have
     to be careful to check for potential overflows.  */
  if (dataoff >= section->size
      || size > section->size - dataoff)
    {
      _bfd_error_handler
	(_("%pB: error: debug data ends beyond end of debug directory"),
	 abfd);
      return;
    }

  /* Read the whole section. */
  if (!bfd_malloc_and_get_section (abfd, section, &data))
    {
      free (data);
      return;
    }

  /* Search for a CodeView entry in the DebugDirectory */
  for (i = 0; i < size / sizeof (struct external_IMAGE_DEBUG_DIRECTORY); i++)
    {
      struct external_IMAGE_DEBUG_DIRECTORY *ext
	= &((struct external_IMAGE_DEBUG_DIRECTORY *)(data + dataoff))[i];
      struct internal_IMAGE_DEBUG_DIRECTORY idd;

      _bfd_XXi_swap_debugdir_in (abfd, ext, &idd);

      if (idd.Type == PE_IMAGE_DEBUG_TYPE_CODEVIEW)
	{
	  char buffer[256 + 1];
	  CODEVIEW_INFO *cvinfo = (CODEVIEW_INFO *) buffer;

	  /*
	    The debug entry doesn't have to have to be in a section, in which
	    case AddressOfRawData is 0, so always use PointerToRawData.
	  */
	  if (_bfd_XXi_slurp_codeview_record (abfd,
					      (file_ptr) idd.PointerToRawData,
					      idd.SizeOfData, cvinfo, NULL))
	    {
	      struct bfd_build_id* build_id = bfd_alloc (abfd,
			 sizeof (struct bfd_build_id) + cvinfo->SignatureLength);
	      if (build_id)
		{
		  build_id->size = cvinfo->SignatureLength;
		  memcpy(build_id->data,  cvinfo->Signature,
			 cvinfo->SignatureLength);
		  abfd->build_id = build_id;
		}
	    }
	  break;
	}
    }

  free (data);
}

static bfd_cleanup
pe_bfd_object_p (bfd * abfd)
{
  bfd_byte buffer[6];
  struct external_DOS_hdr dos_hdr;
  struct external_PEI_IMAGE_hdr image_hdr;
  struct internal_filehdr internal_f;
  struct internal_aouthdr internal_a;
  bfd_size_type opt_hdr_size;
  file_ptr offset;
  bfd_cleanup result;

  /* Detect if this a Microsoft Import Library Format element.  */
  /* First read the beginning of the header.  */
  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0
      || bfd_bread (buffer, (bfd_size_type) 6, abfd) != 6)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* Then check the magic and the version (only 0 is supported).  */
  if (H_GET_32 (abfd, buffer) == 0xffff0000
      && H_GET_16 (abfd, buffer + 4) == 0)
    return pe_ILF_object_p (abfd);

  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0
      || bfd_bread (&dos_hdr, (bfd_size_type) sizeof (dos_hdr), abfd)
	 != sizeof (dos_hdr))
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* There are really two magic numbers involved; the magic number
     that says this is a NT executable (PEI) and the magic number that
     determines the architecture.  The former is IMAGE_DOS_SIGNATURE, stored in
     the e_magic field.  The latter is stored in the f_magic field.
     If the NT magic number isn't valid, the architecture magic number
     could be mimicked by some other field (specifically, the number
     of relocs in section 3).  Since this routine can only be called
     correctly for a PEI file, check the e_magic number here, and, if
     it doesn't match, clobber the f_magic number so that we don't get
     a false match.  */
  if (H_GET_16 (abfd, dos_hdr.e_magic) != IMAGE_DOS_SIGNATURE)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  offset = H_GET_32 (abfd, dos_hdr.e_lfanew);
  if (bfd_seek (abfd, offset, SEEK_SET) != 0
      || (bfd_bread (&image_hdr, (bfd_size_type) sizeof (image_hdr), abfd)
	  != sizeof (image_hdr)))
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  if (H_GET_32 (abfd, image_hdr.nt_signature) != 0x4550)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* Swap file header, so that we get the location for calling
     real_object_p.  */
  bfd_coff_swap_filehdr_in (abfd, &image_hdr, &internal_f);

  if (! bfd_coff_bad_format_hook (abfd, &internal_f)
      || internal_f.f_opthdr > bfd_coff_aoutsz (abfd))
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  memcpy (internal_f.pe.dos_message, dos_hdr.dos_message,
	  sizeof (internal_f.pe.dos_message));

  /* Read the optional header, which has variable size.  */
  opt_hdr_size = internal_f.f_opthdr;

  if (opt_hdr_size != 0)
    {
      bfd_size_type amt = opt_hdr_size;
      bfd_byte * opthdr;

      /* PR 17521 file: 230-131433-0.004.  */
      if (amt < sizeof (PEAOUTHDR))
	amt = sizeof (PEAOUTHDR);

      opthdr = _bfd_alloc_and_read (abfd, amt, opt_hdr_size);
      if (opthdr == NULL)
	return NULL;
      if (amt > opt_hdr_size)
	memset (opthdr + opt_hdr_size, 0, amt - opt_hdr_size);

      bfd_coff_swap_aouthdr_in (abfd, opthdr, &internal_a);

      struct internal_extra_pe_aouthdr *a = &internal_a.pe;

#ifdef ARM
      /* Use Subsystem to distinguish between pei-arm-little and
	 pei-arm-wince-little.  */
#ifdef WINCE
      if (a->Subsystem != IMAGE_SUBSYSTEM_WINDOWS_CE_GUI)
#else
      if (a->Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CE_GUI)
#endif
	{
	  bfd_set_error (bfd_error_wrong_format);
	  return NULL;
	}
#endif

      if ((a->SectionAlignment & -a->SectionAlignment) != a->SectionAlignment
	  || a->SectionAlignment >= 0x80000000)
	{
	  _bfd_error_handler (_("%pB: adjusting invalid SectionAlignment"),
				abfd);
	  a->SectionAlignment &= -a->SectionAlignment;
	  if (a->SectionAlignment >= 0x80000000)
	    a->SectionAlignment = 0x40000000;
	}

      if ((a->FileAlignment & -a->FileAlignment) != a->FileAlignment
	  || a->FileAlignment > a->SectionAlignment)
	{
	  _bfd_error_handler (_("%pB: adjusting invalid FileAlignment"),
			      abfd);
	  a->FileAlignment &= -a->FileAlignment;
	  if (a->FileAlignment > a->SectionAlignment)
	    a->FileAlignment = a->SectionAlignment;
	}

      if (a->NumberOfRvaAndSizes > IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
	_bfd_error_handler (_("%pB: invalid NumberOfRvaAndSizes"), abfd);
    }

  result = coff_real_object_p (abfd, internal_f.f_nscns, &internal_f,
			       (opt_hdr_size != 0
				? &internal_a
				: (struct internal_aouthdr *) NULL));

  if (result)
    {
      /* Now the whole header has been processed, see if there is a build-id */
      pe_bfd_read_buildid(abfd);
    }

  return result;
}

#define coff_object_p pe_bfd_object_p
#endif /* COFF_IMAGE_WITH_PE */
