#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  ctf_dict_t *fp;
  ctf_archive_t *ctf;
  ctf_id_t foo_type, bar_type, sym_type;
  int found_foo = 0, found_bar = 0;
  ctf_next_t *i = NULL;
  const char *name;
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

  /* Make sure we can look up both 'foo' and 'bar' as variables, even though one
     of them is nonstatic: in a full link this should be erased, but this is an
     ld -r link.  */

  if ((foo_type = ctf_lookup_variable (fp, "foo")) == CTF_ERR)
    printf ("Cannot look up foo: %s\n", ctf_errmsg (ctf_errno (fp)));
  else
    printf ("foo is of type %lx\n", foo_type);

  if ((bar_type = ctf_lookup_variable (fp, "bar")) == CTF_ERR)
    printf ("Cannot look up bar: %s\n", ctf_errmsg (ctf_errno (fp)));
  else
    printf ("bar is of type %lx\n", bar_type);

  /* Traverse the entire data object section and make sure it contains entries
     for both foo and bar.  (This is pure laziness: the section is small and
     ctf_lookup_by_symbol_name does not yet exist.)  */
  while ((sym_type = ctf_symbol_next (fp, &i, &name, 0)) != CTF_ERR)
    {
      if (!name)
	continue;

      if (strcmp (name, "foo") == 0)
	found_foo = 1;
      if (strcmp (name, "bar") == 0)
	found_bar = 1;
    }
  if (ctf_errno (fp) != ECTF_NEXT_END)
    fprintf (stderr, "Unexpected error iterating over symbols: %s\n",
	     ctf_errmsg (ctf_errno (fp)));

  if (!found_foo)
    printf ("foo missing from the data object section\n");
  if (!found_bar)
    printf ("bar missing from the data object section\n");

  ctf_dict_close (fp);
  ctf_close (ctf);

  return 0;

 open_err:
  fprintf (stderr, "%s: cannot open: %s\n", argv[0], ctf_errmsg (err));
  return 1;
}
