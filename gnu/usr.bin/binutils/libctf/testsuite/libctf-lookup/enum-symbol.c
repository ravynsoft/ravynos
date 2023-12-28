#include "config.h"
#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  ctf_archive_t *ctf;
  ctf_dict_t *fp, *tmp_fp;
  int err;
  ctf_id_t type, tmp;
  ctf_next_t *i = NULL;
  const char *name;
  int val;

  if (argc != 2)
    {
      fprintf (stderr, "Syntax: %s PROGRAM\n", argv[0]);
      exit(1);
    }

  if ((ctf = ctf_open (argv[1], NULL, &err)) == NULL)
    goto open_err;

  /* Fish out the enumerator, then fish out all its enumerand/value pairs.  */

  if ((fp = ctf_arc_lookup_symbol_name (ctf, "primary1", &type, &err)) == NULL)
    goto sym_err;

  while ((name = ctf_enum_next (fp, type, &i, &val)) != NULL)
    {
      printf ("%s has value %i\n", name, val);
    }
  if (ctf_errno (fp) != ECTF_NEXT_END)
    goto nerr;

  /* Fish it out again to check the caching layer.  */
  if (((tmp_fp = ctf_arc_lookup_symbol_name (ctf, "primary1", &tmp, &err)) != fp)
      || (tmp != type))
    goto sym_cache_err;

  ctf_dict_close (tmp_fp);
  ctf_dict_close (fp);
  ctf_close (ctf);

  return 0;

 open_err:
  fprintf (stderr, "%s: cannot open: %s\n", argv[0], ctf_errmsg (err));
  return 1;
 sym_err:
  fprintf (stderr, "%s: Symbol lookup error: %s\n", argv[0], ctf_errmsg (err));
  return 1;
 sym_cache_err:
  fprintf (stderr, "%s: Symbol re-lookup error (caching bug): %s\n", argv[0],
	   ctf_errmsg (err));
  return 1;
 nerr:
  fprintf (stderr, "iteration failed: %s\n", ctf_errmsg (ctf_errno (fp)));
  return 1;
}
