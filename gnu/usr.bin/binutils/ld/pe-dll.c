/* Routines to help build PEI-format DLLs (Win32 etc)
   Copyright (C) 1998-2023 Free Software Foundation, Inc.
   Written by DJ Delorie <dj@cygnus.com>

   This file is part of the GNU Binutils.

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
#include "bfdlink.h"
#include "libiberty.h"
#include "filenames.h"
#include "safe-ctype.h"
#include "ctf-api.h"

#include <time.h>

#include "ld.h"
#include "ldexp.h"
#include "ldlang.h"
#include "ldwrite.h"
#include "ldmisc.h"
#include <ldgram.h>
#include "ldmain.h"
#include "ldfile.h"
#include "ldemul.h"
#include "coff/internal.h"
#include "../bfd/libcoff.h"
#include "deffile.h"

#ifdef pe_use_plus

#define PE_IDATA4_SIZE	8
#define PE_IDATA5_SIZE	8
#include "pep-dll.h"
#undef  AOUTSZ
#define AOUTSZ		PEPAOUTSZ
#define PEAOUTHDR	PEPAOUTHDR

#else

#include "pe-dll.h"

#endif

#ifndef PE_IDATA4_SIZE
#define PE_IDATA4_SIZE	4
#endif

#ifndef PE_IDATA5_SIZE
#define PE_IDATA5_SIZE	4
#endif

/*  This file turns a regular Windows PE image into a DLL.  Because of
    the complexity of this operation, it has been broken down into a
    number of separate modules which are all called by the main function
    at the end of this file.  This function is not re-entrant and is
    normally only called once, so static variables are used to reduce
    the number of parameters and return values required.

    See also: ld/emultempl/pe.em and ld/emultempl/pep.em.  */

/*  Auto-import feature by Paul Sokolovsky

    Quick facts:

    1. With this feature on, DLL clients can import variables from DLL
    without any concern from their side (for example, without any source
    code modifications).

    2. This is done completely in bounds of the PE specification (to be fair,
    there's a place where it pokes nose out of, but in practice it works).
    So, resulting module can be used with any other PE compiler/linker.

    3. Auto-import is fully compatible with standard import method and they
    can be mixed together.

    4. Overheads: space: 8 bytes per imported symbol, plus 20 for each
    reference to it; load time: negligible; virtual/physical memory: should be
    less than effect of DLL relocation, and I sincerely hope it doesn't affect
    DLL sharability (too much).

    Idea

    The obvious and only way to get rid of dllimport insanity is to make client
    access variable directly in the DLL, bypassing extra dereference. I.e.,
    whenever client contains something like

    mov dll_var,%eax,

    address of dll_var in the command should be relocated to point into loaded
    DLL. The aim is to make OS loader do so, and than make ld help with that.
    Import section of PE made following way: there's a vector of structures
    each describing imports from particular DLL. Each such structure points
    to two other parallel vectors: one holding imported names, and one which
    will hold address of corresponding imported name. So, the solution is
    de-vectorize these structures, making import locations be sparse and
    pointing directly into code. Before continuing, it is worth a note that,
    while authors strives to make PE act ELF-like, there're some other people
    make ELF act PE-like: elfvector, ;-) .

    Implementation

    For each reference of data symbol to be imported from DLL (to set of which
    belong symbols with name <sym>, if __imp_<sym> is found in implib), the
    import fixup entry is generated. That entry is of type
    IMAGE_IMPORT_DESCRIPTOR and stored in .idata$2 subsection. Each
    fixup entry contains pointer to symbol's address within .text section
    (marked with __fuN_<sym> symbol, where N is integer), pointer to DLL name
    (so, DLL name is referenced by multiple entries), and pointer to symbol
    name thunk. Symbol name thunk is singleton vector (__nm_th_<symbol>)
    pointing to IMAGE_IMPORT_BY_NAME structure (__nm_<symbol>) directly
    containing imported name. Here comes that "on the edge" problem mentioned
    above: PE specification rambles that name vector (OriginalFirstThunk)
    should run in parallel with addresses vector (FirstThunk), i.e. that they
    should have same number of elements and terminated with zero. We violate
    this, since FirstThunk points directly into machine code. But in practice,
    OS loader implemented the sane way: it goes through OriginalFirstThunk and
    puts addresses to FirstThunk, not something else. It once again should be
    noted that dll and symbol name structures are reused across fixup entries
    and should be there anyway to support standard import stuff, so sustained
    overhead is 20 bytes per reference. Other question is whether having several
    IMAGE_IMPORT_DESCRIPTORS for the same DLL is possible. Answer is yes, it is
    done even by native compiler/linker (libth32's functions are in fact reside
    in windows9x kernel32.dll, so if you use it, you have two
    IMAGE_IMPORT_DESCRIPTORS for kernel32.dll). Yet other question is whether
    referencing the same PE structures several times is valid. The answer is why
    not, prohibiting that (detecting violation) would require more work on
    behalf of loader than not doing it.

    See also: ld/emultempl/pe.em and ld/emultempl/pep.em.  */

static void add_bfd_to_link (bfd *, const char *, struct bfd_link_info *);

/* For emultempl/pe.em.  */

def_file * pe_def_file = 0;
int pe_dll_export_everything = 0;
int pe_dll_exclude_all_symbols = 0;
int pe_dll_do_default_excludes = 1;
int pe_dll_kill_ats = 0;
int pe_dll_stdcall_aliases = 0;
int pe_dll_warn_dup_exports = 0;
int pe_dll_compat_implib = 0;
int pe_dll_extra_pe_debug = 0;
int pe_use_nul_prefixed_import_tables = 0;
int pe_use_coff_long_section_names = -1;
int pe_leading_underscore = -1;
int pe_dll_enable_reloc_section = 1;

/* Static variables and types.  */

static bfd_vma image_base;
static bfd *filler_bfd;
static struct bfd_section *edata_s, *reloc_s;
static unsigned char *edata_d, *reloc_d;
static unsigned char *reloc_d = NULL;
static size_t edata_sz, reloc_sz = 0;
static int runtime_pseudo_relocs_created = 0;
static bool runtime_pseudp_reloc_v2_init = false;

typedef struct
{
  const char *name;
  int len;
}
autofilter_entry_type;

typedef struct
{
  const char *target_name;
  const char *object_target;
  unsigned int imagebase_reloc;
  int pe_arch;
  int bfd_arch;
  bool underscored;
  const autofilter_entry_type* autofilter_symbollist;
}
pe_details_type;

static const autofilter_entry_type autofilter_symbollist_generic[] =
{
  { STRING_COMMA_LEN ("_NULL_IMPORT_DESCRIPTOR") },
  /* Entry point symbols.  */
  { STRING_COMMA_LEN ("DllMain") },
  { STRING_COMMA_LEN ("DllMainCRTStartup") },
  { STRING_COMMA_LEN ("_DllMainCRTStartup") },
  /* Runtime pseudo-reloc.  */
  { STRING_COMMA_LEN ("_pei386_runtime_relocator") },
  { STRING_COMMA_LEN ("do_pseudo_reloc") },
  { NULL, 0 }
};

static const autofilter_entry_type autofilter_symbollist_i386[] =
{
  { STRING_COMMA_LEN ("_NULL_IMPORT_DESCRIPTOR") },
  /* Entry point symbols, and entry hooks.  */
  { STRING_COMMA_LEN ("cygwin_crt0") },
#ifdef pe_use_plus
  { STRING_COMMA_LEN ("DllMain") },
  { STRING_COMMA_LEN ("DllEntryPoint") },
  { STRING_COMMA_LEN ("DllMainCRTStartup") },
  { STRING_COMMA_LEN ("_cygwin_dll_entry") },
  { STRING_COMMA_LEN ("_cygwin_crt0_common") },
  { STRING_COMMA_LEN ("_cygwin_noncygwin_dll_entry") },
#else
  { STRING_COMMA_LEN ("DllMain@12") },
  { STRING_COMMA_LEN ("DllEntryPoint@0") },
  { STRING_COMMA_LEN ("DllMainCRTStartup@12") },
  { STRING_COMMA_LEN ("_cygwin_dll_entry@12") },
  { STRING_COMMA_LEN ("_cygwin_crt0_common@8") },
  { STRING_COMMA_LEN ("_cygwin_noncygwin_dll_entry@12") },
  { STRING_COMMA_LEN ("cygwin_attach_dll") },
#endif
  { STRING_COMMA_LEN ("cygwin_premain0") },
  { STRING_COMMA_LEN ("cygwin_premain1") },
  { STRING_COMMA_LEN ("cygwin_premain2") },
  { STRING_COMMA_LEN ("cygwin_premain3") },
  /* Runtime pseudo-reloc.  */
  { STRING_COMMA_LEN ("_pei386_runtime_relocator") },
  { STRING_COMMA_LEN ("do_pseudo_reloc") },
  /* Global vars that should not be exported.  */
  { STRING_COMMA_LEN ("impure_ptr") },
  { STRING_COMMA_LEN ("_impure_ptr") },
  { STRING_COMMA_LEN ("_fmode") },
  { STRING_COMMA_LEN ("environ") },
  { STRING_COMMA_LEN ("__dso_handle") },
  { NULL, 0 }
};

#define PE_ARCH_i386	 1
#define PE_ARCH_sh	 2
#define PE_ARCH_mips	 3
#define PE_ARCH_arm	 4
#define PE_ARCH_arm_wince 5
#define PE_ARCH_aarch64  6

/* Don't make it constant as underscore mode gets possibly overriden
   by target or -(no-)leading-underscore option.  */
static pe_details_type pe_detail_list[] =
{
  {
#ifdef pe_use_plus
    "pei-x86-64",
    "pe-x86-64",
    3 /* R_IMAGEBASE */,
#else
    "pei-i386",
    "pe-i386",
    7 /* R_IMAGEBASE */,
#endif
    PE_ARCH_i386,
    bfd_arch_i386,
#ifdef pe_use_plus
    false,
#else
    true,
#endif
    autofilter_symbollist_i386
  },
#ifdef pe_use_plus
  {
    "pei-x86-64",
    "pe-bigobj-x86-64",
    3 /* R_IMAGEBASE */,
    PE_ARCH_i386,
    bfd_arch_i386,
    false,
    autofilter_symbollist_i386
  },
#else
  {
    "pei-i386",
    "pe-bigobj-i386",
    7 /* R_IMAGEBASE */,
    PE_ARCH_i386,
    bfd_arch_i386,
    true,
    autofilter_symbollist_i386
  },
#endif
  {
    "pei-shl",
    "pe-shl",
    16 /* R_SH_IMAGEBASE */,
    PE_ARCH_sh,
    bfd_arch_sh,
    true,
    autofilter_symbollist_generic
  },
  {
    "pei-mips",
    "pe-mips",
    34 /* MIPS_R_RVA */,
    PE_ARCH_mips,
    bfd_arch_mips,
    false,
    autofilter_symbollist_generic
  },
  {
    "pei-arm-little",
    "pe-arm-little",
    11 /* ARM_RVA32 */,
    PE_ARCH_arm,
    bfd_arch_arm,
    true,
    autofilter_symbollist_generic
  },
  {
    "pei-arm-wince-little",
    "pe-arm-wince-little",
    2,  /* ARM_RVA32 on Windows CE, see bfd/coff-arm.c.  */
    PE_ARCH_arm_wince,
    bfd_arch_arm,
    false,
    autofilter_symbollist_generic
  },
  {
    "pei-aarch64-little",
    "pe-aarch64-little",
    2,  /* ARM64_RVA32 */
    PE_ARCH_aarch64,
    bfd_arch_aarch64,
    false,
    autofilter_symbollist_generic
  },
  { NULL, NULL, 0, 0, 0, false, NULL }
};

static const pe_details_type *pe_details;

/* Do not specify library suffix explicitly, to allow for dllized versions.  */
static const autofilter_entry_type autofilter_liblist[] =
{
  { STRING_COMMA_LEN ("libcegcc") },
  { STRING_COMMA_LEN ("libcygwin") },
  { STRING_COMMA_LEN ("libgcc") },
  { STRING_COMMA_LEN ("libgcc_s") },
  { STRING_COMMA_LEN ("libstdc++") },
  { STRING_COMMA_LEN ("libmingw32") },
  { STRING_COMMA_LEN ("libmingwex") },
  { STRING_COMMA_LEN ("libg2c") },
  { STRING_COMMA_LEN ("libsupc++") },
  { STRING_COMMA_LEN ("libobjc") },
  { STRING_COMMA_LEN ("libgcj") },
  { STRING_COMMA_LEN ("libmsvcrt") },
  { STRING_COMMA_LEN ("libmsvcrt-os") },
  { STRING_COMMA_LEN ("libucrt") },
  { STRING_COMMA_LEN ("libucrtbase") },
  { NULL, 0 }
};

/* Regardless of the suffix issue mentioned above, we must ensure that
  we do not falsely match on a leading substring, such as when libtool
  builds libstdc++ as a DLL using libsupc++convenience.a as an intermediate.
  This routine ensures that the leading part of the name matches and that
  it is followed by only an optional version suffix and a file extension,
  returning zero if so or -1 if not.  */
static int libnamencmp (const char *libname, const autofilter_entry_type *afptr)
{
  if (filename_ncmp (libname, afptr->name, afptr->len))
    return -1;

  libname += afptr->len;

  /* Be liberal in interpreting what counts as a version suffix; we
    accept anything that has a dash to separate it from the name and
    begins with a digit.  */
  if (libname[0] == '-')
    {
      if (!ISDIGIT (*++libname))
	return -1;
      /* Ensure the filename has an extension.  */
      while (*++libname != '.')
	if (!*libname)
	  return -1;
    }
  else if (libname[0] != '.')
    return -1;

  return 0;
}

static const autofilter_entry_type autofilter_objlist[] =
{
  { STRING_COMMA_LEN ("crt0.o") },
  { STRING_COMMA_LEN ("crt1.o") },
  { STRING_COMMA_LEN ("crt2.o") },
  { STRING_COMMA_LEN ("dllcrt1.o") },
  { STRING_COMMA_LEN ("dllcrt2.o") },
  { STRING_COMMA_LEN ("gcrt0.o") },
  { STRING_COMMA_LEN ("gcrt1.o") },
  { STRING_COMMA_LEN ("gcrt2.o") },
  { STRING_COMMA_LEN ("crtbegin.o") },
  { STRING_COMMA_LEN ("crtend.o") },
  { NULL, 0 }
};

static const autofilter_entry_type autofilter_symbolprefixlist[] =
{
  /* _imp_ is treated specially, as it is always underscored.  */
  /* { STRING_COMMA_LEN ("_imp_") },  */
  /* Don't export some c++ symbols.  */
  { STRING_COMMA_LEN ("__rtti_") },
  { STRING_COMMA_LEN ("__builtin_") },
  /* Don't re-export auto-imported symbols.  */
  { STRING_COMMA_LEN ("__nm_") },
  /* Don't export symbols specifying internal DLL layout.  */
  { STRING_COMMA_LEN ("_head_") },
  { STRING_COMMA_LEN ("_IMPORT_DESCRIPTOR_") },
  /* Don't export section labels or artificial symbols
  (eg ".weak.foo".  */
  { STRING_COMMA_LEN (".") },
  { NULL, 0 }
};

static const autofilter_entry_type autofilter_symbolsuffixlist[] =
{
  { STRING_COMMA_LEN ("_iname") },
  { STRING_COMMA_LEN ("_NULL_THUNK_DATA") },
  { NULL, 0 }
};

#define U(str) (pe_details->underscored ? "_" str : str)

void
pe_dll_id_target (const char *target)
{
  int i;

  for (i = 0; pe_detail_list[i].target_name; i++)
    if (strcmp (pe_detail_list[i].target_name, target) == 0
	|| strcmp (pe_detail_list[i].object_target, target) == 0)
      {
	int u = pe_leading_underscore; /* Underscoring mode. -1 for use default.  */
	if (u == -1)
	  bfd_get_target_info (target, NULL, NULL, &u, NULL);
	if (u == -1)
	  abort ();
	pe_detail_list[i].underscored = u != 0;
	pe_details = pe_detail_list + i;
	pe_leading_underscore = (u != 0 ? 1 : 0);
	return;
      }
  einfo (_("%X%P: unsupported PEI architecture: %s\n"), target);
  exit (1);
}

/* Helper functions for qsort.  Relocs must be sorted so that we can write
   them out by pages.  */

typedef struct
  {
    bfd_vma vma;
    char type;
    short extra;
    int idx;
  }
reloc_data_type;

static int
reloc_sort (const void *va, const void *vb)
{
  const reloc_data_type *a = (const reloc_data_type *) va;
  const reloc_data_type *b = (const reloc_data_type *) vb;

  if (a->vma > b->vma)
    return 1;
  if (a->vma < b->vma)
    return -1;
  if (a->idx > b->idx)
    return 1;
  if (a->idx < b->idx)
    return -1;
  return 0;
}

static int
pe_export_sort (const void *va, const void *vb)
{
  const def_file_export *a = va;
  const def_file_export *b = vb;
  char *an = a->name;
  char *bn = b->name;
  if (a->its_name)
    an = a->its_name;
  if (b->its_name)
    bn = b->its_name;

  return strcmp (an, bn);
}

/* Read and process the .DEF file.  */

/* These correspond to the entries in pe_def_file->exports[].  I use
   exported_symbol_sections[i] to tag whether or not the symbol was
   defined, since we can't export symbols we don't have.  */

static bfd_vma *exported_symbol_offsets;
static struct bfd_section **exported_symbol_sections;
static int export_table_size;
static int count_exported;
static int count_exported_byname;
static int count_with_ordinals;
static const char *dll_filename;
static int min_ordinal, max_ordinal;
static int *exported_symbols;

typedef struct exclude_list_struct
  {
    char *string;
    struct exclude_list_struct *next;
    exclude_type type;
  }
exclude_list_struct;

static struct exclude_list_struct *excludes = 0;

void
pe_dll_add_excludes (const char *new_excludes, const exclude_type type)
{
  char *local_copy;
  char *exclude_string;

  local_copy = xstrdup (new_excludes);

  exclude_string = strtok (local_copy, ",:");
  for (; exclude_string; exclude_string = strtok (NULL, ",:"))
    {
      struct exclude_list_struct *new_exclude;

      new_exclude = xmalloc (sizeof (struct exclude_list_struct));
      new_exclude->string = xmalloc (strlen (exclude_string) + 1);
      strcpy (new_exclude->string, exclude_string);
      new_exclude->type = type;
      new_exclude->next = excludes;
      excludes = new_exclude;
    }

  free (local_copy);
}

static bool
is_import (const char* n)
{
  return (startswith (n, "__imp_"));
}

/* abfd is a bfd containing n (or NULL)
   It can be used for contextual checks.  */

static int
auto_export (bfd *abfd, def_file *d, const char *n)
{
  def_file_export key;
  struct exclude_list_struct *ex;
  const autofilter_entry_type *afptr;
  const char * libname = NULL;

  if (abfd && abfd->my_archive)
    libname = lbasename (bfd_get_filename (abfd->my_archive));

  key.name = key.its_name = (char *) n;

  /* Return false if n is in the d->exports table.  */
  if (d->num_exports != 0
      && bsearch (&key, d->exports, d->num_exports,
		  sizeof (pe_def_file->exports[0]), pe_export_sort))
    return 0;

  if (pe_dll_do_default_excludes)
    {
      const char * p;
      int    len;

      if (pe_dll_extra_pe_debug)
	printf ("considering exporting: %s, abfd=%p, abfd->my_arc=%p\n",
		n, abfd, abfd->my_archive);

      /* First of all, make context checks:
	 Don't export anything from standard libs.  */
      if (libname)
	{
	  afptr = autofilter_liblist;

	  while (afptr->name)
	    {
	      if (libnamencmp (libname, afptr) == 0 )
		return 0;
	      afptr++;
	    }
	}

      /* Next, exclude symbols from certain startup objects.  */

      if (abfd && (p = lbasename (bfd_get_filename (abfd))))
	{
	  afptr = autofilter_objlist;
	  while (afptr->name)
	    {
	      if (strcmp (p, afptr->name) == 0)
		return 0;
	      afptr++;
	    }
	}

      /* Don't try to blindly exclude all symbols
	 that begin with '__'; this was tried and
	 it is too restrictive.  Instead we have
	 a target specific list to use:  */
      afptr = pe_details->autofilter_symbollist;

      while (afptr->name)
	{
	  if (strcmp (n, afptr->name) == 0)
	    return 0;

	  afptr++;
	}

      /* Next, exclude symbols starting with ...  */
      afptr = autofilter_symbolprefixlist;
      while (afptr->name)
	{
	  if (strncmp (n, afptr->name, afptr->len) == 0)
	    return 0;

	  afptr++;
	}

      /* Finally, exclude symbols ending with ...  */
      len = strlen (n);
      afptr = autofilter_symbolsuffixlist;
      while (afptr->name)
	{
	  if ((len >= afptr->len)
	      /* Add 1 to insure match with trailing '\0'.  */
	      && strncmp (n + len - afptr->len, afptr->name,
			  afptr->len + 1) == 0)
	    return 0;

	  afptr++;
	}
    }

  for (ex = excludes; ex; ex = ex->next)
    {
      if (ex->type == EXCLUDELIBS)
	{
	  if (libname
	      && ((filename_cmp (libname, ex->string) == 0)
		   || (strcasecmp ("ALL", ex->string) == 0)))
	    return 0;
	}
      else if (ex->type == EXCLUDEFORIMPLIB)
	{
	  if (filename_cmp (bfd_get_filename (abfd), ex->string) == 0)
	    return 0;
	}
      else if (strcmp (n, ex->string) == 0)
	return 0;
    }

  return 1;
}

static void
process_def_file_and_drectve (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *info)
{
  int i, j;
  unsigned int ui;
  struct bfd_link_hash_entry *blhe;
  bfd *b;
  struct bfd_section *s;
  def_file_export *e = 0;
  bool resort_needed;

  if (!pe_def_file)
    pe_def_file = def_file_empty ();

  /* First, run around to all the objects looking for the .drectve
     sections, and push those into the def file too.  */
  for (b = info->input_bfds; b; b = b->link.next)
    {
      s = bfd_get_section_by_name (b, ".drectve");
      if (s)
	{
	  long size = s->size;
	  char *buf = xmalloc (size);

	  bfd_get_section_contents (b, s, buf, 0, size);
	  def_file_add_directive (pe_def_file, buf, size);
	  free (buf);
	}
    }

  /* Process aligned common symbol information from the
     .drectve sections now; common symbol allocation is
     done before final link, so it will be too late to
     process them in process_embedded_commands() called
     from _bfd_coff_link_input_bfd().  */
  if (pe_def_file->aligncomms)
    {
      def_file_aligncomm *ac = pe_def_file->aligncomms;
      while (ac)
	{
	  struct coff_link_hash_entry *sym_hash;
	  sym_hash = coff_link_hash_lookup (coff_hash_table (info),
					    ac->symbol_name, false, false, false);
	  if (sym_hash && sym_hash->root.type == bfd_link_hash_common
	      && sym_hash->root.u.c.p->alignment_power < (unsigned) ac->alignment)
	    {
	      sym_hash->root.u.c.p->alignment_power = (unsigned) ac->alignment;
	    }
	  ac = ac->next;
	}
    }

  if (pe_def_file->exclude_symbols)
    {
      for (ui = 0; ui < pe_def_file->num_exclude_symbols; ui++)
	{
	  pe_dll_add_excludes (pe_def_file->exclude_symbols[ui].symbol_name,
			       EXCLUDESYMS);
	}
    }

  /* If we are building an executable and there is nothing
     to export, we do not build an export table at all.  */
  if (bfd_link_executable (info) && pe_def_file->num_exports == 0
      && (!pe_dll_export_everything || pe_dll_exclude_all_symbols))
    return;

  /* Now, maybe export everything else the default way.  */
  if ((pe_dll_export_everything || pe_def_file->num_exports == 0)
      && !pe_dll_exclude_all_symbols)
    {
      for (b = info->input_bfds; b; b = b->link.next)
	{
	  asymbol **symbols;
	  int nsyms;

	  if (!bfd_generic_link_read_symbols (b))
	    {
	      einfo (_("%F%P: %pB: could not read symbols: %E\n"), b);
	      return;
	    }

	  symbols = bfd_get_outsymbols (b);
	  nsyms = bfd_get_symcount (b);

	  for (j = 0; j < nsyms; j++)
	    {
	      /* We should export symbols which are either global or not
		 anything at all.  (.bss data is the latter)
		 We should not export undefined symbols.  */
	      bool would_export
		= (symbols[j]->section != bfd_und_section_ptr
		   && ((symbols[j]->flags & BSF_GLOBAL)
		       || (symbols[j]->flags == 0)));
	      if (link_info.version_info && would_export)
		would_export
		  = !bfd_hide_sym_by_version (link_info.version_info,
					      symbols[j]->name);
	      if (would_export)
		{
		  const char *sn = symbols[j]->name;

		  /* We should not re-export imported stuff.  */
		  {
		    char *name;
		    if (is_import (sn))
		      continue;

		    name = xmalloc (strlen ("__imp_") + strlen (sn) + 1);
		    sprintf (name, "%s%s", "__imp_", sn);

		    blhe = bfd_link_hash_lookup (info->hash, name,
						 false, false, false);
		    free (name);

		    if (blhe && blhe->type == bfd_link_hash_defined)
		      continue;
		  }

		  if (pe_details->underscored && *sn == '_')
		    sn++;

		  if (auto_export (b, pe_def_file, sn))
		    {
		      bool is_dup = false;
		      def_file_export *p;

		      p = def_file_add_export (pe_def_file, sn, 0, -1,
					       NULL, &is_dup);
		      /* Fill data flag properly, from dlltool.c.  */
		      if (!is_dup)
			p->flag_data = !(symbols[j]->flags & BSF_FUNCTION);
		    }
		}
	    }
	}
    }

#undef NE
#define NE pe_def_file->num_exports

  /* Don't create an empty export table.  */
  if (NE == 0)
    return;

  resort_needed = false;

  /* Canonicalize the export list.  */
  if (pe_dll_kill_ats)
    {
      for (i = 0; i < NE; i++)
	{
	  /* Check for fastcall/stdcall-decoration, but ignore
	     C++ mangled names.  */
	  if (pe_def_file->exports[i].name[0] != '?'
	      && strchr (pe_def_file->exports[i].name, '@'))
	    {
	      /* This will preserve internal_name, which may have been
		 pointing to the same memory as name, or might not
		 have.  */
	      int lead_at = (*pe_def_file->exports[i].name == '@');
	      char *tmp = xstrdup (pe_def_file->exports[i].name + lead_at);
	      char *tmp_at = strrchr (tmp, '@');

	      if (tmp_at)
		*tmp_at = 0;
	      else
		einfo (_("%X%P: cannot export %s: invalid export name\n"),
		       pe_def_file->exports[i].name);
	      pe_def_file->exports[i].name = tmp;
	      resort_needed = true;
	    }
	}
    }

  /* Re-sort the exports table as we have possibly changed the order
     by removing leading @.  */
  if (resort_needed)
    qsort (pe_def_file->exports, NE, sizeof (pe_def_file->exports[0]),
	   pe_export_sort);

  if (pe_dll_stdcall_aliases)
    {
      for (i = 0; i < NE; i++)
	{
	  if (is_import (pe_def_file->exports[i].name))
	    continue;

	  if (strchr (pe_def_file->exports[i].name, '@'))
	    {
	      bool is_dup = true;
	      int lead_at = (*pe_def_file->exports[i].name == '@');
	      char *tmp = xstrdup (pe_def_file->exports[i].name + lead_at);

	      *(strchr (tmp, '@')) = 0;
	      if (auto_export (NULL, pe_def_file, tmp))
		def_file_add_export (pe_def_file, tmp,
				     pe_def_file->exports[i].internal_name,
				     -1, NULL, &is_dup);
	      if (is_dup)
		free (tmp);
	    }
	}
    }

  /* Convenience, but watch out for it changing.  */
  e = pe_def_file->exports;

  for (i = 0, j = 0; i < NE; i++)
    {
      if (i > 0 && strcmp (e[i].name, e[i - 1].name) == 0)
	{
	  /* This is a duplicate.  */
	  if (e[j - 1].ordinal != -1
	      && e[i].ordinal != -1
	      && e[j - 1].ordinal != e[i].ordinal)
	    {
	      if (pe_dll_warn_dup_exports)
		/* xgettext:c-format */
		einfo (_("%X%P: error, duplicate EXPORT with ordinals: %s (%d vs %d)\n"),
		       e[j - 1].name, e[j - 1].ordinal, e[i].ordinal);
	    }
	  else
	    {
	      if (pe_dll_warn_dup_exports)
		/* xgettext:c-format */
		einfo (_("%P: warning, duplicate EXPORT: %s\n"),
		       e[j - 1].name);
	    }

	  if (e[i].ordinal != -1)
	    e[j - 1].ordinal = e[i].ordinal;
	  e[j - 1].flag_private |= e[i].flag_private;
	  e[j - 1].flag_constant |= e[i].flag_constant;
	  e[j - 1].flag_noname |= e[i].flag_noname;
	  e[j - 1].flag_data |= e[i].flag_data;
	  free (e[i].name);
	  free (e[i].internal_name);
	  free (e[i].its_name);
	}
      else
	{
	  if (i != j)
	    e[j] = e[i];
	  j++;
	}
    }
  pe_def_file->num_exports = j;	/* == NE */

  exported_symbol_offsets = xmalloc (NE * sizeof (bfd_vma));
  exported_symbol_sections = xmalloc (NE * sizeof (struct bfd_section *));

  memset (exported_symbol_sections, 0, NE * sizeof (struct bfd_section *));
  max_ordinal = 0;
  min_ordinal = 65536;
  count_exported = 0;
  count_exported_byname = 0;
  count_with_ordinals = 0;

  for (i = 0; i < NE; i++)
    {
      char *int_name = pe_def_file->exports[i].internal_name;
      char *name;

      /* PR 19803: Make sure that any exported symbol does not get garbage collected.  */
      lang_add_gc_name (int_name);

      name = xmalloc (strlen (int_name) + 2);
      if (pe_details->underscored && int_name[0] != '@')
	{
	  *name = '_';
	  strcpy (name + 1, int_name);

	  /* PR 19803: The alias must be preserved as well.  */
	  lang_add_gc_name (xstrdup (name));
	}
      else
	strcpy (name, int_name);

      blhe = bfd_link_hash_lookup (info->hash,
				   name,
				   false, false, true);

      if (blhe
	  && (blhe->type == bfd_link_hash_defined
	      || (blhe->type == bfd_link_hash_common)))
	{
	  count_exported++;
	  if (!pe_def_file->exports[i].flag_noname)
	    count_exported_byname++;

	  /* Only fill in the sections. The actual offsets are computed
	     in fill_exported_offsets() after common symbols are laid
	     out.  */
	  if (blhe->type == bfd_link_hash_defined)
	    exported_symbol_sections[i] = blhe->u.def.section;
	  else
	    exported_symbol_sections[i] = blhe->u.c.p->section;

	  if (pe_def_file->exports[i].ordinal != -1)
	    {
	      if (max_ordinal < pe_def_file->exports[i].ordinal)
		max_ordinal = pe_def_file->exports[i].ordinal;
	      if (min_ordinal > pe_def_file->exports[i].ordinal)
		min_ordinal = pe_def_file->exports[i].ordinal;
	      count_with_ordinals++;
	    }
	}
      /* Check for forward exports.  These are indicated in DEF files by an
	 export directive of the form NAME1 = MODULE-NAME.EXTERNAL-NAME
	 but we must take care not to be fooled when the user wants to export
	 a symbol that actually really has a dot in it, so we only check
	 for them here, after real defined symbols have already been matched.  */
      else if (strchr (int_name, '.'))
	{
	  count_exported++;
	  if (!pe_def_file->exports[i].flag_noname)
	    count_exported_byname++;

	  pe_def_file->exports[i].flag_forward = 1;

	  if (pe_def_file->exports[i].ordinal != -1)
	    {
	      if (max_ordinal < pe_def_file->exports[i].ordinal)
		max_ordinal = pe_def_file->exports[i].ordinal;
	      if (min_ordinal > pe_def_file->exports[i].ordinal)
		min_ordinal = pe_def_file->exports[i].ordinal;
	      count_with_ordinals++;
	    }
	}
      else if (blhe && blhe->type == bfd_link_hash_undefined)
	{
	  /* xgettext:c-format */
	  einfo (_("%X%P: cannot export %s: symbol not defined\n"),
		 int_name);
	}
      else if (blhe)
	{
	  /* xgettext:c-format */
	  einfo (_("%X%P: cannot export %s: symbol wrong type (%d vs %d)\n"),
		 int_name,
		 blhe->type, bfd_link_hash_defined);
	}
      else
	{
	  /* xgettext:c-format */
	  einfo (_("%X%P: cannot export %s: symbol not found\n"),
		 int_name);
	}
      free (name);
    }
}

/* Build the bfd that will contain .edata and .reloc sections.  */

static void
build_filler_bfd (bool include_edata)
{
  lang_input_statement_type *filler_file;

  filler_file = lang_add_input_file ("dll stuff",
				     lang_input_file_is_fake_enum,
				     NULL);
  filler_file->the_bfd = filler_bfd = bfd_create ("dll stuff",
						  link_info.output_bfd);
  if (filler_bfd == NULL
      || !bfd_set_arch_mach (filler_bfd,
			     bfd_get_arch (link_info.output_bfd),
			     bfd_get_mach (link_info.output_bfd)))
    {
      einfo (_("%F%P: can not create BFD: %E\n"));
      return;
    }

  if (include_edata)
    {
      edata_s = bfd_make_section_old_way (filler_bfd, ".edata");
      if (edata_s == NULL
	  || !bfd_set_section_flags (edata_s, (SEC_HAS_CONTENTS
					       | SEC_ALLOC
					       | SEC_LOAD
					       | SEC_KEEP
					       | SEC_IN_MEMORY)))
	{
	  einfo (_("%X%P: can not create .edata section: %E\n"));
	  return;
	}
      bfd_set_section_size (edata_s, edata_sz);
    }

  reloc_s = bfd_make_section_old_way (filler_bfd, ".reloc");
  if (reloc_s == NULL
      || !bfd_set_section_flags (reloc_s, (SEC_HAS_CONTENTS
					   | SEC_ALLOC
					   | SEC_LOAD
					   | SEC_KEEP
					   | SEC_IN_MEMORY)))
    {
      einfo (_("%X%P: can not create .reloc section: %E\n"));
      return;
    }

  bfd_set_section_size (reloc_s, 0);

  ldlang_add_file (filler_file);
}

/* Gather all the exported symbols and build the .edata section.  */

static void
generate_edata (void)
{
  int i, next_ordinal;
  int name_table_size = 0;

  /* First, we need to know how many exported symbols there are,
     and what the range of ordinals is.  */
  if (count_with_ordinals && max_ordinal > count_exported)
    {
      if (min_ordinal > max_ordinal - count_exported + 1)
	min_ordinal = max_ordinal - count_exported + 1;
    }
  else
    {
      min_ordinal = 1;
      max_ordinal = count_exported;
    }

  export_table_size = max_ordinal - min_ordinal + 1;
  exported_symbols = xmalloc (export_table_size * sizeof (int));
  for (i = 0; i < export_table_size; i++)
    exported_symbols[i] = -1;

  /* Now we need to assign ordinals to those that don't have them.  */
  for (i = 0; i < NE; i++)
    {
      if (exported_symbol_sections[i]
	  || pe_def_file->exports[i].flag_forward)
	{
	  if (pe_def_file->exports[i].ordinal != -1)
	    {
	      int ei = pe_def_file->exports[i].ordinal - min_ordinal;
	      int pi = exported_symbols[ei];

	      if (pi != -1)
		{
		  /* xgettext:c-format */
		  einfo (_("%X%P: error: ordinal used twice: %d (%s vs %s)\n"),
			 pe_def_file->exports[i].ordinal,
			 pe_def_file->exports[i].name,
			 pe_def_file->exports[pi].name);
		}
	      exported_symbols[ei] = i;
	    }
	  if (pe_def_file->exports[i].its_name)
	    name_table_size += strlen (pe_def_file->exports[i].its_name) + 1;
	  else
	    name_table_size += strlen (pe_def_file->exports[i].name) + 1;
	}

      /* Reserve space for the forward name. */
      if (pe_def_file->exports[i].flag_forward)
	{
	  name_table_size += strlen (pe_def_file->exports[i].internal_name) + 1;
	}
    }

  next_ordinal = min_ordinal;
  for (i = 0; i < NE; i++)
    if ((exported_symbol_sections[i]
	 || pe_def_file->exports[i].flag_forward)
	&& pe_def_file->exports[i].ordinal == -1)
      {
	while (exported_symbols[next_ordinal - min_ordinal] != -1)
	  next_ordinal++;

	exported_symbols[next_ordinal - min_ordinal] = i;
	pe_def_file->exports[i].ordinal = next_ordinal;
      }

  /* PR 12969: Check for more than 1^16 ordinals.  */
  if (max_ordinal > 65535 || next_ordinal > 65535)
    /* xgettext:c-format */
    einfo(_("%X%P: error: export ordinal too large: %d\n"),
	  max_ordinal > next_ordinal ? max_ordinal : next_ordinal);

  /* OK, now we can allocate some memory.  */
  edata_sz = (40				/* directory */
	      + 4 * export_table_size		/* addresses */
	      + 4 * count_exported_byname	/* name ptrs */
	      + 2 * count_exported_byname	/* ordinals */
	      + name_table_size + strlen (dll_filename) + 1);
}

/* Fill the exported symbol offsets. The preliminary work has already
   been done in process_def_file_and_drectve().  */

static void
fill_exported_offsets (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *info)
{
  int i;
  struct bfd_link_hash_entry *blhe;

  for (i = 0; i < pe_def_file->num_exports; i++)
    {
      char *name;

      name = xmalloc (strlen (pe_def_file->exports[i].internal_name) + 2);
      if (pe_details->underscored
	  && *pe_def_file->exports[i].internal_name != '@')
	{
	  *name = '_';
	  strcpy (name + 1, pe_def_file->exports[i].internal_name);
	}
      else
	strcpy (name, pe_def_file->exports[i].internal_name);

      blhe = bfd_link_hash_lookup (info->hash,
				   name,
				   false, false, true);

      if (blhe && blhe->type == bfd_link_hash_defined)
	exported_symbol_offsets[i] = blhe->u.def.value;

      free (name);
    }
}

static void
fill_edata (bfd *abfd, struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  int s, hint;
  unsigned char *edirectory;
  unsigned char *eaddresses;
  unsigned char *enameptrs;
  unsigned char *eordinals;
  char *enamestr;

  edata_d = xmalloc (edata_sz);

  /* Note use of array pointer math here.  */
  edirectory = edata_d;
  eaddresses = edirectory + 40;
  enameptrs = eaddresses + 4 * export_table_size;
  eordinals = enameptrs + 4 * count_exported_byname;
  enamestr = (char *) eordinals + 2 * count_exported_byname;

#define ERVA(ptr) (((unsigned char *)(ptr) - edata_d) \
		   + edata_s->output_section->vma - image_base)

  memset (edata_d, 0, edata_sz);

  if (pe_data (abfd)->timestamp == -1)
    H_PUT_32 (abfd, time (0), edata_d + 4);
  else
    H_PUT_32 (abfd, pe_data (abfd)->timestamp, edata_d + 4);

  if (pe_def_file->version_major != -1)
    {
      bfd_put_16 (abfd, pe_def_file->version_major, edata_d + 8);
      bfd_put_16 (abfd, pe_def_file->version_minor, edata_d + 10);
    }

  bfd_put_32 (abfd, ERVA (enamestr), edata_d + 12);
  strcpy (enamestr, dll_filename);
  enamestr += strlen (enamestr) + 1;
  bfd_put_32 (abfd, min_ordinal, edata_d + 16);
  bfd_put_32 (abfd, export_table_size, edata_d + 20);
  bfd_put_32 (abfd, count_exported_byname, edata_d + 24);
  bfd_put_32 (abfd, ERVA (eaddresses), edata_d + 28);
  bfd_put_32 (abfd, ERVA (enameptrs), edata_d + 32);
  bfd_put_32 (abfd, ERVA (eordinals), edata_d + 36);

  fill_exported_offsets (abfd, info);

  /* Ok, now for the filling in part.
     Scan alphabetically - ie the ordering in the exports[] table,
     rather than by ordinal - the ordering in the exported_symbol[]
     table.  See dlltool.c and:
	http://sources.redhat.com/ml/binutils/2003-04/msg00379.html
     for more information.  */
  hint = 0;
  for (s = 0; s < NE; s++)
    {
      struct bfd_section *ssec = exported_symbol_sections[s];
      if (pe_def_file->exports[s].ordinal != -1
	  && (pe_def_file->exports[s].flag_forward || ssec != NULL))
	{
	  int ord = pe_def_file->exports[s].ordinal;

	  if (pe_def_file->exports[s].flag_forward)
	    {
	      bfd_put_32 (abfd, ERVA (enamestr),
			  eaddresses + 4 * (ord - min_ordinal));

	      strcpy (enamestr, pe_def_file->exports[s].internal_name);
	      enamestr += strlen (pe_def_file->exports[s].internal_name) + 1;
	    }
	  else
	    {
	      bfd_vma srva = (exported_symbol_offsets[s]
				    + ssec->output_section->vma
				    + ssec->output_offset);

	      bfd_put_32 (abfd, srva - image_base,
			  eaddresses + 4 * (ord - min_ordinal));
	    }

	  if (!pe_def_file->exports[s].flag_noname)
	    {
	      char *ename = pe_def_file->exports[s].name;
	      if (pe_def_file->exports[s].its_name)
		ename = pe_def_file->exports[s].its_name;

	      bfd_put_32 (abfd, ERVA (enamestr), enameptrs);
	      enameptrs += 4;
	      strcpy (enamestr, ename);
	      enamestr += strlen (enamestr) + 1;
	      bfd_put_16 (abfd, ord - min_ordinal, eordinals);
	      eordinals += 2;
	      pe_def_file->exports[s].hint = hint++;
	    }
	}
    }
}


static struct bfd_section *current_sec;

static void
pe_walk_relocs (struct bfd_link_info *info,
		char *name,
		const char *symname,
		struct bfd_hash_table *import_hash,
		void (*cb) (arelent *, asection *, char *, const char *))
{
  bfd *b;
  asection *s;

  for (b = info->input_bfds; b; b = b->link.next)
    {
      asymbol **symbols;

      if (!bfd_generic_link_read_symbols (b))
	{
	  einfo (_("%F%P: %pB: could not read symbols: %E\n"), b);
	  return;
	}

      symbols = bfd_get_outsymbols (b);

      for (s = b->sections; s; s = s->next)
	{
	  arelent **relocs;
	  int relsize, nrelocs, i;
	  int flags = bfd_section_flags (s);

	  /* Skip discarded linkonce sections.  */
	  if (flags & SEC_LINK_ONCE
	      && s->output_section == bfd_abs_section_ptr)
	    continue;

	  current_sec = s;

	  relsize = bfd_get_reloc_upper_bound (b, s);
	  relocs = xmalloc (relsize);
	  nrelocs = bfd_canonicalize_reloc (b, s, relocs, symbols);

	  for (i = 0; i < nrelocs; i++)
	    {
	      struct bfd_symbol *sym = *relocs[i]->sym_ptr_ptr;

	      /* Warning: the callback needs to be passed NAME directly.  */
	      if (import_hash)
		{
		  if (bfd_hash_lookup (import_hash, sym->name, false, false))
		    {
		      strcpy (name, sym->name);
		      cb (relocs[i], s, name, symname);
		    }
		}
	      else
		{
		  if (strcmp (name, sym->name) == 0)
		    cb (relocs[i], s, name, symname);
		}
	    }

	  free (relocs);

	  /* Warning: the allocated symbols are remembered in BFD and reused
	     later, so don't free them! */
	  /* free (symbols); */
	}
    }
}

void
pe_find_data_imports (const char *symhead,
		      void (*cb) (arelent *, asection *, char *, const char *))
{
  struct bfd_link_hash_entry *undef;
  const size_t headlen = strlen (symhead);
  size_t namelen = 0;
  char *buf, *name;
  struct bfd_hash_table *import_hash;

  for (undef = link_info.hash->undefs; undef; undef = undef->u.undef.next)
    if (undef->type == bfd_link_hash_undefined)
      {
	size_t len = strlen (undef->root.string);
	if (namelen < len)
	  namelen = len;
      }
  if (namelen == 0)
    return;

  /* For the pseudo-relocation support version 2, we can collect the symbols
     that are subject to auto-import and adjust the relocations en masse.  */
  if (link_info.pei386_runtime_pseudo_reloc == 2)
    {
      import_hash
	= (struct bfd_hash_table *) xmalloc (sizeof (struct bfd_hash_table));
      if (!bfd_hash_table_init (import_hash,
				bfd_hash_newfunc,
				sizeof (struct bfd_hash_entry)))
	einfo (_("%F%P: bfd_hash_table_init failed: %E\n"));
    }
  else
    import_hash = NULL;

  /* We are being a bit cunning here.  The buffer will have space for
     prefixes at the beginning.  The prefix is modified here and in a
     number of functions called from this function.  */
#define PREFIX_LEN 32
  buf = xmalloc (PREFIX_LEN + namelen + 1);
  name = buf + PREFIX_LEN;

  for (undef = link_info.hash->undefs; undef; undef = undef->u.undef.next)
    if (undef->type == bfd_link_hash_undefined)
      {
	struct bfd_link_hash_entry *sym;
	char *impname;

	if (pe_dll_extra_pe_debug)
	  printf ("%s:%s\n", __func__, undef->root.string);

	strcpy (name, undef->root.string);
	impname = name - (sizeof "__imp_" - 1);
	memcpy (impname, "__imp_", sizeof "__imp_" - 1);

	sym = bfd_link_hash_lookup (link_info.hash, impname, 0, 0, 1);

	if (sym && sym->type == bfd_link_hash_defined)
	  {
	    if (import_hash)
	      bfd_hash_lookup (import_hash, undef->root.string, true, false);
	    else
	      {
		bfd *b = sym->u.def.section->owner;
		const char *symname = NULL;
		asymbol **symbols;
		int nsyms, i;

		if (!bfd_generic_link_read_symbols (b))
		  {
		    einfo (_("%F%P: %pB: could not read symbols: %E\n"), b);
		    return;
		  }

		symbols = bfd_get_outsymbols (b);
		nsyms = bfd_get_symcount (b);

		for (i = 0; i < nsyms; i++)
		  if (strncmp (symbols[i]->name, symhead, headlen) == 0)
		    {
		      if (pe_dll_extra_pe_debug)
			printf ("->%s\n", symbols[i]->name);

		      symname = symbols[i]->name + headlen;
		      break;
		    }

		/* If the symobl isn't part of an import table, there is no
		   point in building a fixup, this would give rise to link
		   errors for mangled symbols instead of the original one.  */
		if (symname)
		  pe_walk_relocs (&link_info, name, symname, NULL, cb);
		else
		  continue;
	      }

	    /* Let's differentiate it somehow from defined.  */
	    undef->type = bfd_link_hash_defweak;
	    undef->u.def.value = sym->u.def.value;
	    undef->u.def.section = sym->u.def.section;

	    /* We replace the original name with the __imp_ prefixed one, this
	       1) may trash memory 2) leads to duplicate symbols.  But this is
	       better than having a misleading name that can confuse GDB.  */
	    undef->root.string = sym->root.string;

	    if (link_info.pei386_auto_import == -1)
	      {
		static bool warned = false;

		info_msg (_("Info: resolving %s by linking to %s "
			    "(auto-import)\n"), name, impname);

		/* PR linker/4844.  */
		if (!warned)
		  {
		    einfo (_("%P: warning: auto-importing has been activated "
			     "without --enable-auto-import specified on the "
			     "command line; this should work unless it "
			     "involves constant data structures referencing "
			     "symbols from auto-imported DLLs\n"));
		    warned = true;
		  }
	      }
	  }
      }

  /* If we have the import hash table, walk the relocations only once.  */
  if (import_hash)
    {
      pe_walk_relocs (&link_info, name, NULL, import_hash, cb);
      bfd_hash_table_free (import_hash);
      free (import_hash);
    }

  free (buf);
}

/* Gather all the relocations and build the .reloc section.  */

static void
generate_reloc (bfd *abfd, struct bfd_link_info *info)
{
  /* For .reloc stuff.  */
  reloc_data_type *reloc_data;
  int total_relocs = 0;
  int i;
  bfd_vma sec_page = (bfd_vma) -1;
  bfd_vma page_ptr;
  bfd *b;
  struct bfd_section *s;

  if (reloc_s == NULL || reloc_s->output_section == bfd_abs_section_ptr)
    return;

  /* Set an upper bound for the total number of relocations we will have to generate.  */
  total_relocs = 0;
  for (b = info->input_bfds; b; b = b->link.next)
    for (s = b->sections; s; s = s->next)
      total_relocs += s->reloc_count;

  reloc_data = xmalloc (total_relocs * sizeof (reloc_data_type));

  total_relocs = 0;
  for (b = info->input_bfds; b; b = b->link.next)
    {
      arelent **relocs;
      int relsize, nrelocs;
      asymbol **symbols;

      if (!bfd_generic_link_read_symbols (b))
	{
	  einfo (_("%F%P: %pB: could not read symbols: %E\n"), b);
	  return;
	}

      symbols = bfd_get_outsymbols (b);

      for (s = b->sections; s; s = s->next)
	{
	  bfd_vma sec_vma;

	  /* If the section is not going to be output, then ignore it.  */
	  if (s->output_section == NULL)
	    {
	      /* FIXME: This should not happen.  Convert to the correct
		 form here, but really, this should be investigated.  */
	      s->output_section = bfd_abs_section_ptr;
	      continue;
	    }

	  /* I don't know why there would be a reloc for these, but I've
	     seen it happen - DJ  */
	  if (s->output_section == bfd_abs_section_ptr)
	    continue;

	  /* If it's not loaded, we don't need to relocate it this way.  */
	  if (!(s->output_section->flags & SEC_LOAD))
	    continue;

	  /* This happens when linking with --just-symbols=<file>
	     so do not generate an error.  */
	  if (s->output_section->vma == 0)
	    continue;

	  sec_vma = s->output_section->vma + s->output_offset;

	  relsize = bfd_get_reloc_upper_bound (b, s);
	  relocs = xmalloc (relsize);
	  nrelocs = bfd_canonicalize_reloc (b, s, relocs, symbols);

	  for (i = 0; i < nrelocs; i++)
	    {
	      if (pe_dll_extra_pe_debug)
		{
		  struct bfd_symbol *sym = *relocs[i]->sym_ptr_ptr;
		  printf ("rel: %s\n", sym->name);
		}
	      if (!relocs[i]->howto->pc_relative
		  && relocs[i]->howto->type != pe_details->imagebase_reloc)
		{
		  struct bfd_symbol *sym = *relocs[i]->sym_ptr_ptr;
		  const struct bfd_link_hash_entry *blhe
		    = bfd_wrapped_link_hash_lookup (abfd, info, sym->name,
						    false, false, false);

		  /* Don't create relocs for undefined weak symbols.  */
		  if (sym->flags == BSF_WEAK)
		    {
		      if (blhe && blhe->type == bfd_link_hash_undefweak)
			{
			  /* Check aux sym and see if it is defined or not. */
			  struct coff_link_hash_entry *h, *h2;
			  h = (struct coff_link_hash_entry *)blhe;
			  if (h->symbol_class != C_NT_WEAK || h->numaux != 1)
			    continue;
			  h2 = h->auxbfd->tdata.coff_obj_data->sym_hashes
						[h->aux->x_sym.x_tagndx.u32];
			  /* We don't want a base reloc if the aux sym is not
			     found, undefined, or if it is the constant ABS
			     zero default value.  (We broaden that slightly by
			     not testing the value, just the section; there's
			     no reason we'd want a reference to any absolute
			     address to get relocated during rebasing).  */
			  if (!h2 || h2->root.type == bfd_link_hash_undefined
				|| h2->root.u.def.section == bfd_abs_section_ptr)
			    continue;
			}
		      else if (!blhe || blhe->type != bfd_link_hash_defined)
			continue;
		    }
		  /* Nor for Dwarf FDE references to discarded sections.  */
		  else if (bfd_is_abs_section (sym->section->output_section))
		    {
		      /* We only ignore relocs from .eh_frame sections, as
			 they are discarded by the final link rather than
			 resolved against the kept section.  */
		      if (!strcmp (s->name, ".eh_frame"))
			continue;
		    }
		  /* Nor for absolute symbols.  */
		  else if (blhe && ldexp_is_final_sym_absolute (blhe)
			   && (!blhe->linker_def
			       || (strcmp (sym->name, "__image_base__")
				   && strcmp (sym->name, U ("__ImageBase")))))
		    continue;

		  reloc_data[total_relocs].vma = sec_vma + relocs[i]->address;
		  reloc_data[total_relocs].idx = total_relocs;

		  /* Since we're only about to determine .reloc's size,
		     subsequent output section VMA calculations will shift up
		     sections at this or higher addresses.  Relocations for
		     such sections would hence end up not being correct.  */
		  if (reloc_data[total_relocs].vma
		      >= reloc_s->output_section->vma)
		    einfo (_("%P: base relocation for section `%s' above "
			     ".reloc section\n"), s->output_section->name);

#define BITS_AND_SHIFT(bits, shift) (bits * 1000 | shift)

		  switch BITS_AND_SHIFT (relocs[i]->howto->bitsize,
					 relocs[i]->howto->rightshift)
		    {
#ifdef pe_use_plus
		    case BITS_AND_SHIFT (64, 0):
		      reloc_data[total_relocs].type = IMAGE_REL_BASED_DIR64;
		      total_relocs++;
		      break;
#endif
		    case BITS_AND_SHIFT (32, 0):
		      reloc_data[total_relocs].type = IMAGE_REL_BASED_HIGHLOW;
		      total_relocs++;
		      break;
		    case BITS_AND_SHIFT (16, 0):
		      reloc_data[total_relocs].type = IMAGE_REL_BASED_LOW;
		      total_relocs++;
		      break;
		    case BITS_AND_SHIFT (16, 16):
		      reloc_data[total_relocs].type = IMAGE_REL_BASED_HIGHADJ;
		      /* FIXME: we can't know the symbol's right value
			 yet, but we probably can safely assume that
			 CE will relocate us in 64k blocks, so leaving
			 it zero is safe.  */
		      reloc_data[total_relocs].extra = 0;
		      total_relocs++;
		      break;
		    case BITS_AND_SHIFT (26, 2):
		      reloc_data[total_relocs].type =
                        IMAGE_REL_BASED_ARM_MOV32;
		      total_relocs++;
		      break;
		    case BITS_AND_SHIFT (24, 2):
		      /* FIXME: 0 is ARM_26D, it is defined in bfd/coff-arm.c
			 Those ARM_xxx definitions should go in proper
			 header someday.  */
		      if (relocs[i]->howto->type == 0
			  /* Older GNU linkers used 5 instead of 0 for this reloc.  */
			  || relocs[i]->howto->type == 5)
			/* This is an ARM_26D reloc, which is an ARM_26 reloc
			   that has already been fully processed during a
			   previous link stage, so ignore it here.  */
			break;
		      /* Fall through.  */
		    default:
		      /* xgettext:c-format */
		      einfo (_("%X%P: error: %d-bit reloc in dll\n"),
			     relocs[i]->howto->bitsize);
		      break;
		    }
		}
	    }

	  free (relocs);
	  /* Warning: the allocated symbols are remembered in BFD and
	     reused later, so don't free them!  */
	}
    }

  /* This can happen for example when LTO has eliminated all code.  */
  if (total_relocs == 0)
    return;
  
  /* At this point, we have total_relocs relocation addresses in
     reloc_addresses, which are all suitable for the .reloc section.
     We must now create the new sections.  */
  qsort (reloc_data, total_relocs, sizeof (*reloc_data), reloc_sort);

  for (i = 0; i < total_relocs; i++)
    {
      bfd_vma this_page = (reloc_data[i].vma >> 12);

      if (this_page != sec_page)
	{
	  reloc_sz = (reloc_sz + 3) & ~3;	/* 4-byte align.  */
	  reloc_sz += 8;
	  sec_page = this_page;
	}

      reloc_sz += 2;

      if (reloc_data[i].type == IMAGE_REL_BASED_HIGHADJ)
	reloc_sz += 2;
    }

  reloc_sz = (reloc_sz + 3) & ~3;	/* 4-byte align.  */
  reloc_d = xmalloc (reloc_sz);

  page_ptr = sec_page = (bfd_vma) -1;
  reloc_sz = 0;

  for (i = 0; i < total_relocs; i++)
    {
      bfd_vma rva = reloc_data[i].vma - image_base;
      bfd_vma this_page = (rva & ~0xfff);

      if (this_page != sec_page)
	{
	  while (reloc_sz & 3)
	    reloc_d[reloc_sz++] = 0;

	  if (page_ptr != (bfd_vma) -1)
	    bfd_put_32 (abfd, reloc_sz - page_ptr, reloc_d + page_ptr + 4);

	  bfd_put_32 (abfd, this_page, reloc_d + reloc_sz);
	  page_ptr = reloc_sz;
	  reloc_sz += 8;
	  sec_page = this_page;
	}

      bfd_put_16 (abfd, (rva & 0xfff) + (reloc_data[i].type << 12),
		  reloc_d + reloc_sz);
      reloc_sz += 2;

      if (reloc_data[i].type == IMAGE_REL_BASED_HIGHADJ)
	{
	  bfd_put_16 (abfd, reloc_data[i].extra, reloc_d + reloc_sz);
	  reloc_sz += 2;
	}
    }

  while (reloc_sz & 3)
    reloc_d[reloc_sz++] = 0;

  if (page_ptr != (bfd_vma) -1)
    bfd_put_32 (abfd, reloc_sz - page_ptr, reloc_d + page_ptr + 4);
}

/* Given the exiting def_file structure, print out a .DEF file that
   corresponds to it.  */

static void
quoteput (char *s, FILE *f, int needs_quotes)
{
  char *cp;

  for (cp = s; *cp; cp++)
    if (*cp == '\''
	|| *cp == '"'
	|| *cp == '\\'
	|| ISSPACE (*cp)
	|| *cp == ','
	|| *cp == ';')
      needs_quotes = 1;

  if (needs_quotes)
    {
      putc ('"', f);

      while (*s)
	{
	  if (*s == '"' || *s == '\\')
	    putc ('\\', f);

	  putc (*s, f);
	  s++;
	}

      putc ('"', f);
    }
  else
    fputs (s, f);
}

void
pe_dll_generate_def_file (const char *pe_out_def_filename)
{
  int i;
  FILE *out = fopen (pe_out_def_filename, "w");

  if (out == NULL)
    /* xgettext:c-format */
    einfo (_("%P: can't open output def file %s\n"),
	   pe_out_def_filename);

  if (pe_def_file)
    {
      if (pe_def_file->name)
	{
	  if (pe_def_file->is_dll)
	    fprintf (out, "LIBRARY ");
	  else
	    fprintf (out, "NAME ");

	  quoteput (pe_def_file->name, out, 1);

	  if (pe_data (link_info.output_bfd)->pe_opthdr.ImageBase)
	    fprintf (out, " BASE=0x%" PRIx64,
		     (uint64_t) pe_data (link_info.output_bfd)->pe_opthdr.ImageBase);
	  fprintf (out, "\n");
	}

      if (pe_def_file->description)
	{
	  fprintf (out, "DESCRIPTION ");
	  quoteput (pe_def_file->description, out, 1);
	  fprintf (out, "\n");
	}

      if (pe_def_file->version_minor != -1)
	fprintf (out, "VERSION %d.%d\n", pe_def_file->version_major,
		 pe_def_file->version_minor);
      else if (pe_def_file->version_major != -1)
	fprintf (out, "VERSION %d\n", pe_def_file->version_major);

      if (pe_def_file->stack_reserve != -1 || pe_def_file->heap_reserve != -1)
	fprintf (out, "\n");

      if (pe_def_file->stack_commit != -1)
	fprintf (out, "STACKSIZE 0x%x,0x%x\n",
		 pe_def_file->stack_reserve, pe_def_file->stack_commit);
      else if (pe_def_file->stack_reserve != -1)
	fprintf (out, "STACKSIZE 0x%x\n", pe_def_file->stack_reserve);

      if (pe_def_file->heap_commit != -1)
	fprintf (out, "HEAPSIZE 0x%x,0x%x\n",
		 pe_def_file->heap_reserve, pe_def_file->heap_commit);
      else if (pe_def_file->heap_reserve != -1)
	fprintf (out, "HEAPSIZE 0x%x\n", pe_def_file->heap_reserve);

      if (pe_def_file->num_section_defs > 0)
	{
	  fprintf (out, "\nSECTIONS\n\n");

	  for (i = 0; i < pe_def_file->num_section_defs; i++)
	    {
	      fprintf (out, "    ");
	      quoteput (pe_def_file->section_defs[i].name, out, 0);

	      if (pe_def_file->section_defs[i].class)
		{
		  fprintf (out, " CLASS ");
		  quoteput (pe_def_file->section_defs[i].class, out, 0);
		}

	      if (pe_def_file->section_defs[i].flag_read)
		fprintf (out, " READ");

	      if (pe_def_file->section_defs[i].flag_write)
		fprintf (out, " WRITE");

	      if (pe_def_file->section_defs[i].flag_execute)
		fprintf (out, " EXECUTE");

	      if (pe_def_file->section_defs[i].flag_shared)
		fprintf (out, " SHARED");

	      fprintf (out, "\n");
	    }
	}

      if (pe_def_file->num_exports > 0)
	{
	  fprintf (out, "EXPORTS\n");

	  for (i = 0; i < pe_def_file->num_exports; i++)
	    {
	      def_file_export *e = pe_def_file->exports + i;
	      fprintf (out, "    ");
	      quoteput (e->name, out, 0);

	      if (e->internal_name && strcmp (e->internal_name, e->name))
		{
		  fprintf (out, " = ");
		  quoteput (e->internal_name, out, 0);
		}

	      if (e->ordinal != -1)
		fprintf (out, " @%d", e->ordinal);

	      if (e->flag_private)
		fprintf (out, " PRIVATE");

	      if (e->flag_constant)
		fprintf (out, " CONSTANT");

	      if (e->flag_noname)
		fprintf (out, " NONAME");

	      if (e->flag_data)
		fprintf (out, " DATA");

	      fprintf (out, "\n");
	    }
	}

      if (pe_def_file->num_imports > 0)
	{
	  fprintf (out, "\nIMPORTS\n\n");

	  for (i = 0; i < pe_def_file->num_imports; i++)
	    {
	      def_file_import *im = pe_def_file->imports + i;
	      fprintf (out, "    ");

	      if (im->internal_name
		  && (!im->name || strcmp (im->internal_name, im->name)))
		{
		  quoteput (im->internal_name, out, 0);
		  fprintf (out, " = ");
		}

	      quoteput (im->module->name, out, 0);
	      fprintf (out, ".");

	      if (im->name)
		quoteput (im->name, out, 0);
	      else
		fprintf (out, "%d", im->ordinal);

	      if (im->its_name)
		{
		  fprintf (out, " == ");
		  quoteput (im->its_name, out, 0);
		}

	      fprintf (out, "\n");
	    }
	}
    }
  else
    fprintf (out, _("; no contents available\n"));

  if (fclose (out) == EOF)
    /* xgettext:c-format */
    einfo (_("%P: error closing file `%s'\n"), pe_out_def_filename);
}

/* Generate the import library.  */

static asymbol **symtab;
static int symptr;
static int tmp_seq;
static char *dll_symname;

#define UNDSEC bfd_und_section_ptr

static asection *
quick_section (bfd *abfd, const char *name, int flags, int align)
{
  asection *sec;
  asymbol *sym;

  sec = bfd_make_section_old_way (abfd, name);
  bfd_set_section_flags (sec, flags | SEC_ALLOC | SEC_LOAD | SEC_KEEP);
  bfd_set_section_alignment (sec, align);
  /* Remember to undo this before trying to link internally!  */
  sec->output_section = sec;

  sym = bfd_make_empty_symbol (abfd);
  symtab[symptr++] = sym;
  sym->name = sec->name;
  sym->section = sec;
  sym->flags = BSF_LOCAL;
  sym->value = 0;

  return sec;
}

static void
quick_symbol (bfd *abfd,
	      const char *n1,
	      const char *n2,
	      const char *n3,
	      asection *sec,
	      int flags,
	      int addr)
{
  asymbol *sym;
  char *name = xmalloc (strlen (n1) + strlen (n2) + strlen (n3) + 1);

  strcpy (name, n1);
  strcat (name, n2);
  strcat (name, n3);
  sym = bfd_make_empty_symbol (abfd);
  sym->name = name;
  sym->section = sec;
  sym->flags = flags;
  sym->value = addr;
  symtab[symptr++] = sym;
}

static arelent *reltab = 0;
static int relcount = 0, relsize = 0;

static void
quick_reloc (bfd *abfd, bfd_size_type address, int which_howto, int symidx)
{
  if (relcount >= relsize - 1)
    {
      relsize += 10;
      if (reltab)
	reltab = xrealloc (reltab, relsize * sizeof (arelent));
      else
	reltab = xmalloc (relsize * sizeof (arelent));
    }
  reltab[relcount].address = address;
  reltab[relcount].addend = 0;
  reltab[relcount].howto = bfd_reloc_type_lookup (abfd, which_howto);
  reltab[relcount].sym_ptr_ptr = symtab + symidx;
  relcount++;
}

static void
save_relocs (asection *sec)
{
  int i;

  sec->relocation = reltab;
  sec->reloc_count = relcount;
  sec->orelocation = xmalloc ((relcount + 1) * sizeof (arelent *));
  for (i = 0; i < relcount; i++)
    sec->orelocation[i] = sec->relocation + i;
  sec->orelocation[relcount] = 0;
  sec->flags |= SEC_RELOC;
  reltab = 0;
  relcount = relsize = 0;
}

/*	.section	.idata$2
	.global		__head_my_dll
   __head_my_dll:
	.rva		hname
	.long		0
	.long		0
	.rva		__my_dll_iname
	.rva		fthunk

	.section	.idata$5
	.long		0
   fthunk:

	.section	.idata$4
	.long		0
   hname:                              */

static bfd *
make_head (bfd *parent)
{
  asection *id2, *id5, *id4;
  unsigned char *d2, *d5, *d4;
  char *oname;
  bfd *abfd;

  if (asprintf (&oname, "%s_d%06d.o", dll_symname, tmp_seq) < 4)
    /* In theory we should return NULL here at let our caller decide what to
       do.  But currently the return value is not checked, just used, and
       besides, this condition only happens when the system has run out of
       memory.  So just give up.  */
    exit (EXIT_FAILURE);
  tmp_seq++;

  abfd = bfd_create (oname, parent);
  free (oname);
  bfd_find_target (pe_details->object_target, abfd);
  bfd_make_writable (abfd);

  bfd_set_format (abfd, bfd_object);
  bfd_set_arch_mach (abfd, pe_details->bfd_arch, 0);

  symptr = 0;
  symtab = xmalloc (6 * sizeof (asymbol *));
  id2 = quick_section (abfd, ".idata$2", SEC_HAS_CONTENTS, 2);
  id5 = quick_section (abfd, ".idata$5", SEC_HAS_CONTENTS, 2);
  id4 = quick_section (abfd, ".idata$4", SEC_HAS_CONTENTS, 2);
  quick_symbol (abfd, U ("_head_"), dll_symname, "", id2, BSF_GLOBAL, 0);
  quick_symbol (abfd, U (""), dll_symname, "_iname", UNDSEC, BSF_GLOBAL, 0);

  /* OK, pay attention here.  I got confused myself looking back at
     it.  We create a four-byte section to mark the beginning of the
     list, and we include an offset of 4 in the section, so that the
     pointer to the list points to the *end* of this section, which is
     the start of the list of sections from other objects.  */

  bfd_set_section_size (id2, 20);
  d2 = xmalloc (20);
  id2->contents = d2;
  memset (d2, 0, 20);
  if (pe_use_nul_prefixed_import_tables)
    d2[0] = d2[16] = PE_IDATA5_SIZE; /* Reloc addend.  */
  quick_reloc (abfd,  0, BFD_RELOC_RVA, 2);
  quick_reloc (abfd, 12, BFD_RELOC_RVA, 4);
  quick_reloc (abfd, 16, BFD_RELOC_RVA, 1);
  save_relocs (id2);

  if (pe_use_nul_prefixed_import_tables)
    bfd_set_section_size (id5, PE_IDATA5_SIZE);
  else
    bfd_set_section_size (id5, 0);
  d5 = xmalloc (PE_IDATA5_SIZE);
  id5->contents = d5;
  memset (d5, 0, PE_IDATA5_SIZE);
  if (pe_use_nul_prefixed_import_tables)
    bfd_set_section_size (id4, PE_IDATA4_SIZE);
  else
    bfd_set_section_size (id4, 0);
  d4 = xmalloc (PE_IDATA4_SIZE);
  id4->contents = d4;
  memset (d4, 0, PE_IDATA4_SIZE);

  bfd_set_symtab (abfd, symtab, symptr);

  bfd_set_section_contents (abfd, id2, d2, 0, 20);
  if (pe_use_nul_prefixed_import_tables)
    {
      bfd_set_section_contents (abfd, id5, d5, 0, PE_IDATA5_SIZE);
      bfd_set_section_contents (abfd, id4, d4, 0, PE_IDATA4_SIZE);
    }
  else
    {
      bfd_set_section_contents (abfd, id5, d5, 0, 0);
      bfd_set_section_contents (abfd, id4, d4, 0, 0);
    }

  bfd_make_readable (abfd);
  return abfd;
}

/*	.section	.idata$4
	.long		0
	[.long		0] for PE+
	.section	.idata$5
	.long		0
	[.long		0] for PE+
	.section	idata$7
	.global		__my_dll_iname
  __my_dll_iname:
	.asciz		"my.dll"       */

static bfd *
make_tail (bfd *parent)
{
  asection *id4, *id5, *id7;
  unsigned char *d4, *d5, *d7;
  int len;
  char *oname;
  bfd *abfd;

  if (asprintf (&oname, "%s_d%06d.o", dll_symname, tmp_seq) < 4)
    /* In theory we should return NULL here at let our caller decide what to
       do.  But currently the return value is not checked, just used, and
       besides, this condition only happens when the system has run out of
       memory.  So just give up.  */
    exit (EXIT_FAILURE);
  tmp_seq++;

  abfd = bfd_create (oname, parent);
  free (oname);
  bfd_find_target (pe_details->object_target, abfd);
  bfd_make_writable (abfd);

  bfd_set_format (abfd, bfd_object);
  bfd_set_arch_mach (abfd, pe_details->bfd_arch, 0);

  symptr = 0;
  symtab = xmalloc (5 * sizeof (asymbol *));
  id4 = quick_section (abfd, ".idata$4", SEC_HAS_CONTENTS, 2);
  id5 = quick_section (abfd, ".idata$5", SEC_HAS_CONTENTS, 2);
  id7 = quick_section (abfd, ".idata$7", SEC_HAS_CONTENTS, 2);
  quick_symbol (abfd, U (""), dll_symname, "_iname", id7, BSF_GLOBAL, 0);

  bfd_set_section_size (id4, PE_IDATA4_SIZE);
  d4 = xmalloc (PE_IDATA4_SIZE);
  id4->contents = d4;
  memset (d4, 0, PE_IDATA4_SIZE);

  bfd_set_section_size (id5, PE_IDATA5_SIZE);
  d5 = xmalloc (PE_IDATA5_SIZE);
  id5->contents = d5;
  memset (d5, 0, PE_IDATA5_SIZE);

  len = strlen (dll_filename) + 1;
  if (len & 1)
    len++;
  bfd_set_section_size (id7, len);
  d7 = xmalloc (len);
  id7->contents = d7;
  strcpy ((char *) d7, dll_filename);
  /* If len was odd, the above
     strcpy leaves behind an undefined byte. That is harmless,
     but we set it to 0 just so the binary dumps are pretty.  */
  d7[len - 1] = 0;

  bfd_set_symtab (abfd, symtab, symptr);

  bfd_set_section_contents (abfd, id4, d4, 0, PE_IDATA4_SIZE);
  bfd_set_section_contents (abfd, id5, d5, 0, PE_IDATA5_SIZE);
  bfd_set_section_contents (abfd, id7, d7, 0, len);

  bfd_make_readable (abfd);
  return abfd;
}

/*	.text
	.global		_function
	.global		___imp_function
	.global		__imp__function
  _function:
	jmp		*__imp__function:

	.section	idata$7
	.long		__head_my_dll

	.section	.idata$5
  ___imp_function:
  __imp__function:
  iat?
	.section	.idata$4
  iat?
	.section	.idata$6
  ID<ordinal>:
	.short		<hint>
	.asciz		"function" xlate? (add underscore, kill at)  */

static const unsigned char jmp_ix86_bytes[] =
{
  0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90
};

static const unsigned char jmp_aarch64_bytes[] =
{
  0x10, 0x00, 0x00, 0x90, /* adrp x16, 0        */
  0x10, 0x02, 0x00, 0x91, /* add x16, x16, #0x0 */
  0x10, 0x02, 0x40, 0xf9, /* ldr x16, [x16]     */
  0x00, 0x02, 0x1f, 0xd6  /* br x16             */
};

/* _function:
	mov.l	ip+8,r0
	mov.l	@r0,r0
	jmp	@r0
	nop
	.dw	__imp_function   */

static const unsigned char jmp_sh_bytes[] =
{
  0x01, 0xd0, 0x02, 0x60, 0x2b, 0x40, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* _function:
	lui	$t0,<high:__imp_function>
	lw	$t0,<low:__imp_function>
	jr	$t0
	nop                              */

static const unsigned char jmp_mips_bytes[] =
{
  0x00, 0x00, 0x08, 0x3c,  0x00, 0x00, 0x08, 0x8d,
  0x08, 0x00, 0x00, 0x01,  0x00, 0x00, 0x00, 0x00
};

static const unsigned char jmp_arm_bytes[] =
{
  0x00, 0xc0, 0x9f, 0xe5,	/* ldr  ip, [pc] */
  0x00, 0xf0, 0x9c, 0xe5,	/* ldr  pc, [ip] */
  0,    0,    0,    0
};


static bfd *
make_one (def_file_export *exp, bfd *parent, bool include_jmp_stub)
{
  asection *tx, *id7, *id5, *id4, *id6;
  unsigned char *td = NULL, *d7, *d5, *d4, *d6 = NULL;
  int len;
  char *oname;
  bfd *abfd;
  const unsigned char *jmp_bytes = NULL;
  int jmp_byte_count = 0;
  const char *internal_name = exp->internal_name;

  if (!exp->flag_noname)
    {
      /* Check for a decorated symbol name */
      struct decoration_hash_entry *entry;

      entry = (struct decoration_hash_entry *)
	      bfd_hash_lookup (&(coff_hash_table (&link_info)->decoration_hash),
			       internal_name, false, false);
      if (entry)
	{
	  if (entry->decorated_link)
	    {
	      internal_name = entry->decorated_link->root.string;

	      if (pe_details->underscored && internal_name[0] == '_')
		internal_name++;
	    }
	  else
	    {
	      einfo (_("%P: error: NULL decorated name for %s\n"), internal_name);
	    }
	}
    }

  /* Include the jump stub section only if it is needed. A jump
     stub is needed if the symbol being imported <sym> is a function
     symbol and there is at least one undefined reference to that
     symbol. In other words, if all the import references to <sym> are
     explicitly through _declspec(dllimport) then the jump stub is not
     needed.  */
  if (include_jmp_stub)
    {
      switch (pe_details->pe_arch)
	{
	case PE_ARCH_i386:
	  jmp_bytes = jmp_ix86_bytes;
	  jmp_byte_count = sizeof (jmp_ix86_bytes);
	  break;
	case PE_ARCH_sh:
	  jmp_bytes = jmp_sh_bytes;
	  jmp_byte_count = sizeof (jmp_sh_bytes);
	  break;
	case PE_ARCH_mips:
	  jmp_bytes = jmp_mips_bytes;
	  jmp_byte_count = sizeof (jmp_mips_bytes);
	  break;
	case PE_ARCH_arm:
	case PE_ARCH_arm_wince:
	  jmp_bytes = jmp_arm_bytes;
	  jmp_byte_count = sizeof (jmp_arm_bytes);
	  break;
	case PE_ARCH_aarch64:
	  jmp_bytes = jmp_aarch64_bytes;
	  jmp_byte_count = sizeof (jmp_aarch64_bytes);
	  break;
	default:
	  abort ();
	}
    }

  if (asprintf (&oname, "%s_d%06d.o", dll_symname, tmp_seq) < 4)
    /* In theory we should return NULL here at let our caller decide what to
       do.  But currently the return value is not checked, just used, and
       besides, this condition only happens when the system has run out of
       memory.  So just give up.  */
    exit (EXIT_FAILURE);
  tmp_seq++;

  abfd = bfd_create (oname, parent);
  free (oname);
  bfd_find_target (pe_details->object_target, abfd);
  bfd_make_writable (abfd);

  bfd_set_format (abfd, bfd_object);
  bfd_set_arch_mach (abfd, pe_details->bfd_arch, 0);

  symptr = 0;
  symtab = xmalloc (12 * sizeof (asymbol *));

  tx  = quick_section (abfd, ".text", SEC_CODE | SEC_HAS_CONTENTS | SEC_READONLY, 2);
  id7 = quick_section (abfd, ".idata$7", SEC_HAS_CONTENTS, 2);
  id5 = quick_section (abfd, ".idata$5", SEC_HAS_CONTENTS, 2);
  id4 = quick_section (abfd, ".idata$4", SEC_HAS_CONTENTS, 2);
  id6 = quick_section (abfd, ".idata$6", SEC_HAS_CONTENTS, 2);

  if  (*internal_name == '@')
    {
      quick_symbol (abfd, U ("_head_"), dll_symname, "", UNDSEC,
		    BSF_GLOBAL, 0);
      if (include_jmp_stub)
	quick_symbol (abfd, "", internal_name, "", tx, BSF_GLOBAL, 0);
      quick_symbol (abfd, "__imp_", internal_name, "", id5,
		    BSF_GLOBAL, 0);
      /* Fastcall applies only to functions,
	 so no need for auto-import symbol.  */
    }
  else
    {
      quick_symbol (abfd, U ("_head_"), dll_symname, "", UNDSEC,
		    BSF_GLOBAL, 0);
      if (include_jmp_stub)
	quick_symbol (abfd, U (""), internal_name, "", tx,
		      BSF_GLOBAL, 0);
      quick_symbol (abfd, "__imp_", U (""), internal_name, id5,
		    BSF_GLOBAL, 0);
      /* Symbol to reference ord/name of imported
	 data symbol, used to implement auto-import.  */
      if (exp->flag_data)
	quick_symbol (abfd, "__nm_", U (""), internal_name, id6,
		      BSF_GLOBAL,0);
    }
  if (pe_dll_compat_implib)
    quick_symbol (abfd, "___imp_", internal_name, "", id5,
		  BSF_GLOBAL, 0);

  if (include_jmp_stub)
    {
      bfd_set_section_size (tx, jmp_byte_count);
      td = xmalloc (jmp_byte_count);
      tx->contents = td;
      memcpy (td, jmp_bytes, jmp_byte_count);

      switch (pe_details->pe_arch)
	{
	case PE_ARCH_i386:
#ifdef pe_use_plus
	  quick_reloc (abfd, 2, BFD_RELOC_32_PCREL, 2);
#else
	  /* Mark this object as SAFESEH compatible.  */
	  quick_symbol (abfd, "", "@feat.00", "", bfd_abs_section_ptr,
			BSF_LOCAL, 1);
	  quick_reloc (abfd, 2, BFD_RELOC_32, 2);
#endif
	  break;
	case PE_ARCH_sh:
	  quick_reloc (abfd, 8, BFD_RELOC_32, 2);
	  break;
	case PE_ARCH_mips:
	  quick_reloc (abfd, 0, BFD_RELOC_HI16_S, 2);
	  quick_reloc (abfd, 0, BFD_RELOC_LO16, 0); /* MIPS_R_PAIR */
	  quick_reloc (abfd, 4, BFD_RELOC_LO16, 2);
	  break;
	case PE_ARCH_arm:
	case PE_ARCH_arm_wince:
	  quick_reloc (abfd, 8, BFD_RELOC_32, 2);
	  break;
	case PE_ARCH_aarch64:
	  quick_reloc (abfd, 0, BFD_RELOC_AARCH64_ADR_HI21_NC_PCREL, 2);
	  quick_reloc (abfd, 4, BFD_RELOC_AARCH64_ADD_LO12, 2);
	  break;
	default:
	  abort ();
	}
      save_relocs (tx);
    }
  else
    bfd_set_section_size (tx, 0);

  bfd_set_section_size (id7, 4);
  d7 = xmalloc (4);
  id7->contents = d7;
  memset (d7, 0, 4);
  quick_reloc (abfd, 0, BFD_RELOC_RVA, 5);
  save_relocs (id7);

  bfd_set_section_size (id5, PE_IDATA5_SIZE);
  d5 = xmalloc (PE_IDATA5_SIZE);
  id5->contents = d5;
  memset (d5, 0, PE_IDATA5_SIZE);

  if (exp->flag_noname)
    {
      d5[0] = exp->ordinal;
      d5[1] = exp->ordinal >> 8;
      d5[PE_IDATA5_SIZE - 1] = 0x80;
    }
  else
    {
      quick_reloc (abfd, 0, BFD_RELOC_RVA, 4);
      save_relocs (id5);
    }

  bfd_set_section_size (id4, PE_IDATA4_SIZE);
  d4 = xmalloc (PE_IDATA4_SIZE);
  id4->contents = d4;
  memset (d4, 0, PE_IDATA4_SIZE);

  if (exp->flag_noname)
    {
      d4[0] = exp->ordinal;
      d4[1] = exp->ordinal >> 8;
      d4[PE_IDATA4_SIZE - 1] = 0x80;
    }
  else
    {
      quick_reloc (abfd, 0, BFD_RELOC_RVA, 4);
      save_relocs (id4);
    }

  if (exp->flag_noname)
    {
      len = 0;
      bfd_set_section_size (id6, 0);
    }
  else
    {
      int ord;

      /* { short, asciz }  */
      if (exp->its_name)
	len = 2 + strlen (exp->its_name) + 1;
      else
	len = 2 + strlen (exp->name) + 1;
      if (len & 1)
	len++;
      bfd_set_section_size (id6, len);
      d6 = xmalloc (len);
      id6->contents = d6;
      memset (d6, 0, len);

      /* PR 20880:  Use exp->hint as a backup, just in case exp->ordinal
	 contains an invalid value (-1).  */
      ord = (exp->ordinal >= 0) ? exp->ordinal : exp->hint;
      d6[0] = ord;
      d6[1] = ord >> 8;

      if (exp->its_name)
	strcpy ((char*) d6 + 2, exp->its_name);
      else
	strcpy ((char *) d6 + 2, exp->name);
    }

  bfd_set_symtab (abfd, symtab, symptr);

  if (include_jmp_stub)
    bfd_set_section_contents (abfd, tx, td, 0, jmp_byte_count);
  bfd_set_section_contents (abfd, id7, d7, 0, 4);
  bfd_set_section_contents (abfd, id5, d5, 0, PE_IDATA5_SIZE);
  bfd_set_section_contents (abfd, id4, d4, 0, PE_IDATA4_SIZE);
  if (!exp->flag_noname)
    bfd_set_section_contents (abfd, id6, d6, 0, len);

  bfd_make_readable (abfd);
  return abfd;
}

static bfd *
make_singleton_name_thunk (const char *import, bfd *parent)
{
  /* Name thunks go to idata$4.  */
  asection *id4;
  unsigned char *d4;
  char *oname;
  bfd *abfd;

  if (asprintf (&oname, "%s_nmth%06d.o", dll_symname, tmp_seq) < 4)
    /* In theory we should return NULL here at let our caller decide what to
       do.  But currently the return value is not checked, just used, and
       besides, this condition only happens when the system has run out of
       memory.  So just give up.  */
    exit (EXIT_FAILURE);
  tmp_seq++;

  abfd = bfd_create (oname, parent);
  free (oname);
  bfd_find_target (pe_details->object_target, abfd);
  bfd_make_writable (abfd);

  bfd_set_format (abfd, bfd_object);
  bfd_set_arch_mach (abfd, pe_details->bfd_arch, 0);

  symptr = 0;
  symtab = xmalloc (3 * sizeof (asymbol *));
  id4 = quick_section (abfd, ".idata$4", SEC_HAS_CONTENTS, 2);
  quick_symbol (abfd, "__nm_thnk_", import, "", id4, BSF_GLOBAL, 0);
  quick_symbol (abfd, "__nm_", import, "", UNDSEC, BSF_GLOBAL, 0);

  /* We need space for the real thunk and for the null terminator.  */
  bfd_set_section_size (id4, PE_IDATA4_SIZE * 2);
  d4 = xmalloc (PE_IDATA4_SIZE * 2);
  id4->contents = d4;
  memset (d4, 0, PE_IDATA4_SIZE * 2);
  quick_reloc (abfd, 0, BFD_RELOC_RVA, 2);
  save_relocs (id4);

  bfd_set_symtab (abfd, symtab, symptr);

  bfd_set_section_contents (abfd, id4, d4, 0, PE_IDATA4_SIZE * 2);

  bfd_make_readable (abfd);
  return abfd;
}

static const char *
make_import_fixup_mark (arelent *rel, char *name)
{
  /* We convert reloc to symbol, for later reference.  */
  static unsigned int counter;
  struct bfd_symbol *sym = *rel->sym_ptr_ptr;
  bfd *abfd = bfd_asymbol_bfd (sym);
  struct bfd_link_hash_entry *bh;
  char *fixup_name, buf[256];
  size_t prefix_len;

  /* "name" buffer has space before the symbol name for prefixes.  */
  sprintf (buf, "__fu%d_", counter++);
  prefix_len = strlen (buf);
  fixup_name = name - prefix_len;
  memcpy (fixup_name, buf, prefix_len);

  bh = NULL;
  bfd_coff_link_add_one_symbol (&link_info, abfd, fixup_name, BSF_GLOBAL,
				current_sec, /* sym->section, */
				rel->address, NULL, true, false, &bh);

  return bh->root.string;
}

/*	.section	.idata$2
	.rva		__nm_thnk_SYM (singleton thunk with name of func)
	.long		0
	.long		0
	.rva		__my_dll_iname (name of dll)
	.rva		__fuNN_SYM (pointer to reference (address) in text)  */

static bfd *
make_import_fixup_entry (const char *name,
			 const char *fixup_name,
			 const char *symname,
			 bfd *parent)
{
  asection *id2;
  unsigned char *d2;
  char *oname;
  bfd *abfd;

  if (asprintf (&oname, "%s_fu%06d.o", dll_symname, tmp_seq) < 4)
    /* In theory we should return NULL here at let our caller decide what to
       do.  But currently the return value is not checked, just used, and
       besides, this condition only happens when the system has run out of
       memory.  So just give up.  */
    exit (EXIT_FAILURE);
  tmp_seq++;

  abfd = bfd_create (oname, parent);
  free (oname);
  bfd_find_target (pe_details->object_target, abfd);
  bfd_make_writable (abfd);

  bfd_set_format (abfd, bfd_object);
  bfd_set_arch_mach (abfd, pe_details->bfd_arch, 0);

  symptr = 0;
  symtab = xmalloc (6 * sizeof (asymbol *));
  id2 = quick_section (abfd, ".idata$2", SEC_HAS_CONTENTS, 2);

  quick_symbol (abfd, "__nm_thnk_", name, "", UNDSEC, BSF_GLOBAL, 0);
  quick_symbol (abfd, U (""), symname, "_iname", UNDSEC, BSF_GLOBAL, 0);
  quick_symbol (abfd, "", fixup_name, "", UNDSEC, BSF_GLOBAL, 0);

  bfd_set_section_size (id2, 20);
  d2 = xmalloc (20);
  id2->contents = d2;
  memset (d2, 0, 20);

  quick_reloc (abfd, 0, BFD_RELOC_RVA, 1);
  quick_reloc (abfd, 12, BFD_RELOC_RVA, 2);
  quick_reloc (abfd, 16, BFD_RELOC_RVA, 3);
  save_relocs (id2);

  bfd_set_symtab (abfd, symtab, symptr);

  bfd_set_section_contents (abfd, id2, d2, 0, 20);

  bfd_make_readable (abfd);
  return abfd;
}

/*	.section	.rdata_runtime_pseudo_reloc
	.long		addend
	.rva		__fuNN_SYM (pointer to reference (address) in text)  */

static bfd *
make_runtime_pseudo_reloc (const char *name ATTRIBUTE_UNUSED,
			   const char *fixup_name,
			   bfd_vma addend ATTRIBUTE_UNUSED,
			   bfd_vma bitsize,
			   bfd *parent)
{
  asection *rt_rel;
  unsigned char *rt_rel_d;
  char *oname;
  bfd *abfd;
  bfd_size_type size;

  if (asprintf (&oname, "%s_rtr%06d.o", dll_symname, tmp_seq) < 4)
    /* In theory we should return NULL here at let our caller decide what to
       do.  But currently the return value is not checked, just used, and
       besides, this condition only happens when the system has run out of
       memory.  So just give up.  */
    exit (EXIT_FAILURE);
  tmp_seq++;

  abfd = bfd_create (oname, parent);
  free (oname);
  bfd_find_target (pe_details->object_target, abfd);
  bfd_make_writable (abfd);

  bfd_set_format (abfd, bfd_object);
  bfd_set_arch_mach (abfd, pe_details->bfd_arch, 0);

  if (link_info.pei386_runtime_pseudo_reloc == 2)
    {
      if (runtime_pseudp_reloc_v2_init)
	size = 3 * sizeof (asymbol *);
      else
	size = 6 * sizeof (asymbol *);
    }
  else
    size = 2 * sizeof (asymbol *);

  symptr = 0;
  symtab = xmalloc (size);

  rt_rel
    = quick_section (abfd, ".rdata_runtime_pseudo_reloc", SEC_HAS_CONTENTS, 2);
  bfd_coff_set_long_section_names (abfd, true);

  quick_symbol (abfd, "", fixup_name, "", UNDSEC, BSF_GLOBAL, 0);

  if (link_info.pei386_runtime_pseudo_reloc == 2)
    {
      size = 12;
      if (!runtime_pseudp_reloc_v2_init)
	{
	  size += 12;
	  runtime_pseudp_reloc_v2_init = true;
	}

      quick_symbol (abfd, "__imp_", name, "", UNDSEC, BSF_GLOBAL, 0);

      bfd_set_section_size (rt_rel, size);
      rt_rel_d = xmalloc (size);
      rt_rel->contents = rt_rel_d;
      memset (rt_rel_d, 0, size);
      quick_reloc (abfd, size - 8, BFD_RELOC_RVA, 1);
      quick_reloc (abfd, size - 12, BFD_RELOC_RVA, 2);
      bfd_put_32 (abfd, bitsize, rt_rel_d + (size - 4));
      if (size != 12)
	bfd_put_32 (abfd, 1, rt_rel_d + 8);
      save_relocs (rt_rel);

      bfd_set_symtab (abfd, symtab, symptr);

      bfd_set_section_contents (abfd, rt_rel, rt_rel_d, 0, size);
    }
  else
    {
      bfd_set_section_size (rt_rel, 8);
      rt_rel_d = xmalloc (8);
      rt_rel->contents = rt_rel_d;
      memset (rt_rel_d, 0, 8);

      bfd_put_32 (abfd, addend, rt_rel_d);
      quick_reloc (abfd, 4, BFD_RELOC_RVA, 1);

      save_relocs (rt_rel);

      bfd_set_symtab (abfd, symtab, symptr);

      bfd_set_section_contents (abfd, rt_rel, rt_rel_d, 0, 8);
   }

  bfd_make_readable (abfd);
  return abfd;
}

/*	.section	.rdata
	.rva		__pei386_runtime_relocator  */

static bfd *
pe_create_runtime_relocator_reference (bfd *parent)
{
  asection *extern_rt_rel;
  unsigned char *extern_rt_rel_d;
  char *oname;
  bfd *abfd;

  if (asprintf (&oname, "%s_ertr%06d.o", dll_symname, tmp_seq) < 4)
    /* In theory we should return NULL here at let our caller decide what to
       do.  But currently the return value is not checked, just used, and
       besides, this condition only happens when the system has run out of
       memory.  So just give up.  */
    exit (EXIT_FAILURE);
  tmp_seq++;

  abfd = bfd_create (oname, parent);
  free (oname);
  bfd_find_target (pe_details->object_target, abfd);
  bfd_make_writable (abfd);

  bfd_set_format (abfd, bfd_object);
  bfd_set_arch_mach (abfd, pe_details->bfd_arch, 0);

  symptr = 0;
  symtab = xmalloc (2 * sizeof (asymbol *));
  extern_rt_rel = quick_section (abfd, ".rdata", SEC_HAS_CONTENTS, 2);

  quick_symbol (abfd, "", U ("_pei386_runtime_relocator"), "", UNDSEC,
		BSF_NO_FLAGS, 0);

  bfd_set_section_size (extern_rt_rel, PE_IDATA5_SIZE);
  extern_rt_rel_d = xcalloc (1, PE_IDATA5_SIZE);
  extern_rt_rel->contents = extern_rt_rel_d;

  quick_reloc (abfd, 0, BFD_RELOC_RVA, 1);
  save_relocs (extern_rt_rel);

  bfd_set_symtab (abfd, symtab, symptr);

  bfd_set_section_contents (abfd, extern_rt_rel, extern_rt_rel_d, 0, PE_IDATA5_SIZE);

  bfd_make_readable (abfd);
  return abfd;
}

void
pe_create_import_fixup (arelent *rel, asection *s, bfd_vma addend, char *name,
			const char *symname)
{
  const char *fixup_name = make_import_fixup_mark (rel, name);
  bfd *b;

  /* This is the original implementation of the auto-import feature, which
     primarily relied on the OS loader to patch things up with some help
     from the pseudo-relocator to overcome the main limitation.  See the
     comment at the beginning of the file for an overview of the feature.  */
  if (link_info.pei386_runtime_pseudo_reloc != 2)
    {
      struct bfd_link_hash_entry *name_thunk_sym;
      /* name buffer is allocated with space at beginning for prefixes.  */
      char *thname = name - (sizeof "__nm_thnk_" - 1);
      memcpy (thname, "__nm_thnk_", sizeof "__nm_thnk_" - 1);
      name_thunk_sym = bfd_link_hash_lookup (link_info.hash, thname, 0, 0, 1);

      if (!(name_thunk_sym && name_thunk_sym->type == bfd_link_hash_defined))
	{
	  b = make_singleton_name_thunk (name, link_info.output_bfd);
	  add_bfd_to_link (b, bfd_get_filename (b), &link_info);

	  /* If we ever use autoimport, we have to cast text section writable.  */
	  config.text_read_only = false;
	  link_info.output_bfd->flags &= ~WP_TEXT;
	}

      if (addend == 0 || link_info.pei386_runtime_pseudo_reloc == 1)
	{
	  b = make_import_fixup_entry (name, fixup_name, symname,
				       link_info.output_bfd);
	  add_bfd_to_link (b, bfd_get_filename (b), &link_info);
	}
    }

  /* In the original implementation, the pseudo-relocator was only used when
     the addend was not null.  In the new implementation, the OS loader is
     completely bypassed and the pseudo-relocator does the entire work.  */
  if ((addend != 0 && link_info.pei386_runtime_pseudo_reloc == 1)
      || link_info.pei386_runtime_pseudo_reloc == 2)
    {
      if (pe_dll_extra_pe_debug)
	printf ("creating runtime pseudo-reloc entry for %s (addend=%d)\n",
		fixup_name, (int) addend);

      b = make_runtime_pseudo_reloc (name, fixup_name, addend,
				     rel->howto->bitsize,
				     link_info.output_bfd);
      add_bfd_to_link (b, bfd_get_filename (b), &link_info);

      if (runtime_pseudo_relocs_created++ == 0)
	{
	  b = pe_create_runtime_relocator_reference (link_info.output_bfd);
	  add_bfd_to_link (b, bfd_get_filename (b), &link_info);
	}
    }

  else if (addend != 0)
    einfo (_("%X%P: %H: variable '%pT' can't be auto-imported; please read the documentation for ld's --enable-auto-import for details\n"),
	   s->owner, s, rel->address, (*rel->sym_ptr_ptr)->name);
}

void
pe_dll_generate_implib (def_file *def, const char *impfilename, struct bfd_link_info *info)
{
  int i;
  bfd *ar_head;
  bfd *ar_tail;
  bfd *outarch;
  bfd *ibfd;
  bfd *head = 0;

  unlink_if_ordinary (impfilename);

  outarch = bfd_openw (impfilename, 0);

  if (!outarch)
    {
      /* xgettext:c-format */
      einfo (_("%X%P: can't open .lib file: %s\n"), impfilename);
      return;
    }

  if (verbose)
    /* xgettext:c-format */
    info_msg (_("Creating library file: %s\n"), impfilename);

  bfd_set_format (outarch, bfd_archive);
  outarch->has_armap = 1;

  /* Work out a reasonable size of things to put onto one line.  */
  ar_head = make_head (outarch);

  /* Iterate the input BFDs, looking for exclude-modules-for-implib.  */
  for (ibfd = info->input_bfds; ibfd; ibfd = ibfd->link.next)
    {
      /* Iterate the exclude list.  */
      struct exclude_list_struct *ex;
      char found;
      for (ex = excludes, found = 0; ex && !found; ex = ex->next)
	{
	  if (ex->type != EXCLUDEFORIMPLIB)
	    continue;
	  found = (filename_cmp (ex->string, bfd_get_filename (ibfd)) == 0);
	}
      /* If it matched, we must open a fresh BFD for it (the original
	 input BFD is still needed for the DLL's final link) and add
	 it into the archive member chain.  */
      if (found)
	{
	  bfd *newbfd = bfd_openr (ibfd->my_archive
				   ? bfd_get_filename (ibfd->my_archive)
				   : bfd_get_filename (ibfd), NULL);
	  if (!newbfd)
	    {
	      einfo (_("%X%P: bfd_openr %s: %E\n"), bfd_get_filename (ibfd));
	      return;
	    }
	  if (ibfd->my_archive)
	    {
	      /* Must now iterate through archive until we find the
		required member.  A minor shame that we'll open the
		archive once per member that we require from it, and
		leak those archive bfds rather than reuse them.  */
	      bfd *arbfd = newbfd;
	      if (!bfd_check_format_matches (arbfd, bfd_archive, NULL))
		{
		  einfo (_("%X%P: %s(%s): can't find member in non-archive file"),
			 bfd_get_filename (ibfd->my_archive),
			 bfd_get_filename (ibfd));
		  return;
		}
	      newbfd = NULL;
	      while ((newbfd = bfd_openr_next_archived_file (arbfd, newbfd)) != 0)
		{
		  if (filename_cmp (bfd_get_filename (newbfd),
				    bfd_get_filename (ibfd)) == 0)
		    break;
		}
	      if (!newbfd)
		{
		  einfo (_("%X%P: %s(%s): can't find member in archive"),
			 bfd_get_filename (ibfd->my_archive),
			 bfd_get_filename (ibfd));
		  return;
		}
	    }
	  newbfd->archive_next = head;
	  head = newbfd;
	}
    }

  for (i = 0; i < def->num_exports; i++)
    {
      /* The import library doesn't know about the internal name.  */
      char *internal = def->exports[i].internal_name;
      bfd *n;

      /* Don't add PRIVATE entries to import lib.  */
      if (pe_def_file->exports[i].flag_private)
	continue;

      def->exports[i].internal_name = def->exports[i].name;

      /* PR 19803: If a symbol has been discard due to garbage
	 collection then do not create any exports for it.  */
      {
	struct coff_link_hash_entry *h;

	h = coff_link_hash_lookup (coff_hash_table (info), internal,
				   false, false, false);
	if (h != NULL
	    /* If the symbol is hidden and undefined then it
	       has been swept up by garbage collection.  */
	    && h->symbol_class == C_HIDDEN
	    && h->root.u.def.section == bfd_und_section_ptr)
	  continue;

	/* If necessary, check with an underscore prefix as well.  */
	if (pe_details->underscored && internal[0] != '@')
	  {
	    char *name;

	    name = xmalloc (strlen (internal) + 2);
	    sprintf (name, "_%s", internal);

	    h = coff_link_hash_lookup (coff_hash_table (info), name,
				       false, false, false);
	    free (name);

	    if (h != NULL
		/* If the symbol is hidden and undefined then it
		   has been swept up by garbage collection.  */
		&& h->symbol_class == C_HIDDEN
		&& h->root.u.def.section == bfd_und_section_ptr)
	      continue;
	  }
      }

      n = make_one (def->exports + i, outarch, !(def->exports + i)->flag_data);
      n->archive_next = head;
      head = n;
      def->exports[i].internal_name = internal;
    }

  ar_tail = make_tail (outarch);

  if (ar_head == NULL || ar_tail == NULL)
    return;

  /* Now stick them all into the archive.  */
  ar_head->archive_next = head;
  ar_tail->archive_next = ar_head;
  head = ar_tail;

  if (! bfd_set_archive_head (outarch, head))
    einfo ("%X%P: bfd_set_archive_head: %E\n");

  if (! bfd_close (outarch))
    einfo ("%X%P: bfd_close %s: %E\n", impfilename);

  while (head != NULL)
    {
      bfd *n = head->archive_next;
      bfd_close (head);
      head = n;
    }
}

static int undef_count = 0;

struct key_value
{
  char *key;
  const char *oname;
};

static struct key_value *udef_table;

static int undef_sort_cmp (const void *l1, const void *r1)
{
  const struct key_value *l = l1;
  const struct key_value *r = r1;

  return strcmp (l->key, r->key);
}

static struct bfd_link_hash_entry *
pe_find_cdecl_alias_match (struct bfd_link_info *linfo, char *name)
{
  struct bfd_link_hash_entry *h = NULL;
  struct key_value *kv;
  struct key_value key;
  char *at, *lname = xmalloc (strlen (name) + 3);

  strcpy (lname, name);

  at = strchr (lname + (lname[0] == '@'), '@');
  if (at)
    at[1] = 0;

  key.key = lname;
  kv = bsearch (&key, udef_table, undef_count, sizeof (struct key_value),
		undef_sort_cmp);

  if (kv)
    {
      h = bfd_link_hash_lookup (linfo->hash, kv->oname, false, false, false);
      if (h->type == bfd_link_hash_undefined)
	goto return_h;
    }

  if (lname[0] == '?')
    goto return_NULL;

  if (at || lname[0] == '@')
    {
      if (lname[0] == '@')
	{
	  if (pe_details->underscored)
	    lname[0] = '_';
	  else
	    /* Use memmove rather than strcpy as that
	       can handle overlapping buffers.  */
	    memmove (lname, lname + 1, strlen (lname));
	  key.key = lname;
	  kv = bsearch (&key, udef_table, undef_count,
			sizeof (struct key_value), undef_sort_cmp);
	  if (kv)
	    {
	      h = bfd_link_hash_lookup (linfo->hash, kv->oname, false, false, false);
	      if (h->type == bfd_link_hash_undefined)
		goto return_h;
	    }
	}
      if (at)
	*strchr (lname, '@') = 0;
      key.key = lname;
      kv = bsearch (&key, udef_table, undef_count,
		    sizeof (struct key_value), undef_sort_cmp);
      if (kv)
	{
	  h = bfd_link_hash_lookup (linfo->hash, kv->oname, false, false, false);
	  if (h->type == bfd_link_hash_undefined)
	    goto return_h;
	}
      goto return_NULL;
    }

  strcat (lname, "@");
  key.key = lname;
  kv = bsearch (&key, udef_table, undef_count,
		sizeof (struct key_value), undef_sort_cmp);

  if (kv)
    {
      h = bfd_link_hash_lookup (linfo->hash, kv->oname, false, false, false);
      if (h->type == bfd_link_hash_undefined)
	goto return_h;
    }

  if (lname[0] == '_' && pe_details->underscored)
    lname[0] = '@';
  else
    {
      memmove (lname + 1, lname, strlen (lname) + 1);
      lname[0] = '@';
    }
  key.key = lname;

  kv = bsearch (&key, udef_table, undef_count,
		sizeof (struct key_value), undef_sort_cmp);

  if (kv)
    {
      h = bfd_link_hash_lookup (linfo->hash, kv->oname, false, false, false);
      if (h->type == bfd_link_hash_undefined)
	goto return_h;
    }

 return_NULL:
  h = NULL;
 return_h:
  free (lname);
  return h;
}

static bool
pe_undef_count (struct bfd_link_hash_entry *h ATTRIBUTE_UNUSED,
		void *inf ATTRIBUTE_UNUSED)
{
  if (h->type == bfd_link_hash_undefined)
    undef_count++;
  return true;
}

static bool
pe_undef_fill (struct bfd_link_hash_entry *h, void *inf ATTRIBUTE_UNUSED)
{
  if (h->type == bfd_link_hash_undefined)
    {
      char *at;

      udef_table[undef_count].key = xstrdup (h->root.string);
      at = strchr (udef_table[undef_count].key
		   + (udef_table[undef_count].key[0] == '@'), '@');
      if (at)
	at[1] = 0;
      udef_table[undef_count].oname = h->root.string;
      undef_count++;
    }
  return true;
}

static void
pe_create_undef_table (void)
{
  undef_count = 0;

  /* count undefined symbols */

  bfd_link_hash_traverse (link_info.hash, pe_undef_count, "");

  /* create and fill the corresponding table */
  udef_table = xmalloc (undef_count * sizeof (struct key_value));

  undef_count = 0;
  bfd_link_hash_traverse (link_info.hash, pe_undef_fill, "");

  /* sort items */
  qsort (udef_table, undef_count, sizeof (struct key_value), undef_sort_cmp);
}

static void
add_bfd_to_link (bfd *abfd, const char *name, struct bfd_link_info *linfo)
{
  lang_input_statement_type *fake_file;

  fake_file = lang_add_input_file (name,
				   lang_input_file_is_fake_enum,
				   NULL);
  fake_file->the_bfd = abfd;
  ldlang_add_file (fake_file);

  if (!bfd_link_add_symbols (abfd, linfo))
    einfo (_("%X%P: add symbols %s: %E\n"), name);
}

void
pe_process_import_defs (bfd *output_bfd, struct bfd_link_info *linfo)
{
  pe_dll_id_target (bfd_get_target (output_bfd));

  if (pe_def_file)
    {
      int i, j;
      def_file_module *module;
      def_file_import *imp;

      imp = pe_def_file->imports;

      pe_create_undef_table ();

      for (module = pe_def_file->modules; module; module = module->next)
	{
	  int do_this_dll = 0;

	  for (i = 0; i < pe_def_file->num_imports; i++)
	    if (imp[i].module == module)
	      break;
	  if (i >= pe_def_file->num_imports)
	    continue;

	  dll_filename = module->name;
	  dll_symname = xstrdup (module->name);
	  for (j = 0; dll_symname[j]; j++)
	    if (!ISALNUM (dll_symname[j]))
	      dll_symname[j] = '_';

	  for (; i < pe_def_file->num_imports && imp[i].module == module; i++)
	    {
	      def_file_export exp;
	      struct bfd_link_hash_entry *blhe;
	      int lead_at = (*imp[i].internal_name == '@');
	      /* See if we need this import.  */
	      size_t len = strlen (imp[i].internal_name);
	      char *name = xmalloc (len + 2 + 6);
	      bool include_jmp_stub = false;
	      bool is_cdecl = false;
	      bool is_undef = false;

	      if (!lead_at && strchr (imp[i].internal_name, '@') == NULL)
		is_cdecl = true;

	      if (lead_at)
		sprintf (name, "%s", imp[i].internal_name);
	      else
		sprintf (name, "%s%s",U (""), imp[i].internal_name);

	      blhe = bfd_link_hash_lookup (linfo->hash, name,
					   false, false, false);

	      /* Include the jump stub for <sym> only if the <sym>
		 is undefined.  */
	      if (!blhe || (blhe && blhe->type != bfd_link_hash_undefined))
		{
		  if (lead_at)
		    sprintf (name, "%s%s", "__imp_", imp[i].internal_name);
		  else
		    sprintf (name, "%s%s%s", "__imp_", U (""),
			     imp[i].internal_name);

		  blhe = bfd_link_hash_lookup (linfo->hash, name,
					       false, false, false);
		  if (blhe)
		    is_undef = (blhe->type == bfd_link_hash_undefined);
		}
	      else
		{
		  include_jmp_stub = true;
		  is_undef = (blhe->type == bfd_link_hash_undefined);
		}

	      if (is_cdecl
		  && (!blhe || (blhe && blhe->type != bfd_link_hash_undefined)))
		{
		  sprintf (name, "%s%s",U (""), imp[i].internal_name);
		  blhe = pe_find_cdecl_alias_match (linfo, name);
		  include_jmp_stub = true;
		  if (blhe)
		    is_undef = (blhe->type == bfd_link_hash_undefined);
		}

	      free (name);

	      if (is_undef)
		{
		  bfd *one;
		  /* We do.  */
		  if (!do_this_dll)
		    {
		      bfd *ar_head = make_head (output_bfd);
		      add_bfd_to_link (ar_head, bfd_get_filename (ar_head),
				       linfo);
		      do_this_dll = 1;
		    }
		  exp.internal_name = imp[i].internal_name;
		  exp.name = imp[i].name;
		  exp.its_name = imp[i].its_name;
		  exp.ordinal = imp[i].ordinal;
		  exp.hint = exp.ordinal >= 0 ? exp.ordinal : 0;
		  exp.flag_private = 0;
		  exp.flag_constant = 0;
		  exp.flag_data = imp[i].data;
		  exp.flag_noname = exp.name ? 0 : 1;
		  one = make_one (&exp, output_bfd,
				  !exp.flag_data && include_jmp_stub);
		  add_bfd_to_link (one, bfd_get_filename (one), linfo);
		}
	    }
	  if (do_this_dll)
	    {
	      bfd *ar_tail = make_tail (output_bfd);
	      add_bfd_to_link (ar_tail, bfd_get_filename (ar_tail), linfo);
	    }

	  free (dll_symname);
	}

      while (undef_count)
	{
	  --undef_count;
	  free (udef_table[undef_count].key);
	}
      free (udef_table);
    }

  if (pe_def_file && pe_def_file->name)
    dll_filename = pe_def_file->name;
  else
    {
      dll_filename = bfd_get_filename (output_bfd);
      for (const char *p = dll_filename; *p; p++)
	if (*p == '\\' || *p == '/' || *p == ':')
	  dll_filename = p + 1;
    }
  dll_symname = xstrdup (dll_filename);
  for (int i = 0; dll_symname[i]; i++)
    if (!ISALNUM (dll_symname[i]))
      dll_symname[i] = '_';
}

/* We were handed a *.DLL file.  Parse it and turn it into a set of
   IMPORTS directives in the def file.  Return TRUE if the file was
   handled, FALSE if not.  */

static unsigned int
pe_get16 (bfd *abfd, int where)
{
  unsigned char b[2];

  bfd_seek (abfd, (file_ptr) where, SEEK_SET);
  bfd_bread (b, (bfd_size_type) 2, abfd);
  return b[0] + (b[1] << 8);
}

static unsigned int
pe_get32 (bfd *abfd, int where)
{
  unsigned char b[4];

  bfd_seek (abfd, (file_ptr) where, SEEK_SET);
  bfd_bread (b, (bfd_size_type) 4, abfd);
  return b[0] + (b[1] << 8) + (b[2] << 16) + ((unsigned) b[3] << 24);
}

static unsigned int
pe_as32 (void *ptr)
{
  unsigned char *b = ptr;

  return b[0] + (b[1] << 8) + (b[2] << 16) + ((unsigned) b[3] << 24);
}

bool
pe_implied_import_dll (const char *filename)
{
  bfd *dll;
  bfd_vma pe_header_offset, opthdr_ofs, num_entries, i;
  bfd_vma export_rva, export_size, nsections, secptr, expptr;
  bfd_vma exp_funcbase;
  unsigned char *expdata;
  char *erva;
  bfd_vma name_rvas, nexp;
  const char *dllname;
  /* Initialization with start > end guarantees that is_data
     will not be set by mistake, and avoids compiler warning.  */
  bfd_vma data_start = 1;
  bfd_vma data_end = 0;
  bfd_vma rdata_start = 1;
  bfd_vma rdata_end = 0;
  bfd_vma bss_start = 1;
  bfd_vma bss_end = 0;
  int from;

  /* No, I can't use bfd here.  kernel32.dll puts its export table in
     the middle of the .rdata section.  */
  dll = bfd_openr (filename, pe_details->target_name);
  if (!dll)
    {
      einfo (_("%X%P: open %s: %E\n"), filename);
      return false;
    }

  track_dependency_files (filename);

  /* PEI dlls seem to be bfd_objects.  */
  if (!bfd_check_format (dll, bfd_object))
    {
      einfo (_("%X%P: %s: this doesn't appear to be a DLL\n"), filename);
      return false;
    }

  /* Get pe_header, optional header and numbers of directory entries.  */
  pe_header_offset = pe_get32 (dll, 0x3c);
  opthdr_ofs = pe_header_offset + 4 + 20;
#ifdef pe_use_plus
  num_entries = pe_get32 (dll, opthdr_ofs + 92 + 4 * 4); /*  & NumberOfRvaAndSizes.  */
#else
  num_entries = pe_get32 (dll, opthdr_ofs + 92);
#endif

  /* No import or export directory entry.  */
  if (num_entries < 1)
    return false;

#ifdef pe_use_plus
  export_rva  = pe_get32 (dll, opthdr_ofs + 96 + 4 * 4);
  export_size = pe_get32 (dll, opthdr_ofs + 100 + 4 * 4);
#else
  export_rva = pe_get32 (dll, opthdr_ofs + 96);
  export_size = pe_get32 (dll, opthdr_ofs + 100);
#endif

  /* No export table - nothing to export.  */
  if (export_size == 0)
    return false;

  nsections = pe_get16 (dll, pe_header_offset + 4 + 2);
  secptr = (pe_header_offset + 4 + 20 +
	    pe_get16 (dll, pe_header_offset + 4 + 16));
  expptr = 0;

  /* Get the rva and size of the export section.  */
  for (i = 0; i < nsections; i++)
    {
      char sname[8];
      bfd_vma secptr1 = secptr + 40 * i;
      bfd_vma vaddr = pe_get32 (dll, secptr1 + 12);
      bfd_vma vsize = pe_get32 (dll, secptr1 + 16);
      bfd_vma fptr = pe_get32 (dll, secptr1 + 20);

      bfd_seek (dll, (file_ptr) secptr1, SEEK_SET);
      bfd_bread (sname, (bfd_size_type) 8, dll);

      if (vaddr <= export_rva && vaddr + vsize > export_rva)
	{
	  expptr = fptr + (export_rva - vaddr);
	  if (export_rva + export_size > vaddr + vsize)
	    export_size = vsize - (export_rva - vaddr);
	  break;
	}
    }

  /* Scan sections and store the base and size of the
     data and bss segments in data/base_start/end.  */
  for (i = 0; i < nsections; i++)
    {
      bfd_vma secptr1 = secptr + 40 * i;
      bfd_vma vsize = pe_get32 (dll, secptr1 + 8);
      bfd_vma vaddr = pe_get32 (dll, secptr1 + 12);
      bfd_vma flags = pe_get32 (dll, secptr1 + 36);
      char sec_name[9];

      sec_name[8] = '\0';
      bfd_seek (dll, (file_ptr) secptr1 + 0, SEEK_SET);
      bfd_bread (sec_name, (bfd_size_type) 8, dll);

      if (strcmp(sec_name,".data") == 0)
	{
	  data_start = vaddr;
	  data_end = vaddr + vsize;

	  if (pe_dll_extra_pe_debug)
	    printf ("%s %s: 0x%08lx-0x%08lx (0x%08lx)\n",
		    __func__, sec_name, (unsigned long) vaddr,
		    (unsigned long) (vaddr + vsize), (unsigned long) flags);
	}
      else if (strcmp(sec_name,".rdata") == 0)
	{
	  rdata_start = vaddr;
	  rdata_end = vaddr + vsize;

	  if (pe_dll_extra_pe_debug)
	    printf ("%s %s: 0x%08lx-0x%08lx (0x%08lx)\n",
		    __func__, sec_name, (unsigned long) vaddr,
		    (unsigned long) (vaddr + vsize), (unsigned long) flags);
	}
      else if (strcmp (sec_name,".bss") == 0)
	{
	  bss_start = vaddr;
	  bss_end = vaddr + vsize;

	  if (pe_dll_extra_pe_debug)
	    printf ("%s %s: 0x%08lx-0x%08lx (0x%08lx)\n",
		    __func__, sec_name, (unsigned long) vaddr,
		    (unsigned long) (vaddr + vsize), (unsigned long) flags);
	}
    }

  expdata = xmalloc (export_size);
  bfd_seek (dll, (file_ptr) expptr, SEEK_SET);
  bfd_bread (expdata, (bfd_size_type) export_size, dll);
  erva = (char *) expdata - export_rva;

  if (pe_def_file == 0)
    pe_def_file = def_file_empty ();

  nexp = pe_as32 (expdata + 24);
  name_rvas = pe_as32 (expdata + 32);
  exp_funcbase = pe_as32 (expdata + 28);

  /* Use internal dll name instead of filename
     to enable symbolic dll linking.  */
  dllname = erva + pe_as32 (expdata + 12);

  /* Check to see if the dll has already been added to
     the definition list and if so return without error.
     This avoids multiple symbol definitions.  */
  if (def_get_module (pe_def_file, dllname))
    {
      if (pe_dll_extra_pe_debug)
	printf ("%s is already loaded\n", dllname);
      return true;
    }

  /* This is an optimized version of the insertion loop, which avoids lots of
     calls to realloc and memmove from def_file_add_import.  */
  if ((from = def_file_add_import_from (pe_def_file, nexp,
					erva + pe_as32 (erva + name_rvas),
					dllname, 0, NULL, NULL)) >= 0)
    {
      for (i = 0; i < nexp; i++)
	{
	  /* Pointer to the names vector.  */
	  bfd_vma name_rva = pe_as32 (erva + name_rvas + i * 4);
	  def_file_import *imp;
	  /* Pointer to the function address vector.  */
	  bfd_vma func_rva = pe_as32 (erva + exp_funcbase + i * 4);
	  /* is_data is true if the address is in the data, rdata or bss
	     segment.  */
	  const int is_data =
	    (func_rva >= data_start && func_rva < data_end)
	    || (func_rva >= rdata_start && func_rva < rdata_end)
	    || (func_rva >= bss_start && func_rva < bss_end);

	  imp = def_file_add_import_at (pe_def_file, from + i, erva + name_rva,
					dllname, i, NULL, NULL);
	  /* Mark symbol type.  */
	  imp->data = is_data;

	  if (pe_dll_extra_pe_debug)
	    printf ("%s dll-name: %s sym: %s addr: 0x%lx %s\n",
		    __func__, dllname, erva + name_rva,
		    (unsigned long) func_rva, is_data ? "(data)" : "");
	}

      return true;
    }

  /* Iterate through the list of symbols.  */
  for (i = 0; i < nexp; i++)
    {
      /* Pointer to the names vector.  */
      bfd_vma name_rva = pe_as32 (erva + name_rvas + i * 4);
      def_file_import *imp;
      /* Pointer to the function address vector.  */
      bfd_vma func_rva = pe_as32 (erva + exp_funcbase + i * 4);
      int is_data = 0;

      /* Skip unwanted symbols, which are
	 exported in buggy auto-import releases.  */
      if (! startswith (erva + name_rva, "__nm_"))
	{
	  bool is_dup = false;
	  /* is_data is true if the address is in the data, rdata or bss
	     segment.  */
	  is_data =
	    (func_rva >= data_start && func_rva < data_end)
	    || (func_rva >= rdata_start && func_rva < rdata_end)
	    || (func_rva >= bss_start && func_rva < bss_end);

	  imp = def_file_add_import (pe_def_file, erva + name_rva,
				     dllname, i, NULL, NULL, &is_dup);
	  /* Mark symbol type.  */
	  if (!is_dup)
	    imp->data = is_data;

	  if (pe_dll_extra_pe_debug)
	    printf ("%s dll-name: %s sym: %s addr: 0x%lx %s\n",
		    __func__, dllname, erva + name_rva,
		    (unsigned long) func_rva, is_data ? "(data)" : "");
	}
    }

  return true;
}

void
pe_output_file_set_long_section_names (bfd *abfd)
{
  if (pe_use_coff_long_section_names < 0)
    return;
  if (!bfd_coff_set_long_section_names (abfd, pe_use_coff_long_section_names))
    einfo (_("%X%P: error: can't use long section names on this arch\n"));
}

/* These are the main functions, called from the emulation.  The first
   is called after the bfds are read, so we can guess at how much space
   we need.  The second is called after everything is placed, so we
   can put the right values in place.  */

void
pe_dll_build_sections (bfd *abfd, struct bfd_link_info *info)
{
  pe_dll_id_target (bfd_get_target (abfd));
  pe_output_file_set_long_section_names (abfd);
  process_def_file_and_drectve (abfd, info);

  if (pe_def_file->num_exports == 0
      && (!bfd_link_pic (info) || pe_dll_exclude_all_symbols))
    {
      if (pe_dll_enable_reloc_section)
	{
	  build_filler_bfd (false /* edata not needed.  */);
	  pe_output_file_set_long_section_names (filler_bfd);
	}
      return;
    }

  generate_edata ();
  build_filler_bfd (true /* edata is needed.  */);
  pe_output_file_set_long_section_names (filler_bfd);
}

void
pe_exe_build_sections (bfd *abfd, struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  pe_dll_id_target (bfd_get_target (abfd));
  pe_output_file_set_long_section_names (abfd);
  build_filler_bfd (0);
  pe_output_file_set_long_section_names (filler_bfd);
}

void
pe_dll_fill_sections (bfd *abfd, struct bfd_link_info *info)
{
  pe_exe_fill_sections (abfd, info);

  if (edata_s)
    {
      fill_edata (abfd, info);
      edata_s->contents = edata_d;
    }

  if (bfd_link_dll (info))
    pe_data (abfd)->dll = 1;
}

void
pe_exe_fill_sections (bfd *abfd, struct bfd_link_info *info)
{
  pe_dll_id_target (bfd_get_target (abfd));
  pe_output_file_set_long_section_names (abfd);
  image_base = pe_data (abfd)->pe_opthdr.ImageBase;

  generate_reloc (abfd, info);

  if (reloc_sz > 0)
    {
      bfd_set_section_size (reloc_s, reloc_sz);

      /* Resize the sections.  */
      lang_reset_memory_regions ();
      lang_size_sections (NULL, true);

      /* Redo special stuff.  */
      ldemul_after_allocation ();

      /* Do the assignments again.  */
      lang_do_assignments (lang_final_phase_enum);

      reloc_s->contents = reloc_d;
    }
  else if (reloc_s)
    {
      /* Do not emit an empty reloc section.  */
      bfd_set_section_flags (reloc_s, SEC_IN_MEMORY | SEC_EXCLUDE);
      reloc_s->output_section = bfd_abs_section_ptr;
    }
}

bool
pe_bfd_is_dll (bfd *abfd)
{
  return (bfd_get_format (abfd) == bfd_object
	  && obj_pe (abfd)
	  && pe_data (abfd)->dll);
}
