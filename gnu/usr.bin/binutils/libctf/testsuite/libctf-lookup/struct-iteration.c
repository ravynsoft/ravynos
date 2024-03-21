#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>

static int
print_struct (const char *name, ctf_id_t membtype, unsigned long offset,
	      void *fp_)
{
  ctf_dict_t *fp = (ctf_dict_t *) fp_;
  char *type_name = ctf_type_aname (fp, membtype);

  printf ("iter test: %s, offset %lx, has type %lx/%s\n",
	  name, offset, membtype, type_name);
  free (type_name);

  return 0;
}

int
main (int argc, char *argv[])
{
  ctf_dict_t *fp;
  ctf_archive_t *ctf;
  ctf_id_t type;
  ctf_next_t *i = NULL;
  const char *name;
  ctf_id_t membtype;
  ssize_t offset;
  int icount = 0;
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

  /* Iterate over the structure members with each iterator type in turn.  */

  if ((type = ctf_lookup_by_name (fp, "struct foo_t") ) == CTF_ERR)
    goto err;

  if (ctf_member_iter (fp, type, print_struct, fp) < 0)
    goto ierr;

  while ((offset = ctf_member_next (fp, type, &i, &name, &membtype,
				    CTF_MN_RECURSE)) >= 0)
    {
      char *type_name = ctf_type_aname (fp, membtype);

      printf ("next test: %s, offset %lx, has type %lx/%s\n",
	      name, offset, membtype, type_name);
      free (type_name);
    }
  if (ctf_errno (fp) != ECTF_NEXT_END)
    goto nerr;

  /* Now make sure the count of members does not include any recursive
     members.  */
  while ((offset = ctf_member_next (fp, type, &i, &name, &membtype, 0)) >= 0)
    icount++;

  if (ctf_errno (fp) != ECTF_NEXT_END)
    goto nerr;

  if (icount != ctf_member_count (fp, type))
    printf ("member counts differ: %i by direct iteration, "
	    "%i by ctf_member_count\n", icount, ctf_member_count (fp, type));

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
