/* Make sure we can look up a pointer-to-type where the type is more than a page
   into the parent and the child has never had a lookup before.  */

#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (void)
{
  ctf_dict_t *pfp, *cfp;
  ctf_encoding_t e = { CTF_INT_SIGNED, 0, sizeof (long) };
  ctf_id_t ptype, ptrtype, type, foo;
  size_t i;
  int err;

  if ((pfp = ctf_create (&err)) == NULL)
    goto create_err;

  if ((ptype = ctf_add_integer (pfp, CTF_ADD_NONROOT, "blah", &e)) == CTF_ERR)
    goto create_parent;

  for (i = 0; i < 4096; i++)
    if ((foo = ctf_add_pointer (pfp, CTF_ADD_NONROOT, ptype)) == CTF_ERR)
      goto create_parent;

  if ((cfp = ctf_create (&err)) == NULL)
    goto create_err;

  if (ctf_import (cfp, pfp) < 0)
    goto create_child;

  if ((ptype = ctf_add_integer (pfp, CTF_ADD_ROOT, "foo", &e)) == CTF_ERR)
    goto create_parent;

  if ((ptrtype = ctf_add_pointer (pfp, CTF_ADD_ROOT, ptype)) == CTF_ERR)
    goto create_parent;

  if ((type = ctf_lookup_by_name (cfp, "*foo")) != CTF_ERR)
    {
      fprintf (stderr, "Type lookup unexpectedly succeeded: %s\n", ctf_errmsg (ctf_errno (cfp)));
      exit (1);
    }

  if ((type = ctf_lookup_by_name (cfp, "foo *")) == CTF_ERR)
    {
      fprintf (stderr, "Type lookup error: %s\n", ctf_errmsg (ctf_errno (cfp)));
      exit (1);
    }

  ctf_dict_close (cfp);
  ctf_dict_close (pfp);

  printf ("Type lookup succeeded.\n");

  return 0;

 create_err:
  fprintf (stderr, "Creation failed: %s\n", ctf_errmsg (err));
  exit (1);
 create_parent:
  fprintf (stderr, "Cannot create parent type: %s\n", ctf_errmsg (ctf_errno (pfp)));
  exit (1);
 create_child:
  fprintf (stderr, "Cannot create child type: %s\n", ctf_errmsg (ctf_errno (cfp)));
  exit (1);
}
