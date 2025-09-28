/* CTF dict creation.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#include <ctf-impl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>

#include <elf.h>
#include "elf-bfd.h"

/* Symtypetab sections.  */

/* Symtypetab emission flags.  */

#define CTF_SYMTYPETAB_EMIT_FUNCTION 0x1
#define CTF_SYMTYPETAB_EMIT_PAD 0x2
#define CTF_SYMTYPETAB_FORCE_INDEXED 0x4

/* Properties of symtypetab emission, shared by symtypetab section
   sizing and symtypetab emission itself.  */

typedef struct emit_symtypetab_state
{
  /* True if linker-reported symbols are being filtered out.  symfp is set if
     this is true: otherwise, indexing is forced and the symflags indicate as
     much. */
  int filter_syms;

  /* True if symbols are being sorted.  */
  int sort_syms;

  /* Flags for symtypetab emission.  */
  int symflags;

  /* The dict to which the linker has reported symbols.  */
  ctf_dict_t *symfp;

  /* The maximum number of objects seen.  */
  size_t maxobjt;

  /* The maximum number of func info entris seen.  */
  size_t maxfunc;
} emit_symtypetab_state_t;

/* Determine if a symbol is "skippable" and should never appear in the
   symtypetab sections.  */

int
ctf_symtab_skippable (ctf_link_sym_t *sym)
{
  /* Never skip symbols whose name is not yet known.  */
  if (sym->st_nameidx_set)
    return 0;

  return (sym->st_name == NULL || sym->st_name[0] == 0
	  || sym->st_shndx == SHN_UNDEF
	  || strcmp (sym->st_name, "_START_") == 0
	  || strcmp (sym->st_name, "_END_") == 0
	  || (sym->st_type == STT_OBJECT && sym->st_shndx == SHN_EXTABS
	      && sym->st_value == 0));
}

/* Get the number of symbols in a symbol hash, the count of symbols, the maximum
   seen, the eventual size, without any padding elements, of the func/data and
   (if generated) index sections, and the size of accumulated padding elements.
   The linker-reported set of symbols is found in SYMFP: it may be NULL if
   symbol filtering is not desired, in which case CTF_SYMTYPETAB_FORCE_INDEXED
   will always be set in the flags.

   Also figure out if any symbols need to be moved to the variable section, and
   add them (if not already present).  */

_libctf_nonnull_ ((1,3,4,5,6,7,8))
static int
symtypetab_density (ctf_dict_t *fp, ctf_dict_t *symfp, ctf_dynhash_t *symhash,
		    size_t *count, size_t *max, size_t *unpadsize,
		    size_t *padsize, size_t *idxsize, int flags)
{
  ctf_next_t *i = NULL;
  const void *name;
  const void *ctf_sym;
  ctf_dynhash_t *linker_known = NULL;
  int err;
  int beyond_max = 0;

  *count = 0;
  *max = 0;
  *unpadsize = 0;
  *idxsize = 0;
  *padsize = 0;

  if (!(flags & CTF_SYMTYPETAB_FORCE_INDEXED))
    {
      /* Make a dynhash citing only symbols reported by the linker of the
	 appropriate type, then traverse all potential-symbols we know the types
	 of, removing them from linker_known as we go.  Once this is done, the
	 only symbols remaining in linker_known are symbols we don't know the
	 types of: we must emit pads for those symbols that are below the
	 maximum symbol we will emit (any beyond that are simply skipped).

	 If there are none, this symtypetab will be empty: just report that.  */

      if (!symfp->ctf_dynsyms)
	return 0;

      if ((linker_known = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
					      NULL, NULL)) == NULL)
	return (ctf_set_errno (fp, ENOMEM));

      while ((err = ctf_dynhash_cnext (symfp->ctf_dynsyms, &i,
				       &name, &ctf_sym)) == 0)
	{
	  ctf_link_sym_t *sym = (ctf_link_sym_t *) ctf_sym;

	  if (((flags & CTF_SYMTYPETAB_EMIT_FUNCTION)
	       && sym->st_type != STT_FUNC)
	      || (!(flags & CTF_SYMTYPETAB_EMIT_FUNCTION)
		  && sym->st_type != STT_OBJECT))
	    continue;

	  if (ctf_symtab_skippable (sym))
	    continue;

	  /* This should only be true briefly before all the names are
	     finalized, long before we get this far.  */
	  if (!ctf_assert (fp, !sym->st_nameidx_set))
	    return -1;				/* errno is set for us.  */

	  if (ctf_dynhash_cinsert (linker_known, name, ctf_sym) < 0)
	    {
	      ctf_dynhash_destroy (linker_known);
	      return (ctf_set_errno (fp, ENOMEM));
	    }
	}
      if (err != ECTF_NEXT_END)
	{
	  ctf_err_warn (fp, 0, err, _("iterating over linker-known symbols during "
				  "serialization"));
	  ctf_dynhash_destroy (linker_known);
	  return (ctf_set_errno (fp, err));
	}
    }

  while ((err = ctf_dynhash_cnext (symhash, &i, &name, NULL)) == 0)
    {
      ctf_link_sym_t *sym;

      if (!(flags & CTF_SYMTYPETAB_FORCE_INDEXED))
	{
	  /* Linker did not report symbol in symtab.  Remove it from the
	     set of known data symbols and continue.  */
	  if ((sym = ctf_dynhash_lookup (symfp->ctf_dynsyms, name)) == NULL)
	    {
	      ctf_dynhash_remove (symhash, name);
	      continue;
	    }

	  /* We don't remove skippable symbols from the symhash because we don't
	     want them to be migrated into variables.  */
	  if (ctf_symtab_skippable (sym))
	    continue;

	  if ((flags & CTF_SYMTYPETAB_EMIT_FUNCTION)
	      && sym->st_type != STT_FUNC)
	    {
	      ctf_err_warn (fp, 1, 0, _("symbol %s (%x) added to CTF as a "
					"function but is of type %x.  "
					"The symbol type lookup tables "
					"are probably corrupted"),
			    sym->st_name, sym->st_symidx, sym->st_type);
	      ctf_dynhash_remove (symhash, name);
	      continue;
	    }
	  else if (!(flags & CTF_SYMTYPETAB_EMIT_FUNCTION)
		   && sym->st_type != STT_OBJECT)
	    {
	      ctf_err_warn (fp, 1, 0, _("symbol %s (%x) added to CTF as a "
					"data object but is of type %x.  "
					"The symbol type lookup tables "
					"are probably corrupted"),
			    sym->st_name, sym->st_symidx, sym->st_type);
	      ctf_dynhash_remove (symhash, name);
	      continue;
	    }

	  ctf_dynhash_remove (linker_known, name);
	}
      *unpadsize += sizeof (uint32_t);
      (*count)++;

      if (!(flags & CTF_SYMTYPETAB_FORCE_INDEXED))
	{
	  if (*max < sym->st_symidx)
	    *max = sym->st_symidx;
	}
      else
	(*max)++;
    }
  if (err != ECTF_NEXT_END)
    {
      ctf_err_warn (fp, 0, err, _("iterating over CTF symtypetab during "
				  "serialization"));
      ctf_dynhash_destroy (linker_known);
      return (ctf_set_errno (fp, err));
    }

  if (!(flags & CTF_SYMTYPETAB_FORCE_INDEXED))
    {
      while ((err = ctf_dynhash_cnext (linker_known, &i, NULL, &ctf_sym)) == 0)
	{
	  ctf_link_sym_t *sym = (ctf_link_sym_t *) ctf_sym;

	  if (sym->st_symidx > *max)
	    beyond_max++;
	}
      if (err != ECTF_NEXT_END)
	{
	  ctf_err_warn (fp, 0, err, _("iterating over linker-known symbols "
				      "during CTF serialization"));
	  ctf_dynhash_destroy (linker_known);
	  return (ctf_set_errno (fp, err));
	}
    }

  *idxsize = *count * sizeof (uint32_t);
  if (!(flags & CTF_SYMTYPETAB_FORCE_INDEXED))
    *padsize = (ctf_dynhash_elements (linker_known) - beyond_max) * sizeof (uint32_t);

  ctf_dynhash_destroy (linker_known);
  return 0;
}

/* Emit an objt or func symtypetab into DP in a particular order defined by an
   array of ctf_link_sym_t or symbol names passed in.  The index has NIDX
   elements in it: unindexed output would terminate at symbol OUTMAX and is in
   any case no larger than SIZE bytes.  Some index elements are expected to be
   skipped: see symtypetab_density.  The linker-reported set of symbols (if any)
   is found in SYMFP. */
static int
emit_symtypetab (ctf_dict_t *fp, ctf_dict_t *symfp, uint32_t *dp,
		 ctf_link_sym_t **idx, const char **nameidx, uint32_t nidx,
		 uint32_t outmax, int size, int flags)
{
  uint32_t i;
  uint32_t *dpp = dp;
  ctf_dynhash_t *symhash;

  ctf_dprintf ("Emitting table of size %i, outmax %u, %u symtypetab entries, "
	       "flags %i\n", size, outmax, nidx, flags);

  /* Empty table? Nothing to do.  */
  if (size == 0)
    return 0;

  if (flags & CTF_SYMTYPETAB_EMIT_FUNCTION)
    symhash = fp->ctf_funchash;
  else
    symhash = fp->ctf_objthash;

  for (i = 0; i < nidx; i++)
    {
      const char *sym_name;
      void *type;

      /* If we have a linker-reported set of symbols, we may be given that set
	 to work from, or a set of symbol names.  In both cases we want to look
	 at the corresponding linker-reported symbol (if any).  */
      if (!(flags & CTF_SYMTYPETAB_FORCE_INDEXED))
	{
	  ctf_link_sym_t *this_link_sym;

	  if (idx)
	    this_link_sym = idx[i];
	  else
	    this_link_sym = ctf_dynhash_lookup (symfp->ctf_dynsyms, nameidx[i]);

	  /* Unreported symbol number.  No pad, no nothing.  */
	  if (!this_link_sym)
	    continue;

	  /* Symbol of the wrong type, or skippable?  This symbol is not in this
	     table.  */
	  if (((flags & CTF_SYMTYPETAB_EMIT_FUNCTION)
	       && this_link_sym->st_type != STT_FUNC)
	      || (!(flags & CTF_SYMTYPETAB_EMIT_FUNCTION)
		  && this_link_sym->st_type != STT_OBJECT))
	    continue;

	  if (ctf_symtab_skippable (this_link_sym))
	    continue;

	  sym_name = this_link_sym->st_name;

	  /* Linker reports symbol of a different type to the symbol we actually
	     added?  Skip the symbol.  No pad, since the symbol doesn't actually
	     belong in this table at all.  (Warned about in
	     symtypetab_density.)  */
	  if ((this_link_sym->st_type == STT_FUNC)
	      && (ctf_dynhash_lookup (fp->ctf_objthash, sym_name)))
	    continue;

	  if ((this_link_sym->st_type == STT_OBJECT)
	      && (ctf_dynhash_lookup (fp->ctf_funchash, sym_name)))
	    continue;
	}
      else
	sym_name = nameidx[i];

      /* Symbol in index but no type set? Silently skip and (optionally)
	 pad.  (In force-indexed mode, this is also where we track symbols of
	 the wrong type for this round of insertion.)  */
      if ((type = ctf_dynhash_lookup (symhash, sym_name)) == NULL)
	{
	  if (flags & CTF_SYMTYPETAB_EMIT_PAD)
	    *dpp++ = 0;
	  continue;
	}

      if (!ctf_assert (fp, (((char *) dpp) - (char *) dp) < size))
	return -1;				/* errno is set for us.  */

      *dpp++ = (ctf_id_t) (uintptr_t) type;

      /* When emitting unindexed output, all later symbols are pads: stop
	 early.  */
      if ((flags & CTF_SYMTYPETAB_EMIT_PAD) && idx[i]->st_symidx == outmax)
	break;
    }

  return 0;
}

/* Emit an objt or func symtypetab index into DP in a paticular order defined by
   an array of symbol names passed in.  Stop at NIDX.  The linker-reported set
   of symbols (if any) is found in SYMFP. */
static int
emit_symtypetab_index (ctf_dict_t *fp, ctf_dict_t *symfp, uint32_t *dp,
		       const char **idx, uint32_t nidx, int size, int flags)
{
  uint32_t i;
  uint32_t *dpp = dp;
  ctf_dynhash_t *symhash;

  ctf_dprintf ("Emitting index of size %i, %u entries reported by linker, "
	       "flags %i\n", size, nidx, flags);

  /* Empty table? Nothing to do.  */
  if (size == 0)
    return 0;

  if (flags & CTF_SYMTYPETAB_EMIT_FUNCTION)
    symhash = fp->ctf_funchash;
  else
    symhash = fp->ctf_objthash;

  /* Indexes should always be unpadded.  */
  if (!ctf_assert (fp, !(flags & CTF_SYMTYPETAB_EMIT_PAD)))
    return -1;					/* errno is set for us.  */

  for (i = 0; i < nidx; i++)
    {
      const char *sym_name;
      void *type;

      if (!(flags & CTF_SYMTYPETAB_FORCE_INDEXED))
	{
	  ctf_link_sym_t *this_link_sym;

	  this_link_sym = ctf_dynhash_lookup (symfp->ctf_dynsyms, idx[i]);

	  /* This is an index: unreported symbols should never appear in it.  */
	  if (!ctf_assert (fp, this_link_sym != NULL))
	    return -1;				/* errno is set for us.  */

	  /* Symbol of the wrong type, or skippable?  This symbol is not in this
	     table.  */
	  if (((flags & CTF_SYMTYPETAB_EMIT_FUNCTION)
	       && this_link_sym->st_type != STT_FUNC)
	      || (!(flags & CTF_SYMTYPETAB_EMIT_FUNCTION)
		  && this_link_sym->st_type != STT_OBJECT))
	    continue;

	  if (ctf_symtab_skippable (this_link_sym))
	    continue;

	  sym_name = this_link_sym->st_name;

	  /* Linker reports symbol of a different type to the symbol we actually
	     added?  Skip the symbol.  */
	  if ((this_link_sym->st_type == STT_FUNC)
	      && (ctf_dynhash_lookup (fp->ctf_objthash, sym_name)))
	    continue;

	  if ((this_link_sym->st_type == STT_OBJECT)
	      && (ctf_dynhash_lookup (fp->ctf_funchash, sym_name)))
	    continue;
	}
      else
	sym_name = idx[i];

      /* Symbol in index and reported by linker, but no type set? Silently skip
	 and (optionally) pad.  (In force-indexed mode, this is also where we
	 track symbols of the wrong type for this round of insertion.)  */
      if ((type = ctf_dynhash_lookup (symhash, sym_name)) == NULL)
	continue;

      ctf_str_add_ref (fp, sym_name, dpp++);

      if (!ctf_assert (fp, (((char *) dpp) - (char *) dp) <= size))
	return -1;				/* errno is set for us.  */
    }

  return 0;
}

/* Delete symbols that have been assigned names from the variable section.  Must
   be called from within ctf_serialize, because that is the only place you can
   safely delete variables without messing up ctf_rollback.  */

static int
symtypetab_delete_nonstatics (ctf_dict_t *fp, ctf_dict_t *symfp)
{
  ctf_dvdef_t *dvd, *nvd;
  ctf_id_t type;

  for (dvd = ctf_list_next (&fp->ctf_dvdefs); dvd != NULL; dvd = nvd)
    {
      nvd = ctf_list_next (dvd);

      if ((((type = (ctf_id_t) (uintptr_t)
	     ctf_dynhash_lookup (fp->ctf_objthash, dvd->dvd_name)) > 0)
	   || (type = (ctf_id_t) (uintptr_t)
	       ctf_dynhash_lookup (fp->ctf_funchash, dvd->dvd_name)) > 0)
	  && ctf_dynhash_lookup (symfp->ctf_dynsyms, dvd->dvd_name) != NULL
	  && type == dvd->dvd_type)
	ctf_dvd_delete (fp, dvd);
    }

  return 0;
}

/* Figure out the sizes of the symtypetab sections, their indexed state,
   etc.  */
static int
ctf_symtypetab_sect_sizes (ctf_dict_t *fp, emit_symtypetab_state_t *s,
			   ctf_header_t *hdr, size_t *objt_size,
			   size_t *func_size, size_t *objtidx_size,
			   size_t *funcidx_size)
{
  size_t nfuncs, nobjts;
  size_t objt_unpadsize, func_unpadsize, objt_padsize, func_padsize;

  /* If doing a writeout as part of linking, and the link flags request it,
     filter out reported symbols from the variable section, and filter out all
     other symbols from the symtypetab sections.  (If we are not linking, the
     symbols are sorted; if we are linking, don't bother sorting if we are not
     filtering out reported symbols: this is almost certaily an ld -r and only
     the linker is likely to consume these symtypetabs again.  The linker
     doesn't care what order the symtypetab entries is in, since it only
     iterates over symbols and does not use the ctf_lookup_by_symbol* API.)  */

  s->sort_syms = 1;
  if (fp->ctf_flags & LCTF_LINKING)
    {
      s->filter_syms = !(fp->ctf_link_flags & CTF_LINK_NO_FILTER_REPORTED_SYMS);
      if (!s->filter_syms)
	s->sort_syms = 0;
    }

  /* Find the dict to which the linker has reported symbols, if any.  */

  if (s->filter_syms)
    {
      if (!fp->ctf_dynsyms && fp->ctf_parent && fp->ctf_parent->ctf_dynsyms)
	s->symfp = fp->ctf_parent;
      else
	s->symfp = fp;
    }

  /* If not filtering, keep all potential symbols in an unsorted, indexed
     dict.  */
  if (!s->filter_syms)
    s->symflags = CTF_SYMTYPETAB_FORCE_INDEXED;
  else
    hdr->cth_flags |= CTF_F_IDXSORTED;

  if (!ctf_assert (fp, (s->filter_syms && s->symfp)
		   || (!s->filter_syms && !s->symfp
		       && ((s->symflags & CTF_SYMTYPETAB_FORCE_INDEXED) != 0))))
    return -1;

  /* Work out the sizes of the object and function sections, and work out the
     number of pad (unassigned) symbols in each, and the overall size of the
     sections.  */

  if (symtypetab_density (fp, s->symfp, fp->ctf_objthash, &nobjts, &s->maxobjt,
			  &objt_unpadsize, &objt_padsize, objtidx_size,
			  s->symflags) < 0)
    return -1;					/* errno is set for us.  */

  ctf_dprintf ("Object symtypetab: %i objects, max %i, unpadded size %i, "
	       "%i bytes of pads, index size %i\n", (int) nobjts,
	       (int) s->maxobjt, (int) objt_unpadsize, (int) objt_padsize,
	       (int) *objtidx_size);

  if (symtypetab_density (fp, s->symfp, fp->ctf_funchash, &nfuncs, &s->maxfunc,
			  &func_unpadsize, &func_padsize, funcidx_size,
			  s->symflags | CTF_SYMTYPETAB_EMIT_FUNCTION) < 0)
    return -1;					/* errno is set for us.  */

  ctf_dprintf ("Function symtypetab: %i functions, max %i, unpadded size %i, "
	       "%i bytes of pads, index size %i\n", (int) nfuncs,
	       (int) s->maxfunc, (int) func_unpadsize, (int) func_padsize,
	       (int) *funcidx_size);

  /* It is worth indexing each section if it would save space to do so, due to
     reducing the number of pads sufficiently.  A pad is the same size as a
     single index entry: but index sections compress relatively poorly compared
     to constant pads, so it takes a lot of contiguous padding to equal one
     index section entry.  It would be nice to be able to *verify* whether we
     would save space after compression rather than guessing, but this seems
     difficult, since it would require complete reserialization.  Regardless, if
     the linker has not reported any symbols (e.g. if this is not a final link
     but just an ld -r), we must emit things in indexed fashion just as the
     compiler does.  */

  *objt_size = objt_unpadsize;
  if (!(s->symflags & CTF_SYMTYPETAB_FORCE_INDEXED)
      && ((objt_padsize + objt_unpadsize) * CTF_INDEX_PAD_THRESHOLD
	  > objt_padsize))
    {
      *objt_size += objt_padsize;
      *objtidx_size = 0;
    }

  *func_size = func_unpadsize;
  if (!(s->symflags & CTF_SYMTYPETAB_FORCE_INDEXED)
      && ((func_padsize + func_unpadsize) * CTF_INDEX_PAD_THRESHOLD
	  > func_padsize))
    {
      *func_size += func_padsize;
      *funcidx_size = 0;
    }

  /* If we are filtering symbols out, those symbols that the linker has not
     reported have now been removed from the ctf_objthash and ctf_funchash.
     Delete entries from the variable section that duplicate newly-added
     symbols.  There's no need to migrate new ones in: we do that (if necessary)
     in ctf_link_deduplicating_variables.  */

  if (s->filter_syms && s->symfp->ctf_dynsyms &&
      symtypetab_delete_nonstatics (fp, s->symfp) < 0)
    return -1;

  return 0;
}

static int
ctf_emit_symtypetab_sects (ctf_dict_t *fp, emit_symtypetab_state_t *s,
			   unsigned char **tptr, size_t objt_size,
			   size_t func_size, size_t objtidx_size,
			   size_t funcidx_size)
{
  unsigned char *t = *tptr;
  size_t nsymtypes = 0;
  const char **sym_name_order = NULL;
  int err;

  /* Sort the linker's symbols into name order if need be.  */

  if ((objtidx_size != 0) || (funcidx_size != 0))
    {
      ctf_next_t *i = NULL;
      void *symname;
      const char **walk;

      if (s->filter_syms)
	{
	  if (s->symfp->ctf_dynsyms)
	    nsymtypes = ctf_dynhash_elements (s->symfp->ctf_dynsyms);
	  else
	    nsymtypes = 0;
	}
      else
	nsymtypes = ctf_dynhash_elements (fp->ctf_objthash)
	  + ctf_dynhash_elements (fp->ctf_funchash);

      if ((sym_name_order = calloc (nsymtypes, sizeof (const char *))) == NULL)
	goto oom;

      walk = sym_name_order;

      if (s->filter_syms)
	{
	  if (s->symfp->ctf_dynsyms)
	    {
	      while ((err = ctf_dynhash_next_sorted (s->symfp->ctf_dynsyms, &i,
						     &symname, NULL,
						     ctf_dynhash_sort_by_name,
						     NULL)) == 0)
		*walk++ = (const char *) symname;
	      if (err != ECTF_NEXT_END)
		goto symerr;
	    }
	}
      else
	{
	  ctf_hash_sort_f sort_fun = NULL;

	  /* Since we partition the set of symbols back into objt and func,
	     we can sort the two independently without harm.  */
	  if (s->sort_syms)
	    sort_fun = ctf_dynhash_sort_by_name;

	  while ((err = ctf_dynhash_next_sorted (fp->ctf_objthash, &i, &symname,
						 NULL, sort_fun, NULL)) == 0)
	    *walk++ = (const char *) symname;
	  if (err != ECTF_NEXT_END)
	    goto symerr;

	  while ((err = ctf_dynhash_next_sorted (fp->ctf_funchash, &i, &symname,
						 NULL, sort_fun, NULL)) == 0)
	    *walk++ = (const char *) symname;
	  if (err != ECTF_NEXT_END)
	    goto symerr;
	}
    }

  /* Emit the object and function sections, and if necessary their indexes.
     Emission is done in symtab order if there is no index, and in index
     (name) order otherwise.  */

  if ((objtidx_size == 0) && s->symfp && s->symfp->ctf_dynsymidx)
    {
      ctf_dprintf ("Emitting unindexed objt symtypetab\n");
      if (emit_symtypetab (fp, s->symfp, (uint32_t *) t,
			   s->symfp->ctf_dynsymidx, NULL,
			   s->symfp->ctf_dynsymmax + 1, s->maxobjt,
			   objt_size, s->symflags | CTF_SYMTYPETAB_EMIT_PAD) < 0)
	goto err;				/* errno is set for us.  */
    }
  else
    {
      ctf_dprintf ("Emitting indexed objt symtypetab\n");
      if (emit_symtypetab (fp, s->symfp, (uint32_t *) t, NULL,
			   sym_name_order, nsymtypes, s->maxobjt,
			   objt_size, s->symflags) < 0)
	goto err;				/* errno is set for us.  */
    }

  t += objt_size;

  if ((funcidx_size == 0) && s->symfp && s->symfp->ctf_dynsymidx)
    {
      ctf_dprintf ("Emitting unindexed func symtypetab\n");
      if (emit_symtypetab (fp, s->symfp, (uint32_t *) t,
			   s->symfp->ctf_dynsymidx, NULL,
			   s->symfp->ctf_dynsymmax + 1, s->maxfunc,
			   func_size, s->symflags | CTF_SYMTYPETAB_EMIT_FUNCTION
			   | CTF_SYMTYPETAB_EMIT_PAD) < 0)
	goto err;				/* errno is set for us.  */
    }
  else
    {
      ctf_dprintf ("Emitting indexed func symtypetab\n");
      if (emit_symtypetab (fp, s->symfp, (uint32_t *) t, NULL, sym_name_order,
			   nsymtypes, s->maxfunc, func_size,
			   s->symflags | CTF_SYMTYPETAB_EMIT_FUNCTION) < 0)
	goto err;				/* errno is set for us.  */
    }

  t += func_size;

  if (objtidx_size > 0)
    if (emit_symtypetab_index (fp, s->symfp, (uint32_t *) t, sym_name_order,
			       nsymtypes, objtidx_size, s->symflags) < 0)
      goto err;

  t += objtidx_size;

  if (funcidx_size > 0)
    if (emit_symtypetab_index (fp, s->symfp, (uint32_t *) t, sym_name_order,
			       nsymtypes, funcidx_size,
			       s->symflags | CTF_SYMTYPETAB_EMIT_FUNCTION) < 0)
      goto err;

  t += funcidx_size;
  free (sym_name_order);
  *tptr = t;

  return 0;

 oom:
  ctf_set_errno (fp, EAGAIN);
  goto err;
symerr:
  ctf_err_warn (fp, 0, err, _("error serializing symtypetabs"));
 err:
  free (sym_name_order);
  return -1;
}

/* Type section.  */

/* Iterate through the dynamic type definition list and compute the
   size of the CTF type section.  */

static size_t
ctf_type_sect_size (ctf_dict_t *fp)
{
  ctf_dtdef_t *dtd;
  size_t type_size;

  for (type_size = 0, dtd = ctf_list_next (&fp->ctf_dtdefs);
       dtd != NULL; dtd = ctf_list_next (dtd))
    {
      uint32_t kind = LCTF_INFO_KIND (fp, dtd->dtd_data.ctt_info);
      uint32_t vlen = LCTF_INFO_VLEN (fp, dtd->dtd_data.ctt_info);
      size_t type_ctt_size = dtd->dtd_data.ctt_size;

      /* Shrink ctf_type_t-using types from a ctf_type_t to a ctf_stype_t
	 if possible.  */

      if (kind == CTF_K_STRUCT || kind == CTF_K_UNION)
	{
	  size_t lsize = CTF_TYPE_LSIZE (&dtd->dtd_data);

	  if (lsize <= CTF_MAX_SIZE)
	    type_ctt_size = lsize;
	}

      if (type_ctt_size != CTF_LSIZE_SENT)
	type_size += sizeof (ctf_stype_t);
      else
	type_size += sizeof (ctf_type_t);

      switch (kind)
	{
	case CTF_K_INTEGER:
	case CTF_K_FLOAT:
	  type_size += sizeof (uint32_t);
	  break;
	case CTF_K_ARRAY:
	  type_size += sizeof (ctf_array_t);
	  break;
	case CTF_K_SLICE:
	  type_size += sizeof (ctf_slice_t);
	  break;
	case CTF_K_FUNCTION:
	  type_size += sizeof (uint32_t) * (vlen + (vlen & 1));
	  break;
	case CTF_K_STRUCT:
	case CTF_K_UNION:
	  if (type_ctt_size < CTF_LSTRUCT_THRESH)
	    type_size += sizeof (ctf_member_t) * vlen;
	  else
	    type_size += sizeof (ctf_lmember_t) * vlen;
	  break;
	case CTF_K_ENUM:
	  type_size += sizeof (ctf_enum_t) * vlen;
	  break;
	}
    }

  return type_size;
}

/* Take a final lap through the dynamic type definition list and copy the
   appropriate type records to the output buffer, noting down the strings as
   we go.  */

static void
ctf_emit_type_sect (ctf_dict_t *fp, unsigned char **tptr)
{
  unsigned char *t = *tptr;
  ctf_dtdef_t *dtd;

  for (dtd = ctf_list_next (&fp->ctf_dtdefs);
       dtd != NULL; dtd = ctf_list_next (dtd))
    {
      uint32_t kind = LCTF_INFO_KIND (fp, dtd->dtd_data.ctt_info);
      uint32_t vlen = LCTF_INFO_VLEN (fp, dtd->dtd_data.ctt_info);
      size_t type_ctt_size = dtd->dtd_data.ctt_size;
      size_t len;
      ctf_stype_t *copied;
      const char *name;
      size_t i;

      /* Shrink ctf_type_t-using types from a ctf_type_t to a ctf_stype_t
	 if possible.  */

      if (kind == CTF_K_STRUCT || kind == CTF_K_UNION)
	{
	  size_t lsize = CTF_TYPE_LSIZE (&dtd->dtd_data);

	  if (lsize <= CTF_MAX_SIZE)
	    type_ctt_size = lsize;
	}

      if (type_ctt_size != CTF_LSIZE_SENT)
	len = sizeof (ctf_stype_t);
      else
	len = sizeof (ctf_type_t);

      memcpy (t, &dtd->dtd_data, len);
      copied = (ctf_stype_t *) t;  /* name is at the start: constant offset.  */
      if (copied->ctt_name
	  && (name = ctf_strraw (fp, copied->ctt_name)) != NULL)
	{
	  ctf_str_add_ref (fp, name, &copied->ctt_name);
	  ctf_str_add_ref (fp, name, &dtd->dtd_data.ctt_name);
	}
      copied->ctt_size = type_ctt_size;
      t += len;

      switch (kind)
	{
	case CTF_K_INTEGER:
	case CTF_K_FLOAT:
	  memcpy (t, dtd->dtd_vlen, sizeof (uint32_t));
	  t += sizeof (uint32_t);
	  break;

	case CTF_K_SLICE:
	  memcpy (t, dtd->dtd_vlen, sizeof (struct ctf_slice));
	  t += sizeof (struct ctf_slice);
	  break;

	case CTF_K_ARRAY:
	  memcpy (t, dtd->dtd_vlen, sizeof (struct ctf_array));
	  t += sizeof (struct ctf_array);
	  break;

	case CTF_K_FUNCTION:
	  /* Functions with no args also have no vlen.  */
	  if (dtd->dtd_vlen)
	    memcpy (t, dtd->dtd_vlen, sizeof (uint32_t) * (vlen + (vlen & 1)));
	  t += sizeof (uint32_t) * (vlen + (vlen & 1));
	  break;

	  /* These need to be copied across element by element, depending on
	     their ctt_size.  */
	case CTF_K_STRUCT:
	case CTF_K_UNION:
	  {
	    ctf_lmember_t *dtd_vlen = (ctf_lmember_t *) dtd->dtd_vlen;
	    ctf_lmember_t *t_lvlen = (ctf_lmember_t *) t;
	    ctf_member_t *t_vlen = (ctf_member_t *) t;

	    for (i = 0; i < vlen; i++)
	      {
		const char *name = ctf_strraw (fp, dtd_vlen[i].ctlm_name);

		ctf_str_add_ref (fp, name, &dtd_vlen[i].ctlm_name);

		if (type_ctt_size < CTF_LSTRUCT_THRESH)
		  {
		    t_vlen[i].ctm_name = dtd_vlen[i].ctlm_name;
		    t_vlen[i].ctm_type = dtd_vlen[i].ctlm_type;
		    t_vlen[i].ctm_offset = CTF_LMEM_OFFSET (&dtd_vlen[i]);
		    ctf_str_add_ref (fp, name, &t_vlen[i].ctm_name);
		  }
		else
		  {
		    t_lvlen[i] = dtd_vlen[i];
		    ctf_str_add_ref (fp, name, &t_lvlen[i].ctlm_name);
		  }
	      }
	  }

	  if (type_ctt_size < CTF_LSTRUCT_THRESH)
	    t += sizeof (ctf_member_t) * vlen;
	  else
	    t += sizeof (ctf_lmember_t) * vlen;
	  break;

	case CTF_K_ENUM:
	  {
	    ctf_enum_t *dtd_vlen = (struct ctf_enum *) dtd->dtd_vlen;
	    ctf_enum_t *t_vlen = (struct ctf_enum *) t;

	    memcpy (t, dtd->dtd_vlen, sizeof (struct ctf_enum) * vlen);
	    for (i = 0; i < vlen; i++)
	      {
		const char *name = ctf_strraw (fp, dtd_vlen[i].cte_name);

		ctf_str_add_ref (fp, name, &t_vlen[i].cte_name);
		ctf_str_add_ref (fp, name, &dtd_vlen[i].cte_name);
	      }
	    t += sizeof (struct ctf_enum) * vlen;

	    break;
	  }
	}
    }

  *tptr = t;
}

/* Variable section.  */

/* Sort a newly-constructed static variable array.  */

typedef struct ctf_sort_var_arg_cb
{
  ctf_dict_t *fp;
  ctf_strs_t *strtab;
} ctf_sort_var_arg_cb_t;

static int
ctf_sort_var (const void *one_, const void *two_, void *arg_)
{
  const ctf_varent_t *one = one_;
  const ctf_varent_t *two = two_;
  ctf_sort_var_arg_cb_t *arg = arg_;

  return (strcmp (ctf_strraw_explicit (arg->fp, one->ctv_name, arg->strtab),
		  ctf_strraw_explicit (arg->fp, two->ctv_name, arg->strtab)));
}

/* Overall serialization.  */

/* If the specified CTF dict is writable and has been modified, reload this dict
   with the updated type definitions, ready for serialization.  In order to make
   this code and the rest of libctf as simple as possible, we perform updates by
   taking the dynamic type definitions and creating an in-memory CTF dict
   containing the definitions, and then call ctf_simple_open_internal() on it.
   We perform one extra trick here for the benefit of callers and to keep our
   code simple: ctf_simple_open_internal() will return a new ctf_dict_t, but we
   want to keep the fp constant for the caller, so after
   ctf_simple_open_internal() returns, we use memcpy to swap the interior of the
   old and new ctf_dict_t's, and then free the old.  */
int
ctf_serialize (ctf_dict_t *fp)
{
  ctf_dict_t ofp, *nfp;
  ctf_header_t hdr, *hdrp;
  ctf_dvdef_t *dvd;
  ctf_varent_t *dvarents;
  ctf_strs_writable_t strtab;
  int err;
  int num_missed_str_refs;

  unsigned char *t;
  unsigned long i;
  size_t buf_size, type_size, objt_size, func_size;
  size_t funcidx_size, objtidx_size;
  size_t nvars;
  unsigned char *buf = NULL, *newbuf;

  emit_symtypetab_state_t symstate;
  memset (&symstate, 0, sizeof (emit_symtypetab_state_t));

  if (!(fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (fp, ECTF_RDONLY));

  /* Update required?  */
  if (!(fp->ctf_flags & LCTF_DIRTY))
    return 0;

  /* The strtab refs table must be empty at this stage.  Any refs already added
     will be corrupted by any modifications, including reserialization, after
     strtab finalization is complete.  Only this function, and functions it
     calls, may add refs, and all memory locations (including in the dtds)
     containing strtab offsets must be traversed as part of serialization, and
     refs added.  */

  if (!ctf_assert (fp, fp->ctf_str_num_refs == 0))
    return -1;					/* errno is set for us.  */

  /* Fill in an initial CTF header.  We will leave the label, object,
     and function sections empty and only output a header, type section,
     and string table.  The type section begins at a 4-byte aligned
     boundary past the CTF header itself (at relative offset zero).  The flag
     indicating a new-style function info section (an array of CTF_K_FUNCTION
     type IDs in the types section) is flipped on.  */

  memset (&hdr, 0, sizeof (hdr));
  hdr.cth_magic = CTF_MAGIC;
  hdr.cth_version = CTF_VERSION;

  /* This is a new-format func info section, and the symtab and strtab come out
     of the dynsym and dynstr these days.  */
  hdr.cth_flags = (CTF_F_NEWFUNCINFO | CTF_F_DYNSTR);

  if (ctf_symtypetab_sect_sizes (fp, &symstate, &hdr, &objt_size, &func_size,
				 &objtidx_size, &funcidx_size) < 0)
    return -1;					/* errno is set for us.  */

  for (nvars = 0, dvd = ctf_list_next (&fp->ctf_dvdefs);
       dvd != NULL; dvd = ctf_list_next (dvd), nvars++);

  type_size = ctf_type_sect_size (fp);

  /* Compute the size of the CTF buffer we need, sans only the string table,
     then allocate a new buffer and memcpy the finished header to the start of
     the buffer.  (We will adjust this later with strtab length info.)  */

  hdr.cth_lbloff = hdr.cth_objtoff = 0;
  hdr.cth_funcoff = hdr.cth_objtoff + objt_size;
  hdr.cth_objtidxoff = hdr.cth_funcoff + func_size;
  hdr.cth_funcidxoff = hdr.cth_objtidxoff + objtidx_size;
  hdr.cth_varoff = hdr.cth_funcidxoff + funcidx_size;
  hdr.cth_typeoff = hdr.cth_varoff + (nvars * sizeof (ctf_varent_t));
  hdr.cth_stroff = hdr.cth_typeoff + type_size;
  hdr.cth_strlen = 0;

  buf_size = sizeof (ctf_header_t) + hdr.cth_stroff + hdr.cth_strlen;

  if ((buf = malloc (buf_size)) == NULL)
    return (ctf_set_errno (fp, EAGAIN));

  memcpy (buf, &hdr, sizeof (ctf_header_t));
  t = (unsigned char *) buf + sizeof (ctf_header_t) + hdr.cth_objtoff;

  hdrp = (ctf_header_t *) buf;
  if ((fp->ctf_flags & LCTF_CHILD) && (fp->ctf_parname != NULL))
    ctf_str_add_ref (fp, fp->ctf_parname, &hdrp->cth_parname);
  if (fp->ctf_cuname != NULL)
    ctf_str_add_ref (fp, fp->ctf_cuname, &hdrp->cth_cuname);

  if (ctf_emit_symtypetab_sects (fp, &symstate, &t, objt_size, func_size,
				 objtidx_size, funcidx_size) < 0)
    goto err;

  assert (t == (unsigned char *) buf + sizeof (ctf_header_t) + hdr.cth_varoff);

  /* Work over the variable list, translating everything into ctf_varent_t's and
     prepping the string table.  */

  dvarents = (ctf_varent_t *) t;
  for (i = 0, dvd = ctf_list_next (&fp->ctf_dvdefs); dvd != NULL;
       dvd = ctf_list_next (dvd), i++)
    {
      ctf_varent_t *var = &dvarents[i];

      ctf_str_add_ref (fp, dvd->dvd_name, &var->ctv_name);
      var->ctv_type = (uint32_t) dvd->dvd_type;
    }
  assert (i == nvars);

  t += sizeof (ctf_varent_t) * nvars;

  assert (t == (unsigned char *) buf + sizeof (ctf_header_t) + hdr.cth_typeoff);

  ctf_emit_type_sect (fp, &t);

  assert (t == (unsigned char *) buf + sizeof (ctf_header_t) + hdr.cth_stroff);

  /* Every string added outside serialization by ctf_str_add_pending should
     now have been added by ctf_add_ref.  */
  num_missed_str_refs = ctf_dynset_elements (fp->ctf_str_pending_ref);
  if (!ctf_assert (fp, num_missed_str_refs == 0))
    goto err;					/* errno is set for us.  */

  /* Construct the final string table and fill out all the string refs with the
     final offsets.  Then purge the refs list, because we're about to move this
     strtab onto the end of the buf, invalidating all the offsets.  */
  strtab = ctf_str_write_strtab (fp);
  ctf_str_purge_refs (fp);

  if (strtab.cts_strs == NULL)
    goto oom;

  /* Now the string table is constructed, we can sort the buffer of
     ctf_varent_t's.  */
  ctf_sort_var_arg_cb_t sort_var_arg = { fp, (ctf_strs_t *) &strtab };
  ctf_qsort_r (dvarents, nvars, sizeof (ctf_varent_t), ctf_sort_var,
	       &sort_var_arg);

  if ((newbuf = ctf_realloc (fp, buf, buf_size + strtab.cts_len)) == NULL)
    {
      free (strtab.cts_strs);
      goto oom;
    }
  buf = newbuf;
  memcpy (buf + buf_size, strtab.cts_strs, strtab.cts_len);
  hdrp = (ctf_header_t *) buf;
  hdrp->cth_strlen = strtab.cts_len;
  buf_size += hdrp->cth_strlen;
  free (strtab.cts_strs);

  /* Finally, we are ready to ctf_simple_open() the new dict.  If this is
     successful, we then switch nfp and fp and free the old dict.  */

  if ((nfp = ctf_simple_open_internal ((char *) buf, buf_size, NULL, 0,
				       0, NULL, 0, fp->ctf_syn_ext_strtab,
				       1, &err)) == NULL)
    {
      free (buf);
      return (ctf_set_errno (fp, err));
    }

  (void) ctf_setmodel (nfp, ctf_getmodel (fp));

  nfp->ctf_parent = fp->ctf_parent;
  nfp->ctf_parent_unreffed = fp->ctf_parent_unreffed;
  nfp->ctf_refcnt = fp->ctf_refcnt;
  nfp->ctf_flags |= fp->ctf_flags & ~LCTF_DIRTY;
  if (nfp->ctf_dynbase == NULL)
    nfp->ctf_dynbase = buf;		/* Make sure buf is freed on close.  */
  nfp->ctf_dthash = fp->ctf_dthash;
  nfp->ctf_dtdefs = fp->ctf_dtdefs;
  nfp->ctf_dvhash = fp->ctf_dvhash;
  nfp->ctf_dvdefs = fp->ctf_dvdefs;
  nfp->ctf_dtoldid = fp->ctf_dtoldid;
  nfp->ctf_add_processing = fp->ctf_add_processing;
  nfp->ctf_snapshots = fp->ctf_snapshots + 1;
  nfp->ctf_specific = fp->ctf_specific;
  nfp->ctf_nfuncidx = fp->ctf_nfuncidx;
  nfp->ctf_nobjtidx = fp->ctf_nobjtidx;
  nfp->ctf_objthash = fp->ctf_objthash;
  nfp->ctf_funchash = fp->ctf_funchash;
  nfp->ctf_dynsyms = fp->ctf_dynsyms;
  nfp->ctf_ptrtab = fp->ctf_ptrtab;
  nfp->ctf_pptrtab = fp->ctf_pptrtab;
  nfp->ctf_typemax = fp->ctf_typemax;
  nfp->ctf_dynsymidx = fp->ctf_dynsymidx;
  nfp->ctf_dynsymmax = fp->ctf_dynsymmax;
  nfp->ctf_ptrtab_len = fp->ctf_ptrtab_len;
  nfp->ctf_pptrtab_len = fp->ctf_pptrtab_len;
  nfp->ctf_link_inputs = fp->ctf_link_inputs;
  nfp->ctf_link_outputs = fp->ctf_link_outputs;
  nfp->ctf_errs_warnings = fp->ctf_errs_warnings;
  nfp->ctf_funcidx_names = fp->ctf_funcidx_names;
  nfp->ctf_objtidx_names = fp->ctf_objtidx_names;
  nfp->ctf_funcidx_sxlate = fp->ctf_funcidx_sxlate;
  nfp->ctf_objtidx_sxlate = fp->ctf_objtidx_sxlate;
  nfp->ctf_str_prov_offset = fp->ctf_str_prov_offset;
  nfp->ctf_syn_ext_strtab = fp->ctf_syn_ext_strtab;
  nfp->ctf_pptrtab_typemax = fp->ctf_pptrtab_typemax;
  nfp->ctf_in_flight_dynsyms = fp->ctf_in_flight_dynsyms;
  nfp->ctf_link_in_cu_mapping = fp->ctf_link_in_cu_mapping;
  nfp->ctf_link_out_cu_mapping = fp->ctf_link_out_cu_mapping;
  nfp->ctf_link_type_mapping = fp->ctf_link_type_mapping;
  nfp->ctf_link_memb_name_changer = fp->ctf_link_memb_name_changer;
  nfp->ctf_link_memb_name_changer_arg = fp->ctf_link_memb_name_changer_arg;
  nfp->ctf_link_variable_filter = fp->ctf_link_variable_filter;
  nfp->ctf_link_variable_filter_arg = fp->ctf_link_variable_filter_arg;
  nfp->ctf_symsect_little_endian = fp->ctf_symsect_little_endian;
  nfp->ctf_link_flags = fp->ctf_link_flags;
  nfp->ctf_dedup_atoms = fp->ctf_dedup_atoms;
  nfp->ctf_dedup_atoms_alloc = fp->ctf_dedup_atoms_alloc;
  memcpy (&nfp->ctf_dedup, &fp->ctf_dedup, sizeof (fp->ctf_dedup));

  nfp->ctf_snapshot_lu = fp->ctf_snapshots;

  memcpy (&nfp->ctf_lookups, fp->ctf_lookups, sizeof (fp->ctf_lookups));
  nfp->ctf_structs = fp->ctf_structs;
  nfp->ctf_unions = fp->ctf_unions;
  nfp->ctf_enums = fp->ctf_enums;
  nfp->ctf_names = fp->ctf_names;

  fp->ctf_dthash = NULL;
  ctf_str_free_atoms (nfp);
  nfp->ctf_str_atoms = fp->ctf_str_atoms;
  nfp->ctf_prov_strtab = fp->ctf_prov_strtab;
  nfp->ctf_str_pending_ref = fp->ctf_str_pending_ref;
  fp->ctf_str_atoms = NULL;
  fp->ctf_prov_strtab = NULL;
  fp->ctf_str_pending_ref = NULL;
  memset (&fp->ctf_dtdefs, 0, sizeof (ctf_list_t));
  memset (&fp->ctf_errs_warnings, 0, sizeof (ctf_list_t));
  fp->ctf_add_processing = NULL;
  fp->ctf_ptrtab = NULL;
  fp->ctf_pptrtab = NULL;
  fp->ctf_funcidx_names = NULL;
  fp->ctf_objtidx_names = NULL;
  fp->ctf_funcidx_sxlate = NULL;
  fp->ctf_objtidx_sxlate = NULL;
  fp->ctf_objthash = NULL;
  fp->ctf_funchash = NULL;
  fp->ctf_dynsyms = NULL;
  fp->ctf_dynsymidx = NULL;
  fp->ctf_link_inputs = NULL;
  fp->ctf_link_outputs = NULL;
  fp->ctf_syn_ext_strtab = NULL;
  fp->ctf_link_in_cu_mapping = NULL;
  fp->ctf_link_out_cu_mapping = NULL;
  fp->ctf_link_type_mapping = NULL;
  fp->ctf_dedup_atoms = NULL;
  fp->ctf_dedup_atoms_alloc = NULL;
  fp->ctf_parent_unreffed = 1;

  fp->ctf_dvhash = NULL;
  memset (&fp->ctf_dvdefs, 0, sizeof (ctf_list_t));
  memset (fp->ctf_lookups, 0, sizeof (fp->ctf_lookups));
  memset (&fp->ctf_in_flight_dynsyms, 0, sizeof (fp->ctf_in_flight_dynsyms));
  memset (&fp->ctf_dedup, 0, sizeof (fp->ctf_dedup));
  fp->ctf_structs.ctn_writable = NULL;
  fp->ctf_unions.ctn_writable = NULL;
  fp->ctf_enums.ctn_writable = NULL;
  fp->ctf_names.ctn_writable = NULL;

  memcpy (&ofp, fp, sizeof (ctf_dict_t));
  memcpy (fp, nfp, sizeof (ctf_dict_t));
  memcpy (nfp, &ofp, sizeof (ctf_dict_t));

  nfp->ctf_refcnt = 1;				/* Force nfp to be freed.  */
  ctf_dict_close (nfp);

  return 0;

oom:
  free (buf);
  return (ctf_set_errno (fp, EAGAIN));
err:
  free (buf);
  return -1;					/* errno is set for us.  */
}

/* File writing.  */

/* Write the compressed CTF data stream to the specified gzFile descriptor.  The
   whole stream is compressed, and cannot be read by CTF opening functions in
   this library until it is decompressed.  (The functions below this one leave
   the header uncompressed, and the CTF opening functions work on them without
   manual decompression.)

   No support for (testing-only) endian-flipping.  */
int
ctf_gzwrite (ctf_dict_t *fp, gzFile fd)
{
  const unsigned char *buf;
  ssize_t resid;
  ssize_t len;

  resid = sizeof (ctf_header_t);
  buf = (unsigned char *) fp->ctf_header;
  while (resid != 0)
    {
      if ((len = gzwrite (fd, buf, resid)) <= 0)
	return (ctf_set_errno (fp, errno));
      resid -= len;
      buf += len;
    }

  resid = fp->ctf_size;
  buf = fp->ctf_buf;
  while (resid != 0)
    {
      if ((len = gzwrite (fd, buf, resid)) <= 0)
	return (ctf_set_errno (fp, errno));
      resid -= len;
      buf += len;
    }

  return 0;
}

/* Optionally compress the specified CTF data stream and return it as a new
   dynamically-allocated string.  Possibly write it with reversed
   endianness.  */
unsigned char *
ctf_write_mem (ctf_dict_t *fp, size_t *size, size_t threshold)
{
  unsigned char *buf;
  unsigned char *bp;
  ctf_header_t *hp;
  unsigned char *flipped, *src;
  ssize_t header_len = sizeof (ctf_header_t);
  ssize_t compress_len;
  int flip_endian;
  int uncompressed;
  int rc;

  flip_endian = getenv ("LIBCTF_WRITE_FOREIGN_ENDIAN") != NULL;
  uncompressed = (fp->ctf_size < threshold);

  if (ctf_serialize (fp) < 0)
    return NULL;				/* errno is set for us.  */

  compress_len = compressBound (fp->ctf_size);
  if (fp->ctf_size < threshold)
    compress_len = fp->ctf_size;
  if ((buf = malloc (compress_len
		     + sizeof (struct ctf_header))) == NULL)
    {
      ctf_set_errno (fp, ENOMEM);
      ctf_err_warn (fp, 0, 0, _("ctf_write_mem: cannot allocate %li bytes"),
		    (unsigned long) (compress_len + sizeof (struct ctf_header)));
      return NULL;
    }

  hp = (ctf_header_t *) buf;
  memcpy (hp, fp->ctf_header, header_len);
  bp = buf + sizeof (struct ctf_header);
  *size = sizeof (struct ctf_header);

  if (uncompressed)
    hp->cth_flags &= ~CTF_F_COMPRESS;
  else
    hp->cth_flags |= CTF_F_COMPRESS;

  src = fp->ctf_buf;
  flipped = NULL;

  if (flip_endian)
    {
      if ((flipped = malloc (fp->ctf_size)) == NULL)
	{
	  ctf_set_errno (fp, ENOMEM);
	  ctf_err_warn (fp, 0, 0, _("ctf_write_mem: cannot allocate %li bytes"),
			(unsigned long) (fp->ctf_size + sizeof (struct ctf_header)));
	  return NULL;
	}
      ctf_flip_header (hp);
      memcpy (flipped, fp->ctf_buf, fp->ctf_size);
      if (ctf_flip (fp, fp->ctf_header, flipped, 1) < 0)
	{
	  free (buf);
	  free (flipped);
	  return NULL;				/* errno is set for us.  */
	}
      src = flipped;
    }

  if (uncompressed)
    {
      memcpy (bp, src, fp->ctf_size);
      *size += fp->ctf_size;
    }
  else
    {
      if ((rc = compress (bp, (uLongf *) &compress_len,
			  src, fp->ctf_size)) != Z_OK)
	{
	  ctf_set_errno (fp, ECTF_COMPRESS);
	  ctf_err_warn (fp, 0, 0, _("zlib deflate err: %s"), zError (rc));
	  free (buf);
	  return NULL;
	}
      *size += compress_len;
    }

  free (flipped);

  return buf;
}

/* Compress the specified CTF data stream and write it to the specified file
   descriptor.  */
int
ctf_compress_write (ctf_dict_t *fp, int fd)
{
  unsigned char *buf;
  unsigned char *bp;
  size_t tmp;
  ssize_t buf_len;
  ssize_t len;
  int err = 0;

  if ((buf = ctf_write_mem (fp, &tmp, 0)) == NULL)
    return -1;					/* errno is set for us.  */

  buf_len = tmp;
  bp = buf;

  while (buf_len > 0)
    {
      if ((len = write (fd, bp, buf_len)) < 0)
	{
	  err = ctf_set_errno (fp, errno);
	  ctf_err_warn (fp, 0, 0, _("ctf_compress_write: error writing"));
	  goto ret;
	}
      buf_len -= len;
      bp += len;
    }

ret:
  free (buf);
  return err;
}

/* Write the uncompressed CTF data stream to the specified file descriptor.  */
int
ctf_write (ctf_dict_t *fp, int fd)
{
  unsigned char *buf;
  unsigned char *bp;
  size_t tmp;
  ssize_t buf_len;
  ssize_t len;
  int err = 0;

  if ((buf = ctf_write_mem (fp, &tmp, (size_t) -1)) == NULL)
    return -1;					/* errno is set for us.  */

  buf_len = tmp;
  bp = buf;

  while (buf_len > 0)
    {
      if ((len = write (fd, bp, buf_len)) < 0)
	{
	  err = ctf_set_errno (fp, errno);
	  ctf_err_warn (fp, 0, 0, _("ctf_compress_write: error writing"));
	  goto ret;
	}
      buf_len -= len;
      bp += len;
    }

ret:
  free (buf);
  return err;
}
