/* CTF linking.
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

#if defined (PIC)
#pragma weak ctf_open
#endif

/* CTF linking consists of adding CTF archives full of content to be merged into
   this one to the current file (which must be writable) by calling
   ctf_link_add_ctf.  Once this is done, a call to ctf_link will merge the type
   tables together, generating new CTF files as needed, with this one as a
   parent, to contain types from the inputs which conflict.  ctf_link_add_strtab
   takes a callback which provides string/offset pairs to be added to the
   external symbol table and deduplicated from all CTF string tables in the
   output link; ctf_link_shuffle_syms takes a callback which provides symtab
   entries in ascending order, and shuffles the function and data sections to
   match; and ctf_link_write emits a CTF file (if there are no conflicts
   requiring per-compilation-unit sub-CTF files) or CTF archives (otherwise) and
   returns it, suitable for addition in the .ctf section of the output.  */

/* Return the name of the compilation unit this CTF dict or its parent applies
   to, or a non-null string otherwise: prefer the parent.  Used in debugging
   output.  Sometimes used for outputs too.  */
const char *
ctf_link_input_name (ctf_dict_t *fp)
{
  if (fp->ctf_parent && fp->ctf_parent->ctf_cuname)
    return fp->ctf_parent->ctf_cuname;
  else if (fp->ctf_cuname)
    return fp->ctf_cuname;
  else
    return "(unnamed)";
}

/* Return the cuname of a dict, or the string "unnamed-CU" if none.  */

static const char *
ctf_unnamed_cuname (ctf_dict_t *fp)
{
  const char *cuname = ctf_cuname (fp);

  if (!cuname)
    cuname = "unnamed-CU";

  return cuname;
}

/* The linker inputs look like this.  clin_fp is used for short-circuited
   CU-mapped links that can entirely avoid the first link phase in some
   situations in favour of just passing on the contained ctf_dict_t: it is
   always the sole ctf_dict_t inside the corresponding clin_arc.  If set, it
   gets assigned directly to the final link inputs and freed from there, so it
   never gets explicitly freed in the ctf_link_input.  */
typedef struct ctf_link_input
{
  char *clin_filename;
  ctf_archive_t *clin_arc;
  ctf_dict_t *clin_fp;
  int n;
} ctf_link_input_t;

static void
ctf_link_input_close (void *input)
{
  ctf_link_input_t *i = (ctf_link_input_t *) input;
  if (i->clin_arc)
    ctf_arc_close (i->clin_arc);
  free (i->clin_filename);
  free (i);
}

/* Like ctf_link_add_ctf, below, but with no error-checking, so it can be called
   in the middle of an ongoing link.  */
static int
ctf_link_add_ctf_internal (ctf_dict_t *fp, ctf_archive_t *ctf,
			   ctf_dict_t *fp_input, const char *name)
{
  int existing = 0;
  ctf_link_input_t *input;
  char *filename, *keyname;

  /* Existing: return it, or (if a different dict with the same name
     is already there) make up a new unique name.  Always use the actual name
     for the filename, because that needs to be ctf_open()ed.  */

  if ((input = ctf_dynhash_lookup (fp->ctf_link_inputs, name)) != NULL)
    {
      if ((fp_input != NULL && (input->clin_fp == fp_input))
	  || (ctf != NULL && (input->clin_arc == ctf)))
	return 0;
      existing = 1;
    }

  if ((filename = strdup (name)) == NULL)
    goto oom;

  if ((input = calloc (1, sizeof (ctf_link_input_t))) == NULL)
    goto oom1;

  input->clin_arc = ctf;
  input->clin_fp = fp_input;
  input->clin_filename = filename;
  input->n = ctf_dynhash_elements (fp->ctf_link_inputs);

  if (existing)
    {
      if (asprintf (&keyname, "%s#%li", name, (long int)
		    ctf_dynhash_elements (fp->ctf_link_inputs)) < 0)
	goto oom2;
    }
  else if ((keyname = strdup (name)) == NULL)
    goto oom2;

  if (ctf_dynhash_insert (fp->ctf_link_inputs, keyname, input) < 0)
    goto oom3;

  return 0;

 oom3:
  free (keyname);
 oom2:
  free (input);
 oom1:
  free (filename);
 oom:
  return ctf_set_errno (fp, ENOMEM);
}

/* Add a file, memory buffer, or unopened file (by name) to a link.

   You can call this with:

    CTF and NAME: link the passed ctf_archive_t, with the given NAME.
    NAME alone: open NAME as a CTF file when needed.
    BUF and NAME: open the BUF (of length N) as CTF, with the given NAME.  (Not
    yet implemented.)

    Passed in CTF args are owned by the dictionary and will be freed by it.
    The BUF arg is *not* owned by the dictionary, and the user should not free
    its referent until the link is done.

    The order of calls to this function influences the order of types in the
    final link output, but otherwise is not important.

    Repeated additions of the same NAME have no effect; repeated additions of
    different dicts with the same NAME add all the dicts with unique NAMEs
    derived from NAME.

    Private for now, but may in time become public once support for BUF is
    implemented.  */

static int
ctf_link_add (ctf_dict_t *fp, ctf_archive_t *ctf, const char *name,
	      void *buf _libctf_unused_, size_t n _libctf_unused_)
{
  if (buf)
    return (ctf_set_errno (fp, ECTF_NOTYET));

  if (!((ctf && name && !buf)
	|| (name && !buf && !ctf)
	|| (buf && name && !ctf)))
    return (ctf_set_errno (fp, EINVAL));

  /* We can only lazily open files if libctf.so is in use rather than
     libctf-nobfd.so.  This is a little tricky: in shared libraries, we can use
     a weak symbol so that -lctf -lctf-nobfd works, but in static libraries we
     must distinguish between the two libraries explicitly.  */

#if defined (PIC)
  if (!buf && !ctf && name && !ctf_open)
    return (ctf_set_errno (fp, ECTF_NEEDSBFD));
#elif NOBFD
  if (!buf && !ctf && name)
    return (ctf_set_errno (fp, ECTF_NEEDSBFD));
#endif

  if (fp->ctf_link_outputs)
    return (ctf_set_errno (fp, ECTF_LINKADDEDLATE));
  if (fp->ctf_link_inputs == NULL)
    fp->ctf_link_inputs = ctf_dynhash_create (ctf_hash_string,
					      ctf_hash_eq_string, free,
					      ctf_link_input_close);

  if (fp->ctf_link_inputs == NULL)
    return (ctf_set_errno (fp, ENOMEM));

  return ctf_link_add_ctf_internal (fp, ctf, NULL, name);
}

/* Add an opened CTF archive or unopened file (by name) to a link.
   If CTF is NULL and NAME is non-null, an unopened file is meant:
   otherwise, the specified archive is assumed to have the given NAME.

    Passed in CTF args are owned by the dictionary and will be freed by it.

    The order of calls to this function influences the order of types in the
    final link output, but otherwise is not important.  */

int
ctf_link_add_ctf (ctf_dict_t *fp, ctf_archive_t *ctf, const char *name)
{
  return ctf_link_add (fp, ctf, name, NULL, 0);
}

/* Lazily open a CTF archive for linking, if not already open.

   Returns the number of files contained within the opened archive (0 for none),
   or -1 on error, as usual.  */
static ssize_t
ctf_link_lazy_open (ctf_dict_t *fp, ctf_link_input_t *input)
{
  size_t count;
  int err;

  if (input->clin_arc)
    return ctf_archive_count (input->clin_arc);

  if (input->clin_fp)
    return 1;

  /* See ctf_link_add_ctf.  */
#if defined (PIC) || !NOBFD
  input->clin_arc = ctf_open (input->clin_filename, NULL, &err);
#else
  ctf_err_warn (fp, 0, ECTF_NEEDSBFD, _("cannot open %s lazily"),
		input->clin_filename);
  ctf_set_errno (fp, ECTF_NEEDSBFD);
  return -1;
#endif

  /* Having no CTF sections is not an error.  We just don't need to do
     anything.  */

  if (!input->clin_arc)
    {
      if (err == ECTF_NOCTFDATA)
	return 0;

      ctf_err_warn (fp, 0, err, _("opening CTF %s failed"),
		    input->clin_filename);
      ctf_set_errno (fp, err);
      return -1;
    }

  if ((count = ctf_archive_count (input->clin_arc)) == 0)
    ctf_arc_close (input->clin_arc);

  return (ssize_t) count;
}

/* Find a non-clashing unique name for a per-CU output dict, to prevent distinct
   members corresponding to inputs with identical cunames from overwriting each
   other.  The name should be something like NAME.  */

static char *
ctf_new_per_cu_name (ctf_dict_t *fp, const char *name)
{
  char *dynname;
  long int i = 0;

  if ((dynname = strdup (name)) == NULL)
    return NULL;

  while ((ctf_dynhash_lookup (fp->ctf_link_outputs, dynname)) != NULL)
    {
      free (dynname);
      if (asprintf (&dynname, "%s#%li", name, i++) < 0)
	return NULL;
    }

  return dynname;
}

/* Return a per-CU output CTF dictionary suitable for the given INPUT or CU,
   creating and interning it if need be.  */

static ctf_dict_t *
ctf_create_per_cu (ctf_dict_t *fp, ctf_dict_t *input, const char *cu_name)
{
  ctf_dict_t *cu_fp;
  const char *ctf_name = NULL;
  char *dynname = NULL;

  /* Already has a per-CU mapping?  Just return it.  */

  if (input && input->ctf_link_in_out)
    return input->ctf_link_in_out;

  /* Check the mapping table and translate the per-CU name we use
     accordingly.  */

  if (cu_name == NULL)
    cu_name = ctf_unnamed_cuname (input);

  if (fp->ctf_link_in_cu_mapping)
    {
      if ((ctf_name = ctf_dynhash_lookup (fp->ctf_link_in_cu_mapping,
					  cu_name)) == NULL)
	ctf_name = cu_name;
    }

  if (ctf_name == NULL)
    ctf_name = cu_name;

  /* Look up the per-CU dict.  If we don't know of one, or it is for a different input
     CU which just happens to have the same name, create a new one.  If we are creating
     a dict with no input specified, anything will do.  */

  if ((cu_fp = ctf_dynhash_lookup (fp->ctf_link_outputs, ctf_name)) == NULL
      || (input && cu_fp->ctf_link_in_out != fp))
    {
      int err;

      if ((cu_fp = ctf_create (&err)) == NULL)
	{
	  ctf_err_warn (fp, 0, err, _("cannot create per-CU CTF archive for "
				      "input CU %s"), cu_name);
	  ctf_set_errno (fp, err);
	  return NULL;
	}

      ctf_import_unref (cu_fp, fp);

      if ((dynname = ctf_new_per_cu_name (fp, ctf_name)) == NULL)
	goto oom;

      ctf_cuname_set (cu_fp, cu_name);

      ctf_parent_name_set (cu_fp, _CTF_SECTION);
      cu_fp->ctf_link_in_out = fp;
      fp->ctf_link_in_out = cu_fp;

      if (ctf_dynhash_insert (fp->ctf_link_outputs, dynname, cu_fp) < 0)
	goto oom;
    }
  return cu_fp;

 oom:
  free (dynname);
  ctf_dict_close (cu_fp);
  ctf_set_errno (fp, ENOMEM);
  return NULL;
}

/* Add a mapping directing that the CU named FROM should have its
   conflicting/non-duplicate types (depending on link mode) go into a dict
   named TO.  Many FROMs can share a TO.

   We forcibly add a dict named TO in every case, even though it may well
   wind up empty, because clients that use this facility usually expect to find
   every TO dict present, even if empty, and malfunction otherwise.  */

int
ctf_link_add_cu_mapping (ctf_dict_t *fp, const char *from, const char *to)
{
  int err;
  char *f = NULL, *t = NULL;
  ctf_dynhash_t *one_out;

  /* Mappings cannot be set up if per-CU output dicts already exist.  */
  if (fp->ctf_link_outputs && ctf_dynhash_elements (fp->ctf_link_outputs) != 0)
      return (ctf_set_errno (fp, ECTF_LINKADDEDLATE));

  if (fp->ctf_link_in_cu_mapping == NULL)
    fp->ctf_link_in_cu_mapping = ctf_dynhash_create (ctf_hash_string,
						     ctf_hash_eq_string, free,
						     free);
  if (fp->ctf_link_in_cu_mapping == NULL)
    goto oom;

  if (fp->ctf_link_out_cu_mapping == NULL)
    fp->ctf_link_out_cu_mapping = ctf_dynhash_create (ctf_hash_string,
						      ctf_hash_eq_string, free,
						      (ctf_hash_free_fun)
						      ctf_dynhash_destroy);
  if (fp->ctf_link_out_cu_mapping == NULL)
    goto oom;

  f = strdup (from);
  t = strdup (to);
  if (!f || !t)
    goto oom;

  /* Track both in a list from FROM to TO and in a list from TO to a list of
     FROM.  The former is used to create TUs with the mapped-to name at need:
     the latter is used in deduplicating links to pull in all input CUs
     corresponding to a single output CU.  */

  if ((err = ctf_dynhash_insert (fp->ctf_link_in_cu_mapping, f, t)) < 0)
    {
      ctf_set_errno (fp, err);
      goto oom_noerrno;
    }

  /* f and t are now owned by the in_cu_mapping: reallocate them.  */
  f = strdup (from);
  t = strdup (to);
  if (!f || !t)
    goto oom;

  if ((one_out = ctf_dynhash_lookup (fp->ctf_link_out_cu_mapping, t)) == NULL)
    {
      if ((one_out = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
					 free, NULL)) == NULL)
	goto oom;
      if ((err = ctf_dynhash_insert (fp->ctf_link_out_cu_mapping,
				     t, one_out)) < 0)
	{
	  ctf_dynhash_destroy (one_out);
	  ctf_set_errno (fp, err);
	  goto oom_noerrno;
	}
    }
  else
    {
      free (t);
      t = NULL;
    }

  if (ctf_dynhash_insert (one_out, f, NULL) < 0)
    {
      ctf_set_errno (fp, err);
      goto oom_noerrno;
    }

  return 0;

 oom:
  ctf_set_errno (fp, errno);
 oom_noerrno:
  free (f);
  free (t);
  return -1;
}

/* Set a function which is called to transform the names of archive members.
   This is useful for applying regular transformations to many names, where
   ctf_link_add_cu_mapping applies arbitrarily irregular changes to single
   names.  The member name changer is applied at ctf_link_write time, so it
   cannot conflate multiple CUs into one the way ctf_link_add_cu_mapping can.
   The changer function accepts a name and should return a new
   dynamically-allocated name, or NULL if the name should be left unchanged.  */
void
ctf_link_set_memb_name_changer (ctf_dict_t *fp,
				ctf_link_memb_name_changer_f *changer,
				void *arg)
{
  fp->ctf_link_memb_name_changer = changer;
  fp->ctf_link_memb_name_changer_arg = arg;
}

/* Set a function which is used to filter out unwanted variables from the link.  */
int
ctf_link_set_variable_filter (ctf_dict_t *fp, ctf_link_variable_filter_f *filter,
			      void *arg)
{
  fp->ctf_link_variable_filter = filter;
  fp->ctf_link_variable_filter_arg = arg;
  return 0;
}

/* Check if we can safely add a variable with the given type to this dict.  */

static int
check_variable (const char *name, ctf_dict_t *fp, ctf_id_t type,
		ctf_dvdef_t **out_dvd)
{
  ctf_dvdef_t *dvd;

  dvd = ctf_dynhash_lookup (fp->ctf_dvhash, name);
  *out_dvd = dvd;
  if (!dvd)
    return 1;

  if (dvd->dvd_type != type)
    {
      /* Variable here.  Wrong type: cannot add.  Just skip it, because there is
	 no way to express this in CTF.  Don't even warn: this case is too
	 common.  (This might be the parent, in which case we'll try adding in
	 the child first, and only then give up.)  */
      ctf_dprintf ("Inexpressible duplicate variable %s skipped.\n", name);
    }

  return 0;				      /* Already exists.  */
}

/* Link one variable named NAME of type TYPE found in IN_FP into FP.  */

static int
ctf_link_one_variable (ctf_dict_t *fp, ctf_dict_t *in_fp, const char *name,
		       ctf_id_t type, int cu_mapped)
{
  ctf_dict_t *per_cu_out_fp;
  ctf_id_t dst_type = 0;
  ctf_dvdef_t *dvd;

  /* See if this variable is filtered out.  */

  if (fp->ctf_link_variable_filter)
    {
      void *farg = fp->ctf_link_variable_filter_arg;
      if (fp->ctf_link_variable_filter (in_fp, name, type, farg))
	return 0;
    }

  /* If this type is mapped to a type in the parent dict, we want to try to add
     to that first: if it reports a duplicate, or if the type is in a child
     already, add straight to the child.  */

  if ((dst_type = ctf_dedup_type_mapping (fp, in_fp, type)) == CTF_ERR)
    return -1;					/* errno is set for us.  */

  if (dst_type != 0)
    {
      if (!ctf_assert (fp, ctf_type_isparent (fp, dst_type)))
	return -1;				/* errno is set for us.  */

      if (check_variable (name, fp, dst_type, &dvd))
	{
	  /* No variable here: we can add it.  */
	  if (ctf_add_variable (fp, name, dst_type) < 0)
	    return -1; 				/* errno is set for us.  */
	  return 0;
	}

      /* Already present?  Nothing to do.  */
      if (dvd && dvd->dvd_type == dst_type)
	return 0;
    }

  /* Can't add to the parent due to a name clash, or because it references a
     type only present in the child.  Try adding to the child, creating if need
     be.  If we can't do that, skip it.  Don't add to a child if we're doing a
     CU-mapped link, since that has only one output.  */

  if (cu_mapped)
    {
      ctf_dprintf ("Variable %s in input file %s depends on a type %lx hidden "
		   "due to conflicts: skipped.\n", name,
		   ctf_unnamed_cuname (in_fp), type);
      return 0;
    }

  if ((per_cu_out_fp = ctf_create_per_cu (fp, in_fp, NULL)) == NULL)
    return -1;					/* errno is set for us.  */

  /* If the type was not found, check for it in the child too.  */
  if (dst_type == 0)
    {
      if ((dst_type = ctf_dedup_type_mapping (per_cu_out_fp,
					      in_fp, type)) == CTF_ERR)
	return -1;				/* errno is set for us.   */

      if (dst_type == 0)
	{
	  ctf_err_warn (fp, 1, 0, _("type %lx for variable %s in input file %s "
				    "not found: skipped"), type, name,
			ctf_unnamed_cuname (in_fp));
	  /* Do not terminate the link: just skip the variable.  */
	  return 0;
	}
    }

  if (check_variable (name, per_cu_out_fp, dst_type, &dvd))
    if (ctf_add_variable (per_cu_out_fp, name, dst_type) < 0)
      return (ctf_set_errno (fp, ctf_errno (per_cu_out_fp)));
  return 0;
}

typedef struct link_sort_inputs_cb_arg
{
  int is_cu_mapped;
  ctf_dict_t *fp;
} link_sort_inputs_cb_arg_t;

/* Sort the inputs by N (the link order).  For CU-mapped links, this is a
   mapping of input to output name, not a mapping of input name to input
   ctf_link_input_t: compensate accordingly.  */
static int
ctf_link_sort_inputs (const ctf_next_hkv_t *one, const ctf_next_hkv_t *two,
		      void *arg)
{
  ctf_link_input_t *input_1;
  ctf_link_input_t *input_2;
  link_sort_inputs_cb_arg_t *cu_mapped = (link_sort_inputs_cb_arg_t *) arg;

  if (!cu_mapped || !cu_mapped->is_cu_mapped)
    {
      input_1 = (ctf_link_input_t *) one->hkv_value;
      input_2 = (ctf_link_input_t *) two->hkv_value;
    }
  else
    {
      const char *name_1 = (const char *) one->hkv_key;
      const char *name_2 = (const char *) two->hkv_key;

      input_1 = ctf_dynhash_lookup (cu_mapped->fp->ctf_link_inputs, name_1);
      input_2 = ctf_dynhash_lookup (cu_mapped->fp->ctf_link_inputs, name_2);

      /* There is no guarantee that CU-mappings actually have corresponding
	 inputs: the relative ordering in that case is unimportant.  */
      if (!input_1)
	return -1;
      if (!input_2)
	return 1;
    }

  if (input_1->n < input_2->n)
    return -1;
  else if (input_1->n > input_2->n)
    return 1;
  else
    return 0;
}

/* Count the number of input dicts in the ctf_link_inputs, or that subset of the
   ctf_link_inputs given by CU_NAMES if set.  Return the number of input dicts,
   and optionally the name and ctf_link_input_t of the single input archive if
   only one exists (no matter how many dicts it contains).  */
static ssize_t
ctf_link_deduplicating_count_inputs (ctf_dict_t *fp, ctf_dynhash_t *cu_names,
				     ctf_link_input_t **only_one_input)
{
  ctf_dynhash_t *inputs = fp->ctf_link_inputs;
  ctf_next_t *i = NULL;
  void *name, *input;
  ctf_link_input_t *one_input = NULL;
  const char *one_name = NULL;
  ssize_t count = 0, narcs = 0;
  int err;

  if (cu_names)
    inputs = cu_names;

  while ((err = ctf_dynhash_next (inputs, &i, &name, &input)) == 0)
    {
      ssize_t one_count;

      one_name = (const char *) name;
      /* If we are processing CU names, get the real input.  */
      if (cu_names)
	one_input = ctf_dynhash_lookup (fp->ctf_link_inputs, one_name);
      else
	one_input = (ctf_link_input_t *) input;

      if (!one_input)
	continue;

      one_count = ctf_link_lazy_open (fp, one_input);

      if (one_count < 0)
	{
	  ctf_next_destroy (i);
	  return -1;				/* errno is set for us.  */
	}

      count += one_count;
      narcs++;
    }
  if (err != ECTF_NEXT_END)
    {
      ctf_err_warn (fp, 0, err, _("iteration error counting deduplicating "
				  "CTF link inputs"));
      ctf_set_errno (fp, err);
      return -1;
    }

  if (!count)
    return 0;

  if (narcs == 1)
    {
      if (only_one_input)
	*only_one_input = one_input;
    }
  else if (only_one_input)
    *only_one_input = NULL;

  return count;
}

/* Allocate and populate an inputs array big enough for a given set of inputs:
   either a specific set of CU names (those from that set found in the
   ctf_link_inputs), or the entire ctf_link_inputs (if cu_names is not set).
   The number of inputs (from ctf_link_deduplicating_count_inputs, above) is
   passed in NINPUTS: an array of uint32_t containing parent pointers
   (corresponding to those members of the inputs that have parents) is allocated
   and returned in PARENTS.

   The inputs are *archives*, not files: the archive can have multiple members
   if it is the result of a previous incremental link.  We want to add every one
   in turn, including the shared parent.  (The dedup machinery knows that a type
   used by a single dictionary and its parent should not be shared in
   CTF_LINK_SHARE_DUPLICATED mode.)

   If no inputs exist that correspond to these CUs, return NULL with the errno
   set to ECTF_NOCTFDATA.  */
static ctf_dict_t **
ctf_link_deduplicating_open_inputs (ctf_dict_t *fp, ctf_dynhash_t *cu_names,
				    ssize_t ninputs, uint32_t **parents)
{
  ctf_dynhash_t *inputs = fp->ctf_link_inputs;
  ctf_next_t *i = NULL;
  void *name, *input;
  link_sort_inputs_cb_arg_t sort_arg;
  ctf_dict_t **dedup_inputs = NULL;
  ctf_dict_t **walk;
  uint32_t *parents_ = NULL;
  int err;

  if (cu_names)
    inputs = cu_names;

  if ((dedup_inputs = calloc (ninputs, sizeof (ctf_dict_t *))) == NULL)
    goto oom;

  if ((parents_ = calloc (ninputs, sizeof (uint32_t))) == NULL)
    goto oom;

  walk = dedup_inputs;

  /* Counting done: push every input into the array, in the order they were
     passed to ctf_link_add_ctf (and ultimately ld).  */

  sort_arg.is_cu_mapped = (cu_names != NULL);
  sort_arg.fp = fp;

  while ((err = ctf_dynhash_next_sorted (inputs, &i, &name, &input,
					 ctf_link_sort_inputs, &sort_arg)) == 0)
    {
      const char *one_name = (const char *) name;
      ctf_link_input_t *one_input;
      ctf_dict_t *one_fp;
      ctf_dict_t *parent_fp = NULL;
      uint32_t parent_i;
      ctf_next_t *j = NULL;

      /* If we are processing CU names, get the real input.  All the inputs
	 will have been opened, if they contained any CTF at all.  */
      if (cu_names)
	one_input = ctf_dynhash_lookup (fp->ctf_link_inputs, one_name);
      else
	one_input = (ctf_link_input_t *) input;

      if (!one_input || (!one_input->clin_arc && !one_input->clin_fp))
	continue;

      /* Short-circuit: if clin_fp is set, just use it.   */
      if (one_input->clin_fp)
	{
	  parents_[walk - dedup_inputs] = walk - dedup_inputs;
	  *walk = one_input->clin_fp;
	  walk++;
	  continue;
	}

      /* Get and insert the parent archive (if any), if this archive has
	 multiple members.  We assume, as elsewhere, that the parent is named
	 _CTF_SECTION.  */

      if ((parent_fp = ctf_dict_open (one_input->clin_arc, _CTF_SECTION,
				      &err)) == NULL)
	{
	  if (err != ECTF_NOMEMBNAM)
	    {
	      ctf_next_destroy (i);
	      ctf_set_errno (fp, err);
	      goto err;
	    }
	}
      else
	{
	  *walk = parent_fp;
	  parent_i = walk - dedup_inputs;
	  walk++;
	}

      /* We disregard the input archive name: either it is the parent (which we
	 already have), or we want to put everything into one TU sharing the
	 cuname anyway (if this is a CU-mapped link), or this is the final phase
	 of a relink with CU-mapping off (i.e. ld -r) in which case the cuname
	 is correctly set regardless.  */
      while ((one_fp = ctf_archive_next (one_input->clin_arc, &j, NULL,
					 1, &err)) != NULL)
	{
	  if (one_fp->ctf_flags & LCTF_CHILD)
	    {
	      /* The contents of the parents array for elements not
		 corresponding to children is undefined.  If there is no parent
		 (itself a sign of a likely linker bug or corrupt input), we set
		 it to itself.  */

	      ctf_import (one_fp, parent_fp);
	      if (parent_fp)
		parents_[walk - dedup_inputs] = parent_i;
	      else
		parents_[walk - dedup_inputs] = walk - dedup_inputs;
	    }
	  *walk = one_fp;
	  walk++;
	}
      if (err != ECTF_NEXT_END)
	{
	  ctf_next_destroy (i);
	  goto iterr;
	}
    }
  if (err != ECTF_NEXT_END)
    goto iterr;

  *parents = parents_;

  return dedup_inputs;

 oom:
  err = ENOMEM;

 iterr:
  ctf_set_errno (fp, err);

 err:
  free (dedup_inputs);
  free (parents_);
  ctf_err_warn (fp, 0, 0, _("error in deduplicating CTF link "
			    "input allocation"));
  return NULL;
}

/* Close INPUTS that have already been linked, first the passed array, and then
   that subset of the ctf_link_inputs archives they came from cited by the
   CU_NAMES.  If CU_NAMES is not specified, close all the ctf_link_inputs in one
   go, leaving it empty.  */
static int
ctf_link_deduplicating_close_inputs (ctf_dict_t *fp, ctf_dynhash_t *cu_names,
				     ctf_dict_t **inputs, ssize_t ninputs)
{
  ctf_next_t *it = NULL;
  void *name;
  int err;
  ssize_t i;

  /* This is the inverse of ctf_link_deduplicating_open_inputs: so first, close
     all the individual input dicts, opened by the archive iterator.  */
  for (i = 0; i < ninputs; i++)
    ctf_dict_close (inputs[i]);

  /* Now close the archives they are part of.  */
  if (cu_names)
    {
      while ((err = ctf_dynhash_next (cu_names, &it, &name, NULL)) == 0)
	{
	  /* Remove the input from the linker inputs, if it exists, which also
	     closes it.  */

	  ctf_dynhash_remove (fp->ctf_link_inputs, (const char *) name);
	}
      if (err != ECTF_NEXT_END)
	{
	  ctf_err_warn (fp, 0, err, _("iteration error in deduplicating link "
				      "input freeing"));
	  ctf_set_errno (fp, err);
	}
    }
  else
    ctf_dynhash_empty (fp->ctf_link_inputs);

  return 0;
}

/* Do a deduplicating link of all variables in the inputs.

   Also, if we are not omitting the variable section, integrate all symbols from
   the symtypetabs into the variable section too.  (Duplication with the
   symtypetab section in the output will be eliminated at serialization time.)  */

static int
ctf_link_deduplicating_variables (ctf_dict_t *fp, ctf_dict_t **inputs,
				  size_t ninputs, int cu_mapped)
{
  size_t i;

  for (i = 0; i < ninputs; i++)
    {
      ctf_next_t *it = NULL;
      ctf_id_t type;
      const char *name;

      /* First the variables on the inputs.  */

      while ((type = ctf_variable_next (inputs[i], &it, &name)) != CTF_ERR)
	{
	  if (ctf_link_one_variable (fp, inputs[i], name, type, cu_mapped) < 0)
	    {
	      ctf_next_destroy (it);
	      return -1;			/* errno is set for us.  */
	    }
	}
      if (ctf_errno (inputs[i]) != ECTF_NEXT_END)
	return ctf_set_errno (fp, ctf_errno (inputs[i]));

      /* Next the symbols.  We integrate data symbols even though the compiler
	 is currently doing the same, to allow the compiler to stop in
	 future.  */

      while ((type = ctf_symbol_next (inputs[i], &it, &name, 0)) != CTF_ERR)
	{
	  if (ctf_link_one_variable (fp, inputs[i], name, type, 1) < 0)
	    {
	      ctf_next_destroy (it);
	      return -1;			/* errno is set for us.  */
	    }
	}
      if (ctf_errno (inputs[i]) != ECTF_NEXT_END)
	return ctf_set_errno (fp, ctf_errno (inputs[i]));

      /* Finally the function symbols.  */

      while ((type = ctf_symbol_next (inputs[i], &it, &name, 1)) != CTF_ERR)
	{
	  if (ctf_link_one_variable (fp, inputs[i], name, type, 1) < 0)
	    {
	      ctf_next_destroy (it);
	      return -1;			/* errno is set for us.  */
	    }
	}
      if (ctf_errno (inputs[i]) != ECTF_NEXT_END)
	return ctf_set_errno (fp, ctf_errno (inputs[i]));
    }
  return 0;
}

/* Check for symbol conflicts during linking.  Three possibilities: already
   exists, conflicting, or nonexistent.  We don't have a dvd structure we can
   use as a flag like check_variable does, so we use a tristate return
   value instead: -1: conflicting; 1: nonexistent: 0: already exists.  */

static int
check_sym (ctf_dict_t *fp, const char *name, ctf_id_t type, int functions)
{
  ctf_dynhash_t *thishash = functions ? fp->ctf_funchash : fp->ctf_objthash;
  ctf_dynhash_t *thathash = functions ? fp->ctf_objthash : fp->ctf_funchash;
  void *value;

  /* Wrong type (function when object is wanted, etc).  */
  if (ctf_dynhash_lookup_kv (thathash, name, NULL, NULL))
    return -1;

  /* Not present at all yet.  */
  if (!ctf_dynhash_lookup_kv (thishash, name, NULL, &value))
    return 1;

  /* Already present.  */
  if ((ctf_id_t) (uintptr_t) value == type)
    return 0;

  /* Wrong type.  */
  return -1;
}

/* Do a deduplicating link of one symtypetab (function info or data object) in
   one input dict.  */

static int
ctf_link_deduplicating_one_symtypetab (ctf_dict_t *fp, ctf_dict_t *input,
				       int cu_mapped, int functions)
{
  ctf_next_t *it = NULL;
  const char *name;
  ctf_id_t type;

  while ((type = ctf_symbol_next (input, &it, &name, functions)) != CTF_ERR)
    {
      ctf_id_t dst_type;
      ctf_dict_t *per_cu_out_fp;
      int sym;

      /* Look in the parent first.  */

      if ((dst_type = ctf_dedup_type_mapping (fp, input, type)) == CTF_ERR)
	return -1;				/* errno is set for us.  */

      if (dst_type != 0)
	{
	  if (!ctf_assert (fp, ctf_type_isparent (fp, dst_type)))
	    return -1;				/* errno is set for us.  */

	  sym = check_sym (fp, name, dst_type, functions);

	  /* Already present: next symbol.  */
	  if (sym == 0)
	    continue;
	  /* Not present: add it.  */
	  else if (sym > 0)
	    {
	      if (ctf_add_funcobjt_sym (fp, functions,
					name, dst_type) < 0)
		return -1; 			/* errno is set for us.  */
	      continue;
	    }
	}

      /* Can't add to the parent due to a name clash (most unlikely), or because
	 it references a type only present in the child.  Try adding to the
	 child, creating if need be.  If we can't do that, skip it.  Don't add
	 to a child if we're doing a CU-mapped link, since that has only one
	 output.  */
      if (cu_mapped)
	{
	  ctf_dprintf ("Symbol %s in input file %s depends on a type %lx "
		       "hidden due to conflicts: skipped.\n", name,
		       ctf_unnamed_cuname (input), type);
	  continue;
	}

      if ((per_cu_out_fp = ctf_create_per_cu (fp, input, NULL)) == NULL)
	return -1;				/* errno is set for us.  */

      /* If the type was not found, check for it in the child too.  */
      if (dst_type == 0)
	{
	  if ((dst_type = ctf_dedup_type_mapping (per_cu_out_fp,
						  input, type)) == CTF_ERR)
	    return -1;				/* errno is set for us.  */

	  if (dst_type == 0)
	    {
	      ctf_err_warn (fp, 1, 0,
			    _("type %lx for symbol %s in input file %s "
			      "not found: skipped"), type, name,
			    ctf_unnamed_cuname (input));
	      continue;
	    }
	}

      sym = check_sym (per_cu_out_fp, name, dst_type, functions);

      /* Already present: next symbol.  */
      if (sym == 0)
	continue;
      /* Not present: add it.  */
      else if (sym > 0)
	{
	  if (ctf_add_funcobjt_sym (per_cu_out_fp, functions,
				    name, dst_type) < 0)
	    return -1;				/* errno is set for us.  */
	}
      else
	{
	  /* Perhaps this should be an assertion failure.  */
	  ctf_err_warn (fp, 0, ECTF_DUPLICATE,
			_("symbol %s in input file %s found conflicting "
			  "even when trying in per-CU dict."), name,
			ctf_unnamed_cuname (input));
	  return (ctf_set_errno (fp, ECTF_DUPLICATE));
	}
    }
  if (ctf_errno (input) != ECTF_NEXT_END)
    {
      ctf_set_errno (fp, ctf_errno (input));
      ctf_err_warn (fp, 0, ctf_errno (input),
		    functions ? _("iterating over function symbols") :
		    _("iterating over data symbols"));
      return -1;
    }

  return 0;
}

/* Do a deduplicating link of the function info and data objects
   in the inputs.  */
static int
ctf_link_deduplicating_syms (ctf_dict_t *fp, ctf_dict_t **inputs,
			     size_t ninputs, int cu_mapped)
{
  size_t i;

  for (i = 0; i < ninputs; i++)
    {
      if (ctf_link_deduplicating_one_symtypetab (fp, inputs[i],
						 cu_mapped, 0) < 0)
	return -1;				/* errno is set for us.  */

      if (ctf_link_deduplicating_one_symtypetab (fp, inputs[i],
						 cu_mapped, 1) < 0)
	return -1;				/* errno is set for us.  */
    }

  return 0;
}

/* Do the per-CU part of a deduplicating link.  */
static int
ctf_link_deduplicating_per_cu (ctf_dict_t *fp)
{
  ctf_next_t *i = NULL;
  int err;
  void *out_cu;
  void *in_cus;

  /* Links with a per-CU mapping in force get a first pass of deduplication,
     dedupping the inputs for a given CU mapping into the output for that
     mapping.  The outputs from this process get fed back into the final pass
     that is carried out even for non-CU links.  */

  while ((err = ctf_dynhash_next (fp->ctf_link_out_cu_mapping, &i, &out_cu,
				  &in_cus)) == 0)
    {
      const char *out_name = (const char *) out_cu;
      ctf_dynhash_t *in = (ctf_dynhash_t *) in_cus;
      ctf_dict_t *out = NULL;
      ctf_dict_t **inputs;
      ctf_dict_t **outputs;
      ctf_archive_t *in_arc;
      ssize_t ninputs;
      ctf_link_input_t *only_input;
      uint32_t noutputs;
      uint32_t *parents;

      if ((ninputs = ctf_link_deduplicating_count_inputs (fp, in,
							  &only_input)) == -1)
	goto err_open_inputs;

      /* CU mapping with no inputs?  Skip.  */
      if (ninputs == 0)
	continue;

      if (labs ((long int) ninputs) > 0xfffffffe)
	{
	  ctf_err_warn (fp, 0, EFBIG, _("too many inputs in deduplicating "
					"link: %li"), (long int) ninputs);
	  ctf_set_errno (fp, EFBIG);
	  goto err_open_inputs;
	}

      /* Short-circuit: a cu-mapped link with only one input archive with
	 unconflicting contents is a do-nothing, and we can just leave the input
	 in place: we do have to change the cuname, though, so we unwrap it,
	 change the cuname, then stuff it back in the linker input again, via
	 the clin_fp short-circuit member.  ctf_link_deduplicating_open_inputs
	 will spot this member and jam it straight into the next link phase,
	 ignoring the corresponding archive.  */
      if (only_input && ninputs == 1)
	{
	  ctf_next_t *ai = NULL;
	  int err;

	  /* We can abuse an archive iterator to get the only member cheaply, no
	     matter what its name.  */
	  only_input->clin_fp = ctf_archive_next (only_input->clin_arc,
						  &ai, NULL, 0, &err);
	  if (!only_input->clin_fp)
	    {
	      ctf_err_warn (fp, 0, err, _("cannot open archive %s in "
					  "CU-mapped CTF link"),
			    only_input->clin_filename);
	      ctf_set_errno (fp, err);
	      goto err_open_inputs;
	    }
	  ctf_next_destroy (ai);

	  if (strcmp (only_input->clin_filename, out_name) != 0)
	    {
	      /* Renaming. We need to add a new input, then null out the
		 clin_arc and clin_fp of the old one to stop it being
		 auto-closed on removal.  The new input needs its cuname changed
		 to out_name, which is doable only because the cuname is a
		 dynamic property which can be changed even in readonly
		 dicts. */

	      ctf_cuname_set (only_input->clin_fp, out_name);
	      if (ctf_link_add_ctf_internal (fp, only_input->clin_arc,
					     only_input->clin_fp,
					     out_name) < 0)
		{
		  ctf_err_warn (fp, 0, 0, _("cannot add intermediate files "
					    "to link"));
		  goto err_open_inputs;
		}
	      only_input->clin_arc = NULL;
	      only_input->clin_fp = NULL;
	      ctf_dynhash_remove (fp->ctf_link_inputs,
				  only_input->clin_filename);
	    }
	  continue;
	}

      /* This is a real CU many-to-one mapping: we must dedup the inputs into
	 a new output to be used in the final link phase.  */

      if ((inputs = ctf_link_deduplicating_open_inputs (fp, in, ninputs,
							&parents)) == NULL)
	{
	  ctf_next_destroy (i);
	  goto err_inputs;
	}

      if ((out = ctf_create (&err)) == NULL)
	{
	  ctf_err_warn (fp, 0, err, _("cannot create per-CU CTF archive "
				      "for %s"),
			out_name);
	  ctf_set_errno (fp, err);
	  goto err_inputs;
	}

      /* Share the atoms table to reduce memory usage.  */
      out->ctf_dedup_atoms = fp->ctf_dedup_atoms_alloc;

      /* No ctf_imports at this stage: this per-CU dictionary has no parents.
	 Parent/child deduplication happens in the link's final pass.  However,
	 the cuname *is* important, as it is propagated into the final
	 dictionary.  */
      ctf_cuname_set (out, out_name);

      if (ctf_dedup (out, inputs, ninputs, parents, 1) < 0)
	{
	  ctf_set_errno (fp, ctf_errno (out));
	  ctf_err_warn (fp, 0, 0, _("CU-mapped deduplication failed for %s"),
			out_name);
	  goto err_inputs;
	}

      if ((outputs = ctf_dedup_emit (out, inputs, ninputs, parents,
				     &noutputs, 1)) == NULL)
	{
	  ctf_set_errno (fp, ctf_errno (out));
	  ctf_err_warn (fp, 0, 0, _("CU-mapped deduplicating link type emission "
				     "failed for %s"), out_name);
	  goto err_inputs;
	}
      if (!ctf_assert (fp, noutputs == 1))
	{
	  size_t j;
	  for (j = 1; j < noutputs; j++)
	    ctf_dict_close (outputs[j]);
	  goto err_inputs_outputs;
	}

      if (!(fp->ctf_link_flags & CTF_LINK_OMIT_VARIABLES_SECTION)
	  && ctf_link_deduplicating_variables (out, inputs, ninputs, 1) < 0)
	{
	  ctf_set_errno (fp, ctf_errno (out));
	  ctf_err_warn (fp, 0, 0, _("CU-mapped deduplicating link variable "
				    "emission failed for %s"), out_name);
	  goto err_inputs_outputs;
	}

      ctf_dedup_fini (out, outputs, noutputs);

      /* For now, we omit symbol section linking for CU-mapped links, until it
	 is clear how to unify the symbol table across such links.  (Perhaps we
	 should emit an unconditionally indexed symtab, like the compiler
	 does.)  */

      if (ctf_link_deduplicating_close_inputs (fp, in, inputs, ninputs) < 0)
	{
	  free (inputs);
	  free (parents);
	  goto err_outputs;
	}
      free (inputs);
      free (parents);

      /* Splice any errors or warnings created during this link back into the
	 dict that the caller knows about.  */
      ctf_list_splice (&fp->ctf_errs_warnings, &outputs[0]->ctf_errs_warnings);

      /* This output now becomes an input to the next link phase, with a name
	 equal to the CU name.  We have to wrap it in an archive wrapper
	 first.  */

      if ((in_arc = ctf_new_archive_internal (0, 0, NULL, outputs[0], NULL,
					      NULL, &err)) == NULL)
	{
	  ctf_set_errno (fp, err);
	  goto err_outputs;
	}

      if (ctf_link_add_ctf_internal (fp, in_arc, NULL,
				     ctf_cuname (outputs[0])) < 0)
	{
	  ctf_err_warn (fp, 0, 0, _("cannot add intermediate files to link"));
	  goto err_outputs;
	}

      ctf_dict_close (out);
      free (outputs);
      continue;

    err_inputs_outputs:
      ctf_list_splice (&fp->ctf_errs_warnings, &outputs[0]->ctf_errs_warnings);
      ctf_dict_close (outputs[0]);
      free (outputs);
    err_inputs:
      ctf_link_deduplicating_close_inputs (fp, in, inputs, ninputs);
      ctf_dict_close (out);
      free (inputs);
      free (parents);
    err_open_inputs:
      ctf_next_destroy (i);
      return -1;

    err_outputs:
      ctf_list_splice (&fp->ctf_errs_warnings, &outputs[0]->ctf_errs_warnings);
      ctf_dict_close (outputs[0]);
      free (outputs);
      ctf_next_destroy (i);
      return -1;				/* Errno is set for us.  */
    }
  if (err != ECTF_NEXT_END)
    {
      ctf_err_warn (fp, 0, err, _("iteration error in CU-mapped deduplicating "
				  "link"));
      return ctf_set_errno (fp, err);
    }

  return 0;
}

/* Empty all the ctf_link_outputs.  */
static int
ctf_link_empty_outputs (ctf_dict_t *fp)
{
  ctf_next_t *i = NULL;
  void *v;
  int err;

  ctf_dynhash_empty (fp->ctf_link_outputs);

  while ((err = ctf_dynhash_next (fp->ctf_link_inputs, &i, NULL, &v)) == 0)
    {
      ctf_dict_t *in = (ctf_dict_t *) v;
      in->ctf_link_in_out = NULL;
    }
  if (err != ECTF_NEXT_END)
    {
      fp->ctf_flags &= ~LCTF_LINKING;
      ctf_err_warn (fp, 1, err, _("iteration error removing old outputs"));
      ctf_set_errno (fp, err);
      return -1;
    }
  return 0;
}

/* Do a deduplicating link using the ctf-dedup machinery.  */
static void
ctf_link_deduplicating (ctf_dict_t *fp)
{
  size_t i;
  ctf_dict_t **inputs, **outputs = NULL;
  ssize_t ninputs;
  uint32_t noutputs;
  uint32_t *parents;

  if (ctf_dedup_atoms_init (fp) < 0)
    {
      ctf_err_warn (fp, 0, 0, _("allocating CTF dedup atoms table"));
      return;					/* Errno is set for us.  */
    }

  if (fp->ctf_link_out_cu_mapping
      && (ctf_link_deduplicating_per_cu (fp) < 0))
    return;					/* Errno is set for us.  */

  if ((ninputs = ctf_link_deduplicating_count_inputs (fp, NULL, NULL)) < 0)
    return;					/* Errno is set for us.  */

  if ((inputs = ctf_link_deduplicating_open_inputs (fp, NULL, ninputs,
						    &parents)) == NULL)
    return;					/* Errno is set for us.  */

  if (ninputs == 1 && ctf_cuname (inputs[0]) != NULL)
    ctf_cuname_set (fp, ctf_cuname (inputs[0]));

  if (ctf_dedup (fp, inputs, ninputs, parents, 0) < 0)
    {
      ctf_err_warn (fp, 0, 0, _("deduplication failed for %s"),
		    ctf_link_input_name (fp));
      goto err;
    }

  if ((outputs = ctf_dedup_emit (fp, inputs, ninputs, parents, &noutputs,
				 0)) == NULL)
    {
      ctf_err_warn (fp, 0, 0, _("deduplicating link type emission failed "
				"for %s"), ctf_link_input_name (fp));
      goto err;
    }

  if (!ctf_assert (fp, outputs[0] == fp))
    {
      for (i = 1; i < noutputs; i++)
	ctf_dict_close (outputs[i]);
      goto err;
    }

  for (i = 0; i < noutputs; i++)
    {
      char *dynname;

      /* We already have access to this one.  Close the duplicate.  */
      if (i == 0)
	{
	  ctf_dict_close (outputs[0]);
	  continue;
	}

      if ((dynname = ctf_new_per_cu_name (fp, ctf_cuname (outputs[i]))) == NULL)
	goto oom_one_output;

      if (ctf_dynhash_insert (fp->ctf_link_outputs, dynname, outputs[i]) < 0)
	goto oom_one_output;

      continue;

    oom_one_output:
      ctf_set_errno (fp, ENOMEM);
      ctf_err_warn (fp, 0, 0, _("out of memory allocating link outputs"));
      free (dynname);

      for (; i < noutputs; i++)
	ctf_dict_close (outputs[i]);
      goto err;
    }

  if (!(fp->ctf_link_flags & CTF_LINK_OMIT_VARIABLES_SECTION)
      && ctf_link_deduplicating_variables (fp, inputs, ninputs, 0) < 0)
    {
      ctf_err_warn (fp, 0, 0, _("deduplicating link variable emission failed for "
				"%s"), ctf_link_input_name (fp));
      goto err_clean_outputs;
    }

  if (ctf_link_deduplicating_syms (fp, inputs, ninputs, 0) < 0)
    {
      ctf_err_warn (fp, 0, 0, _("deduplicating link symbol emission failed for "
				"%s"), ctf_link_input_name (fp));
      goto err_clean_outputs;
    }

  ctf_dedup_fini (fp, outputs, noutputs);

  /* Now close all the inputs, including per-CU intermediates.  */

  if (ctf_link_deduplicating_close_inputs (fp, NULL, inputs, ninputs) < 0)
    return;					/* errno is set for us.  */

  ninputs = 0;					/* Prevent double-close.  */
  ctf_set_errno (fp, 0);

  /* Fall through.  */

 err:
  for (i = 0; i < (size_t) ninputs; i++)
    ctf_dict_close (inputs[i]);
  free (inputs);
  free (parents);
  free (outputs);
  return;

 err_clean_outputs:
  ctf_link_empty_outputs (fp);
  goto err;
}

/* Merge types and variable sections in all dicts added to the link together.
   The result of any previous link is discarded.  */
int
ctf_link (ctf_dict_t *fp, int flags)
{
  int err;

  fp->ctf_link_flags = flags;

  if (fp->ctf_link_inputs == NULL)
    return 0;					/* Nothing to do. */

  if (fp->ctf_link_outputs != NULL)
    ctf_link_empty_outputs (fp);
  else
    fp->ctf_link_outputs = ctf_dynhash_create (ctf_hash_string,
					       ctf_hash_eq_string, free,
					       (ctf_hash_free_fun)
					       ctf_dict_close);

  if (fp->ctf_link_outputs == NULL)
    return ctf_set_errno (fp, ENOMEM);

  fp->ctf_flags |= LCTF_LINKING;
  ctf_link_deduplicating (fp);
  fp->ctf_flags &= ~LCTF_LINKING;

  if ((ctf_errno (fp) != 0) && (ctf_errno (fp) != ECTF_NOCTFDATA))
    return -1;

  /* Create empty CUs if requested.  We do not currently claim that multiple
     links in succession with CTF_LINK_EMPTY_CU_MAPPINGS set in some calls and
     not set in others will do anything especially sensible.  */

  if (fp->ctf_link_out_cu_mapping && (flags & CTF_LINK_EMPTY_CU_MAPPINGS))
    {
      ctf_next_t *i = NULL;
      void *k;

      while ((err = ctf_dynhash_next (fp->ctf_link_out_cu_mapping, &i, &k,
				      NULL)) == 0)
	{
	  const char *to = (const char *) k;
	  if (ctf_create_per_cu (fp, NULL, to) == NULL)
	    {
	      fp->ctf_flags &= ~LCTF_LINKING;
	      ctf_next_destroy (i);
	      return -1;			/* Errno is set for us.  */
	    }
	}
      if (err != ECTF_NEXT_END)
	{
	  fp->ctf_flags &= ~LCTF_LINKING;
	  ctf_err_warn (fp, 1, err, _("iteration error creating empty CUs"));
	  ctf_set_errno (fp, err);
	  return -1;
	}
    }

  return 0;
}

typedef struct ctf_link_out_string_cb_arg
{
  const char *str;
  uint32_t offset;
  int err;
} ctf_link_out_string_cb_arg_t;

/* Intern a string in the string table of an output per-CU CTF file.  */
static void
ctf_link_intern_extern_string (void *key _libctf_unused_, void *value,
			       void *arg_)
{
  ctf_dict_t *fp = (ctf_dict_t *) value;
  ctf_link_out_string_cb_arg_t *arg = (ctf_link_out_string_cb_arg_t *) arg_;

  fp->ctf_flags |= LCTF_DIRTY;
  if (!ctf_str_add_external (fp, arg->str, arg->offset))
    arg->err = ENOMEM;
}

/* Repeatedly call ADD_STRING to acquire strings from the external string table,
   adding them to the atoms table for this CU and all subsidiary CUs.

   If ctf_link is also called, it must be called first if you want the new CTF
   files ctf_link can create to get their strings dedupped against the ELF
   strtab properly.  */
int
ctf_link_add_strtab (ctf_dict_t *fp, ctf_link_strtab_string_f *add_string,
		     void *arg)
{
  const char *str;
  uint32_t offset;
  int err = 0;

  while ((str = add_string (&offset, arg)) != NULL)
    {
      ctf_link_out_string_cb_arg_t iter_arg = { str, offset, 0 };

      fp->ctf_flags |= LCTF_DIRTY;
      if (!ctf_str_add_external (fp, str, offset))
	err = ENOMEM;

      ctf_dynhash_iter (fp->ctf_link_outputs, ctf_link_intern_extern_string,
			&iter_arg);
      if (iter_arg.err)
	err = iter_arg.err;
    }

  if (err)
    ctf_set_errno (fp, err);

  return -err;
}

/* Inform the ctf-link machinery of a new symbol in the target symbol table
   (which must be some symtab that is not usually stripped, and which
   is in agreement with ctf_bfdopen_ctfsect).  May be called either before or
   after ctf_link_add_strtab.  */
int
ctf_link_add_linker_symbol (ctf_dict_t *fp, ctf_link_sym_t *sym)
{
  ctf_in_flight_dynsym_t *cid;

  /* Cheat a little: if there is already an ENOMEM error code recorded against
     this dict, we shouldn't even try to add symbols because there will be no
     memory to do so: probably we failed to add some previous symbol.  This
     makes out-of-memory exits 'sticky' across calls to this function, so the
     caller doesn't need to worry about error conditions.  */

  if (ctf_errno (fp) == ENOMEM)
    return -ENOMEM;				/* errno is set for us.  */

  if (ctf_symtab_skippable (sym))
    return 0;

  if (sym->st_type != STT_OBJECT && sym->st_type != STT_FUNC)
    return 0;

  /* Add the symbol to the in-flight list.  */

  if ((cid = malloc (sizeof (ctf_in_flight_dynsym_t))) == NULL)
    goto oom;

  cid->cid_sym = *sym;
  ctf_list_append (&fp->ctf_in_flight_dynsyms, cid);

  return 0;

 oom:
  ctf_dynhash_destroy (fp->ctf_dynsyms);
  fp->ctf_dynsyms = NULL;
  ctf_set_errno (fp, ENOMEM);
  return -ENOMEM;
}

/* Impose an ordering on symbols.  The ordering takes effect immediately, but
   since the ordering info does not include type IDs, lookups may return nothing
   until such IDs are added by calls to ctf_add_*_sym.  Must be called after
   ctf_link_add_strtab and ctf_link_add_linker_symbol.  */
int
ctf_link_shuffle_syms (ctf_dict_t *fp)
{
  ctf_in_flight_dynsym_t *did, *nid;
  ctf_next_t *i = NULL;
  int err = ENOMEM;
  void *name_, *sym_;

  if (!fp->ctf_dynsyms)
    {
      fp->ctf_dynsyms = ctf_dynhash_create (ctf_hash_string,
					    ctf_hash_eq_string,
					    NULL, free);
      if (!fp->ctf_dynsyms)
	{
	  ctf_set_errno (fp, ENOMEM);
	  return -ENOMEM;
	}
    }

  /* Add all the symbols, excluding only those we already know are prohibited
     from appearing in symtypetabs.  */

  for (did = ctf_list_next (&fp->ctf_in_flight_dynsyms); did != NULL; did = nid)
    {
      ctf_link_sym_t *new_sym;

      nid = ctf_list_next (did);
      ctf_list_delete (&fp->ctf_in_flight_dynsyms, did);

      /* We might get a name or an external strtab offset.  The strtab offset is
	 guaranteed resolvable at this point, so turn it into a string.  */

      if (did->cid_sym.st_name == NULL)
	{
	  uint32_t off = CTF_SET_STID (did->cid_sym.st_nameidx, CTF_STRTAB_1);

	  did->cid_sym.st_name = ctf_strraw (fp, off);
	  did->cid_sym.st_nameidx_set = 0;
	  if (!ctf_assert (fp, did->cid_sym.st_name != NULL))
	    return -ECTF_INTERNAL;		/* errno is set for us.  */
	}

      /* The symbol might have turned out to be nameless, so we have to recheck
	 for skippability here.  */
      if (!ctf_symtab_skippable (&did->cid_sym))
	{
	  ctf_dprintf ("symbol from linker: %s (%x)\n", did->cid_sym.st_name,
		       did->cid_sym.st_symidx);

	  if ((new_sym = malloc (sizeof (ctf_link_sym_t))) == NULL)
	    goto local_oom;

	  memcpy (new_sym, &did->cid_sym, sizeof (ctf_link_sym_t));
	  if (ctf_dynhash_cinsert (fp->ctf_dynsyms, new_sym->st_name, new_sym) < 0)
	    goto local_oom;

	  if (fp->ctf_dynsymmax < new_sym->st_symidx)
	    fp->ctf_dynsymmax = new_sym->st_symidx;
	}

      free (did);
      continue;

    local_oom:
      free (did);
      free (new_sym);
      goto err;
    }

  /* If no symbols are reported, unwind what we have done and return.  This
     makes it a bit easier for the serializer to tell that no symbols have been
     reported and that it should look elsewhere for reported symbols.  */
  if (!ctf_dynhash_elements (fp->ctf_dynsyms))
    {
      ctf_dprintf ("No symbols: not a final link.\n");
      ctf_dynhash_destroy (fp->ctf_dynsyms);
      fp->ctf_dynsyms = NULL;
      return 0;
    }

  /* Construct a mapping from shndx to the symbol info.  */
  free (fp->ctf_dynsymidx);
  if ((fp->ctf_dynsymidx = calloc (fp->ctf_dynsymmax + 1,
				   sizeof (ctf_link_sym_t *))) == NULL)
    goto err;

  while ((err = ctf_dynhash_next (fp->ctf_dynsyms, &i, &name_, &sym_)) == 0)
    {
      const char *name = (const char *) name;
      ctf_link_sym_t *symp = (ctf_link_sym_t *) sym_;

      if (!ctf_assert (fp, symp->st_symidx <= fp->ctf_dynsymmax))
	{
	  ctf_next_destroy (i);
	  err = ctf_errno (fp);
	  goto err;
	}
      fp->ctf_dynsymidx[symp->st_symidx] = symp;
    }
  if (err != ECTF_NEXT_END)
    {
      ctf_err_warn (fp, 0, err, _("error iterating over shuffled symbols"));
      goto err;
    }
  return 0;

 err:
  /* Leave the in-flight symbols around: they'll be freed at
     dict close time regardless.  */
  ctf_dynhash_destroy (fp->ctf_dynsyms);
  fp->ctf_dynsyms = NULL;
  free (fp->ctf_dynsymidx);
  fp->ctf_dynsymidx = NULL;
  fp->ctf_dynsymmax = 0;
  ctf_set_errno (fp, err);
  return -err;
}

typedef struct ctf_name_list_accum_cb_arg
{
  char **names;
  ctf_dict_t *fp;
  ctf_dict_t **files;
  size_t i;
  char **dynames;
  size_t ndynames;
} ctf_name_list_accum_cb_arg_t;

/* Accumulate the names and a count of the names in the link output hash.  */
static void
ctf_accumulate_archive_names (void *key, void *value, void *arg_)
{
  const char *name = (const char *) key;
  ctf_dict_t *fp = (ctf_dict_t *) value;
  char **names;
  ctf_dict_t **files;
  ctf_name_list_accum_cb_arg_t *arg = (ctf_name_list_accum_cb_arg_t *) arg_;

  if ((names = realloc (arg->names, sizeof (char *) * ++(arg->i))) == NULL)
    {
      (arg->i)--;
      ctf_set_errno (arg->fp, ENOMEM);
      return;
    }

  if ((files = realloc (arg->files, sizeof (ctf_dict_t *) * arg->i)) == NULL)
    {
      (arg->i)--;
      ctf_set_errno (arg->fp, ENOMEM);
      return;
    }

  /* Allow the caller to get in and modify the name at the last minute.  If the
     caller *does* modify the name, we have to stash away the new name the
     caller returned so we can free it later on.  (The original name is the key
     of the ctf_link_outputs hash and is freed by the dynhash machinery.)  */

  if (fp->ctf_link_memb_name_changer)
    {
      char **dynames;
      char *dyname;
      void *nc_arg = fp->ctf_link_memb_name_changer_arg;

      dyname = fp->ctf_link_memb_name_changer (fp, name, nc_arg);

      if (dyname != NULL)
	{
	  if ((dynames = realloc (arg->dynames,
				  sizeof (char *) * ++(arg->ndynames))) == NULL)
	    {
	      (arg->ndynames)--;
	      ctf_set_errno (arg->fp, ENOMEM);
	      return;
	    }
	    arg->dynames = dynames;
	    name = (const char *) dyname;
	}
    }

  arg->names = names;
  arg->names[(arg->i) - 1] = (char *) name;
  arg->files = files;
  arg->files[(arg->i) - 1] = fp;
}

/* Change the name of the parent CTF section, if the name transformer has got to
   it.  */
static void
ctf_change_parent_name (void *key _libctf_unused_, void *value, void *arg)
{
  ctf_dict_t *fp = (ctf_dict_t *) value;
  const char *name = (const char *) arg;

  ctf_parent_name_set (fp, name);
}

/* Warn if we may suffer information loss because the CTF input files are too
   old.  Usually we provide complete backward compatibility, but compiler
   changes etc which never hit a release may have a flag in the header that
   simply prevents those changes from being used.  */
static void
ctf_link_warn_outdated_inputs (ctf_dict_t *fp)
{
  ctf_next_t *i = NULL;
  void *name_;
  void *input_;
  int err;

  while ((err = ctf_dynhash_next (fp->ctf_link_inputs, &i, &name_, &input_)) == 0)
    {
      const char *name = (const char *) name_;
      ctf_link_input_t *input = (ctf_link_input_t *) input_;
      ctf_next_t *j = NULL;
      ctf_dict_t *ifp;
      int err;

      /* We only care about CTF archives by this point: lazy-opened archives
	 have always been opened by this point, and short-circuited entries have
	 a matching corresponding archive member. Entries with NULL clin_arc can
	 exist, and constitute old entries renamed via a name changer: the
	 renamed entries exist elsewhere in the list, so we can just skip
	 those.  */

      if (!input->clin_arc)
	continue;

      /* All entries in the archive will necessarily contain the same
	 CTF_F_NEWFUNCINFO flag, so we only need to check the first. We don't
	 even need to do that if we can't open it for any reason at all: the
	 link will fail later on regardless, since an input can't be opened. */

      ifp = ctf_archive_next (input->clin_arc, &j, NULL, 0, &err);
      if (!ifp)
	continue;
      ctf_next_destroy (j);

      if (!(ifp->ctf_header->cth_flags & CTF_F_NEWFUNCINFO)
	  && (ifp->ctf_header->cth_varoff - ifp->ctf_header->cth_funcoff) > 0)
	ctf_err_warn (fp, 1, 0, _("linker input %s has CTF func info but uses "
				  "an old, unreleased func info format: "
				  "this func info section will be dropped."),
		      name);
    }
  if (err != ECTF_NEXT_END)
    ctf_err_warn (fp, 0, err, _("error checking for outdated inputs"));
}

/* Write out a CTF archive (if there are per-CU CTF files) or a CTF file
   (otherwise) into a new dynamically-allocated string, and return it.
   Members with sizes above THRESHOLD are compressed.  */
unsigned char *
ctf_link_write (ctf_dict_t *fp, size_t *size, size_t threshold)
{
  ctf_name_list_accum_cb_arg_t arg;
  char **names;
  char *transformed_name = NULL;
  ctf_dict_t **files;
  FILE *f = NULL;
  size_t i;
  int err;
  long fsize;
  const char *errloc;
  unsigned char *buf = NULL;

  memset (&arg, 0, sizeof (ctf_name_list_accum_cb_arg_t));
  arg.fp = fp;
  fp->ctf_flags |= LCTF_LINKING;

  ctf_link_warn_outdated_inputs (fp);

  if (fp->ctf_link_outputs)
    {
      ctf_dynhash_iter (fp->ctf_link_outputs, ctf_accumulate_archive_names, &arg);
      if (ctf_errno (fp) < 0)
	{
	  errloc = "hash creation";
	  goto err;
	}
    }

  /* No extra outputs? Just write a simple ctf_dict_t.  */
  if (arg.i == 0)
    {
      unsigned char *ret = ctf_write_mem (fp, size, threshold);
      fp->ctf_flags &= ~LCTF_LINKING;
      return ret;
    }

  /* Writing an archive.  Stick ourselves (the shared repository, parent of all
     other archives) on the front of it with the default name.  */
  if ((names = realloc (arg.names, sizeof (char *) * (arg.i + 1))) == NULL)
    {
      errloc = "name reallocation";
      goto err_no;
    }
  arg.names = names;
  memmove (&(arg.names[1]), arg.names, sizeof (char *) * (arg.i));

  arg.names[0] = (char *) _CTF_SECTION;
  if (fp->ctf_link_memb_name_changer)
    {
      void *nc_arg = fp->ctf_link_memb_name_changer_arg;

      transformed_name = fp->ctf_link_memb_name_changer (fp, _CTF_SECTION,
							 nc_arg);

      if (transformed_name != NULL)
	{
	  arg.names[0] = transformed_name;
	  ctf_dynhash_iter (fp->ctf_link_outputs, ctf_change_parent_name,
			    transformed_name);
	}
    }

  /* Propagate the link flags to all the dicts in this link.  */
  for (i = 0; i < arg.i; i++)
    {
      arg.files[i]->ctf_link_flags = fp->ctf_link_flags;
      arg.files[i]->ctf_flags |= LCTF_LINKING;
    }

  if ((files = realloc (arg.files,
			sizeof (struct ctf_dict *) * (arg.i + 1))) == NULL)
    {
      errloc = "ctf_dict reallocation";
      goto err_no;
    }
  arg.files = files;
  memmove (&(arg.files[1]), arg.files, sizeof (ctf_dict_t *) * (arg.i));
  arg.files[0] = fp;

  if ((f = tmpfile ()) == NULL)
    {
      errloc = "tempfile creation";
      goto err_no;
    }

  if ((err = ctf_arc_write_fd (fileno (f), arg.files, arg.i + 1,
			       (const char **) arg.names,
			       threshold)) < 0)
    {
      errloc = "archive writing";
      ctf_set_errno (fp, err);
      goto err;
    }

  if (fseek (f, 0, SEEK_END) < 0)
    {
      errloc = "seeking to end";
      goto err_no;
    }

  if ((fsize = ftell (f)) < 0)
    {
      errloc = "filesize determination";
      goto err_no;
    }

  if (fseek (f, 0, SEEK_SET) < 0)
    {
      errloc = "filepos resetting";
      goto err_no;
    }

  if ((buf = malloc (fsize)) == NULL)
    {
      errloc = "CTF archive buffer allocation";
      goto err_no;
    }

  while (!feof (f) && fread (buf, fsize, 1, f) == 0)
    if (ferror (f))
      {
	errloc = "reading archive from temporary file";
	goto err_no;
      }

  *size = fsize;
  free (arg.names);
  free (arg.files);
  free (transformed_name);
  if (arg.ndynames)
    {
      size_t i;
      for (i = 0; i < arg.ndynames; i++)
	free (arg.dynames[i]);
      free (arg.dynames);
    }
  fclose (f);
  return buf;

 err_no:
  ctf_set_errno (fp, errno);

  /* Turn off the is-linking flag on all the dicts in this link.  */
  for (i = 0; i < arg.i; i++)
    arg.files[i]->ctf_flags &= ~LCTF_LINKING;
 err:
  free (buf);
  if (f)
    fclose (f);
  free (arg.names);
  free (arg.files);
  free (transformed_name);
  if (arg.ndynames)
    {
      size_t i;
      for (i = 0; i < arg.ndynames; i++)
	free (arg.dynames[i]);
      free (arg.dynames);
    }
  ctf_err_warn (fp, 0, 0, _("cannot write archive in link: %s failure"),
		errloc);
  return NULL;
}
