/* Make sure that errors are propagated properly from parent dicts to children
   when errors are encountered in child functions that can recurse to parents.

   We check specifically a subset of known-buggy functions.
   Functions that require a buggy linker to expose, or that only fail on
   assertion-failure-incurring corrupted dicts, are not tested. */

#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *desc;

static void
check_prop_err (ctf_dict_t *child, ctf_dict_t *parent, int expected)
{
  if (ctf_errno (child) == expected)
    return;

  if (ctf_errno (parent) == expected)
    fprintf (stderr, "%s: error propagation failure: error \"%s\" not seen on child, "
             "but instead on parent\n", desc, ctf_errmsg (ctf_errno (parent)));
  else
    fprintf (stderr, "%s: expected error is entirely lost: "
             "\"%s\" seen on parent, \"%s\" on child\n", desc,
             ctf_errmsg (ctf_errno (parent)),
             ctf_errmsg (ctf_errno (child)));
}

static void
no_prop_err (void)
{
  fprintf (stderr, "%s: expected error return not observed.\n", desc);
}

int main (void)
{
  ctf_dict_t *parent;
  ctf_dict_t *blank;
  ctf_dict_t *child;
  ctf_id_t void_id;
  ctf_id_t base;
  ctf_id_t slice;
  ctf_id_t function;
  ctf_encoding_t long_encoding = { CTF_INT_SIGNED, 0, sizeof (long) };
  ctf_encoding_t void_encoding = { CTF_INT_SIGNED, 0, 0 };
  ctf_encoding_t foo;
  ctf_funcinfo_t fi;
  ctf_id_t bar;
  char *funcname;
  int err;

  if ((parent = ctf_create (&err)) == NULL
      || (child = ctf_create (&err)) == NULL
      || (blank = ctf_create (&err)) == NULL)
    {
      fprintf (stderr, "Cannot create dicts: %s\n", ctf_errmsg (err));
      return 1;
    }

  if ((ctf_import (child, parent)) < 0)
    {
      fprintf (stderr, "cannot import: %s\n", ctf_errmsg (ctf_errno (child)));
      return 1;
    }

  if ((void_id = ctf_add_integer (parent, CTF_ADD_ROOT, "void", &void_encoding))
      == CTF_ERR)
    goto parent_err;

  if ((base = ctf_add_integer (parent, CTF_ADD_ROOT, "long int", &long_encoding))
      == CTF_ERR)
    goto parent_err;

  foo.cte_format = 0;
  foo.cte_bits = 4;
  foo.cte_offset = 4;
  if ((slice = ctf_add_slice (child, CTF_ADD_ROOT, base, &foo)) == CTF_ERR)
    goto parent_err;

  if (ctf_add_variable (parent, "foo", base) < 0)
    goto child_err;

  fi.ctc_return = void_id;
  fi.ctc_argc = 0;
  fi.ctc_flags = 0;
  if ((function = ctf_add_function (child, CTF_ADD_ROOT, &fi, NULL)) == CTF_ERR)
    goto child_err;

  desc = "func info lookup of non-function";
  if ((ctf_func_type_info (child, base, &fi)) != CTF_ERR)
    no_prop_err ();
  check_prop_err (child, parent, ECTF_NOTFUNC);

  desc = "func args lookup of non-function";
  if ((ctf_func_type_args (child, base, 0, &bar)) != CTF_ERR)
    no_prop_err ();
  check_prop_err (child, parent, ECTF_NOTFUNC);

  if ((ctf_import (child, blank)) < 0)
    {
      fprintf (stderr, "cannot reimport: %s\n", ctf_errmsg (ctf_errno (child)));
      return 1;
    }

  /* This is testing ctf_type_resolve_unsliced(), which is called by the enum
     functions (which are not themselves buggy).  This typea isn't an enum, but
     that's OK: we're after an error, after all, and the type we're slicing is
     not visible any longer, so nothing can tell it's not an enum.  */

  desc = "child slice resolution";
  if ((ctf_enum_value (child, slice, "foo", NULL)) != CTF_ERR)
    no_prop_err ();
  check_prop_err (child, parent, ECTF_BADID);

  desc = "child slice encoding lookup";
  if ((ctf_type_encoding (child, slice, &foo)) != CTF_ERR)
    no_prop_err ();
  check_prop_err (child, parent, ECTF_BADID);

  desc = "func info lookup of non-function";
  if ((ctf_func_type_info (child, base, &fi)) != CTF_ERR)
    no_prop_err ();
  check_prop_err (child, parent, ECTF_BADID);

  desc = "func args lookup of non-function";
  if ((ctf_func_type_args (child, base, 0, &bar)) != CTF_ERR)
    no_prop_err ();
  check_prop_err (child, parent, ECTF_BADID);

  desc = "child slice addition";
  if ((slice = ctf_add_slice (child, CTF_ADD_ROOT, base, &foo)) != CTF_ERR)
    no_prop_err ();
  check_prop_err (child, parent, ECTF_BADID);

  desc = "variable lookup";
  if (ctf_lookup_variable (child, "foo") != CTF_ERR)
    no_prop_err ();
  check_prop_err (child, parent, ECTF_NOTYPEDAT);

  desc = "function lookup via ctf_type_aname";
  if ((funcname = ctf_type_aname (child, function)) != NULL)
    {
      no_prop_err ();
      free (funcname);
    }
  check_prop_err (child, parent, ECTF_BADID);

  ctf_dict_close (child);
  ctf_dict_close (parent);
  ctf_dict_close (blank);
  fprintf (stderr, "All done.\n");
  return 0;

 parent_err:
  fprintf (stderr, "cannot populate parent: %s\n", ctf_errmsg (ctf_errno (parent)));
  return 1;

 child_err:
  fprintf (stderr, "cannot populate child: %s\n", ctf_errmsg (ctf_errno (parent)));
  return 1;

}
