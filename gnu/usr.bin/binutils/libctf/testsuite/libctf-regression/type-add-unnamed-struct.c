#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  ctf_dict_t *fp;
  ctf_archive_t *ctf;
  ctf_dict_t *dyn;
  ctf_id_t type;
  ctf_id_t newtype;
  ctf_membinfo_t mi;
  const char *membs[] = { "bar", "baz", "foo", NULL };
  const char **walk;
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

  if ((dyn = ctf_create (&err)) == NULL)
    goto create_err;

  /* Copy 'struct foo' into the dynamic dict, then make sure we can look up a
     member situated inside an unnamed struct.  */

  if ((type = ctf_lookup_by_name (fp, "struct foo")) == CTF_ERR)
    {
      fprintf (stderr, "Cannot look up struct foo: %s\n", ctf_errmsg (ctf_errno (dyn)));
      return 1;
    }

  if ((newtype = ctf_add_type (dyn, fp, type)) == CTF_ERR)
    goto copy_err;

  for (walk = membs; *walk != NULL; walk++)
    {
      if (ctf_member_info (dyn, newtype, *walk, &mi) < 0)
	goto lookup_err;
      printf ("Looked up %s, type %lx, offset %lx\n", *walk, (long) mi.ctm_type, mi.ctm_offset);
    }

  ctf_dict_close (dyn);
  ctf_dict_close (fp);
  ctf_close (ctf);

  return 0;

 open_err:
  fprintf (stderr, "%s: cannot open: %s\n", argv[0], ctf_errmsg (err));
  return 1;
 create_err:
  fprintf (stderr, "%s: cannot create: %s\n", argv[0], ctf_errmsg (err));
  return 1;
 copy_err:
  fprintf (stderr, "Type addition failed: %s\n", ctf_errmsg (ctf_errno (dyn)));
  return 1;
 lookup_err:
  fprintf (stderr, "Cannot look up %s: %s\n", *walk, ctf_errmsg (ctf_errno (dyn)));
  return 1;
}
