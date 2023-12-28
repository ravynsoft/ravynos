/* Make sure serializing a dict (possibly repeatedly) does not corrupt either
   type lookup or the string content of the dict.  */

#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  ctf_dict_t *fp;
  ctf_id_t zygal, autoschediastic;
  ctf_snapshot_id_t snap;
  unsigned char *foo;
  size_t foo_size;
  int err;
  char name[64];

  /* Adding things after serialization should not corrupt names created before
     serialization.  */

  if ((fp = ctf_create (&err)) == NULL)
    goto create_err;

  if ((zygal = ctf_add_struct (fp, CTF_ADD_ROOT, "zygal")) == CTF_ERR)
    goto add_err;

  if ((foo = ctf_write_mem (fp, &foo_size, 4096)) == NULL)
    goto write_err;
  free (foo);

  if (ctf_type_name (fp, zygal, name, sizeof (name)) == NULL)
    fprintf (stderr, "Can't get name of zygal: %s\n", ctf_errmsg (ctf_errno (fp)));
  else
    printf ("zygal's name is %s\n", name);

  if ((autoschediastic = ctf_add_enum (fp, CTF_ADD_ROOT, "autoschediastic")) == CTF_ERR)
    goto add_err;

  if (ctf_type_name (fp, zygal, name, sizeof (name)) == NULL)
    fprintf (stderr, "Can't get name of zygal: %s\n", ctf_errmsg (ctf_errno (fp)));
  else
    printf ("zygal's name is %s\n", name);

  /* Serializing again should not corrupt names either.  */
  if ((foo = ctf_write_mem (fp, &foo_size, 4096)) == NULL)
    goto write_err;
  free (foo);

  if (ctf_type_name (fp, zygal, name, sizeof (name)) == NULL)
    fprintf (stderr, "Can't get name of zygal: %s\n", ctf_errmsg (ctf_errno (fp)));
  else
    printf ("zygal's name is %s\n", name);

  /* Add another new name, roll back, and make sure the strings are
     uncorrupted.  */

  snap = ctf_snapshot (fp);
  if (ctf_add_enumerator (fp, autoschediastic, "aichmophobia", 0) < 0)
    goto add_err;

  if (ctf_rollback (fp, snap) < 0)
    goto roll_err;

  if (ctf_type_name (fp, zygal, name, sizeof (name)) == NULL)
    fprintf (stderr, "Can't get name of zygal: %s\n", ctf_errmsg (ctf_errno (fp)));
  else
    printf ("zygal's name is %s after first rollback\n", name);

  if (ctf_type_name (fp, autoschediastic, name, sizeof (name)) == NULL)
    fprintf (stderr, "Can't get name of autoschediastic: %s\n", ctf_errmsg (ctf_errno (fp)));
  else
    printf ("autoschediastic's name is %s after first rollback\n", name);

  ctf_dict_close (fp);
  return 0;

 create_err:
  fprintf (stderr, "Cannot create: %s\n", ctf_errmsg (err));
  return 1;
 add_err:
  fprintf (stderr, "Cannot add: %s\n", ctf_errmsg (ctf_errno (fp)));
  return 1;
 write_err:
  fprintf (stderr, "Cannot serialize: %s\n", ctf_errmsg (ctf_errno (fp)));
  return 1;
 roll_err:
  fprintf (stderr, "Cannot roll back: %s\n", ctf_errmsg (ctf_errno (fp)));
  return 1;
}
