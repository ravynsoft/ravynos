/* Test ambiguous forward lookups post-deduplication.

   This also makes sure that deduplication succeeds in this case and does not
   throw spurious errors.  */

#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  ctf_dict_t *fp;
  ctf_archive_t *ctf;
  ctf_id_t type;
  ctf_arinfo_t ar;
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

  /* Dig out the array type and resolve its element type.  */

  if ((type = ctf_lookup_by_name (fp, "a_array") ) == CTF_ERR)
    goto err;
  if ((type = ctf_type_resolve (fp, type)) == CTF_ERR)
    goto err;
  if (ctf_array_info (fp, type, &ar) < 0)
    goto err;
  printf ("Kind of array element is %i\n", ctf_type_kind (fp, ar.ctr_contents));

  ctf_dict_close (fp);
  ctf_close (ctf);

  return 0;

 open_err:
  fprintf (stderr, "%s: cannot open: %s\n", argv[0], ctf_errmsg (err));
  return 1;
 err:
  fprintf (stderr, "Lookup failed: %s\n", ctf_errmsg (ctf_errno (fp)));
  return 1;
}
