/* linker.c -- BFD linker routines
   Copyright (C) 1993-2023 Free Software Foundation, Inc.
   Written by Steve Chamberlain and Ian Lance Taylor, Cygnus Support

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
#include "bfdlink.h"
#include "genlink.h"

/*
SECTION
	Linker Functions

@cindex Linker
	The linker uses three special entry points in the BFD target
	vector.  It is not necessary to write special routines for
	these entry points when creating a new BFD back end, since
	generic versions are provided.  However, writing them can
	speed up linking and make it use significantly less runtime
	memory.

	The first routine creates a hash table used by the other
	routines.  The second routine adds the symbols from an object
	file to the hash table.  The third routine takes all the
	object files and links them together to create the output
	file.  These routines are designed so that the linker proper
	does not need to know anything about the symbols in the object
	files that it is linking.  The linker merely arranges the
	sections as directed by the linker script and lets BFD handle
	the details of symbols and relocs.

	The second routine and third routines are passed a pointer to
	a <<struct bfd_link_info>> structure (defined in
	<<bfdlink.h>>) which holds information relevant to the link,
	including the linker hash table (which was created by the
	first routine) and a set of callback functions to the linker
	proper.

	The generic linker routines are in <<linker.c>>, and use the
	header file <<genlink.h>>.  As of this writing, the only back
	ends which have implemented versions of these routines are
	a.out (in <<aoutx.h>>) and ECOFF (in <<ecoff.c>>).  The a.out
	routines are used as examples throughout this section.

@menu
@* Creating a Linker Hash Table::
@* Adding Symbols to the Hash Table::
@* Performing the Final Link::
@end menu

INODE
Creating a Linker Hash Table, Adding Symbols to the Hash Table, Linker Functions, Linker Functions
SUBSECTION
	Creating a linker hash table

@cindex _bfd_link_hash_table_create in target vector
@cindex target vector (_bfd_link_hash_table_create)
	The linker routines must create a hash table, which must be
	derived from <<struct bfd_link_hash_table>> described in
	<<bfdlink.c>>.  @xref{Hash Tables}, for information on how to
	create a derived hash table.  This entry point is called using
	the target vector of the linker output file.

	The <<_bfd_link_hash_table_create>> entry point must allocate
	and initialize an instance of the desired hash table.  If the
	back end does not require any additional information to be
	stored with the entries in the hash table, the entry point may
	simply create a <<struct bfd_link_hash_table>>.  Most likely,
	however, some additional information will be needed.

	For example, with each entry in the hash table the a.out
	linker keeps the index the symbol has in the final output file
	(this index number is used so that when doing a relocatable
	link the symbol index used in the output file can be quickly
	filled in when copying over a reloc).  The a.out linker code
	defines the required structures and functions for a hash table
	derived from <<struct bfd_link_hash_table>>.  The a.out linker
	hash table is created by the function
	<<NAME(aout,link_hash_table_create)>>; it simply allocates
	space for the hash table, initializes it, and returns a
	pointer to it.

	When writing the linker routines for a new back end, you will
	generally not know exactly which fields will be required until
	you have finished.  You should simply create a new hash table
	which defines no additional fields, and then simply add fields
	as they become necessary.

INODE
Adding Symbols to the Hash Table, Performing the Final Link, Creating a Linker Hash Table, Linker Functions
SUBSECTION
	Adding symbols to the hash table

@cindex _bfd_link_add_symbols in target vector
@cindex target vector (_bfd_link_add_symbols)
	The linker proper will call the <<_bfd_link_add_symbols>>
	entry point for each object file or archive which is to be
	linked (typically these are the files named on the command
	line, but some may also come from the linker script).  The
	entry point is responsible for examining the file.  For an
	object file, BFD must add any relevant symbol information to
	the hash table.  For an archive, BFD must determine which
	elements of the archive should be used and adding them to the
	link.

	The a.out version of this entry point is
	<<NAME(aout,link_add_symbols)>>.

@menu
@* Differing file formats::
@* Adding symbols from an object file::
@* Adding symbols from an archive::
@end menu

INODE
Differing file formats, Adding symbols from an object file, Adding Symbols to the Hash Table, Adding Symbols to the Hash Table
SUBSUBSECTION
	Differing file formats

	Normally all the files involved in a link will be of the same
	format, but it is also possible to link together different
	format object files, and the back end must support that.  The
	<<_bfd_link_add_symbols>> entry point is called via the target
	vector of the file to be added.  This has an important
	consequence: the function may not assume that the hash table
	is the type created by the corresponding
	<<_bfd_link_hash_table_create>> vector.  All the
	<<_bfd_link_add_symbols>> function can assume about the hash
	table is that it is derived from <<struct
	bfd_link_hash_table>>.

	Sometimes the <<_bfd_link_add_symbols>> function must store
	some information in the hash table entry to be used by the
	<<_bfd_final_link>> function.  In such a case the output bfd
	xvec must be checked to make sure that the hash table was
	created by an object file of the same format.

	The <<_bfd_final_link>> routine must be prepared to handle a
	hash entry without any extra information added by the
	<<_bfd_link_add_symbols>> function.  A hash entry without
	extra information will also occur when the linker script
	directs the linker to create a symbol.  Note that, regardless
	of how a hash table entry is added, all the fields will be
	initialized to some sort of null value by the hash table entry
	initialization function.

	See <<ecoff_link_add_externals>> for an example of how to
	check the output bfd before saving information (in this
	case, the ECOFF external symbol debugging information) in a
	hash table entry.

INODE
Adding symbols from an object file, Adding symbols from an archive, Differing file formats, Adding Symbols to the Hash Table
SUBSUBSECTION
	Adding symbols from an object file

	When the <<_bfd_link_add_symbols>> routine is passed an object
	file, it must add all externally visible symbols in that
	object file to the hash table.  The actual work of adding the
	symbol to the hash table is normally handled by the function
	<<_bfd_generic_link_add_one_symbol>>.  The
	<<_bfd_link_add_symbols>> routine is responsible for reading
	all the symbols from the object file and passing the correct
	information to <<_bfd_generic_link_add_one_symbol>>.

	The <<_bfd_link_add_symbols>> routine should not use
	<<bfd_canonicalize_symtab>> to read the symbols.  The point of
	providing this routine is to avoid the overhead of converting
	the symbols into generic <<asymbol>> structures.

@findex _bfd_generic_link_add_one_symbol
	<<_bfd_generic_link_add_one_symbol>> handles the details of
	combining common symbols, warning about multiple definitions,
	and so forth.  It takes arguments which describe the symbol to
	add, notably symbol flags, a section, and an offset.  The
	symbol flags include such things as <<BSF_WEAK>> or
	<<BSF_INDIRECT>>.  The section is a section in the object
	file, or something like <<bfd_und_section_ptr>> for an undefined
	symbol or <<bfd_com_section_ptr>> for a common symbol.

	If the <<_bfd_final_link>> routine is also going to need to
	read the symbol information, the <<_bfd_link_add_symbols>>
	routine should save it somewhere attached to the object file
	BFD.  However, the information should only be saved if the
	<<keep_memory>> field of the <<info>> argument is TRUE, so
	that the <<-no-keep-memory>> linker switch is effective.

	The a.out function which adds symbols from an object file is
	<<aout_link_add_object_symbols>>, and most of the interesting
	work is in <<aout_link_add_symbols>>.  The latter saves
	pointers to the hash tables entries created by
	<<_bfd_generic_link_add_one_symbol>> indexed by symbol number,
	so that the <<_bfd_final_link>> routine does not have to call
	the hash table lookup routine to locate the entry.

INODE
Adding symbols from an archive, , Adding symbols from an object file, Adding Symbols to the Hash Table
SUBSUBSECTION
	Adding symbols from an archive

	When the <<_bfd_link_add_symbols>> routine is passed an
	archive, it must look through the symbols defined by the
	archive and decide which elements of the archive should be
	included in the link.  For each such element it must call the
	<<add_archive_element>> linker callback, and it must add the
	symbols from the object file to the linker hash table.  (The
	callback may in fact indicate that a replacement BFD should be
	used, in which case the symbols from that BFD should be added
	to the linker hash table instead.)

@findex _bfd_generic_link_add_archive_symbols
	In most cases the work of looking through the symbols in the
	archive should be done by the
	<<_bfd_generic_link_add_archive_symbols>> function.
	<<_bfd_generic_link_add_archive_symbols>> is passed a function
	to call to make the final decision about adding an archive
	element to the link and to do the actual work of adding the
	symbols to the linker hash table.  If the element is to
	be included, the <<add_archive_element>> linker callback
	routine must be called with the element as an argument, and
	the element's symbols must be added to the linker hash table
	just as though the element had itself been passed to the
	<<_bfd_link_add_symbols>> function.

	When the a.out <<_bfd_link_add_symbols>> function receives an
	archive, it calls <<_bfd_generic_link_add_archive_symbols>>
	passing <<aout_link_check_archive_element>> as the function
	argument. <<aout_link_check_archive_element>> calls
	<<aout_link_check_ar_symbols>>.  If the latter decides to add
	the element (an element is only added if it provides a real,
	non-common, definition for a previously undefined or common
	symbol) it calls the <<add_archive_element>> callback and then
	<<aout_link_check_archive_element>> calls
	<<aout_link_add_symbols>> to actually add the symbols to the
	linker hash table - possibly those of a substitute BFD, if the
	<<add_archive_element>> callback avails itself of that option.

	The ECOFF back end is unusual in that it does not normally
	call <<_bfd_generic_link_add_archive_symbols>>, because ECOFF
	archives already contain a hash table of symbols.  The ECOFF
	back end searches the archive itself to avoid the overhead of
	creating a new hash table.

INODE
Performing the Final Link, , Adding Symbols to the Hash Table, Linker Functions
SUBSECTION
	Performing the final link

@cindex _bfd_link_final_link in target vector
@cindex target vector (_bfd_final_link)
	When all the input files have been processed, the linker calls
	the <<_bfd_final_link>> entry point of the output BFD.  This
	routine is responsible for producing the final output file,
	which has several aspects.  It must relocate the contents of
	the input sections and copy the data into the output sections.
	It must build an output symbol table including any local
	symbols from the input files and the global symbols from the
	hash table.  When producing relocatable output, it must
	modify the input relocs and write them into the output file.
	There may also be object format dependent work to be done.

	The linker will also call the <<write_object_contents>> entry
	point when the BFD is closed.  The two entry points must work
	together in order to produce the correct output file.

	The details of how this works are inevitably dependent upon
	the specific object file format.  The a.out
	<<_bfd_final_link>> routine is <<NAME(aout,final_link)>>.

@menu
@* Information provided by the linker::
@* Relocating the section contents::
@* Writing the symbol table::
@end menu

INODE
Information provided by the linker, Relocating the section contents, Performing the Final Link, Performing the Final Link
SUBSUBSECTION
	Information provided by the linker

	Before the linker calls the <<_bfd_final_link>> entry point,
	it sets up some data structures for the function to use.

	The <<input_bfds>> field of the <<bfd_link_info>> structure
	will point to a list of all the input files included in the
	link.  These files are linked through the <<link.next>> field
	of the <<bfd>> structure.

	Each section in the output file will have a list of
	<<link_order>> structures attached to the <<map_head.link_order>>
	field (the <<link_order>> structure is defined in
	<<bfdlink.h>>).  These structures describe how to create the
	contents of the output section in terms of the contents of
	various input sections, fill constants, and, eventually, other
	types of information.  They also describe relocs that must be
	created by the BFD backend, but do not correspond to any input
	file; this is used to support -Ur, which builds constructors
	while generating a relocatable object file.

INODE
Relocating the section contents, Writing the symbol table, Information provided by the linker, Performing the Final Link
SUBSUBSECTION
	Relocating the section contents

	The <<_bfd_final_link>> function should look through the
	<<link_order>> structures attached to each section of the
	output file.  Each <<link_order>> structure should either be
	handled specially, or it should be passed to the function
	<<_bfd_default_link_order>> which will do the right thing
	(<<_bfd_default_link_order>> is defined in <<linker.c>>).

	For efficiency, a <<link_order>> of type
	<<bfd_indirect_link_order>> whose associated section belongs
	to a BFD of the same format as the output BFD must be handled
	specially.  This type of <<link_order>> describes part of an
	output section in terms of a section belonging to one of the
	input files.  The <<_bfd_final_link>> function should read the
	contents of the section and any associated relocs, apply the
	relocs to the section contents, and write out the modified
	section contents.  If performing a relocatable link, the
	relocs themselves must also be modified and written out.

@findex _bfd_relocate_contents
@findex _bfd_final_link_relocate
	The functions <<_bfd_relocate_contents>> and
	<<_bfd_final_link_relocate>> provide some general support for
	performing the actual relocations, notably overflow checking.
	Their arguments include information about the symbol the
	relocation is against and a <<reloc_howto_type>> argument
	which describes the relocation to perform.  These functions
	are defined in <<reloc.c>>.

	The a.out function which handles reading, relocating, and
	writing section contents is <<aout_link_input_section>>.  The
	actual relocation is done in <<aout_link_input_section_std>>
	and <<aout_link_input_section_ext>>.

INODE
Writing the symbol table, , Relocating the section contents, Performing the Final Link
SUBSUBSECTION
	Writing the symbol table

	The <<_bfd_final_link>> function must gather all the symbols
	in the input files and write them out.  It must also write out
	all the symbols in the global hash table.  This must be
	controlled by the <<strip>> and <<discard>> fields of the
	<<bfd_link_info>> structure.

	The local symbols of the input files will not have been
	entered into the linker hash table.  The <<_bfd_final_link>>
	routine must consider each input file and include the symbols
	in the output file.  It may be convenient to do this when
	looking through the <<link_order>> structures, or it may be
	done by stepping through the <<input_bfds>> list.

	The <<_bfd_final_link>> routine must also traverse the global
	hash table to gather all the externally visible symbols.  It
	is possible that most of the externally visible symbols may be
	written out when considering the symbols of each input file,
	but it is still necessary to traverse the hash table since the
	linker script may have defined some symbols that are not in
	any of the input files.

	The <<strip>> field of the <<bfd_link_info>> structure
	controls which symbols are written out.  The possible values
	are listed in <<bfdlink.h>>.  If the value is <<strip_some>>,
	then the <<keep_hash>> field of the <<bfd_link_info>>
	structure is a hash table of symbols to keep; each symbol
	should be looked up in this hash table, and only symbols which
	are present should be included in the output file.

	If the <<strip>> field of the <<bfd_link_info>> structure
	permits local symbols to be written out, the <<discard>> field
	is used to further controls which local symbols are included
	in the output file.  If the value is <<discard_l>>, then all
	local symbols which begin with a certain prefix are discarded;
	this is controlled by the <<bfd_is_local_label_name>> entry point.

	The a.out backend handles symbols by calling
	<<aout_link_write_symbols>> on each input BFD and then
	traversing the global hash table with the function
	<<aout_link_write_other_symbol>>.  It builds a string table
	while writing out the symbols, which is written to the output
	file at the end of <<NAME(aout,final_link)>>.
*/

static bool generic_link_add_object_symbols
  (bfd *, struct bfd_link_info *);
static bool generic_link_check_archive_element
  (bfd *, struct bfd_link_info *, struct bfd_link_hash_entry *, const char *,
   bool *);
static bool generic_link_add_symbol_list
  (bfd *, struct bfd_link_info *, bfd_size_type count, asymbol **);
static bool generic_add_output_symbol
  (bfd *, size_t *psymalloc, asymbol *);
static bool default_data_link_order
  (bfd *, struct bfd_link_info *, asection *, struct bfd_link_order *);
static bool default_indirect_link_order
  (bfd *, struct bfd_link_info *, asection *, struct bfd_link_order *,
   bool);

/* The link hash table structure is defined in bfdlink.h.  It provides
   a base hash table which the backend specific hash tables are built
   upon.  */

/* Routine to create an entry in the link hash table.  */

struct bfd_hash_entry *
_bfd_link_hash_newfunc (struct bfd_hash_entry *entry,
			struct bfd_hash_table *table,
			const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = (struct bfd_hash_entry *)
	  bfd_hash_allocate (table, sizeof (struct bfd_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry)
    {
      struct bfd_link_hash_entry *h = (struct bfd_link_hash_entry *) entry;

      /* Initialize the local fields.  */
      memset ((char *) &h->root + sizeof (h->root), 0,
	      sizeof (*h) - sizeof (h->root));
    }

  return entry;
}

/* Initialize a link hash table.  The BFD argument is the one
   responsible for creating this table.  */

bool
_bfd_link_hash_table_init
  (struct bfd_link_hash_table *table,
   bfd *abfd ATTRIBUTE_UNUSED,
   struct bfd_hash_entry *(*newfunc) (struct bfd_hash_entry *,
				      struct bfd_hash_table *,
				      const char *),
   unsigned int entsize)
{
  bool ret;

  BFD_ASSERT (!abfd->is_linker_output && !abfd->link.hash);
  table->undefs = NULL;
  table->undefs_tail = NULL;
  table->type = bfd_link_generic_hash_table;

  ret = bfd_hash_table_init (&table->table, newfunc, entsize);
  if (ret)
    {
      /* Arrange for destruction of this hash table on closing ABFD.  */
      table->hash_table_free = _bfd_generic_link_hash_table_free;
      abfd->link.hash = table;
      abfd->is_linker_output = true;
    }
  return ret;
}

/* Look up a symbol in a link hash table.  If follow is TRUE, we
   follow bfd_link_hash_indirect and bfd_link_hash_warning links to
   the real symbol.

.{* Return TRUE if the symbol described by a linker hash entry H
.   is going to be absolute.  Linker-script defined symbols can be
.   converted from absolute to section-relative ones late in the
.   link.  Use this macro to correctly determine whether the symbol
.   will actually end up absolute in output.  *}
.#define bfd_is_abs_symbol(H) \
.  (((H)->type == bfd_link_hash_defined \
.    || (H)->type == bfd_link_hash_defweak) \
.   && bfd_is_abs_section ((H)->u.def.section) \
.   && !(H)->rel_from_abs)
.
*/

struct bfd_link_hash_entry *
bfd_link_hash_lookup (struct bfd_link_hash_table *table,
		      const char *string,
		      bool create,
		      bool copy,
		      bool follow)
{
  struct bfd_link_hash_entry *ret;

  if (table == NULL || string == NULL)
    return NULL;

  ret = ((struct bfd_link_hash_entry *)
	 bfd_hash_lookup (&table->table, string, create, copy));

  if (follow && ret != NULL)
    {
      while (ret->type == bfd_link_hash_indirect
	     || ret->type == bfd_link_hash_warning)
	ret = ret->u.i.link;
    }

  return ret;
}

/* Look up a symbol in the main linker hash table if the symbol might
   be wrapped.  This should only be used for references to an
   undefined symbol, not for definitions of a symbol.  */

struct bfd_link_hash_entry *
bfd_wrapped_link_hash_lookup (bfd *abfd,
			      struct bfd_link_info *info,
			      const char *string,
			      bool create,
			      bool copy,
			      bool follow)
{
  size_t amt;

  if (info->wrap_hash != NULL)
    {
      const char *l;
      char prefix = '\0';

      l = string;
      if (*l == bfd_get_symbol_leading_char (abfd) || *l == info->wrap_char)
	{
	  prefix = *l;
	  ++l;
	}

#undef WRAP
#define WRAP "__wrap_"

      if (bfd_hash_lookup (info->wrap_hash, l, false, false) != NULL)
	{
	  char *n;
	  struct bfd_link_hash_entry *h;

	  /* This symbol is being wrapped.  We want to replace all
	     references to SYM with references to __wrap_SYM.  */

	  amt = strlen (l) + sizeof WRAP + 1;
	  n = (char *) bfd_malloc (amt);
	  if (n == NULL)
	    return NULL;

	  n[0] = prefix;
	  n[1] = '\0';
	  strcat (n, WRAP);
	  strcat (n, l);
	  h = bfd_link_hash_lookup (info->hash, n, create, true, follow);
	  free (n);
	  return h;
	}

#undef  REAL
#define REAL "__real_"

      if (*l == '_'
	  && startswith (l, REAL)
	  && bfd_hash_lookup (info->wrap_hash, l + sizeof REAL - 1,
			      false, false) != NULL)
	{
	  char *n;
	  struct bfd_link_hash_entry *h;

	  /* This is a reference to __real_SYM, where SYM is being
	     wrapped.  We want to replace all references to __real_SYM
	     with references to SYM.  */

	  amt = strlen (l + sizeof REAL - 1) + 2;
	  n = (char *) bfd_malloc (amt);
	  if (n == NULL)
	    return NULL;

	  n[0] = prefix;
	  n[1] = '\0';
	  strcat (n, l + sizeof REAL - 1);
	  h = bfd_link_hash_lookup (info->hash, n, create, true, follow);
	  if (h != NULL)
	    h->ref_real = 1;
	  free (n);
	  return h;
	}

#undef REAL
    }

  return bfd_link_hash_lookup (info->hash, string, create, copy, follow);
}

/* If H is a wrapped symbol, ie. the symbol name starts with "__wrap_"
   and the remainder is found in wrap_hash, return the real symbol.  */

struct bfd_link_hash_entry *
unwrap_hash_lookup (struct bfd_link_info *info,
		    bfd *input_bfd,
		    struct bfd_link_hash_entry *h)
{
  const char *l = h->root.string;

  if (*l == bfd_get_symbol_leading_char (input_bfd)
      || *l == info->wrap_char)
    ++l;

  if (startswith (l, WRAP))
    {
      l += sizeof WRAP - 1;

      if (bfd_hash_lookup (info->wrap_hash, l, false, false) != NULL)
	{
	  char save = 0;
	  if (l - (sizeof WRAP - 1) != h->root.string)
	    {
	      --l;
	      save = *l;
	      *(char *) l = *h->root.string;
	    }
	  h = bfd_link_hash_lookup (info->hash, l, false, false, false);
	  if (save)
	    *(char *) l = save;
	}
    }
  return h;
}
#undef WRAP

/* Traverse a generic link hash table.  Differs from bfd_hash_traverse
   in the treatment of warning symbols.  When warning symbols are
   created they replace the real symbol, so you don't get to see the
   real symbol in a bfd_hash_traverse.  This traversal calls func with
   the real symbol.  */

void
bfd_link_hash_traverse
  (struct bfd_link_hash_table *htab,
   bool (*func) (struct bfd_link_hash_entry *, void *),
   void *info)
{
  unsigned int i;

  htab->table.frozen = 1;
  for (i = 0; i < htab->table.size; i++)
    {
      struct bfd_link_hash_entry *p;

      p = (struct bfd_link_hash_entry *) htab->table.table[i];
      for (; p != NULL; p = (struct bfd_link_hash_entry *) p->root.next)
	if (!(*func) (p->type == bfd_link_hash_warning ? p->u.i.link : p, info))
	  goto out;
    }
 out:
  htab->table.frozen = 0;
}

/* Add a symbol to the linker hash table undefs list.  */

void
bfd_link_add_undef (struct bfd_link_hash_table *table,
		    struct bfd_link_hash_entry *h)
{
  BFD_ASSERT (h->u.undef.next == NULL);
  if (table->undefs_tail != NULL)
    table->undefs_tail->u.undef.next = h;
  if (table->undefs == NULL)
    table->undefs = h;
  table->undefs_tail = h;
}

/* The undefs list was designed so that in normal use we don't need to
   remove entries.  However, if symbols on the list are changed from
   bfd_link_hash_undefined to either bfd_link_hash_undefweak or
   bfd_link_hash_new for some reason, then they must be removed from the
   list.  Failure to do so might result in the linker attempting to add
   the symbol to the list again at a later stage.  */

void
bfd_link_repair_undef_list (struct bfd_link_hash_table *table)
{
  struct bfd_link_hash_entry **pun;

  pun = &table->undefs;
  while (*pun != NULL)
    {
      struct bfd_link_hash_entry *h = *pun;

      if (h->type == bfd_link_hash_new
	  || h->type == bfd_link_hash_undefweak)
	{
	  *pun = h->u.undef.next;
	  h->u.undef.next = NULL;
	  if (h == table->undefs_tail)
	    {
	      if (pun == &table->undefs)
		table->undefs_tail = NULL;
	      else
		/* pun points at an u.undef.next field.  Go back to
		   the start of the link_hash_entry.  */
		table->undefs_tail = (struct bfd_link_hash_entry *)
		  ((char *) pun - ((char *) &h->u.undef.next - (char *) h));
	      break;
	    }
	}
      else
	pun = &h->u.undef.next;
    }
}

/* Routine to create an entry in a generic link hash table.  */

struct bfd_hash_entry *
_bfd_generic_link_hash_newfunc (struct bfd_hash_entry *entry,
				struct bfd_hash_table *table,
				const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = (struct bfd_hash_entry *)
	bfd_hash_allocate (table, sizeof (struct generic_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_link_hash_newfunc (entry, table, string);
  if (entry)
    {
      struct generic_link_hash_entry *ret;

      /* Set local fields.  */
      ret = (struct generic_link_hash_entry *) entry;
      ret->written = false;
      ret->sym = NULL;
    }

  return entry;
}

/* Create a generic link hash table.  */

struct bfd_link_hash_table *
_bfd_generic_link_hash_table_create (bfd *abfd)
{
  struct generic_link_hash_table *ret;
  size_t amt = sizeof (struct generic_link_hash_table);

  ret = (struct generic_link_hash_table *) bfd_malloc (amt);
  if (ret == NULL)
    return NULL;
  if (! _bfd_link_hash_table_init (&ret->root, abfd,
				   _bfd_generic_link_hash_newfunc,
				   sizeof (struct generic_link_hash_entry)))
    {
      free (ret);
      return NULL;
    }
  return &ret->root;
}

void
_bfd_generic_link_hash_table_free (bfd *obfd)
{
  struct generic_link_hash_table *ret;

  BFD_ASSERT (obfd->is_linker_output && obfd->link.hash);
  ret = (struct generic_link_hash_table *) obfd->link.hash;
  bfd_hash_table_free (&ret->root.table);
  free (ret);
  obfd->link.hash = NULL;
  obfd->is_linker_output = false;
}

/* Grab the symbols for an object file when doing a generic link.  We
   store the symbols in the outsymbols field.  We need to keep them
   around for the entire link to ensure that we only read them once.
   If we read them multiple times, we might wind up with relocs and
   the hash table pointing to different instances of the symbol
   structure.  */

bool
bfd_generic_link_read_symbols (bfd *abfd)
{
  if (bfd_get_outsymbols (abfd) == NULL)
    {
      long symsize;
      long symcount;

      symsize = bfd_get_symtab_upper_bound (abfd);
      if (symsize < 0)
	return false;
      abfd->outsymbols = bfd_alloc (abfd, symsize);
      if (bfd_get_outsymbols (abfd) == NULL && symsize != 0)
	return false;
      symcount = bfd_canonicalize_symtab (abfd, bfd_get_outsymbols (abfd));
      if (symcount < 0)
	return false;
      abfd->symcount = symcount;
    }

  return true;
}

/* Indicate that we are only retrieving symbol values from this
   section.  We want the symbols to act as though the values in the
   file are absolute.  */

void
_bfd_generic_link_just_syms (asection *sec,
			     struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  sec->sec_info_type = SEC_INFO_TYPE_JUST_SYMS;
  sec->output_section = bfd_abs_section_ptr;
  sec->output_offset = sec->vma;
}

/* Copy the symbol type and other attributes for a linker script
   assignment from HSRC to HDEST.
   The default implementation does nothing.  */
void
_bfd_generic_copy_link_hash_symbol_type (bfd *abfd ATTRIBUTE_UNUSED,
    struct bfd_link_hash_entry *hdest ATTRIBUTE_UNUSED,
    struct bfd_link_hash_entry *hsrc ATTRIBUTE_UNUSED)
{
}

/* Generic function to add symbols from an object file to the
   global hash table.  */

bool
_bfd_generic_link_add_symbols (bfd *abfd, struct bfd_link_info *info)
{
  bool ret;

  switch (bfd_get_format (abfd))
    {
    case bfd_object:
      ret = generic_link_add_object_symbols (abfd, info);
      break;
    case bfd_archive:
      ret = (_bfd_generic_link_add_archive_symbols
	     (abfd, info, generic_link_check_archive_element));
      break;
    default:
      bfd_set_error (bfd_error_wrong_format);
      ret = false;
    }

  return ret;
}

/* Add symbols from an object file to the global hash table.  */

static bool
generic_link_add_object_symbols (bfd *abfd,
				 struct bfd_link_info *info)
{
  bfd_size_type symcount;
  struct bfd_symbol **outsyms;

  if (!bfd_generic_link_read_symbols (abfd))
    return false;
  symcount = _bfd_generic_link_get_symcount (abfd);
  outsyms = _bfd_generic_link_get_symbols (abfd);
  return generic_link_add_symbol_list (abfd, info, symcount, outsyms);
}

/* Generic function to add symbols from an archive file to the global
   hash file.  This function presumes that the archive symbol table
   has already been read in (this is normally done by the
   bfd_check_format entry point).  It looks through the archive symbol
   table for symbols that are undefined or common in the linker global
   symbol hash table.  When one is found, the CHECKFN argument is used
   to see if an object file should be included.  This allows targets
   to customize common symbol behaviour.  CHECKFN should set *PNEEDED
   to TRUE if the object file should be included, and must also call
   the bfd_link_info add_archive_element callback function and handle
   adding the symbols to the global hash table.  CHECKFN must notice
   if the callback indicates a substitute BFD, and arrange to add
   those symbols instead if it does so.  CHECKFN should only return
   FALSE if some sort of error occurs.  */

bool
_bfd_generic_link_add_archive_symbols
  (bfd *abfd,
   struct bfd_link_info *info,
   bool (*checkfn) (bfd *, struct bfd_link_info *,
		    struct bfd_link_hash_entry *, const char *, bool *))
{
  bool loop;
  bfd_size_type amt;
  unsigned char *included;

  if (! bfd_has_map (abfd))
    {
      /* An empty archive is a special case.  */
      if (bfd_openr_next_archived_file (abfd, NULL) == NULL)
	return true;
      bfd_set_error (bfd_error_no_armap);
      return false;
    }

  amt = bfd_ardata (abfd)->symdef_count;
  if (amt == 0)
    return true;
  amt *= sizeof (*included);
  included = (unsigned char *) bfd_zmalloc (amt);
  if (included == NULL)
    return false;

  do
    {
      carsym *arsyms;
      carsym *arsym_end;
      carsym *arsym;
      unsigned int indx;
      file_ptr last_ar_offset = -1;
      bool needed = false;
      bfd *element = NULL;

      loop = false;
      arsyms = bfd_ardata (abfd)->symdefs;
      arsym_end = arsyms + bfd_ardata (abfd)->symdef_count;
      for (arsym = arsyms, indx = 0; arsym < arsym_end; arsym++, indx++)
	{
	  struct bfd_link_hash_entry *h;
	  struct bfd_link_hash_entry *undefs_tail;

	  if (included[indx])
	    continue;
	  if (needed && arsym->file_offset == last_ar_offset)
	    {
	      included[indx] = 1;
	      continue;
	    }

	  if (arsym->name == NULL)
	    goto error_return;

	  h = bfd_link_hash_lookup (info->hash, arsym->name,
				    false, false, true);

	  if (h == NULL
	      && info->pei386_auto_import
	      && startswith (arsym->name, "__imp_"))
	    h = bfd_link_hash_lookup (info->hash, arsym->name + 6,
				      false, false, true);
	  if (h == NULL)
	    continue;

	  if (h->type != bfd_link_hash_undefined
	      && h->type != bfd_link_hash_common)
	    {
	      if (h->type != bfd_link_hash_undefweak)
		/* Symbol must be defined.  Don't check it again.  */
		included[indx] = 1;
	      continue;
	    }

	  if (last_ar_offset != arsym->file_offset)
	    {
	      last_ar_offset = arsym->file_offset;
	      element = _bfd_get_elt_at_filepos (abfd, last_ar_offset,
						 info);
	      if (element == NULL
		  || !bfd_check_format (element, bfd_object))
		goto error_return;
	    }

	  undefs_tail = info->hash->undefs_tail;

	  /* CHECKFN will see if this element should be included, and
	     go ahead and include it if appropriate.  */
	  if (! (*checkfn) (element, info, h, arsym->name, &needed))
	    goto error_return;

	  if (needed)
	    {
	      unsigned int mark;

	      /* Look backward to mark all symbols from this object file
		 which we have already seen in this pass.  */
	      mark = indx;
	      do
		{
		  included[mark] = 1;
		  if (mark == 0)
		    break;
		  --mark;
		}
	      while (arsyms[mark].file_offset == last_ar_offset);

	      if (undefs_tail != info->hash->undefs_tail)
		loop = true;
	    }
	}
    } while (loop);

  free (included);
  return true;

 error_return:
  free (included);
  return false;
}

/* See if we should include an archive element.  */

static bool
generic_link_check_archive_element (bfd *abfd,
				    struct bfd_link_info *info,
				    struct bfd_link_hash_entry *h,
				    const char *name ATTRIBUTE_UNUSED,
				    bool *pneeded)
{
  asymbol **pp, **ppend;

  *pneeded = false;

  if (!bfd_generic_link_read_symbols (abfd))
    return false;

  pp = _bfd_generic_link_get_symbols (abfd);
  ppend = pp + _bfd_generic_link_get_symcount (abfd);
  for (; pp < ppend; pp++)
    {
      asymbol *p;

      p = *pp;

      /* We are only interested in globally visible symbols.  */
      if (! bfd_is_com_section (p->section)
	  && (p->flags & (BSF_GLOBAL | BSF_INDIRECT | BSF_WEAK)) == 0)
	continue;

      /* We are only interested if we know something about this
	 symbol, and it is undefined or common.  An undefined weak
	 symbol (type bfd_link_hash_undefweak) is not considered to be
	 a reference when pulling files out of an archive.  See the
	 SVR4 ABI, p. 4-27.  */
      h = bfd_link_hash_lookup (info->hash, bfd_asymbol_name (p), false,
				false, true);
      if (h == NULL
	  || (h->type != bfd_link_hash_undefined
	      && h->type != bfd_link_hash_common))
	continue;

      /* P is a symbol we are looking for.  */

      if (! bfd_is_com_section (p->section)
	  || (h->type == bfd_link_hash_undefined
	      && h->u.undef.abfd == NULL))
	{
	  /* P is not a common symbol, or an undefined reference was
	     created from outside BFD such as from a linker -u option.
	     This object file defines the symbol, so pull it in.  */
	  *pneeded = true;
	  if (!(*info->callbacks
		->add_archive_element) (info, abfd, bfd_asymbol_name (p),
					&abfd))
	    return false;
	  /* Potentially, the add_archive_element hook may have set a
	     substitute BFD for us.  */
	  return bfd_link_add_symbols (abfd, info);
	}

      /* P is a common symbol.  */

      if (h->type == bfd_link_hash_undefined)
	{
	  bfd *symbfd;
	  bfd_vma size;
	  unsigned int power;

	  /* Turn the symbol into a common symbol but do not link in
	     the object file.  This is how a.out works.  Object
	     formats that require different semantics must implement
	     this function differently.  This symbol is already on the
	     undefs list.  We add the section to a common section
	     attached to symbfd to ensure that it is in a BFD which
	     will be linked in.  */
	  symbfd = h->u.undef.abfd;
	  h->type = bfd_link_hash_common;
	  h->u.c.p = (struct bfd_link_hash_common_entry *)
	    bfd_hash_allocate (&info->hash->table,
			       sizeof (struct bfd_link_hash_common_entry));
	  if (h->u.c.p == NULL)
	    return false;

	  size = bfd_asymbol_value (p);
	  h->u.c.size = size;

	  power = bfd_log2 (size);
	  if (power > 4)
	    power = 4;
	  h->u.c.p->alignment_power = power;

	  if (p->section == bfd_com_section_ptr)
	    h->u.c.p->section = bfd_make_section_old_way (symbfd, "COMMON");
	  else
	    h->u.c.p->section = bfd_make_section_old_way (symbfd,
							  p->section->name);
	  h->u.c.p->section->flags |= SEC_ALLOC;
	}
      else
	{
	  /* Adjust the size of the common symbol if necessary.  This
	     is how a.out works.  Object formats that require
	     different semantics must implement this function
	     differently.  */
	  if (bfd_asymbol_value (p) > h->u.c.size)
	    h->u.c.size = bfd_asymbol_value (p);
	}
    }

  /* This archive element is not needed.  */
  return true;
}

/* Add the symbols from an object file to the global hash table.  ABFD
   is the object file.  INFO is the linker information.  SYMBOL_COUNT
   is the number of symbols.  SYMBOLS is the list of symbols.  */

static bool
generic_link_add_symbol_list (bfd *abfd,
			      struct bfd_link_info *info,
			      bfd_size_type symbol_count,
			      asymbol **symbols)
{
  asymbol **pp, **ppend;

  pp = symbols;
  ppend = symbols + symbol_count;
  for (; pp < ppend; pp++)
    {
      asymbol *p;

      p = *pp;

      if ((p->flags & (BSF_INDIRECT
		       | BSF_WARNING
		       | BSF_GLOBAL
		       | BSF_CONSTRUCTOR
		       | BSF_WEAK)) != 0
	  || bfd_is_und_section (bfd_asymbol_section (p))
	  || bfd_is_com_section (bfd_asymbol_section (p))
	  || bfd_is_ind_section (bfd_asymbol_section (p)))
	{
	  const char *name;
	  const char *string;
	  struct generic_link_hash_entry *h;
	  struct bfd_link_hash_entry *bh;

	  string = name = bfd_asymbol_name (p);
	  if (((p->flags & BSF_INDIRECT) != 0
	       || bfd_is_ind_section (p->section))
	      && pp + 1 < ppend)
	    {
	      pp++;
	      string = bfd_asymbol_name (*pp);
	    }
	  else if ((p->flags & BSF_WARNING) != 0
		   && pp + 1 < ppend)
	    {
	      /* The name of P is actually the warning string, and the
		 next symbol is the one to warn about.  */
	      pp++;
	      name = bfd_asymbol_name (*pp);
	    }

	  bh = NULL;
	  if (! (_bfd_generic_link_add_one_symbol
		 (info, abfd, name, p->flags, bfd_asymbol_section (p),
		  p->value, string, false, false, &bh)))
	    return false;
	  h = (struct generic_link_hash_entry *) bh;

	  /* If this is a constructor symbol, and the linker didn't do
	     anything with it, then we want to just pass the symbol
	     through to the output file.  This will happen when
	     linking with -r.  */
	  if ((p->flags & BSF_CONSTRUCTOR) != 0
	      && (h == NULL || h->root.type == bfd_link_hash_new))
	    {
	      p->udata.p = NULL;
	      continue;
	    }

	  /* Save the BFD symbol so that we don't lose any backend
	     specific information that may be attached to it.  We only
	     want this one if it gives more information than the
	     existing one; we don't want to replace a defined symbol
	     with an undefined one.  This routine may be called with a
	     hash table other than the generic hash table, so we only
	     do this if we are certain that the hash table is a
	     generic one.  */
	  if (info->output_bfd->xvec == abfd->xvec)
	    {
	      if (h->sym == NULL
		  || (! bfd_is_und_section (bfd_asymbol_section (p))
		      && (! bfd_is_com_section (bfd_asymbol_section (p))
			  || bfd_is_und_section (bfd_asymbol_section (h->sym)))))
		{
		  h->sym = p;
		  /* BSF_OLD_COMMON is a hack to support COFF reloc
		     reading, and it should go away when the COFF
		     linker is switched to the new version.  */
		  if (bfd_is_com_section (bfd_asymbol_section (p)))
		    p->flags |= BSF_OLD_COMMON;
		}
	    }

	  /* Store a back pointer from the symbol to the hash
	     table entry for the benefit of relaxation code until
	     it gets rewritten to not use asymbol structures.
	     Setting this is also used to check whether these
	     symbols were set up by the generic linker.  */
	  p->udata.p = h;
	}
    }

  return true;
}

/* We use a state table to deal with adding symbols from an object
   file.  The first index into the state table describes the symbol
   from the object file.  The second index into the state table is the
   type of the symbol in the hash table.  */

/* The symbol from the object file is turned into one of these row
   values.  */

enum link_row
{
  UNDEF_ROW,		/* Undefined.  */
  UNDEFW_ROW,		/* Weak undefined.  */
  DEF_ROW,		/* Defined.  */
  DEFW_ROW,		/* Weak defined.  */
  COMMON_ROW,		/* Common.  */
  INDR_ROW,		/* Indirect.  */
  WARN_ROW,		/* Warning.  */
  SET_ROW		/* Member of set.  */
};

/* apparently needed for Hitachi 3050R(HI-UX/WE2)? */
#undef FAIL

/* The actions to take in the state table.  */

enum link_action
{
  FAIL,		/* Abort.  */
  UND,		/* Mark symbol undefined.  */
  WEAK,		/* Mark symbol weak undefined.  */
  DEF,		/* Mark symbol defined.  */
  DEFW,		/* Mark symbol weak defined.  */
  COM,		/* Mark symbol common.  */
  REF,		/* Mark defined symbol referenced.  */
  CREF,		/* Possibly warn about common reference to defined symbol.  */
  CDEF,		/* Define existing common symbol.  */
  NOACT,	/* No action.  */
  BIG,		/* Mark symbol common using largest size.  */
  MDEF,		/* Multiple definition error.  */
  MIND,		/* Multiple indirect symbols.  */
  IND,		/* Make indirect symbol.  */
  CIND,		/* Make indirect symbol from existing common symbol.  */
  SET,		/* Add value to set.  */
  MWARN,	/* Make warning symbol.  */
  WARN,		/* Warn if referenced, else MWARN.  */
  CYCLE,	/* Repeat with symbol pointed to.  */
  REFC,		/* Mark indirect symbol referenced and then CYCLE.  */
  WARNC		/* Issue warning and then CYCLE.  */
};

/* The state table itself.  The first index is a link_row and the
   second index is a bfd_link_hash_type.  */

static const enum link_action link_action[8][8] =
{
  /* current\prev    new    undef  undefw def    defw   com    indr   warn  */
  /* UNDEF_ROW	*/  {UND,   NOACT, UND,   REF,   REF,   NOACT, REFC,  WARNC },
  /* UNDEFW_ROW	*/  {WEAK,  NOACT, NOACT, REF,   REF,   NOACT, REFC,  WARNC },
  /* DEF_ROW	*/  {DEF,   DEF,   DEF,   MDEF,  DEF,   CDEF,  MIND,  CYCLE },
  /* DEFW_ROW	*/  {DEFW,  DEFW,  DEFW,  NOACT, NOACT, NOACT, NOACT, CYCLE },
  /* COMMON_ROW	*/  {COM,   COM,   COM,   CREF,  COM,   BIG,   REFC,  WARNC },
  /* INDR_ROW	*/  {IND,   IND,   IND,   MDEF,  IND,   CIND,  MIND,  CYCLE },
  /* WARN_ROW   */  {MWARN, WARN,  WARN,  WARN,  WARN,  WARN,  WARN,  NOACT },
  /* SET_ROW	*/  {SET,   SET,   SET,   SET,   SET,   SET,   CYCLE, CYCLE }
};

/* Most of the entries in the LINK_ACTION table are straightforward,
   but a few are somewhat subtle.

   A reference to an indirect symbol (UNDEF_ROW/indr or
   UNDEFW_ROW/indr) is counted as a reference both to the indirect
   symbol and to the symbol the indirect symbol points to.

   A reference to a warning symbol (UNDEF_ROW/warn or UNDEFW_ROW/warn)
   causes the warning to be issued.

   A common definition of an indirect symbol (COMMON_ROW/indr) is
   treated as a multiple definition error.  Likewise for an indirect
   definition of a common symbol (INDR_ROW/com).

   An indirect definition of a warning (INDR_ROW/warn) does not cause
   the warning to be issued.

   If a warning is created for an indirect symbol (WARN_ROW/indr) no
   warning is created for the symbol the indirect symbol points to.

   Adding an entry to a set does not count as a reference to a set,
   and no warning is issued (SET_ROW/warn).  */

/* Return the BFD in which a hash entry has been defined, if known.  */

static bfd *
hash_entry_bfd (struct bfd_link_hash_entry *h)
{
  while (h->type == bfd_link_hash_warning)
    h = h->u.i.link;
  switch (h->type)
    {
    default:
      return NULL;
    case bfd_link_hash_undefined:
    case bfd_link_hash_undefweak:
      return h->u.undef.abfd;
    case bfd_link_hash_defined:
    case bfd_link_hash_defweak:
      return h->u.def.section->owner;
    case bfd_link_hash_common:
      return h->u.c.p->section->owner;
    }
  /*NOTREACHED*/
}

/* Add a symbol to the global hash table.
   ABFD is the BFD the symbol comes from.
   NAME is the name of the symbol.
   FLAGS is the BSF_* bits associated with the symbol.
   SECTION is the section in which the symbol is defined; this may be
     bfd_und_section_ptr or bfd_com_section_ptr.
   VALUE is the value of the symbol, relative to the section.
   STRING is used for either an indirect symbol, in which case it is
     the name of the symbol to indirect to, or a warning symbol, in
     which case it is the warning string.
   COPY is TRUE if NAME or STRING must be copied into locally
     allocated memory if they need to be saved.
   COLLECT is TRUE if we should automatically collect gcc constructor
     or destructor names as collect2 does.
   HASHP, if not NULL, is a place to store the created hash table
     entry; if *HASHP is not NULL, the caller has already looked up
     the hash table entry, and stored it in *HASHP.  */

bool
_bfd_generic_link_add_one_symbol (struct bfd_link_info *info,
				  bfd *abfd,
				  const char *name,
				  flagword flags,
				  asection *section,
				  bfd_vma value,
				  const char *string,
				  bool copy,
				  bool collect,
				  struct bfd_link_hash_entry **hashp)
{
  enum link_row row;
  struct bfd_link_hash_entry *h;
  struct bfd_link_hash_entry *inh = NULL;
  bool cycle;

  BFD_ASSERT (section != NULL);

  if (bfd_is_ind_section (section)
      || (flags & BSF_INDIRECT) != 0)
    {
      row = INDR_ROW;
      /* Create the indirect symbol here.  This is for the benefit of
	 the plugin "notice" function.
	 STRING is the name of the symbol we want to indirect to.  */
      inh = bfd_wrapped_link_hash_lookup (abfd, info, string, true,
					  copy, false);
      if (inh == NULL)
	return false;
    }
  else if ((flags & BSF_WARNING) != 0)
    row = WARN_ROW;
  else if ((flags & BSF_CONSTRUCTOR) != 0)
    row = SET_ROW;
  else if (bfd_is_und_section (section))
    {
      if ((flags & BSF_WEAK) != 0)
	row = UNDEFW_ROW;
      else
	row = UNDEF_ROW;
    }
  else if ((flags & BSF_WEAK) != 0)
    row = DEFW_ROW;
  else if (bfd_is_com_section (section))
    {
      row = COMMON_ROW;
      if (!bfd_link_relocatable (info)
	  && name != NULL
	  && name[0] == '_'
	  && name[1] == '_'
	  && strcmp (name + (name[2] == '_'), "__gnu_lto_slim") == 0)
	_bfd_error_handler
	  (_("%pB: plugin needed to handle lto object"), abfd);
    }
  else
    row = DEF_ROW;

  if (hashp != NULL && *hashp != NULL)
    h = *hashp;
  else
    {
      if (row == UNDEF_ROW || row == UNDEFW_ROW)
	h = bfd_wrapped_link_hash_lookup (abfd, info, name, true, copy, false);
      else
	h = bfd_link_hash_lookup (info->hash, name, true, copy, false);
      if (h == NULL)
	{
	  if (hashp != NULL)
	    *hashp = NULL;
	  return false;
	}
    }

  if (info->notice_all
      || (info->notice_hash != NULL
	  && bfd_hash_lookup (info->notice_hash, name, false, false) != NULL))
    {
      if (! (*info->callbacks->notice) (info, h, inh,
					abfd, section, value, flags))
	return false;
    }

  if (hashp != NULL)
    *hashp = h;

  do
    {
      enum link_action action;
      int prev;

      prev = h->type;
      /* Treat symbols defined by early linker script pass as undefined.  */
      if (h->ldscript_def)
	prev = bfd_link_hash_undefined;
      cycle = false;
      action = link_action[(int) row][prev];
      switch (action)
	{
	case FAIL:
	  abort ();

	case NOACT:
	  /* Do nothing.  */
	  break;

	case UND:
	  /* Make a new undefined symbol.  */
	  h->type = bfd_link_hash_undefined;
	  h->u.undef.abfd = abfd;
	  bfd_link_add_undef (info->hash, h);
	  break;

	case WEAK:
	  /* Make a new weak undefined symbol.  */
	  h->type = bfd_link_hash_undefweak;
	  h->u.undef.abfd = abfd;
	  break;

	case CDEF:
	  /* We have found a definition for a symbol which was
	     previously common.  */
	  BFD_ASSERT (h->type == bfd_link_hash_common);
	  (*info->callbacks->multiple_common) (info, h, abfd,
					       bfd_link_hash_defined, 0);
	  /* Fall through.  */
	case DEF:
	case DEFW:
	  {
	    enum bfd_link_hash_type oldtype;

	    /* Define a symbol.  */
	    oldtype = h->type;
	    if (action == DEFW)
	      h->type = bfd_link_hash_defweak;
	    else
	      h->type = bfd_link_hash_defined;
	    h->u.def.section = section;
	    h->u.def.value = value;
	    h->linker_def = 0;
	    h->ldscript_def = 0;

	    /* If we have been asked to, we act like collect2 and
	       identify all functions that might be global
	       constructors and destructors and pass them up in a
	       callback.  We only do this for certain object file
	       types, since many object file types can handle this
	       automatically.  */
	    if (collect && name[0] == '_')
	      {
		const char *s;

		/* A constructor or destructor name starts like this:
		   _+GLOBAL_[_.$][ID][_.$] where the first [_.$] and
		   the second are the same character (we accept any
		   character there, in case a new object file format
		   comes along with even worse naming restrictions).  */

#define CONS_PREFIX "GLOBAL_"
#define CONS_PREFIX_LEN (sizeof CONS_PREFIX - 1)

		s = name + 1;
		while (*s == '_')
		  ++s;
		if (s[0] == 'G' && startswith (s, CONS_PREFIX))
		  {
		    char c;

		    c = s[CONS_PREFIX_LEN + 1];
		    if ((c == 'I' || c == 'D')
			&& s[CONS_PREFIX_LEN] == s[CONS_PREFIX_LEN + 2])
		      {
			/* If this is a definition of a symbol which
			   was previously weakly defined, we are in
			   trouble.  We have already added a
			   constructor entry for the weak defined
			   symbol, and now we are trying to add one
			   for the new symbol.  Fortunately, this case
			   should never arise in practice.  */
			if (oldtype == bfd_link_hash_defweak)
			  abort ();

			(*info->callbacks->constructor) (info, c == 'I',
							 h->root.string, abfd,
							 section, value);
		      }
		  }
	      }
	  }

	  break;

	case COM:
	  /* We have found a common definition for a symbol.  */
	  if (h->type == bfd_link_hash_new)
	    bfd_link_add_undef (info->hash, h);
	  h->type = bfd_link_hash_common;
	  h->u.c.p = (struct bfd_link_hash_common_entry *)
	    bfd_hash_allocate (&info->hash->table,
			       sizeof (struct bfd_link_hash_common_entry));
	  if (h->u.c.p == NULL)
	    return false;

	  h->u.c.size = value;

	  /* Select a default alignment based on the size.  This may
	     be overridden by the caller.  */
	  {
	    unsigned int power;

	    power = bfd_log2 (value);
	    if (power > 4)
	      power = 4;
	    h->u.c.p->alignment_power = power;
	  }

	  /* The section of a common symbol is only used if the common
	     symbol is actually allocated.  It basically provides a
	     hook for the linker script to decide which output section
	     the common symbols should be put in.  In most cases, the
	     section of a common symbol will be bfd_com_section_ptr,
	     the code here will choose a common symbol section named
	     "COMMON", and the linker script will contain *(COMMON) in
	     the appropriate place.  A few targets use separate common
	     sections for small symbols, and they require special
	     handling.  */
	  if (section == bfd_com_section_ptr)
	    {
	      h->u.c.p->section = bfd_make_section_old_way (abfd, "COMMON");
	      h->u.c.p->section->flags |= SEC_ALLOC;
	    }
	  else if (section->owner != abfd)
	    {
	      h->u.c.p->section = bfd_make_section_old_way (abfd,
							    section->name);
	      h->u.c.p->section->flags |= SEC_ALLOC;
	    }
	  else
	    h->u.c.p->section = section;
	  h->linker_def = 0;
	  h->ldscript_def = 0;
	  break;

	case REF:
	  /* A reference to a defined symbol.  */
	  if (h->u.undef.next == NULL && info->hash->undefs_tail != h)
	    h->u.undef.next = h;
	  break;

	case BIG:
	  /* We have found a common definition for a symbol which
	     already had a common definition.  Use the maximum of the
	     two sizes, and use the section required by the larger symbol.  */
	  BFD_ASSERT (h->type == bfd_link_hash_common);
	  (*info->callbacks->multiple_common) (info, h, abfd,
					       bfd_link_hash_common, value);
	  if (value > h->u.c.size)
	    {
	      unsigned int power;

	      h->u.c.size = value;

	      /* Select a default alignment based on the size.  This may
		 be overridden by the caller.  */
	      power = bfd_log2 (value);
	      if (power > 4)
		power = 4;
	      h->u.c.p->alignment_power = power;

	      /* Some systems have special treatment for small commons,
		 hence we want to select the section used by the larger
		 symbol.  This makes sure the symbol does not go in a
		 small common section if it is now too large.  */
	      if (section == bfd_com_section_ptr)
		{
		  h->u.c.p->section
		    = bfd_make_section_old_way (abfd, "COMMON");
		  h->u.c.p->section->flags |= SEC_ALLOC;
		}
	      else if (section->owner != abfd)
		{
		  h->u.c.p->section
		    = bfd_make_section_old_way (abfd, section->name);
		  h->u.c.p->section->flags |= SEC_ALLOC;
		}
	      else
		h->u.c.p->section = section;
	    }
	  break;

	case CREF:
	  /* We have found a common definition for a symbol which
	     was already defined.  */
	  (*info->callbacks->multiple_common) (info, h, abfd,
					       bfd_link_hash_common, value);
	  break;

	case MIND:
	  /* Multiple indirect symbols.  This is OK if they both point
	     to the same symbol.  */
	  if (h->u.i.link->type == bfd_link_hash_defweak)
	    {
	      /* It is also OK to redefine a symbol that indirects to
		 a weak definition.  So for sym@ver -> sym@@ver where
		 sym@@ver is weak and we have a new strong sym@ver,
		 redefine sym@@ver.  Of course if there exists
		 sym -> sym@@ver then this also redefines sym.  */
	      h = h->u.i.link;
	      cycle = true;
	      break;
	    }
	  if (string != NULL && strcmp (h->u.i.link->root.string, string) == 0)
	    break;
	  /* Fall through.  */
	case MDEF:
	  /* Handle a multiple definition.  */
	  (*info->callbacks->multiple_definition) (info, h,
						   abfd, section, value);
	  break;

	case CIND:
	  /* Create an indirect symbol from an existing common symbol.  */
	  BFD_ASSERT (h->type == bfd_link_hash_common);
	  (*info->callbacks->multiple_common) (info, h, abfd,
					       bfd_link_hash_indirect, 0);
	  /* Fall through.  */
	case IND:
	  if (inh->type == bfd_link_hash_indirect
	      && inh->u.i.link == h)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: indirect symbol `%s' to `%s' is a loop"),
		 abfd, name, string);
	      bfd_set_error (bfd_error_invalid_operation);
	      return false;
	    }
	  if (inh->type == bfd_link_hash_new)
	    {
	      inh->type = bfd_link_hash_undefined;
	      inh->u.undef.abfd = abfd;
	      bfd_link_add_undef (info->hash, inh);
	    }

	  /* If the indirect symbol has been referenced, we need to
	     push the reference down to the symbol we are referencing.  */
	  if (h->type != bfd_link_hash_new)
	    {
	      /* ??? If inh->type == bfd_link_hash_undefweak this
		 converts inh to bfd_link_hash_undefined.  */
	      row = UNDEF_ROW;
	      cycle = true;
	    }

	  h->type = bfd_link_hash_indirect;
	  h->u.i.link = inh;
	  /* Not setting h = h->u.i.link here means that when cycle is
	     set above we'll always go to REFC, and then cycle again
	     to the indirected symbol.  This means that any successful
	     change of an existing symbol to indirect counts as a
	     reference.  ??? That may not be correct when the existing
	     symbol was defweak.  */
	  break;

	case SET:
	  /* Add an entry to a set.  */
	  (*info->callbacks->add_to_set) (info, h, BFD_RELOC_CTOR,
					  abfd, section, value);
	  break;

	case WARNC:
	  /* Issue a warning and cycle, except when the reference is
	     in LTO IR.  */
	  if (h->u.i.warning != NULL
	      && (abfd->flags & BFD_PLUGIN) == 0)
	    {
	      (*info->callbacks->warning) (info, h->u.i.warning,
					   h->root.string, abfd, NULL, 0);
	      /* Only issue a warning once.  */
	      h->u.i.warning = NULL;
	    }
	  /* Fall through.  */
	case CYCLE:
	  /* Try again with the referenced symbol.  */
	  h = h->u.i.link;
	  cycle = true;
	  break;

	case REFC:
	  /* A reference to an indirect symbol.  */
	  if (h->u.undef.next == NULL && info->hash->undefs_tail != h)
	    h->u.undef.next = h;
	  h = h->u.i.link;
	  cycle = true;
	  break;

	case WARN:
	  /* Warn if this symbol has been referenced already from non-IR,
	     otherwise add a warning.  */
	  if ((!info->lto_plugin_active
	       && (h->u.undef.next != NULL || info->hash->undefs_tail == h))
	      || h->non_ir_ref_regular
	      || h->non_ir_ref_dynamic)
	    {
	      (*info->callbacks->warning) (info, string, h->root.string,
					   hash_entry_bfd (h), NULL, 0);
	      break;
	    }
	  /* Fall through.  */
	case MWARN:
	  /* Make a warning symbol.  */
	  {
	    struct bfd_link_hash_entry *sub;

	    /* STRING is the warning to give.  */
	    sub = ((struct bfd_link_hash_entry *)
		   ((*info->hash->table.newfunc)
		    (NULL, &info->hash->table, h->root.string)));
	    if (sub == NULL)
	      return false;
	    *sub = *h;
	    sub->type = bfd_link_hash_warning;
	    sub->u.i.link = h;
	    if (! copy)
	      sub->u.i.warning = string;
	    else
	      {
		char *w;
		size_t len = strlen (string) + 1;

		w = (char *) bfd_hash_allocate (&info->hash->table, len);
		if (w == NULL)
		  return false;
		memcpy (w, string, len);
		sub->u.i.warning = w;
	      }

	    bfd_hash_replace (&info->hash->table,
			      (struct bfd_hash_entry *) h,
			      (struct bfd_hash_entry *) sub);
	    if (hashp != NULL)
	      *hashp = sub;
	  }
	  break;
	}
    }
  while (cycle);

  return true;
}

/* Generic final link routine.  */

bool
_bfd_generic_final_link (bfd *abfd, struct bfd_link_info *info)
{
  bfd *sub;
  asection *o;
  struct bfd_link_order *p;
  size_t outsymalloc;
  struct generic_write_global_symbol_info wginfo;

  abfd->outsymbols = NULL;
  abfd->symcount = 0;
  outsymalloc = 0;

  /* Mark all sections which will be included in the output file.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    for (p = o->map_head.link_order; p != NULL; p = p->next)
      if (p->type == bfd_indirect_link_order)
	p->u.indirect.section->linker_mark = true;

  /* Build the output symbol table.  */
  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    if (! _bfd_generic_link_output_symbols (abfd, sub, info, &outsymalloc))
      return false;

  /* Accumulate the global symbols.  */
  wginfo.info = info;
  wginfo.output_bfd = abfd;
  wginfo.psymalloc = &outsymalloc;
  _bfd_generic_link_hash_traverse (_bfd_generic_hash_table (info),
				   _bfd_generic_link_write_global_symbol,
				   &wginfo);

  /* Make sure we have a trailing NULL pointer on OUTSYMBOLS.  We
     shouldn't really need one, since we have SYMCOUNT, but some old
     code still expects one.  */
  if (! generic_add_output_symbol (abfd, &outsymalloc, NULL))
    return false;

  if (bfd_link_relocatable (info))
    {
      /* Allocate space for the output relocs for each section.  */
      for (o = abfd->sections; o != NULL; o = o->next)
	{
	  o->reloc_count = 0;
	  for (p = o->map_head.link_order; p != NULL; p = p->next)
	    {
	      if (p->type == bfd_section_reloc_link_order
		  || p->type == bfd_symbol_reloc_link_order)
		++o->reloc_count;
	      else if (p->type == bfd_indirect_link_order)
		{
		  asection *input_section;
		  bfd *input_bfd;
		  long relsize;
		  arelent **relocs;
		  asymbol **symbols;
		  long reloc_count;

		  input_section = p->u.indirect.section;
		  input_bfd = input_section->owner;
		  relsize = bfd_get_reloc_upper_bound (input_bfd,
						       input_section);
		  if (relsize < 0)
		    return false;
		  relocs = (arelent **) bfd_malloc (relsize);
		  if (!relocs && relsize != 0)
		    return false;
		  symbols = _bfd_generic_link_get_symbols (input_bfd);
		  reloc_count = bfd_canonicalize_reloc (input_bfd,
							input_section,
							relocs,
							symbols);
		  free (relocs);
		  if (reloc_count < 0)
		    return false;
		  BFD_ASSERT ((unsigned long) reloc_count
			      == input_section->reloc_count);
		  o->reloc_count += reloc_count;
		}
	    }
	  if (o->reloc_count > 0)
	    {
	      bfd_size_type amt;

	      amt = o->reloc_count;
	      amt *= sizeof (arelent *);
	      o->orelocation = (struct reloc_cache_entry **) bfd_alloc (abfd, amt);
	      if (!o->orelocation)
		return false;
	      o->flags |= SEC_RELOC;
	      /* Reset the count so that it can be used as an index
		 when putting in the output relocs.  */
	      o->reloc_count = 0;
	    }
	}
    }

  /* Handle all the link order information for the sections.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      for (p = o->map_head.link_order; p != NULL; p = p->next)
	{
	  switch (p->type)
	    {
	    case bfd_section_reloc_link_order:
	    case bfd_symbol_reloc_link_order:
	      if (! _bfd_generic_reloc_link_order (abfd, info, o, p))
		return false;
	      break;
	    case bfd_indirect_link_order:
	      if (! default_indirect_link_order (abfd, info, o, p, true))
		return false;
	      break;
	    default:
	      if (! _bfd_default_link_order (abfd, info, o, p))
		return false;
	      break;
	    }
	}
    }

  return true;
}

/* Add an output symbol to the output BFD.  */

static bool
generic_add_output_symbol (bfd *output_bfd, size_t *psymalloc, asymbol *sym)
{
  if (bfd_get_symcount (output_bfd) >= *psymalloc)
    {
      asymbol **newsyms;
      bfd_size_type amt;

      if (*psymalloc == 0)
	*psymalloc = 124;
      else
	*psymalloc *= 2;
      amt = *psymalloc;
      amt *= sizeof (asymbol *);
      newsyms = (asymbol **) bfd_realloc (bfd_get_outsymbols (output_bfd), amt);
      if (newsyms == NULL)
	return false;
      output_bfd->outsymbols = newsyms;
    }

  output_bfd->outsymbols[output_bfd->symcount] = sym;
  if (sym != NULL)
    ++output_bfd->symcount;

  return true;
}

/* Handle the symbols for an input BFD.  */

bool
_bfd_generic_link_output_symbols (bfd *output_bfd,
				  bfd *input_bfd,
				  struct bfd_link_info *info,
				  size_t *psymalloc)
{
  asymbol **sym_ptr;
  asymbol **sym_end;

  if (!bfd_generic_link_read_symbols (input_bfd))
    return false;

  /* Create a filename symbol if we are supposed to.  */
  if (info->create_object_symbols_section != NULL)
    {
      asection *sec;

      for (sec = input_bfd->sections; sec != NULL; sec = sec->next)
	{
	  if (sec->output_section == info->create_object_symbols_section)
	    {
	      asymbol *newsym;

	      newsym = bfd_make_empty_symbol (input_bfd);
	      if (!newsym)
		return false;
	      newsym->name = bfd_get_filename (input_bfd);
	      newsym->value = 0;
	      newsym->flags = BSF_LOCAL | BSF_FILE;
	      newsym->section = sec;

	      if (! generic_add_output_symbol (output_bfd, psymalloc,
					       newsym))
		return false;

	      break;
	    }
	}
    }

  /* Adjust the values of the globally visible symbols, and write out
     local symbols.  */
  sym_ptr = _bfd_generic_link_get_symbols (input_bfd);
  sym_end = sym_ptr + _bfd_generic_link_get_symcount (input_bfd);
  for (; sym_ptr < sym_end; sym_ptr++)
    {
      asymbol *sym;
      struct generic_link_hash_entry *h;
      bool output;

      h = NULL;
      sym = *sym_ptr;
      if ((sym->flags & (BSF_INDIRECT
			 | BSF_WARNING
			 | BSF_GLOBAL
			 | BSF_CONSTRUCTOR
			 | BSF_WEAK)) != 0
	  || bfd_is_und_section (bfd_asymbol_section (sym))
	  || bfd_is_com_section (bfd_asymbol_section (sym))
	  || bfd_is_ind_section (bfd_asymbol_section (sym)))
	{
	  if (sym->udata.p != NULL)
	    h = (struct generic_link_hash_entry *) sym->udata.p;
	  else if ((sym->flags & BSF_CONSTRUCTOR) != 0)
	    {
	      /* This case normally means that the main linker code
		 deliberately ignored this constructor symbol.  We
		 should just pass it through.  This will screw up if
		 the constructor symbol is from a different,
		 non-generic, object file format, but the case will
		 only arise when linking with -r, which will probably
		 fail anyhow, since there will be no way to represent
		 the relocs in the output format being used.  */
	      h = NULL;
	    }
	  else if (bfd_is_und_section (bfd_asymbol_section (sym)))
	    h = ((struct generic_link_hash_entry *)
		 bfd_wrapped_link_hash_lookup (output_bfd, info,
					       bfd_asymbol_name (sym),
					       false, false, true));
	  else
	    h = _bfd_generic_link_hash_lookup (_bfd_generic_hash_table (info),
					       bfd_asymbol_name (sym),
					       false, false, true);

	  if (h != NULL)
	    {
	      /* Force all references to this symbol to point to
		 the same area in memory.  It is possible that
		 this routine will be called with a hash table
		 other than a generic hash table, so we double
		 check that.  */
	      if (info->output_bfd->xvec == input_bfd->xvec)
		{
		  if (h->sym != NULL)
		    *sym_ptr = sym = h->sym;
		}

	      switch (h->root.type)
		{
		default:
		case bfd_link_hash_new:
		  abort ();
		case bfd_link_hash_undefined:
		  break;
		case bfd_link_hash_undefweak:
		  sym->flags |= BSF_WEAK;
		  break;
		case bfd_link_hash_indirect:
		  h = (struct generic_link_hash_entry *) h->root.u.i.link;
		  /* fall through */
		case bfd_link_hash_defined:
		  sym->flags |= BSF_GLOBAL;
		  sym->flags &=~ (BSF_WEAK | BSF_CONSTRUCTOR);
		  sym->value = h->root.u.def.value;
		  sym->section = h->root.u.def.section;
		  break;
		case bfd_link_hash_defweak:
		  sym->flags |= BSF_WEAK;
		  sym->flags &=~ BSF_CONSTRUCTOR;
		  sym->value = h->root.u.def.value;
		  sym->section = h->root.u.def.section;
		  break;
		case bfd_link_hash_common:
		  sym->value = h->root.u.c.size;
		  sym->flags |= BSF_GLOBAL;
		  if (! bfd_is_com_section (sym->section))
		    {
		      BFD_ASSERT (bfd_is_und_section (sym->section));
		      sym->section = bfd_com_section_ptr;
		    }
		  /* We do not set the section of the symbol to
		     h->root.u.c.p->section.  That value was saved so
		     that we would know where to allocate the symbol
		     if it was defined.  In this case the type is
		     still bfd_link_hash_common, so we did not define
		     it, so we do not want to use that section.  */
		  break;
		}
	    }
	}

      if ((sym->flags & BSF_KEEP) == 0
	  && (info->strip == strip_all
	      || (info->strip == strip_some
		  && bfd_hash_lookup (info->keep_hash, bfd_asymbol_name (sym),
				      false, false) == NULL)))
	output = false;
      else if ((sym->flags & (BSF_GLOBAL | BSF_WEAK | BSF_GNU_UNIQUE)) != 0)
	{
	  /* If this symbol is marked as occurring now, rather
	     than at the end, output it now.  This is used for
	     COFF C_EXT FCN symbols.  FIXME: There must be a
	     better way.  */
	  if (bfd_asymbol_bfd (sym) == input_bfd
	      && (sym->flags & BSF_NOT_AT_END) != 0)
	    output = true;
	  else
	    output = false;
	}
      else if ((sym->flags & BSF_KEEP) != 0)
	output = true;
      else if (bfd_is_ind_section (sym->section))
	output = false;
      else if ((sym->flags & BSF_DEBUGGING) != 0)
	{
	  if (info->strip == strip_none)
	    output = true;
	  else
	    output = false;
	}
      else if (bfd_is_und_section (sym->section)
	       || bfd_is_com_section (sym->section))
	output = false;
      else if ((sym->flags & BSF_LOCAL) != 0)
	{
	  if ((sym->flags & BSF_WARNING) != 0)
	    output = false;
	  else
	    {
	      switch (info->discard)
		{
		default:
		case discard_all:
		  output = false;
		  break;
		case discard_sec_merge:
		  output = true;
		  if (bfd_link_relocatable (info)
		      || ! (sym->section->flags & SEC_MERGE))
		    break;
		  /* FALLTHROUGH */
		case discard_l:
		  if (bfd_is_local_label (input_bfd, sym))
		    output = false;
		  else
		    output = true;
		  break;
		case discard_none:
		  output = true;
		  break;
		}
	    }
	}
      else if ((sym->flags & BSF_CONSTRUCTOR))
	{
	  if (info->strip != strip_all)
	    output = true;
	  else
	    output = false;
	}
      else if (sym->flags == 0
	       && (sym->section->owner->flags & BFD_PLUGIN) != 0)
	/* LTO doesn't set symbol information.  We get here with the
	   generic linker for a symbol that was "common" but no longer
	   needs to be global.  */
	output = false;
      else
	abort ();

      /* If this symbol is in a section which is not being included
	 in the output file, then we don't want to output the
	 symbol.  */
      if (!bfd_is_abs_section (sym->section)
	  && bfd_section_removed_from_list (output_bfd,
					    sym->section->output_section))
	output = false;

      if (output)
	{
	  if (! generic_add_output_symbol (output_bfd, psymalloc, sym))
	    return false;
	  if (h != NULL)
	    h->written = true;
	}
    }

  return true;
}

/* Set the section and value of a generic BFD symbol based on a linker
   hash table entry.  */

static void
set_symbol_from_hash (asymbol *sym, struct bfd_link_hash_entry *h)
{
  switch (h->type)
    {
    default:
      abort ();
      break;
    case bfd_link_hash_new:
      /* This can happen when a constructor symbol is seen but we are
	 not building constructors.  */
      if (sym->section != NULL)
	{
	  BFD_ASSERT ((sym->flags & BSF_CONSTRUCTOR) != 0);
	}
      else
	{
	  sym->flags |= BSF_CONSTRUCTOR;
	  sym->section = bfd_abs_section_ptr;
	  sym->value = 0;
	}
      break;
    case bfd_link_hash_undefined:
      sym->section = bfd_und_section_ptr;
      sym->value = 0;
      break;
    case bfd_link_hash_undefweak:
      sym->section = bfd_und_section_ptr;
      sym->value = 0;
      sym->flags |= BSF_WEAK;
      break;
    case bfd_link_hash_defined:
      sym->section = h->u.def.section;
      sym->value = h->u.def.value;
      break;
    case bfd_link_hash_defweak:
      sym->flags |= BSF_WEAK;
      sym->section = h->u.def.section;
      sym->value = h->u.def.value;
      break;
    case bfd_link_hash_common:
      sym->value = h->u.c.size;
      if (sym->section == NULL)
	sym->section = bfd_com_section_ptr;
      else if (! bfd_is_com_section (sym->section))
	{
	  BFD_ASSERT (bfd_is_und_section (sym->section));
	  sym->section = bfd_com_section_ptr;
	}
      /* Do not set the section; see _bfd_generic_link_output_symbols.  */
      break;
    case bfd_link_hash_indirect:
    case bfd_link_hash_warning:
      /* FIXME: What should we do here?  */
      break;
    }
}

/* Write out a global symbol, if it hasn't already been written out.
   This is called for each symbol in the hash table.  */

bool
_bfd_generic_link_write_global_symbol (struct generic_link_hash_entry *h,
				       void *data)
{
  struct generic_write_global_symbol_info *wginfo =
      (struct generic_write_global_symbol_info *) data;
  asymbol *sym;

  if (h->written)
    return true;

  h->written = true;

  if (wginfo->info->strip == strip_all
      || (wginfo->info->strip == strip_some
	  && bfd_hash_lookup (wginfo->info->keep_hash, h->root.root.string,
			      false, false) == NULL))
    return true;

  if (h->sym != NULL)
    sym = h->sym;
  else
    {
      sym = bfd_make_empty_symbol (wginfo->output_bfd);
      if (!sym)
	return false;
      sym->name = h->root.root.string;
      sym->flags = 0;
    }

  set_symbol_from_hash (sym, &h->root);

  sym->flags |= BSF_GLOBAL;

  if (! generic_add_output_symbol (wginfo->output_bfd, wginfo->psymalloc,
				   sym))
    {
      /* FIXME: No way to return failure.  */
      abort ();
    }

  return true;
}

/* Create a relocation.  */

bool
_bfd_generic_reloc_link_order (bfd *abfd,
			       struct bfd_link_info *info,
			       asection *sec,
			       struct bfd_link_order *link_order)
{
  arelent *r;

  if (! bfd_link_relocatable (info))
    abort ();
  if (sec->orelocation == NULL)
    abort ();

  r = (arelent *) bfd_alloc (abfd, sizeof (arelent));
  if (r == NULL)
    return false;

  r->address = link_order->offset;
  r->howto = bfd_reloc_type_lookup (abfd, link_order->u.reloc.p->reloc);
  if (r->howto == 0)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* Get the symbol to use for the relocation.  */
  if (link_order->type == bfd_section_reloc_link_order)
    r->sym_ptr_ptr = link_order->u.reloc.p->u.section->symbol_ptr_ptr;
  else
    {
      struct generic_link_hash_entry *h;

      h = ((struct generic_link_hash_entry *)
	   bfd_wrapped_link_hash_lookup (abfd, info,
					 link_order->u.reloc.p->u.name,
					 false, false, true));
      if (h == NULL
	  || ! h->written)
	{
	  (*info->callbacks->unattached_reloc)
	    (info, link_order->u.reloc.p->u.name, NULL, NULL, 0);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      r->sym_ptr_ptr = &h->sym;
    }

  /* If this is an inplace reloc, write the addend to the object file.
     Otherwise, store it in the reloc addend.  */
  if (! r->howto->partial_inplace)
    r->addend = link_order->u.reloc.p->addend;
  else
    {
      bfd_size_type size;
      bfd_reloc_status_type rstat;
      bfd_byte *buf;
      bool ok;
      file_ptr loc;

      size = bfd_get_reloc_size (r->howto);
      buf = (bfd_byte *) bfd_zmalloc (size);
      if (buf == NULL && size != 0)
	return false;
      rstat = _bfd_relocate_contents (r->howto, abfd,
				      (bfd_vma) link_order->u.reloc.p->addend,
				      buf);
      switch (rstat)
	{
	case bfd_reloc_ok:
	  break;
	default:
	case bfd_reloc_outofrange:
	  abort ();
	case bfd_reloc_overflow:
	  (*info->callbacks->reloc_overflow)
	    (info, NULL,
	     (link_order->type == bfd_section_reloc_link_order
	      ? bfd_section_name (link_order->u.reloc.p->u.section)
	      : link_order->u.reloc.p->u.name),
	     r->howto->name, link_order->u.reloc.p->addend,
	     NULL, NULL, 0);
	  break;
	}
      loc = link_order->offset * bfd_octets_per_byte (abfd, sec);
      ok = bfd_set_section_contents (abfd, sec, buf, loc, size);
      free (buf);
      if (! ok)
	return false;

      r->addend = 0;
    }

  sec->orelocation[sec->reloc_count] = r;
  ++sec->reloc_count;

  return true;
}

/* Allocate a new link_order for a section.  */

struct bfd_link_order *
bfd_new_link_order (bfd *abfd, asection *section)
{
  size_t amt = sizeof (struct bfd_link_order);
  struct bfd_link_order *new_lo;

  new_lo = (struct bfd_link_order *) bfd_zalloc (abfd, amt);
  if (!new_lo)
    return NULL;

  new_lo->type = bfd_undefined_link_order;

  if (section->map_tail.link_order != NULL)
    section->map_tail.link_order->next = new_lo;
  else
    section->map_head.link_order = new_lo;
  section->map_tail.link_order = new_lo;

  return new_lo;
}

/* Default link order processing routine.  Note that we can not handle
   the reloc_link_order types here, since they depend upon the details
   of how the particular backends generates relocs.  */

bool
_bfd_default_link_order (bfd *abfd,
			 struct bfd_link_info *info,
			 asection *sec,
			 struct bfd_link_order *link_order)
{
  switch (link_order->type)
    {
    case bfd_undefined_link_order:
    case bfd_section_reloc_link_order:
    case bfd_symbol_reloc_link_order:
    default:
      abort ();
    case bfd_indirect_link_order:
      return default_indirect_link_order (abfd, info, sec, link_order,
					  false);
    case bfd_data_link_order:
      return default_data_link_order (abfd, info, sec, link_order);
    }
}

/* Default routine to handle a bfd_data_link_order.  */

static bool
default_data_link_order (bfd *abfd,
			 struct bfd_link_info *info,
			 asection *sec,
			 struct bfd_link_order *link_order)
{
  bfd_size_type size;
  size_t fill_size;
  bfd_byte *fill;
  file_ptr loc;
  bool result;

  BFD_ASSERT ((sec->flags & SEC_HAS_CONTENTS) != 0);

  size = link_order->size;
  if (size == 0)
    return true;

  fill = link_order->u.data.contents;
  fill_size = link_order->u.data.size;
  if (fill_size == 0)
    {
      fill = abfd->arch_info->fill (size, info->big_endian,
				    (sec->flags & SEC_CODE) != 0);
      if (fill == NULL)
	return false;
    }
  else if (fill_size < size)
    {
      bfd_byte *p;
      fill = (bfd_byte *) bfd_malloc (size);
      if (fill == NULL)
	return false;
      p = fill;
      if (fill_size == 1)
	memset (p, (int) link_order->u.data.contents[0], (size_t) size);
      else
	{
	  do
	    {
	      memcpy (p, link_order->u.data.contents, fill_size);
	      p += fill_size;
	      size -= fill_size;
	    }
	  while (size >= fill_size);
	  if (size != 0)
	    memcpy (p, link_order->u.data.contents, (size_t) size);
	  size = link_order->size;
	}
    }

  loc = link_order->offset * bfd_octets_per_byte (abfd, sec);
  result = bfd_set_section_contents (abfd, sec, fill, loc, size);

  if (fill != link_order->u.data.contents)
    free (fill);
  return result;
}

/* Default routine to handle a bfd_indirect_link_order.  */

static bool
default_indirect_link_order (bfd *output_bfd,
			     struct bfd_link_info *info,
			     asection *output_section,
			     struct bfd_link_order *link_order,
			     bool generic_linker)
{
  asection *input_section;
  bfd *input_bfd;
  bfd_byte *alloced = NULL;
  bfd_byte *new_contents;
  file_ptr loc;

  BFD_ASSERT ((output_section->flags & SEC_HAS_CONTENTS) != 0);

  input_section = link_order->u.indirect.section;
  input_bfd = input_section->owner;
  if (input_section->size == 0)
    return true;

  BFD_ASSERT (input_section->output_section == output_section);
  BFD_ASSERT (input_section->output_offset == link_order->offset);
  BFD_ASSERT (input_section->size == link_order->size);

  if (bfd_link_relocatable (info)
      && input_section->reloc_count > 0
      && output_section->orelocation == NULL)
    {
      /* Space has not been allocated for the output relocations.
	 This can happen when we are called by a specific backend
	 because somebody is attempting to link together different
	 types of object files.  Handling this case correctly is
	 difficult, and sometimes impossible.  */
      _bfd_error_handler
	/* xgettext:c-format */
	(_("attempt to do relocatable link with %s input and %s output"),
	 bfd_get_target (input_bfd), bfd_get_target (output_bfd));
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  if (! generic_linker)
    {
      asymbol **sympp;
      asymbol **symppend;

      /* Get the canonical symbols.  The generic linker will always
	 have retrieved them by this point, but we are being called by
	 a specific linker, presumably because we are linking
	 different types of object files together.  */
      if (!bfd_generic_link_read_symbols (input_bfd))
	return false;

      /* Since we have been called by a specific linker, rather than
	 the generic linker, the values of the symbols will not be
	 right.  They will be the values as seen in the input file,
	 not the values of the final link.  We need to fix them up
	 before we can relocate the section.  */
      sympp = _bfd_generic_link_get_symbols (input_bfd);
      symppend = sympp + _bfd_generic_link_get_symcount (input_bfd);
      for (; sympp < symppend; sympp++)
	{
	  asymbol *sym;
	  struct bfd_link_hash_entry *h;

	  sym = *sympp;

	  if ((sym->flags & (BSF_INDIRECT
			     | BSF_WARNING
			     | BSF_GLOBAL
			     | BSF_CONSTRUCTOR
			     | BSF_WEAK)) != 0
	      || bfd_is_und_section (bfd_asymbol_section (sym))
	      || bfd_is_com_section (bfd_asymbol_section (sym))
	      || bfd_is_ind_section (bfd_asymbol_section (sym)))
	    {
	      /* sym->udata may have been set by
		 generic_link_add_symbol_list.  */
	      if (sym->udata.p != NULL)
		h = (struct bfd_link_hash_entry *) sym->udata.p;
	      else if (bfd_is_und_section (bfd_asymbol_section (sym)))
		h = bfd_wrapped_link_hash_lookup (output_bfd, info,
						  bfd_asymbol_name (sym),
						  false, false, true);
	      else
		h = bfd_link_hash_lookup (info->hash,
					  bfd_asymbol_name (sym),
					  false, false, true);
	      if (h != NULL)
		set_symbol_from_hash (sym, h);
	    }
	}
    }

  if ((output_section->flags & (SEC_GROUP | SEC_LINKER_CREATED)) == SEC_GROUP
      && input_section->size != 0)
    {
      /* Group section contents are set by bfd_elf_set_group_contents.  */
      if (!output_bfd->output_has_begun)
	{
	  /* FIXME: This hack ensures bfd_elf_set_group_contents is called.  */
	  if (!bfd_set_section_contents (output_bfd, output_section, "", 0, 1))
	    goto error_return;
	}
      new_contents = output_section->contents;
      BFD_ASSERT (new_contents != NULL);
      BFD_ASSERT (input_section->output_offset == 0);
    }
  else
    {
      /* Get and relocate the section contents.  */
      new_contents = (bfd_get_relocated_section_contents
		      (output_bfd, info, link_order, NULL,
		       bfd_link_relocatable (info),
		       _bfd_generic_link_get_symbols (input_bfd)));
      alloced = new_contents;
      if (!new_contents)
	goto error_return;
    }

  /* Output the section contents.  */
  loc = (input_section->output_offset
	 * bfd_octets_per_byte (output_bfd, output_section));
  if (! bfd_set_section_contents (output_bfd, output_section,
				  new_contents, loc, input_section->size))
    goto error_return;

  free (alloced);
  return true;

 error_return:
  free (alloced);
  return false;
}

/* A little routine to count the number of relocs in a link_order
   list.  */

unsigned int
_bfd_count_link_order_relocs (struct bfd_link_order *link_order)
{
  register unsigned int c;
  register struct bfd_link_order *l;

  c = 0;
  for (l = link_order; l != NULL; l = l->next)
    {
      if (l->type == bfd_section_reloc_link_order
	  || l->type == bfd_symbol_reloc_link_order)
	++c;
    }

  return c;
}

/*
FUNCTION
	bfd_link_split_section

SYNOPSIS
	bool bfd_link_split_section (bfd *abfd, asection *sec);

DESCRIPTION
	Return nonzero if @var{sec} should be split during a
	reloceatable or final link.

.#define bfd_link_split_section(abfd, sec) \
.	BFD_SEND (abfd, _bfd_link_split_section, (abfd, sec))
.

*/

bool
_bfd_generic_link_split_section (bfd *abfd ATTRIBUTE_UNUSED,
				 asection *sec ATTRIBUTE_UNUSED)
{
  return false;
}

/*
FUNCTION
	bfd_section_already_linked

SYNOPSIS
	bool bfd_section_already_linked (bfd *abfd,
					 asection *sec,
					 struct bfd_link_info *info);

DESCRIPTION
	Check if @var{data} has been already linked during a reloceatable
	or final link.  Return TRUE if it has.

.#define bfd_section_already_linked(abfd, sec, info) \
.	BFD_SEND (abfd, _section_already_linked, (abfd, sec, info))
.

*/

/* Sections marked with the SEC_LINK_ONCE flag should only be linked
   once into the output.  This routine checks each section, and
   arrange to discard it if a section of the same name has already
   been linked.  This code assumes that all relevant sections have the
   SEC_LINK_ONCE flag set; that is, it does not depend solely upon the
   section name.  bfd_section_already_linked is called via
   bfd_map_over_sections.  */

/* The hash table.  */

static struct bfd_hash_table _bfd_section_already_linked_table;

/* Support routines for the hash table used by section_already_linked,
   initialize the table, traverse, lookup, fill in an entry and remove
   the table.  */

void
bfd_section_already_linked_table_traverse
  (bool (*func) (struct bfd_section_already_linked_hash_entry *, void *),
   void *info)
{
  bfd_hash_traverse (&_bfd_section_already_linked_table,
		     (bool (*) (struct bfd_hash_entry *, void *)) func,
		     info);
}

struct bfd_section_already_linked_hash_entry *
bfd_section_already_linked_table_lookup (const char *name)
{
  return ((struct bfd_section_already_linked_hash_entry *)
	  bfd_hash_lookup (&_bfd_section_already_linked_table, name,
			   true, false));
}

bool
bfd_section_already_linked_table_insert
  (struct bfd_section_already_linked_hash_entry *already_linked_list,
   asection *sec)
{
  struct bfd_section_already_linked *l;

  /* Allocate the memory from the same obstack as the hash table is
     kept in.  */
  l = (struct bfd_section_already_linked *)
      bfd_hash_allocate (&_bfd_section_already_linked_table, sizeof *l);
  if (l == NULL)
    return false;
  l->sec = sec;
  l->next = already_linked_list->entry;
  already_linked_list->entry = l;
  return true;
}

static struct bfd_hash_entry *
already_linked_newfunc (struct bfd_hash_entry *entry ATTRIBUTE_UNUSED,
			struct bfd_hash_table *table,
			const char *string ATTRIBUTE_UNUSED)
{
  struct bfd_section_already_linked_hash_entry *ret =
    (struct bfd_section_already_linked_hash_entry *)
      bfd_hash_allocate (table, sizeof *ret);

  if (ret == NULL)
    return NULL;

  ret->entry = NULL;

  return &ret->root;
}

bool
bfd_section_already_linked_table_init (void)
{
  return bfd_hash_table_init_n (&_bfd_section_already_linked_table,
				already_linked_newfunc,
				sizeof (struct bfd_section_already_linked_hash_entry),
				42);
}

void
bfd_section_already_linked_table_free (void)
{
  bfd_hash_table_free (&_bfd_section_already_linked_table);
}

/* Report warnings as appropriate for duplicate section SEC.
   Return FALSE if we decide to keep SEC after all.  */

bool
_bfd_handle_already_linked (asection *sec,
			    struct bfd_section_already_linked *l,
			    struct bfd_link_info *info)
{
  switch (sec->flags & SEC_LINK_DUPLICATES)
    {
    default:
      abort ();

    case SEC_LINK_DUPLICATES_DISCARD:
      /* If we found an LTO IR match for this comdat group on
	 the first pass, replace it with the LTO output on the
	 second pass.  We can't simply choose real object
	 files over IR because the first pass may contain a
	 mix of LTO and normal objects and we must keep the
	 first match, be it IR or real.  */
      if (sec->owner->lto_output
	  && (l->sec->owner->flags & BFD_PLUGIN) != 0)
	{
	  l->sec = sec;
	  return false;
	}
      break;

    case SEC_LINK_DUPLICATES_ONE_ONLY:
      info->callbacks->einfo
	/* xgettext:c-format */
	(_("%pB: ignoring duplicate section `%pA'\n"),
	 sec->owner, sec);
      break;

    case SEC_LINK_DUPLICATES_SAME_SIZE:
      if ((l->sec->owner->flags & BFD_PLUGIN) != 0)
	;
      else if (sec->size != l->sec->size)
	info->callbacks->einfo
	  /* xgettext:c-format */
	  (_("%pB: duplicate section `%pA' has different size\n"),
	   sec->owner, sec);
      break;

    case SEC_LINK_DUPLICATES_SAME_CONTENTS:
      if ((l->sec->owner->flags & BFD_PLUGIN) != 0)
	;
      else if (sec->size != l->sec->size)
	info->callbacks->einfo
	  /* xgettext:c-format */
	  (_("%pB: duplicate section `%pA' has different size\n"),
	   sec->owner, sec);
      else if (sec->size != 0)
	{
	  bfd_byte *sec_contents, *l_sec_contents;

	  if ((sec->flags & SEC_HAS_CONTENTS) == 0
	      && (l->sec->flags & SEC_HAS_CONTENTS) == 0)
	    ;
	  else if ((sec->flags & SEC_HAS_CONTENTS) == 0
		   || !bfd_malloc_and_get_section (sec->owner, sec,
						   &sec_contents))
	    info->callbacks->einfo
	      /* xgettext:c-format */
	      (_("%pB: could not read contents of section `%pA'\n"),
	       sec->owner, sec);
	  else if ((l->sec->flags & SEC_HAS_CONTENTS) == 0
		   || !bfd_malloc_and_get_section (l->sec->owner, l->sec,
						   &l_sec_contents))
	    {
	      info->callbacks->einfo
		/* xgettext:c-format */
		(_("%pB: could not read contents of section `%pA'\n"),
		 l->sec->owner, l->sec);
	      free (sec_contents);
	    }
	  else
	    {
	      if (memcmp (sec_contents, l_sec_contents, sec->size) != 0)
		info->callbacks->einfo
		  /* xgettext:c-format */
		  (_("%pB: duplicate section `%pA' has different contents\n"),
		   sec->owner, sec);
	      free (l_sec_contents);
	      free (sec_contents);
	    }
	}
      break;
    }

  /* Set the output_section field so that lang_add_section
     does not create a lang_input_section structure for this
     section.  Since there might be a symbol in the section
     being discarded, we must retain a pointer to the section
     which we are really going to use.  */
  sec->output_section = bfd_abs_section_ptr;
  sec->kept_section = l->sec;
  return true;
}

/* This is used on non-ELF inputs.  */

bool
_bfd_generic_section_already_linked (bfd *abfd ATTRIBUTE_UNUSED,
				     asection *sec,
				     struct bfd_link_info *info)
{
  const char *name;
  struct bfd_section_already_linked *l;
  struct bfd_section_already_linked_hash_entry *already_linked_list;

  if ((sec->flags & SEC_LINK_ONCE) == 0)
    return false;

  /* The generic linker doesn't handle section groups.  */
  if ((sec->flags & SEC_GROUP) != 0)
    return false;

  /* FIXME: When doing a relocatable link, we may have trouble
     copying relocations in other sections that refer to local symbols
     in the section being discarded.  Those relocations will have to
     be converted somehow; as of this writing I'm not sure that any of
     the backends handle that correctly.

     It is tempting to instead not discard link once sections when
     doing a relocatable link (technically, they should be discarded
     whenever we are building constructors).  However, that fails,
     because the linker winds up combining all the link once sections
     into a single large link once section, which defeats the purpose
     of having link once sections in the first place.  */

  name = bfd_section_name (sec);

  already_linked_list = bfd_section_already_linked_table_lookup (name);

  l = already_linked_list->entry;
  if (l != NULL)
    {
      /* The section has already been linked.  See if we should
	 issue a warning.  */
      return _bfd_handle_already_linked (sec, l, info);
    }

  /* This is the first section with this name.  Record it.  */
  if (!bfd_section_already_linked_table_insert (already_linked_list, sec))
    info->callbacks->einfo (_("%F%P: already_linked_table: %E\n"));
  return false;
}

/* Choose a neighbouring section to S in OBFD that will be output, or
   the absolute section if ADDR is out of bounds of the neighbours.  */

asection *
_bfd_nearby_section (bfd *obfd, asection *s, bfd_vma addr)
{
  asection *next, *prev, *best;

  /* Find preceding kept section.  */
  for (prev = s->prev; prev != NULL; prev = prev->prev)
    if ((prev->flags & SEC_EXCLUDE) == 0
	&& !bfd_section_removed_from_list (obfd, prev))
      break;

  /* Find following kept section.  Start at prev->next because
     other sections may have been added after S was removed.  */
  if (s->prev != NULL)
    next = s->prev->next;
  else
    next = s->owner->sections;
  for (; next != NULL; next = next->next)
    if ((next->flags & SEC_EXCLUDE) == 0
	&& !bfd_section_removed_from_list (obfd, next))
      break;

  /* Choose better of two sections, based on flags.  The idea
     is to choose a section that will be in the same segment
     as S would have been if it was kept.  */
  best = next;
  if (prev == NULL)
    {
      if (next == NULL)
	best = bfd_abs_section_ptr;
    }
  else if (next == NULL)
    best = prev;
  else if (((prev->flags ^ next->flags)
	    & (SEC_ALLOC | SEC_THREAD_LOCAL | SEC_LOAD)) != 0)
    {
      if (((next->flags ^ s->flags)
	   & (SEC_ALLOC | SEC_THREAD_LOCAL)) != 0
	  /* We prefer to choose a loaded section.  Section S
	     doesn't have SEC_LOAD set (it being excluded, that
	     part of the flag processing didn't happen) so we
	     can't compare that flag to those of NEXT and PREV.  */
	  || ((prev->flags & SEC_LOAD) != 0
	      && (next->flags & SEC_LOAD) == 0))
	best = prev;
    }
  else if (((prev->flags ^ next->flags) & SEC_READONLY) != 0)
    {
      if (((next->flags ^ s->flags) & SEC_READONLY) != 0)
	best = prev;
    }
  else if (((prev->flags ^ next->flags) & SEC_CODE) != 0)
    {
      if (((next->flags ^ s->flags) & SEC_CODE) != 0)
	best = prev;
    }
  else
    {
      /* Flags we care about are the same.  Prefer the following
	 section if that will result in a positive valued sym.  */
      if (addr < next->vma)
	best = prev;
    }

  return best;
}

/* Convert symbols in excluded output sections to use a kept section.  */

static bool
fix_syms (struct bfd_link_hash_entry *h, void *data)
{
  bfd *obfd = (bfd *) data;

  if (h->type == bfd_link_hash_defined
      || h->type == bfd_link_hash_defweak)
    {
      asection *s = h->u.def.section;
      if (s != NULL
	  && s->output_section != NULL
	  && (s->output_section->flags & SEC_EXCLUDE) != 0
	  && bfd_section_removed_from_list (obfd, s->output_section))
	{
	  asection *op;

	  h->u.def.value += s->output_offset + s->output_section->vma;
	  op = _bfd_nearby_section (obfd, s->output_section, h->u.def.value);
	  h->u.def.value -= op->vma;
	  h->u.def.section = op;
	}
    }

  return true;
}

void
_bfd_fix_excluded_sec_syms (bfd *obfd, struct bfd_link_info *info)
{
  bfd_link_hash_traverse (info->hash, fix_syms, obfd);
}

/*
FUNCTION
	bfd_generic_define_common_symbol

SYNOPSIS
	bool bfd_generic_define_common_symbol
	  (bfd *output_bfd, struct bfd_link_info *info,
	   struct bfd_link_hash_entry *h);

DESCRIPTION
	Convert common symbol @var{h} into a defined symbol.
	Return TRUE on success and FALSE on failure.

.#define bfd_define_common_symbol(output_bfd, info, h) \
.	BFD_SEND (output_bfd, _bfd_define_common_symbol, (output_bfd, info, h))
.
*/

bool
bfd_generic_define_common_symbol (bfd *output_bfd,
				  struct bfd_link_info *info ATTRIBUTE_UNUSED,
				  struct bfd_link_hash_entry *h)
{
  unsigned int power_of_two;
  bfd_vma alignment, size;
  asection *section;

  BFD_ASSERT (h != NULL && h->type == bfd_link_hash_common);

  size = h->u.c.size;
  power_of_two = h->u.c.p->alignment_power;
  section = h->u.c.p->section;

  /* Increase the size of the section to align the common symbol.
     The alignment must be a power of two.  But if the section does
     not have any alignment requirement then do not increase the
     alignment unnecessarily.  */
  if (power_of_two)
    alignment = bfd_octets_per_byte (output_bfd, section) << power_of_two;
  else
    alignment = 1;
  BFD_ASSERT (alignment != 0 && (alignment & -alignment) == alignment);
  section->size += alignment - 1;
  section->size &= -alignment;

  /* Adjust the section's overall alignment if necessary.  */
  if (power_of_two > section->alignment_power)
    section->alignment_power = power_of_two;

  /* Change the symbol from common to defined.  */
  h->type = bfd_link_hash_defined;
  h->u.def.section = section;
  h->u.def.value = section->size;

  /* Increase the size of the section.  */
  section->size += size;

  /* Make sure the section is allocated in memory, and make sure that
     it is no longer a common section.  */
  section->flags |= SEC_ALLOC;
  section->flags &= ~(SEC_IS_COMMON | SEC_HAS_CONTENTS);
  return true;
}

/*
FUNCTION
	_bfd_generic_link_hide_symbol

SYNOPSIS
	void _bfd_generic_link_hide_symbol
	  (bfd *output_bfd, struct bfd_link_info *info,
	   struct bfd_link_hash_entry *h);

DESCRIPTION
	Hide symbol @var{h}.
	This is an internal function.  It should not be called from
	outside the BFD library.

.#define bfd_link_hide_symbol(output_bfd, info, h) \
.	BFD_SEND (output_bfd, _bfd_link_hide_symbol, (output_bfd, info, h))
.
*/

void
_bfd_generic_link_hide_symbol (bfd *output_bfd ATTRIBUTE_UNUSED,
			       struct bfd_link_info *info ATTRIBUTE_UNUSED,
			       struct bfd_link_hash_entry *h ATTRIBUTE_UNUSED)
{
}

/*
FUNCTION
	bfd_generic_define_start_stop

SYNOPSIS
	struct bfd_link_hash_entry *bfd_generic_define_start_stop
	  (struct bfd_link_info *info,
	   const char *symbol, asection *sec);

DESCRIPTION
	Define a __start, __stop, .startof. or .sizeof. symbol.
	Return the symbol or NULL if no such undefined symbol exists.

.#define bfd_define_start_stop(output_bfd, info, symbol, sec) \
.	BFD_SEND (output_bfd, _bfd_define_start_stop, (info, symbol, sec))
.
*/

struct bfd_link_hash_entry *
bfd_generic_define_start_stop (struct bfd_link_info *info,
			       const char *symbol, asection *sec)
{
  struct bfd_link_hash_entry *h;

  h = bfd_link_hash_lookup (info->hash, symbol, false, false, true);
  if (h != NULL
      && !h->ldscript_def
      && (h->type == bfd_link_hash_undefined
	  || h->type == bfd_link_hash_undefweak))
    {
      h->type = bfd_link_hash_defined;
      h->u.def.section = sec;
      h->u.def.value = 0;
      return h;
    }
  return NULL;
}

/*
FUNCTION
	bfd_find_version_for_sym

SYNOPSIS
	struct bfd_elf_version_tree * bfd_find_version_for_sym
	  (struct bfd_elf_version_tree *verdefs,
	   const char *sym_name, bool *hide);

DESCRIPTION
	Search an elf version script tree for symbol versioning
	info and export / don't-export status for a given symbol.
	Return non-NULL on success and NULL on failure; also sets
	the output @samp{hide} boolean parameter.

*/

struct bfd_elf_version_tree *
bfd_find_version_for_sym (struct bfd_elf_version_tree *verdefs,
			  const char *sym_name,
			  bool *hide)
{
  struct bfd_elf_version_tree *t;
  struct bfd_elf_version_tree *local_ver, *global_ver, *exist_ver;
  struct bfd_elf_version_tree *star_local_ver, *star_global_ver;

  local_ver = NULL;
  global_ver = NULL;
  star_local_ver = NULL;
  star_global_ver = NULL;
  exist_ver = NULL;
  for (t = verdefs; t != NULL; t = t->next)
    {
      if (t->globals.list != NULL)
	{
	  struct bfd_elf_version_expr *d = NULL;

	  while ((d = (*t->match) (&t->globals, d, sym_name)) != NULL)
	    {
	      if (d->literal || strcmp (d->pattern, "*") != 0)
		global_ver = t;
	      else
		star_global_ver = t;
	      if (d->symver)
		exist_ver = t;
	      d->script = 1;
	      /* If the match is a wildcard pattern, keep looking for
		 a more explicit, perhaps even local, match.  */
	      if (d->literal)
		break;
	    }

	  if (d != NULL)
	    break;
	}

      if (t->locals.list != NULL)
	{
	  struct bfd_elf_version_expr *d = NULL;

	  while ((d = (*t->match) (&t->locals, d, sym_name)) != NULL)
	    {
	      if (d->literal || strcmp (d->pattern, "*") != 0)
		local_ver = t;
	      else
		star_local_ver = t;
	      /* If the match is a wildcard pattern, keep looking for
		 a more explicit, perhaps even global, match.  */
	      if (d->literal)
		{
		  /* An exact match overrides a global wildcard.  */
		  global_ver = NULL;
		  star_global_ver = NULL;
		  break;
		}
	    }

	  if (d != NULL)
	    break;
	}
    }

  if (global_ver == NULL && local_ver == NULL)
    global_ver = star_global_ver;

  if (global_ver != NULL)
    {
      /* If we already have a versioned symbol that matches the
	 node for this symbol, then we don't want to create a
	 duplicate from the unversioned symbol.  Instead hide the
	 unversioned symbol.  */
      *hide = exist_ver == global_ver;
      return global_ver;
    }

  if (local_ver == NULL)
    local_ver = star_local_ver;

  if (local_ver != NULL)
    {
      *hide = true;
      return local_ver;
    }

  return NULL;
}

/*
FUNCTION
	bfd_hide_sym_by_version

SYNOPSIS
	bool bfd_hide_sym_by_version
	  (struct bfd_elf_version_tree *verdefs, const char *sym_name);

DESCRIPTION
	Search an elf version script tree for symbol versioning
	info for a given symbol.  Return TRUE if the symbol is hidden.

*/

bool
bfd_hide_sym_by_version (struct bfd_elf_version_tree *verdefs,
			 const char *sym_name)
{
  bool hidden = false;
  bfd_find_version_for_sym (verdefs, sym_name, &hidden);
  return hidden;
}

/*
FUNCTION
	bfd_link_check_relocs

SYNOPSIS
	bool bfd_link_check_relocs
	  (bfd *abfd, struct bfd_link_info *info);

DESCRIPTION
	Checks the relocs in ABFD for validity.
	Does not execute the relocs.
	Return TRUE if everything is OK, FALSE otherwise.
	This is the external entry point to this code.
*/

bool
bfd_link_check_relocs (bfd *abfd, struct bfd_link_info *info)
{
  return BFD_SEND (abfd, _bfd_link_check_relocs, (abfd, info));
}

/*
FUNCTION
	_bfd_generic_link_check_relocs

SYNOPSIS
	bool _bfd_generic_link_check_relocs
	  (bfd *abfd, struct bfd_link_info *info);

DESCRIPTION
	Stub function for targets that do not implement reloc checking.
	Return TRUE.
	This is an internal function.  It should not be called from
	outside the BFD library.
*/

bool
_bfd_generic_link_check_relocs (bfd *abfd ATTRIBUTE_UNUSED,
				struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return true;
}

/*
FUNCTION
	bfd_merge_private_bfd_data

SYNOPSIS
	bool bfd_merge_private_bfd_data
	  (bfd *ibfd, struct bfd_link_info *info);

DESCRIPTION
	Merge private BFD information from the BFD @var{ibfd} to the
	the output file BFD when linking.  Return <<TRUE>> on success,
	<<FALSE>> on error.  Possible error returns are:

	o <<bfd_error_no_memory>> -
	Not enough memory exists to create private data for @var{obfd}.

.#define bfd_merge_private_bfd_data(ibfd, info) \
.	BFD_SEND ((info)->output_bfd, _bfd_merge_private_bfd_data, \
.		  (ibfd, info))
.
*/

/*
INTERNAL_FUNCTION
	_bfd_generic_verify_endian_match

SYNOPSIS
	bool _bfd_generic_verify_endian_match
	  (bfd *ibfd, struct bfd_link_info *info);

DESCRIPTION
	Can be used from / for bfd_merge_private_bfd_data to check that
	endianness matches between input and output file.  Returns
	TRUE for a match, otherwise returns FALSE and emits an error.
*/

bool
_bfd_generic_verify_endian_match (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;

  if (ibfd->xvec->byteorder != obfd->xvec->byteorder
      && ibfd->xvec->byteorder != BFD_ENDIAN_UNKNOWN
      && obfd->xvec->byteorder != BFD_ENDIAN_UNKNOWN)
    {
      if (bfd_big_endian (ibfd))
	_bfd_error_handler (_("%pB: compiled for a big endian system "
			      "and target is little endian"), ibfd);
      else
	_bfd_error_handler (_("%pB: compiled for a little endian system "
			      "and target is big endian"), ibfd);
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  return true;
}

int
_bfd_nolink_sizeof_headers (bfd *abfd ATTRIBUTE_UNUSED,
			    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return 0;
}

bool
_bfd_nolink_bfd_relax_section (bfd *abfd,
			       asection *section ATTRIBUTE_UNUSED,
			       struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			       bool *again ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

bfd_byte *
_bfd_nolink_bfd_get_relocated_section_contents
    (bfd *abfd,
     struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
     struct bfd_link_order *link_order ATTRIBUTE_UNUSED,
     bfd_byte *data ATTRIBUTE_UNUSED,
     bool relocatable ATTRIBUTE_UNUSED,
     asymbol **symbols ATTRIBUTE_UNUSED)
{
  return (bfd_byte *) _bfd_ptr_bfd_null_error (abfd);
}

bool
_bfd_nolink_bfd_lookup_section_flags
    (struct bfd_link_info *info ATTRIBUTE_UNUSED,
     struct flag_info *flaginfo ATTRIBUTE_UNUSED,
     asection *section)
{
  return _bfd_bool_bfd_false_error (section->owner);
}

bool
_bfd_nolink_bfd_is_group_section (bfd *abfd,
				  const asection *sec ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

const char *
_bfd_nolink_bfd_group_name (bfd *abfd,
			    const asection *sec ATTRIBUTE_UNUSED)
{
  return _bfd_ptr_bfd_null_error (abfd);
}

bool
_bfd_nolink_bfd_discard_group (bfd *abfd, asection *sec ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

struct bfd_link_hash_table *
_bfd_nolink_bfd_link_hash_table_create (bfd *abfd)
{
  return (struct bfd_link_hash_table *) _bfd_ptr_bfd_null_error (abfd);
}

void
_bfd_nolink_bfd_link_just_syms (asection *sec ATTRIBUTE_UNUSED,
				struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
}

void
_bfd_nolink_bfd_copy_link_hash_symbol_type
    (bfd *abfd ATTRIBUTE_UNUSED,
     struct bfd_link_hash_entry *from ATTRIBUTE_UNUSED,
     struct bfd_link_hash_entry *to ATTRIBUTE_UNUSED)
{
}

bool
_bfd_nolink_bfd_link_split_section (bfd *abfd, asection *sec ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

bool
_bfd_nolink_section_already_linked (bfd *abfd,
				    asection *sec ATTRIBUTE_UNUSED,
				    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

bool
_bfd_nolink_bfd_define_common_symbol
    (bfd *abfd,
     struct bfd_link_info *info ATTRIBUTE_UNUSED,
     struct bfd_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

struct bfd_link_hash_entry *
_bfd_nolink_bfd_define_start_stop (struct bfd_link_info *info ATTRIBUTE_UNUSED,
				   const char *name ATTRIBUTE_UNUSED,
				   asection *sec)
{
  return (struct bfd_link_hash_entry *) _bfd_ptr_bfd_null_error (sec->owner);
}

/* Return false if linker should avoid caching relocation infomation
   and symbol tables of input files in memory.  */

bool
_bfd_link_keep_memory (struct bfd_link_info * info)
{
  bfd *abfd;
  bfd_size_type size;

  if (!info->keep_memory)
    return false;

  if (info->max_cache_size == (bfd_size_type) -1)
    return true;

  abfd = info->input_bfds;
  size = info->cache_size;
  do
    {
      if (size >= info->max_cache_size)
	{
	  /* Over the limit.  Reduce the memory usage.  */
	  info->keep_memory = false;
	  return false;
	}
      if (!abfd)
	break;
      size += abfd->alloc_size;
      abfd = abfd->link.next;
    }
  while (1);

  return true;
}
