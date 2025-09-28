#include <config.h>
#include "test-utils.h"

static DBusLoop *loop;

static void die (const char *message) _DBUS_GNUC_NORETURN;

static void
die (const char *message)
{
  fprintf (stderr, "*** test-names: %s", message);
  exit (1);
}

static void
TestName(DBusConnection *connection, const char *name, int expectedSuccess)
{
  DBusError error;
  dbus_error_init (&error);

  (void) dbus_bus_request_name (connection, name, 0, &error);
  if (dbus_error_is_set (&error))
    {
      if (expectedSuccess)
        fprintf (stderr, "Error acquiring name '%s': %s\n", name, error.message);
      else
        fprintf (stdout, "Expected Error acquiring name '%s': %s\n", name, error.message);
      _dbus_verbose ("*** Failed to acquire name '%s': %s\n", name,
                     error.message);
      dbus_error_free (&error);
      if (expectedSuccess)
        exit (1);
    }
  else 
    {
      if (!expectedSuccess)
        fprintf (stderr, "Unexpected Success acquiring name '%s'\n", name);
      else
        fprintf (stdout, "Successfully acquired name '%s'\n", name);
      _dbus_verbose ("*** Managed to acquire name '%s'\n", name);
      if (!expectedSuccess)
        exit (1);
    }
}

int
main (int    argc,
      char **argv)
{
  DBusError error;
  DBusConnection *connection;
  
  dbus_error_init (&error);
  connection = dbus_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL)
    {
      fprintf (stderr, "*** Failed to open connection to system bus: %s\n",
               error.message);
      dbus_error_free (&error);
      return 1;
    }

  loop = _dbus_loop_new ();
  if (loop == NULL)
    die ("No memory\n");
  
  test_connection_setup (loop, connection);

  TestName(connection, "org.freedesktop.DBus.Test", TRUE);
  TestName(connection, "org.freedesktop.DBus.Test-2", TRUE);
  TestName(connection, "org.freedesktop.DBus.Test_2", TRUE);
#if 0
  TestName(connection, "Test_2", TRUE);
#endif

  _dbus_verbose ("*** Test service name exiting\n");
  dbus_shutdown ();
  return 0;
}
