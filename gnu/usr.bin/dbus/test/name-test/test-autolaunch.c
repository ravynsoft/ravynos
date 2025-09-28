#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <dbus/dbus.h>
#include "dbus/dbus-internals.h"
#include "dbus/dbus-sysdeps.h"

int
main (int argc, char *argv[])
{
  DBusConnection *conn = NULL;
  DBusError error;
  DBusGUID uuid;

  dbus_setenv ("DBUS_SESSION_BUS_ADDRESS", NULL);

  dbus_error_init (&error);

  if (!_dbus_read_local_machine_uuid (&uuid, FALSE, &error))
    {
      /* We can't expect autolaunching to work in this situation */
      fprintf (stderr, "*** %s\n", error.message);
      dbus_error_free (&error);
      return 0;
    }

  conn = dbus_bus_get (DBUS_BUS_SESSION, &error);

#ifdef DBUS_ENABLE_X11_AUTOLAUNCH
  /* If X11 autolaunch was enabled, we expect dbus-launch to have worked. */
  if (_dbus_getenv ("DISPLAY") != NULL && dbus_error_is_set (&error))
    {
      fprintf (stderr, "*** Failed to autolaunch session bus: %s\n",
               error.message);
      dbus_error_free (&error);
      return 1;
    }
#endif

  /* We don't necessarily expect it to *work* without X (although it might -
   * for instance on Mac OS it might have used launchd). Just check that the
   * results are consistent. */

  if (dbus_error_is_set (&error) && conn != NULL)
    {
      fprintf (stderr, "*** Autolaunched session bus, but an error was set!\n");
      return 1;
    }

  if (!dbus_error_is_set (&error) && conn == NULL)
    {
      fprintf (stderr, "*** Failed to autolaunch session bus but no error was set\n");
      return 1;
    }

  return 0;
}
