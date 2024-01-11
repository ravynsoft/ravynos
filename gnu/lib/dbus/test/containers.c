/* Integration tests for restricted sockets for containers
 *
 * Copyright © 2017-2018 Collabora Ltd.
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

#include <dbus/dbus.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib/gstdio.h>

#if defined(DBUS_ENABLE_CONTAINERS) && defined(HAVE_GIO_UNIX)

#define HAVE_CONTAINERS_TEST

#include <gio/gunixfdlist.h>
#include <gio/gunixsocketaddress.h>

#include "dbus/dbus-sysdeps-unix.h"

#endif

#include "test-utils-glib.h"

#define DBUS_INTERFACE_CONTAINERS1    "org.freedesktop.DBus.Containers1"

typedef struct {
    TestMainContext *ctx;
    gboolean skip;
    gchar *bus_address;
    GPid daemon_pid;
    GError *error;

    GDBusProxy *proxy;

    gchar *instance_path;
    gchar *socket_path;
    gchar *socket_dbus_address;
    GDBusConnection *unconfined_conn;
    gchar *unconfined_unique_name;
    GDBusConnection *confined_conn;

    GDBusConnection *observer_conn;
    GDBusProxy *observer_proxy;
    GHashTable *containers_removed;
    guint removed_sub;
    DBusConnection *libdbus_observer;
    DBusMessage *latest_shout;
} Fixture;

typedef struct
{
  const gchar *config_file;
  enum
    {
      STOP_SERVER_EXPLICITLY,
      STOP_SERVER_DISCONNECT_FIRST,
      STOP_SERVER_NEVER_CONNECTED,
      STOP_SERVER_FORCE,
      STOP_SERVER_WITH_MANAGER
    }
  stop_server;
} Config;

static const Config default_config =
{
  NULL,
  0 /* not used, the stop-server test always uses non-default config */
};

#ifdef DBUS_ENABLE_CONTAINERS
/* A GDBusNameVanishedCallback that sets a boolean flag. */
static void
name_gone_set_boolean_cb (GDBusConnection *conn,
                          const gchar *name,
                          gpointer user_data)
{
  gboolean *gone_p = user_data;

  g_assert_nonnull (gone_p);
  g_assert_false (*gone_p);
  *gone_p = TRUE;
}
#endif

#ifdef HAVE_CONTAINERS_TEST
static void
iterate_both_main_loops (TestMainContext *ctx)
{
  /* TODO: Gluing these two main loops together so they can block would
   * be better than sleeping, but do we have enough API to do that without
   * reinventing dbus-glib? */
  g_usleep (G_USEC_PER_SEC / 100);
  test_main_context_iterate (ctx, FALSE);
  g_main_context_iteration (NULL, FALSE);
}
#endif

static DBusHandlerResult
observe_shouting_cb (DBusConnection *observer,
                     DBusMessage *message,
                     void *user_data)
{
  Fixture *f = user_data;

  if (dbus_message_is_signal (message, "com.example.Shouting", "Shouted"))
    {
      dbus_clear_message (&f->latest_shout);
      f->latest_shout = dbus_message_ref (message);
    }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
instance_removed_cb (GDBusConnection *observer,
                     const gchar *sender,
                     const gchar *path,
                     const gchar *iface,
                     const gchar *member,
                     GVariant *parameters,
                     gpointer user_data)
{
  Fixture *f = user_data;
  const gchar *container;

  g_assert_cmpstr (sender, ==, DBUS_SERVICE_DBUS);
  g_assert_cmpstr (path, ==, DBUS_PATH_DBUS);
  g_assert_cmpstr (iface, ==, DBUS_INTERFACE_CONTAINERS1);
  g_assert_cmpstr (member, ==, "InstanceRemoved");
  g_assert_cmpstr (g_variant_get_type_string (parameters), ==, "(o)");
  g_variant_get (parameters, "(&o)", &container);
  g_assert (!g_hash_table_contains (f->containers_removed, container));
  g_hash_table_add (f->containers_removed, g_strdup (container));
}

static void
fixture_disconnect_unconfined (Fixture *f)
{
  if (f->unconfined_conn != NULL)
    {
      GError *error = NULL;

      g_dbus_connection_close_sync (f->unconfined_conn, NULL, &error);

      if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CLOSED))
        g_clear_error (&error);
      else
        g_assert_no_error (error);
    }

  g_clear_object (&f->unconfined_conn);
}

static void
fixture_disconnect_observer (Fixture *f)
{
  if (f->observer_conn != NULL)
    {
      GError *error = NULL;

      g_dbus_connection_signal_unsubscribe (f->observer_conn,
                                            f->removed_sub);

      g_dbus_connection_close_sync (f->observer_conn, NULL, &error);

      if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CLOSED))
        g_clear_error (&error);
      else
        g_assert_no_error (error);
    }

  g_clear_object (&f->observer_conn);
}

static void
setup (Fixture *f,
       gconstpointer context)
{
  const Config *config = context;

  if (config == NULL)
    config = &default_config;

  f->ctx = test_main_context_get ();

  f->bus_address = test_get_dbus_daemon (config->config_file, TEST_USER_ME,
                                         NULL, &f->daemon_pid);

  if (f->bus_address == NULL)
    {
      f->skip = TRUE;
      return;
    }

  f->unconfined_conn = g_dbus_connection_new_for_address_sync (f->bus_address,
      (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
       G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
      NULL, NULL, &f->error);
  g_assert_no_error (f->error);
  f->unconfined_unique_name = g_strdup (
      g_dbus_connection_get_unique_name (f->unconfined_conn));
  g_test_message ("Unconfined connection: \"%s\"",
                  f->unconfined_unique_name);

  f->observer_conn = g_dbus_connection_new_for_address_sync (f->bus_address,
      (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
       G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
      NULL, NULL, &f->error);
  g_assert_no_error (f->error);
  f->containers_removed = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                 g_free, NULL);
  f->removed_sub = g_dbus_connection_signal_subscribe (f->observer_conn,
                                                       DBUS_SERVICE_DBUS,
                                                       DBUS_INTERFACE_CONTAINERS1,
                                                       "InstanceRemoved",
                                                       DBUS_PATH_DBUS, NULL,
                                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                                       instance_removed_cb,
                                                       f, NULL);

  /* We have to use libdbus for new header fields, because GDBus doesn't
   * yet have API for that. */
  f->libdbus_observer = test_connect_to_bus (f->ctx, f->bus_address);
  dbus_bus_add_match (f->libdbus_observer,
                      "interface='com.example.Shouting'", NULL);

  if (!dbus_connection_add_filter (f->libdbus_observer, observe_shouting_cb, f,
                                   NULL))
    g_error ("OOM");
}

/*
 * Assert that Get(SupportedArguments) contains what we expect it to.
 */
static void
test_get_supported_arguments (Fixture *f,
                              gconstpointer context)
{
  GVariant *v;
#ifdef DBUS_ENABLE_CONTAINERS
  const gchar **args;
  gsize len;
#endif

  if (f->skip)
    return;

  f->proxy = g_dbus_proxy_new_sync (f->unconfined_conn, G_DBUS_PROXY_FLAGS_NONE,
                                    NULL, DBUS_SERVICE_DBUS,
                                    DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,
                                    NULL, &f->error);

  /* This one is DBUS_ENABLE_CONTAINERS rather than HAVE_CONTAINERS_TEST
   * because we can still test whether the interface appears or not, even
   * if we were not able to detect gio-unix-2.0 */
#ifdef DBUS_ENABLE_CONTAINERS
  g_assert_no_error (f->error);

  v = g_dbus_proxy_get_cached_property (f->proxy, "SupportedArguments");
  g_assert_cmpstr (g_variant_get_type_string (v), ==, "as");
  args = g_variant_get_strv (v, &len);

  /* No arguments are defined yet */
  g_assert_cmpuint (len, ==, 0);

  g_free (args);
  g_variant_unref (v);
#else /* !DBUS_ENABLE_CONTAINERS */
  g_assert_no_error (f->error);
  v = g_dbus_proxy_get_cached_property (f->proxy, "SupportedArguments");
  g_assert_null (v);
#endif /* !DBUS_ENABLE_CONTAINERS */
}

#ifdef HAVE_CONTAINERS_TEST
/*
 * Try to make an AddServer call that usually succeeds, but may fail and
 * be skipped if we are running as root and this version of dbus has not
 * been fully installed. Return TRUE if we can continue.
 *
 * parameters is sunk if it is a floating reference.
 */
static gboolean
add_container_server (Fixture *f,
                      GVariant *parameters)
{
  GVariant *tuple;
  GStatBuf stat_buf;

  f->proxy = g_dbus_proxy_new_sync (f->unconfined_conn,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                    NULL, DBUS_SERVICE_DBUS,
                                    DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,
                                    NULL, &f->error);
  g_assert_no_error (f->error);

  g_test_message ("Calling AddServer...");
  tuple = g_dbus_proxy_call_sync (f->proxy, "AddServer", parameters,
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);

  /* For root, the sockets go in /run/dbus/containers, which we rely on
   * system infrastructure to create; so it's OK for AddServer to fail
   * when uninstalled, although not OK if it fails as an installed-test. */
  if (f->error != NULL &&
      _dbus_getuid () == 0 &&
      _dbus_getenv ("DBUS_TEST_UNINSTALLED") != NULL)
    {
      g_test_message ("AddServer: %s", f->error->message);
      g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_FILE_NOT_FOUND);
      g_test_skip ("AddServer failed, probably because this dbus "
                   "version is not fully installed");
      return FALSE;
    }

  g_assert_no_error (f->error);
  g_assert_nonnull (tuple);

  g_assert_cmpstr (g_variant_get_type_string (tuple), ==, "(oays)");
  g_variant_get (tuple, "(o^ays)", &f->instance_path, &f->socket_path,
                 &f->socket_dbus_address);
  g_assert_true (g_str_has_prefix (f->socket_dbus_address, "unix:"));
  g_assert_null (strchr (f->socket_dbus_address, ';'));
  g_assert_null (strchr (f->socket_dbus_address + strlen ("unix:"), ':'));
  g_clear_pointer (&tuple, g_variant_unref);

  g_assert_nonnull (f->instance_path);
  g_assert_true (g_variant_is_object_path (f->instance_path));
  g_assert_nonnull (f->socket_path);
  g_assert_true (g_path_is_absolute (f->socket_path));
  g_assert_nonnull (f->socket_dbus_address);
  g_assert_cmpstr (g_stat (f->socket_path, &stat_buf) == 0 ? NULL :
                   g_strerror (errno), ==, NULL);
  g_assert_cmpuint ((stat_buf.st_mode & S_IFMT), ==, S_IFSOCK);
  return TRUE;
}
#endif

/*
 * Assert that a simple AddServer() call succeeds and has the behaviour
 * we expect (we can connect a confined connection to it, the confined
 * connection can talk to the dbus-daemon and to an unconfined connection,
 * and the socket gets cleaned up when the dbus-daemon terminates).
 *
 * This also tests simple cases for metadata.
 */
static void
test_basic (Fixture *f,
            gconstpointer context)
{
#ifdef HAVE_CONTAINERS_TEST
  GVariant *asv;
  GVariant *creator;
  GVariant *parameters;
  GVariantDict dict;
  const gchar *confined_unique_name;
  const gchar *path_from_query;
  const gchar *name;
  const gchar *name_owner;
  const gchar *type;
  guint32 uid;
  GStatBuf stat_buf;
  GVariant *tuple;
  DBusMessage *libdbus_message = NULL;
  DBusMessage *libdbus_reply = NULL;
  DBusError libdbus_error = DBUS_ERROR_INIT;

  if (f->skip)
    return;

  parameters = g_variant_new ("(ssa{sv}a{sv})",
                              "com.example.NotFlatpak",
                              "sample-app",
                              NULL, /* no metadata */
                              NULL); /* no named arguments */
  if (!add_container_server (f, g_steal_pointer (&parameters)))
    return;

  g_test_message ("Connecting to %s...", f->socket_dbus_address);
  f->confined_conn = g_dbus_connection_new_for_address_sync (
      f->socket_dbus_address,
      (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
       G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
      NULL, NULL, &f->error);
  g_assert_no_error (f->error);

  g_test_message ("Making a method call from confined app...");
  tuple = g_dbus_connection_call_sync (f->confined_conn, DBUS_SERVICE_DBUS,
                                       DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,
                                       "GetNameOwner",
                                       g_variant_new ("(s)", DBUS_SERVICE_DBUS),
                                       G_VARIANT_TYPE ("(s)"),
                                       G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                       &f->error);
  g_assert_no_error (f->error);
  g_assert_nonnull (tuple);
  g_assert_cmpstr (g_variant_get_type_string (tuple), ==, "(s)");
  g_variant_get (tuple, "(&s)", &name_owner);
  g_assert_cmpstr (name_owner, ==, DBUS_SERVICE_DBUS);
  g_clear_pointer (&tuple, g_variant_unref);

  g_test_message ("Making a method call from confined app to unconfined...");
  tuple = g_dbus_connection_call_sync (f->confined_conn,
                                       f->unconfined_unique_name,
                                       "/", DBUS_INTERFACE_PEER,
                                       "Ping",
                                       NULL, G_VARIANT_TYPE_UNIT,
                                       G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                       &f->error);
  g_assert_no_error (f->error);
  g_assert_nonnull (tuple);
  g_assert_cmpstr (g_variant_get_type_string (tuple), ==, "()");
  g_clear_pointer (&tuple, g_variant_unref);

  g_test_message ("Receiving signals without requesting extra headers");
  g_dbus_connection_emit_signal (f->confined_conn, NULL, "/",
                                 "com.example.Shouting", "Shouted",
                                 NULL, NULL);

  while (f->latest_shout == NULL)
    iterate_both_main_loops (f->ctx);

  g_assert_cmpstr (dbus_message_get_container_instance (f->latest_shout), ==,
                   NULL);
  dbus_clear_message (&f->latest_shout);

  g_dbus_connection_emit_signal (f->unconfined_conn, NULL, "/",
                                 "com.example.Shouting", "Shouted",
                                 NULL, NULL);

  while (f->latest_shout == NULL)
    iterate_both_main_loops (f->ctx);

  g_assert_cmpstr (dbus_message_get_container_instance (f->latest_shout), ==,
                   NULL);
  dbus_clear_message (&f->latest_shout);

  g_test_message ("Receiving signals after requesting extra headers");

  libdbus_message = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
                                                  DBUS_PATH_DBUS,
                                                  DBUS_INTERFACE_CONTAINERS1,
                                                  "RequestHeader");
  libdbus_reply = test_main_context_call_and_wait (f->ctx,
                                                   f->libdbus_observer,
                                                   libdbus_message,
                                                   DBUS_TIMEOUT_USE_DEFAULT);

  if (dbus_set_error_from_message (&libdbus_error, libdbus_reply))
    g_error ("%s", libdbus_error.message);

  dbus_clear_message (&libdbus_message);
  dbus_clear_message (&libdbus_reply);

  g_dbus_connection_emit_signal (f->confined_conn, NULL, "/",
                                 "com.example.Shouting", "Shouted",
                                 NULL, NULL);

  while (f->latest_shout == NULL)
    iterate_both_main_loops (f->ctx);

  g_assert_cmpstr (dbus_message_get_container_instance (f->latest_shout), ==,
                   f->instance_path);
  dbus_clear_message (&f->latest_shout);

  g_dbus_connection_emit_signal (f->unconfined_conn, NULL, "/",
                                 "com.example.Shouting", "Shouted",
                                 NULL, NULL);

  while (f->latest_shout == NULL)
    iterate_both_main_loops (f->ctx);

  g_assert_cmpstr (dbus_message_get_container_instance (f->latest_shout), ==,
                   "/");
  dbus_clear_message (&f->latest_shout);

  g_test_message ("Checking that confined app is not considered privileged...");
  tuple = g_dbus_connection_call_sync (f->confined_conn, DBUS_SERVICE_DBUS,
                                       DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,
                                       "UpdateActivationEnvironment",
                                       g_variant_new ("(a{ss})", NULL),
                                       G_VARIANT_TYPE_UNIT,
                                       G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                       &f->error);
  g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED);
  g_test_message ("Access denied as expected: %s", f->error->message);
  g_clear_error (&f->error);
  g_assert_null (tuple);

  g_test_message ("Inspecting connection container info");
  confined_unique_name = g_dbus_connection_get_unique_name (f->confined_conn);
  tuple = g_dbus_proxy_call_sync (f->proxy, "GetConnectionInstance",
                                  g_variant_new ("(s)", confined_unique_name),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error (f->error);
  g_assert_nonnull (tuple);
  g_assert_cmpstr (g_variant_get_type_string (tuple), ==, "(oa{sv}ssa{sv})");
  g_variant_get (tuple, "(&o@a{sv}&s&s@a{sv})",
                 &path_from_query, &creator, &type, &name, &asv);
  g_assert_cmpstr (path_from_query, ==, f->instance_path);
  g_variant_dict_init (&dict, creator);
  g_assert_true (g_variant_dict_lookup (&dict, "UnixUserID", "u", &uid));
  g_assert_cmpuint (uid, ==, _dbus_getuid ());
  g_variant_dict_clear (&dict);
  g_assert_cmpstr (type, ==, "com.example.NotFlatpak");
  g_assert_cmpstr (name, ==, "sample-app");
  /* Trivial case: the metadata a{sv} is empty */
  g_assert_cmpuint (g_variant_n_children (asv), ==, 0);
  g_clear_pointer (&asv, g_variant_unref);
  g_clear_pointer (&creator, g_variant_unref);
  g_clear_pointer (&tuple, g_variant_unref);

  g_test_message ("Inspecting container instance info");
  tuple = g_dbus_proxy_call_sync (f->proxy, "GetInstanceInfo",
                                  g_variant_new ("(o)", f->instance_path),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error (f->error);
  g_assert_nonnull (tuple);
  g_assert_cmpstr (g_variant_get_type_string (tuple), ==, "(a{sv}ssa{sv})");
  g_variant_get (tuple, "(@a{sv}&s&s@a{sv})", &creator, &type, &name, &asv);
  g_variant_dict_init (&dict, creator);
  g_assert_true (g_variant_dict_lookup (&dict, "UnixUserID", "u", &uid));
  g_assert_cmpuint (uid, ==, _dbus_getuid ());
  g_variant_dict_clear (&dict);
  g_assert_cmpstr (type, ==, "com.example.NotFlatpak");
  g_assert_cmpstr (name, ==, "sample-app");
  /* Trivial case: the metadata a{sv} is empty */
  g_assert_cmpuint (g_variant_n_children (asv), ==, 0);
  g_clear_pointer (&asv, g_variant_unref);
  g_clear_pointer (&creator, g_variant_unref);
  g_clear_pointer (&tuple, g_variant_unref);

  /* Check that the socket is cleaned up when the dbus-daemon is terminated */
  test_kill_pid (f->daemon_pid);
  g_spawn_close_pid (f->daemon_pid);
  f->daemon_pid = 0;

  while (g_stat (f->socket_path, &stat_buf) == 0)
    g_usleep (G_USEC_PER_SEC / 20);

  g_assert_cmpint (errno, ==, ENOENT);

#else /* !HAVE_CONTAINERS_TEST */
  g_test_skip ("Containers or gio-unix-2.0 not supported");
#endif /* !HAVE_CONTAINERS_TEST */
}

/*
 * If we are running as root, assert that when one uid (root) creates a
 * container server, another uid (TEST_USER_OTHER) cannot connect to it
 */
static void
test_wrong_uid (Fixture *f,
                gconstpointer context)
{
#ifdef HAVE_CONTAINERS_TEST
  GVariant *parameters;

  if (f->skip)
    return;

  parameters = g_variant_new ("(ssa{sv}a{sv})",
                              "com.example.NotFlatpak",
                              "sample-app",
                              NULL, /* no metadata */
                              NULL); /* no named arguments */
  if (!add_container_server (f, g_steal_pointer (&parameters)))
    return;

  g_test_message ("Connecting to %s...", f->socket_dbus_address);
  f->confined_conn = test_try_connect_gdbus_as_user (f->socket_dbus_address,
                                                     TEST_USER_OTHER,
                                                     &f->error);

  /* That might be skipped if we can't become TEST_USER_OTHER */
  if (f->error != NULL &&
      g_error_matches (f->error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
    {
      g_test_skip (f->error->message);
      return;
    }

  /* The connection was unceremoniously closed */
  g_assert_error (f->error, G_IO_ERROR, G_IO_ERROR_CLOSED);

#else /* !HAVE_CONTAINERS_TEST */
  g_test_skip ("Containers or gio-unix-2.0 not supported");
#endif /* !HAVE_CONTAINERS_TEST */
}

/*
 * Test for non-trivial metadata: assert that the metadata a{sv} is
 * carried through correctly, and that the app name is allowed to be empty.
 */
static void
test_metadata (Fixture *f,
               gconstpointer context)
{
#ifdef HAVE_CONTAINERS_TEST
  GVariant *asv;
  GVariant *creator;
  GVariant *tuple;
  GVariant *parameters;
  GVariantDict dict;
  const gchar *confined_unique_name;
  const gchar *path_from_query;
  const gchar *name;
  const gchar *type;
  guint32 uid;
  guint u;
  gboolean b;
  const gchar *s;

  if (f->skip)
    return;

  g_variant_dict_init (&dict, NULL);
  g_variant_dict_insert (&dict, "Species", "s", "Martes martes");
  g_variant_dict_insert (&dict, "IsCrepuscular", "b", TRUE);
  g_variant_dict_insert (&dict, "NChildren", "u", 2);

  parameters = g_variant_new ("(ss@a{sv}a{sv})",
                              "org.example.Springwatch",
                              /* Verify that empty app names are OK */
                              "",
                              g_variant_dict_end (&dict),
                              NULL); /* no named arguments */
  if (!add_container_server (f, g_steal_pointer (&parameters)))
    return;

  g_test_message ("Connecting to %s...", f->socket_dbus_address);
  f->confined_conn = g_dbus_connection_new_for_address_sync (
      f->socket_dbus_address,
      (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
       G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
      NULL, NULL, &f->error);
  g_assert_no_error (f->error);

  g_test_message ("Inspecting connection credentials...");
  confined_unique_name = g_dbus_connection_get_unique_name (f->confined_conn);
  tuple = g_dbus_connection_call_sync (f->confined_conn, DBUS_SERVICE_DBUS,
                                       DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,
                                       "GetConnectionCredentials",
                                       g_variant_new ("(s)",
                                                      confined_unique_name),
                                       G_VARIANT_TYPE ("(a{sv})"),
                                       G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                       &f->error);
  g_assert_no_error (f->error);
  g_assert_nonnull (tuple);
  g_assert_cmpstr (g_variant_get_type_string (tuple), ==, "(a{sv})");
  asv = g_variant_get_child_value (tuple, 0);
  g_variant_dict_init (&dict, asv);
  g_assert_true (g_variant_dict_lookup (&dict,
                                        DBUS_INTERFACE_CONTAINERS1 ".Instance",
                                        "&o", &path_from_query));
  g_assert_cmpstr (path_from_query, ==, f->instance_path);
  g_variant_dict_clear (&dict);
  g_clear_pointer (&asv, g_variant_unref);
  g_clear_pointer (&tuple, g_variant_unref);

  g_test_message ("Inspecting connection container info");
  tuple = g_dbus_proxy_call_sync (f->proxy, "GetConnectionInstance",
                                  g_variant_new ("(s)", confined_unique_name),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error (f->error);
  g_assert_nonnull (tuple);
  g_assert_cmpstr (g_variant_get_type_string (tuple), ==, "(oa{sv}ssa{sv})");
  g_variant_get (tuple, "(&o@a{sv}&s&s@a{sv})",
                 &path_from_query, &creator, &type, &name, &asv);
  g_assert_cmpstr (path_from_query, ==, f->instance_path);
  g_variant_dict_init (&dict, creator);
  g_assert_true (g_variant_dict_lookup (&dict, "UnixUserID", "u", &uid));
  g_assert_cmpuint (uid, ==, _dbus_getuid ());
  g_variant_dict_clear (&dict);
  g_assert_cmpstr (type, ==, "org.example.Springwatch");
  g_assert_cmpstr (name, ==, "");
  g_variant_dict_init (&dict, asv);
  g_assert_true (g_variant_dict_lookup (&dict, "NChildren", "u", &u));
  g_assert_cmpuint (u, ==, 2);
  g_assert_true (g_variant_dict_lookup (&dict, "IsCrepuscular", "b", &b));
  g_assert_cmpint (b, ==, TRUE);
  g_assert_true (g_variant_dict_lookup (&dict, "Species", "&s", &s));
  g_assert_cmpstr (s, ==, "Martes martes");
  g_variant_dict_clear (&dict);
  g_assert_cmpuint (g_variant_n_children (asv), ==, 3);
  g_clear_pointer (&asv, g_variant_unref);
  g_clear_pointer (&creator, g_variant_unref);
  g_clear_pointer (&tuple, g_variant_unref);

  g_test_message ("Inspecting container instance info");
  tuple = g_dbus_proxy_call_sync (f->proxy, "GetInstanceInfo",
                                  g_variant_new ("(o)", f->instance_path),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error (f->error);
  g_assert_nonnull (tuple);
  g_assert_cmpstr (g_variant_get_type_string (tuple), ==, "(a{sv}ssa{sv})");
  g_variant_get (tuple, "(@a{sv}&s&s@a{sv})", &creator, &type, &name, &asv);
  g_variant_dict_init (&dict, creator);
  g_assert_true (g_variant_dict_lookup (&dict, "UnixUserID", "u", &uid));
  g_assert_cmpuint (uid, ==, _dbus_getuid ());
  g_variant_dict_clear (&dict);
  g_assert_cmpstr (type, ==, "org.example.Springwatch");
  g_assert_cmpstr (name, ==, "");
  g_variant_dict_init (&dict, asv);
  g_assert_true (g_variant_dict_lookup (&dict, "NChildren", "u", &u));
  g_assert_cmpuint (u, ==, 2);
  g_assert_true (g_variant_dict_lookup (&dict, "IsCrepuscular", "b", &b));
  g_assert_cmpint (b, ==, TRUE);
  g_assert_true (g_variant_dict_lookup (&dict, "Species", "&s", &s));
  g_assert_cmpstr (s, ==, "Martes martes");
  g_variant_dict_clear (&dict);
  g_assert_cmpuint (g_variant_n_children (asv), ==, 3);
  g_clear_pointer (&asv, g_variant_unref);
  g_clear_pointer (&creator, g_variant_unref);
  g_clear_pointer (&tuple, g_variant_unref);

#else /* !HAVE_CONTAINERS_TEST */
  g_test_skip ("Containers or gio-unix-2.0 not supported");
#endif /* !HAVE_CONTAINERS_TEST */
}

/*
 * With config->stop_server == STOP_SERVER_WITH_MANAGER:
 * Assert that without special parameters, when the container manager
 * disappears from the bus, so does the confined server.
 *
 * With config->stop_server == STOP_SERVER_EXPLICITLY or
 * config->stop_server == STOP_SERVER_DISCONNECT_FIRST:
 * Test StopListening(), which just closes the listening socket.
 *
 * With config->stop_server == STOP_SERVER_FORCE:
 * Test StopInstance(), which closes the listening socket and
 * disconnects all its clients.
 */
static void
test_stop_server (Fixture *f,
                  gconstpointer context)
{
#ifdef HAVE_CONTAINERS_TEST
  const Config *config = context;
  GDBusConnection *attacker;
  GDBusConnection *second_confined_conn;
  GDBusProxy *attacker_proxy;
  GSocket *client_socket;
  GSocketAddress *socket_address;
  GVariant *tuple;
  GVariant *parameters;
  gchar *error_name;
  const gchar *confined_unique_name;
  const gchar *name_owner;
  gboolean gone = FALSE;
  guint name_watch;
  guint i;

  g_assert_nonnull (config);

  if (f->skip)
    return;

  f->observer_proxy = g_dbus_proxy_new_sync (f->observer_conn,
                                             G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                             NULL, DBUS_SERVICE_DBUS,
                                             DBUS_PATH_DBUS,
                                             DBUS_INTERFACE_CONTAINERS1, NULL,
                                             &f->error);
  g_assert_no_error (f->error);

  parameters = g_variant_new ("(ssa{sv}a{sv})",
                              "com.example.NotFlatpak",
                              "sample-app",
                              NULL, /* no metadata */
                              NULL); /* no named arguments */
  if (!add_container_server (f, g_steal_pointer (&parameters)))
    return;

  socket_address = g_unix_socket_address_new (f->socket_path);

  if (config->stop_server != STOP_SERVER_NEVER_CONNECTED)
    {
      g_test_message ("Connecting to %s...", f->socket_dbus_address);
      f->confined_conn = g_dbus_connection_new_for_address_sync (
          f->socket_dbus_address,
          (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
           G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
          NULL, NULL, &f->error);
      g_assert_no_error (f->error);

      if (config->stop_server == STOP_SERVER_DISCONNECT_FIRST)
        {
          g_test_message ("Disconnecting confined connection...");
          gone = FALSE;
          confined_unique_name = g_dbus_connection_get_unique_name (
              f->confined_conn);
          name_watch = g_bus_watch_name_on_connection (f->observer_conn,
                                                       confined_unique_name,
                                                       G_BUS_NAME_WATCHER_FLAGS_NONE,
                                                       NULL,
                                                       name_gone_set_boolean_cb,
                                                       &gone, NULL);
          g_dbus_connection_close_sync (f->confined_conn, NULL, &f->error);
          g_assert_no_error (f->error);

          g_test_message ("Waiting for confined app bus name to disappear...");

          while (!gone)
            g_main_context_iteration (NULL, TRUE);

          g_bus_unwatch_name (name_watch);
        }
    }

  /* If we are able to switch uid (i.e. we are root), check that a local
   * attacker with a different uid cannot close our container instances. */
  attacker = test_try_connect_gdbus_as_user (f->bus_address, TEST_USER_OTHER,
                                             &f->error);

  if (attacker != NULL)
    {
      attacker_proxy = g_dbus_proxy_new_sync (attacker,
                                              G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                              NULL, DBUS_SERVICE_DBUS,
                                              DBUS_PATH_DBUS,
                                              DBUS_INTERFACE_CONTAINERS1, NULL,
                                              &f->error);
      g_assert_no_error (f->error);

      tuple = g_dbus_proxy_call_sync (attacker_proxy, "StopListening",
                                      g_variant_new ("(o)", f->instance_path),
                                      G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                      &f->error);
      g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED);
      g_assert_null (tuple);
      g_clear_error (&f->error);

      tuple = g_dbus_proxy_call_sync (attacker_proxy, "StopInstance",
                                      g_variant_new ("(o)", f->instance_path),
                                      G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                      &f->error);
      g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED);
      g_assert_null (tuple);
      g_clear_error (&f->error);

      g_clear_object (&attacker_proxy);
      g_dbus_connection_close_sync (attacker, NULL, &f->error);
      g_assert_no_error (f->error);
      g_clear_object (&attacker);
    }
  else
    {
      /* If we aren't running as root, it's OK to not be able to connect again
       * as some other user (usually 'nobody'). We don't g_test_skip() here
       * because this is just extra coverage */
      g_assert_error (f->error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED);
      g_clear_error (&f->error);
    }

  g_assert_false (g_hash_table_contains (f->containers_removed,
                                         f->instance_path));

  switch (config->stop_server)
    {
      case STOP_SERVER_WITH_MANAGER:
        /* Close the unconfined connection (the container manager) and wait
         * for it to go away */
        g_test_message ("Closing container manager...");
        name_watch = g_bus_watch_name_on_connection (f->confined_conn,
                                                     f->unconfined_unique_name,
                                                     G_BUS_NAME_WATCHER_FLAGS_NONE,
                                                     NULL,
                                                     name_gone_set_boolean_cb,
                                                     &gone, NULL);
        fixture_disconnect_unconfined (f);

        g_test_message ("Waiting for container manager bus name to disappear...");

        while (!gone)
          g_main_context_iteration (NULL, TRUE);

        g_bus_unwatch_name (name_watch);
        break;

      case STOP_SERVER_EXPLICITLY:
        g_test_message ("Stopping server (but not confined connection)...");
        tuple = g_dbus_proxy_call_sync (f->proxy, "StopListening",
                                        g_variant_new ("(o)", f->instance_path),
                                        G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                        &f->error);
        g_assert_no_error (f->error);
        g_variant_unref (tuple);

        /* The container instance remains open, because the connection has
         * not gone away yet. Do another method call: if we were going to
         * get the signal, it would arrive before the reply to this second
         * method call. Any method will do here, even one that doesn't
         * exist. */
        g_test_message ("Checking we do not get InstanceRemoved...");
        tuple = g_dbus_proxy_call_sync (f->proxy, "NoSuchMethod", NULL,
                                        G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                        &f->error);
        g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD);
        g_assert_null (tuple);
        g_clear_error (&f->error);
        break;

      case STOP_SERVER_DISCONNECT_FIRST:
      case STOP_SERVER_NEVER_CONNECTED:
        g_test_message ("Stopping server (with no confined connections)...");
        tuple = g_dbus_proxy_call_sync (f->proxy, "StopListening",
                                        g_variant_new ("(o)", f->instance_path),
                                        G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                        &f->error);
        g_assert_no_error (f->error);
        g_variant_unref (tuple);

        g_test_message ("Waiting for InstanceRemoved...");
        while (!g_hash_table_contains (f->containers_removed, f->instance_path))
          g_main_context_iteration (NULL, TRUE);

        break;

      case STOP_SERVER_FORCE:
        g_test_message ("Stopping server and all confined connections...");
        tuple = g_dbus_proxy_call_sync (f->proxy, "StopInstance",
                                        g_variant_new ("(o)", f->instance_path),
                                        G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                        &f->error);
        g_assert_no_error (f->error);
        g_variant_unref (tuple);

        g_test_message ("Waiting for InstanceRemoved...");
        while (!g_hash_table_contains (f->containers_removed, f->instance_path))
          g_main_context_iteration (NULL, TRUE);

        break;

      default:
        g_assert_not_reached ();
    }

  /* Now if we try to connect to the server again, it will fail (eventually -
   * closing the socket is not synchronous with respect to the name owner
   * change, so try a few times) */
  for (i = 0; i < 50; i++)
    {
      g_test_message ("Trying to connect to %s again...", f->socket_path);
      client_socket = g_socket_new (G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM,
                                    G_SOCKET_PROTOCOL_DEFAULT, &f->error);
      g_assert_no_error (f->error);

      if (!g_socket_connect (client_socket, socket_address, NULL, &f->error))
        {
          g_assert_cmpstr (g_quark_to_string (f->error->domain), ==,
                           g_quark_to_string (G_IO_ERROR));

          if (f->error->code != G_IO_ERROR_CONNECTION_REFUSED &&
              f->error->code != G_IO_ERROR_NOT_FOUND)
            g_error ("Unexpected error code %d", f->error->code);

          g_clear_error (&f->error);
          g_clear_object (&client_socket);
          break;
        }

      g_clear_object (&client_socket);
      g_usleep (G_USEC_PER_SEC / 10);
    }

  /* The same thing happens for a D-Bus connection */
  g_test_message ("Trying to connect to %s again...", f->socket_dbus_address);
  second_confined_conn = g_dbus_connection_new_for_address_sync (
      f->socket_dbus_address,
      (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
       G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
      NULL, NULL, &f->error);
  g_assert_cmpstr (g_quark_to_string (f->error->domain), ==,
                   g_quark_to_string (G_IO_ERROR));

  if (f->error->code != G_IO_ERROR_CONNECTION_REFUSED &&
      f->error->code != G_IO_ERROR_NOT_FOUND)
    g_error ("Unexpected error code %d", f->error->code);

  g_clear_error (&f->error);
  g_assert_null (second_confined_conn);

  /* Deleting the socket is not synchronous with respect to stopping
   * listening on it, so again we are willing to wait a few seconds */
  for (i = 0; i < 50; i++)
    {
      if (g_file_test (f->socket_path, G_FILE_TEST_EXISTS))
        g_usleep (G_USEC_PER_SEC / 10);
    }

  /* The socket has been deleted */
  g_assert_false (g_file_test (f->socket_path, G_FILE_TEST_EXISTS));

  switch (config->stop_server)
    {
      case STOP_SERVER_FORCE:
        g_test_message ("Checking that the confined app gets disconnected...");

        while (!g_dbus_connection_is_closed (f->confined_conn))
          g_main_context_iteration (NULL, TRUE);
        break;

      case STOP_SERVER_DISCONNECT_FIRST:
      case STOP_SERVER_NEVER_CONNECTED:
        /* Nothing to be done here, no confined app is connected */
        break;

      case STOP_SERVER_EXPLICITLY:
      case STOP_SERVER_WITH_MANAGER:
        g_test_message ("Checking that the confined app still works...");
        tuple = g_dbus_connection_call_sync (f->confined_conn,
                                             DBUS_SERVICE_DBUS,
                                             DBUS_PATH_DBUS,
                                             DBUS_INTERFACE_DBUS,
                                             "GetNameOwner",
                                             g_variant_new ("(s)",
                                                            DBUS_SERVICE_DBUS),
                                             G_VARIANT_TYPE ("(s)"),
                                             G_DBUS_CALL_FLAGS_NONE, -1,
                                             NULL, &f->error);
        g_assert_no_error (f->error);
        g_assert_nonnull (tuple);
        g_assert_cmpstr (g_variant_get_type_string (tuple), ==, "(s)");
        g_variant_get (tuple, "(&s)", &name_owner);
        g_assert_cmpstr (name_owner, ==, DBUS_SERVICE_DBUS);
        g_clear_pointer (&tuple, g_variant_unref);

        /* The container instance will not disappear from the bus
         * until the confined connection goes away */
        tuple = g_dbus_proxy_call_sync (f->observer_proxy, "GetInstanceInfo",
                                        g_variant_new ("(o)", f->instance_path),
                                        G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                        &f->error);
        g_assert_no_error (f->error);
        g_assert_nonnull (tuple);
        g_clear_pointer (&tuple, g_variant_unref);

        /* Now disconnect the last confined connection, which will make the
         * container instance go away */
        g_test_message ("Closing confined connection...");
        g_dbus_connection_close_sync (f->confined_conn, NULL, &f->error);
        g_assert_no_error (f->error);
        break;

      default:
        g_assert_not_reached ();
    }

  /* Whatever happened above, by now it has gone away */

  g_test_message ("Waiting for InstanceRemoved...");
  while (!g_hash_table_contains (f->containers_removed, f->instance_path))
    g_main_context_iteration (NULL, TRUE);

  tuple = g_dbus_proxy_call_sync (f->observer_proxy, "GetInstanceInfo",
                                  g_variant_new ("(o)", f->instance_path),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                  &f->error);
  g_assert_nonnull (f->error);
  error_name = g_dbus_error_get_remote_error (f->error);
  g_assert_cmpstr (error_name, ==, DBUS_ERROR_NOT_CONTAINER);
  g_free (error_name);
  g_assert_null (tuple);
  g_clear_error (&f->error);
  g_clear_object (&socket_address);

#else /* !HAVE_CONTAINERS_TEST */
  g_test_skip ("Containers or gio-unix-2.0 not supported");
#endif /* !HAVE_CONTAINERS_TEST */
}

/*
 * Assert that we cannot get the container metadata for a path that
 * isn't a container instance, or a bus name that isn't in a container
 * or doesn't exist at all.
 */
static void
test_invalid_metadata_getters (Fixture *f,
                               gconstpointer context)
{
  const gchar *unique_name;
  GVariant *tuple;
  gchar *error_name;

  f->proxy = g_dbus_proxy_new_sync (f->unconfined_conn,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                    NULL, DBUS_SERVICE_DBUS,
                                    DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,
                                    NULL, &f->error);
  g_assert_no_error (f->error);

  g_test_message ("Inspecting unconfined connection");
  unique_name = g_dbus_connection_get_unique_name (f->unconfined_conn);
  tuple = g_dbus_proxy_call_sync (f->proxy, "GetConnectionInstance",
                                  g_variant_new ("(s)", unique_name),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_nonnull (f->error);
  g_assert_null (tuple);
  error_name = g_dbus_error_get_remote_error (f->error);
#ifdef DBUS_ENABLE_CONTAINERS
  g_assert_cmpstr (error_name, ==, DBUS_ERROR_NOT_CONTAINER);
#else
  /* TODO: We can use g_assert_error for this when we depend on GLib 2.42 */
  g_assert_cmpstr (error_name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
#endif
  g_free (error_name);
  g_clear_error (&f->error);

  g_test_message ("Inspecting dbus-daemon");
  tuple = g_dbus_proxy_call_sync (f->proxy, "GetConnectionInstance",
                                  g_variant_new ("(s)", DBUS_SERVICE_DBUS),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_nonnull (f->error);
  g_assert_null (tuple);
  error_name = g_dbus_error_get_remote_error (f->error);
#ifdef DBUS_ENABLE_CONTAINERS
  g_assert_cmpstr (error_name, ==, DBUS_ERROR_NOT_CONTAINER);
#else
  /* TODO: We can use g_assert_error for this when we depend on GLib 2.42 */
  g_assert_cmpstr (error_name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
#endif
  g_free (error_name);
  g_clear_error (&f->error);

  g_test_message ("Inspecting a non-connection");
  unique_name = g_dbus_connection_get_unique_name (f->unconfined_conn);
  tuple = g_dbus_proxy_call_sync (f->proxy, "GetConnectionInstance",
                                  g_variant_new ("(s)", "com.example.Nope"),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_nonnull (f->error);
  g_assert_null (tuple);
#ifdef DBUS_ENABLE_CONTAINERS
  g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_NAME_HAS_NO_OWNER);
#else
  /* TODO: We can use g_assert_error for this when we depend on GLib 2.42 */
  error_name = g_dbus_error_get_remote_error (f->error);
  g_assert_cmpstr (error_name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  g_free (error_name);
#endif
  g_clear_error (&f->error);


  g_test_message ("Inspecting container instance info");
  tuple = g_dbus_proxy_call_sync (f->proxy, "GetInstanceInfo",
                                  g_variant_new ("(o)", "/nope"),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_nonnull (f->error);
  g_assert_null (tuple);
  error_name = g_dbus_error_get_remote_error (f->error);
#ifdef DBUS_ENABLE_CONTAINERS
  g_assert_cmpstr (error_name, ==, DBUS_ERROR_NOT_CONTAINER);
#else
  /* TODO: We can use g_assert_error for this when we depend on GLib 2.42 */
  g_assert_cmpstr (error_name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
#endif
  g_free (error_name);
  g_clear_error (&f->error);
}

/*
 * Assert that named arguments are validated: passing an unsupported
 * named argument causes an error.
 */
static void
test_unsupported_parameter (Fixture *f,
                            gconstpointer context)
{
#ifdef HAVE_CONTAINERS_TEST
  GVariant *tuple;
  GVariant *parameters;
  GVariantDict named_argument_builder;

  if (f->skip)
    return;

  f->proxy = g_dbus_proxy_new_sync (f->unconfined_conn,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                    NULL, DBUS_SERVICE_DBUS,
                                    DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,
                                    NULL, &f->error);
  g_assert_no_error (f->error);

  g_variant_dict_init (&named_argument_builder, NULL);
  g_variant_dict_insert (&named_argument_builder,
                         "ThisArgumentIsntImplemented",
                         "b", FALSE);

  parameters = g_variant_new ("(ssa{sv}@a{sv})",
                              "com.example.NotFlatpak",
                              "sample-app",
                              NULL, /* no metadata */
                              g_variant_dict_end (&named_argument_builder));
  tuple = g_dbus_proxy_call_sync (f->proxy, "AddServer",
                                  g_steal_pointer (&parameters),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);

  g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS);
  g_assert_null (tuple);
  g_clear_error (&f->error);
#else /* !HAVE_CONTAINERS_TEST */
  g_test_skip ("Containers or gio-unix-2.0 not supported");
#endif /* !HAVE_CONTAINERS_TEST */
}

/*
 * Assert that container types are validated: a container type (container
 * technology) that is not a syntactically valid D-Bus interface name
 * causes an error.
 */
static void
test_invalid_type_name (Fixture *f,
                        gconstpointer context)
{
#ifdef HAVE_CONTAINERS_TEST
  GVariant *tuple;
  GVariant *parameters;

  if (f->skip)
    return;

  f->proxy = g_dbus_proxy_new_sync (f->unconfined_conn,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                    NULL, DBUS_SERVICE_DBUS,
                                    DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,
                                    NULL, &f->error);
  g_assert_no_error (f->error);

  parameters = g_variant_new ("(ssa{sv}a{sv})",
                              "this is not a valid container type name",
                              "sample-app",
                              NULL, /* no metadata */
                              NULL); /* no named arguments */
  tuple = g_dbus_proxy_call_sync (f->proxy, "AddServer",
                                  g_steal_pointer (&parameters),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);

  g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS);
  g_assert_null (tuple);
  g_clear_error (&f->error);
#else /* !HAVE_CONTAINERS_TEST */
  g_test_skip ("Containers or gio-unix-2.0 not supported");
#endif /* !HAVE_CONTAINERS_TEST */
}

/*
 * Assert that a request to create a container server cannot come from a
 * connection to an existing container server.
 * (You cannot put containers in your container so you can sandbox while
 * you sandbox.)
 */
static void
test_invalid_nesting (Fixture *f,
                      gconstpointer context)
{
#ifdef HAVE_CONTAINERS_TEST
  GDBusProxy *nested_proxy;
  GVariant *tuple;
  GVariant *parameters;

  if (f->skip)
    return;

  parameters = g_variant_new ("(ssa{sv}a{sv})",
                              "com.example.NotFlatpak",
                              "sample-app",
                              NULL, /* no metadata */
                              NULL); /* no named arguments */
  if (!add_container_server (f, g_steal_pointer (&parameters)))
    return;

  g_test_message ("Connecting to %s...", f->socket_dbus_address);
  f->confined_conn = g_dbus_connection_new_for_address_sync (
      f->socket_dbus_address,
      (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
       G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
      NULL, NULL, &f->error);
  g_assert_no_error (f->error);

  g_test_message ("Checking that confined app cannot nest containers...");
  nested_proxy = g_dbus_proxy_new_sync (f->confined_conn,
                                        G_DBUS_PROXY_FLAGS_NONE, NULL,
                                        DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                        DBUS_INTERFACE_CONTAINERS1, NULL,
                                        &f->error);
  g_assert_no_error (f->error);

  parameters = g_variant_new ("(ssa{sv}a{sv})",
                              "com.example.NotFlatpak",
                              "inner-app",
                              NULL, /* no metadata */
                              NULL); /* no named arguments */
  tuple = g_dbus_proxy_call_sync (nested_proxy, "AddServer",
                                  g_steal_pointer (&parameters),
                                  G_DBUS_CALL_FLAGS_NONE,
                                  -1, NULL, &f->error);

  g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED);
  g_assert_null (tuple);
  g_clear_error (&f->error);

  g_clear_object (&nested_proxy);

#else /* !HAVE_CONTAINERS_TEST */
  g_test_skip ("Containers or gio-unix-2.0 not supported");
#endif /* !HAVE_CONTAINERS_TEST */
}

/*
 * Assert that we can have up to 3 containers, but no more than that,
 * either because max-containers.conf imposes max_containers=3
 * or because limit-containers.conf imposes max_containers_per_user=3
 * (and we only have one uid).
 */
static void
test_max_containers (Fixture *f,
                     gconstpointer context)
{
#ifdef HAVE_CONTAINERS_TEST
  GVariant *parameters;
  GVariant *tuple;
  /* Length must match max_containers in max-containers.conf, and also
   * max_containers_per_user in limit-containers.conf */
  gchar *placeholders[3] = { NULL };
  guint i;

  if (f->skip)
    return;

  f->proxy = g_dbus_proxy_new_sync (f->unconfined_conn,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                    NULL, DBUS_SERVICE_DBUS,
                                    DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,
                                    NULL, &f->error);
  g_assert_no_error (f->error);

  parameters = g_variant_new ("(ssa{sv}a{sv})",
                              "com.example.NotFlatpak",
                              "sample-app",
                              NULL, /* no metadata */
                              NULL); /* no named arguments */
  /* We will reuse this variant several times, so don't use floating refs */
  g_variant_ref_sink (parameters);

  /* We can go up to the limit without exceeding it */
  for (i = 0; i < G_N_ELEMENTS (placeholders); i++)
    {
      tuple = g_dbus_proxy_call_sync (f->proxy, "AddServer",
                                      parameters, G_DBUS_CALL_FLAGS_NONE, -1,
                                      NULL, &f->error);
      g_assert_no_error (f->error);
      g_assert_nonnull (tuple);
      g_variant_get (tuple, "(o^ays)", &placeholders[i], NULL, NULL);
      g_variant_unref (tuple);
      g_test_message ("Placeholder server at %s", placeholders[i]);
    }

  /* We cannot exceed the limit */
  tuple = g_dbus_proxy_call_sync (f->proxy, "AddServer",
                                  parameters, G_DBUS_CALL_FLAGS_NONE, -1,
                                  NULL, &f->error);
  g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_LIMITS_EXCEEDED);
  g_clear_error (&f->error);
  g_assert_null (tuple);

  /* Stop one of the placeholders */
  tuple = g_dbus_proxy_call_sync (f->proxy, "StopListening",
                                  g_variant_new ("(o)", placeholders[1]),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                  &f->error);
  g_assert_no_error (f->error);
  g_assert_nonnull (tuple);
  g_variant_unref (tuple);

  /* We can have another container server now that we are back below the
   * limit */
  tuple = g_dbus_proxy_call_sync (f->proxy, "AddServer",
                                  parameters, G_DBUS_CALL_FLAGS_NONE, -1,
                                  NULL, &f->error);
  g_assert_no_error (f->error);
  g_assert_nonnull (tuple);
  g_variant_unref (tuple);

  g_variant_unref (parameters);

  for (i = 0; i < G_N_ELEMENTS (placeholders); i++)
    g_free (placeholders[i]);

#else /* !HAVE_CONTAINERS_TEST */
  g_test_skip ("Containers or gio-unix-2.0 not supported");
#endif /* !HAVE_CONTAINERS_TEST */
}

#ifdef HAVE_CONTAINERS_TEST
static void
assert_connection_closed (GError *error)
{
  /* "before 2.44 some "connection closed" errors returned
   * G_IO_ERROR_BROKEN_PIPE, but others returned G_IO_ERROR_FAILED"
   * —GIO documentation */
  if (error->code == G_IO_ERROR_BROKEN_PIPE)
    {
      g_assert_error (error, G_IO_ERROR, G_IO_ERROR_BROKEN_PIPE);
    }
  else
    {
      g_assert_error (error, G_IO_ERROR, G_IO_ERROR_FAILED);
      g_test_message ("Old GLib: %s", error->message);
      /* This is wrong and bad, but it's the only way to detect this, and
       * the older GLib versions that raised FAILED are no longer a moving
       * target */
      g_assert_true (strstr (error->message, g_strerror (ECONNRESET)) != NULL);
    }
}
#endif

/*
 * Test that if we have multiple app-containers,
 * max_connections_per_container applies to each one individually.
 */
static void
test_max_connections_per_container (Fixture *f,
                                    gconstpointer context)
{
#ifdef HAVE_CONTAINERS_TEST
  /* Length is arbitrary */
  gchar *socket_paths[2] = { NULL };
  gchar *dbus_addresses[G_N_ELEMENTS (socket_paths)] = { NULL };
  GSocketAddress *socket_addresses[G_N_ELEMENTS (socket_paths)] = { NULL };
  /* Length must be length of socket_paths * max_connections_per_container in
   * limit-containers.conf */
  GSocket *placeholders[G_N_ELEMENTS (socket_paths) * 3] = { NULL };
  GVariant *parameters;
  GVariant *tuple;
  guint i;

  if (f->skip)
    return;

  f->proxy = g_dbus_proxy_new_sync (f->unconfined_conn,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                    NULL, DBUS_SERVICE_DBUS,
                                    DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,
                                    NULL, &f->error);
  g_assert_no_error (f->error);

  parameters = g_variant_new ("(ssa{sv}a{sv})",
                              "com.example.NotFlatpak",
                              "sample-app",
                              NULL, /* no metadata */
                              NULL); /* no named arguments */
  /* We will reuse this variant several times, so don't use floating refs */
  g_variant_ref_sink (parameters);

  for (i = 0; i < G_N_ELEMENTS (socket_paths); i++)
    {
      tuple = g_dbus_proxy_call_sync (f->proxy, "AddServer",
                                      parameters, G_DBUS_CALL_FLAGS_NONE, -1,
                                      NULL, &f->error);
      g_assert_no_error (f->error);
      g_assert_nonnull (tuple);
      g_variant_get (tuple, "(o^ays)", NULL, &socket_paths[i],
                     &dbus_addresses[i]);
      g_variant_unref (tuple);
      socket_addresses[i] = g_unix_socket_address_new (socket_paths[i]);
      g_test_message ("Server #%u at %s", i, socket_paths[i]);
    }

  for (i = 0; i < G_N_ELEMENTS (placeholders); i++)
    {
      /* We enforce the resource limit for any connection to the socket,
       * not just D-Bus connections that have done the handshake */
      placeholders[i] = g_socket_new (G_SOCKET_FAMILY_UNIX,
                                      G_SOCKET_TYPE_STREAM,
                                      G_SOCKET_PROTOCOL_DEFAULT, &f->error);
      g_assert_no_error (f->error);

      g_socket_connect (placeholders[i],
                        socket_addresses[i % G_N_ELEMENTS (socket_paths)],
                        NULL, &f->error);
      g_assert_no_error (f->error);
      g_test_message ("Placeholder connection #%u to %s", i,
                      socket_paths[i % G_N_ELEMENTS (socket_paths)]);
    }

  /* An extra connection to either of the sockets fails: they are both at
   * capacity now */
  for (i = 0; i < G_N_ELEMENTS (socket_paths); i++)
    {
      f->confined_conn = g_dbus_connection_new_for_address_sync (
          dbus_addresses[i],
          (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
           G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
          NULL, NULL, &f->error);
      assert_connection_closed (f->error);

      g_clear_error (&f->error);
      g_assert_null (f->confined_conn);
    }

  /* Free up one slot (this happens to be connected to socket_paths[0]) */
  g_socket_close (placeholders[2], &f->error);
  g_assert_no_error (f->error);

  /* Now we can connect, but only once. Use a retry loop since the dbus-daemon
   * won't necessarily notice our socket closing synchronously. */
  while (f->confined_conn == NULL)
    {
      g_test_message ("Trying to use the slot we just freed up...");
      f->confined_conn = g_dbus_connection_new_for_address_sync (
          dbus_addresses[0],
          (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
           G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
          NULL, NULL, &f->error);

      if (f->confined_conn == NULL)
        {
          assert_connection_closed (f->error);
          g_clear_error (&f->error);
          g_assert_nonnull (f->confined_conn);
        }
      else
        {
          g_assert_no_error (f->error);
        }
    }

  /* An extra connection to either of the sockets fails: they are both at
   * capacity again */
  for (i = 0; i < G_N_ELEMENTS (socket_paths); i++)
    {
      GDBusConnection *another = g_dbus_connection_new_for_address_sync (
          dbus_addresses[i],
          (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
           G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
          NULL, NULL, &f->error);

      assert_connection_closed (f->error);
      g_clear_error (&f->error);
      g_assert_null (another);
    }

  g_variant_unref (parameters);

  for (i = 0; i < G_N_ELEMENTS (socket_paths); i++)
    {
      g_free (socket_paths[i]);
      g_free (dbus_addresses[i]);
      g_clear_object (&socket_addresses[i]);
    }

  for (i = 0; i < G_N_ELEMENTS (placeholders); i++)
    g_clear_object (&placeholders[i]);

#undef LIMIT
#else /* !HAVE_CONTAINERS_TEST */
  g_test_skip ("Containers or gio-unix-2.0 not supported");
#endif /* !HAVE_CONTAINERS_TEST */
}

/*
 * Test what happens when we exceed max_container_metadata_bytes.
 * test_metadata() exercises the non-excessive case with the same
 * configuration.
 */
static void
test_max_container_metadata_bytes (Fixture *f,
                                   gconstpointer context)
{
#ifdef HAVE_CONTAINERS_TEST
  /* Must be >= max_container_metadata_bytes in limit-containers.conf, so that
   * when the serialization overhead, app-container type and app name are
   * added, it is too much for the limit */
  guchar waste_of_space[4096] = { 0 };
  GVariant *tuple;
  GVariant *parameters;
  GVariantDict dict;

  if (f->skip)
    return;

  f->proxy = g_dbus_proxy_new_sync (f->unconfined_conn,
                                    G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                    NULL, DBUS_SERVICE_DBUS,
                                    DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,
                                    NULL, &f->error);
  g_assert_no_error (f->error);

  g_variant_dict_init (&dict, NULL);
  g_variant_dict_insert (&dict, "waste of space", "@ay",
                         g_variant_new_fixed_array (G_VARIANT_TYPE_BYTE,
                                                    waste_of_space,
                                                    sizeof (waste_of_space),
                                                    1));

  /* Floating reference, call_..._sync takes ownership */
  parameters = g_variant_new ("(ss@a{sv}a{sv})",
                              "com.wasteheadquarters",
                              "Packt Like Sardines in a Crushd Tin Box",
                              g_variant_dict_end (&dict),
                              NULL); /* no named arguments */

  tuple = g_dbus_proxy_call_sync (f->proxy, "AddServer", parameters,
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_error (f->error, G_DBUS_ERROR, G_DBUS_ERROR_LIMITS_EXCEEDED);
  g_assert_null (tuple);
  g_clear_error (&f->error);

#else /* !HAVE_CONTAINERS_TEST */
  g_test_skip ("Containers or gio-unix-2.0 not supported");
#endif /* !HAVE_CONTAINERS_TEST */
}

static void
teardown (Fixture *f,
    gconstpointer context G_GNUC_UNUSED)
{
  g_clear_object (&f->proxy);

  fixture_disconnect_observer (f);
  g_clear_pointer (&f->containers_removed, g_hash_table_unref);

  if (f->libdbus_observer != NULL)
    {
      dbus_connection_remove_filter (f->libdbus_observer,
                                     observe_shouting_cb, f);
      test_connection_shutdown (f->ctx, f->libdbus_observer);
      dbus_connection_close (f->libdbus_observer);
    }

  dbus_clear_connection (&f->libdbus_observer);

  fixture_disconnect_unconfined (f);

  if (f->confined_conn != NULL)
    {
      GError *error = NULL;

      g_dbus_connection_close_sync (f->confined_conn, NULL, &error);

      if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CLOSED))
        g_clear_error (&error);
      else
        g_assert_no_error (error);
    }

  g_clear_object (&f->confined_conn);

  if (f->daemon_pid != 0)
    {
      test_kill_pid (f->daemon_pid);
      g_spawn_close_pid (f->daemon_pid);
      f->daemon_pid = 0;
    }

  dbus_clear_message (&f->latest_shout);
  g_free (f->instance_path);
  g_free (f->socket_path);
  g_free (f->socket_dbus_address);
  g_free (f->bus_address);
  g_clear_error (&f->error);
  test_main_context_unref (f->ctx);
  g_free (f->unconfined_unique_name);
}

static const Config stop_server_explicitly =
{
  "valid-config-files/multi-user.conf",
  STOP_SERVER_EXPLICITLY
};
static const Config stop_server_disconnect_first =
{
  "valid-config-files/multi-user.conf",
  STOP_SERVER_DISCONNECT_FIRST
};
static const Config stop_server_never_connected =
{
  "valid-config-files/multi-user.conf",
  STOP_SERVER_NEVER_CONNECTED
};
static const Config stop_server_force =
{
  "valid-config-files/multi-user.conf",
  STOP_SERVER_FORCE
};
static const Config stop_server_with_manager =
{
  "valid-config-files/multi-user.conf",
  STOP_SERVER_WITH_MANAGER
};
static const Config limit_containers =
{
  "valid-config-files/limit-containers.conf",
  0 /* not relevant for this test */
};
static const Config max_containers =
{
  "valid-config-files/max-containers.conf",
  0 /* not relevant for this test */
};

int
main (int argc,
    char **argv)
{
  GError *error = NULL;
  gchar *runtime_dir;
  gchar *runtime_dbus_dir;
  gchar *runtime_containers_dir;
  gchar *runtime_services_dir;
  int ret;

  runtime_dir = g_dir_make_tmp ("dbus-test-containers.XXXXXX", &error);

  if (runtime_dir == NULL)
    {
      g_print ("Bail out! %s\n", error->message);
      g_clear_error (&error);
      return 1;
    }

  g_setenv ("XDG_RUNTIME_DIR", runtime_dir, TRUE);
  runtime_dbus_dir = g_build_filename (runtime_dir, "dbus-1", NULL);
  runtime_containers_dir = g_build_filename (runtime_dir, "dbus-1",
      "containers", NULL);
  runtime_services_dir = g_build_filename (runtime_dir, "dbus-1",
      "services", NULL);

  test_init (&argc, &argv);

  g_test_add ("/containers/get-supported-arguments", Fixture, NULL,
              setup, test_get_supported_arguments, teardown);
  g_test_add ("/containers/basic", Fixture, NULL,
              setup, test_basic, teardown);
  g_test_add ("/containers/wrong-uid", Fixture, NULL,
              setup, test_wrong_uid, teardown);
  g_test_add ("/containers/stop-server/explicitly", Fixture,
              &stop_server_explicitly, setup, test_stop_server, teardown);
  g_test_add ("/containers/stop-server/disconnect-first", Fixture,
              &stop_server_disconnect_first, setup, test_stop_server, teardown);
  g_test_add ("/containers/stop-server/never-connected", Fixture,
              &stop_server_never_connected, setup, test_stop_server, teardown);
  g_test_add ("/containers/stop-server/force", Fixture,
              &stop_server_force, setup, test_stop_server, teardown);
  g_test_add ("/containers/stop-server/with-manager", Fixture,
              &stop_server_with_manager, setup, test_stop_server, teardown);
  g_test_add ("/containers/metadata", Fixture, &limit_containers,
              setup, test_metadata, teardown);
  g_test_add ("/containers/invalid-metadata-getters", Fixture, NULL,
              setup, test_invalid_metadata_getters, teardown);
  g_test_add ("/containers/unsupported-parameter", Fixture, NULL,
              setup, test_unsupported_parameter, teardown);
  g_test_add ("/containers/invalid-type-name", Fixture, NULL,
              setup, test_invalid_type_name, teardown);
  g_test_add ("/containers/invalid-nesting", Fixture, NULL,
              setup, test_invalid_nesting, teardown);
  g_test_add ("/containers/max-containers", Fixture, &max_containers,
              setup, test_max_containers, teardown);
  g_test_add ("/containers/max-containers-per-user", Fixture, &limit_containers,
              setup, test_max_containers, teardown);
  g_test_add ("/containers/max-connections-per-container", Fixture,
              &limit_containers,
              setup, test_max_connections_per_container, teardown);
  g_test_add ("/containers/max-container-metadata-bytes", Fixture,
              &limit_containers,
              setup, test_max_container_metadata_bytes, teardown);

  ret = g_test_run ();

  test_rmdir_if_exists (runtime_containers_dir);
  test_rmdir_if_exists (runtime_services_dir);
  test_rmdir_if_exists (runtime_dbus_dir);
  test_rmdir_must_exist (runtime_dir);
  g_free (runtime_containers_dir);
  g_free (runtime_services_dir);
  g_free (runtime_dbus_dir);
  g_free (runtime_dir);
  dbus_shutdown ();
  return ret;
}
