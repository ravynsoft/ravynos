#include <config.h>
#include "test-utils.h"

#include "dbus/dbus-macros.h"
#include "dbus/dbus-sysdeps.h"

static void oom (const char *doing) _DBUS_GNUC_NORETURN;
static void die (const char *message) _DBUS_GNUC_NORETURN;

void
oom (const char *doing)
{
  fprintf (stderr, "*** manual-dir-iter: OOM while %s\n", doing);
  exit (1);
}

void
die (const char *message)
{
  fprintf (stderr, "*** manual-dir-iter: %s\n", message);
  exit (1);
}

static void
debug (const char *message)
{
  fprintf (stdout, "+++ manual-dir-iter: %s\n", message);
}

int
main (int    argc,
      char **argv)
{
  DBusString filename;
  DBusString dirname;
  DBusError tmp_error;
  DBusDirIter *dir;

  if (argc != 2)
      die ("syntax: manual-dir-iter <path>");

  dbus_error_init (&tmp_error);

  if (!_dbus_string_init (&filename))
      oom ("init filename");

  if (!_dbus_string_init (&dirname))
      oom ("init dirname");

  if (!_dbus_string_append (&dirname, argv[1]))
      oom ("append argv[1]");

  dir = _dbus_directory_open (&dirname, &tmp_error);

  if (dir == NULL)
    {
      fprintf (stderr, "could not open directory: %s: %s\n",
               tmp_error.name, tmp_error.message);
      exit(1);
    }

  while (_dbus_directory_get_next_file (dir, &filename, &tmp_error))
    {
      DBusString full_path;
      if (!_dbus_string_init (&full_path))
        {
          oom ("init full_path");
        }

      if (!_dbus_string_copy (&dirname, 0, &full_path, 0))
        {
          oom ("copying full_path to dirname");
        }

      if (!_dbus_concat_dir_and_file (&full_path, &filename))
        {
          oom ("concat full_path");
        }
      debug (_dbus_string_get_const_data (&filename));
      _dbus_string_free (&full_path);
    }

  if (dbus_error_is_set (&tmp_error))
      die (tmp_error.message);

  _dbus_string_free (&filename);

  if (dir)
    _dbus_directory_close (dir);

  _dbus_verbose ("*** Test dir name exiting\n");

  return 0;
}
