#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  ctf_dict_t *fp;
  ctf_archive_t *ctf;
  ctf_next_t *i = NULL;
  ctf_id_t type;
  const char *arcname;
  int err;

  if (argc != 2)
    {
      fprintf (stderr, "Syntax: %s PROGRAM\n", argv[0]);
      exit(1);
    }

  if ((ctf = ctf_open (argv[1], NULL, &err)) == NULL)
    goto open_err;

  /* Make sure we can look up a_t * by name in all non-parent dicts, even though
     the a_t * and the type it points to are in distinct dicts; make sure we
     cannot look up b_t *.  */

  while ((fp = ctf_archive_next (ctf, &i, &arcname, 1, &err)) != NULL)
    {
      if ((type = ctf_lookup_by_name (fp, "a_t *")) == CTF_ERR)
	goto err;

      if ((ctf_lookup_by_name (fp, "b_t *")) != CTF_ERR ||
          ctf_errno (fp) != ECTF_NOTYPE)
	goto noerr;

      if (ctf_type_reference (fp, type) == CTF_ERR)
	goto err;

      printf ("%s: a_t * points to a type of kind %i\n", arcname,
	      ctf_type_kind (fp, ctf_type_reference (fp, type)));

      ctf_dict_close (fp);
    }
  if (err != ECTF_NEXT_END)
    goto open_err;

  ctf_close (ctf);

  return 0;

 open_err:
  fprintf (stderr, "%s: cannot open: %s\n", argv[0], ctf_errmsg (err));
  return 1;
 err:
  fprintf (stderr, "Lookup failed in %s: %s\n", arcname, ctf_errmsg (ctf_errno (fp)));
  return 1;
 noerr:
  fprintf (stderr, "Lookup unexpectedly succeeded in %s\n", arcname);
  return 1;
}
