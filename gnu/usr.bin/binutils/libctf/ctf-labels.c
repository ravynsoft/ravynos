/* Labelled ranges of type IDs.
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

static int
extract_label_info (ctf_dict_t *fp, const ctf_lblent_t **ctl,
		    uint32_t *num_labels)
{
  const ctf_header_t *h;

  h = (const ctf_header_t *) fp->ctf_data.cts_data;

  *ctl = (const ctf_lblent_t *) (fp->ctf_buf + h->cth_lbloff);
  *num_labels = (h->cth_objtoff - h->cth_lbloff) / sizeof (ctf_lblent_t);

  return 0;
}

/* Returns the topmost label, or NULL if any errors are encountered.  */

const char *
ctf_label_topmost (ctf_dict_t *fp)
{
  const ctf_lblent_t *ctlp = NULL;
  const char *s;
  uint32_t num_labels = 0;

  if (extract_label_info (fp, &ctlp, &num_labels) < 0)
    return NULL;				/* errno is set for us.  */

  if (num_labels == 0)
    {
      (void) ctf_set_errno (fp, ECTF_NOLABELDATA);
      return NULL;
    }

  if ((s = ctf_strraw (fp, (ctlp + num_labels - 1)->ctl_label)) == NULL)
    (void) ctf_set_errno (fp, ECTF_CORRUPT);

  return s;
}

/* Iterate over all labels.  We pass the label string and the lblinfo_t struct
   to the specified callback function.  */
int
ctf_label_iter (ctf_dict_t *fp, ctf_label_f *func, void *arg)
{
  const ctf_lblent_t *ctlp = NULL;
  uint32_t i;
  uint32_t num_labels = 0;
  ctf_lblinfo_t linfo;
  const char *lname;
  int rc;

  if (extract_label_info (fp, &ctlp, &num_labels) < 0)
    return -1;			/* errno is set for us.  */

  if (num_labels == 0)
    return (ctf_set_errno (fp, ECTF_NOLABELDATA));

  for (i = 0; i < num_labels; i++, ctlp++)
    {
      if ((lname = ctf_strraw (fp, ctlp->ctl_label)) == NULL)
	{
	  /* Not marked for translation: label code not used yet.  */
	  ctf_err_warn (fp, 0, ECTF_CORRUPT,
			"failed to decode label %u with type %u",
			ctlp->ctl_label, ctlp->ctl_type);
	  return (ctf_set_errno (fp, ECTF_CORRUPT));
	}

      linfo.ctb_type = ctlp->ctl_type;
      if ((rc = func (lname, &linfo, arg)) != 0)
	return rc;
    }

  return 0;
}

typedef struct linfo_cb_arg
{
  const char *lca_name;		/* Label we want to retrieve info for.  */
  ctf_lblinfo_t *lca_info;	/* Where to store the info about the label.  */
} linfo_cb_arg_t;

static int
label_info_cb (const char *lname, const ctf_lblinfo_t *linfo, void *arg)
{
  /* If lname matches the label we are looking for, copy the
    lblinfo_t struct for the caller.  */

  if (strcmp (lname, ((linfo_cb_arg_t *) arg)->lca_name) == 0)
    {
      /* * Allow caller not to allocate storage to test if label exists.  */

      if (((linfo_cb_arg_t *) arg)->lca_info != NULL)
	memcpy (((linfo_cb_arg_t *) arg)->lca_info, linfo,
	       sizeof (ctf_lblinfo_t));
      return 1;		/* Indicate we found a match.  */
    }

  return 0;
}

/* Retrieve information about the label with name "lname". */
int
ctf_label_info (ctf_dict_t *fp, const char *lname, ctf_lblinfo_t *linfo)
{
  linfo_cb_arg_t cb_arg;
  int rc;

  cb_arg.lca_name = lname;
  cb_arg.lca_info = linfo;

  if ((rc = ctf_label_iter (fp, label_info_cb, &cb_arg)) < 0)
    return rc;

  if (rc != 1)
    return (ctf_set_errno (fp, ECTF_NOLABEL));

  return 0;
}
