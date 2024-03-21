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
#include <sys/param.h>
#include <string.h>
#include <unistd.h>

#ifndef EOVERFLOW
#define EOVERFLOW ERANGE
#endif

#ifndef roundup
#define roundup(x, y)  ((((x) + ((y) - 1)) / (y)) * (y))
#endif

/* The initial size of a dynamic type's vlen in members.  Arbitrary: the bigger
   this is, the less allocation needs to be done for small structure
   initialization, and the more memory is wasted for small structures during CTF
   construction.  No effect on generated CTF or ctf_open()ed CTF. */
#define INITIAL_VLEN 16

/* Make sure the ptrtab has enough space for at least one more type.

   We start with 4KiB of ptrtab, enough for a thousand types, then grow it 25%
   at a time.  */

static int
ctf_grow_ptrtab (ctf_dict_t *fp)
{
  size_t new_ptrtab_len = fp->ctf_ptrtab_len;

  /* We allocate one more ptrtab entry than we need, for the initial zero,
     plus one because the caller will probably allocate a new type.  */

  if (fp->ctf_ptrtab == NULL)
    new_ptrtab_len = 1024;
  else if ((fp->ctf_typemax + 2) > fp->ctf_ptrtab_len)
    new_ptrtab_len = fp->ctf_ptrtab_len * 1.25;

  if (new_ptrtab_len != fp->ctf_ptrtab_len)
    {
      uint32_t *new_ptrtab;

      if ((new_ptrtab = realloc (fp->ctf_ptrtab,
				 new_ptrtab_len * sizeof (uint32_t))) == NULL)
	return (ctf_set_errno (fp, ENOMEM));

      fp->ctf_ptrtab = new_ptrtab;
      memset (fp->ctf_ptrtab + fp->ctf_ptrtab_len, 0,
	      (new_ptrtab_len - fp->ctf_ptrtab_len) * sizeof (uint32_t));
      fp->ctf_ptrtab_len = new_ptrtab_len;
    }
  return 0;
}

/* Make sure a vlen has enough space: expand it otherwise.  Unlike the ptrtab,
   which grows quite slowly, the vlen grows in big jumps because it is quite
   expensive to expand: the caller has to scan the old vlen for string refs
   first and remove them, then re-add them afterwards.  The initial size is
   more or less arbitrary.  */
static int
ctf_grow_vlen (ctf_dict_t *fp, ctf_dtdef_t *dtd, size_t vlen)
{
  unsigned char *old = dtd->dtd_vlen;

  if (dtd->dtd_vlen_alloc > vlen)
    return 0;

  if ((dtd->dtd_vlen = realloc (dtd->dtd_vlen,
				dtd->dtd_vlen_alloc * 2)) == NULL)
    {
      dtd->dtd_vlen = old;
      return (ctf_set_errno (fp, ENOMEM));
    }
  memset (dtd->dtd_vlen + dtd->dtd_vlen_alloc, 0, dtd->dtd_vlen_alloc);
  dtd->dtd_vlen_alloc *= 2;
  return 0;
}

/* To create an empty CTF dict, we just declare a zeroed header and call
   ctf_bufopen() on it.  If ctf_bufopen succeeds, we mark the new dict r/w and
   initialize the dynamic members.  We start assigning type IDs at 1 because
   type ID 0 is used as a sentinel and a not-found indicator.  */

ctf_dict_t *
ctf_create (int *errp)
{
  static const ctf_header_t hdr = { .cth_preamble = { CTF_MAGIC, CTF_VERSION, 0 } };

  ctf_dynhash_t *dthash;
  ctf_dynhash_t *dvhash;
  ctf_dynhash_t *structs = NULL, *unions = NULL, *enums = NULL, *names = NULL;
  ctf_dynhash_t *objthash = NULL, *funchash = NULL;
  ctf_sect_t cts;
  ctf_dict_t *fp;

  libctf_init_debug();
  dthash = ctf_dynhash_create (ctf_hash_integer, ctf_hash_eq_integer,
			       NULL, NULL);
  if (dthash == NULL)
    {
      ctf_set_open_errno (errp, EAGAIN);
      goto err;
    }

  dvhash = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
			       NULL, NULL);
  if (dvhash == NULL)
    {
      ctf_set_open_errno (errp, EAGAIN);
      goto err_dt;
    }

  structs = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
				NULL, NULL);
  unions = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
			       NULL, NULL);
  enums = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
			      NULL, NULL);
  names = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
			      NULL, NULL);
  objthash = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
				 free, NULL);
  funchash = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
				 free, NULL);
  if (!structs || !unions || !enums || !names)
    {
      ctf_set_open_errno (errp, EAGAIN);
      goto err_dv;
    }

  cts.cts_name = _CTF_SECTION;
  cts.cts_data = &hdr;
  cts.cts_size = sizeof (hdr);
  cts.cts_entsize = 1;

  if ((fp = ctf_bufopen_internal (&cts, NULL, NULL, NULL, 1, errp)) == NULL)
    goto err_dv;

  fp->ctf_structs.ctn_writable = structs;
  fp->ctf_unions.ctn_writable = unions;
  fp->ctf_enums.ctn_writable = enums;
  fp->ctf_names.ctn_writable = names;
  fp->ctf_objthash = objthash;
  fp->ctf_funchash = funchash;
  fp->ctf_dthash = dthash;
  fp->ctf_dvhash = dvhash;
  fp->ctf_dtoldid = 0;
  fp->ctf_snapshots = 1;
  fp->ctf_snapshot_lu = 0;
  fp->ctf_flags |= LCTF_DIRTY;

  ctf_set_ctl_hashes (fp);
  ctf_setmodel (fp, CTF_MODEL_NATIVE);
  if (ctf_grow_ptrtab (fp) < 0)
    {
      ctf_set_open_errno (errp, ctf_errno (fp));
      ctf_dict_close (fp);
      return NULL;
    }

  return fp;

 err_dv:
  ctf_dynhash_destroy (structs);
  ctf_dynhash_destroy (unions);
  ctf_dynhash_destroy (enums);
  ctf_dynhash_destroy (names);
  ctf_dynhash_destroy (objthash);
  ctf_dynhash_destroy (funchash);
  ctf_dynhash_destroy (dvhash);
 err_dt:
  ctf_dynhash_destroy (dthash);
 err:
  return NULL;
}

/* Compatibility: just update the threshold for ctf_discard.  */
int
ctf_update (ctf_dict_t *fp)
{
  if (!(fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (fp, ECTF_RDONLY));

  fp->ctf_dtoldid = fp->ctf_typemax;
  return 0;
}

ctf_names_t *
ctf_name_table (ctf_dict_t *fp, int kind)
{
  switch (kind)
    {
    case CTF_K_STRUCT:
      return &fp->ctf_structs;
    case CTF_K_UNION:
      return &fp->ctf_unions;
    case CTF_K_ENUM:
      return &fp->ctf_enums;
    default:
      return &fp->ctf_names;
    }
}

int
ctf_dtd_insert (ctf_dict_t *fp, ctf_dtdef_t *dtd, int flag, int kind)
{
  const char *name;
  if (ctf_dynhash_insert (fp->ctf_dthash, (void *) (uintptr_t) dtd->dtd_type,
			  dtd) < 0)
    {
      ctf_set_errno (fp, ENOMEM);
      return -1;
    }

  if (flag == CTF_ADD_ROOT && dtd->dtd_data.ctt_name
      && (name = ctf_strraw (fp, dtd->dtd_data.ctt_name)) != NULL)
    {
      if (ctf_dynhash_insert (ctf_name_table (fp, kind)->ctn_writable,
			      (char *) name, (void *) (uintptr_t)
			      dtd->dtd_type) < 0)
	{
	  ctf_dynhash_remove (fp->ctf_dthash, (void *) (uintptr_t)
			      dtd->dtd_type);
	  ctf_set_errno (fp, ENOMEM);
	  return -1;
	}
    }
  ctf_list_append (&fp->ctf_dtdefs, dtd);
  return 0;
}

void
ctf_dtd_delete (ctf_dict_t *fp, ctf_dtdef_t *dtd)
{
  int kind = LCTF_INFO_KIND (fp, dtd->dtd_data.ctt_info);
  size_t vlen = LCTF_INFO_VLEN (fp, dtd->dtd_data.ctt_info);
  int name_kind = kind;
  const char *name;

  ctf_dynhash_remove (fp->ctf_dthash, (void *) (uintptr_t) dtd->dtd_type);

  switch (kind)
    {
    case CTF_K_STRUCT:
    case CTF_K_UNION:
      {
	ctf_lmember_t *memb = (ctf_lmember_t *) dtd->dtd_vlen;
	size_t i;

	for (i = 0; i < vlen; i++)
	  ctf_str_remove_ref (fp, ctf_strraw (fp, memb[i].ctlm_name),
			      &memb[i].ctlm_name);
      }
      break;
    case CTF_K_ENUM:
      {
	ctf_enum_t *en = (ctf_enum_t *) dtd->dtd_vlen;
	size_t i;

	for (i = 0; i < vlen; i++)
	  ctf_str_remove_ref (fp, ctf_strraw (fp, en[i].cte_name),
			      &en[i].cte_name);
      }
      break;
    case CTF_K_FORWARD:
      name_kind = dtd->dtd_data.ctt_type;
      break;
    }
  free (dtd->dtd_vlen);
  dtd->dtd_vlen_alloc = 0;

  if (dtd->dtd_data.ctt_name
      && (name = ctf_strraw (fp, dtd->dtd_data.ctt_name)) != NULL
      && LCTF_INFO_ISROOT (fp, dtd->dtd_data.ctt_info))
    {
      ctf_dynhash_remove (ctf_name_table (fp, name_kind)->ctn_writable,
			  name);
      ctf_str_remove_ref (fp, name, &dtd->dtd_data.ctt_name);
    }

  ctf_list_delete (&fp->ctf_dtdefs, dtd);
  free (dtd);
}

ctf_dtdef_t *
ctf_dtd_lookup (const ctf_dict_t *fp, ctf_id_t type)
{
  return (ctf_dtdef_t *)
    ctf_dynhash_lookup (fp->ctf_dthash, (void *) (uintptr_t) type);
}

ctf_dtdef_t *
ctf_dynamic_type (const ctf_dict_t *fp, ctf_id_t id)
{
  ctf_id_t idx;

  if (!(fp->ctf_flags & LCTF_RDWR))
    return NULL;

  if ((fp->ctf_flags & LCTF_CHILD) && LCTF_TYPE_ISPARENT (fp, id))
    fp = fp->ctf_parent;

  idx = LCTF_TYPE_TO_INDEX(fp, id);

  if ((unsigned long) idx <= fp->ctf_typemax)
    return ctf_dtd_lookup (fp, id);
  return NULL;
}

int
ctf_dvd_insert (ctf_dict_t *fp, ctf_dvdef_t *dvd)
{
  if (ctf_dynhash_insert (fp->ctf_dvhash, dvd->dvd_name, dvd) < 0)
    {
      ctf_set_errno (fp, ENOMEM);
      return -1;
    }
  ctf_list_append (&fp->ctf_dvdefs, dvd);
  return 0;
}

void
ctf_dvd_delete (ctf_dict_t *fp, ctf_dvdef_t *dvd)
{
  ctf_dynhash_remove (fp->ctf_dvhash, dvd->dvd_name);
  free (dvd->dvd_name);

  ctf_list_delete (&fp->ctf_dvdefs, dvd);
  free (dvd);
}

ctf_dvdef_t *
ctf_dvd_lookup (const ctf_dict_t *fp, const char *name)
{
  return (ctf_dvdef_t *) ctf_dynhash_lookup (fp->ctf_dvhash, name);
}

/* Discard all of the dynamic type definitions and variable definitions that
   have been added to the dict since the last call to ctf_update().  We locate
   such types by scanning the dtd list and deleting elements that have type IDs
   greater than ctf_dtoldid, which is set by ctf_update(), above, and by
   scanning the variable list and deleting elements that have update IDs equal
   to the current value of the last-update snapshot count (indicating that they
   were added after the most recent call to ctf_update()).  */
int
ctf_discard (ctf_dict_t *fp)
{
  ctf_snapshot_id_t last_update =
    { fp->ctf_dtoldid,
      fp->ctf_snapshot_lu + 1 };

  /* Update required?  */
  if (!(fp->ctf_flags & LCTF_DIRTY))
    return 0;

  return (ctf_rollback (fp, last_update));
}

ctf_snapshot_id_t
ctf_snapshot (ctf_dict_t *fp)
{
  ctf_snapshot_id_t snapid;
  snapid.dtd_id = fp->ctf_typemax;
  snapid.snapshot_id = fp->ctf_snapshots++;
  return snapid;
}

/* Like ctf_discard(), only discards everything after a particular ID.  */
int
ctf_rollback (ctf_dict_t *fp, ctf_snapshot_id_t id)
{
  ctf_dtdef_t *dtd, *ntd;
  ctf_dvdef_t *dvd, *nvd;

  if (!(fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (fp, ECTF_RDONLY));

  if (fp->ctf_snapshot_lu >= id.snapshot_id)
    return (ctf_set_errno (fp, ECTF_OVERROLLBACK));

  for (dtd = ctf_list_next (&fp->ctf_dtdefs); dtd != NULL; dtd = ntd)
    {
      int kind;
      const char *name;

      ntd = ctf_list_next (dtd);

      if (LCTF_TYPE_TO_INDEX (fp, dtd->dtd_type) <= id.dtd_id)
	continue;

      kind = LCTF_INFO_KIND (fp, dtd->dtd_data.ctt_info);
      if (kind == CTF_K_FORWARD)
	kind = dtd->dtd_data.ctt_type;

      if (dtd->dtd_data.ctt_name
	  && (name = ctf_strraw (fp, dtd->dtd_data.ctt_name)) != NULL
	  && LCTF_INFO_ISROOT (fp, dtd->dtd_data.ctt_info))
	{
	  ctf_dynhash_remove (ctf_name_table (fp, kind)->ctn_writable,
			      name);
	  ctf_str_remove_ref (fp, name, &dtd->dtd_data.ctt_name);
	}

      ctf_dynhash_remove (fp->ctf_dthash, (void *) (uintptr_t) dtd->dtd_type);
      ctf_dtd_delete (fp, dtd);
    }

  for (dvd = ctf_list_next (&fp->ctf_dvdefs); dvd != NULL; dvd = nvd)
    {
      nvd = ctf_list_next (dvd);

      if (dvd->dvd_snapshots <= id.snapshot_id)
	continue;

      ctf_dvd_delete (fp, dvd);
    }

  fp->ctf_typemax = id.dtd_id;
  fp->ctf_snapshots = id.snapshot_id;

  if (fp->ctf_snapshots == fp->ctf_snapshot_lu)
    fp->ctf_flags &= ~LCTF_DIRTY;

  return 0;
}

/* Note: vlen is the amount of space *allocated* for the vlen.  It may well not
   be the amount of space used (yet): the space used is declared in per-kind
   fashion in the dtd_data's info word.  */
static ctf_id_t
ctf_add_generic (ctf_dict_t *fp, uint32_t flag, const char *name, int kind,
		 size_t vlen, ctf_dtdef_t **rp)
{
  ctf_dtdef_t *dtd;
  ctf_id_t type;

  if (flag != CTF_ADD_NONROOT && flag != CTF_ADD_ROOT)
    return (ctf_set_errno (fp, EINVAL));

  if (!(fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (fp, ECTF_RDONLY));

  if (LCTF_INDEX_TO_TYPE (fp, fp->ctf_typemax, 1) >= CTF_MAX_TYPE)
    return (ctf_set_errno (fp, ECTF_FULL));

  if (LCTF_INDEX_TO_TYPE (fp, fp->ctf_typemax, 1) == (CTF_MAX_PTYPE - 1))
    return (ctf_set_errno (fp, ECTF_FULL));

  /* Make sure ptrtab always grows to be big enough for all types.  */
  if (ctf_grow_ptrtab (fp) < 0)
      return CTF_ERR;				/* errno is set for us. */

  if ((dtd = calloc (1, sizeof (ctf_dtdef_t))) == NULL)
    return (ctf_set_errno (fp, EAGAIN));

  dtd->dtd_vlen_alloc = vlen;
  if (vlen > 0)
    {
      if ((dtd->dtd_vlen = calloc (1, vlen)) == NULL)
	goto oom;
    }
  else
    dtd->dtd_vlen = NULL;

  type = ++fp->ctf_typemax;
  type = LCTF_INDEX_TO_TYPE (fp, type, (fp->ctf_flags & LCTF_CHILD));

  dtd->dtd_data.ctt_name = ctf_str_add_pending (fp, name,
						&dtd->dtd_data.ctt_name);
  dtd->dtd_type = type;

  if (dtd->dtd_data.ctt_name == 0 && name != NULL && name[0] != '\0')
    goto oom;

  if (ctf_dtd_insert (fp, dtd, flag, kind) < 0)
    goto err;					/* errno is set for us.  */

  fp->ctf_flags |= LCTF_DIRTY;

  *rp = dtd;
  return type;

 oom:
  ctf_set_errno (fp, EAGAIN);
 err:
  free (dtd->dtd_vlen);
  free (dtd);
  return CTF_ERR;
}

/* When encoding integer sizes, we want to convert a byte count in the range
   1-8 to the closest power of 2 (e.g. 3->4, 5->8, etc).  The clp2() function
   is a clever implementation from "Hacker's Delight" by Henry Warren, Jr.  */
static size_t
clp2 (size_t x)
{
  x--;

  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);

  return (x + 1);
}

ctf_id_t
ctf_add_encoded (ctf_dict_t *fp, uint32_t flag,
		 const char *name, const ctf_encoding_t *ep, uint32_t kind)
{
  ctf_dtdef_t *dtd;
  ctf_id_t type;
  uint32_t encoding;

  if (ep == NULL)
    return (ctf_set_errno (fp, EINVAL));

  if (name == NULL || name[0] == '\0')
    return (ctf_set_errno (fp, ECTF_NONAME));

  if (!ctf_assert (fp, kind == CTF_K_INTEGER || kind == CTF_K_FLOAT))
    return -1;					/* errno is set for us.  */

  if ((type = ctf_add_generic (fp, flag, name, kind, sizeof (uint32_t),
			       &dtd)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us.  */

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (kind, flag, 0);
  dtd->dtd_data.ctt_size = clp2 (P2ROUNDUP (ep->cte_bits, CHAR_BIT)
				 / CHAR_BIT);
  switch (kind)
    {
    case CTF_K_INTEGER:
      encoding = CTF_INT_DATA (ep->cte_format, ep->cte_offset, ep->cte_bits);
      break;
    case CTF_K_FLOAT:
      encoding = CTF_FP_DATA (ep->cte_format, ep->cte_offset, ep->cte_bits);
      break;
    }
  memcpy (dtd->dtd_vlen, &encoding, sizeof (encoding));

  return type;
}

ctf_id_t
ctf_add_reftype (ctf_dict_t *fp, uint32_t flag, ctf_id_t ref, uint32_t kind)
{
  ctf_dtdef_t *dtd;
  ctf_id_t type;
  ctf_dict_t *tmp = fp;
  int child = fp->ctf_flags & LCTF_CHILD;

  if (ref == CTF_ERR || ref > CTF_MAX_TYPE)
    return (ctf_set_errno (fp, EINVAL));

  if (ref != 0 && ctf_lookup_by_id (&tmp, ref) == NULL)
    return CTF_ERR;		/* errno is set for us.  */

  if ((type = ctf_add_generic (fp, flag, NULL, kind, 0, &dtd)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us.  */

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (kind, flag, 0);
  dtd->dtd_data.ctt_type = (uint32_t) ref;

  if (kind != CTF_K_POINTER)
    return type;

  /* If we are adding a pointer, update the ptrtab, pointing at this type from
     the type it points to.  Note that ctf_typemax is at this point one higher
     than we want to check against, because it's just been incremented for the
     addition of this type.  The pptrtab is lazily-updated as needed, so is not
     touched here.  */

  uint32_t type_idx = LCTF_TYPE_TO_INDEX (fp, type);
  uint32_t ref_idx = LCTF_TYPE_TO_INDEX (fp, ref);

  if (LCTF_TYPE_ISCHILD (fp, ref) == child
      && ref_idx < fp->ctf_typemax)
    fp->ctf_ptrtab[ref_idx] = type_idx;

  return type;
}

ctf_id_t
ctf_add_slice (ctf_dict_t *fp, uint32_t flag, ctf_id_t ref,
	       const ctf_encoding_t *ep)
{
  ctf_dtdef_t *dtd;
  ctf_slice_t slice;
  ctf_id_t resolved_ref = ref;
  ctf_id_t type;
  int kind;
  const ctf_type_t *tp;
  ctf_dict_t *tmp = fp;

  if (ep == NULL)
    return (ctf_set_errno (fp, EINVAL));

  if ((ep->cte_bits > 255) || (ep->cte_offset > 255))
    return (ctf_set_errno (fp, ECTF_SLICEOVERFLOW));

  if (ref == CTF_ERR || ref > CTF_MAX_TYPE)
    return (ctf_set_errno (fp, EINVAL));

  if (ref != 0 && ((tp = ctf_lookup_by_id (&tmp, ref)) == NULL))
    return CTF_ERR;		/* errno is set for us.  */

  /* Make sure we ultimately point to an integral type.  We also allow slices to
     point to the unimplemented type, for now, because the compiler can emit
     such slices, though they're not very much use.  */

  resolved_ref = ctf_type_resolve_unsliced (fp, ref);
  kind = ctf_type_kind_unsliced (fp, resolved_ref);

  if ((kind != CTF_K_INTEGER) && (kind != CTF_K_FLOAT) &&
      (kind != CTF_K_ENUM)
      && (ref != 0))
    return (ctf_set_errno (fp, ECTF_NOTINTFP));

  if ((type = ctf_add_generic (fp, flag, NULL, CTF_K_SLICE,
			       sizeof (ctf_slice_t), &dtd)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us.  */

  memset (&slice, 0, sizeof (ctf_slice_t));

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (CTF_K_SLICE, flag, 0);
  dtd->dtd_data.ctt_size = clp2 (P2ROUNDUP (ep->cte_bits, CHAR_BIT)
				 / CHAR_BIT);
  slice.cts_type = (uint32_t) ref;
  slice.cts_bits = ep->cte_bits;
  slice.cts_offset = ep->cte_offset;
  memcpy (dtd->dtd_vlen, &slice, sizeof (ctf_slice_t));

  return type;
}

ctf_id_t
ctf_add_integer (ctf_dict_t *fp, uint32_t flag,
		 const char *name, const ctf_encoding_t *ep)
{
  return (ctf_add_encoded (fp, flag, name, ep, CTF_K_INTEGER));
}

ctf_id_t
ctf_add_float (ctf_dict_t *fp, uint32_t flag,
	       const char *name, const ctf_encoding_t *ep)
{
  return (ctf_add_encoded (fp, flag, name, ep, CTF_K_FLOAT));
}

ctf_id_t
ctf_add_pointer (ctf_dict_t *fp, uint32_t flag, ctf_id_t ref)
{
  return (ctf_add_reftype (fp, flag, ref, CTF_K_POINTER));
}

ctf_id_t
ctf_add_array (ctf_dict_t *fp, uint32_t flag, const ctf_arinfo_t *arp)
{
  ctf_dtdef_t *dtd;
  ctf_array_t cta;
  ctf_id_t type;
  ctf_dict_t *tmp = fp;

  if (arp == NULL)
    return (ctf_set_errno (fp, EINVAL));

  if (arp->ctr_contents != 0
      && ctf_lookup_by_id (&tmp, arp->ctr_contents) == NULL)
    return CTF_ERR;		/* errno is set for us.  */

  tmp = fp;
  if (ctf_lookup_by_id (&tmp, arp->ctr_index) == NULL)
    return CTF_ERR;		/* errno is set for us.  */

  if (ctf_type_kind (fp, arp->ctr_index) == CTF_K_FORWARD)
    {
      ctf_err_warn (fp, 1, ECTF_INCOMPLETE,
		    _("ctf_add_array: index type %lx is incomplete"),
		    arp->ctr_contents);
      return (ctf_set_errno (fp, ECTF_INCOMPLETE));
    }

  if ((type = ctf_add_generic (fp, flag, NULL, CTF_K_ARRAY,
			       sizeof (ctf_array_t), &dtd)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us.  */

  memset (&cta, 0, sizeof (ctf_array_t));

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (CTF_K_ARRAY, flag, 0);
  dtd->dtd_data.ctt_size = 0;
  cta.cta_contents = (uint32_t) arp->ctr_contents;
  cta.cta_index = (uint32_t) arp->ctr_index;
  cta.cta_nelems = arp->ctr_nelems;
  memcpy (dtd->dtd_vlen, &cta, sizeof (ctf_array_t));

  return type;
}

int
ctf_set_array (ctf_dict_t *fp, ctf_id_t type, const ctf_arinfo_t *arp)
{
  ctf_dtdef_t *dtd = ctf_dtd_lookup (fp, type);
  ctf_array_t *vlen;

  if (!(fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (fp, ECTF_RDONLY));

  if (dtd == NULL
      || LCTF_INFO_KIND (fp, dtd->dtd_data.ctt_info) != CTF_K_ARRAY)
    return (ctf_set_errno (fp, ECTF_BADID));

  vlen = (ctf_array_t *) dtd->dtd_vlen;
  fp->ctf_flags |= LCTF_DIRTY;
  vlen->cta_contents = (uint32_t) arp->ctr_contents;
  vlen->cta_index = (uint32_t) arp->ctr_index;
  vlen->cta_nelems = arp->ctr_nelems;

  return 0;
}

ctf_id_t
ctf_add_function (ctf_dict_t *fp, uint32_t flag,
		  const ctf_funcinfo_t *ctc, const ctf_id_t *argv)
{
  ctf_dtdef_t *dtd;
  ctf_id_t type;
  uint32_t vlen;
  uint32_t *vdat;
  ctf_dict_t *tmp = fp;
  size_t initial_vlen;
  size_t i;

  if (!(fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (fp, ECTF_RDONLY));

  if (ctc == NULL || (ctc->ctc_flags & ~CTF_FUNC_VARARG) != 0
      || (ctc->ctc_argc != 0 && argv == NULL))
    return (ctf_set_errno (fp, EINVAL));

  vlen = ctc->ctc_argc;
  if (ctc->ctc_flags & CTF_FUNC_VARARG)
    vlen++;	       /* Add trailing zero to indicate varargs (see below).  */

  if (ctc->ctc_return != 0
      && ctf_lookup_by_id (&tmp, ctc->ctc_return) == NULL)
    return CTF_ERR;				/* errno is set for us.  */

  if (vlen > CTF_MAX_VLEN)
    return (ctf_set_errno (fp, EOVERFLOW));

  /* One word extra allocated for padding for 4-byte alignment if need be.
     Not reflected in vlen: we don't want to copy anything into it, and
     it's in addition to (e.g.) the trailing 0 indicating varargs.  */

  initial_vlen = (sizeof (uint32_t) * (vlen + (vlen & 1)));
  if ((type = ctf_add_generic (fp, flag, NULL, CTF_K_FUNCTION,
			       initial_vlen, &dtd)) == CTF_ERR)
    return CTF_ERR;				/* errno is set for us.  */

  vdat = (uint32_t *) dtd->dtd_vlen;

  for (i = 0; i < ctc->ctc_argc; i++)
    {
      tmp = fp;
      if (argv[i] != 0 && ctf_lookup_by_id (&tmp, argv[i]) == NULL)
	return CTF_ERR;				/* errno is set for us.  */
      vdat[i] = (uint32_t) argv[i];
    }

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (CTF_K_FUNCTION, flag, vlen);
  dtd->dtd_data.ctt_type = (uint32_t) ctc->ctc_return;

  if (ctc->ctc_flags & CTF_FUNC_VARARG)
    vdat[vlen - 1] = 0;		   /* Add trailing zero to indicate varargs.  */

  return type;
}

ctf_id_t
ctf_add_struct_sized (ctf_dict_t *fp, uint32_t flag, const char *name,
		      size_t size)
{
  ctf_dtdef_t *dtd;
  ctf_id_t type = 0;
  size_t initial_vlen = sizeof (ctf_lmember_t) * INITIAL_VLEN;

  /* Promote root-visible forwards to structs.  */
  if (name != NULL)
    type = ctf_lookup_by_rawname (fp, CTF_K_STRUCT, name);

  if (type != 0 && ctf_type_kind (fp, type) == CTF_K_FORWARD)
    dtd = ctf_dtd_lookup (fp, type);
  else if ((type = ctf_add_generic (fp, flag, name, CTF_K_STRUCT,
				    initial_vlen, &dtd)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us.  */

  /* Forwards won't have any vlen yet.  */
  if (dtd->dtd_vlen_alloc == 0)
    {
      if ((dtd->dtd_vlen = calloc (1, initial_vlen)) == NULL)
	return (ctf_set_errno (fp, ENOMEM));
      dtd->dtd_vlen_alloc = initial_vlen;
    }

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (CTF_K_STRUCT, flag, 0);
  dtd->dtd_data.ctt_size = CTF_LSIZE_SENT;
  dtd->dtd_data.ctt_lsizehi = CTF_SIZE_TO_LSIZE_HI (size);
  dtd->dtd_data.ctt_lsizelo = CTF_SIZE_TO_LSIZE_LO (size);

  return type;
}

ctf_id_t
ctf_add_struct (ctf_dict_t *fp, uint32_t flag, const char *name)
{
  return (ctf_add_struct_sized (fp, flag, name, 0));
}

ctf_id_t
ctf_add_union_sized (ctf_dict_t *fp, uint32_t flag, const char *name,
		     size_t size)
{
  ctf_dtdef_t *dtd;
  ctf_id_t type = 0;
  size_t initial_vlen = sizeof (ctf_lmember_t) * INITIAL_VLEN;

  /* Promote root-visible forwards to unions.  */
  if (name != NULL)
    type = ctf_lookup_by_rawname (fp, CTF_K_UNION, name);

  if (type != 0 && ctf_type_kind (fp, type) == CTF_K_FORWARD)
    dtd = ctf_dtd_lookup (fp, type);
  else if ((type = ctf_add_generic (fp, flag, name, CTF_K_UNION,
				    initial_vlen, &dtd)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us */

  /* Forwards won't have any vlen yet.  */
  if (dtd->dtd_vlen_alloc == 0)
    {
      if ((dtd->dtd_vlen = calloc (1, initial_vlen)) == NULL)
	return (ctf_set_errno (fp, ENOMEM));
      dtd->dtd_vlen_alloc = initial_vlen;
    }

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (CTF_K_UNION, flag, 0);
  dtd->dtd_data.ctt_size = CTF_LSIZE_SENT;
  dtd->dtd_data.ctt_lsizehi = CTF_SIZE_TO_LSIZE_HI (size);
  dtd->dtd_data.ctt_lsizelo = CTF_SIZE_TO_LSIZE_LO (size);

  return type;
}

ctf_id_t
ctf_add_union (ctf_dict_t *fp, uint32_t flag, const char *name)
{
  return (ctf_add_union_sized (fp, flag, name, 0));
}

ctf_id_t
ctf_add_enum (ctf_dict_t *fp, uint32_t flag, const char *name)
{
  ctf_dtdef_t *dtd;
  ctf_id_t type = 0;
  size_t initial_vlen = sizeof (ctf_enum_t) * INITIAL_VLEN;

  /* Promote root-visible forwards to enums.  */
  if (name != NULL)
    type = ctf_lookup_by_rawname (fp, CTF_K_ENUM, name);

  if (type != 0 && ctf_type_kind (fp, type) == CTF_K_FORWARD)
    dtd = ctf_dtd_lookup (fp, type);
  else if ((type = ctf_add_generic (fp, flag, name, CTF_K_ENUM,
				    initial_vlen, &dtd)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us.  */

  /* Forwards won't have any vlen yet.  */
  if (dtd->dtd_vlen_alloc == 0)
    {
      if ((dtd->dtd_vlen = calloc (1, initial_vlen)) == NULL)
	return (ctf_set_errno (fp, ENOMEM));
      dtd->dtd_vlen_alloc = initial_vlen;
    }

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (CTF_K_ENUM, flag, 0);
  dtd->dtd_data.ctt_size = fp->ctf_dmodel->ctd_int;

  return type;
}

ctf_id_t
ctf_add_enum_encoded (ctf_dict_t *fp, uint32_t flag, const char *name,
		      const ctf_encoding_t *ep)
{
  ctf_id_t type = 0;

  /* First, create the enum if need be, using most of the same machinery as
     ctf_add_enum(), to ensure that we do not allow things past that are not
     enums or forwards to them.  (This includes other slices: you cannot slice a
     slice, which would be a useless thing to do anyway.)  */

  if (name != NULL)
    type = ctf_lookup_by_rawname (fp, CTF_K_ENUM, name);

  if (type != 0)
    {
      if ((ctf_type_kind (fp, type) != CTF_K_FORWARD) &&
	  (ctf_type_kind_unsliced (fp, type) != CTF_K_ENUM))
	return (ctf_set_errno (fp, ECTF_NOTINTFP));
    }
  else if ((type = ctf_add_enum (fp, flag, name)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us.  */

  /* Now attach a suitable slice to it.  */

  return ctf_add_slice (fp, flag, type, ep);
}

ctf_id_t
ctf_add_forward (ctf_dict_t *fp, uint32_t flag, const char *name,
		 uint32_t kind)
{
  ctf_dtdef_t *dtd;
  ctf_id_t type = 0;

  if (!ctf_forwardable_kind (kind))
    return (ctf_set_errno (fp, ECTF_NOTSUE));

  if (name == NULL || name[0] == '\0')
    return (ctf_set_errno (fp, ECTF_NONAME));

  /* If the type is already defined or exists as a forward tag, just
     return the ctf_id_t of the existing definition.  */

  type = ctf_lookup_by_rawname (fp, kind, name);

  if (type)
    return type;

  if ((type = ctf_add_generic (fp, flag, name, kind, 0, &dtd)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us.  */

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (CTF_K_FORWARD, flag, 0);
  dtd->dtd_data.ctt_type = kind;

  return type;
}

ctf_id_t
ctf_add_unknown (ctf_dict_t *fp, uint32_t flag, const char *name)
{
  ctf_dtdef_t *dtd;
  ctf_id_t type = 0;

  /* If a type is already defined with this name, error (if not CTF_K_UNKNOWN)
     or just return it.  */

  if (name != NULL && name[0] != '\0' && flag == CTF_ADD_ROOT
      && (type = ctf_lookup_by_rawname (fp, CTF_K_UNKNOWN, name)))
    {
      if (ctf_type_kind (fp, type) == CTF_K_UNKNOWN)
	return type;
      else
	{
	  ctf_err_warn (fp, 1, ECTF_CONFLICT,
			_("ctf_add_unknown: cannot add unknown type "
			  "named %s: type of this name already defined"),
			name ? name : _("(unnamed type)"));
	  return (ctf_set_errno (fp, ECTF_CONFLICT));
	}
    }

  if ((type = ctf_add_generic (fp, flag, name, CTF_K_UNKNOWN, 0, &dtd)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us.  */

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (CTF_K_UNKNOWN, flag, 0);
  dtd->dtd_data.ctt_type = 0;

  return type;
}

ctf_id_t
ctf_add_typedef (ctf_dict_t *fp, uint32_t flag, const char *name,
		 ctf_id_t ref)
{
  ctf_dtdef_t *dtd;
  ctf_id_t type;
  ctf_dict_t *tmp = fp;

  if (ref == CTF_ERR || ref > CTF_MAX_TYPE)
    return (ctf_set_errno (fp, EINVAL));

  if (name == NULL || name[0] == '\0')
    return (ctf_set_errno (fp, ECTF_NONAME));

  if (ref != 0 && ctf_lookup_by_id (&tmp, ref) == NULL)
    return CTF_ERR;		/* errno is set for us.  */

  if ((type = ctf_add_generic (fp, flag, name, CTF_K_TYPEDEF, 0,
			       &dtd)) == CTF_ERR)
    return CTF_ERR;		/* errno is set for us.  */

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (CTF_K_TYPEDEF, flag, 0);
  dtd->dtd_data.ctt_type = (uint32_t) ref;

  return type;
}

ctf_id_t
ctf_add_volatile (ctf_dict_t *fp, uint32_t flag, ctf_id_t ref)
{
  return (ctf_add_reftype (fp, flag, ref, CTF_K_VOLATILE));
}

ctf_id_t
ctf_add_const (ctf_dict_t *fp, uint32_t flag, ctf_id_t ref)
{
  return (ctf_add_reftype (fp, flag, ref, CTF_K_CONST));
}

ctf_id_t
ctf_add_restrict (ctf_dict_t *fp, uint32_t flag, ctf_id_t ref)
{
  return (ctf_add_reftype (fp, flag, ref, CTF_K_RESTRICT));
}

int
ctf_add_enumerator (ctf_dict_t *fp, ctf_id_t enid, const char *name,
		    int value)
{
  ctf_dtdef_t *dtd = ctf_dtd_lookup (fp, enid);
  unsigned char *old_vlen;
  ctf_enum_t *en;
  size_t i;

  uint32_t kind, vlen, root;

  if (name == NULL)
    return (ctf_set_errno (fp, EINVAL));

  if (!(fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (fp, ECTF_RDONLY));

  if (dtd == NULL)
    return (ctf_set_errno (fp, ECTF_BADID));

  kind = LCTF_INFO_KIND (fp, dtd->dtd_data.ctt_info);
  root = LCTF_INFO_ISROOT (fp, dtd->dtd_data.ctt_info);
  vlen = LCTF_INFO_VLEN (fp, dtd->dtd_data.ctt_info);

  if (kind != CTF_K_ENUM)
    return (ctf_set_errno (fp, ECTF_NOTENUM));

  if (vlen == CTF_MAX_VLEN)
    return (ctf_set_errno (fp, ECTF_DTFULL));

  old_vlen = dtd->dtd_vlen;
  if (ctf_grow_vlen (fp, dtd, sizeof (ctf_enum_t) * (vlen + 1)) < 0)
    return -1;					/* errno is set for us.  */
  en = (ctf_enum_t *) dtd->dtd_vlen;

  if (dtd->dtd_vlen != old_vlen)
    {
      ptrdiff_t move = (signed char *) dtd->dtd_vlen - (signed char *) old_vlen;

      /* Remove pending refs in the old vlen region and reapply them.  */

      for (i = 0; i < vlen; i++)
	ctf_str_move_pending (fp, &en[i].cte_name, move);
    }

  for (i = 0; i < vlen; i++)
    if (strcmp (ctf_strptr (fp, en[i].cte_name), name) == 0)
      return (ctf_set_errno (fp, ECTF_DUPLICATE));

  en[i].cte_name = ctf_str_add_pending (fp, name, &en[i].cte_name);
  en[i].cte_value = value;

  if (en[i].cte_name == 0 && name != NULL && name[0] != '\0')
    return -1;					/* errno is set for us. */

  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (kind, root, vlen + 1);

  fp->ctf_flags |= LCTF_DIRTY;

  return 0;
}

int
ctf_add_member_offset (ctf_dict_t *fp, ctf_id_t souid, const char *name,
		       ctf_id_t type, unsigned long bit_offset)
{
  ctf_dtdef_t *dtd = ctf_dtd_lookup (fp, souid);

  ssize_t msize, malign, ssize;
  uint32_t kind, vlen, root;
  size_t i;
  int is_incomplete = 0;
  unsigned char *old_vlen;
  ctf_lmember_t *memb;

  if (!(fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (fp, ECTF_RDONLY));

  if (dtd == NULL)
    return (ctf_set_errno (fp, ECTF_BADID));

  if (name != NULL && name[0] == '\0')
    name = NULL;

  kind = LCTF_INFO_KIND (fp, dtd->dtd_data.ctt_info);
  root = LCTF_INFO_ISROOT (fp, dtd->dtd_data.ctt_info);
  vlen = LCTF_INFO_VLEN (fp, dtd->dtd_data.ctt_info);

  if (kind != CTF_K_STRUCT && kind != CTF_K_UNION)
    return (ctf_set_errno (fp, ECTF_NOTSOU));

  if (vlen == CTF_MAX_VLEN)
    return (ctf_set_errno (fp, ECTF_DTFULL));

  old_vlen = dtd->dtd_vlen;
  if (ctf_grow_vlen (fp, dtd, sizeof (ctf_lmember_t) * (vlen + 1)) < 0)
    return -1;					/* errno is set for us.  */
  memb = (ctf_lmember_t *) dtd->dtd_vlen;

  if (dtd->dtd_vlen != old_vlen)
    {
      ptrdiff_t move = (signed char *) dtd->dtd_vlen - (signed char *) old_vlen;

      /* Remove pending refs in the old vlen region and reapply them.  */

      for (i = 0; i < vlen; i++)
	ctf_str_move_pending (fp, &memb[i].ctlm_name, move);
    }

  if (name != NULL)
    {
      for (i = 0; i < vlen; i++)
	if (strcmp (ctf_strptr (fp, memb[i].ctlm_name), name) == 0)
	  return (ctf_set_errno (fp, ECTF_DUPLICATE));
    }

  if ((msize = ctf_type_size (fp, type)) < 0 ||
      (malign = ctf_type_align (fp, type)) < 0)
    {
      /* The unimplemented type, and any type that resolves to it, has no size
	 and no alignment: it can correspond to any number of compiler-inserted
	 types.  We allow incomplete types through since they are routinely
	 added to the ends of structures, and can even be added elsewhere in
	 structures by the deduplicator.  They are assumed to be zero-size with
	 no alignment: this is often wrong, but problems can be avoided in this
	 case by explicitly specifying the size of the structure via the _sized
	 functions.  The deduplicator always does this.  */

      msize = 0;
      malign = 0;
      if (ctf_errno (fp) == ECTF_NONREPRESENTABLE)
	ctf_set_errno (fp, 0);
      else if (ctf_errno (fp) == ECTF_INCOMPLETE)
	is_incomplete = 1;
      else
	return -1;		/* errno is set for us.  */
    }

  memb[vlen].ctlm_name = ctf_str_add_pending (fp, name, &memb[vlen].ctlm_name);
  memb[vlen].ctlm_type = type;
  if (memb[vlen].ctlm_name == 0 && name != NULL && name[0] != '\0')
    return -1;			/* errno is set for us.  */

  if (kind == CTF_K_STRUCT && vlen != 0)
    {
      if (bit_offset == (unsigned long) - 1)
	{
	  /* Natural alignment.  */

	  ctf_id_t ltype = ctf_type_resolve (fp, memb[vlen - 1].ctlm_type);
	  size_t off = CTF_LMEM_OFFSET(&memb[vlen - 1]);

	  ctf_encoding_t linfo;
	  ssize_t lsize;

	  /* Propagate any error from ctf_type_resolve.  If the last member was
	     of unimplemented type, this may be -ECTF_NONREPRESENTABLE: we
	     cannot insert right after such a member without explicit offset
	     specification, because its alignment and size is not known.  */
	  if (ltype == CTF_ERR)
	    return -1;	/* errno is set for us.  */

	  if (is_incomplete)
	    {
	      ctf_err_warn (fp, 1, ECTF_INCOMPLETE,
			    _("ctf_add_member_offset: cannot add member %s of "
			      "incomplete type %lx to struct %lx without "
			      "specifying explicit offset\n"),
			    name ? name : _("(unnamed member)"), type, souid);
	      return (ctf_set_errno (fp, ECTF_INCOMPLETE));
	    }

	  if (ctf_type_encoding (fp, ltype, &linfo) == 0)
	    off += linfo.cte_bits;
	  else if ((lsize = ctf_type_size (fp, ltype)) > 0)
	    off += lsize * CHAR_BIT;
	  else if (lsize == -1 && ctf_errno (fp) == ECTF_INCOMPLETE)
	    {
	      const char *lname = ctf_strraw (fp, memb[vlen - 1].ctlm_name);

	      ctf_err_warn (fp, 1, ECTF_INCOMPLETE,
			    _("ctf_add_member_offset: cannot add member %s of "
			      "type %lx to struct %lx without specifying "
			      "explicit offset after member %s of type %lx, "
			      "which is an incomplete type\n"),
			    name ? name : _("(unnamed member)"), type, souid,
			    lname ? lname : _("(unnamed member)"), ltype);
	      return -1;			/* errno is set for us.  */
	    }

	  /* Round up the offset of the end of the last member to
	     the next byte boundary, convert 'off' to bytes, and
	     then round it up again to the next multiple of the
	     alignment required by the new member.  Finally,
	     convert back to bits and store the result in
	     dmd_offset.  Technically we could do more efficient
	     packing if the new member is a bit-field, but we're
	     the "compiler" and ANSI says we can do as we choose.  */

	  off = roundup (off, CHAR_BIT) / CHAR_BIT;
	  off = roundup (off, MAX (malign, 1));
	  memb[vlen].ctlm_offsethi = CTF_OFFSET_TO_LMEMHI (off * CHAR_BIT);
	  memb[vlen].ctlm_offsetlo = CTF_OFFSET_TO_LMEMLO (off * CHAR_BIT);
	  ssize = off + msize;
	}
      else
	{
	  /* Specified offset in bits.  */

	  memb[vlen].ctlm_offsethi = CTF_OFFSET_TO_LMEMHI (bit_offset);
	  memb[vlen].ctlm_offsetlo = CTF_OFFSET_TO_LMEMLO (bit_offset);
	  ssize = ctf_get_ctt_size (fp, &dtd->dtd_data, NULL, NULL);
	  ssize = MAX (ssize, ((signed) bit_offset / CHAR_BIT) + msize);
	}
    }
  else
    {
      memb[vlen].ctlm_offsethi = 0;
      memb[vlen].ctlm_offsetlo = 0;
      ssize = ctf_get_ctt_size (fp, &dtd->dtd_data, NULL, NULL);
      ssize = MAX (ssize, msize);
    }

  dtd->dtd_data.ctt_size = CTF_LSIZE_SENT;
  dtd->dtd_data.ctt_lsizehi = CTF_SIZE_TO_LSIZE_HI (ssize);
  dtd->dtd_data.ctt_lsizelo = CTF_SIZE_TO_LSIZE_LO (ssize);
  dtd->dtd_data.ctt_info = CTF_TYPE_INFO (kind, root, vlen + 1);

  fp->ctf_flags |= LCTF_DIRTY;
  return 0;
}

int
ctf_add_member_encoded (ctf_dict_t *fp, ctf_id_t souid, const char *name,
			ctf_id_t type, unsigned long bit_offset,
			const ctf_encoding_t encoding)
{
  ctf_dtdef_t *dtd = ctf_dtd_lookup (fp, type);
  int kind = LCTF_INFO_KIND (fp, dtd->dtd_data.ctt_info);
  int otype = type;

  if ((kind != CTF_K_INTEGER) && (kind != CTF_K_FLOAT) && (kind != CTF_K_ENUM))
    return (ctf_set_errno (fp, ECTF_NOTINTFP));

  if ((type = ctf_add_slice (fp, CTF_ADD_NONROOT, otype, &encoding)) == CTF_ERR)
    return -1;			/* errno is set for us.  */

  return ctf_add_member_offset (fp, souid, name, type, bit_offset);
}

int
ctf_add_member (ctf_dict_t *fp, ctf_id_t souid, const char *name,
		ctf_id_t type)
{
  return ctf_add_member_offset (fp, souid, name, type, (unsigned long) - 1);
}

int
ctf_add_variable (ctf_dict_t *fp, const char *name, ctf_id_t ref)
{
  ctf_dvdef_t *dvd;
  ctf_dict_t *tmp = fp;

  if (!(fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (fp, ECTF_RDONLY));

  if (ctf_dvd_lookup (fp, name) != NULL)
    return (ctf_set_errno (fp, ECTF_DUPLICATE));

  if (ctf_lookup_by_id (&tmp, ref) == NULL)
    return -1;			/* errno is set for us.  */

  /* Make sure this type is representable.  */
  if ((ctf_type_resolve (fp, ref) == CTF_ERR)
      && (ctf_errno (fp) == ECTF_NONREPRESENTABLE))
    return -1;

  if ((dvd = malloc (sizeof (ctf_dvdef_t))) == NULL)
    return (ctf_set_errno (fp, EAGAIN));

  if (name != NULL && (dvd->dvd_name = strdup (name)) == NULL)
    {
      free (dvd);
      return (ctf_set_errno (fp, EAGAIN));
    }
  dvd->dvd_type = ref;
  dvd->dvd_snapshots = fp->ctf_snapshots;

  if (ctf_dvd_insert (fp, dvd) < 0)
    {
      free (dvd->dvd_name);
      free (dvd);
      return -1;			/* errno is set for us.  */
    }

  fp->ctf_flags |= LCTF_DIRTY;
  return 0;
}

int
ctf_add_funcobjt_sym (ctf_dict_t *fp, int is_function, const char *name, ctf_id_t id)
{
  ctf_dict_t *tmp = fp;
  char *dupname;
  ctf_dynhash_t *h = is_function ? fp->ctf_funchash : fp->ctf_objthash;

  if (!(fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (fp, ECTF_RDONLY));

  if (ctf_dynhash_lookup (fp->ctf_objthash, name) != NULL ||
      ctf_dynhash_lookup (fp->ctf_funchash, name) != NULL)
    return (ctf_set_errno (fp, ECTF_DUPLICATE));

  if (ctf_lookup_by_id (&tmp, id) == NULL)
    return -1;                                  /* errno is set for us.  */

  if (is_function && ctf_type_kind (fp, id) != CTF_K_FUNCTION)
    return (ctf_set_errno (fp, ECTF_NOTFUNC));

  if ((dupname = strdup (name)) == NULL)
    return (ctf_set_errno (fp, ENOMEM));

  if (ctf_dynhash_insert (h, dupname, (void *) (uintptr_t) id) < 0)
    {
      free (dupname);
      return (ctf_set_errno (fp, ENOMEM));
    }
  return 0;
}

int
ctf_add_objt_sym (ctf_dict_t *fp, const char *name, ctf_id_t id)
{
  return (ctf_add_funcobjt_sym (fp, 0, name, id));
}

int
ctf_add_func_sym (ctf_dict_t *fp, const char *name, ctf_id_t id)
{
  return (ctf_add_funcobjt_sym (fp, 1, name, id));
}

typedef struct ctf_bundle
{
  ctf_dict_t *ctb_dict;		/* CTF dict handle.  */
  ctf_id_t ctb_type;		/* CTF type identifier.  */
  ctf_dtdef_t *ctb_dtd;		/* CTF dynamic type definition (if any).  */
} ctf_bundle_t;

static int
enumcmp (const char *name, int value, void *arg)
{
  ctf_bundle_t *ctb = arg;
  int bvalue;

  if (ctf_enum_value (ctb->ctb_dict, ctb->ctb_type, name, &bvalue) < 0)
    {
      ctf_err_warn (ctb->ctb_dict, 0, 0,
		    _("conflict due to enum %s iteration error"), name);
      return 1;
    }
  if (value != bvalue)
    {
      ctf_err_warn (ctb->ctb_dict, 1, ECTF_CONFLICT,
		    _("conflict due to enum value change: %i versus %i"),
		    value, bvalue);
      return 1;
    }
  return 0;
}

static int
enumadd (const char *name, int value, void *arg)
{
  ctf_bundle_t *ctb = arg;

  return (ctf_add_enumerator (ctb->ctb_dict, ctb->ctb_type,
			      name, value) < 0);
}

static int
membcmp (const char *name, ctf_id_t type _libctf_unused_, unsigned long offset,
	 void *arg)
{
  ctf_bundle_t *ctb = arg;
  ctf_membinfo_t ctm;

  /* Don't check nameless members (e.g. anonymous structs/unions) against each
     other.  */
  if (name[0] == 0)
    return 0;

  if (ctf_member_info (ctb->ctb_dict, ctb->ctb_type, name, &ctm) < 0)
    {
      ctf_err_warn (ctb->ctb_dict, 0, 0,
		    _("conflict due to struct member %s iteration error"),
		    name);
      return 1;
    }
  if (ctm.ctm_offset != offset)
    {
      ctf_err_warn (ctb->ctb_dict, 1, ECTF_CONFLICT,
		    _("conflict due to struct member %s offset change: "
		      "%lx versus %lx"),
		    name, ctm.ctm_offset, offset);
      return 1;
    }
  return 0;
}

/* Record the correspondence between a source and ctf_add_type()-added
   destination type: both types are translated into parent type IDs if need be,
   so they relate to the actual dictionary they are in.  Outside controlled
   circumstances (like linking) it is probably not useful to do more than
   compare these pointers, since there is nothing stopping the user closing the
   source dict whenever they want to.

   Our OOM handling here is just to not do anything, because this is called deep
   enough in the call stack that doing anything useful is painfully difficult:
   the worst consequence if we do OOM is a bit of type duplication anyway.  */

static void
ctf_add_type_mapping (ctf_dict_t *src_fp, ctf_id_t src_type,
		      ctf_dict_t *dst_fp, ctf_id_t dst_type)
{
  if (LCTF_TYPE_ISPARENT (src_fp, src_type) && src_fp->ctf_parent)
    src_fp = src_fp->ctf_parent;

  src_type = LCTF_TYPE_TO_INDEX(src_fp, src_type);

  if (LCTF_TYPE_ISPARENT (dst_fp, dst_type) && dst_fp->ctf_parent)
    dst_fp = dst_fp->ctf_parent;

  dst_type = LCTF_TYPE_TO_INDEX(dst_fp, dst_type);

  if (dst_fp->ctf_link_type_mapping == NULL)
    {
      ctf_hash_fun f = ctf_hash_type_key;
      ctf_hash_eq_fun e = ctf_hash_eq_type_key;

      if ((dst_fp->ctf_link_type_mapping = ctf_dynhash_create (f, e, free,
							       NULL)) == NULL)
	return;
    }

  ctf_link_type_key_t *key;
  key = calloc (1, sizeof (struct ctf_link_type_key));
  if (!key)
    return;

  key->cltk_fp = src_fp;
  key->cltk_idx = src_type;

  /* No OOM checking needed, because if this doesn't work the worst we'll do is
     add a few more duplicate types (which will probably run out of memory
     anyway).  */
  ctf_dynhash_insert (dst_fp->ctf_link_type_mapping, key,
		      (void *) (uintptr_t) dst_type);
}

/* Look up a type mapping: return 0 if none.  The DST_FP is modified to point to
   the parent if need be.  The ID returned is from the dst_fp's perspective.  */
static ctf_id_t
ctf_type_mapping (ctf_dict_t *src_fp, ctf_id_t src_type, ctf_dict_t **dst_fp)
{
  ctf_link_type_key_t key;
  ctf_dict_t *target_fp = *dst_fp;
  ctf_id_t dst_type = 0;

  if (LCTF_TYPE_ISPARENT (src_fp, src_type) && src_fp->ctf_parent)
    src_fp = src_fp->ctf_parent;

  src_type = LCTF_TYPE_TO_INDEX(src_fp, src_type);
  key.cltk_fp = src_fp;
  key.cltk_idx = src_type;

  if (target_fp->ctf_link_type_mapping)
    dst_type = (uintptr_t) ctf_dynhash_lookup (target_fp->ctf_link_type_mapping,
					       &key);

  if (dst_type != 0)
    {
      dst_type = LCTF_INDEX_TO_TYPE (target_fp, dst_type,
				     target_fp->ctf_parent != NULL);
      *dst_fp = target_fp;
      return dst_type;
    }

  if (target_fp->ctf_parent)
    target_fp = target_fp->ctf_parent;
  else
    return 0;

  if (target_fp->ctf_link_type_mapping)
    dst_type = (uintptr_t) ctf_dynhash_lookup (target_fp->ctf_link_type_mapping,
					       &key);

  if (dst_type)
    dst_type = LCTF_INDEX_TO_TYPE (target_fp, dst_type,
				   target_fp->ctf_parent != NULL);

  *dst_fp = target_fp;
  return dst_type;
}

/* The ctf_add_type routine is used to copy a type from a source CTF dictionary
   to a dynamic destination dictionary.  This routine operates recursively by
   following the source type's links and embedded member types.  If the
   destination dict already contains a named type which has the same attributes,
   then we succeed and return this type but no changes occur.  */
static ctf_id_t
ctf_add_type_internal (ctf_dict_t *dst_fp, ctf_dict_t *src_fp, ctf_id_t src_type,
		       ctf_dict_t *proc_tracking_fp)
{
  ctf_id_t dst_type = CTF_ERR;
  uint32_t dst_kind = CTF_K_UNKNOWN;
  ctf_dict_t *tmp_fp = dst_fp;
  ctf_id_t tmp;

  const char *name;
  uint32_t kind, forward_kind, flag, vlen;

  const ctf_type_t *src_tp, *dst_tp;
  ctf_bundle_t src, dst;
  ctf_encoding_t src_en, dst_en;
  ctf_arinfo_t src_ar, dst_ar;

  ctf_funcinfo_t ctc;

  ctf_id_t orig_src_type = src_type;

  if (!(dst_fp->ctf_flags & LCTF_RDWR))
    return (ctf_set_errno (dst_fp, ECTF_RDONLY));

  if ((src_tp = ctf_lookup_by_id (&src_fp, src_type)) == NULL)
    return (ctf_set_errno (dst_fp, ctf_errno (src_fp)));

  if ((ctf_type_resolve (src_fp, src_type) == CTF_ERR)
      && (ctf_errno (src_fp) == ECTF_NONREPRESENTABLE))
    return (ctf_set_errno (dst_fp, ECTF_NONREPRESENTABLE));

  name = ctf_strptr (src_fp, src_tp->ctt_name);
  kind = LCTF_INFO_KIND (src_fp, src_tp->ctt_info);
  flag = LCTF_INFO_ISROOT (src_fp, src_tp->ctt_info);
  vlen = LCTF_INFO_VLEN (src_fp, src_tp->ctt_info);

  /* If this is a type we are currently in the middle of adding, hand it
     straight back.  (This lets us handle self-referential structures without
     considering forwards and empty structures the same as their completed
     forms.)  */

  tmp = ctf_type_mapping (src_fp, src_type, &tmp_fp);

  if (tmp != 0)
    {
      if (ctf_dynhash_lookup (proc_tracking_fp->ctf_add_processing,
			      (void *) (uintptr_t) src_type))
	return tmp;

      /* If this type has already been added from this dictionary, and is the
	 same kind and (if a struct or union) has the same number of members,
	 hand it straight back.  */

      if (ctf_type_kind_unsliced (tmp_fp, tmp) == (int) kind)
	{
	  if (kind == CTF_K_STRUCT || kind == CTF_K_UNION
	      || kind == CTF_K_ENUM)
	    {
	      if ((dst_tp = ctf_lookup_by_id (&tmp_fp, dst_type)) != NULL)
		if (vlen == LCTF_INFO_VLEN (tmp_fp, dst_tp->ctt_info))
		  return tmp;
	    }
	  else
	    return tmp;
	}
    }

  forward_kind = kind;
  if (kind == CTF_K_FORWARD)
    forward_kind = src_tp->ctt_type;

  /* If the source type has a name and is a root type (visible at the top-level
     scope), lookup the name in the destination dictionary and verify that it is
     of the same kind before we do anything else.  */

  if ((flag & CTF_ADD_ROOT) && name[0] != '\0'
      && (tmp = ctf_lookup_by_rawname (dst_fp, forward_kind, name)) != 0)
    {
      dst_type = tmp;
      dst_kind = ctf_type_kind_unsliced (dst_fp, dst_type);
    }

  /* If an identically named dst_type exists, fail with ECTF_CONFLICT
     unless dst_type is a forward declaration and src_type is a struct,
     union, or enum (i.e. the definition of the previous forward decl).

     We also allow addition in the opposite order (addition of a forward when a
     struct, union, or enum already exists), which is a NOP and returns the
     already-present struct, union, or enum.  */

  if (dst_type != CTF_ERR && dst_kind != kind)
    {
      if (kind == CTF_K_FORWARD
	  && (dst_kind == CTF_K_ENUM || dst_kind == CTF_K_STRUCT
	      || dst_kind == CTF_K_UNION))
	{
	  ctf_add_type_mapping (src_fp, src_type, dst_fp, dst_type);
	  return dst_type;
	}

      if (dst_kind != CTF_K_FORWARD
	  || (kind != CTF_K_ENUM && kind != CTF_K_STRUCT
	      && kind != CTF_K_UNION))
	{
	  ctf_err_warn (dst_fp, 1, ECTF_CONFLICT,
			_("ctf_add_type: conflict for type %s: "
			  "kinds differ, new: %i; old (ID %lx): %i"),
			name, kind, dst_type, dst_kind);
	  return (ctf_set_errno (dst_fp, ECTF_CONFLICT));
	}
    }

  /* We take special action for an integer, float, or slice since it is
     described not only by its name but also its encoding.  For integers,
     bit-fields exploit this degeneracy.  */

  if (kind == CTF_K_INTEGER || kind == CTF_K_FLOAT || kind == CTF_K_SLICE)
    {
      if (ctf_type_encoding (src_fp, src_type, &src_en) != 0)
	return (ctf_set_errno (dst_fp, ctf_errno (src_fp)));

      if (dst_type != CTF_ERR)
	{
	  ctf_dict_t *fp = dst_fp;

	  if ((dst_tp = ctf_lookup_by_id (&fp, dst_type)) == NULL)
	    return CTF_ERR;

	  if (ctf_type_encoding (dst_fp, dst_type, &dst_en) != 0)
	    return CTF_ERR;			/* errno set for us.  */

	  if (LCTF_INFO_ISROOT (fp, dst_tp->ctt_info) & CTF_ADD_ROOT)
	    {
	      /* The type that we found in the hash is also root-visible.  If
		 the two types match then use the existing one; otherwise,
		 declare a conflict.  Note: slices are not certain to match
		 even if there is no conflict: we must check the contained type
		 too.  */

	      if (memcmp (&src_en, &dst_en, sizeof (ctf_encoding_t)) == 0)
		{
		  if (kind != CTF_K_SLICE)
		    {
		      ctf_add_type_mapping (src_fp, src_type, dst_fp, dst_type);
		      return dst_type;
		    }
		}
	      else
		  {
		    return (ctf_set_errno (dst_fp, ECTF_CONFLICT));
		  }
	    }
	  else
	    {
	      /* We found a non-root-visible type in the hash.  If its encoding
		 is the same, we can reuse it, unless it is a slice.  */

	      if (memcmp (&src_en, &dst_en, sizeof (ctf_encoding_t)) == 0)
		{
		  if (kind != CTF_K_SLICE)
		    {
		      ctf_add_type_mapping (src_fp, src_type, dst_fp, dst_type);
		      return dst_type;
		    }
		}
	    }
	}
    }

  src.ctb_dict = src_fp;
  src.ctb_type = src_type;
  src.ctb_dtd = NULL;

  dst.ctb_dict = dst_fp;
  dst.ctb_type = dst_type;
  dst.ctb_dtd = NULL;

  /* Now perform kind-specific processing.  If dst_type is CTF_ERR, then we add
     a new type with the same properties as src_type to dst_fp.  If dst_type is
     not CTF_ERR, then we verify that dst_type has the same attributes as
     src_type.  We recurse for embedded references.  Before we start, we note
     that we are processing this type, to prevent infinite recursion: we do not
     re-process any type that appears in this list.  The list is emptied
     wholesale at the end of processing everything in this recursive stack.  */

  if (ctf_dynhash_insert (proc_tracking_fp->ctf_add_processing,
			  (void *) (uintptr_t) src_type, (void *) 1) < 0)
    return ctf_set_errno (dst_fp, ENOMEM);

  switch (kind)
    {
    case CTF_K_INTEGER:
      /*  If we found a match we will have either returned it or declared a
	  conflict.  */
      dst_type = ctf_add_integer (dst_fp, flag, name, &src_en);
      break;

    case CTF_K_FLOAT:
      /* If we found a match we will have either returned it or declared a
       conflict.  */
      dst_type = ctf_add_float (dst_fp, flag, name, &src_en);
      break;

    case CTF_K_SLICE:
      /* We have checked for conflicting encodings: now try to add the
	 contained type.  */
      src_type = ctf_type_reference (src_fp, src_type);
      src_type = ctf_add_type_internal (dst_fp, src_fp, src_type,
					proc_tracking_fp);

      if (src_type == CTF_ERR)
	return CTF_ERR;				/* errno is set for us.  */

      dst_type = ctf_add_slice (dst_fp, flag, src_type, &src_en);
      break;

    case CTF_K_POINTER:
    case CTF_K_VOLATILE:
    case CTF_K_CONST:
    case CTF_K_RESTRICT:
      src_type = ctf_type_reference (src_fp, src_type);
      src_type = ctf_add_type_internal (dst_fp, src_fp, src_type,
					proc_tracking_fp);

      if (src_type == CTF_ERR)
	return CTF_ERR;				/* errno is set for us.  */

      dst_type = ctf_add_reftype (dst_fp, flag, src_type, kind);
      break;

    case CTF_K_ARRAY:
      if (ctf_array_info (src_fp, src_type, &src_ar) != 0)
	return (ctf_set_errno (dst_fp, ctf_errno (src_fp)));

      src_ar.ctr_contents =
	ctf_add_type_internal (dst_fp, src_fp, src_ar.ctr_contents,
			       proc_tracking_fp);
      src_ar.ctr_index = ctf_add_type_internal (dst_fp, src_fp,
						src_ar.ctr_index,
						proc_tracking_fp);
      src_ar.ctr_nelems = src_ar.ctr_nelems;

      if (src_ar.ctr_contents == CTF_ERR || src_ar.ctr_index == CTF_ERR)
	return CTF_ERR;				/* errno is set for us.  */

      if (dst_type != CTF_ERR)
	{
	  if (ctf_array_info (dst_fp, dst_type, &dst_ar) != 0)
	    return CTF_ERR;			/* errno is set for us.  */

	  if (memcmp (&src_ar, &dst_ar, sizeof (ctf_arinfo_t)))
	    {
	      ctf_err_warn (dst_fp, 1, ECTF_CONFLICT,
			    _("conflict for type %s against ID %lx: array info "
			      "differs, old %lx/%lx/%x; new: %lx/%lx/%x"),
			    name, dst_type, src_ar.ctr_contents,
			    src_ar.ctr_index, src_ar.ctr_nelems,
			    dst_ar.ctr_contents, dst_ar.ctr_index,
			    dst_ar.ctr_nelems);
	      return (ctf_set_errno (dst_fp, ECTF_CONFLICT));
	    }
	}
      else
	dst_type = ctf_add_array (dst_fp, flag, &src_ar);
      break;

    case CTF_K_FUNCTION:
      ctc.ctc_return = ctf_add_type_internal (dst_fp, src_fp,
					      src_tp->ctt_type,
					      proc_tracking_fp);
      ctc.ctc_argc = 0;
      ctc.ctc_flags = 0;

      if (ctc.ctc_return == CTF_ERR)
	return CTF_ERR;				/* errno is set for us.  */

      dst_type = ctf_add_function (dst_fp, flag, &ctc, NULL);
      break;

    case CTF_K_STRUCT:
    case CTF_K_UNION:
      {
	ctf_next_t *i = NULL;
	ssize_t offset;
	const char *membname;
	ctf_id_t src_membtype;

	/* Technically to match a struct or union we need to check both
	   ways (src members vs. dst, dst members vs. src) but we make
	   this more optimal by only checking src vs. dst and comparing
	   the total size of the structure (which we must do anyway)
	   which covers the possibility of dst members not in src.
	   This optimization can be defeated for unions, but is so
	   pathological as to render it irrelevant for our purposes.  */

	if (dst_type != CTF_ERR && kind != CTF_K_FORWARD
	    && dst_kind != CTF_K_FORWARD)
	  {
	    if (ctf_type_size (src_fp, src_type) !=
		ctf_type_size (dst_fp, dst_type))
	      {
		ctf_err_warn (dst_fp, 1, ECTF_CONFLICT,
			      _("conflict for type %s against ID %lx: union "
				"size differs, old %li, new %li"), name,
			      dst_type, (long) ctf_type_size (src_fp, src_type),
			      (long) ctf_type_size (dst_fp, dst_type));
		return (ctf_set_errno (dst_fp, ECTF_CONFLICT));
	      }

	    if (ctf_member_iter (src_fp, src_type, membcmp, &dst))
	      {
		ctf_err_warn (dst_fp, 1, ECTF_CONFLICT,
			      _("conflict for type %s against ID %lx: members "
				"differ, see above"), name, dst_type);
		return (ctf_set_errno (dst_fp, ECTF_CONFLICT));
	      }

	    break;
	  }

	dst_type = ctf_add_struct_sized (dst_fp, flag, name,
					 ctf_type_size (src_fp, src_type));
	if (dst_type == CTF_ERR)
	  return CTF_ERR;			/* errno is set for us.  */

	/* Pre-emptively add this struct to the type mapping so that
	   structures that refer to themselves work.  */
	ctf_add_type_mapping (src_fp, src_type, dst_fp, dst_type);

	while ((offset = ctf_member_next (src_fp, src_type, &i, &membname,
					  &src_membtype, 0)) >= 0)
	  {
	    ctf_dict_t *dst = dst_fp;
	    ctf_id_t dst_membtype = ctf_type_mapping (src_fp, src_membtype, &dst);

	    if (dst_membtype == 0)
	      {
		dst_membtype = ctf_add_type_internal (dst_fp, src_fp,
						      src_membtype,
						      proc_tracking_fp);
		if (dst_membtype == CTF_ERR)
		  {
		    if (ctf_errno (dst_fp) != ECTF_NONREPRESENTABLE)
		      {
			ctf_next_destroy (i);
			break;
		      }
		  }
	      }

	    if (ctf_add_member_offset (dst_fp, dst_type, membname,
				       dst_membtype, offset) < 0)
	      {
		ctf_next_destroy (i);
		break;
	      }
	  }
	if (ctf_errno (src_fp) != ECTF_NEXT_END)
	  return CTF_ERR;			/* errno is set for us.  */
	break;
      }

    case CTF_K_ENUM:
      if (dst_type != CTF_ERR && kind != CTF_K_FORWARD
	  && dst_kind != CTF_K_FORWARD)
	{
	  if (ctf_enum_iter (src_fp, src_type, enumcmp, &dst)
	      || ctf_enum_iter (dst_fp, dst_type, enumcmp, &src))
	    {
	      ctf_err_warn (dst_fp, 1, ECTF_CONFLICT,
			    _("conflict for enum %s against ID %lx: members "
			      "differ, see above"), name, dst_type);
	      return (ctf_set_errno (dst_fp, ECTF_CONFLICT));
	    }
	}
      else
	{
	  dst_type = ctf_add_enum (dst_fp, flag, name);
	  if ((dst.ctb_type = dst_type) == CTF_ERR
	      || ctf_enum_iter (src_fp, src_type, enumadd, &dst))
	    return CTF_ERR;			/* errno is set for us */
	}
      break;

    case CTF_K_FORWARD:
      if (dst_type == CTF_ERR)
	  dst_type = ctf_add_forward (dst_fp, flag, name, forward_kind);
      break;

    case CTF_K_TYPEDEF:
      src_type = ctf_type_reference (src_fp, src_type);
      src_type = ctf_add_type_internal (dst_fp, src_fp, src_type,
					proc_tracking_fp);

      if (src_type == CTF_ERR)
	return CTF_ERR;				/* errno is set for us.  */

      /* If dst_type is not CTF_ERR at this point, we should check if
	 ctf_type_reference(dst_fp, dst_type) != src_type and if so fail with
	 ECTF_CONFLICT.  However, this causes problems with bitness typedefs
	 that vary based on things like if 32-bit then pid_t is int otherwise
	 long.  We therefore omit this check and assume that if the identically
	 named typedef already exists in dst_fp, it is correct or
	 equivalent.  */

      if (dst_type == CTF_ERR)
	  dst_type = ctf_add_typedef (dst_fp, flag, name, src_type);

      break;

    default:
      return (ctf_set_errno (dst_fp, ECTF_CORRUPT));
    }

  if (dst_type != CTF_ERR)
    ctf_add_type_mapping (src_fp, orig_src_type, dst_fp, dst_type);
  return dst_type;
}

ctf_id_t
ctf_add_type (ctf_dict_t *dst_fp, ctf_dict_t *src_fp, ctf_id_t src_type)
{
  ctf_id_t id;

  if (!src_fp->ctf_add_processing)
    src_fp->ctf_add_processing = ctf_dynhash_create (ctf_hash_integer,
						     ctf_hash_eq_integer,
						     NULL, NULL);

  /* We store the hash on the source, because it contains only source type IDs:
     but callers will invariably expect errors to appear on the dest.  */
  if (!src_fp->ctf_add_processing)
    return (ctf_set_errno (dst_fp, ENOMEM));

  id = ctf_add_type_internal (dst_fp, src_fp, src_type, src_fp);
  ctf_dynhash_empty (src_fp->ctf_add_processing);

  return id;
}
