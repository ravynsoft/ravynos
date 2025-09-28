#include "config.h"
#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  ctf_archive_t *ctf;
  ctf_dict_t *a_fp, *ignore1_fp, *b_fp, *ignore2_fp, *tmp_fp;
  int err;
  ctf_id_t a, b, ignore1, ignore2, tmp;
  char *foo;

  if (argc != 2)
    {
      fprintf (stderr, "Syntax: %s PROGRAM\n", argv[0]);
      exit(1);
    }

  if ((ctf = ctf_open (argv[1], NULL, &err)) == NULL)
    goto open_err;

  /* Fish out each symbol in turn: also try to fish out a nonexistent one.  */

  if ((a_fp = ctf_arc_lookup_symbol_name (ctf, "a", &a, &err)) == NULL)
    goto sym_err;
  printf ("Type of a is %s\n", foo = ctf_type_aname (a_fp, a));

  if ((b_fp = ctf_arc_lookup_symbol_name (ctf, "b", &b, &err)) == NULL)
    goto sym_err;
  printf ("Type of b is %s\n", foo = ctf_type_aname (b_fp, b));

  if ((ignore1_fp = ctf_arc_lookup_symbol_name (ctf, "ignore1", &ignore1, &err)) == NULL)
    goto sym_err;
  printf ("Type of ignore1 is %s\n", foo = ctf_type_aname (ignore1_fp, ignore1));

  if ((ignore2_fp = ctf_arc_lookup_symbol_name (ctf, "ignore2", &ignore2, &err)) == NULL)
    goto sym_err;
  printf ("Type of ignore2 is %s\n", foo = ctf_type_aname (ignore2_fp, ignore1));

  /* Try a call in just-get-the-dict mode and make sure it doesn't fail.  */
  if ((tmp_fp = ctf_arc_lookup_symbol_name (ctf, "ignore2", NULL, &err)) == NULL)
    goto sym_err;
  ctf_dict_close (tmp_fp);

  /* Make sure failures fail.  */
  if  ((ctf_arc_lookup_symbol_name (ctf, "nonexistent", NULL, &err) != NULL)
       || err != ECTF_NOTYPEDAT)
    goto nosym_err;

  /* Fish them out again to check the caching layer.  */
  if (((tmp_fp = ctf_arc_lookup_symbol_name (ctf, "a", &tmp, &err)) != a_fp)
      || (tmp != a))
    goto sym_cache_err;
  ctf_dict_close (tmp_fp);

  if (((tmp_fp = ctf_arc_lookup_symbol_name (ctf, "b", &tmp, &err)) != b_fp)
      || (tmp != b))
    goto sym_cache_err;
  ctf_dict_close (tmp_fp);

  if (((tmp_fp = ctf_arc_lookup_symbol_name (ctf, "ignore1", &tmp, &err)) != ignore1_fp)
      || (tmp != ignore1))
    goto sym_cache_err;
  ctf_dict_close (tmp_fp);

  if (((tmp_fp = ctf_arc_lookup_symbol_name (ctf, "ignore2", &tmp, &err)) != ignore2_fp)
      || (tmp != ignore2))
    goto sym_cache_err;
  ctf_dict_close (tmp_fp);

  ctf_dict_close (a_fp);
  ctf_dict_close (b_fp);
  ctf_dict_close (ignore1_fp);
  ctf_dict_close (ignore2_fp);
  ctf_close (ctf);

  return 0;

 open_err:
  fprintf (stderr, "%s: cannot open: %s\n", argv[0], ctf_errmsg (err));
  return 1;
 sym_err:
  fprintf (stderr, "%s: Symbol lookup error: %s\n", argv[0], ctf_errmsg (err));
  return 1;
 nosym_err:
  fprintf (stderr, "%s: Nonexistent symbol lookup unexpected error: %s\n", argv[0],
	   ctf_errmsg (err));
  return 1;
 sym_cache_err:
  fprintf (stderr, "%s: Symbol re-lookup error (caching bug): %s\n", argv[0],
	   ctf_errmsg (err));
  return 1;
}
