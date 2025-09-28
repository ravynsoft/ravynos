/* Make sure unnamed field offsets are relative to the containing struct.  */

#include <ctf-api.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "unnamed-field-info-ctf.c"

static void
verify_offsetof_matching (ctf_dict_t *fp, ctf_id_t type, const char *name, size_t offset)
{
  ctf_membinfo_t mi;

  if (ctf_member_info (fp, type, name, &mi) < 0)
    goto err;

  if (mi.ctm_offset != offset * 8)
    fprintf (stderr, "field %s inconsistency: offsetof() says %zi bits, CTF says %zi\n",
	     name, offset * 8, mi.ctm_offset);

  return;

 err:
  fprintf (stderr, "Cannot look up field %s: %s\n", name,
	   ctf_errmsg (ctf_errno (fp)));
  return;
}

int
main (int argc, char *argv[])
{
  ctf_dict_t *fp;
  ctf_archive_t *ctf;
  ctf_id_t type;
  int err;

  if (argc != 2)
    {
      fprintf (stderr, "Syntax: %s PROGRAM\n", argv[0]);
      exit(1);
    }

  if ((ctf = ctf_open (argv[1], NULL, &err)) == NULL)
    goto open_err;
  if ((fp = ctf_dict_open (ctf, NULL, &err)) == NULL)
    goto open_err;

  /* Dig out some structure members by name.  */

  if ((type = ctf_lookup_by_name (fp, "struct A") ) == CTF_ERR)
    goto err;

  verify_offsetof_matching (fp, type, "a", offsetof (struct A, a));
  verify_offsetof_matching (fp, type, "b", offsetof (struct A, b));
  verify_offsetof_matching (fp, type, "one", offsetof (struct A, one));
  verify_offsetof_matching (fp, type, "two", offsetof (struct A, two));
  verify_offsetof_matching (fp, type, "three", offsetof (struct A, three));
  verify_offsetof_matching (fp, type, "four", offsetof (struct A, four));
  verify_offsetof_matching (fp, type, "x", offsetof (struct A, x));
  verify_offsetof_matching (fp, type, "y", offsetof (struct A, y));
  verify_offsetof_matching (fp, type, "z", offsetof (struct A, z));
  verify_offsetof_matching (fp, type, "aleph", offsetof (struct A, aleph));

  ctf_dict_close (fp);
  ctf_arc_close (ctf);

  printf ("Offset validation complete.\n");

  return 0;

 open_err:
  fprintf (stderr, "%s: cannot open: %s\n", argv[0], ctf_errmsg (err));
  return 1;

 err:
  fprintf (stderr, "Cannot look up type: %s\n", ctf_errmsg (ctf_errno (fp)));
  return 1;
}
