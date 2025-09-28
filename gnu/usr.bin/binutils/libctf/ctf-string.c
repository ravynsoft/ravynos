/* CTF string table management.
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
#include <string.h>
#include <assert.h>

/* Convert an encoded CTF string name into a pointer to a C string, using an
  explicit internal strtab rather than the fp-based one.  */
const char *
ctf_strraw_explicit (ctf_dict_t *fp, uint32_t name, ctf_strs_t *strtab)
{
  ctf_strs_t *ctsp = &fp->ctf_str[CTF_NAME_STID (name)];

  if ((CTF_NAME_STID (name) == CTF_STRTAB_0) && (strtab != NULL))
    ctsp = strtab;

  /* If this name is in the external strtab, and there is a synthetic strtab,
     use it in preference.  */

  if (CTF_NAME_STID (name) == CTF_STRTAB_1
      && fp->ctf_syn_ext_strtab != NULL)
    return ctf_dynhash_lookup (fp->ctf_syn_ext_strtab,
			       (void *) (uintptr_t) name);

  /* If the name is in the internal strtab, and the offset is beyond the end of
     the ctsp->cts_len but below the ctf_str_prov_offset, this is a provisional
     string added by ctf_str_add*() but not yet built into a real strtab: get
     the value out of the ctf_prov_strtab.  */

  if (CTF_NAME_STID (name) == CTF_STRTAB_0
      && name >= ctsp->cts_len && name < fp->ctf_str_prov_offset)
      return ctf_dynhash_lookup (fp->ctf_prov_strtab,
				 (void *) (uintptr_t) name);

  if (ctsp->cts_strs != NULL && CTF_NAME_OFFSET (name) < ctsp->cts_len)
    return (ctsp->cts_strs + CTF_NAME_OFFSET (name));

  /* String table not loaded or corrupt offset.  */
  return NULL;
}

/* Convert an encoded CTF string name into a pointer to a C string by looking
  up the appropriate string table buffer and then adding the offset.  */
const char *
ctf_strraw (ctf_dict_t *fp, uint32_t name)
{
  return ctf_strraw_explicit (fp, name, NULL);
}

/* Return a guaranteed-non-NULL pointer to the string with the given CTF
   name.  */
const char *
ctf_strptr (ctf_dict_t *fp, uint32_t name)
{
  const char *s = ctf_strraw (fp, name);
  return (s != NULL ? s : "(?)");
}

/* Remove all refs to a given atom.  */
static void
ctf_str_purge_atom_refs (ctf_str_atom_t *atom)
{
  ctf_str_atom_ref_t *ref, *next;

  for (ref = ctf_list_next (&atom->csa_refs); ref != NULL; ref = next)
    {
      next = ctf_list_next (ref);
      ctf_list_delete (&atom->csa_refs, ref);
      free (ref);
    }
}

/* Free an atom (only called on ctf_close().)  */
static void
ctf_str_free_atom (void *a)
{
  ctf_str_atom_t *atom = a;

  ctf_str_purge_atom_refs (atom);
  free (atom);
}

/* Create the atoms table.  There is always at least one atom in it, the null
   string.  */
int
ctf_str_create_atoms (ctf_dict_t *fp)
{
  fp->ctf_str_atoms = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
					  free, ctf_str_free_atom);
  if (!fp->ctf_str_atoms)
    return -ENOMEM;

  if (!fp->ctf_prov_strtab)
    fp->ctf_prov_strtab = ctf_dynhash_create (ctf_hash_integer,
					      ctf_hash_eq_integer,
					      NULL, NULL);
  if (!fp->ctf_prov_strtab)
    goto oom_prov_strtab;

  if (!fp->ctf_str_pending_ref)
    fp->ctf_str_pending_ref = ctf_dynset_create (htab_hash_pointer,
						 htab_eq_pointer,
						 NULL);
  if (!fp->ctf_str_pending_ref)
    goto oom_str_pending_ref;

  errno = 0;
  ctf_str_add (fp, "");
  if (errno == ENOMEM)
    goto oom_str_add;

  return 0;

 oom_str_add:
  ctf_dynhash_destroy (fp->ctf_prov_strtab);
  fp->ctf_prov_strtab = NULL;
 oom_str_pending_ref:
  ctf_dynset_destroy (fp->ctf_str_pending_ref);
  fp->ctf_str_pending_ref = NULL;
 oom_prov_strtab:
  ctf_dynhash_destroy (fp->ctf_str_atoms);
  fp->ctf_str_atoms = NULL;
  return -ENOMEM;
}

/* Destroy the atoms table.  */
void
ctf_str_free_atoms (ctf_dict_t *fp)
{
  ctf_dynhash_destroy (fp->ctf_prov_strtab);
  ctf_dynhash_destroy (fp->ctf_str_atoms);
  ctf_dynset_destroy (fp->ctf_str_pending_ref);
}

#define CTF_STR_ADD_REF 0x1
#define CTF_STR_MAKE_PROVISIONAL 0x2
#define CTF_STR_PENDING_REF 0x4

/* Add a string to the atoms table, copying the passed-in string.  Return the
   atom added. Return NULL only when out of memory (and do not touch the
   passed-in string in that case).  Possibly augment the ref list with the
   passed-in ref.  Possibly add a provisional entry for this string to the
   provisional strtab.   */
static ctf_str_atom_t *
ctf_str_add_ref_internal (ctf_dict_t *fp, const char *str,
			  int flags, uint32_t *ref)
{
  char *newstr = NULL;
  ctf_str_atom_t *atom = NULL;
  ctf_str_atom_ref_t *aref = NULL;

  atom = ctf_dynhash_lookup (fp->ctf_str_atoms, str);

  if (flags & CTF_STR_ADD_REF)
    {
      if ((aref = malloc (sizeof (struct ctf_str_atom_ref))) == NULL)
	return NULL;
      aref->caf_ref = ref;
    }

  if (atom)
    {
      if (flags & CTF_STR_ADD_REF)
	{
	  ctf_dynset_remove (fp->ctf_str_pending_ref, (void *) ref);
	  ctf_list_append (&atom->csa_refs, aref);
	  fp->ctf_str_num_refs++;
	}
      return atom;
    }

  if ((atom = malloc (sizeof (struct ctf_str_atom))) == NULL)
    goto oom;
  memset (atom, 0, sizeof (struct ctf_str_atom));

  if ((newstr = strdup (str)) == NULL)
    goto oom;

  if (ctf_dynhash_insert (fp->ctf_str_atoms, newstr, atom) < 0)
    goto oom;

  atom->csa_str = newstr;
  atom->csa_snapshot_id = fp->ctf_snapshots;

  if (flags & CTF_STR_MAKE_PROVISIONAL)
    {
      atom->csa_offset = fp->ctf_str_prov_offset;

      if (ctf_dynhash_insert (fp->ctf_prov_strtab, (void *) (uintptr_t)
			      atom->csa_offset, (void *) atom->csa_str) < 0)
	goto oom;

      fp->ctf_str_prov_offset += strlen (atom->csa_str) + 1;
    }

  if (flags & CTF_STR_PENDING_REF)
    {
      if (ctf_dynset_insert (fp->ctf_str_pending_ref, (void *) ref) < 0)
	goto oom;
    }
  else if (flags & CTF_STR_ADD_REF)
    {
      ctf_dynset_remove (fp->ctf_str_pending_ref, (void *) ref);
      ctf_list_append (&atom->csa_refs, aref);
      fp->ctf_str_num_refs++;
    }
  return atom;

 oom:
  if (newstr)
    ctf_dynhash_remove (fp->ctf_str_atoms, newstr);
  free (atom);
  free (aref);
  free (newstr);
  ctf_set_errno (fp, ENOMEM);
  return NULL;
}

/* Add a string to the atoms table, without augmenting the ref list for this
   string: return a 'provisional offset' which can be used to return this string
   until ctf_str_write_strtab is called, or 0 on failure.  (Everywhere the
   provisional offset is assigned to should be added as a ref using
   ctf_str_add_ref() as well.) */
uint32_t
ctf_str_add (ctf_dict_t *fp, const char *str)
{
  ctf_str_atom_t *atom;

  if (!str)
    str = "";

  atom = ctf_str_add_ref_internal (fp, str, CTF_STR_MAKE_PROVISIONAL, 0);
  if (!atom)
    return 0;

  return atom->csa_offset;
}

/* Like ctf_str_add(), but additionally augment the atom's refs list with the
   passed-in ref, whether or not the string is already present.  There is no
   attempt to deduplicate the refs list (but duplicates are harmless).  */
uint32_t
ctf_str_add_ref (ctf_dict_t *fp, const char *str, uint32_t *ref)
{
  ctf_str_atom_t *atom;

  if (!str)
    str = "";

  atom = ctf_str_add_ref_internal (fp, str, CTF_STR_ADD_REF
				   | CTF_STR_MAKE_PROVISIONAL, ref);
  if (!atom)
    return 0;

  return atom->csa_offset;
}

/* Like ctf_str_add_ref(), but notes that this memory location must be added as
   a ref by a later serialization phase, rather than adding it itself.  */
uint32_t
ctf_str_add_pending (ctf_dict_t *fp, const char *str, uint32_t *ref)
{
  ctf_str_atom_t *atom;

  if (!str)
    str = "";

  atom = ctf_str_add_ref_internal (fp, str, CTF_STR_PENDING_REF
				   | CTF_STR_MAKE_PROVISIONAL, ref);
  if (!atom)
    return 0;

  return atom->csa_offset;
}

/* Note that a pending ref now located at NEW_REF has moved by BYTES bytes.  */
int
ctf_str_move_pending (ctf_dict_t *fp, uint32_t *new_ref, ptrdiff_t bytes)
{
  if (bytes == 0)
    return 0;

  if (ctf_dynset_insert (fp->ctf_str_pending_ref, (void *) new_ref) < 0)
    return (ctf_set_errno (fp, ENOMEM));

  ctf_dynset_remove (fp->ctf_str_pending_ref,
		     (void *) ((signed char *) new_ref - bytes));
  return 0;
}

/* Add an external strtab reference at OFFSET.  Returns zero if the addition
   failed, nonzero otherwise.  */
int
ctf_str_add_external (ctf_dict_t *fp, const char *str, uint32_t offset)
{
  ctf_str_atom_t *atom;

  if (!str)
    str = "";

  atom = ctf_str_add_ref_internal (fp, str, 0, 0);
  if (!atom)
    return 0;

  atom->csa_external_offset = CTF_SET_STID (offset, CTF_STRTAB_1);

  if (!fp->ctf_syn_ext_strtab)
    fp->ctf_syn_ext_strtab = ctf_dynhash_create (ctf_hash_integer,
						 ctf_hash_eq_integer,
						 NULL, NULL);
  if (!fp->ctf_syn_ext_strtab)
    {
      ctf_set_errno (fp, ENOMEM);
      return 0;
    }

  if (ctf_dynhash_insert (fp->ctf_syn_ext_strtab,
			  (void *) (uintptr_t)
			  atom->csa_external_offset,
			  (void *) atom->csa_str) < 0)
    {
      /* No need to bother freeing the syn_ext_strtab: it will get freed at
	 ctf_str_write_strtab time if unreferenced.  */
      ctf_set_errno (fp, ENOMEM);
      return 0;
    }

  return 1;
}

/* Remove a single ref.  */
void
ctf_str_remove_ref (ctf_dict_t *fp, const char *str, uint32_t *ref)
{
  ctf_str_atom_ref_t *aref, *anext;
  ctf_str_atom_t *atom = NULL;

  atom = ctf_dynhash_lookup (fp->ctf_str_atoms, str);
  if (!atom)
    return;

  for (aref = ctf_list_next (&atom->csa_refs); aref != NULL; aref = anext)
    {
      anext = ctf_list_next (aref);
      if (aref->caf_ref == ref)
	{
	  ctf_list_delete (&atom->csa_refs, aref);
	  free (aref);
	}
    }

  ctf_dynset_remove (fp->ctf_str_pending_ref, (void *) ref);
}

/* A ctf_dynhash_iter_remove() callback that removes atoms later than a given
   snapshot ID.  External atoms are never removed, because they came from the
   linker string table and are still present even if you roll back type
   additions.  */
static int
ctf_str_rollback_atom (void *key _libctf_unused_, void *value, void *arg)
{
  ctf_str_atom_t *atom = (ctf_str_atom_t *) value;
  ctf_snapshot_id_t *id = (ctf_snapshot_id_t *) arg;

  return (atom->csa_snapshot_id > id->snapshot_id)
    && (atom->csa_external_offset == 0);
}

/* Roll back, deleting all (internal) atoms created after a particular ID.  */
void
ctf_str_rollback (ctf_dict_t *fp, ctf_snapshot_id_t id)
{
  ctf_dynhash_iter_remove (fp->ctf_str_atoms, ctf_str_rollback_atom, &id);
}

/* An adaptor around ctf_purge_atom_refs.  */
static void
ctf_str_purge_one_atom_refs (void *key _libctf_unused_, void *value,
			     void *arg _libctf_unused_)
{
  ctf_str_atom_t *atom = (ctf_str_atom_t *) value;
  ctf_str_purge_atom_refs (atom);
}

/* Remove all the recorded refs from the atoms table.  */
void
ctf_str_purge_refs (ctf_dict_t *fp)
{
  if (fp->ctf_str_num_refs > 0)
    ctf_dynhash_iter (fp->ctf_str_atoms, ctf_str_purge_one_atom_refs, NULL);
  fp->ctf_str_num_refs = 0;
}

/* Update a list of refs to the specified value. */
static void
ctf_str_update_refs (ctf_str_atom_t *refs, uint32_t value)
{
  ctf_str_atom_ref_t *ref;

  for (ref = ctf_list_next (&refs->csa_refs); ref != NULL;
       ref = ctf_list_next (ref))
      *(ref->caf_ref) = value;
}

/* State shared across the strtab write process.  */
typedef struct ctf_strtab_write_state
{
  /* Strtab we are writing, and the number of strings in it.  */
  ctf_strs_writable_t *strtab;
  size_t strtab_count;

  /* Pointers to (existing) atoms in the atoms table, for qsorting.  */
  ctf_str_atom_t **sorttab;

  /* Loop counter for sorttab population.  */
  size_t i;

  /* The null-string atom (skipped during population).  */
  ctf_str_atom_t *nullstr;
} ctf_strtab_write_state_t;

/* Count the number of entries in the strtab, and its length.  */
static void
ctf_str_count_strtab (void *key _libctf_unused_, void *value,
	      void *arg)
{
  ctf_str_atom_t *atom = (ctf_str_atom_t *) value;
  ctf_strtab_write_state_t *s = (ctf_strtab_write_state_t *) arg;

  /* We only factor in the length of items that have no offset and have refs:
     other items are in the external strtab, or will simply not be written out
     at all.  They still contribute to the total count, though, because we still
     have to sort them.  We add in the null string's length explicitly, outside
     this function, since it is explicitly written out even if it has no refs at
     all.  */

  if (s->nullstr == atom)
    {
      s->strtab_count++;
      return;
    }

  if (!ctf_list_empty_p (&atom->csa_refs))
    {
      if (!atom->csa_external_offset)
	s->strtab->cts_len += strlen (atom->csa_str) + 1;
      s->strtab_count++;
    }
}

/* Populate the sorttab with pointers to the strtab atoms.  */
static void
ctf_str_populate_sorttab (void *key _libctf_unused_, void *value,
		  void *arg)
{
  ctf_str_atom_t *atom = (ctf_str_atom_t *) value;
  ctf_strtab_write_state_t *s = (ctf_strtab_write_state_t *) arg;

  /* Skip the null string.  */
  if (s->nullstr == atom)
    return;

  /* Skip atoms with no refs.  */
  if (!ctf_list_empty_p (&atom->csa_refs))
    s->sorttab[s->i++] = atom;
}

/* Sort the strtab.  */
static int
ctf_str_sort_strtab (const void *a, const void *b)
{
  ctf_str_atom_t **one = (ctf_str_atom_t **) a;
  ctf_str_atom_t **two = (ctf_str_atom_t **) b;

  return (strcmp ((*one)->csa_str, (*two)->csa_str));
}

/* Write out and return a strtab containing all strings with recorded refs,
   adjusting the refs to refer to the corresponding string.  The returned strtab
   may be NULL on error.  Also populate the synthetic strtab with mappings from
   external strtab offsets to names, so we can look them up with ctf_strptr().
   Only external strtab offsets with references are added.  */
ctf_strs_writable_t
ctf_str_write_strtab (ctf_dict_t *fp)
{
  ctf_strs_writable_t strtab;
  ctf_str_atom_t *nullstr;
  uint32_t cur_stroff = 0;
  ctf_strtab_write_state_t s;
  ctf_str_atom_t **sorttab;
  size_t i;
  int any_external = 0;

  memset (&strtab, 0, sizeof (struct ctf_strs_writable));
  memset (&s, 0, sizeof (struct ctf_strtab_write_state));
  s.strtab = &strtab;

  nullstr = ctf_dynhash_lookup (fp->ctf_str_atoms, "");
  if (!nullstr)
    {
      ctf_err_warn (fp, 0, ECTF_INTERNAL, _("null string not found in strtab"));
      strtab.cts_strs = NULL;
      return strtab;
    }

  s.nullstr = nullstr;
  ctf_dynhash_iter (fp->ctf_str_atoms, ctf_str_count_strtab, &s);
  strtab.cts_len++;				/* For the null string.  */

  ctf_dprintf ("%lu bytes of strings in strtab.\n",
	       (unsigned long) strtab.cts_len);

  /* Sort the strtab.  Force the null string to be first.  */
  sorttab = calloc (s.strtab_count, sizeof (ctf_str_atom_t *));
  if (!sorttab)
    goto oom;

  sorttab[0] = nullstr;
  s.i = 1;
  s.sorttab = sorttab;
  ctf_dynhash_iter (fp->ctf_str_atoms, ctf_str_populate_sorttab, &s);

  qsort (&sorttab[1], s.strtab_count - 1, sizeof (ctf_str_atom_t *),
	 ctf_str_sort_strtab);

  if ((strtab.cts_strs = malloc (strtab.cts_len)) == NULL)
    goto oom_sorttab;

  /* Update all refs: also update the strtab appropriately.  */
  for (i = 0; i < s.strtab_count; i++)
    {
      if (sorttab[i]->csa_external_offset)
	{
	  /* External strtab entry.  */

	  any_external = 1;
	  ctf_str_update_refs (sorttab[i], sorttab[i]->csa_external_offset);
	  sorttab[i]->csa_offset = sorttab[i]->csa_external_offset;
	}
      else
	{
	  /* Internal strtab entry with refs: actually add to the string
	     table.  */

	  ctf_str_update_refs (sorttab[i], cur_stroff);
	  sorttab[i]->csa_offset = cur_stroff;
	  strcpy (&strtab.cts_strs[cur_stroff], sorttab[i]->csa_str);
	  cur_stroff += strlen (sorttab[i]->csa_str) + 1;
	}
    }
  free (sorttab);

  if (!any_external)
    {
      ctf_dynhash_destroy (fp->ctf_syn_ext_strtab);
      fp->ctf_syn_ext_strtab = NULL;
    }

  /* All the provisional strtab entries are now real strtab entries, and
     ctf_strptr() will find them there.  The provisional offset now starts right
     beyond the new end of the strtab.  */

  ctf_dynhash_empty (fp->ctf_prov_strtab);
  fp->ctf_str_prov_offset = strtab.cts_len + 1;
  return strtab;

 oom_sorttab:
  free (sorttab);
 oom:
  return strtab;
}
