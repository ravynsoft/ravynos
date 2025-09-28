/* Generic symbol-table support for the BFD library.
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
   Written by Cygnus Support.

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
	Symbols

	BFD tries to maintain as much symbol information as it can when
	it moves information from file to file. BFD passes information
	to applications though the <<asymbol>> structure. When the
	application requests the symbol table, BFD reads the table in
	the native form and translates parts of it into the internal
	format. To maintain more than the information passed to
	applications, some targets keep some information ``behind the
	scenes'' in a structure only the particular back end knows
	about. For example, the coff back end keeps the original
	symbol table structure as well as the canonical structure when
	a BFD is read in. On output, the coff back end can reconstruct
	the output symbol table so that no information is lost, even
	information unique to coff which BFD doesn't know or
	understand. If a coff symbol table were read, but were written
	through an a.out back end, all the coff specific information
	would be lost. The symbol table of a BFD
	is not necessarily read in until a canonicalize request is
	made. Then the BFD back end fills in a table provided by the
	application with pointers to the canonical information.  To
	output symbols, the application provides BFD with a table of
	pointers to pointers to <<asymbol>>s. This allows applications
	like the linker to output a symbol as it was read, since the ``behind
	the scenes'' information will be still available.
@menu
@* Reading Symbols::
@* Writing Symbols::
@* Mini Symbols::
@* typedef asymbol::
@* symbol handling functions::
@end menu

INODE
Reading Symbols, Writing Symbols, Symbols, Symbols
SUBSECTION
	Reading symbols

	There are two stages to reading a symbol table from a BFD:
	allocating storage, and the actual reading process. This is an
	excerpt from an application which reads the symbol table:

|         long storage_needed;
|         asymbol **symbol_table;
|         long number_of_symbols;
|         long i;
|
|         storage_needed = bfd_get_symtab_upper_bound (abfd);
|
|         if (storage_needed < 0)
|           FAIL
|
|         if (storage_needed == 0)
|           return;
|
|         symbol_table = xmalloc (storage_needed);
|           ...
|         number_of_symbols =
|            bfd_canonicalize_symtab (abfd, symbol_table);
|
|         if (number_of_symbols < 0)
|           FAIL
|
|         for (i = 0; i < number_of_symbols; i++)
|           process_symbol (symbol_table[i]);

	All storage for the symbols themselves is in an objalloc
	connected to the BFD; it is freed when the BFD is closed.

INODE
Writing Symbols, Mini Symbols, Reading Symbols, Symbols
SUBSECTION
	Writing symbols

	Writing of a symbol table is automatic when a BFD open for
	writing is closed. The application attaches a vector of
	pointers to pointers to symbols to the BFD being written, and
	fills in the symbol count. The close and cleanup code reads
	through the table provided and performs all the necessary
	operations. The BFD output code must always be provided with an
	``owned'' symbol: one which has come from another BFD, or one
	which has been created using <<bfd_make_empty_symbol>>.  Here is an
	example showing the creation of a symbol table with only one element:

|       #include "sysdep.h"
|       #include "bfd.h"
|       int main (void)
|       {
|         bfd *abfd;
|         asymbol *ptrs[2];
|         asymbol *new;
|
|         abfd = bfd_openw ("foo","a.out-sunos-big");
|         bfd_set_format (abfd, bfd_object);
|         new = bfd_make_empty_symbol (abfd);
|         new->name = "dummy_symbol";
|         new->section = bfd_make_section_old_way (abfd, ".text");
|         new->flags = BSF_GLOBAL;
|         new->value = 0x12345;
|
|         ptrs[0] = new;
|         ptrs[1] = 0;
|
|         bfd_set_symtab (abfd, ptrs, 1);
|         bfd_close (abfd);
|         return 0;
|       }
|
|       ./makesym
|       nm foo
|       00012345 A dummy_symbol

	Many formats cannot represent arbitrary symbol information; for
	instance, the <<a.out>> object format does not allow an
	arbitrary number of sections. A symbol pointing to a section
	which is not one  of <<.text>>, <<.data>> or <<.bss>> cannot
	be described.

INODE
Mini Symbols, typedef asymbol, Writing Symbols, Symbols
SUBSECTION
	Mini Symbols

	Mini symbols provide read-only access to the symbol table.
	They use less memory space, but require more time to access.
	They can be useful for tools like nm or objdump, which may
	have to handle symbol tables of extremely large executables.

	The <<bfd_read_minisymbols>> function will read the symbols
	into memory in an internal form.  It will return a <<void *>>
	pointer to a block of memory, a symbol count, and the size of
	each symbol.  The pointer is allocated using <<malloc>>, and
	should be freed by the caller when it is no longer needed.

	The function <<bfd_minisymbol_to_symbol>> will take a pointer
	to a minisymbol, and a pointer to a structure returned by
	<<bfd_make_empty_symbol>>, and return a <<asymbol>> structure.
	The return value may or may not be the same as the value from
	<<bfd_make_empty_symbol>> which was passed in.

*/

/*
DOCDD
INODE
typedef asymbol, symbol handling functions, Mini Symbols, Symbols

SUBSECTION
	typedef asymbol

	An <<asymbol>> has the form:

CODE_FRAGMENT
.typedef struct bfd_symbol
.{
.  {* A pointer to the BFD which owns the symbol. This information
.     is necessary so that a back end can work out what additional
.     information (invisible to the application writer) is carried
.     with the symbol.
.
.     This field is *almost* redundant, since you can use section->owner
.     instead, except that some symbols point to the global sections
.     bfd_{abs,com,und}_section.  This could be fixed by making
.     these globals be per-bfd (or per-target-flavor).  FIXME.  *}
.  struct bfd *the_bfd; {* Use bfd_asymbol_bfd(sym) to access this field.  *}
.
.  {* The text of the symbol. The name is left alone, and not copied; the
.     application may not alter it.  *}
.  const char *name;
.
.  {* The value of the symbol.  This really should be a union of a
.     numeric value with a pointer, since some flags indicate that
.     a pointer to another symbol is stored here.  *}
.  symvalue value;
.
.  {* Attributes of a symbol.  *}
.#define BSF_NO_FLAGS            0
.
.  {* The symbol has local scope; <<static>> in <<C>>. The value
.     is the offset into the section of the data.  *}
.#define BSF_LOCAL               (1 << 0)
.
.  {* The symbol has global scope; initialized data in <<C>>. The
.     value is the offset into the section of the data.  *}
.#define BSF_GLOBAL              (1 << 1)
.
.  {* The symbol has global scope and is exported. The value is
.     the offset into the section of the data.  *}
.#define BSF_EXPORT              BSF_GLOBAL {* No real difference.  *}
.
.  {* A normal C symbol would be one of:
.     <<BSF_LOCAL>>, <<BSF_UNDEFINED>> or <<BSF_GLOBAL>>.  *}
.
.  {* The symbol is a debugging record. The value has an arbitrary
.     meaning, unless BSF_DEBUGGING_RELOC is also set.  *}
.#define BSF_DEBUGGING           (1 << 2)
.
.  {* The symbol denotes a function entry point.  Used in ELF,
.     perhaps others someday.  *}
.#define BSF_FUNCTION            (1 << 3)
.
.  {* Used by the linker.  *}
.#define BSF_KEEP                (1 << 5)
.
.  {* An ELF common symbol.  *}
.#define BSF_ELF_COMMON          (1 << 6)
.
.  {* A weak global symbol, overridable without warnings by
.     a regular global symbol of the same name.  *}
.#define BSF_WEAK                (1 << 7)
.
.  {* This symbol was created to point to a section, e.g. ELF's
.     STT_SECTION symbols.  *}
.#define BSF_SECTION_SYM         (1 << 8)
.
.  {* The symbol used to be a common symbol, but now it is
.     allocated.  *}
.#define BSF_OLD_COMMON          (1 << 9)
.
.  {* In some files the type of a symbol sometimes alters its
.     location in an output file - ie in coff a <<ISFCN>> symbol
.     which is also <<C_EXT>> symbol appears where it was
.     declared and not at the end of a section.  This bit is set
.     by the target BFD part to convey this information.  *}
.#define BSF_NOT_AT_END          (1 << 10)
.
.  {* Signal that the symbol is the label of constructor section.  *}
.#define BSF_CONSTRUCTOR         (1 << 11)
.
.  {* Signal that the symbol is a warning symbol.  The name is a
.     warning.  The name of the next symbol is the one to warn about;
.     if a reference is made to a symbol with the same name as the next
.     symbol, a warning is issued by the linker.  *}
.#define BSF_WARNING             (1 << 12)
.
.  {* Signal that the symbol is indirect.  This symbol is an indirect
.     pointer to the symbol with the same name as the next symbol.  *}
.#define BSF_INDIRECT            (1 << 13)
.
.  {* BSF_FILE marks symbols that contain a file name.  This is used
.     for ELF STT_FILE symbols.  *}
.#define BSF_FILE                (1 << 14)
.
.  {* Symbol is from dynamic linking information.  *}
.#define BSF_DYNAMIC             (1 << 15)
.
.  {* The symbol denotes a data object.  Used in ELF, and perhaps
.     others someday.  *}
.#define BSF_OBJECT              (1 << 16)
.
.  {* This symbol is a debugging symbol.  The value is the offset
.     into the section of the data.  BSF_DEBUGGING should be set
.     as well.  *}
.#define BSF_DEBUGGING_RELOC     (1 << 17)
.
.  {* This symbol is thread local.  Used in ELF.  *}
.#define BSF_THREAD_LOCAL        (1 << 18)
.
.  {* This symbol represents a complex relocation expression,
.     with the expression tree serialized in the symbol name.  *}
.#define BSF_RELC                (1 << 19)
.
.  {* This symbol represents a signed complex relocation expression,
.     with the expression tree serialized in the symbol name.  *}
.#define BSF_SRELC               (1 << 20)
.
.  {* This symbol was created by bfd_get_synthetic_symtab.  *}
.#define BSF_SYNTHETIC           (1 << 21)
.
.  {* This symbol is an indirect code object.  Unrelated to BSF_INDIRECT.
.     The dynamic linker will compute the value of this symbol by
.     calling the function that it points to.  BSF_FUNCTION must
.     also be also set.  *}
.#define BSF_GNU_INDIRECT_FUNCTION (1 << 22)
.  {* This symbol is a globally unique data object.  The dynamic linker
.     will make sure that in the entire process there is just one symbol
.     with this name and type in use.  BSF_OBJECT must also be set.  *}
.#define BSF_GNU_UNIQUE          (1 << 23)
.
.  {* This section symbol should be included in the symbol table.  *}
.#define BSF_SECTION_SYM_USED    (1 << 24)
.
.  flagword flags;
.
.  {* A pointer to the section to which this symbol is
.     relative.  This will always be non NULL, there are special
.     sections for undefined and absolute symbols.  *}
.  struct bfd_section *section;
.
.  {* Back end special data.  *}
.  union
.    {
.      void *p;
.      bfd_vma i;
.    }
.  udata;
.}
.asymbol;
.

EXTERNAL
.typedef enum bfd_print_symbol
.{
.  bfd_print_symbol_name,
.  bfd_print_symbol_more,
.  bfd_print_symbol_all
.} bfd_print_symbol_type;
.
.{* Information about a symbol that nm needs.  *}
.
.typedef struct _symbol_info
.{
.  symvalue value;
.  char type;
.  const char *name;		{* Symbol name.  *}
.  unsigned char stab_type;	{* Stab type.  *}
.  char stab_other;		{* Stab other.  *}
.  short stab_desc;		{* Stab desc.  *}
.  const char *stab_name;	{* String for stab type.  *}
.} symbol_info;
.
*/

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "safe-ctype.h"
#include "bfdlink.h"
#include "aout/stab_gnu.h"

/*
DOCDD
INODE
symbol handling functions,  , typedef asymbol, Symbols
SUBSECTION
	Symbol handling functions
*/

/*
FUNCTION
	bfd_get_symtab_upper_bound

DESCRIPTION
	Return the number of bytes required to store a vector of pointers
	to <<asymbols>> for all the symbols in the BFD @var{abfd},
	including a terminal NULL pointer. If there are no symbols in
	the BFD, then return 0.  If an error occurs, return -1.

.#define bfd_get_symtab_upper_bound(abfd) \
.	BFD_SEND (abfd, _bfd_get_symtab_upper_bound, (abfd))
.
*/

/*
FUNCTION
	bfd_is_local_label

SYNOPSIS
	bool bfd_is_local_label (bfd *abfd, asymbol *sym);

DESCRIPTION
	Return TRUE if the given symbol @var{sym} in the BFD @var{abfd} is
	a compiler generated local label, else return FALSE.
*/

bool
bfd_is_local_label (bfd *abfd, asymbol *sym)
{
  /* The BSF_SECTION_SYM check is needed for IA-64, where every label that
     starts with '.' is local.  This would accidentally catch section names
     if we didn't reject them here.  */
  if ((sym->flags & (BSF_GLOBAL | BSF_WEAK | BSF_FILE | BSF_SECTION_SYM)) != 0)
    return false;
  if (sym->name == NULL)
    return false;
  return bfd_is_local_label_name (abfd, sym->name);
}

/*
FUNCTION
	bfd_is_local_label_name

SYNOPSIS
	bool bfd_is_local_label_name (bfd *abfd, const char *name);

DESCRIPTION
	Return TRUE if a symbol with the name @var{name} in the BFD
	@var{abfd} is a compiler generated local label, else return
	FALSE.  This just checks whether the name has the form of a
	local label.

.#define bfd_is_local_label_name(abfd, name) \
.	BFD_SEND (abfd, _bfd_is_local_label_name, (abfd, name))
.
*/

/*
FUNCTION
	bfd_is_target_special_symbol

SYNOPSIS
	bool bfd_is_target_special_symbol (bfd *abfd, asymbol *sym);

DESCRIPTION
	Return TRUE iff a symbol @var{sym} in the BFD @var{abfd} is something
	special to the particular target represented by the BFD.  Such symbols
	should normally not be mentioned to the user.

.#define bfd_is_target_special_symbol(abfd, sym) \
.	BFD_SEND (abfd, _bfd_is_target_special_symbol, (abfd, sym))
.
*/

/*
FUNCTION
	bfd_canonicalize_symtab

DESCRIPTION
	Read the symbols from the BFD @var{abfd}, and fills in
	the vector @var{location} with pointers to the symbols and
	a trailing NULL.
	Return the actual number of symbol pointers, not
	including the NULL.

.#define bfd_canonicalize_symtab(abfd, location) \
.	BFD_SEND (abfd, _bfd_canonicalize_symtab, (abfd, location))
.
*/

/*
FUNCTION
	bfd_set_symtab

SYNOPSIS
	bool bfd_set_symtab
	  (bfd *abfd, asymbol **location, unsigned int count);

DESCRIPTION
	Arrange that when the output BFD @var{abfd} is closed,
	the table @var{location} of @var{count} pointers to symbols
	will be written.
*/

bool
bfd_set_symtab (bfd *abfd, asymbol **location, unsigned int symcount)
{
  if (abfd->format != bfd_object || bfd_read_p (abfd))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  abfd->outsymbols = location;
  abfd->symcount = symcount;
  return true;
}

/*
FUNCTION
	bfd_print_symbol_vandf

SYNOPSIS
	void bfd_print_symbol_vandf (bfd *abfd, void *file, asymbol *symbol);

DESCRIPTION
	Print the value and flags of the @var{symbol} supplied to the
	stream @var{file}.
*/
void
bfd_print_symbol_vandf (bfd *abfd, void *arg, asymbol *symbol)
{
  FILE *file = (FILE *) arg;

  flagword type = symbol->flags;

  if (symbol->section != NULL)
    bfd_fprintf_vma (abfd, file, symbol->value + symbol->section->vma);
  else
    bfd_fprintf_vma (abfd, file, symbol->value);

  /* This presumes that a symbol can not be both BSF_DEBUGGING and
     BSF_DYNAMIC, nor more than one of BSF_FUNCTION, BSF_FILE, and
     BSF_OBJECT.  */
  fprintf (file, " %c%c%c%c%c%c%c",
	   ((type & BSF_LOCAL)
	    ? (type & BSF_GLOBAL) ? '!' : 'l'
	    : (type & BSF_GLOBAL) ? 'g'
	    : (type & BSF_GNU_UNIQUE) ? 'u' : ' '),
	   (type & BSF_WEAK) ? 'w' : ' ',
	   (type & BSF_CONSTRUCTOR) ? 'C' : ' ',
	   (type & BSF_WARNING) ? 'W' : ' ',
	   (type & BSF_INDIRECT) ? 'I' : (type & BSF_GNU_INDIRECT_FUNCTION) ? 'i' : ' ',
	   (type & BSF_DEBUGGING) ? 'd' : (type & BSF_DYNAMIC) ? 'D' : ' ',
	   ((type & BSF_FUNCTION)
	    ? 'F'
	    : ((type & BSF_FILE)
	       ? 'f'
	       : ((type & BSF_OBJECT) ? 'O' : ' '))));
}

/*
FUNCTION
	bfd_make_empty_symbol

DESCRIPTION
	Create a new <<asymbol>> structure for the BFD @var{abfd}
	and return a pointer to it.

	This routine is necessary because each back end has private
	information surrounding the <<asymbol>>. Building your own
	<<asymbol>> and pointing to it will not create the private
	information, and will cause problems later on.

.#define bfd_make_empty_symbol(abfd) \
.	BFD_SEND (abfd, _bfd_make_empty_symbol, (abfd))
.
*/

/*
FUNCTION
	_bfd_generic_make_empty_symbol

SYNOPSIS
	asymbol *_bfd_generic_make_empty_symbol (bfd *);

DESCRIPTION
	Create a new <<asymbol>> structure for the BFD @var{abfd}
	and return a pointer to it.  Used by core file routines,
	binary back-end and anywhere else where no private info
	is needed.
*/

asymbol *
_bfd_generic_make_empty_symbol (bfd *abfd)
{
  size_t amt = sizeof (asymbol);
  asymbol *new_symbol = (asymbol *) bfd_zalloc (abfd, amt);
  if (new_symbol)
    new_symbol->the_bfd = abfd;
  return new_symbol;
}

/*
FUNCTION
	bfd_make_debug_symbol

DESCRIPTION
	Create a new <<asymbol>> structure for the BFD @var{abfd},
	to be used as a debugging symbol.

.#define bfd_make_debug_symbol(abfd) \
.	BFD_SEND (abfd, _bfd_make_debug_symbol, (abfd))
.
*/

struct section_to_type
{
  const char *section;
  char type;
};

/* Map special section names to POSIX/BSD single-character symbol types.
   This table is probably incomplete.  It is sorted for convenience of
   adding entries.  Since it is so short, a linear search is used.  */
static const struct section_to_type stt[] =
{
  {".drectve", 'i'},		/* MSVC's .drective section */
  {".edata", 'e'},		/* MSVC's .edata (export) section */
  {".idata", 'i'},		/* MSVC's .idata (import) section */
  {".pdata", 'p'},		/* MSVC's .pdata (stack unwind) section */
  {0, 0}
};

/* Return the single-character symbol type corresponding to
   section S, or '?' for an unknown COFF section.

   Check for leading strings which match, followed by a number, '.',
   or '$' so .idata5 matches the .idata entry.  */

static char
coff_section_type (const char *s)
{
  const struct section_to_type *t;

  for (t = &stt[0]; t->section; t++)
    {
      size_t len = strlen (t->section);
      if (strncmp (s, t->section, len) == 0
	  && memchr (".$0123456789", s[len], 13) != 0)
	return t->type;
    }

  return '?';
}

/* Return the single-character symbol type corresponding to section
   SECTION, or '?' for an unknown section.  This uses section flags to
   identify sections.

   FIXME These types are unhandled: e, i, p.  If we handled these also,
   we could perhaps obsolete coff_section_type.  */

static char
decode_section_type (const struct bfd_section *section)
{
  if (section->flags & SEC_CODE)
    return 't';
  if (section->flags & SEC_DATA)
    {
      if (section->flags & SEC_READONLY)
	return 'r';
      else if (section->flags & SEC_SMALL_DATA)
	return 'g';
      else
	return 'd';
    }
  if ((section->flags & SEC_HAS_CONTENTS) == 0)
    {
      if (section->flags & SEC_SMALL_DATA)
	return 's';
      else
	return 'b';
    }
  if (section->flags & SEC_DEBUGGING)
    return 'N';
  if ((section->flags & SEC_HAS_CONTENTS) && (section->flags & SEC_READONLY))
    return 'n';

  return '?';
}

/*
FUNCTION
	bfd_decode_symclass

SYNOPSIS
	int bfd_decode_symclass (asymbol *symbol);

DESCRIPTION
	Return a character corresponding to the symbol
	class of @var{symbol}, or '?' for an unknown class.
*/
int
bfd_decode_symclass (asymbol *symbol)
{
  char c;

  /* Paranoia...  */
  if (symbol == NULL || symbol->section == NULL)
    return '?';

  if (symbol->section && bfd_is_com_section (symbol->section))
    {
      if (symbol->section->flags & SEC_SMALL_DATA)
	return 'c';
      else
	return 'C';
    }
  if (bfd_is_und_section (symbol->section))
    {
      if (symbol->flags & BSF_WEAK)
	{
	  /* If weak, determine if it's specifically an object
	     or non-object weak.  */
	  if (symbol->flags & BSF_OBJECT)
	    return 'v';
	  else
	    return 'w';
	}
      else
	return 'U';
    }
  if (bfd_is_ind_section (symbol->section))
    return 'I';
  if (symbol->flags & BSF_GNU_INDIRECT_FUNCTION)
    return 'i';
  if (symbol->flags & BSF_WEAK)
    {
      /* If weak, determine if it's specifically an object
	 or non-object weak.  */
      if (symbol->flags & BSF_OBJECT)
	return 'V';
      else
	return 'W';
    }
  if (symbol->flags & BSF_GNU_UNIQUE)
    return 'u';
  if (!(symbol->flags & (BSF_GLOBAL | BSF_LOCAL)))
    return '?';

  if (bfd_is_abs_section (symbol->section))
    c = 'a';
  else if (symbol->section)
    {
      c = coff_section_type (symbol->section->name);
      if (c == '?')
	c = decode_section_type (symbol->section);
    }
  else
    return '?';
  if (symbol->flags & BSF_GLOBAL)
    c = TOUPPER (c);
  return c;

  /* We don't have to handle these cases just yet, but we will soon:
     N_SETV: 'v';
     N_SETA: 'l';
     N_SETT: 'x';
     N_SETD: 'z';
     N_SETB: 's';
     N_INDR: 'i';
     */
}

/*
FUNCTION
	bfd_is_undefined_symclass

SYNOPSIS
	bool bfd_is_undefined_symclass (int symclass);

DESCRIPTION
	Returns non-zero if the class symbol returned by
	bfd_decode_symclass represents an undefined symbol.
	Returns zero otherwise.
*/

bool
bfd_is_undefined_symclass (int symclass)
{
  return symclass == 'U' || symclass == 'w' || symclass == 'v';
}

/*
FUNCTION
	bfd_symbol_info

SYNOPSIS
	void bfd_symbol_info (asymbol *symbol, symbol_info *ret);

DESCRIPTION
	Fill in the basic info about symbol that nm needs.
	Additional info may be added by the back-ends after
	calling this function.
*/

void
bfd_symbol_info (asymbol *symbol, symbol_info *ret)
{
  ret->type = bfd_decode_symclass (symbol);

  if (bfd_is_undefined_symclass (ret->type))
    ret->value = 0;
  else
    ret->value = symbol->value + symbol->section->vma;

  ret->name = symbol->name;
}

/*
FUNCTION
	bfd_copy_private_symbol_data

SYNOPSIS
	bool bfd_copy_private_symbol_data
	  (bfd *ibfd, asymbol *isym, bfd *obfd, asymbol *osym);

DESCRIPTION
	Copy private symbol information from @var{isym} in the BFD
	@var{ibfd} to the symbol @var{osym} in the BFD @var{obfd}.
	Return <<TRUE>> on success, <<FALSE>> on error.  Possible error
	returns are:

	o <<bfd_error_no_memory>> -
	Not enough memory exists to create private data for @var{osec}.

.#define bfd_copy_private_symbol_data(ibfd, isymbol, obfd, osymbol) \
.	BFD_SEND (obfd, _bfd_copy_private_symbol_data, \
.		  (ibfd, isymbol, obfd, osymbol))
.
*/

/* The generic version of the function which returns mini symbols.
   This is used when the backend does not provide a more efficient
   version.  It just uses BFD asymbol structures as mini symbols.  */

long
_bfd_generic_read_minisymbols (bfd *abfd,
			       bool dynamic,
			       void **minisymsp,
			       unsigned int *sizep)
{
  long storage;
  asymbol **syms = NULL;
  long symcount;

  if (dynamic)
    storage = bfd_get_dynamic_symtab_upper_bound (abfd);
  else
    storage = bfd_get_symtab_upper_bound (abfd);
  if (storage < 0)
    goto error_return;
  if (storage == 0)
    return 0;

  syms = (asymbol **) bfd_malloc (storage);
  if (syms == NULL)
    goto error_return;

  if (dynamic)
    symcount = bfd_canonicalize_dynamic_symtab (abfd, syms);
  else
    symcount = bfd_canonicalize_symtab (abfd, syms);
  if (symcount < 0)
    goto error_return;

  if (symcount == 0)
    /* We return 0 above when storage is 0.  Exit in the same state
       here, so as to not complicate callers with having to deal with
       freeing memory for zero symcount.  */
    free (syms);
  else
    {
      *minisymsp = syms;
      *sizep = sizeof (asymbol *);
    }
  return symcount;

 error_return:
  bfd_set_error (bfd_error_no_symbols);
  free (syms);
  return -1;
}

/* The generic version of the function which converts a minisymbol to
   an asymbol.  We don't worry about the sym argument we are passed;
   we just return the asymbol the minisymbol points to.  */

asymbol *
_bfd_generic_minisymbol_to_symbol (bfd *abfd ATTRIBUTE_UNUSED,
				   bool dynamic ATTRIBUTE_UNUSED,
				   const void *minisym,
				   asymbol *sym ATTRIBUTE_UNUSED)
{
  return *(asymbol **) minisym;
}

/* Look through stabs debugging information in .stab and .stabstr
   sections to find the source file and line closest to a desired
   location.  This is used by COFF and ELF targets.  It sets *pfound
   to TRUE if it finds some information.  The *pinfo field is used to
   pass cached information in and out of this routine; this first time
   the routine is called for a BFD, *pinfo should be NULL.  The value
   placed in *pinfo should be saved with the BFD, and passed back each
   time this function is called.  */

/* We use a cache by default.  */

#define ENABLE_CACHING

/* We keep an array of indexentry structures to record where in the
   stabs section we should look to find line number information for a
   particular address.  */

struct indexentry
{
  bfd_vma val;
  bfd_byte *stab;
  bfd_byte *str;
  char *directory_name;
  char *file_name;
  char *function_name;
  int idx;
};

/* Compare two indexentry structures.  This is called via qsort.  */

static int
cmpindexentry (const void *a, const void *b)
{
  const struct indexentry *contestantA = (const struct indexentry *) a;
  const struct indexentry *contestantB = (const struct indexentry *) b;

  if (contestantA->val < contestantB->val)
    return -1;
  if (contestantA->val > contestantB->val)
    return 1;
  return contestantA->idx - contestantB->idx;
}

/* A pointer to this structure is stored in *pinfo.  */

struct stab_find_info
{
  /* The .stab section.  */
  asection *stabsec;
  /* The .stabstr section.  */
  asection *strsec;
  /* The contents of the .stab section.  */
  bfd_byte *stabs;
  /* The contents of the .stabstr section.  */
  bfd_byte *strs;

  /* A table that indexes stabs by memory address.  */
  struct indexentry *indextable;
  /* The number of entries in indextable.  */
  int indextablesize;

#ifdef ENABLE_CACHING
  /* Cached values to restart quickly.  */
  struct indexentry *cached_indexentry;
  bfd_vma cached_offset;
  bfd_byte *cached_stab;
  char *cached_file_name;
#endif

  /* Saved ptr to malloc'ed filename.  */
  char *filename;
};

bool
_bfd_stab_section_find_nearest_line (bfd *abfd,
				     asymbol **symbols,
				     asection *section,
				     bfd_vma offset,
				     bool *pfound,
				     const char **pfilename,
				     const char **pfnname,
				     unsigned int *pline,
				     void **pinfo)
{
  struct stab_find_info *info;
  bfd_size_type stabsize, strsize;
  bfd_byte *stab, *str;
  bfd_byte *nul_fun, *nul_str;
  bfd_size_type stroff;
  struct indexentry *indexentry;
  char *file_name;
  char *directory_name;
  bool saw_line, saw_func;

  *pfound = false;
  *pfilename = bfd_get_filename (abfd);
  *pfnname = NULL;
  *pline = 0;

  /* Stabs entries use a 12 byte format:
       4 byte string table index
       1 byte stab type
       1 byte stab other field
       2 byte stab desc field
       4 byte stab value
     FIXME: This will have to change for a 64 bit object format.

     The stabs symbols are divided into compilation units.  For the
     first entry in each unit, the type of 0, the value is the length
     of the string table for this unit, and the desc field is the
     number of stabs symbols for this unit.  */

#define STRDXOFF (0)
#define TYPEOFF (4)
#define OTHEROFF (5)
#define DESCOFF (6)
#define VALOFF (8)
#define STABSIZE (12)

  info = (struct stab_find_info *) *pinfo;
  if (info != NULL)
    {
      if (info->stabsec == NULL || info->strsec == NULL)
	{
	  /* No usable stabs debugging information.  */
	  return true;
	}

      stabsize = (info->stabsec->rawsize
		  ? info->stabsec->rawsize
		  : info->stabsec->size);
      strsize = (info->strsec->rawsize
		 ? info->strsec->rawsize
		 : info->strsec->size);
    }
  else
    {
      long reloc_size, reloc_count;
      arelent **reloc_vector;
      int i;
      char *function_name;
      bfd_size_type amt = sizeof *info;

      info = (struct stab_find_info *) bfd_zalloc (abfd, amt);
      if (info == NULL)
	return false;
      *pinfo = info;

      /* FIXME: When using the linker --split-by-file or
	 --split-by-reloc options, it is possible for the .stab and
	 .stabstr sections to be split.  We should handle that.  */

      info->stabsec = bfd_get_section_by_name (abfd, ".stab");
      info->strsec = bfd_get_section_by_name (abfd, ".stabstr");

      if (info->stabsec == NULL || info->strsec == NULL)
	{
	  /* Try SOM section names.  */
	  info->stabsec = bfd_get_section_by_name (abfd, "$GDB_SYMBOLS$");
	  info->strsec  = bfd_get_section_by_name (abfd, "$GDB_STRINGS$");

	  if (info->stabsec == NULL || info->strsec == NULL)
	    return true;
	}

      if ((info->stabsec->flags & SEC_HAS_CONTENTS) == 0
	  || (info->strsec->flags & SEC_HAS_CONTENTS) == 0)
	goto out;

      stabsize = (info->stabsec->rawsize
		  ? info->stabsec->rawsize
		  : info->stabsec->size);
      stabsize = (stabsize / STABSIZE) * STABSIZE;
      strsize = (info->strsec->rawsize
		 ? info->strsec->rawsize
		 : info->strsec->size);

      if (stabsize == 0 || strsize == 0)
	goto out;

      if (!bfd_malloc_and_get_section (abfd, info->stabsec, &info->stabs))
	goto out;
      if (!bfd_malloc_and_get_section (abfd, info->strsec, &info->strs))
	goto out1;

      /* Stab strings ought to be nul terminated.  Ensure the last one
	 is, to prevent running off the end of the buffer.  */
      info->strs[strsize - 1] = 0;

      /* If this is a relocatable object file, we have to relocate
	 the entries in .stab.  This should always be simple 32 bit
	 relocations against symbols defined in this object file, so
	 this should be no big deal.  */
      reloc_size = bfd_get_reloc_upper_bound (abfd, info->stabsec);
      if (reloc_size < 0)
	goto out2;
      reloc_vector = (arelent **) bfd_malloc (reloc_size);
      if (reloc_vector == NULL && reloc_size != 0)
	goto out2;
      reloc_count = bfd_canonicalize_reloc (abfd, info->stabsec, reloc_vector,
					    symbols);
      if (reloc_count < 0)
	{
	out3:
	  free (reloc_vector);
	out2:
	  free (info->strs);
	  info->strs = NULL;
	out1:
	  free (info->stabs);
	  info->stabs = NULL;
	out:
	  info->stabsec = NULL;
	  return false;
	}
      if (reloc_count > 0)
	{
	  arelent **pr;

	  for (pr = reloc_vector; *pr != NULL; pr++)
	    {
	      arelent *r;
	      unsigned long val;
	      asymbol *sym;
	      bfd_size_type octets;

	      r = *pr;
	      /* Ignore R_*_NONE relocs.  */
	      if (r->howto->dst_mask == 0)
		continue;

	      octets = r->address * bfd_octets_per_byte (abfd, NULL);
	      if (r->howto->rightshift != 0
		  || bfd_get_reloc_size (r->howto) != 4
		  || r->howto->bitsize != 32
		  || r->howto->pc_relative
		  || r->howto->bitpos != 0
		  || r->howto->dst_mask != 0xffffffff
		  || octets > stabsize - 4)
		{
		  _bfd_error_handler
		    (_("unsupported .stab relocation"));
		  bfd_set_error (bfd_error_invalid_operation);
		  goto out3;
		}

	      val = bfd_get_32 (abfd, info->stabs + octets);
	      val &= r->howto->src_mask;
	      sym = *r->sym_ptr_ptr;
	      val += sym->value + sym->section->vma + r->addend;
	      bfd_put_32 (abfd, (bfd_vma) val, info->stabs + octets);
	    }
	}

      free (reloc_vector);

      /* First time through this function, build a table matching
	 function VM addresses to stabs, then sort based on starting
	 VM address.  Do this in two passes: once to count how many
	 table entries we'll need, and a second to actually build the
	 table.  */

      info->indextablesize = 0;
      nul_fun = NULL;
      for (stab = info->stabs; stab < info->stabs + stabsize; stab += STABSIZE)
	{
	  if (stab[TYPEOFF] == (bfd_byte) N_SO)
	    {
	      /* if we did not see a function def, leave space for one.  */
	      if (nul_fun != NULL)
		++info->indextablesize;

	      /* N_SO with null name indicates EOF */
	      if (bfd_get_32 (abfd, stab + STRDXOFF) == 0)
		nul_fun = NULL;
	      else
		{
		  nul_fun = stab;

		  /* two N_SO's in a row is a filename and directory. Skip */
		  if (stab + STABSIZE + TYPEOFF < info->stabs + stabsize
		      && *(stab + STABSIZE + TYPEOFF) == (bfd_byte) N_SO)
		    stab += STABSIZE;
		}
	    }
	  else if (stab[TYPEOFF] == (bfd_byte) N_FUN
		   && bfd_get_32 (abfd, stab + STRDXOFF) != 0)
	    {
	      nul_fun = NULL;
	      ++info->indextablesize;
	    }
	}

      if (nul_fun != NULL)
	++info->indextablesize;

      if (info->indextablesize == 0)
	{
	  free (info->strs);
	  info->strs = NULL;
	  free (info->stabs);
	  info->stabs = NULL;
	  info->stabsec = NULL;
	  return true;
	}
      ++info->indextablesize;

      amt = info->indextablesize;
      amt *= sizeof (struct indexentry);
      info->indextable = (struct indexentry *) bfd_malloc (amt);
      if (info->indextable == NULL)
	goto out3;

      file_name = NULL;
      directory_name = NULL;
      nul_fun = NULL;
      stroff = 0;

      for (i = 0, stab = info->stabs, nul_str = str = info->strs;
	   i < info->indextablesize && stab < info->stabs + stabsize;
	   stab += STABSIZE)
	{
	  switch (stab[TYPEOFF])
	    {
	    case 0:
	      /* This is the first entry in a compilation unit.  */
	      if ((bfd_size_type) ((info->strs + strsize) - str) < stroff)
		break;
	      str += stroff;
	      stroff = bfd_get_32 (abfd, stab + VALOFF);
	      break;

	    case N_SO:
	      /* The main file name.  */

	      /* The following code creates a new indextable entry with
		 a NULL function name if there were no N_FUNs in a file.
		 Note that a N_SO without a file name is an EOF and
		 there could be 2 N_SO following it with the new filename
		 and directory.  */
	      if (nul_fun != NULL)
		{
		  info->indextable[i].val = bfd_get_32 (abfd, nul_fun + VALOFF);
		  info->indextable[i].stab = nul_fun;
		  info->indextable[i].str = nul_str;
		  info->indextable[i].directory_name = directory_name;
		  info->indextable[i].file_name = file_name;
		  info->indextable[i].function_name = NULL;
		  info->indextable[i].idx = i;
		  ++i;
		}

	      directory_name = NULL;
	      file_name = (char *) str + bfd_get_32 (abfd, stab + STRDXOFF);
	      if (file_name == (char *) str)
		{
		  file_name = NULL;
		  nul_fun = NULL;
		}
	      else
		{
		  nul_fun = stab;
		  nul_str = str;
		  if (file_name >= (char *) info->strs + strsize
		      || file_name < (char *) str)
		    file_name = NULL;
		  if (stab + STABSIZE + TYPEOFF < info->stabs + stabsize
		      && *(stab + STABSIZE + TYPEOFF) == (bfd_byte) N_SO)
		    {
		      /* Two consecutive N_SOs are a directory and a
			 file name.  */
		      stab += STABSIZE;
		      directory_name = file_name;
		      file_name = ((char *) str
				   + bfd_get_32 (abfd, stab + STRDXOFF));
		      if (file_name >= (char *) info->strs + strsize
			  || file_name < (char *) str)
			file_name = NULL;
		    }
		}
	      break;

	    case N_SOL:
	      /* The name of an include file.  */
	      file_name = (char *) str + bfd_get_32 (abfd, stab + STRDXOFF);
	      /* PR 17512: file: 0c680a1f.  */
	      /* PR 17512: file: 5da8aec4.  */
	      if (file_name >= (char *) info->strs + strsize
		  || file_name < (char *) str)
		file_name = NULL;
	      break;

	    case N_FUN:
	      /* A function name.  */
	      function_name = (char *) str + bfd_get_32 (abfd, stab + STRDXOFF);
	      if (function_name == (char *) str)
		continue;
	      if (function_name >= (char *) info->strs + strsize
		  || function_name < (char *) str)
		function_name = NULL;

	      nul_fun = NULL;
	      info->indextable[i].val = bfd_get_32 (abfd, stab + VALOFF);
	      info->indextable[i].stab = stab;
	      info->indextable[i].str = str;
	      info->indextable[i].directory_name = directory_name;
	      info->indextable[i].file_name = file_name;
	      info->indextable[i].function_name = function_name;
	      info->indextable[i].idx = i;
	      ++i;
	      break;
	    }
	}

      if (nul_fun != NULL)
	{
	  info->indextable[i].val = bfd_get_32 (abfd, nul_fun + VALOFF);
	  info->indextable[i].stab = nul_fun;
	  info->indextable[i].str = nul_str;
	  info->indextable[i].directory_name = directory_name;
	  info->indextable[i].file_name = file_name;
	  info->indextable[i].function_name = NULL;
	  info->indextable[i].idx = i;
	  ++i;
	}

      info->indextable[i].val = (bfd_vma) -1;
      info->indextable[i].stab = info->stabs + stabsize;
      info->indextable[i].str = str;
      info->indextable[i].directory_name = NULL;
      info->indextable[i].file_name = NULL;
      info->indextable[i].function_name = NULL;
      info->indextable[i].idx = i;
      ++i;

      info->indextablesize = i;
      qsort (info->indextable, (size_t) i, sizeof (struct indexentry),
	     cmpindexentry);
    }

  /* We are passed a section relative offset.  The offsets in the
     stabs information are absolute.  */
  offset += bfd_section_vma (section);

#ifdef ENABLE_CACHING
  if (info->cached_indexentry != NULL
      && offset >= info->cached_offset
      && offset < (info->cached_indexentry + 1)->val)
    {
      stab = info->cached_stab;
      indexentry = info->cached_indexentry;
      file_name = info->cached_file_name;
    }
  else
#endif
    {
      long low, high;
      long mid = -1;

      /* Cache non-existent or invalid.  Do binary search on
	 indextable.  */
      indexentry = NULL;

      low = 0;
      high = info->indextablesize - 1;
      while (low != high)
	{
	  mid = (high + low) / 2;
	  if (offset >= info->indextable[mid].val
	      && offset < info->indextable[mid + 1].val)
	    {
	      indexentry = &info->indextable[mid];
	      break;
	    }

	  if (info->indextable[mid].val > offset)
	    high = mid;
	  else
	    low = mid + 1;
	}

      if (indexentry == NULL)
	return true;

      stab = indexentry->stab + STABSIZE;
      file_name = indexentry->file_name;
    }

  directory_name = indexentry->directory_name;
  str = indexentry->str;

  saw_line = false;
  saw_func = false;
  for (; stab < (indexentry+1)->stab; stab += STABSIZE)
    {
      bool done;
      bfd_vma val;

      done = false;

      switch (stab[TYPEOFF])
	{
	case N_SOL:
	  /* The name of an include file.  */
	  val = bfd_get_32 (abfd, stab + VALOFF);
	  if (val <= offset)
	    {
	      file_name = (char *) str + bfd_get_32 (abfd, stab + STRDXOFF);
	      if (file_name >= (char *) info->strs + strsize
		  || file_name < (char *) str)
		file_name = NULL;
	      *pline = 0;
	    }
	  break;

	case N_SLINE:
	case N_DSLINE:
	case N_BSLINE:
	  /* A line number.  If the function was specified, then the value
	     is relative to the start of the function.  Otherwise, the
	     value is an absolute address.  */
	  val = ((indexentry->function_name ? indexentry->val : 0)
		 + bfd_get_32 (abfd, stab + VALOFF));
	  /* If this line starts before our desired offset, or if it's
	     the first line we've been able to find, use it.  The
	     !saw_line check works around a bug in GCC 2.95.3, which emits
	     the first N_SLINE late.  */
	  if (!saw_line || val <= offset)
	    {
	      *pline = bfd_get_16 (abfd, stab + DESCOFF);

#ifdef ENABLE_CACHING
	      info->cached_stab = stab;
	      info->cached_offset = val;
	      info->cached_file_name = file_name;
	      info->cached_indexentry = indexentry;
#endif
	    }
	  if (val > offset)
	    done = true;
	  saw_line = true;
	  break;

	case N_FUN:
	case N_SO:
	  if (saw_func || saw_line)
	    done = true;
	  saw_func = true;
	  break;
	}

      if (done)
	break;
    }

  *pfound = true;

  if (file_name == NULL || IS_ABSOLUTE_PATH (file_name)
      || directory_name == NULL)
    *pfilename = file_name;
  else
    {
      size_t dirlen;

      dirlen = strlen (directory_name);
      if (info->filename == NULL
	  || filename_ncmp (info->filename, directory_name, dirlen) != 0
	  || filename_cmp (info->filename + dirlen, file_name) != 0)
	{
	  size_t len;

	  /* Don't free info->filename here.  objdump and other
	     apps keep a copy of a previously returned file name
	     pointer.  */
	  len = strlen (file_name) + 1;
	  info->filename = (char *) bfd_alloc (abfd, dirlen + len);
	  if (info->filename == NULL)
	    return false;
	  memcpy (info->filename, directory_name, dirlen);
	  memcpy (info->filename + dirlen, file_name, len);
	}

      *pfilename = info->filename;
    }

  if (indexentry->function_name != NULL)
    {
      char *s;

      /* This will typically be something like main:F(0,1), so we want
	 to clobber the colon.  It's OK to change the name, since the
	 string is in our own local storage anyhow.  */
      s = strchr (indexentry->function_name, ':');
      if (s != NULL)
	*s = '\0';

      *pfnname = indexentry->function_name;
    }

  return true;
}

void
_bfd_stab_cleanup (bfd *abfd ATTRIBUTE_UNUSED, void **pinfo)
{
  struct stab_find_info *info = (struct stab_find_info *) *pinfo;
  if (info == NULL)
    return;

  free (info->indextable);
  free (info->strs);
  free (info->stabs);
}

long
_bfd_nosymbols_canonicalize_symtab (bfd *abfd ATTRIBUTE_UNUSED,
				    asymbol **location ATTRIBUTE_UNUSED)
{
  return 0;
}

void
_bfd_nosymbols_print_symbol (bfd *abfd ATTRIBUTE_UNUSED,
			     void *afile ATTRIBUTE_UNUSED,
			     asymbol *symbol ATTRIBUTE_UNUSED,
			     bfd_print_symbol_type how ATTRIBUTE_UNUSED)
{
}

void
_bfd_nosymbols_get_symbol_info (bfd *abfd ATTRIBUTE_UNUSED,
				asymbol *sym ATTRIBUTE_UNUSED,
				symbol_info *ret ATTRIBUTE_UNUSED)
{
}

const char *
_bfd_nosymbols_get_symbol_version_string (bfd *abfd,
					  asymbol *symbol ATTRIBUTE_UNUSED,
					  bool base_p ATTRIBUTE_UNUSED,
					  bool *hidden ATTRIBUTE_UNUSED)
{
  return (const char *) _bfd_ptr_bfd_null_error (abfd);
}

bool
_bfd_nosymbols_bfd_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED,
					const char *name ATTRIBUTE_UNUSED)
{
  return false;
}

alent *
_bfd_nosymbols_get_lineno (bfd *abfd, asymbol *sym ATTRIBUTE_UNUSED)
{
  return (alent *) _bfd_ptr_bfd_null_error (abfd);
}

bool
_bfd_nosymbols_find_nearest_line
    (bfd *abfd,
     asymbol **symbols ATTRIBUTE_UNUSED,
     asection *section ATTRIBUTE_UNUSED,
     bfd_vma offset ATTRIBUTE_UNUSED,
     const char **filename_ptr ATTRIBUTE_UNUSED,
     const char **functionname_ptr ATTRIBUTE_UNUSED,
     unsigned int *line_ptr ATTRIBUTE_UNUSED,
     unsigned int *discriminator_ptr ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

bool
_bfd_nosymbols_find_nearest_line_with_alt
    (bfd *abfd,
     const char *alt_filename ATTRIBUTE_UNUSED,
     asymbol **symbols ATTRIBUTE_UNUSED,
     asection *section ATTRIBUTE_UNUSED,
     bfd_vma offset ATTRIBUTE_UNUSED,
     const char **filename_ptr ATTRIBUTE_UNUSED,
     const char **functionname_ptr ATTRIBUTE_UNUSED,
     unsigned int *line_ptr ATTRIBUTE_UNUSED,
     unsigned int *discriminator_ptr ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

bool
_bfd_nosymbols_find_line (bfd *abfd,
			  asymbol **symbols ATTRIBUTE_UNUSED,
			  asymbol *symbol ATTRIBUTE_UNUSED,
			  const char **filename_ptr ATTRIBUTE_UNUSED,
			  unsigned int *line_ptr ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

bool
_bfd_nosymbols_find_inliner_info
    (bfd *abfd,
     const char **filename_ptr ATTRIBUTE_UNUSED,
     const char **functionname_ptr ATTRIBUTE_UNUSED,
     unsigned int *line_ptr ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

asymbol *
_bfd_nosymbols_bfd_make_debug_symbol (bfd *abfd)
{
  return (asymbol *) _bfd_ptr_bfd_null_error (abfd);
}

long
_bfd_nosymbols_read_minisymbols (bfd *abfd,
				 bool dynamic ATTRIBUTE_UNUSED,
				 void **minisymsp ATTRIBUTE_UNUSED,
				 unsigned int *sizep ATTRIBUTE_UNUSED)
{
  return _bfd_long_bfd_n1_error (abfd);
}

asymbol *
_bfd_nosymbols_minisymbol_to_symbol (bfd *abfd,
				     bool dynamic ATTRIBUTE_UNUSED,
				     const void *minisym ATTRIBUTE_UNUSED,
				     asymbol *sym ATTRIBUTE_UNUSED)
{
  return (asymbol *) _bfd_ptr_bfd_null_error (abfd);
}

long
_bfd_nodynamic_get_synthetic_symtab (bfd *abfd,
				     long symcount ATTRIBUTE_UNUSED,
				     asymbol **syms ATTRIBUTE_UNUSED,
				     long dynsymcount ATTRIBUTE_UNUSED,
				     asymbol **dynsyms ATTRIBUTE_UNUSED,
				     asymbol **ret ATTRIBUTE_UNUSED)
{
  return _bfd_long_bfd_n1_error (abfd);
}
