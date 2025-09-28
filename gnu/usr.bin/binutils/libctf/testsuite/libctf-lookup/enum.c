#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>

static int
print_enum (const char *name, int val, void *unused)
{
  printf ("iter test: %s has value %i\n", name, val);
  return 0;
}

int
main (int argc, char *argv[])
{
  ctf_dict_t *fp;
  ctf_archive_t *ctf;
  ctf_id_t type;
  const char *name;
  ctf_next_t *i = NULL;
  int val;
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

  /* Try getting some enum values by hand.  */

  if ((type = ctf_lookup_by_name (fp, "enum e") ) == CTF_ERR)
    goto err;
  if (ctf_enum_value (fp, type, "ENUMSAMPLE_1", &val) < 0)
    goto err;
  printf ("Enum e enumerand ENUMSAMPLE_1 has value %i\n", val);

  if ((name = ctf_enum_name (fp, type, 1)) == NULL)
    goto err;
  printf ("Enum e enumerand %s has value 1\n", name);

  /* Try getting some values using both sorts of iterator.  */

  if ((type = ctf_lookup_by_name (fp, "enum ie") ) == CTF_ERR)
    goto err;

  if ((ctf_enum_iter (fp, type, print_enum, NULL)) < 0)
    goto ierr;

  while ((name = ctf_enum_next (fp, type, &i, &val)) != NULL)
    {
      printf ("next test: %s has value %i\n", name, val);
    }
  if (ctf_errno (fp) != ECTF_NEXT_END)
    goto nerr;

  ctf_dict_close (fp);
  ctf_close (ctf);

  return 0;

 open_err:
  fprintf (stderr, "%s: cannot open: %s\n", argv[0], ctf_errmsg (err));
  return 1;
 err:
  fprintf (stderr, "Lookup failed: %s\n", ctf_errmsg (ctf_errno (fp)));
  return 1;
 ierr:
  fprintf (stderr, "_iter iteration failed: %s\n", ctf_errmsg (ctf_errno (fp)));
  return 1;
 nerr:
  fprintf (stderr, "_next iteration failed: %s\n", ctf_errmsg (ctf_errno (fp)));
  return 1;
}
