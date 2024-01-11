#include <config.h>

#include "test-utils.h"

static DBusLoop *loop;
static dbus_bool_t already_quit = FALSE;
static const char* echo_path = "/org/freedesktop/TestSuite";

typedef struct
{
  int argc;
  char **argv;
} EchoData;

static void
quit (void)
{
  if (!already_quit)
    {
      _dbus_loop_quit (loop);
      already_quit = TRUE;
    }
}

static void die (const char *message) _DBUS_GNUC_NORETURN;

static void
die (const char *message)
{
  fprintf (stderr, "*** test-service: %s", message);
  exit (1);
}

static DBusHandlerResult
handle_echo (DBusConnection     *connection,
             DBusMessage        *message)
{
  DBusError error;
  DBusMessage *reply;
  DBusMessageIter iter;
  int i;
  EchoData *d;

  _dbus_verbose ("sending reply to Echo method\n");

  if (!dbus_connection_get_object_path_data (connection, echo_path, (void **)&d))
      die ("No memory");


  dbus_error_init (&error);

  reply = dbus_message_new_method_return (message);
  if (reply == NULL)
    die ("No memory\n");
  
  dbus_message_iter_init_append (reply, &iter);
  for (i = 0; i < d->argc; ++i)
    if (!dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &(d->argv[i])))
      die ("No memory\n");

  if (!dbus_connection_send (connection, reply, NULL))
    die ("No memory\n");

  fprintf (stderr, "Shell echo service echoed the command line\n");
  
  dbus_message_unref (reply);
    
  return DBUS_HANDLER_RESULT_HANDLED;
}

static void
path_unregistered_func (DBusConnection  *connection,
                        void            *user_data)
{
  /* connection was finalized */
}

static DBusHandlerResult
path_message_func (DBusConnection  *connection,
                   DBusMessage     *message,
                   void            *user_data)
{
  if (dbus_message_is_method_call (message,
                                   "org.freedesktop.TestSuite",
                                   "Echo"))
    return handle_echo (connection, message);
  else if (dbus_message_is_method_call (message,
                                        "org.freedesktop.TestSuite",
                                        "Exit"))
    {
      quit ();
      return DBUS_HANDLER_RESULT_HANDLED;
    }
  else
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusObjectPathVTable
echo_vtable = {
  path_unregistered_func,
  path_message_func,
  NULL,
};

static DBusHandlerResult
filter_func (DBusConnection     *connection,
             DBusMessage        *message,
             void               *user_data)
{
  if (dbus_message_is_signal (message,
                              DBUS_INTERFACE_LOCAL,
                              "Disconnected"))
    {
      quit ();
      return DBUS_HANDLER_RESULT_HANDLED;
    }
  else
    {
      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
}

int
main (int    argc,
      char **argv)
{
  DBusConnection *connection;
  DBusError error;
  EchoData echo_data;
  int result;
  
  echo_data.argc = argc;
  echo_data.argv = argv;
  
  dbus_error_init (&error);
  connection = dbus_bus_get (DBUS_BUS_STARTER, &error);
  if (connection == NULL)
    {
      fprintf (stderr, "*** Failed to open connection to activating message bus: %s\n",
               error.message);
      dbus_error_free (&error);
      return 1;
    }

  loop = _dbus_loop_new ();
  if (loop == NULL)
    die ("No memory\n");

  test_connection_setup (loop, connection);

  if (!dbus_connection_add_filter (connection,
                                   filter_func, NULL, NULL))
    die ("No memory");

  if (!dbus_connection_register_object_path (connection,
                                             echo_path,
                                             &echo_vtable,
                                             (void*) &echo_data))
    die ("No memory");

  {
    void *d;
    if (!dbus_connection_get_object_path_data (connection, echo_path, &d))
      die ("No memory");
    if (d != (void*) &echo_data)
      die ("dbus_connection_get_object_path_data() doesn't seem to work right\n");
  }
  
  result = dbus_bus_request_name (connection, "org.freedesktop.DBus.TestSuiteShellEchoServiceSuccess",
                                  0, &error);
  if (dbus_error_is_set (&error))
    {
      fprintf (stderr, "Error %s\n", error.message);
      _dbus_verbose ("*** Failed to acquire service: %s\n",
                     error.message);
      dbus_error_free (&error);
      exit (1);
    }

  if (result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
      fprintf (stderr, "Unable to acquire service: code %d\n", result);
      _dbus_verbose ("*** Failed to acquire service: %d\n", result);
      exit (1);
    }

  _dbus_verbose ("*** Test service entering main loop\n");
  _dbus_loop_run (loop);

  test_connection_shutdown (loop, connection);

  dbus_connection_remove_filter (connection, filter_func, NULL);
  
  dbus_connection_unref (connection);

  _dbus_loop_unref (loop);
  loop = NULL;
  
  dbus_shutdown ();

  _dbus_verbose ("*** Test service exiting\n");
  
  return 0;
}
