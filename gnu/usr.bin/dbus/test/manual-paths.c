/*
 * Simple manual paths check
 *
 * syntax:  manual-paths
 *
*/

#include "config.h"
#include "dbus/dbus-list.h"
#include "dbus/dbus-internals.h"
#include "dbus/dbus-sysdeps.h"
#include "test-utils.h"

#include <stdio.h>

static dbus_bool_t
print_install_root (void)
{
  DBusString runtime_prefix;

  if (!_dbus_string_init (&runtime_prefix))
    {
      _dbus_test_fatal ("out of memory");
      return FALSE;
    }

  if (!_dbus_get_install_root (&runtime_prefix))
    {
      _dbus_test_fatal ("out of memory");
      _dbus_string_free (&runtime_prefix);
      return FALSE;
    }

  if (_dbus_string_get_length (&runtime_prefix) == 0)
    {
      fprintf (stderr, "_dbus_get_install_root() failed\n");
      _dbus_string_free (&runtime_prefix);
      return FALSE;
    }

  fprintf (stdout, "_dbus_get_install_root() returned '%s'\n",
      _dbus_string_get_const_data (&runtime_prefix));
  _dbus_string_free (&runtime_prefix);
  return TRUE;
}

static dbus_bool_t
print_service_dirs (void)
{
  DBusList *dirs;
  DBusList *link;
  dirs = NULL;

  if (!_dbus_get_standard_session_servicedirs (&dirs))
    _dbus_test_fatal ("couldn't get standard dirs");

  while ((link = _dbus_list_pop_first_link (&dirs)))
    {
      printf ("default service dir: %s\n", (char *)link->data);
      dbus_free (link->data);
      _dbus_list_free_link (link);
    }
  dbus_free (dirs);
  return TRUE;
}

static dbus_bool_t print_replace_install_prefix(const char *s)
{
  DBusString str;

  if (!_dbus_string_init (&str))
    {
      _dbus_test_fatal ("out of memory");
      return FALSE;
    }

  if (!_dbus_string_append (&str, s) ||
      !_dbus_replace_install_prefix (&str))
    {
      _dbus_test_fatal ("out of memory");
      _dbus_string_free (&str);
      return FALSE;
    }

  fprintf(stdout, "replaced '%s' by '%s'\n", s,
      _dbus_string_get_const_data (&str));
  _dbus_string_free (&str);
  return TRUE;
}

int
main (int argc, char **argv)
{
  if (!print_install_root())
    return -1;

  if (!print_service_dirs())
    return -2;

  if (!print_replace_install_prefix(DBUS_BINDIR "/dbus-daemon"))
    return -3;

  if (!print_replace_install_prefix("c:\\Windows\\System32\\testfile"))
    return -4;

  return 0;
}
