#include <config.h>
#include <dbus/dbus-valgrind-internal.h>
#include "../test-utils.h"

static void die (const char *message,
                 ...) _DBUS_GNUC_NORETURN _DBUS_GNUC_PRINTF (1, 2);

static void
die (const char *message, ...)
{
  va_list args;
  va_start (args, message);
  vfprintf (stderr, message, args);
  va_end (args);
  exit (1);
}

#define PRIVSERVER_SERVICE "org.freedesktop.DBus.TestSuite.PrivServer"
#define PRIVSERVER_INTERFACE PRIVSERVER_SERVICE
#define PRIVSERVER_DIED_RULE \
      "type='signal',sender='" DBUS_SERVICE_DBUS "'," \
      "interface='" DBUS_INTERFACE_DBUS "',member='NameOwnerChanged'," \
      "arg0='" PRIVSERVER_SERVICE "',arg2=''"

static DBusHandlerResult
filter_session_message (DBusConnection     *connection,
                        DBusMessage        *message,
                        void               *user_data)
{
  dbus_bool_t *service_died_p = user_data;
  const char *name, *old_owner, *new_owner;

  if (dbus_message_is_signal (message,
                              DBUS_INTERFACE_DBUS,
                              "NameOwnerChanged") &&
      dbus_message_has_sender (message, DBUS_SERVICE_DBUS) &&
      dbus_message_get_args (message, NULL,
                             DBUS_TYPE_STRING, &name,
                             DBUS_TYPE_STRING, &old_owner,
                             DBUS_TYPE_STRING, &new_owner,
                             DBUS_TYPE_INVALID) &&
      strcmp (name, PRIVSERVER_SERVICE) == 0 &&
      old_owner[0] != '\0' &&
      new_owner[0] == '\0')
    {
      *service_died_p = TRUE;
    }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult 
filter_private_message (DBusConnection     *connection,
                        DBusMessage        *message,
                        void               *user_data)
{
  dbus_bool_t *private_conn_lost_p = user_data;

  if (dbus_message_is_signal (message,
                              DBUS_INTERFACE_LOCAL,
                              "Disconnected"))
    {
      *private_conn_lost_p = TRUE;
    }
  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
open_shutdown_private_connection (dbus_bool_t use_guid)
{
  DBusError error;
  DBusLoop *loop;
  DBusConnection *session;
  DBusMessage *msg;
  DBusMessage *reply;
  DBusConnection *privconn;
  char *addr;
  dbus_bool_t service_died;
  dbus_bool_t private_conn_lost;

  dbus_error_init (&error);
  service_died = FALSE;
  private_conn_lost = FALSE;

  loop = _dbus_loop_new ();

  session = dbus_bus_get (DBUS_BUS_SESSION, &error);
  if (!session)
    die ("couldn't access session bus\n");
  dbus_connection_set_exit_on_disconnect (session, FALSE);
  test_connection_setup (loop, session);

  dbus_bus_add_match (session, PRIVSERVER_DIED_RULE, &error);
  if (dbus_error_is_set (&error))
    die ("couldn't add match rule \"%s\": %s: %s", PRIVSERVER_DIED_RULE,
         error.name, error.message);

  if (!dbus_connection_add_filter (session, filter_session_message,
                                   &service_died, NULL))
    die ("couldn't add filter to session bus\n");

  msg = dbus_message_new_method_call (PRIVSERVER_SERVICE, "/",
                                      PRIVSERVER_INTERFACE, "GetPrivateAddress");
  if (!(reply = dbus_connection_send_with_reply_and_block (session, msg, -1, &error)))
    die ("couldn't send message: %s\n", error.message);
  dbus_message_unref (msg);

  if (dbus_set_error_from_message (&error, reply))
    die ("%s: %s", error.name, error.message);

  if (!dbus_message_get_args (reply, &error, DBUS_TYPE_STRING, &addr, DBUS_TYPE_INVALID))
    die ("couldn't parse message replym\n");
  printf ("# got private temp address %s\n", addr);
  addr = strdup (addr);
  if (!use_guid)
    {
      char *comma = strrchr (addr, ',');
      if (comma)
        *comma = '\0';
    }
  privconn = dbus_connection_open (addr, &error);
  free (addr);
  if (!privconn)
    die ("couldn't connect to server direct connection: %s\n", error.message);
  dbus_message_unref (reply);

  dbus_connection_set_exit_on_disconnect (privconn, FALSE);
  if (!dbus_connection_add_filter (privconn, filter_private_message,
                                   &private_conn_lost, NULL))
    die ("couldn't add filter to private connection\n");
  test_connection_setup (loop, privconn);

  msg = dbus_message_new_method_call (PRIVSERVER_SERVICE, "/",
                                      PRIVSERVER_INTERFACE, "Quit");
  if (!dbus_connection_send (session, msg, NULL))
    die ("couldn't send Quit message\n");
  dbus_message_unref (msg);

  while (!service_died || !private_conn_lost)
    _dbus_loop_iterate (loop, TRUE);

  dbus_connection_remove_filter (session, filter_session_message,
                                 &service_died);
  dbus_bus_remove_match (session, PRIVSERVER_DIED_RULE, NULL);
  test_connection_shutdown (loop, session);
  dbus_connection_unref (session);

  test_connection_shutdown (loop, privconn);
  dbus_connection_remove_filter (privconn, filter_private_message,
                                 &private_conn_lost);
  dbus_connection_unref (privconn);

  _dbus_loop_unref (loop);
}

/* This test outputs TAP syntax: http://testanything.org/ */
int
main (int argc, char *argv[])
{
  int test_num = 0;

  if (RUNNING_ON_VALGRIND)
    {
      printf ("1..0 # SKIP Not ready to run under valgrind yet\n");
      return 0;
    }

  open_shutdown_private_connection (TRUE);

  dbus_shutdown ();
  printf ("ok %d\n", ++test_num);

  open_shutdown_private_connection (TRUE);

  dbus_shutdown ();
  printf ("ok %d\n", ++test_num);

  open_shutdown_private_connection (FALSE);

  dbus_shutdown ();
  printf ("ok %d\n", ++test_num);

  open_shutdown_private_connection (FALSE);

  dbus_shutdown ();
  printf ("ok %d\n", ++test_num);

  printf ("1..%d\n", test_num);
  return 0;
}
