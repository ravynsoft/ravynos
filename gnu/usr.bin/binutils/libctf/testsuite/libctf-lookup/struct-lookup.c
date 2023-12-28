#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  ctf_dict_t *fp;
  ctf_archive_t *ctf;
  ctf_id_t type;
  char *type_name;
  ctf_membinfo_t mi;
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

  /* Dig out some strucutre members by name.  */

  if ((type = ctf_lookup_by_name (fp, "struct foo_t") ) == CTF_ERR)
    goto err;

  if (ctf_member_info (fp, type, "baz", &mi) < 0)
    goto err;

  type_name = ctf_type_aname (fp, mi.ctm_type);
  printf ("baz is of type %s, at offset %lx\n", type_name, mi.ctm_offset);
  free (type_name);

  if (ctf_member_info (fp, type, "one_more_level", &mi) < 0)
    goto err;

  type_name = ctf_type_aname (fp, mi.ctm_type);
  printf ("one_more_level is of type %s, at offset %lx\n", type_name, mi.ctm_offset);
  free (type_name);

  if (ctf_member_info (fp, type, "should_not_appear", &mi) >= 0
      || ctf_errno (fp) != ECTF_NOMEMBNAM)
    fprintf (stderr, "should_not_appear appeared.\n");

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
