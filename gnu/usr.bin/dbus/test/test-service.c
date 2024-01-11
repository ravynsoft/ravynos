#include <config.h>

#include "test-utils.h"
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static DBusLoop *loop;
static dbus_bool_t already_quit = FALSE;
static dbus_bool_t hello_from_self_reply_received = FALSE;

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

static void
check_hello_from_self_reply (DBusPendingCall *pcall, 
                             void *user_data)
{
  DBusMessage *reply;
  DBusMessage *echo_message, *echo_reply = NULL;
  DBusError error;
  DBusConnection *connection;
  
  int type;
  
  dbus_error_init (&error);
 
  connection = dbus_bus_get (DBUS_BUS_STARTER, &error);
  if (connection == NULL)
    {
      fprintf (stderr, "*** Failed to open connection to activating message bus: %s\n",
               error.message);
      dbus_error_free (&error);
      die("no memory");
    }

  
  echo_message = (DBusMessage *)user_data;
    
  reply = dbus_pending_call_steal_reply (pcall);
    
  type = dbus_message_get_type (reply);
    
  if (type == DBUS_MESSAGE_TYPE_METHOD_RETURN)
    {
      const char *s;
      fprintf (stderr, "Reply from HelloFromSelf received\n");
     
      if (!dbus_message_get_args (echo_message,
                              &error,
                              DBUS_TYPE_STRING, &s,
                              DBUS_TYPE_INVALID))
        {
            echo_reply = dbus_message_new_error (echo_message,
                                      error.name,
                                      error.message);

            if (echo_reply == NULL)
              die ("No memory\n");

        } 
      else
        {  
          echo_reply = dbus_message_new_method_return (echo_message);
          if (echo_reply == NULL)
            die ("No memory\n");
  
          if (!dbus_message_append_args (echo_reply,
                                 DBUS_TYPE_STRING, &s,
                                 DBUS_TYPE_INVALID))
            die ("No memory");
        }
        
      if (!dbus_connection_send (connection, echo_reply, NULL))
        die ("No memory\n");
      
      dbus_message_unref (echo_reply);
    }
  else if (dbus_set_error_from_message (&error, reply))
    {
      fprintf (stderr, "Error type in reply: %s\n", error.message);

      if (strcmp (error.name, DBUS_ERROR_NO_MEMORY) != 0)
        {
            echo_reply = dbus_message_new_error (echo_reply,
                                      error.name,
                                      error.message);

            if (echo_reply == NULL)
              die ("No memory\n");

            if (!dbus_connection_send (connection, echo_reply, NULL))
              die ("No memory\n");

            dbus_message_unref (echo_reply);
        }
      dbus_error_free (&error);
    }
  else
     die ("Unexpected message received");

  hello_from_self_reply_received = TRUE;
  
  dbus_message_unref (reply);
  dbus_message_unref (echo_message);
  dbus_pending_call_unref (pcall);
  dbus_connection_unref (connection);
}

static DBusHandlerResult
handle_run_hello_from_self (DBusConnection     *connection,
                                               DBusMessage        *message)
{
  DBusError error;
  DBusMessage *reply, *self_message;
  DBusPendingCall *pcall;
  char *s;

  _dbus_verbose ("sending reply to Echo method\n");
  
  dbus_error_init (&error);
  
  if (!dbus_message_get_args (message,
                              &error,
                              DBUS_TYPE_STRING, &s,
                              DBUS_TYPE_INVALID))
    {
      reply = dbus_message_new_error (message,
                                      error.name,
                                      error.message);

      if (reply == NULL)
        die ("No memory\n");

      if (!dbus_connection_send (connection, reply, NULL))
        die ("No memory\n");

      dbus_message_unref (reply);

      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    fprintf (stderr, "Sending HelloFromSelf\n");

 _dbus_verbose ("*** Sending message to self\n");
 self_message = dbus_message_new_method_call ("org.freedesktop.DBus.TestSuiteEchoService",
                                          "/org/freedesktop/TestSuite",
                                          "org.freedesktop.TestSuite",
                                          "HelloFromSelf");
  
  if (self_message == NULL)
    die ("No memory");
  
  if (!dbus_connection_send_with_reply (connection, self_message, &pcall, -1))
    die("No memory");
  
  dbus_message_ref (message);
  if (!dbus_pending_call_set_notify (pcall, check_hello_from_self_reply, (void *)message, NULL))
    die("No memory");
    
  fprintf (stderr, "Sent HelloFromSelf\n");
  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
handle_echo (DBusConnection     *connection,
             DBusMessage        *message)
{
  DBusError error;
  DBusMessage *reply;
  char *s;

  _dbus_verbose ("sending reply to Echo method\n");
  
  dbus_error_init (&error);
  
  if (!dbus_message_get_args (message,
                              &error,
                              DBUS_TYPE_STRING, &s,
                              DBUS_TYPE_INVALID))
    {
      reply = dbus_message_new_error (message,
                                      error.name,
                                      error.message);

      if (reply == NULL)
        die ("No memory\n");

      if (!dbus_connection_send (connection, reply, NULL))
        die ("No memory\n");

      dbus_message_unref (reply);

      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

  reply = dbus_message_new_method_return (message);
  if (reply == NULL)
    die ("No memory\n");
  
  if (!dbus_message_append_args (reply,
                                 DBUS_TYPE_STRING, &s,
                                 DBUS_TYPE_INVALID))
    die ("No memory");
  
  if (!dbus_connection_send (connection, reply, NULL))
    die ("No memory\n");

  fprintf (stderr, "Echo service echoed string: \"%s\"\n", s);
  
  dbus_message_unref (reply);
    
  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
handle_delay_echo (DBusConnection     *connection,
                   DBusMessage        *message)
{
  DBusError error;
  DBusMessage *reply;
  char *s;

  _dbus_verbose ("sleeping for a short time\n");

  _dbus_sleep_milliseconds (50);

  _dbus_verbose ("sending reply to DelayEcho method\n");
  
  dbus_error_init (&error);
  
  if (!dbus_message_get_args (message,
                              &error,
                              DBUS_TYPE_STRING, &s,
                              DBUS_TYPE_INVALID))
    {
      reply = dbus_message_new_error (message,
                                      error.name,
                                      error.message);

      if (reply == NULL)
        die ("No memory\n");

      if (!dbus_connection_send (connection, reply, NULL))
        die ("No memory\n");

      dbus_message_unref (reply);

      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

  reply = dbus_message_new_method_return (message);
  if (reply == NULL)
    die ("No memory\n");
  
  if (!dbus_message_append_args (reply,
                                 DBUS_TYPE_STRING, &s,
                                 DBUS_TYPE_INVALID))
    die ("No memory");
  
  if (!dbus_connection_send (connection, reply, NULL))
    die ("No memory\n");

  fprintf (stderr, "DelayEcho service echoed string: \"%s\"\n", s);
  
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
                                        "DelayEcho"))
    return handle_delay_echo (connection, message);
  else if (dbus_message_is_method_call (message,
                                        "org.freedesktop.TestSuite",
                                        "Exit"))
    {
      quit ();
      return DBUS_HANDLER_RESULT_HANDLED;
    }
  else if (dbus_message_is_method_call (message,
                                        "org.freedesktop.TestSuite",
                                        "EmitFoo"))
    {
      /* Emit the Foo signal */
      DBusMessage *signal;
      double v_DOUBLE;

      _dbus_verbose ("emitting signal Foo\n");
      
      signal = dbus_message_new_signal ("/org/freedesktop/TestSuite",
                                        "org.freedesktop.TestSuite",
                                        "Foo");
      if (signal == NULL)
        die ("No memory\n");

      v_DOUBLE = 42.6;
      if (!dbus_message_append_args (signal,
                                     DBUS_TYPE_DOUBLE, &v_DOUBLE,
                                     DBUS_TYPE_INVALID))
        die ("No memory");
  
      if (!dbus_connection_send (connection, signal, NULL))
        die ("No memory\n");
      
      return DBUS_HANDLER_RESULT_HANDLED;
    }
    
  else if (dbus_message_is_method_call (message,
                                   "org.freedesktop.TestSuite",
                                   "RunHelloFromSelf"))
    {
      return handle_run_hello_from_self (connection, message);
    }
  else if (dbus_message_is_method_call (message,
                                        "org.freedesktop.TestSuite",
                                        "HelloFromSelf"))
    {
        DBusMessage *reply;
        fprintf (stderr, "Received the HelloFromSelf message\n");
        
        reply = dbus_message_new_method_return (message);
        if (reply == NULL)
          die ("No memory");
        
        if (!dbus_connection_send (connection, reply, NULL))
          die ("No memory");

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


static const char* echo_path = "/org/freedesktop/TestSuite" ;

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
  DBusError error;
  int result;
  DBusConnection *connection;
  const char *name;
#ifndef DBUS_WIN
  dbus_bool_t do_fork = FALSE;
#endif
  if (argc != 3)
    {
      name = "org.freedesktop.DBus.TestSuiteEchoService";
    }
  else
    {
      name = argv[1];
#ifndef DBUS_WIN
      do_fork = strcmp (argv[2], "fork") == 0;
#endif
    }

  /* The bare minimum for simulating a program "daemonizing"; the intent
   * is to test services which move from being legacy init scripts to
   * activated services.
   * https://bugzilla.redhat.com/show_bug.cgi?id=545267
   */
#ifndef DBUS_WIN
   if (do_fork)
    {
      pid_t pid;

      /* Make sure our output buffers aren't redundantly printed by both the
       * parent and the child */
      fflush (stdout);
      fflush (stderr);

      pid = fork ();
      if (pid != 0)
        exit (0);
      sleep (1);
    }
#endif

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
                                             (void*) 0xdeadbeef))
    die ("No memory");

  {
    void *d;
    if (!dbus_connection_get_object_path_data (connection, echo_path, &d))
      die ("No memory");
    if (d != (void*) 0xdeadbeef)
      die ("dbus_connection_get_object_path_data() doesn't seem to work right\n");
  }

  result = dbus_bus_request_name (connection, name,
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
