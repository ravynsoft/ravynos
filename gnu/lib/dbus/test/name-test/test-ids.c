#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include <dbus/dbus-connection-internal.h>
#include <dbus/dbus-valgrind-internal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static void die (const char *message) _DBUS_GNUC_NORETURN;

static void
die (const char *message)
{
  printf ("Bail out! test-ids: %s\n", message);
  exit (1);
}

static int test_num = 0;

/* This test outputs TAP syntax: http://testanything.org/ */
int
main (int    argc,
      char **argv)
{
  DBusError error;
  DBusConnection *connection;
  char *id;
  char *server_id;

  if (RUNNING_ON_VALGRIND)
    {
      printf ("1..0 # SKIP Not ready to run under valgrind yet\n");
      return 0;
    }

  dbus_error_init (&error);
  connection = dbus_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL)
    {
      fprintf (stderr, "*** Failed to open connection to session bus: %s\n",
               error.message);
      dbus_error_free (&error);
      return 1;
    }
  printf ("ok %d - connected to session bus\n", ++test_num);

  server_id = dbus_connection_get_server_id (connection);

  if (server_id == NULL)
    die ("No bus server ID retrieved\n");

  printf ("ok %d - session bus server ID is %s\n", ++test_num, server_id);

  if (strlen (server_id) != 32)
    die ("Bus server id should have length 32\n");

  printf ("ok %d - session bus server ID length is 32\n", ++test_num);

  dbus_free (server_id);

  id = dbus_bus_get_id (connection, NULL);
  if (id == NULL)
    die ("No bus ID retrieved\n");

  printf ("ok %d - session bus ID is %s\n", ++test_num, id);

  if (strlen (id) != 32)
    die ("Bus ID should have length 32\n");

  printf ("ok %d - session bus ID length is 32\n", ++test_num);

  dbus_free (id);  

  printf ("1..%d\n", test_num);
  return 0;
}
