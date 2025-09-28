#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  ctf_dict_t *pfp;
  ctf_dict_t *cfp;
  ctf_id_t base, base2, ptr, type, last_type;
  ctf_encoding_t encoding = { CTF_INT_SIGNED, 0, sizeof (int) };
  ctf_encoding_t encoding2 = { CTF_INT_SIGNED, 0, sizeof (long) };
  char *type_name;
  int err;

  if ((pfp = ctf_create (&err)) == NULL)
    goto create_err;

  if ((cfp = ctf_create (&err)) == NULL)
    goto create_err;

  if (ctf_import (cfp, pfp) < 0)
    goto create_child;

  /* First, try an int in the parent with a pointer in the child.  Also make
     another pair of types we will chain to later: these end up before the
     pptrtab lazy-update watermark.  */

  if ((base = ctf_add_integer (pfp, CTF_ADD_ROOT, "int", &encoding)) == CTF_ERR)
    goto create_parent;

  if ((base2 = ctf_add_integer (pfp, CTF_ADD_ROOT, "long int", &encoding2)) == CTF_ERR)
    goto create_parent;

  if ((ptr = ctf_add_pointer (cfp, CTF_ADD_ROOT, base)) == CTF_ERR)
    goto create_child;

  if ((type = ctf_lookup_by_name (cfp, "int *") ) == CTF_ERR)
    goto err;

  type_name = ctf_type_aname (cfp, type);
  printf ("First lookup: %s in the child points to a type of kind %i\n",
	  type_name, ctf_type_kind (cfp, ctf_type_reference (cfp, type)));
  free (type_name);

  if (ctf_type_reference (cfp, type) != base)
    printf ("First lookup ref diff: %lx versus %lx\n", base,
	    ctf_type_reference (cfp, type));
  last_type = type;

  /* Add another pointer to the same type in the parent and try a lookup.  */

  if ((ptr = ctf_add_pointer (pfp, CTF_ADD_ROOT, base2)) == CTF_ERR)
    goto create_parent;

  if ((type = ctf_lookup_by_name (cfp, "long int *") ) == CTF_ERR)
    goto err;

  type_name = ctf_type_aname (cfp, type);
  printf ("Second lookup: %s in the child points to a type of kind %i\n",
	  type_name, ctf_type_kind (cfp, ctf_type_reference (cfp, type)));
  free (type_name);

  if (ctf_type_reference (cfp, type) != base2)
    printf ("Second lookup ref diff: %lx versus %lx\n", base2,
	    ctf_type_reference (cfp, type));
  if (last_type == type)
    printf ("Second lookup should not return the same type as the first: %lx\n", type);

  last_type = type;

  /* Add another pointer to the same type in the child and try a lookup.  */

  if ((ptr = ctf_add_pointer (cfp, CTF_ADD_ROOT, base2)) == CTF_ERR)
    goto create_child;

  if ((type = ctf_lookup_by_name (cfp, "long int *") ) == CTF_ERR)
    goto err;

  type_name = ctf_type_aname (cfp, type);
  printf ("Third lookup: %s in the child points to a type of kind %i\n",
	  type_name, ctf_type_kind (cfp, ctf_type_reference (cfp, type)));
  free (type_name);

  if (ctf_type_reference (cfp, type) != base2)
    printf ("Third lookup ref diff: %lx versus %lx\n", base2,
	    ctf_type_reference (cfp, type));

  if (last_type == type)
    printf ("Third lookup should not return the same type as the second: %lx\n", type);

  ctf_file_close (cfp);
  ctf_file_close (pfp);

  return 0;

 create_err:
  fprintf (stderr, "Creation failed: %s\n", ctf_errmsg (err));
  return 1;
 create_parent:
  fprintf (stderr, "Cannot create type: %s\n", ctf_errmsg (ctf_errno (pfp)));
  return 1;
 create_child:
  fprintf (stderr, "Cannot create type: %s\n", ctf_errmsg (ctf_errno (cfp)));
  return 1;
 err:
  fprintf (stderr, "Lookup failed: %s\n", ctf_errmsg (ctf_errno (cfp)));
  return 1;
}
