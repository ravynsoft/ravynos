/* Integration tests for monitor-mode D-Bus connections
 *
 * Copyright © 2010-2011 Nokia Corporation
 * Copyright © 2015 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <config.h>

#include <string.h>

#include "dbus/dbus-connection-internal.h"

#include "test-utils-glib.h"

typedef struct {
    const char *config_file;
    const char * const *match_rules;
    gboolean care_about_our_names;
} Config;

typedef struct {
    const Config *config;
    TestMainContext *ctx;
    DBusError e;
    GError *ge;

    gchar *address;
    GPid daemon_pid;

    DBusConnection *monitor;
    DBusConnection *sender;
    DBusConnection *recipient;
    gboolean recipient_enqueue_filter_added;

    GQueue monitored;
    GQueue received;

    const char *monitor_name;
    const char *sender_name;
    const char *recipient_name;

    DBusConnection *systemd;
    const char *systemd_name;
    DBusMessage *systemd_message;
    DBusConnection *activated;
    const char *activated_name;
    DBusMessage *activated_message;
} Fixture;

static const char * const no_match_rules[] = {
    NULL
};

static const char * const wildcard_match_rules[] = {
    "",
    NULL,
    FALSE
};

static const char * const eavesdrop_match_rules[] = {
    "eavesdrop=true",
    NULL,
    FALSE
};

static const char * const no_eavesdrop_match_rules[] = {
    "eavesdrop=false",
    NULL,
    FALSE
};

static const char * const selective_match_rules[] = {
    "interface='com.example.Interesting'",
    "interface='com.example.Fun'",
    NULL,
    FALSE
};

static const char * const well_known_destination_match_rules[] = {
    "destination='com.example.Recipient'",
    NULL
};

static Config forbidding_config = {
    "valid-config-files/forbidding.conf",
    NULL,
    FALSE
};

static Config wildcard_config = {
    NULL,
    wildcard_match_rules,
    FALSE
};

static Config selective_config = {
    NULL,
    selective_match_rules,
    FALSE
};

static Config well_known_destination_config = {
    NULL,
    well_known_destination_match_rules,
    FALSE
};

static Config no_rules_config = {
    NULL,
    no_match_rules,
    FALSE
};

static Config eavesdrop_config = {
    NULL,
    eavesdrop_match_rules,
    FALSE
};

static Config no_eavesdrop_config = {
    NULL,
    no_eavesdrop_match_rules,
    FALSE
};

#ifdef DBUS_UNIX
static Config fake_systemd_config = {
    "valid-config-files/systemd-activation.conf",
    NULL,
    FALSE
};
#endif

static Config side_effects_config = {
    NULL,
    NULL,
    TRUE
};

static dbus_bool_t
config_forbids_name_acquired_signal (const Config *config)
{
  if (config == NULL)
    return FALSE;

  if (config->config_file == NULL)
    return FALSE;

  if (strcmp (config->config_file, forbidding_config.config_file) == 0)
    return TRUE;

  return FALSE;
}

static inline const char *
not_null2 (const char *x,
    const char *fallback)
{
  if (x == NULL)
    return fallback;

  return x;
}

static inline const char *
not_null (const char *x)
{
  return not_null2 (x, "(null)");
}

#define log_message(m) _log_message (m, __FILE__, __LINE__)

G_GNUC_UNUSED
static void
_log_message (DBusMessage *m,
    const char *file,
    int line)
{
  g_test_message ("%s:%d: message type %d (%s)", file, line,
      dbus_message_get_type (m),
      dbus_message_type_to_string (dbus_message_get_type (m)));
  g_test_message ("\tfrom: %s",
      not_null2 (dbus_message_get_sender (m), "(dbus-daemon)"));
  g_test_message ("\tto: %s",
      not_null2 (dbus_message_get_destination (m), "(broadcast)"));
  g_test_message ("\tpath: %s",
      not_null (dbus_message_get_path (m)));
  g_test_message ("\tinterface: %s",
      not_null (dbus_message_get_interface (m)));
  g_test_message ("\tmember: %s",
      not_null (dbus_message_get_member (m)));
  g_test_message ("\tsignature: %s",
      not_null (dbus_message_get_signature (m)));
  g_test_message ("\terror name: %s",
      not_null (dbus_message_get_error_name (m)));
  g_test_message ("\tserial number: %u",
      dbus_message_get_serial (m));
  g_test_message ("\tin reply to: %u",
      dbus_message_get_reply_serial (m));

  if (strcmp ("s", dbus_message_get_signature (m)) == 0)
    {
      DBusError e = DBUS_ERROR_INIT;
      const char *s;

      dbus_message_get_args (m, &e,
            DBUS_TYPE_STRING, &s,
            DBUS_TYPE_INVALID);
      test_assert_no_error (&e);
      g_test_message ("\tstring payload: %s", s);
    }
}

/* these are macros so they get the right line number */

#define assert_hello(m) \
do { \
  g_assert_cmpstr (dbus_message_type_to_string (dbus_message_get_type (m)), \
      ==, dbus_message_type_to_string (DBUS_MESSAGE_TYPE_METHOD_CALL)); \
  g_assert_cmpstr (dbus_message_get_destination (m), ==, DBUS_SERVICE_DBUS); \
  g_assert_cmpstr (dbus_message_get_path (m), ==, DBUS_PATH_DBUS); \
  g_assert_cmpstr (dbus_message_get_interface (m), ==, DBUS_INTERFACE_DBUS); \
  g_assert_cmpstr (dbus_message_get_member (m), ==, "Hello"); \
  g_assert_cmpstr (dbus_message_get_signature (m), ==, ""); \
  g_assert_cmpint (dbus_message_get_serial (m), !=, 0); \
  g_assert_cmpint (dbus_message_get_reply_serial (m), ==, 0); \
} while (0)

#define assert_hello_reply(m) \
do { \
  DBusError _e = DBUS_ERROR_INIT; \
  const char *_s; \
    \
  g_assert_cmpstr (dbus_message_type_to_string (dbus_message_get_type (m)), \
      ==, dbus_message_type_to_string (DBUS_MESSAGE_TYPE_METHOD_RETURN)); \
  g_assert_cmpstr (dbus_message_get_sender (m), ==, DBUS_SERVICE_DBUS); \
  g_assert_cmpstr (dbus_message_get_path (m), ==, NULL); \
  g_assert_cmpstr (dbus_message_get_interface (m), ==, NULL); \
  g_assert_cmpstr (dbus_message_get_member (m), ==, NULL); \
  g_assert_cmpstr (dbus_message_get_signature (m), ==, "s"); \
  g_assert_cmpint (dbus_message_get_serial (m), !=, 0); \
  g_assert_cmpint (dbus_message_get_reply_serial (m), !=, 0); \
    \
  dbus_message_get_args (m, &_e, \
        DBUS_TYPE_STRING, &_s, \
        DBUS_TYPE_INVALID); \
  test_assert_no_error (&_e); \
  g_assert_cmpstr (dbus_message_get_destination (m), ==, _s); \
} while (0)

#define assert_name_acquired(m) \
do { \
  g_assert_cmpstr (dbus_message_type_to_string (dbus_message_get_type (m)), \
      ==, dbus_message_type_to_string (DBUS_MESSAGE_TYPE_SIGNAL)); \
  g_assert_cmpstr (dbus_message_get_sender (m), ==, DBUS_SERVICE_DBUS); \
  g_assert_cmpstr (dbus_message_get_path (m), ==, DBUS_PATH_DBUS); \
  g_assert_cmpstr (dbus_message_get_interface (m), ==, DBUS_INTERFACE_DBUS); \
  g_assert_cmpstr (dbus_message_get_member (m), ==, "NameAcquired"); \
  g_assert_cmpstr (dbus_message_get_signature (m), ==, "s"); \
  g_assert_cmpint (dbus_message_get_serial (m), !=, 0); \
  g_assert_cmpint (dbus_message_get_reply_serial (m), ==, 0); \
} while (0)

#define assert_unique_name_acquired(m) \
do { \
  DBusError _e = DBUS_ERROR_INIT; \
  const char *_s; \
    \
  assert_name_acquired (m); \
  dbus_message_get_args (m, &_e, \
        DBUS_TYPE_STRING, &_s, \
        DBUS_TYPE_INVALID); \
  test_assert_no_error (&_e); \
  g_assert_cmpstr (dbus_message_get_destination (m), ==, _s); \
} while (0)

#define assert_method_call(m, sender, \
    destination, path, iface, method, signature) \
do { \
  g_assert_cmpstr (dbus_message_type_to_string (dbus_message_get_type (m)), \
      ==, dbus_message_type_to_string (DBUS_MESSAGE_TYPE_METHOD_CALL)); \
  g_assert_cmpstr (dbus_message_get_sender (m), ==, sender); \
  g_assert_cmpstr (dbus_message_get_destination (m), ==, destination); \
  g_assert_cmpstr (dbus_message_get_path (m), ==, path); \
  g_assert_cmpstr (dbus_message_get_interface (m), ==, iface); \
  g_assert_cmpstr (dbus_message_get_member (m), ==, method); \
  g_assert_cmpstr (dbus_message_get_signature (m), ==, signature); \
  g_assert_cmpint (dbus_message_get_serial (m), !=, 0); \
  g_assert_cmpint (dbus_message_get_reply_serial (m), ==, 0); \
} while (0)

#define assert_signal(m, \
    sender, path, iface, member, signature, \
    destination) \
do { \
  g_assert_cmpstr (dbus_message_type_to_string (dbus_message_get_type (m)), \
      ==, dbus_message_type_to_string (DBUS_MESSAGE_TYPE_SIGNAL)); \
  g_assert_cmpstr (dbus_message_get_sender (m), ==, sender); \
  g_assert_cmpstr (dbus_message_get_destination (m), ==, destination); \
  g_assert_cmpstr (dbus_message_get_path (m), ==, path); \
  g_assert_cmpstr (dbus_message_get_interface (m), ==, iface); \
  g_assert_cmpstr (dbus_message_get_member (m), ==, member); \
  g_assert_cmpstr (dbus_message_get_signature (m), ==, signature); \
  g_assert_cmpint (dbus_message_get_serial (m), !=, 0); \
  g_assert_cmpint (dbus_message_get_reply_serial (m), ==, 0); \
} while (0)

#define assert_method_reply(m, sender, destination, signature) \
do { \
  g_assert_cmpstr (dbus_message_type_to_string (dbus_message_get_type (m)), \
      ==, dbus_message_type_to_string (DBUS_MESSAGE_TYPE_METHOD_RETURN)); \
  g_assert_cmpstr (dbus_message_get_sender (m), ==, sender); \
  g_assert_cmpstr (dbus_message_get_destination (m), ==, destination); \
  g_assert_cmpstr (dbus_message_get_path (m), ==, NULL); \
  g_assert_cmpstr (dbus_message_get_interface (m), ==, NULL); \
  g_assert_cmpstr (dbus_message_get_member (m), ==, NULL); \
  g_assert_cmpstr (dbus_message_get_signature (m), ==, signature); \
  g_assert_cmpint (dbus_message_get_serial (m), !=, 0); \
  g_assert_cmpint (dbus_message_get_reply_serial (m), !=, 0); \
} while (0)

#define assert_error_reply(m, sender, destination, error_name) \
do { \
  g_assert_cmpstr (dbus_message_type_to_string (dbus_message_get_type (m)), \
      ==, dbus_message_type_to_string (DBUS_MESSAGE_TYPE_ERROR)); \
  g_assert_cmpstr (dbus_message_get_sender (m), ==, sender); \
  g_assert_cmpstr (dbus_message_get_destination (m), ==, destination); \
  g_assert_cmpstr (dbus_message_get_error_name (m), ==, error_name); \
  g_assert_cmpstr (dbus_message_get_path (m), ==, NULL); \
  g_assert_cmpstr (dbus_message_get_interface (m), ==, NULL); \
  g_assert_cmpstr (dbus_message_get_member (m), ==, NULL); \
  g_assert_cmpstr (dbus_message_get_signature (m), ==, "s"); \
  g_assert_cmpint (dbus_message_get_serial (m), !=, 0); \
  g_assert_cmpint (dbus_message_get_reply_serial (m), !=, 0); \
} while (0)

/* forbidding.conf does not allow receiving NameAcquired, so if we are in
 * that configuration, then dbus-daemon synthesizes an error reply to itself
 * and sends that to monitors */
#define expect_name_acquired_error(queue, in_reply_to) \
do { \
  DBusMessage *message; \
  \
  message = g_queue_pop_head (queue); \
  assert_error_reply (message, DBUS_SERVICE_DBUS, DBUS_SERVICE_DBUS, \
                      DBUS_ERROR_ACCESS_DENIED); \
  g_assert_cmpint (dbus_message_get_reply_serial (message), ==, \
                   dbus_message_get_serial (in_reply_to)); \
  dbus_message_unref (message); \
} while (0)

/* This is called after processing pending replies to our own method
 * calls, but before anything else.
 */
static DBusHandlerResult
monitor_filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  Fixture *f = user_data;

  g_test_message ("Monitor received message:");
  log_message (message);

  g_assert_cmpstr (dbus_message_get_interface (message), !=,
      "com.example.Tedious");

  /* we are not interested in the monitor getting NameAcquired or NameLost
   * for most tests */
  if (f->config == NULL || !f->config->care_about_our_names)
    {
      if (dbus_message_is_signal (message, DBUS_INTERFACE_DBUS,
            "NameAcquired") ||
          dbus_message_is_signal (message, DBUS_INTERFACE_DBUS,
            "NameLost"))
        {
          DBusError e = DBUS_ERROR_INIT;
          const char *s;

          dbus_message_get_args (message, &e,
                DBUS_TYPE_STRING, &s,
                DBUS_TYPE_INVALID);
          test_assert_no_error (&e);

          if (strcmp (s, f->monitor_name) == 0)
            {
              /* ignore */
              return DBUS_HANDLER_RESULT_HANDLED;
            }
        }
    }

  g_queue_push_tail (&f->monitored, dbus_message_ref (message));

  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
recipient_check_filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  g_assert_cmpstr (dbus_message_get_interface (message), !=,
      "com.example.CannotSend");
  g_assert_cmpstr (dbus_message_get_interface (message), !=,
      "com.example.CannotReceive");

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult
recipient_enqueue_filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  Fixture *f = user_data;

  if (dbus_message_is_signal (message, DBUS_INTERFACE_DBUS,
        "NameAcquired") ||
      dbus_message_is_signal (message, DBUS_INTERFACE_DBUS,
        "NameLost") ||
      dbus_message_is_signal (message, DBUS_INTERFACE_DBUS,
        "NameOwnerChanged"))
    {
      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

  g_queue_push_tail (&f->received, dbus_message_ref (message));
  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult
systemd_filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  Fixture *f = user_data;

  if (dbus_message_is_signal (message, DBUS_INTERFACE_DBUS,
        "NameAcquired") ||
      dbus_message_is_signal (message, DBUS_INTERFACE_DBUS,
        "NameLost"))
    {
      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

  g_assert (f->systemd_message == NULL);
  f->systemd_message = dbus_message_ref (message);

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult
activated_filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  Fixture *f = user_data;

  if (dbus_message_is_signal (message, DBUS_INTERFACE_DBUS,
        "NameAcquired") ||
      dbus_message_is_signal (message, DBUS_INTERFACE_DBUS,
        "NameLost"))
    {
      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

  g_assert (f->activated_message == NULL);
  f->activated_message = dbus_message_ref (message);

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
take_well_known_name (Fixture *f,
    DBusConnection *connection,
    const char *name)
{
  int ret;

  ret = dbus_bus_request_name (connection, name,
      DBUS_NAME_FLAG_DO_NOT_QUEUE, &f->e);
  test_assert_no_error (&f->e);
  g_assert_cmpint (ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
}

static void
setup (Fixture *f,
    gconstpointer context)
{
  f->config = context;

  f->ctx = test_main_context_get ();

  f->ge = NULL;
  dbus_error_init (&f->e);

  f->address = test_get_dbus_daemon (f->config ? f->config->config_file : NULL,
      TEST_USER_ME, NULL, &f->daemon_pid);

  if (f->address == NULL)
    return;

  f->monitor = test_connect_to_bus (f->ctx, f->address);
  f->monitor_name = dbus_bus_get_unique_name (f->monitor);
  f->sender = test_connect_to_bus (f->ctx, f->address);
  f->sender_name = dbus_bus_get_unique_name (f->sender);
  f->recipient = test_connect_to_bus (f->ctx, f->address);
  f->recipient_name = dbus_bus_get_unique_name (f->recipient);

  if (!dbus_connection_add_filter (f->monitor, monitor_filter, f, NULL))
    g_error ("OOM");

  if (!dbus_connection_add_filter (f->recipient, recipient_check_filter,
        f, NULL))
    g_error ("OOM");
}

static void
become_monitor (Fixture *f,
    const Config *config)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  dbus_bool_t ok;
  DBusMessageIter appender, array_appender;
  const char * const *match_rules;
  int i;
  dbus_uint32_t zero = 0;

  _dbus_connection_set_builtin_filters_enabled (f->monitor, FALSE);

  if (config == NULL)
    config = f->config;

  if (config != NULL && config->match_rules != NULL)
    match_rules = config->match_rules;
  else
    match_rules = wildcard_match_rules;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_MONITORING, "BecomeMonitor");

  if (m == NULL)
    g_error ("OOM");

  dbus_message_iter_init_append (m, &appender);

  if (!dbus_message_iter_open_container (&appender, DBUS_TYPE_ARRAY, "s",
        &array_appender))
    g_error ("OOM");

  for (i = 0; match_rules[i] != NULL; i++)
    {
      if (!dbus_message_iter_append_basic (&array_appender, DBUS_TYPE_STRING,
            &match_rules[i]))
        g_error ("OOM");
    }

  if (!dbus_message_iter_close_container (&appender, &array_appender) ||
      !dbus_message_iter_append_basic (&appender, DBUS_TYPE_UINT32, &zero))
    g_error ("OOM");

  reply = test_main_context_call_and_wait (f->ctx, f->monitor, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  ok = dbus_message_get_args (reply, &f->e,
      DBUS_TYPE_INVALID);
  test_assert_no_error (&f->e);
  g_assert (ok);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

/*
 * Test what happens if the method call arguments are invalid.
 */
static void
test_invalid (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  dbus_bool_t ok;
  DBusMessageIter appender, array_appender;
  dbus_uint32_t zero = 0;
  dbus_uint32_t invalid_flags = G_MAXUINT32;
  const char *s;

  if (f->address == NULL)
    return;

  dbus_connection_set_route_peer_messages (f->monitor, TRUE);

  /* Try to become a monitor but specify nonzero flags - not allowed */

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_MONITORING, "BecomeMonitor");

  if (m == NULL)
    g_error ("OOM");

  dbus_message_iter_init_append (m, &appender);

  if (!dbus_message_iter_open_container (&appender, DBUS_TYPE_ARRAY, "s",
        &array_appender))
    g_error ("OOM");

  if (!dbus_message_iter_close_container (&appender, &array_appender) ||
      !dbus_message_iter_append_basic (&appender, DBUS_TYPE_UINT32,
        &invalid_flags))
    g_error ("OOM");

  reply = test_main_context_call_and_wait (f->ctx, f->monitor, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  g_assert_cmpint (dbus_message_get_type (reply), ==, DBUS_MESSAGE_TYPE_ERROR);
  g_assert_cmpstr (dbus_message_get_error_name (reply), ==,
      DBUS_ERROR_INVALID_ARGS);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);

  /* Try to become a monitor but use the wrong object path - not allowed
   * (security hardening against inappropriate XML policy rules) */

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      "/", DBUS_INTERFACE_MONITORING, "BecomeMonitor");

  if (m == NULL)
    g_error ("OOM");

  dbus_message_iter_init_append (m, &appender);

  if (!dbus_message_iter_open_container (&appender, DBUS_TYPE_ARRAY, "s",
        &array_appender))
    g_error ("OOM");

  if (!dbus_message_iter_close_container (&appender, &array_appender) ||
      !dbus_message_iter_append_basic (&appender, DBUS_TYPE_UINT32, &zero))
    g_error ("OOM");

  reply = test_main_context_call_and_wait (f->ctx, f->monitor, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  g_assert_cmpint (dbus_message_get_type (reply), ==, DBUS_MESSAGE_TYPE_ERROR);
  g_assert_cmpstr (dbus_message_get_error_name (reply), ==,
      DBUS_ERROR_UNKNOWN_INTERFACE);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);

  /* Try to become a monitor but specify a bad match rule -
   * also not allowed */

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_MONITORING, "BecomeMonitor");

  if (m == NULL)
    g_error ("OOM");

  dbus_message_iter_init_append (m, &appender);

  if (!dbus_message_iter_open_container (&appender, DBUS_TYPE_ARRAY, "s",
        &array_appender))
    g_error ("OOM");

  /* Syntactically incorrect match rule taken from #92298 - was probably
   * intended to be path='/modules/...'
   */
  s = "interface='org.kde.walletd',member='/modules/kwalletd/org.kde.KWallet/walletOpened'";

  if (!dbus_message_iter_append_basic (&array_appender, DBUS_TYPE_STRING,
        &s) ||
      !dbus_message_iter_close_container (&appender, &array_appender) ||
      !dbus_message_iter_append_basic (&appender, DBUS_TYPE_UINT32, &zero))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->monitor, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  g_assert_cmpint (dbus_message_get_type (reply), ==, DBUS_MESSAGE_TYPE_ERROR);
  g_assert_cmpstr (dbus_message_get_error_name (reply), ==,
      DBUS_ERROR_MATCH_RULE_INVALID);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);

  /* We did not become a monitor, so we can still call methods. */

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "GetId");

  if (m == NULL)
    g_error ("OOM");

  reply = test_main_context_call_and_wait (f->ctx, f->monitor, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (dbus_set_error_from_message (&f->e, reply))
    g_error ("%s: %s", f->e.name, f->e.message);

  ok = dbus_message_get_args (reply, &f->e,
      DBUS_TYPE_STRING, &s,
      DBUS_TYPE_INVALID);
  test_assert_no_error (&f->e);
  g_assert (ok);
  g_assert_cmpstr (s, !=, NULL);
  g_assert_cmpstr (s, !=, "");

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

/*
 * Test the side-effects of becoming a monitor.
 */
static void
test_become_monitor (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;
  int ret;
  dbus_bool_t got_unique = FALSE, got_a = FALSE, got_b = FALSE, got_c = FALSE;
  dbus_bool_t lost_unique = FALSE, lost_a = FALSE, lost_b = FALSE, lost_c = FALSE;

  if (f->address == NULL)
    return;

  ret = dbus_bus_request_name (f->monitor, "com.example.A",
      DBUS_NAME_FLAG_DO_NOT_QUEUE, &f->e);
  test_assert_no_error (&f->e);
  g_assert_cmpint (ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

  ret = dbus_bus_request_name (f->monitor, "com.example.B",
      DBUS_NAME_FLAG_DO_NOT_QUEUE, &f->e);
  test_assert_no_error (&f->e);
  g_assert_cmpint (ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

  ret = dbus_bus_request_name (f->monitor, "com.example.C",
      DBUS_NAME_FLAG_DO_NOT_QUEUE, &f->e);
  test_assert_no_error (&f->e);
  g_assert_cmpint (ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

  /* If the policy forbids receiving NameAcquired, then we'll never
   * receive it, so behave as though we had */
  if (config_forbids_name_acquired_signal (f->config))
    got_unique = got_a = got_b = got_c = TRUE;

  while (!got_unique || !got_a || !got_b || !got_c)
    {
      if (g_queue_is_empty (&f->monitored))
        test_main_context_iterate (f->ctx, TRUE);

      while ((m = g_queue_pop_head (&f->monitored)) != NULL)
        {
          if (dbus_message_is_signal (m, DBUS_INTERFACE_DBUS,
                "NameAcquired"))
            {
              const char *name;
              dbus_bool_t ok = dbus_message_get_args (m, &f->e,
                  DBUS_TYPE_STRING, &name,
                  DBUS_TYPE_INVALID);

              g_assert_cmpstr (dbus_message_get_path (m), ==,
                  DBUS_PATH_DBUS);

              test_assert_no_error (&f->e);
              g_assert (ok);

              if (g_str_equal (name, f->monitor_name))
                {
                  g_assert (!got_unique);
                  got_unique = TRUE;
                }
              else if (g_str_equal (name, "com.example.A"))
                {
                  g_assert (!got_a);
                  got_a = TRUE;
                }
              else if (g_str_equal (name, "com.example.B"))
                {
                  g_assert (!got_b);
                  got_b = TRUE;
                }
              else
                {
                  g_assert_cmpstr (name, ==, "com.example.C");
                  g_assert (!got_c);
                  got_c = TRUE;
                }
            }
          else
            {
              g_error ("unexpected message %s.%s",
                  dbus_message_get_interface (m),
                  dbus_message_get_member (m));
            }

          dbus_message_unref (m);
        }
    }

  become_monitor (f, NULL);

  while (!lost_unique || !lost_a || !lost_b || !lost_c)
    {
      if (g_queue_is_empty (&f->monitored))
        test_main_context_iterate (f->ctx, TRUE);

      while ((m = g_queue_pop_head (&f->monitored)) != NULL)
        {
          if (dbus_message_is_signal (m, DBUS_INTERFACE_DBUS,
                "NameLost"))
            {
              const char *name;
              dbus_bool_t ok = dbus_message_get_args (m, &f->e,
                  DBUS_TYPE_STRING, &name,
                  DBUS_TYPE_INVALID);

              test_assert_no_error (&f->e);
              g_assert (ok);

              if (g_str_equal (name, f->monitor_name))
                {
                  g_assert (!lost_unique);
                  lost_unique = TRUE;
                }
              else if (g_str_equal (name, "com.example.A"))
                {
                  g_assert (!lost_a);
                  lost_a = TRUE;
                }
              else if (g_str_equal (name, "com.example.B"))
                {
                  g_assert (!lost_b);
                  lost_b = TRUE;
                }
              else
                {
                  g_assert_cmpstr (name, ==, "com.example.C");
                  g_assert (!lost_c);
                  lost_c = TRUE;
                }
            }
          else
            {
              g_error ("unexpected message %s.%s",
                  dbus_message_get_interface (m),
                  dbus_message_get_member (m));
            }

          dbus_message_unref (m);
        }
    }

  /* Calling methods is forbidden; we get disconnected. */
  dbus_bus_add_match (f->monitor, "", &f->e);
  g_assert_cmpstr (f->e.name, ==, DBUS_ERROR_NO_REPLY);
  g_assert (!dbus_connection_get_is_connected (f->monitor));

  while (TRUE)
    {
      if (g_queue_is_empty (&f->monitored))
        test_main_context_iterate (f->ctx, TRUE);

      /* When we iterate all the connection's messages, we see ourselves
       * losing all our names, then we're disconnected. */
      while ((m = g_queue_pop_head (&f->monitored)) != NULL)
        {
          if (dbus_message_is_signal (m, DBUS_INTERFACE_LOCAL, "Disconnected"))
            {
              dbus_message_unref (m);
              goto disconnected;
            }
          else
            {
              g_error ("unexpected message %s.%s",
                  dbus_message_get_interface (m),
                  dbus_message_get_member (m));
            }

          dbus_message_unref (m);
        }
    }

disconnected:

  g_assert (lost_a);
  g_assert (lost_b);
  g_assert (lost_c);
}

static void
test_broadcast (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;

  if (f->address == NULL)
    return;

  dbus_bus_add_match (f->recipient, "type='signal'", &f->e);
  test_assert_no_error (&f->e);

  become_monitor (f, NULL);

  m = dbus_message_new_signal ("/foo", "com.example.bar", "BroadcastSignal1");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.bar", "BroadcastSignal2");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.bar", "BroadcastSignal3");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 3)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.bar",
      "BroadcastSignal1", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.bar",
      "BroadcastSignal2", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.bar",
      "BroadcastSignal3", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);
}

static void
test_forbidden_broadcast (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;

  if (f->address == NULL)
    return;

  dbus_bus_add_match (f->recipient, "type='signal'", &f->e);
  test_assert_no_error (&f->e);

  if (!dbus_connection_add_filter (f->recipient, recipient_enqueue_filter,
        f, NULL))
    g_error ("OOM");
  f->recipient_enqueue_filter_added = TRUE;

  become_monitor (f, NULL);

  m = dbus_message_new_signal ("/foo", "com.example.CannotSend",
      "BroadcastSignal1");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.CannotReceive",
      "BroadcastSignal2");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.CannotSend",
      "BroadcastSignal3");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.CannotBroadcast",
      "CannotBroadcast");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.CannotBroadcast2",
      "CannotBroadcast2");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  /* these two will go through: we use them as an indirect way to assert that
   * the recipient has not received anything earlier */
  m = dbus_message_new_signal ("/foo", "com.example.CannotUnicast",
      "CannotUnicast");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);
  m = dbus_message_new_signal ("/foo", "com.example.CannotUnicast2",
      "CannotUnicast2");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 12)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotSend",
      "BroadcastSignal1", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotReceive",
      "BroadcastSignal2", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotSend",
      "BroadcastSignal3", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotBroadcast",
      "CannotBroadcast", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotBroadcast2",
      "CannotBroadcast2", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotUnicast",
      "CannotUnicast", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotUnicast2",
      "CannotUnicast2", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);

  /* the intended recipient only received the ones that were on the interface
   * where broadcasts are allowed */

  while (g_queue_get_length (&f->received) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->received);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotUnicast",
      "CannotUnicast", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->received);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotUnicast2",
      "CannotUnicast2", "", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->received);
  g_assert (m == NULL);
}

static void
test_unicast_signal (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;

  if (f->address == NULL)
    return;

  become_monitor (f, NULL);

  m = dbus_message_new_signal ("/foo", "com.example.bar", "UnicastSignal1");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.bar", "UnicastSignal2");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.bar", "UnicastSignal3");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 3)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.bar", "UnicastSignal1", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.bar", "UnicastSignal2", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.bar", "UnicastSignal3", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);
}

static void
test_forbidden (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;

  if (f->address == NULL)
    return;

  if (!dbus_connection_add_filter (f->recipient, recipient_enqueue_filter,
        f, NULL))
    g_error ("OOM");
  f->recipient_enqueue_filter_added = TRUE;

  become_monitor (f, NULL);

  m = dbus_message_new_signal ("/foo", "com.example.CannotSend",
      "UnicastSignal1");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.CannotReceive",
      "UnicastSignal2");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.CannotSend",
      "UnicastSignal3");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.CannotUnicast",
      "CannotUnicast");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.CannotUnicast2",
      "CannotUnicast2");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  /* these two will go through: we use them as an indirect way to assert that
   * the recipient has not received anything earlier */
  m = dbus_message_new_signal ("/foo", "com.example.CannotBroadcast",
      "CannotBroadcast");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);
  m = dbus_message_new_signal ("/foo", "com.example.CannotBroadcast2",
      "CannotBroadcast2");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 12)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.CannotSend", "UnicastSignal1", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.CannotReceive", "UnicastSignal2", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.CannotSend", "UnicastSignal3", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotUnicast",
      "CannotUnicast", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotUnicast2",
      "CannotUnicast2", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotBroadcast",
      "CannotBroadcast", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotBroadcast2",
      "CannotBroadcast2", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);

  /* the intended recipient only received the ones that were on the interface
   * where unicasts are allowed */

  while (g_queue_get_length (&f->received) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->received);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotBroadcast",
      "CannotBroadcast", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->received);
  assert_signal (m, f->sender_name, "/foo", "com.example.CannotBroadcast2",
      "CannotBroadcast2", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->received);
  g_assert (m == NULL);
}

static void
test_method_call (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;

  if (f->address == NULL)
    return;

  become_monitor (f, NULL);

  /* regression test for
   * https://bugs.freedesktop.org/show_bug.cgi?id=90952 */
  m = dbus_message_new_method_call (f->recipient_name, "/foo",
      DBUS_INTERFACE_PEER, "Ping");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_method_call (m, f->sender_name, f->recipient_name, "/foo",
      DBUS_INTERFACE_PEER, "Ping", "");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_method_reply (m, f->recipient_name, f->sender_name, "");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);

  m = dbus_message_new_method_call (f->recipient_name, "/foo", "com.example.bar",
      "Call1");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_method_call (m, f->sender_name, f->recipient_name, "/foo",
      "com.example.bar", "Call1", "");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, f->recipient_name, f->sender_name,
      DBUS_ERROR_UNKNOWN_METHOD);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);
}

static void
test_forbidden_method_call (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;

  if (f->address == NULL)
    return;

  become_monitor (f, NULL);

  m = dbus_message_new_method_call (f->recipient_name, "/foo",
      "com.example.CannotSend", "Call1");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_method_call (m, f->sender_name, f->recipient_name, "/foo",
      "com.example.CannotSend", "Call1", "");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);

  m = dbus_message_new_method_call (f->recipient_name, "/foo",
      "com.example.CannotReceive", "Call2");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_method_call (m, f->sender_name, f->recipient_name, "/foo",
      "com.example.CannotReceive", "Call2", "");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);
}

static void
test_dbus_daemon (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;
  int res;
  size_t n_expected;

  if (f->address == NULL)
    return;

  become_monitor (f, NULL);

  res = dbus_bus_request_name (f->sender, "com.example.Sender",
      DBUS_NAME_FLAG_DO_NOT_QUEUE, &f->e);
  test_assert_no_error (&f->e);
  g_assert_cmpint (res, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

  res = dbus_bus_release_name (f->sender, "com.example.Sender", &f->e);
  test_assert_no_error (&f->e);
  g_assert_cmpint (res, ==, DBUS_RELEASE_NAME_REPLY_RELEASED);

  n_expected = 8;

  if (config_forbids_name_acquired_signal (context))
    n_expected += 1;

  while (g_queue_get_length (&f->monitored) < n_expected)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_method_call (m, f->sender_name, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      DBUS_INTERFACE_DBUS, "RequestName", "su");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,
      "NameOwnerChanged", "sss", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_name_acquired (m);

  if (config_forbids_name_acquired_signal (f->config))
    expect_name_acquired_error (&f->monitored, m);

  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_method_reply (m, DBUS_SERVICE_DBUS, f->sender_name, "u");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_method_call (m, f->sender_name, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      DBUS_INTERFACE_DBUS, "ReleaseName", "s");
  dbus_message_unref (m);

  /* FIXME: should we get this? */
  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,
      "NameLost", "s", f->sender_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,
      "NameOwnerChanged", "sss", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_method_reply (m, DBUS_SERVICE_DBUS, f->sender_name, "u");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);
}

static void
test_selective (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;

  if (f->address == NULL)
    return;

  /* Match rules added before becoming a monitor should be cleared:
   * if they weren't, this test would get Interesting twice, then Tedious,
   * and only see Fun after that. */
  dbus_bus_add_match (f->monitor,
      "eavesdrop='true',interface='com.example.Interesting'", &f->e);
  test_assert_no_error (&f->e);
  dbus_bus_add_match (f->monitor,
      "eavesdrop='true',interface='com.example.Tedious'", &f->e);
  test_assert_no_error (&f->e);

  become_monitor (f, NULL);

  m = dbus_message_new_signal ("/foo", "com.example.Interesting",
      "UnicastSignal1");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.Tedious",
      "UnicastSignal2");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  m = dbus_message_new_signal ("/foo", "com.example.Fun",
      "UnicastSignal3");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  /* We get the interesting signal and the fun signal, but not the tedious
   * signal. */

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.Interesting", "UnicastSignal1", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.Fun", "UnicastSignal3", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);
}

static void
test_well_known_destination (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;

  if (f->address == NULL)
    return;

  take_well_known_name (f, f->recipient, "com.example.Recipient");
  /* we don't expect_take_well_known_name here because the
   * monitor isn't up yet */

  become_monitor (f, NULL);

  /* The sender sends a message to itself. It will not be observed. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "Unobserved");
  if (!dbus_message_set_destination (m, f->sender_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  /* The sender sends a message to the recipient by well-known name.
   * It will be observed. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "Observed1");
  if (!dbus_message_set_destination (m, "com.example.Recipient"))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  /* The sender sends a message to the recipient by unique name.
   * It will still be observed. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "Observed2");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.bar",
      "Observed1", "", "com.example.Recipient");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.bar",
      "Observed2", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);
}

static void
test_unique_destination (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;
  Config config = {
    NULL,
    NULL, /* match rules */
    FALSE
  };
  const gchar *match_rules[2] = { NULL, NULL };
  gchar *rule;

  if (f->address == NULL)
    return;

  take_well_known_name (f, f->recipient, "com.example.Recipient");
  /* we don't expect_take_well_known_name here because the
   * monitor isn't up yet */

  rule = g_strdup_printf ("destination='%s'", f->recipient_name);
  /* free it later */
  g_test_queue_free (rule);
  match_rules[0] = rule;
  config.match_rules = match_rules;

  become_monitor (f, &config);

  /* The sender sends a message to itself. It will not be observed. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "Unobserved");
  if (!dbus_message_set_destination (m, f->sender_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  /* The sender sends a message to the recipient by well-known name.
   * It will be observed. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "Observed1");
  if (!dbus_message_set_destination (m, "com.example.Recipient"))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  /* The sender sends a message to the recipient by unique name.
   * It will still be observed. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "Observed2");
  if (!dbus_message_set_destination (m, f->recipient_name))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  while (g_queue_get_length (&f->monitored) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.bar",
      "Observed1", "", "com.example.Recipient");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo", "com.example.bar",
      "Observed2", "", f->recipient_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  g_assert (m == NULL);
}

#ifdef DBUS_UNIX
/* currently only used for the systemd activation test */
static void
expect_new_connection (Fixture *f)
{
  DBusMessage *m;
  size_t n_expected;

  n_expected = 4;

  if (config_forbids_name_acquired_signal (f->config))
    n_expected += 1;

  while (g_queue_get_length (&f->monitored) < n_expected)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_hello (m);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_hello_reply (m);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,
      "NameOwnerChanged", "sss", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_unique_name_acquired (m);

  if (config_forbids_name_acquired_signal (f->config))
    expect_name_acquired_error (&f->monitored, m);

  dbus_message_unref (m);
}

/* currently only used for the systemd activation test */
static void
expect_take_well_known_name (Fixture *f,
    DBusConnection *connection,
    const char *name)
{
  DBusMessage *m;
  const char *connection_name = dbus_bus_get_unique_name (connection);

  while (g_queue_get_length (&f->monitored) < 4)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_method_call (m, connection_name, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      DBUS_INTERFACE_DBUS, "RequestName", "su");
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,
      "NameOwnerChanged", "sss", NULL);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,
      "NameAcquired", "s", connection_name);
  dbus_message_unref (m);

  m = g_queue_pop_head (&f->monitored);
  assert_method_reply (m, DBUS_SERVICE_DBUS, connection_name, "u");
  dbus_message_unref (m);
}

static void
test_activation (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;

  if (f->address == NULL)
    return;

  become_monitor (f, NULL);

  /* The sender sends a message to an activatable service. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "UnicastSignal1");
  if (!dbus_message_set_destination (m, "com.example.SystemdActivatable1"))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  /* We observe the activation request, and the message that caused it,
   * before systemd has even joined the bus. */
  while (g_queue_get_length (&f->monitored) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      "org.freedesktop.systemd1.Activator", "ActivationRequest", "s",
      "org.freedesktop.systemd1");
  dbus_message_unref (m);
  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.bar", "UnicastSignal1", "",
      "com.example.SystemdActivatable1");
  dbus_message_unref (m);

  /* The fake systemd connects to the bus. */
  f->systemd = test_connect_to_bus (f->ctx, f->address);
  if (!dbus_connection_add_filter (f->systemd, systemd_filter, f, NULL))
    g_error ("OOM");
  f->systemd_name = dbus_bus_get_unique_name (f->systemd);

  expect_new_connection (f);
  take_well_known_name (f, f->systemd, "org.freedesktop.systemd1");
  expect_take_well_known_name (f, f->systemd, "org.freedesktop.systemd1");

  /* It gets its activation request. */
  while (f->systemd_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->systemd_message;
  f->systemd_message = NULL;
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      "org.freedesktop.systemd1.Activator", "ActivationRequest", "s",
      "org.freedesktop.systemd1");
  dbus_message_unref (m);

  /* systemd starts the activatable service. */
  f->activated = test_connect_to_bus (f->ctx, f->address);
  if (!dbus_connection_add_filter (f->activated, activated_filter,
        f, NULL))
    g_error ("OOM");
  f->activated_name = dbus_bus_get_unique_name (f->activated);

  expect_new_connection (f);
  take_well_known_name (f, f->activated, "com.example.SystemdActivatable1");
  expect_take_well_known_name (f, f->activated,
      "com.example.SystemdActivatable1");

  /* The message is delivered to the activatable service. */
  while (f->activated_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->activated_message;
  f->activated_message = NULL;
  assert_signal (m, f->sender_name, "/foo",
      "com.example.bar", "UnicastSignal1", "",
      "com.example.SystemdActivatable1");
  dbus_message_unref (m);

  /* The sender sends a message to a different activatable service. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "UnicastSignal2");
  if (!dbus_message_set_destination (m, "com.example.SystemdActivatable2"))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  /* This time systemd is already ready for it. */
  while (g_queue_get_length (&f->monitored) < 2 ||
      f->systemd_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->systemd_message;
  f->systemd_message = NULL;
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      "org.freedesktop.systemd1.Activator", "ActivationRequest", "s",
      "org.freedesktop.systemd1");
  dbus_message_unref (m);

  /* The monitor sees the activation request and the signal that
   * prompted it.*/
  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      "org.freedesktop.systemd1.Activator", "ActivationRequest", "s",
      "org.freedesktop.systemd1");
  dbus_message_unref (m);
  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.bar", "UnicastSignal2", "",
      "com.example.SystemdActivatable2");
  dbus_message_unref (m);

  /* The activatable service takes its name. Here I'm faking it by using
   * an existing connection. */
  take_well_known_name (f, f->activated, "com.example.SystemdActivatable2");

  /* The message is delivered to the activatable service.
   * Implementation detail: the monitor sees this happen before it even
   * sees that the name request happened, which is pretty odd. */
  while (f->activated_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->activated_message;
  f->activated_message = NULL;
  assert_signal (m, f->sender_name, "/foo",
      "com.example.bar", "UnicastSignal2", "",
      "com.example.SystemdActivatable2");
  dbus_message_unref (m);

  expect_take_well_known_name (f, f->activated,
      "com.example.SystemdActivatable2");

  /* A third activation. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "UnicastSignal3");
  if (!dbus_message_set_destination (m, "com.example.SystemdActivatable3"))
    g_error ("OOM");
  dbus_connection_send (f->sender, m, NULL);
  dbus_message_unref (m);

  /* Once again, we see the activation request and the reason. */
  while (g_queue_get_length (&f->monitored) < 2)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      "org.freedesktop.systemd1.Activator", "ActivationRequest", "s",
      "org.freedesktop.systemd1");
  dbus_message_unref (m);
  m = g_queue_pop_head (&f->monitored);
  assert_signal (m, f->sender_name, "/foo",
      "com.example.bar", "UnicastSignal3", "",
      "com.example.SystemdActivatable3");
  dbus_message_unref (m);

  /* systemd gets the request too. */
  while (f->systemd_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->systemd_message;
  f->systemd_message = NULL;
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      "org.freedesktop.systemd1.Activator", "ActivationRequest", "s",
      "org.freedesktop.systemd1");
  dbus_message_unref (m);

  /* This time activation fails */
  m = dbus_message_new_signal ("/org/freedesktop/systemd1",
      "org.freedesktop.systemd1.Activator", "ActivationFailure");

  do
    {
      const char *unit = "dbus-com.example.SystemdActivatable3.service";
      const char *error_name = "com.example.Nope";
      const char *error_message = "Computer says no";

      if (!dbus_message_append_args (m,
            DBUS_TYPE_STRING, &unit,
            DBUS_TYPE_STRING, &error_name,
            DBUS_TYPE_STRING, &error_message,
            DBUS_TYPE_INVALID))
        g_error ("OOM");
    }
  while (0);

  if (!dbus_message_set_destination (m, "org.freedesktop.DBus"))
    g_error ("OOM");
  dbus_connection_send (f->systemd, m, NULL);
  dbus_message_unref (m);

  /* The monitor sees activation fail */

  /* Once again, we see the activation request and the reason. */
  while (g_queue_get_length (&f->monitored) < 1)
    test_main_context_iterate (f->ctx, TRUE);

  m = g_queue_pop_head (&f->monitored);
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->sender_name,
      "com.example.Nope");
  dbus_message_unref (m);
}
#endif /* DBUS_UNIX */

static void
teardown (Fixture *f,
    gconstpointer context G_GNUC_UNUSED)
{
  GList *link;

  dbus_error_free (&f->e);
  g_clear_error (&f->ge);

  if (f->monitor != NULL)
    {
      dbus_connection_remove_filter (f->monitor, monitor_filter, f);
      test_connection_shutdown (f->ctx, f->monitor);
      dbus_connection_close (f->monitor);
      dbus_connection_unref (f->monitor);
      f->monitor = NULL;
    }

  if (f->sender != NULL)
    {
      test_connection_shutdown (f->ctx, f->sender);
      dbus_connection_close (f->sender);
      dbus_connection_unref (f->sender);
      f->sender = NULL;
    }

  if (f->recipient != NULL)
    {
      dbus_connection_remove_filter (f->recipient, recipient_check_filter, f);
      if (f->recipient_enqueue_filter_added)
        dbus_connection_remove_filter (f->recipient, recipient_enqueue_filter,
            f);

      test_connection_shutdown (f->ctx, f->recipient);
      dbus_connection_close (f->recipient);
      dbus_connection_unref (f->recipient);
      f->recipient = NULL;
    }

  if (f->systemd != NULL)
    {
      dbus_connection_remove_filter (f->systemd, systemd_filter, f);
      test_connection_shutdown (f->ctx, f->systemd);
      dbus_connection_close (f->systemd);
      dbus_connection_unref (f->systemd);
      f->systemd = NULL;
    }

  if (f->activated != NULL)
    {
      dbus_connection_remove_filter (f->activated, activated_filter, f);
      test_connection_shutdown (f->ctx, f->activated);
      dbus_connection_close (f->activated);
      dbus_connection_unref (f->activated);
      f->activated = NULL;
    }

  if (f->daemon_pid != 0)
    {
      test_kill_pid (f->daemon_pid);
      g_spawn_close_pid (f->daemon_pid);
      f->daemon_pid = 0;
    }

  test_main_context_unref (f->ctx);

  for (link = f->monitored.head; link != NULL; link = link->next)
    dbus_message_unref (link->data);

  g_queue_clear (&f->monitored);

  for (link = f->received.head; link != NULL; link = link->next)
    dbus_message_unref (link->data);

  g_queue_clear (&f->received);

  g_free (f->address);
}

int
main (int argc,
    char **argv)
{
  int ret;

  test_init (&argc, &argv);

  g_test_add ("/monitor/invalid", Fixture, NULL,
      setup, test_invalid, teardown);
  g_test_add ("/monitor/become", Fixture, &side_effects_config,
      setup, test_become_monitor, teardown);
  g_test_add ("/monitor/broadcast", Fixture, NULL,
      setup, test_broadcast, teardown);
  g_test_add ("/monitor/forbidden-broadcast", Fixture, &forbidding_config,
      setup, test_forbidden_broadcast, teardown);
  g_test_add ("/monitor/unicast-signal", Fixture, NULL,
      setup, test_unicast_signal, teardown);
  g_test_add ("/monitor/forbidden", Fixture, &forbidding_config,
      setup, test_forbidden, teardown);
  g_test_add ("/monitor/method-call", Fixture, NULL,
      setup, test_method_call, teardown);
  g_test_add ("/monitor/forbidden-method", Fixture, &forbidding_config,
      setup, test_forbidden_method_call, teardown);
  g_test_add ("/monitor/forbidden-reply", Fixture, &forbidding_config,
      setup, test_dbus_daemon, teardown);
  g_test_add ("/monitor/dbus-daemon", Fixture, NULL,
      setup, test_dbus_daemon, teardown);
  g_test_add ("/monitor/selective", Fixture, &selective_config,
      setup, test_selective, teardown);
  g_test_add ("/monitor/well-known-destination",
      Fixture, &well_known_destination_config,
      setup, test_well_known_destination, teardown);
  g_test_add ("/monitor/unique-destination",
      Fixture, NULL,
      setup, test_unique_destination, teardown);
  g_test_add ("/monitor/wildcard", Fixture, &wildcard_config,
      setup, test_unicast_signal, teardown);
  g_test_add ("/monitor/no-rule", Fixture, &no_rules_config,
      setup, test_unicast_signal, teardown);
  g_test_add ("/monitor/eavesdrop", Fixture, &eavesdrop_config,
      setup, test_unicast_signal, teardown);
  g_test_add ("/monitor/no-eavesdrop", Fixture, &no_eavesdrop_config,
      setup, test_unicast_signal, teardown);

#ifdef DBUS_UNIX
  /* this relies on the systemd activation code path */
  g_test_add ("/monitor/activation", Fixture, &fake_systemd_config,
      setup, test_activation, teardown);
#endif

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
