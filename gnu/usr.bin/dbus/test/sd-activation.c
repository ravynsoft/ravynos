/* Unit tests for systemd activation, with or without AppArmor.
 *
 * We compile this source file twice: once with AppArmor support (if available)
 * and once without.
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

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include <glib/gstdio.h>

#if defined(HAVE_APPARMOR_2_10) && defined(DBUS_TEST_APPARMOR_ACTIVATION)
#include <sys/apparmor.h>
#endif

#include "test-utils-glib.h"

typedef struct {
    TestMainContext *ctx;
    DBusError e;
    GError *ge;

    gchar *address;
    GPid daemon_pid;

    DBusConnection *caller;
    const char *caller_name;
    DBusMessage *caller_message;
    dbus_bool_t caller_filter_added;

    DBusConnection *systemd;
    const char *systemd_name;
    DBusMessage *systemd_message;
    dbus_bool_t systemd_filter_added;

    DBusConnection *activated;
    const char *activated_name;
    DBusMessage *activated_message;
    dbus_bool_t activated_filter_added;

    gchar *transient_service_file;
    gchar *tmp_runtime_dir;
} Fixture;

typedef enum
{
  FLAG_EARLY_TRANSIENT_SERVICE = (1 << 0),
  FLAG_NONE = 0
} Flags;

typedef struct
{
  const gchar *bus_name;
  Flags flags;
} Config;

/* this is a macro so it gets the right line number */
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

  g_test_message("sender %s iface %s member %s",
                 dbus_message_get_sender (message),
                 dbus_message_get_interface (message),
                 dbus_message_get_member (message));


  g_assert (f->systemd_message == NULL);
  f->systemd_message = dbus_message_ref (message);

  if (dbus_message_is_method_call (message, "org.freedesktop.systemd1.Manager",
                                   "SetEnvironment"))
    {
      g_assert (dbus_message_get_no_reply (message));
      g_test_message("got call");
      return DBUS_HANDLER_RESULT_HANDLED;
    }

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

  /* Test code is expected to reply to method calls itself */
  if (dbus_message_get_type (message) == DBUS_MESSAGE_TYPE_METHOD_CALL)
    return DBUS_HANDLER_RESULT_HANDLED;

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult
caller_filter (DBusConnection *connection,
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

  g_assert (f->caller_message == NULL);
  f->caller_message = dbus_message_ref (message);

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
fixture_create_transient_service (Fixture *f,
                                  const gchar *name)
{
  gchar *service;
  gchar *content;
  gboolean ok;

  service = g_strdup_printf ("%s.service", name);
  f->transient_service_file = g_build_filename (f->tmp_runtime_dir, "dbus-1",
      "services", service, NULL);
  g_free (service);

  content = g_strdup_printf (
      "[D-BUS Service]\n"
      "Name=%s\n"
      "Exec=/bin/false %s\n"
      "SystemdService=dbus-%s.service\n", name, name, name);
  ok = g_file_set_contents (f->transient_service_file, content, -1, &f->ge);
  g_assert_no_error (f->ge);
  g_assert (ok);
  g_free (content);
}

static void
setup (Fixture *f,
    gconstpointer context)
{
  const Config *config = context;
#if defined(DBUS_TEST_APPARMOR_ACTIVATION) && defined(HAVE_APPARMOR_2_10)
  aa_features *features;
#endif

  f->ge = NULL;
  dbus_error_init (&f->e);

  f->tmp_runtime_dir = g_dir_make_tmp ("dbus-daemon-test.XXXXXX", &f->ge);
  g_assert_no_error (f->ge);

  if (config != NULL && (config->flags & FLAG_EARLY_TRANSIENT_SERVICE) != 0)
    {
      gchar *dbus1 = g_build_filename (f->tmp_runtime_dir, "dbus-1", NULL);
      gchar *services = g_build_filename (dbus1, "services", NULL);

      /* We just created it so the directories shouldn't exist yet */
      test_mkdir (dbus1, 0700);
      test_mkdir (services, 0700);
      fixture_create_transient_service (f, config->bus_name);
      g_free (dbus1);
      g_free (services);
    }

#if defined(DBUS_TEST_APPARMOR_ACTIVATION) && !defined(HAVE_APPARMOR_2_10)

  g_test_skip ("AppArmor support not compiled or AppArmor 2.10 unavailable");
  return;

#else

#if defined(DBUS_TEST_APPARMOR_ACTIVATION)
  if (!aa_is_enabled ())
    {
      g_test_message ("aa_is_enabled() -> %s", g_strerror (errno));
      g_test_skip ("AppArmor not enabled");
      return;
    }

  if (aa_features_new_from_kernel (&features) != 0)
    {
      g_test_skip ("Unable to check AppArmor features");
      return;
    }

  if (!aa_features_supports (features, "dbus/mask/send") ||
      !aa_features_supports (features, "dbus/mask/receive"))
    {
      g_test_skip ("D-Bus send/receive mediation unavailable");
      aa_features_unref (features);
      return;
    }

  aa_features_unref (features);
#endif

  f->ctx = test_main_context_get ();

  f->address = test_get_dbus_daemon (
      "valid-config-files/systemd-activation.conf",
      TEST_USER_ME, f->tmp_runtime_dir, &f->daemon_pid);

  if (f->address == NULL)
    return;

#if defined(DBUS_TEST_APPARMOR_ACTIVATION)
  /*
   * Make use of the fact that the LSM security label (and other process
   * properties) that are used for access-control are whatever was current
   * at the time the connection was opened.
   *
   * 42 is arbitrary. In a real use of AppArmor it would be a securely-random
   * value, to prevent less-privileged code (that does not know the magic
   * value) from changing back.
   */
  if (aa_change_hat ("caller", 42) != 0)
    g_error ("Unable to change profile to ...//^caller: %s",
             g_strerror (errno));
#endif

  f->caller = test_connect_to_bus (f->ctx, f->address);
  f->caller_name = dbus_bus_get_unique_name (f->caller);

#if defined(DBUS_TEST_APPARMOR_ACTIVATION)
  if (aa_change_hat (NULL, 42) != 0)
    g_error ("Unable to change back to initial profile: %s",
             g_strerror (errno));
#endif

#endif
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
test_activation (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;

  if (f->address == NULL)
    return;

  /* The sender sends a message to an activatable service. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "UnicastSignal1");
  if (!dbus_message_set_destination (m, "com.example.SystemdActivatable1"))
    g_error ("OOM");
  dbus_connection_send (f->caller, m, NULL);
  dbus_message_unref (m);

  /* The fake systemd connects to the bus. */
  f->systemd = test_connect_to_bus (f->ctx, f->address);
  if (!dbus_connection_add_filter (f->systemd, systemd_filter, f, NULL))
    g_error ("OOM");
  f->systemd_filter_added = TRUE;
  f->systemd_name = dbus_bus_get_unique_name (f->systemd);
  take_well_known_name (f, f->systemd, "org.freedesktop.systemd1");

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
  f->activated_filter_added = TRUE;
  f->activated_name = dbus_bus_get_unique_name (f->activated);
  take_well_known_name (f, f->activated, "com.example.SystemdActivatable1");

  /* The message is delivered to the activatable service. */
  while (f->activated_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->activated_message;
  f->activated_message = NULL;
  assert_signal (m, f->caller_name, "/foo",
      "com.example.bar", "UnicastSignal1", "",
      "com.example.SystemdActivatable1");
  dbus_message_unref (m);

  /* The sender sends a message to a different activatable service. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "UnicastSignal2");
  if (!dbus_message_set_destination (m, "com.example.SystemdActivatable2"))
    g_error ("OOM");
  dbus_connection_send (f->caller, m, NULL);
  dbus_message_unref (m);

  /* This time systemd is already ready for it. */
  while (f->systemd_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->systemd_message;
  f->systemd_message = NULL;
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      "org.freedesktop.systemd1.Activator", "ActivationRequest", "s",
      "org.freedesktop.systemd1");
  dbus_message_unref (m);

  /* A malicious process tries to disrupt the activation.
   * In a more realistic scenario this would be another parallel
   * connection. */
  m = dbus_message_new_signal ("/org/freedesktop/systemd1",
      "org.freedesktop.systemd1.Activator", "ActivationFailure");
  if (!dbus_message_set_destination (m, "org.freedesktop.DBus"))
    g_error ("OOM");

  do
    {
      const char *unit = "dbus-com.example.SystemdActivatable2.service";
      const char *error_name = "com.example.Malice";
      const char *error_message = "I'm on yr bus, making yr activations fail";

      if (!dbus_message_append_args (m,
            DBUS_TYPE_STRING, &unit,
            DBUS_TYPE_STRING, &error_name,
            DBUS_TYPE_STRING, &error_message,
            DBUS_TYPE_INVALID))
        g_error ("OOM");
    }
  while (0);

  dbus_connection_send (f->caller, m, NULL);
  dbus_message_unref (m);

  /* This is just to make sure that the malicious message has arrived and
   * been processed by the dbus-daemon, i.e. @caller won the race
   * with @activated. */
  take_well_known_name (f, f->caller, "com.example.Sync");

  /* The activatable service takes its name. Here I'm faking it by using
   * an existing connection; in real life it would be yet another
   * connection. */
  take_well_known_name (f, f->activated, "com.example.SystemdActivatable2");

  /* The message is delivered to the activatable service. */
  while (f->activated_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->activated_message;
  f->activated_message = NULL;
  assert_signal (m, f->caller_name, "/foo",
      "com.example.bar", "UnicastSignal2", "",
      "com.example.SystemdActivatable2");
  dbus_message_unref (m);

  /* A third activation. */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "UnicastSignal3");
  if (!dbus_message_set_destination (m, "com.example.SystemdActivatable3"))
    g_error ("OOM");
  dbus_connection_send (f->caller, m, NULL);
  dbus_message_unref (m);

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

  /* A fourth activation: for name from send_destination_prefix namespace */
  m = dbus_message_new_signal ("/foo", "com.example.bar", "UnicastSignal4");
  if (!dbus_message_set_destination (m, "com.example.SendPrefixDenied.SendPrefixAllowed.internal"))
    g_error ("OOM");
  dbus_connection_send (f->caller, m, NULL);
  dbus_message_unref (m);

  /* systemd is already ready for it. */
  while (f->systemd_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->systemd_message;
  f->systemd_message = NULL;
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      "org.freedesktop.systemd1.Activator", "ActivationRequest", "s",
      "org.freedesktop.systemd1");

  /* Check ActivationRequest for the required name. */
  /* If it is correct, then it passed through policy checking, and the test is over. */
  do
    {
      const char *name;
      DBusError error;

      dbus_error_init (&error);
      dbus_message_get_args (m, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID);
      test_assert_no_error (&error);
      g_assert_cmpstr (name, ==, "dbus-com.example.SendPrefixDenied.SendPrefixAllowed.internal.service");
    } while (0);
  dbus_message_unref (m);
}

static void
test_uae (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter, arr_iter, entry_iter;
  const char *s;

  if (f->address == NULL)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      DBUS_INTERFACE_DBUS, "UpdateActivationEnvironment");

  if (m == NULL)
    g_error ("OOM");

  dbus_message_iter_init_append (m, &args_iter);

  /* Append an empty a{ss} (string => string dictionary). */
  if (!dbus_message_iter_open_container (&args_iter, DBUS_TYPE_ARRAY,
        "{ss}", &arr_iter) ||
      !dbus_message_iter_close_container (&args_iter, &arr_iter))
    g_error ("OOM");

  reply = test_main_context_call_and_wait (f->ctx, f->caller, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  assert_method_reply (reply, DBUS_SERVICE_DBUS, f->caller_name, "");

  dbus_clear_message (&reply);
  dbus_clear_message (&m);

  /* The fake systemd connects to the bus. */
  f->systemd = test_connect_to_bus (f->ctx, f->address);
  if (!dbus_connection_add_filter (f->systemd, systemd_filter, f, NULL))
    g_error ("OOM");
  f->systemd_name = dbus_bus_get_unique_name (f->systemd);
  take_well_known_name (f, f->systemd, "org.freedesktop.systemd1");

  /* It gets the SetEnvironment */
  while (f->systemd_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->systemd_message;
  f->systemd_message = NULL;

  /* With activation, the destination is the well-known name */
  assert_method_call (m, DBUS_SERVICE_DBUS, "org.freedesktop.systemd1",
      "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager",
      "SetEnvironment", "as");

  dbus_message_iter_init (m, &args_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&args_iter), ==,
      DBUS_TYPE_ARRAY);
  g_assert_cmpuint (dbus_message_iter_get_element_type (&args_iter), ==,
      DBUS_TYPE_STRING);
  dbus_message_iter_recurse (&args_iter, &arr_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&arr_iter), ==,
      DBUS_TYPE_INVALID);
  dbus_message_iter_next (&args_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&args_iter), ==,
      DBUS_TYPE_INVALID);
  dbus_clear_message (&m);

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      DBUS_INTERFACE_DBUS, "UpdateActivationEnvironment");

  if (m == NULL)
    g_error ("OOM");

  dbus_message_iter_init_append (m, &args_iter);


  {
    const char *k1 = "Key1", *v1 = "Value1",
               *k2 = "Key2", *v2 = "Value2";

    /* Append a filled a{ss} (string => string dictionary). */
    if (!dbus_message_iter_open_container (&args_iter, DBUS_TYPE_ARRAY,
          "{ss}", &arr_iter) ||
        !dbus_message_iter_open_container (&arr_iter, DBUS_TYPE_DICT_ENTRY,
          NULL, &entry_iter) ||
        !dbus_message_iter_append_basic (&entry_iter, DBUS_TYPE_STRING,
          &k1) ||
        !dbus_message_iter_append_basic (&entry_iter, DBUS_TYPE_STRING,
          &v1) ||
        !dbus_message_iter_close_container (&arr_iter, &entry_iter) ||
        !dbus_message_iter_open_container (&arr_iter, DBUS_TYPE_DICT_ENTRY,
          NULL, &entry_iter) ||
        !dbus_message_iter_append_basic (&entry_iter, DBUS_TYPE_STRING,
          &k2) ||
        !dbus_message_iter_append_basic (&entry_iter, DBUS_TYPE_STRING,
          &v2) ||
        !dbus_message_iter_close_container (&arr_iter, &entry_iter) ||
        !dbus_message_iter_close_container (&args_iter, &arr_iter))
      g_error ("OOM");
  }

  reply = test_main_context_call_and_wait (f->ctx, f->caller, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  assert_method_reply (reply, DBUS_SERVICE_DBUS, f->caller_name, "");

  dbus_clear_message (&reply);
  dbus_clear_message (&m);

  while (f->systemd_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->systemd_message;
  f->systemd_message = NULL;

  /* Without activation, the destination is the unique name */
  assert_method_call (m, DBUS_SERVICE_DBUS, f->systemd_name,
      "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager",
      "SetEnvironment", "as");

  dbus_message_iter_init (m, &args_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&args_iter), ==,
      DBUS_TYPE_ARRAY);
  g_assert_cmpuint (dbus_message_iter_get_element_type (&args_iter), ==,
      DBUS_TYPE_STRING);
  dbus_message_iter_recurse (&args_iter, &arr_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&arr_iter), ==,
      DBUS_TYPE_STRING);
  dbus_message_iter_get_basic (&arr_iter, &s);
  g_assert_cmpstr (s, ==, "Key1=Value1");
  dbus_message_iter_next (&arr_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&arr_iter), ==,
      DBUS_TYPE_STRING);
  dbus_message_iter_get_basic (&arr_iter, &s);
  g_assert_cmpstr (s, ==, "Key2=Value2");
  dbus_message_iter_next (&arr_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&arr_iter), ==,
      DBUS_TYPE_INVALID);
  dbus_message_iter_next (&args_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&args_iter), ==,
      DBUS_TYPE_INVALID);
  dbus_clear_message (&m);
}

static void
test_deny_send (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;
  const Config *config = context;

  g_assert (config != NULL);
  g_assert (config->bus_name != NULL);

  if (f->address == NULL)
    return;

  if (!dbus_connection_add_filter (f->caller, caller_filter, f, NULL))
    g_error ("OOM");

  f->caller_filter_added = TRUE;

  /* The sender sends a message to an activatable service. */
  m = dbus_message_new_method_call (config->bus_name, "/foo",
      "com.example.bar", "Call");
  if (m == NULL)
    g_error ("OOM");

  dbus_connection_send (f->caller, m, NULL);
  dbus_message_unref (m);

  /*
   * Even before the fake systemd connects to the bus, we get an error
   * back: activation is not allowed.
   *
   * In the normal case, this is because the XML policy does not allow
   * anyone to send messages to the bus name com.example.SendDenied.
   *
   * In the AppArmor case, this is because the AppArmor policy does not allow
   * this process to send messages to the bus name
   * com.example.SendDeniedByAppArmorName, or to the label
   * @DBUS_TEST_EXEC@/com.example.SendDeniedByAppArmorLabel that we assume the
   * service com.example.SendDeniedByAppArmorLabel will receive after systemd
   * runs it.
   */

  while (f->caller_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->caller_message;
  f->caller_message = NULL;
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->caller_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);
}

static void
test_deny_receive (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m;
  const Config *config = context;

  g_assert (config != NULL);
  g_assert (config->bus_name != NULL);

  if (f->address == NULL)
    return;

  if (!dbus_connection_add_filter (f->caller, caller_filter, f, NULL))
    g_error ("OOM");

  f->caller_filter_added = TRUE;

  /* The sender sends a message to an activatable service.
   * We set the interface name equal to the bus name to make it
   * easier to write the necessary policy rules. */
  m = dbus_message_new_method_call (config->bus_name, "/foo",
                                    config->bus_name, "Call");
  if (m == NULL)
    g_error ("OOM");

  dbus_connection_send (f->caller, m, NULL);
  dbus_message_unref (m);

  /* The fake systemd connects to the bus. */
  f->systemd = test_connect_to_bus (f->ctx, f->address);
  if (!dbus_connection_add_filter (f->systemd, systemd_filter, f, NULL))
    g_error ("OOM");
  f->systemd_filter_added = TRUE;
  f->systemd_name = dbus_bus_get_unique_name (f->systemd);
  take_well_known_name (f, f->systemd, "org.freedesktop.systemd1");

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

#if defined(DBUS_TEST_APPARMOR_ACTIVATION) && defined(HAVE_APPARMOR_2_10)
  /* The use of 42 here is arbitrary, see setup(). */
  if (aa_change_hat (config->bus_name, 42) != 0)
    g_error ("Unable to change profile to ...//^%s: %s",
             config->bus_name, g_strerror (errno));
#endif

  f->activated = test_connect_to_bus (f->ctx, f->address);
  if (!dbus_connection_add_filter (f->activated, activated_filter,
        f, NULL))
    g_error ("OOM");
  f->activated_filter_added = TRUE;
  f->activated_name = dbus_bus_get_unique_name (f->activated);
  take_well_known_name (f, f->activated, config->bus_name);

#if defined(DBUS_TEST_APPARMOR_ACTIVATION) && defined(HAVE_APPARMOR_2_10)
  if (aa_change_hat (NULL, 42) != 0)
    g_error ("Unable to change back to initial profile: %s",
             g_strerror (errno));
#endif

  /*
   * We re-do the message matching, and now the message is
   * forbidden by the receive policy.
   *
   * In the normal case, this is because the XML policy does not allow
   * receiving any message with interface com.example.ReceiveDenied.
   * We can't use the recipient's bus name here because the XML policy
   * has no syntax for preventing the owner of a name from receiving
   * messages - that would be pointless, because the sender could just
   * open another connection and not own the same name on that connection.
   *
   * In the AppArmor case, this is because the AppArmor policy does not allow
   * receiving messages with interface com.example.ReceiveDeniedByAppArmor
   * from a peer with the same label we have. Again, we can't use the
   * recipient's bus name because there is no syntax for this.
   */
  while (f->caller_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->caller_message;
  f->caller_message = NULL;
  assert_error_reply (m, DBUS_SERVICE_DBUS, f->caller_name,
      DBUS_ERROR_ACCESS_DENIED);
  dbus_message_unref (m);

  /* The activated service never even saw it. */
  g_assert (f->activated_message == NULL);
}

/*
 * Test that we can set up transient services.
 *
 * If (flags & FLAG_EARLY_TRANSIENT_SERVICE), we assert that a service that
 * was deployed before starting systemd (in setup()) is available.
 *
 * Otherwise, we assert that a service that is deployed while dbus-daemon
 * is already running becomes available after reloading the dbus-daemon
 * configuration.
 */
static void
test_transient_services (Fixture *f,
    gconstpointer context)
{
  const Config *config = context;
  DBusMessage *m = NULL;
  DBusMessage *send_reply = NULL;
  DBusMessage *reply = NULL;
  DBusPendingCall *pc;

  g_assert (config != NULL);
  g_assert (config->bus_name != NULL);

  if (f->address == NULL)
    return;

  /* Connect the fake systemd to the bus. */
  f->systemd = test_connect_to_bus (f->ctx, f->address);
  if (!dbus_connection_add_filter (f->systemd, systemd_filter, f, NULL))
    g_error ("OOM");
  f->systemd_filter_added = TRUE;
  f->systemd_name = dbus_bus_get_unique_name (f->systemd);
  take_well_known_name (f, f->systemd, "org.freedesktop.systemd1");

  if ((config->flags & FLAG_EARLY_TRANSIENT_SERVICE) == 0)
    {
      /* Try to activate a service that isn't there. */
      m = dbus_message_new_method_call (config->bus_name,
                                        "/foo", "com.example.bar", "Activate");

      if (m == NULL)
        test_oom ();

      /* It fails. */
      reply = test_main_context_call_and_wait (f->ctx, f->caller, m,
          DBUS_TIMEOUT_USE_DEFAULT);
      assert_error_reply (reply, DBUS_SERVICE_DBUS, f->caller_name,
          DBUS_ERROR_SERVICE_UNKNOWN);

      dbus_clear_message (&reply);
      dbus_clear_message (&m);

      /* Now generate a transient D-Bus service file for it. The directory
       * should have been created during dbus-daemon startup, so we don't have to
       * recreate it. */
      fixture_create_transient_service (f, config->bus_name);

      /* To guarantee that the transient service has been picked up, we have
       * to reload. */
      m = dbus_message_new_method_call (DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                        DBUS_INTERFACE_DBUS, "ReloadConfig");

      if (m == NULL)
        test_oom ();

      reply = test_main_context_call_and_wait (f->ctx, f->caller, m,
          DBUS_TIMEOUT_USE_DEFAULT);
      assert_method_reply (reply, DBUS_SERVICE_DBUS, f->caller_name, "");

      dbus_clear_message (&reply);
      dbus_clear_message (&m);
    }

  /* The service is present now. */
  m = dbus_message_new_method_call (config->bus_name,
                                    "/foo", "com.example.bar", "Activate");

  if (m == NULL ||
      !dbus_connection_send_with_reply (f->caller, m, &pc,
        DBUS_TIMEOUT_USE_DEFAULT) || pc == NULL)
    g_error ("OOM");

  dbus_clear_message (&m);

  if (dbus_pending_call_get_completed (pc))
    test_pending_call_store_reply (pc, &reply);
  else if (!dbus_pending_call_set_notify (pc, test_pending_call_store_reply,
        &reply, NULL))
    g_error ("OOM");

  dbus_clear_pending_call (&pc);

  /* The mock systemd is told to start the service. */
  while (f->systemd_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->systemd_message;
  f->systemd_message = NULL;
  assert_signal (m, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      "org.freedesktop.systemd1.Activator", "ActivationRequest", "s",
      "org.freedesktop.systemd1");
  dbus_clear_message (&m);

  /* The activatable service connects and gets its name. */
  f->activated = test_connect_to_bus (f->ctx, f->address);
  if (!dbus_connection_add_filter (f->activated, activated_filter,
        f, NULL))
    g_error ("OOM");
  f->activated_filter_added = TRUE;
  f->activated_name = dbus_bus_get_unique_name (f->activated);
  take_well_known_name (f, f->activated, config->bus_name);

  /* The message is delivered to the activatable service. */
  while (f->activated_message == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  m = f->activated_message;
  f->activated_message = NULL;
  assert_method_call (m, f->caller_name, config->bus_name, "/foo",
      "com.example.bar", "Activate", "");

  /* The activatable service sends back a reply. */
  send_reply = dbus_message_new_method_return (m);

  if (send_reply == NULL ||
      !dbus_connection_send (f->activated, send_reply, NULL))
    g_error ("OOM");

  dbus_clear_message (&send_reply);
  dbus_clear_message (&m);

  /* The caller receives the reply. */
  while (reply == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  assert_method_reply (reply, f->activated_name, f->caller_name, "");
  dbus_clear_message (&reply);
}

static void
teardown (Fixture *f,
    gconstpointer context G_GNUC_UNUSED)
{
  dbus_error_free (&f->e);
  g_clear_error (&f->ge);

  if (f->caller != NULL)
    {
      if (f->caller_filter_added)
        dbus_connection_remove_filter (f->caller, caller_filter, f);

      test_connection_shutdown (f->ctx, f->caller);
      dbus_connection_close (f->caller);
      dbus_connection_unref (f->caller);
      f->caller = NULL;
    }

  if (f->systemd != NULL)
    {
      if (f->systemd_filter_added)
        dbus_connection_remove_filter (f->systemd, systemd_filter, f);

      test_connection_shutdown (f->ctx, f->systemd);
      dbus_connection_close (f->systemd);
      dbus_connection_unref (f->systemd);
      f->systemd = NULL;
    }

  if (f->activated != NULL)
    {
      if (f->activated_filter_added)
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
    }

  if (f->ctx != NULL)
    test_main_context_unref (f->ctx);

  g_free (f->address);

  if (f->transient_service_file != NULL)
    {
      test_remove_if_exists (f->transient_service_file);
      g_free (f->transient_service_file);
    }

  if (f->tmp_runtime_dir != NULL)
    {
      gchar *dbus1 = g_build_filename (f->tmp_runtime_dir, "dbus-1", NULL);
      gchar *services = g_build_filename (dbus1, "services", NULL);

      test_rmdir_if_exists (services);
      test_rmdir_if_exists (dbus1);
      test_rmdir_if_exists (f->tmp_runtime_dir);

      g_free (f->tmp_runtime_dir);
      g_free (dbus1);
      g_free (services);
    }
}

static const Config deny_send_tests[] =
{
#if defined(DBUS_TEST_APPARMOR_ACTIVATION)
    { "com.example.SendDeniedByAppArmorLabel" },
    { "com.example.SendDeniedByNonexistentAppArmorLabel" },
    { "com.example.SendDeniedByAppArmorName" },
#endif
    { "com.example.SendDenied" },
    { "com.example.SendPrefixDenied" },
    { "com.example.SendPrefixDenied.internal" }
};

static const Config deny_receive_tests[] =
{
#if defined(DBUS_TEST_APPARMOR_ACTIVATION)
    { "com.example.ReceiveDeniedByAppArmorLabel" },
#endif
    { "com.example.ReceiveDenied" }
};

static const Config transient_service_later =
{
  "com.example.TransientActivatable1",
  FLAG_NONE
};

static const Config transient_service_in_advance =
{
  "com.example.TransientActivatable1",
  FLAG_EARLY_TRANSIENT_SERVICE
};

int
main (int argc,
    char **argv)
{
  gsize i;
  int ret;

  test_init (&argc, &argv);

  g_test_add ("/sd-activation/activation", Fixture, NULL,
      setup, test_activation, teardown);
  g_test_add ("/sd-activation/uae", Fixture, NULL,
      setup, test_uae, teardown);

  for (i = 0; i < G_N_ELEMENTS (deny_send_tests); i++)
    {
      gchar *name = g_strdup_printf ("/sd-activation/deny-send/%s",
                                     deny_send_tests[i].bus_name);

      g_test_add (name, Fixture, &deny_send_tests[i],
                  setup, test_deny_send, teardown);
      g_free (name);
    }

  for (i = 0; i < G_N_ELEMENTS (deny_receive_tests); i++)
    {
      gchar *name = g_strdup_printf ("/sd-activation/deny-receive/%s",
                                     deny_receive_tests[i].bus_name);

      g_test_add (name, Fixture, &deny_receive_tests[i],
                  setup, test_deny_receive, teardown);
      g_free (name);
    }

  g_test_add ("/sd-activation/transient-services/later", Fixture,
      &transient_service_later, setup, test_transient_services, teardown);
  g_test_add ("/sd-activation/transient-services/in-advance", Fixture,
      &transient_service_in_advance, setup, test_transient_services, teardown);

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
