/* Symbol, variable and name lookup.
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
#include <elf.h>
#include <string.h>
#include <assert.h>

/* Grow the pptrtab so that it is at least NEW_LEN long.  */
static int
grow_pptrtab (ctf_dict_t *fp, size_t new_len)
{
  uint32_t *new_pptrtab;

  if ((new_pptrtab = realloc (fp->ctf_pptrtab, sizeof (uint32_t)
			      * new_len)) == NULL)
    return (ctf_set_errno (fp, ENOMEM));

  fp->ctf_pptrtab = new_pptrtab;

  memset (fp->ctf_pptrtab + fp->ctf_pptrtab_len, 0,
	  sizeof (uint32_t) * (new_len - fp->ctf_pptrtab_len));

  fp->ctf_pptrtab_len = new_len;
  return 0;
}

/* Update entries in the pptrtab that relate to types newly added in the
   child.  */
static int
refresh_pptrtab (ctf_dict_t *fp, ctf_dict_t *pfp)
{
  uint32_t i;
  for (i = fp->ctf_pptrtab_typemax; i <= fp->ctf_typemax; i++)
    {
      ctf_id_t type = LCTF_INDEX_TO_TYPE (fp, i, 1);
      ctf_id_t reffed_type;

      if (ctf_type_kind (fp, type) != CTF_K_POINTER)
	continue;

      reffed_type = ctf_type_reference (fp, type);

      if (LCTF_TYPE_ISPARENT (fp, reffed_type))
	{
	  uint32_t idx = LCTF_TYPE_TO_INDEX (fp, reffed_type);

	  /* Guard against references to invalid types.  No need to consider
	     the CTF dict corrupt in this case: this pointer just can't be a
	     pointer to any type we know about.  */
	  if (idx <= pfp->ctf_typemax)
	    {
	      if (idx >= fp->ctf_pptrtab_len
		  && grow_pptrtab (fp, pfp->ctf_ptrtab_len) < 0)
		return -1;			/* errno is set for us.  */

	      fp->ctf_pptrtab[idx] = i;
	    }
	}
    }

  fp->ctf_pptrtab_typemax = fp->ctf_typemax;

  return 0;
}

/* Compare the given input string and length against a table of known C storage
   qualifier keywords.  We just ignore these in ctf_lookup_by_name, below.  To
   do this quickly, we use a pre-computed Perfect Hash Function similar to the
   technique originally described in the classic paper:

   R.J. Cichelli, "Minimal Perfect Hash Functions Made Simple",
   Communications of the ACM, Volume 23, Issue 1, January 1980, pp. 17-19.

   For an input string S of length N, we use hash H = S[N - 1] + N - 105, which
   for the current set of qualifiers yields a unique H in the range [0 .. 20].
   The hash can be modified when the keyword set changes as necessary.  We also
   store the length of each keyword and check it prior to the final strcmp().

   TODO: just use gperf.  */

static int
isqualifier (const char *s, size_t len)
{
  static const struct qual
  {
    const char *q_name;
    size_t q_len;
  } qhash[] = {
    {"static", 6}, {"", 0}, {"", 0}, {"", 0},
    {"volatile", 8}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
    {"", 0}, {"auto", 4}, {"extern", 6}, {"", 0}, {"", 0},
    {"", 0}, {"", 0}, {"const", 5}, {"register", 8},
    {"", 0}, {"restrict", 8}, {"_Restrict", 9}
  };

  int h = s[len - 1] + (int) len - 105;
  const struct qual *qp;

  if (h < 0 || (size_t) h >= sizeof (qhash) / sizeof (qhash[0]))
    return 0;

  qp = &qhash[h];

  return ((size_t) len == qp->q_len &&
	  strncmp (qp->q_name, s, qp->q_len) == 0);
}

/* Attempt to convert the given C type name into the corresponding CTF type ID.
   It is not possible to do complete and proper conversion of type names
   without implementing a more full-fledged parser, which is necessary to
   handle things like types that are function pointers to functions that
   have arguments that are function pointers, and fun stuff like that.
   Instead, this function implements a very simple conversion algorithm that
   finds the things that we actually care about: structs, unions, enums,
   integers, floats, typedefs, and pointers to any of these named types.  */

static ctf_id_t
ctf_lookup_by_name_internal (ctf_dict_t *fp, ctf_dict_t *child,
			     const char *name)
{
  static const char delimiters[] = " \t\n\r\v\f*";

  const ctf_lookup_t *lp;
  const char *p, *q, *end;
  ctf_id_t type = 0;
  ctf_id_t ntype, ptype;

  if (name == NULL)
    return (ctf_set_errno (fp, EINVAL));

  for (p = name, end = name + strlen (name); *p != '\0'; p = q)
    {
      while (isspace ((int) *p))
	p++;			/* Skip leading whitespace.  */

      if (p == end)
	break;

      if ((q = strpbrk (p + 1, delimiters)) == NULL)
	q = end;		/* Compare until end.  */

      if (*p == '*')
	{
	  /* Find a pointer to type by looking in child->ctf_pptrtab (if child
	     is set) and fp->ctf_ptrtab.  If we can't find a pointer to the
	     given type, see if we can compute a pointer to the type resulting
	     from resolving the type down to its base type and use that instead.
	     This helps with cases where the CTF data includes "struct foo *"
	     but not "foo_t *" and the user tries to access "foo_t *" in the
	     debugger.

	     There is extra complexity here because uninitialized elements in
	     the pptrtab and ptrtab are set to zero, but zero (as the type ID
	     meaning the unimplemented type) is a valid return type from
	     ctf_lookup_by_name.  (Pointers to types are never of type 0, so
	     this is unambiguous, just fiddly to deal with.)  */

	  uint32_t idx = LCTF_TYPE_TO_INDEX (fp, type);
	  int in_child = 0;

	  ntype = CTF_ERR;
	  if (child && idx < child->ctf_pptrtab_len)
	    {
	      ntype = child->ctf_pptrtab[idx];
	      if (ntype)
		in_child = 1;
	      else
		ntype = CTF_ERR;
	    }

	  if (ntype == CTF_ERR)
	    {
	      ntype = fp->ctf_ptrtab[idx];
	      if (ntype == 0)
		ntype = CTF_ERR;
	    }

	  /* Try resolving to its base type and check again.  */
	  if (ntype == CTF_ERR)
	    {
	      if (child)
		ntype = ctf_type_resolve_unsliced (child, type);
	      else
		ntype = ctf_type_resolve_unsliced (fp, type);

	      if (ntype == CTF_ERR)
		goto notype;

	      idx = LCTF_TYPE_TO_INDEX (fp, ntype);

	      ntype = CTF_ERR;
	      if (child && idx < child->ctf_pptrtab_len)
		{
		  ntype = child->ctf_pptrtab[idx];
		  if (ntype)
		    in_child = 1;
		  else
		    ntype = CTF_ERR;
		}

	      if (ntype == CTF_ERR)
		{
		  ntype = fp->ctf_ptrtab[idx];
		  if (ntype == 0)
		    ntype = CTF_ERR;
		}
	      if (ntype == CTF_ERR)
		goto notype;
	    }

	  type = LCTF_INDEX_TO_TYPE (fp, ntype, (fp->ctf_flags & LCTF_CHILD)
				     || in_child);

	  /* We are looking up a type in the parent, but the pointed-to type is
	     in the child.  Switch to looking in the child: if we need to go
	     back into the parent, we can recurse again.  */
	  if (in_child)
	    {
	      fp = child;
	      child = NULL;
	    }

	  q = p + 1;
	  continue;
	}

      if (isqualifier (p, (size_t) (q - p)))
	continue;		/* Skip qualifier keyword.  */

      for (lp = fp->ctf_lookups; lp->ctl_prefix != NULL; lp++)
	{
	  /* TODO: This is not MT-safe.  */
	  if ((lp->ctl_prefix[0] == '\0' ||
	       strncmp (p, lp->ctl_prefix, (size_t) (q - p)) == 0) &&
	      (size_t) (q - p) >= lp->ctl_len)
	    {
	      for (p += lp->ctl_len; isspace ((int) *p); p++)
		continue;	/* Skip prefix and next whitespace.  */

	      if ((q = strchr (p, '*')) == NULL)
		q = end;	/* Compare until end.  */

	      while (isspace ((int) q[-1]))
		q--;		/* Exclude trailing whitespace.  */

	      /* Expand and/or allocate storage for a slice of the name, then
		 copy it in.  */

	      if (fp->ctf_tmp_typeslicelen >= (size_t) (q - p) + 1)
		{
		  memcpy (fp->ctf_tmp_typeslice, p, (size_t) (q - p));
		  fp->ctf_tmp_typeslice[(size_t) (q - p)] = '\0';
		}
	      else
		{
		  free (fp->ctf_tmp_typeslice);
		  fp->ctf_tmp_typeslice = xstrndup (p, (size_t) (q - p));
		  if (fp->ctf_tmp_typeslice == NULL)
		    {
		      ctf_set_errno (fp, ENOMEM);
		      return CTF_ERR;
		    }
		}

	      if ((type = ctf_lookup_by_rawhash (fp, lp->ctl_hash,
						 fp->ctf_tmp_typeslice)) == 0)
		goto notype;

	      break;
	    }
	}

      if (lp->ctl_prefix == NULL)
	goto notype;
    }

  if (*p != '\0' || type == 0)
    return (ctf_set_errno (fp, ECTF_SYNTAX));

  return type;

 notype:
  ctf_set_errno (fp, ECTF_NOTYPE);
  if (fp->ctf_parent != NULL)
    {
      /* Need to look up in the parent, from the child's perspective.
	 Make sure the pptrtab is up to date.  */

      if (fp->ctf_pptrtab_typemax < fp->ctf_typemax)
	{
	  if (refresh_pptrtab (fp, fp->ctf_parent) < 0)
	    return -1;			/* errno is set for us.  */
	}

      if ((ptype = ctf_lookup_by_name_internal (fp->ctf_parent, fp,
						name)) != CTF_ERR)
	return ptype;
      return (ctf_set_errno (fp, ctf_errno (fp->ctf_parent)));
    }

  return CTF_ERR;
}

ctf_id_t
ctf_lookup_by_name (ctf_dict_t *fp, const char *name)
{
  return ctf_lookup_by_name_internal (fp, NULL, name);
}

/* Return the pointer to the internal CTF type data corresponding to the
   given type ID.  If the ID is invalid, the function returns NULL.
   This function is not exported outside of the library.  */

const ctf_type_t *
ctf_lookup_by_id (ctf_dict_t **fpp, ctf_id_t type)
{
  ctf_dict_t *fp = *fpp;	/* Caller passes in starting CTF dict.  */
  ctf_id_t idx;

  if ((fp = ctf_get_dict (fp, type)) == NULL)
    {
      (void) ctf_set_errno (*fpp, ECTF_NOPARENT);
      return NULL;
    }

  /* If this dict is writable, check for a dynamic type.  */

  if (fp->ctf_flags & LCTF_RDWR)
    {
      ctf_dtdef_t *dtd;

      if ((dtd = ctf_dynamic_type (fp, type)) != NULL)
	{
	  *fpp = fp;
	  return &dtd->dtd_data;
	}
      (void) ctf_set_errno (*fpp, ECTF_BADID);
      return NULL;
    }

  /* Check for a type in the static portion.  */

  idx = LCTF_TYPE_TO_INDEX (fp, type);
  if (idx > 0 && (unsigned long) idx <= fp->ctf_typemax)
    {
      *fpp = fp;		/* Function returns ending CTF dict.  */
      return (LCTF_INDEX_TO_TYPEPTR (fp, idx));
    }

  (void) ctf_set_errno (*fpp, ECTF_BADID);
  return NULL;
}

typedef struct ctf_lookup_idx_key
{
  ctf_dict_t *clik_fp;
  const char *clik_name;
  uint32_t *clik_names;
} ctf_lookup_idx_key_t;

/* A bsearch function for variable names.  */

static int
ctf_lookup_var (const void *key_, const void *lookup_)
{
  const ctf_lookup_idx_key_t *key = key_;
  const ctf_varent_t *lookup = lookup_;

  return (strcmp (key->clik_name, ctf_strptr (key->clik_fp, lookup->ctv_name)));
}

/* Given a variable name, return the type of the variable with that name.  */

ctf_id_t
ctf_lookup_variable (ctf_dict_t *fp, const char *name)
{
  ctf_varent_t *ent;
  ctf_lookup_idx_key_t key = { fp, name, NULL };

  /* This array is sorted, so we can bsearch for it.  */

  ent = bsearch (&key, fp->ctf_vars, fp->ctf_nvars, sizeof (ctf_varent_t),
		 ctf_lookup_var);

  if (ent == NULL)
    {
      if (fp->ctf_parent != NULL)
        {
          ctf_id_t ptype;

          if ((ptype = ctf_lookup_variable (fp->ctf_parent, name)) != CTF_ERR)
            return ptype;
          return (ctf_set_errno (fp, ctf_errno (fp->ctf_parent)));
        }

      return (ctf_set_errno (fp, ECTF_NOTYPEDAT));
    }

  return ent->ctv_type;
}

typedef struct ctf_symidx_sort_arg_cb
{
  ctf_dict_t *fp;
  uint32_t *names;
} ctf_symidx_sort_arg_cb_t;

static int
sort_symidx_by_name (const void *one_, const void *two_, void *arg_)
{
  const uint32_t *one = one_;
  const uint32_t *two = two_;
  ctf_symidx_sort_arg_cb_t *arg = arg_;

  return (strcmp (ctf_strptr (arg->fp, arg->names[*one]),
		  ctf_strptr (arg->fp, arg->names[*two])));
}

/* Sort a symbol index section by name.  Takes a 1:1 mapping of names to the
   corresponding symbol table.  Returns a lexicographically sorted array of idx
   indexes (and thus, of indexes into the corresponding func info / data object
   section).  */

static uint32_t *
ctf_symidx_sort (ctf_dict_t *fp, uint32_t *idx, size_t *nidx,
			 size_t len)
{
  uint32_t *sorted;
  size_t i;

  if ((sorted = malloc (len)) == NULL)
    {
      ctf_set_errno (fp, ENOMEM);
      return NULL;
    }

  *nidx = len / sizeof (uint32_t);
  for (i = 0; i < *nidx; i++)
    sorted[i] = i;

  if (!(fp->ctf_header->cth_flags & CTF_F_IDXSORTED))
    {
      ctf_symidx_sort_arg_cb_t arg = { fp, idx };
      ctf_dprintf ("Index section unsorted: sorting.");
      ctf_qsort_r (sorted, *nidx, sizeof (uint32_t), sort_symidx_by_name, &arg);
      fp->ctf_header->cth_flags |= CTF_F_IDXSORTED;
    }

  return sorted;
}

/* Given a symbol index, return the name of that symbol from the table provided
   by ctf_link_shuffle_syms, or failing that from the secondary string table, or
   the null string.  */
static const char *
ctf_lookup_symbol_name (ctf_dict_t *fp, unsigned long symidx)
{
  const ctf_sect_t *sp = &fp->ctf_symtab;
  ctf_link_sym_t sym;
  int err;

  if (fp->ctf_dynsymidx)
    {
      err = EINVAL;
      if (symidx > fp->ctf_dynsymmax)
	goto try_parent;

      ctf_link_sym_t *symp = fp->ctf_dynsymidx[symidx];

      if (!symp)
	goto try_parent;

      return symp->st_name;
    }

  err = ECTF_NOSYMTAB;
  if (sp->cts_data == NULL)
    goto try_parent;

  if (symidx >= fp->ctf_nsyms)
    goto try_parent;

  switch (sp->cts_entsize)
    {
    case sizeof (Elf64_Sym):
      {
	const Elf64_Sym *symp = (Elf64_Sym *) sp->cts_data + symidx;
	ctf_elf64_to_link_sym (fp, &sym, symp, symidx);
      }
      break;
    case sizeof (Elf32_Sym):
      {
	const Elf32_Sym *symp = (Elf32_Sym *) sp->cts_data + symidx;
	ctf_elf32_to_link_sym (fp, &sym, symp, symidx);
      }
      break;
    default:
      ctf_set_errno (fp, ECTF_SYMTAB);
      return _CTF_NULLSTR;
    }

  assert (!sym.st_nameidx_set);

  return sym.st_name;

 try_parent:
  if (fp->ctf_parent)
    {
      const char *ret;
      ret = ctf_lookup_symbol_name (fp->ctf_parent, symidx);
      if (ret == NULL)
	ctf_set_errno (fp, ctf_errno (fp->ctf_parent));
      return ret;
    }
  else
    {
      ctf_set_errno (fp, err);
      return _CTF_NULLSTR;
    }
}

/* Given a symbol name, return the index of that symbol, or -1 on error or if
   not found.  */
static unsigned long
ctf_lookup_symbol_idx (ctf_dict_t *fp, const char *symname)
{
  const ctf_sect_t *sp = &fp->ctf_symtab;
  ctf_link_sym_t sym;
  void *known_idx;
  int err;
  ctf_dict_t *cache = fp;

  if (fp->ctf_dynsyms)
    {
      err = EINVAL;

      ctf_link_sym_t *symp;

      if ((symp = ctf_dynhash_lookup (fp->ctf_dynsyms, symname)) == NULL)
	goto try_parent;

      return symp->st_symidx;
    }

  err = ECTF_NOSYMTAB;
  if (sp->cts_data == NULL)
    goto try_parent;

  /* First, try a hash lookup to see if we have already spotted this symbol
     during a past iteration: create the hash first if need be.  The lifespan
     of the strings is equal to the lifespan of the cts_data, so we don't
     need to strdup them.  If this dict was opened as part of an archive,
     and this archive has designed a crossdict_cache to cache results that
     are the same across all dicts in an archive, use it.  */

  if (fp->ctf_archive && fp->ctf_archive->ctfi_crossdict_cache)
    cache = fp->ctf_archive->ctfi_crossdict_cache;

  if (!cache->ctf_symhash)
    if ((cache->ctf_symhash = ctf_dynhash_create (ctf_hash_string,
						  ctf_hash_eq_string,
						  NULL, NULL)) == NULL)
      goto oom;

  if (ctf_dynhash_lookup_kv (cache->ctf_symhash, symname, NULL, &known_idx))
    return (unsigned long) (uintptr_t) known_idx;

  /* Hash lookup unsuccessful: linear search, populating the hashtab for later
     lookups as we go.  */

  for (; cache->ctf_symhash_latest < sp->cts_size / sp->cts_entsize;
       cache->ctf_symhash_latest++)
    {
      switch (sp->cts_entsize)
	{
	case sizeof (Elf64_Sym):
	  {
	    Elf64_Sym *symp = (Elf64_Sym *) sp->cts_data;
	    ctf_elf64_to_link_sym (fp, &sym, &symp[cache->ctf_symhash_latest],
				   cache->ctf_symhash_latest);
	    if (!ctf_dynhash_lookup_kv (cache->ctf_symhash, sym.st_name,
					NULL, NULL))
	      if (ctf_dynhash_cinsert (cache->ctf_symhash, sym.st_name,
				       (const void *) (uintptr_t)
				       cache->ctf_symhash_latest) < 0)
		goto oom;
	    if (strcmp (sym.st_name, symname) == 0)
	      return cache->ctf_symhash_latest++;
	  }
	  break;
	case sizeof (Elf32_Sym):
	  {
	    Elf32_Sym *symp = (Elf32_Sym *) sp->cts_data;
	    ctf_elf32_to_link_sym (fp, &sym, &symp[cache->ctf_symhash_latest],
				   cache->ctf_symhash_latest);
	    if (!ctf_dynhash_lookup_kv (cache->ctf_symhash, sym.st_name,
					NULL, NULL))
	      if (ctf_dynhash_cinsert (cache->ctf_symhash, sym.st_name,
				       (const void *) (uintptr_t)
				       cache->ctf_symhash_latest) < 0)
		goto oom;
	    if (strcmp (sym.st_name, symname) == 0)
	      return cache->ctf_symhash_latest++;
	  }
	  break;
	default:
	  ctf_set_errno (fp, ECTF_SYMTAB);
	  return (unsigned long) -1;
	}
    }

  /* Searched everything, still not found.  */

  return (unsigned long) -1;

 try_parent:
  if (fp->ctf_parent)
    {
      unsigned long psym;

      if ((psym = ctf_lookup_symbol_idx (fp->ctf_parent, symname))
          != (unsigned long) -1)
        return psym;

      ctf_set_errno (fp, ctf_errno (fp->ctf_parent));
      return (unsigned long) -1;
    }
  else
    {
      ctf_set_errno (fp, err);
      return (unsigned long) -1;
    }
oom:
  ctf_set_errno (fp, ENOMEM);
  ctf_err_warn (fp, 0, ENOMEM, _("cannot allocate memory for symbol "
				 "lookup hashtab"));
  return (unsigned long) -1;

}

/* Iterate over all symbols with types: if FUNC, function symbols, otherwise,
   data symbols.  The name argument is not optional.  The return order is
   arbitrary, though is likely to be in symbol index or name order.  You can
   change the value of 'functions' in the middle of iteration over non-dynamic
   dicts, but doing so on dynamic dicts will fail.  (This is probably not very
   useful, but there is no reason to prohibit it.)  */

ctf_id_t
ctf_symbol_next (ctf_dict_t *fp, ctf_next_t **it, const char **name,
		 int functions)
{
  ctf_id_t sym = CTF_ERR;
  ctf_next_t *i = *it;
  int err;

  if (!i)
    {
      if ((i = ctf_next_create ()) == NULL)
	return ctf_set_errno (fp, ENOMEM);

      i->cu.ctn_fp = fp;
      i->ctn_iter_fun = (void (*) (void)) ctf_symbol_next;
      i->ctn_n = 0;
      *it = i;
    }

  if ((void (*) (void)) ctf_symbol_next != i->ctn_iter_fun)
    return (ctf_set_errno (fp, ECTF_NEXT_WRONGFUN));

  if (fp != i->cu.ctn_fp)
    return (ctf_set_errno (fp, ECTF_NEXT_WRONGFP));

  /* We intentionally use raw access, not ctf_lookup_by_symbol, to avoid
     incurring additional sorting cost for unsorted symtypetabs coming from the
     compiler, to allow ctf_symbol_next to work in the absence of a symtab, and
     finally because it's easier to work out what the name of each symbol is if
     we do that.  */

  if (fp->ctf_flags & LCTF_RDWR)
    {
      ctf_dynhash_t *dynh = functions ? fp->ctf_funchash : fp->ctf_objthash;
      void *dyn_name = NULL, *dyn_value = NULL;

      if (!dynh)
	{
	  ctf_next_destroy (i);
	  return (ctf_set_errno (fp, ECTF_NEXT_END));
	}

      err = ctf_dynhash_next (dynh, &i->ctn_next, &dyn_name, &dyn_value);
      /* This covers errors and also end-of-iteration.  */
      if (err != 0)
	{
	  ctf_next_destroy (i);
	  *it = NULL;
	  return ctf_set_errno (fp, err);
	}

      *name = dyn_name;
      sym = (ctf_id_t) (uintptr_t) dyn_value;
    }
  else if ((!functions && fp->ctf_objtidx_names) ||
	   (functions && fp->ctf_funcidx_names))
    {
      ctf_header_t *hp = fp->ctf_header;
      uint32_t *idx = functions ? fp->ctf_funcidx_names : fp->ctf_objtidx_names;
      uint32_t *tab;
      size_t len;

      if (functions)
	{
	  len = (hp->cth_varoff - hp->cth_funcidxoff) / sizeof (uint32_t);
	  tab = (uint32_t *) (fp->ctf_buf + hp->cth_funcoff);
	}
      else
	{
	  len = (hp->cth_funcidxoff - hp->cth_objtidxoff) / sizeof (uint32_t);
	  tab = (uint32_t *) (fp->ctf_buf + hp->cth_objtoff);
	}

      do
	{
	  if (i->ctn_n >= len)
	    goto end;

	  *name = ctf_strptr (fp, idx[i->ctn_n]);
	  sym = tab[i->ctn_n++];
	}
      while (sym == -1u || sym == 0);
    }
  else
    {
      /* Skip over pads in ctf_xslate, padding for typeless symbols in the
	 symtypetab itself, and symbols in the wrong table.  */
      for (; i->ctn_n < fp->ctf_nsyms; i->ctn_n++)
	{
	  ctf_header_t *hp = fp->ctf_header;

	  if (fp->ctf_sxlate[i->ctn_n] == -1u)
	    continue;

	  sym = *(uint32_t *) ((uintptr_t) fp->ctf_buf + fp->ctf_sxlate[i->ctn_n]);

	  if (sym == 0)
	    continue;

	  if (functions)
	    {
	      if (fp->ctf_sxlate[i->ctn_n] >= hp->cth_funcoff
		  && fp->ctf_sxlate[i->ctn_n] < hp->cth_objtidxoff)
		break;
	    }
	  else
	    {
	      if (fp->ctf_sxlate[i->ctn_n] >= hp->cth_objtoff
		  && fp->ctf_sxlate[i->ctn_n] < hp->cth_funcoff)
		break;
	    }
	}

      if (i->ctn_n >= fp->ctf_nsyms)
	goto end;

      *name = ctf_lookup_symbol_name (fp, i->ctn_n++);
    }

  return sym;

 end:
  ctf_next_destroy (i);
  *it = NULL;
  return (ctf_set_errno (fp, ECTF_NEXT_END));
}

/* A bsearch function for function and object index names.  */

static int
ctf_lookup_idx_name (const void *key_, const void *idx_)
{
  const ctf_lookup_idx_key_t *key = key_;
  const uint32_t *idx = idx_;

  return (strcmp (key->clik_name, ctf_strptr (key->clik_fp, key->clik_names[*idx])));
}

/* Given a symbol name or (failing that) number, look up that symbol in the
   function or object index table (which must exist).  Return 0 if not found
   there (or pad).  */

static ctf_id_t
ctf_try_lookup_indexed (ctf_dict_t *fp, unsigned long symidx,
			const char *symname, int is_function)
{
  struct ctf_header *hp = fp->ctf_header;
  uint32_t *symtypetab;
  uint32_t *names;
  uint32_t *sxlate;
  size_t nidx;

  if (symname == NULL)
    symname = ctf_lookup_symbol_name (fp, symidx);

  ctf_dprintf ("Looking up type of object with symtab idx %lx or name %s in "
	       "indexed symtypetab\n", symidx, symname);

  if (symname[0] == '\0')
    return -1;					/* errno is set for us.  */

  if (is_function)
    {
      if (!fp->ctf_funcidx_sxlate)
	{
	  if ((fp->ctf_funcidx_sxlate
	       = ctf_symidx_sort (fp, (uint32_t *)
				  (fp->ctf_buf + hp->cth_funcidxoff),
				  &fp->ctf_nfuncidx,
				  hp->cth_varoff - hp->cth_funcidxoff))
	      == NULL)
	    {
	      ctf_err_warn (fp, 0, 0, _("cannot sort function symidx"));
	      return -1;				/* errno is set for us.  */
	    }
	}
      symtypetab = (uint32_t *) (fp->ctf_buf + hp->cth_funcoff);
      sxlate = fp->ctf_funcidx_sxlate;
      names = fp->ctf_funcidx_names;
      nidx = fp->ctf_nfuncidx;
    }
  else
    {
      if (!fp->ctf_objtidx_sxlate)
	{
	  if ((fp->ctf_objtidx_sxlate
	       = ctf_symidx_sort (fp, (uint32_t *)
				  (fp->ctf_buf + hp->cth_objtidxoff),
				  &fp->ctf_nobjtidx,
				  hp->cth_funcidxoff - hp->cth_objtidxoff))
	      == NULL)
	    {
	      ctf_err_warn (fp, 0, 0, _("cannot sort object symidx"));
	      return -1;				/* errno is set for us. */
	    }
	}

      symtypetab = (uint32_t *) (fp->ctf_buf + hp->cth_objtoff);
      sxlate = fp->ctf_objtidx_sxlate;
      names = fp->ctf_objtidx_names;
      nidx = fp->ctf_nobjtidx;
    }

  ctf_lookup_idx_key_t key = { fp, symname, names };
  uint32_t *idx;

  idx = bsearch (&key, sxlate, nidx, sizeof (uint32_t), ctf_lookup_idx_name);

  if (!idx)
    {
      ctf_dprintf ("%s not found in idx\n", symname);
      return 0;
    }

  /* Should be impossible, but be paranoid.  */
  if ((idx - sxlate) > (ptrdiff_t) nidx)
    return (ctf_set_errno (fp, ECTF_CORRUPT));

  ctf_dprintf ("Symbol %lx (%s) is of type %x\n", symidx, symname,
	       symtypetab[*idx]);
  return symtypetab[*idx];
}

/* Given a symbol name or (if NULL) symbol index, return the type of the
   function or data object described by the corresponding entry in the symbol
   table.  We can only return symbols in read-only dicts and in dicts for which
   ctf_link_shuffle_syms has been called to assign symbol indexes to symbol
   names.  */

static ctf_id_t
ctf_lookup_by_sym_or_name (ctf_dict_t *fp, unsigned long symidx,
			   const char *symname)
{
  const ctf_sect_t *sp = &fp->ctf_symtab;
  ctf_id_t type = 0;
  int err = 0;

  /* Shuffled dynsymidx present?  Use that.  */
  if (fp->ctf_dynsymidx)
    {
      const ctf_link_sym_t *sym;

      if (symname)
	ctf_dprintf ("Looking up type of object with symname %s in "
		     "writable dict symtypetab\n", symname);
      else
	ctf_dprintf ("Looking up type of object with symtab idx %lx in "
		     "writable dict symtypetab\n", symidx);

      /* The dict must be dynamic.  */
      if (!ctf_assert (fp, fp->ctf_flags & LCTF_RDWR))
	return CTF_ERR;

      /* No name? Need to look it up.  */
      if (!symname)
	{
	  err = EINVAL;
	  if (symidx > fp->ctf_dynsymmax)
	    goto try_parent;

	  sym = fp->ctf_dynsymidx[symidx];
	  err = ECTF_NOTYPEDAT;
	  if (!sym || (sym->st_shndx != STT_OBJECT && sym->st_shndx != STT_FUNC))
	    goto try_parent;

	  if (!ctf_assert (fp, !sym->st_nameidx_set))
	    return CTF_ERR;
	  symname = sym->st_name;
     }

      if (fp->ctf_objthash == NULL
	  || ((type = (ctf_id_t) (uintptr_t)
	       ctf_dynhash_lookup (fp->ctf_objthash, symname)) == 0))
	{
	  if (fp->ctf_funchash == NULL
	      || ((type = (ctf_id_t) (uintptr_t)
		   ctf_dynhash_lookup (fp->ctf_funchash, symname)) == 0))
	    goto try_parent;
	}

      return type;
    }

  /* Lookup by name in a dynamic dict: just do it directly.  */
  if (symname && fp->ctf_flags & LCTF_RDWR)
    {
      if (fp->ctf_objthash == NULL
	  || ((type = (ctf_id_t) (uintptr_t)
	       ctf_dynhash_lookup (fp->ctf_objthash, symname)) == 0))
	{
	  if (fp->ctf_funchash == NULL
	      || ((type = (ctf_id_t) (uintptr_t)
		   ctf_dynhash_lookup (fp->ctf_funchash, symname)) == 0))
	    goto try_parent;
	}
      return type;
    }

  err = ECTF_NOSYMTAB;
  if (sp->cts_data == NULL)
    goto try_parent;

  /* This covers both out-of-range lookups and a dynamic dict which hasn't been
     shuffled yet.  */
  err = EINVAL;
  if (symname == NULL && symidx >= fp->ctf_nsyms)
    goto try_parent;

  if (fp->ctf_objtidx_names)
    {
      if ((type = ctf_try_lookup_indexed (fp, symidx, symname, 0)) == CTF_ERR)
	return CTF_ERR;				/* errno is set for us.  */
    }
  if (type == 0 && fp->ctf_funcidx_names)
    {
      if ((type = ctf_try_lookup_indexed (fp, symidx, symname, 1)) == CTF_ERR)
	return CTF_ERR;				/* errno is set for us.  */
    }
  if (type != 0)
    return type;

  err = ECTF_NOTYPEDAT;
  if (fp->ctf_objtidx_names && fp->ctf_funcidx_names)
    goto try_parent;

  /* Table must be nonindexed.  */

  ctf_dprintf ("Looking up object type %lx in 1:1 dict symtypetab\n", symidx);

  if (symname != NULL)
    if ((symidx = ctf_lookup_symbol_idx (fp, symname)) == (unsigned long) -1)
      goto try_parent;

  if (fp->ctf_sxlate[symidx] == -1u)
    goto try_parent;

  type = *(uint32_t *) ((uintptr_t) fp->ctf_buf + fp->ctf_sxlate[symidx]);

  if (type == 0)
    goto try_parent;

  return type;
 try_parent:
  if (fp->ctf_parent)
    {
      ctf_id_t ret = ctf_lookup_by_sym_or_name (fp->ctf_parent, symidx,
						symname);
      if (ret == CTF_ERR)
	ctf_set_errno (fp, ctf_errno (fp->ctf_parent));
      return ret;
    }
  else
    return (ctf_set_errno (fp, err));
}

/* Given a symbol table index, return the type of the function or data object
   described by the corresponding entry in the symbol table.  */
ctf_id_t
ctf_lookup_by_symbol (ctf_dict_t *fp, unsigned long symidx)
{
  return ctf_lookup_by_sym_or_name (fp, symidx, NULL);
}

/* Given a symbol name, return the type of the function or data object described
   by the corresponding entry in the symbol table.  */
ctf_id_t
ctf_lookup_by_symbol_name (ctf_dict_t *fp, const char *symname)
{
  return ctf_lookup_by_sym_or_name (fp, 0, symname);
}

/* Given a symbol table index, return the info for the function described
   by the corresponding entry in the symbol table, which may be a function
   symbol or may be a data symbol that happens to be a function pointer.  */

int
ctf_func_info (ctf_dict_t *fp, unsigned long symidx, ctf_funcinfo_t *fip)
{
  ctf_id_t type;

  if ((type = ctf_lookup_by_symbol (fp, symidx)) == CTF_ERR)
    return -1;					/* errno is set for us.  */

  if (ctf_type_kind (fp, type) != CTF_K_FUNCTION)
    return (ctf_set_errno (fp, ECTF_NOTFUNC));

  return ctf_func_type_info (fp, type, fip);
}

/* Given a symbol table index, return the arguments for the function described
   by the corresponding entry in the symbol table.  */

int
ctf_func_args (ctf_dict_t *fp, unsigned long symidx, uint32_t argc,
	       ctf_id_t *argv)
{
  ctf_id_t type;

  if ((type = ctf_lookup_by_symbol (fp, symidx)) == CTF_ERR)
    return -1;					/* errno is set for us.  */

  if (ctf_type_kind (fp, type) != CTF_K_FUNCTION)
    return (ctf_set_errno (fp, ECTF_NOTFUNC));

  return ctf_func_type_args (fp, type, argc, argv);
}
