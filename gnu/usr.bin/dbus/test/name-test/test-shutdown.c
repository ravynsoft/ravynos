
#include <config.h>
#include <dbus/dbus-valgrind-internal.h>
#include "../test-utils.h"

static DBusLoop *loop;

static void die (const char *message) _DBUS_GNUC_NORETURN;

static void
die (const char *message)
{
  printf ("Bail out! test-shutdown: %s", message);
  exit (1);
}

static void
open_destroy_shared_session_bus_connection (void)
{
  DBusError error;
  DBusConnection *connection;
  char *session_addr_no_guid;
  char *comma;

  dbus_error_init (&error);

  session_addr_no_guid = strdup (getenv ("DBUS_SESSION_BUS_ADDRESS"));
  comma = strchr (session_addr_no_guid, ',');

#ifdef DBUS_WIN
  _dbus_assert (comma == NULL);
#else
  _dbus_assert (comma != NULL);
  *comma = '\0';
#endif

  connection = dbus_connection_open (session_addr_no_guid, &error);
  free (session_addr_no_guid);
  if (connection == NULL)
    die ("Failed to open connection to temp session bus\n");

  loop = _dbus_loop_new ();
  if (loop == NULL)
    die ("No memory\n");
  
  test_connection_setup (loop, connection);

  test_connection_shutdown (loop, connection);
 
  _dbus_loop_unref (loop);

  dbus_connection_unref (connection); 
}

/* This test outputs TAP syntax: http://testanything.org/ */
int
main (int    argc,
      char **argv)
{
  int test_num = 0;

  if (RUNNING_ON_VALGRIND)
    {
      printf ("1..0 # SKIP Not ready to run under valgrind yet\n");
      return 0;
    }

  open_destroy_shared_session_bus_connection ();

  dbus_shutdown ();
  printf ("ok %d\n", ++test_num);

  open_destroy_shared_session_bus_connection ();

  dbus_shutdown ();
  printf ("ok %d\n", ++test_num);

  open_destroy_shared_session_bus_connection ();

  dbus_shutdown ();
  printf ("ok %d\n", ++test_num);

  printf ("1..%d\n", test_num);
  return 0;
}
