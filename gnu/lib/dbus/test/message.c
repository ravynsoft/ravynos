/* Targeted unit tests for OOM paths in DBusMessage
 *
 * Copyright © 2017 Collabora Ltd.
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

#include <glib.h>
#include <glib/gstdio.h>

#include <dbus/dbus.h>
#include "dbus/dbus-internals.h"
#include "dbus/dbus-message-internal.h"
#include "dbus/dbus-pipe.h"
#include "test-utils-glib.h"

/* Return TRUE if the right thing happens, but the right thing might include
 * OOM. */
static dbus_bool_t
test_array (void        *contained_signature,
            dbus_bool_t  have_memory)
{
  DBusMessage *m;
  DBusMessageIter iter;
  DBusMessageIter arr_iter;
  dbus_bool_t arr_iter_open = FALSE;
  DBusMessageIter inner_iter;
  dbus_bool_t inner_iter_open = FALSE;

  m = dbus_message_new_signal ("/", "a.b", "c");

  if (m == NULL)
    goto out;

  dbus_message_iter_init_append (m, &iter);

  /* open_container only opens the container if it succeeds */
  if (!dbus_message_iter_open_container (&iter, DBUS_TYPE_ARRAY,
                                         contained_signature,
                                         &arr_iter))
    goto out;

  arr_iter_open = TRUE;

  if (g_strcmp0 (contained_signature, "ai") == 0)
    {
      /* open_container only opens the container if it succeeds */
      if (!dbus_message_iter_open_container (&arr_iter, DBUS_TYPE_ARRAY, "i",
                                             &inner_iter))
        goto out;

      /* We do not set inner_iter_open to TRUE here because we would
       * immediately set it to FALSE again */

      /* close_container closes the container, even when it fails */
      if (!dbus_message_iter_close_container (&arr_iter, &inner_iter))
        goto out;
    }
  else if (g_strcmp0 (contained_signature, "{ss}") == 0)
    {
      const char *s = "hello";

      /* open_container only opens the container if it succeeds */
      if (!dbus_message_iter_open_container (&arr_iter, DBUS_TYPE_DICT_ENTRY,
                                             NULL, &inner_iter))
        goto out;

      inner_iter_open = TRUE;

      if (!dbus_message_iter_append_basic (&inner_iter, DBUS_TYPE_STRING, &s))
        goto out;

      if (!dbus_message_iter_append_basic (&inner_iter, DBUS_TYPE_STRING, &s))
        goto out;

      /* close_container closes the container, even when it fails */
      inner_iter_open = FALSE;

      if (!dbus_message_iter_close_container (&arr_iter, &inner_iter))
        goto out;
    }
  else if (g_strcmp0 (contained_signature, "v") == 0)
    {
      dbus_bool_t yes = TRUE;

      /* open_container only opens the container if it succeeds */
      if (!dbus_message_iter_open_container (&arr_iter, DBUS_TYPE_VARIANT,
                                             "b", &inner_iter))
        goto out;

      inner_iter_open = TRUE;

      if (!dbus_message_iter_append_basic (&inner_iter, DBUS_TYPE_BOOLEAN,
                                           &yes))
        goto out;

      /* close_container closes the container, even when it fails */
      inner_iter_open = FALSE;

      if (!dbus_message_iter_close_container (&arr_iter, &inner_iter))
        goto out;
    }
  else
    {
      g_assert_not_reached ();
    }

  /* close_container closes the container, even when it fails */
  arr_iter_open = FALSE;

  if (!dbus_message_iter_close_container (&iter, &arr_iter))
    goto out;

out:
  if (inner_iter_open)
    dbus_message_iter_abandon_container (&arr_iter, &inner_iter);

  if (arr_iter_open)
    dbus_message_iter_abandon_container (&iter, &arr_iter);

  if (m != NULL)
    dbus_message_unref (m);

  dbus_shutdown ();
  g_assert_cmpint (_dbus_get_malloc_blocks_outstanding (), ==, 0);

  return !g_test_failed ();
}

/* Return TRUE if the right thing happens, but the right thing might include
 * OOM or inability to pass fds. */
static dbus_bool_t
test_fd (void        *ignored,
         dbus_bool_t  have_memory)
{
  DBusMessage *m = NULL;
  DBusPipe pipe;

  _dbus_pipe_init_stdout (&pipe);

  m = dbus_message_new_signal ("/", "a.b", "c");

  if (m == NULL)
    goto out;

  if (!dbus_message_append_args (m,
                                 DBUS_TYPE_UNIX_FD, &pipe.fd,
                                 DBUS_TYPE_INVALID))
    goto out;

out:
  if (m != NULL)
    dbus_message_unref (m);

  dbus_shutdown ();
  g_assert_cmpint (_dbus_get_malloc_blocks_outstanding (), ==, 0);

  return !g_test_failed ();
}

static void iterate_fully (DBusMessageIter *iter,
                           int              n_elements);

/* Iterate over @iter. If n_elements >= 0, then @iter is
 * expected to yield exactly @n_elements elements. */
static void
iterate_fully (DBusMessageIter *iter,
               int              n_elements)
{
  int i = 0;

  while (TRUE)
    {
      int arg_type = dbus_message_iter_get_arg_type (iter);
      dbus_bool_t should_have_next;
      dbus_bool_t had_next;

      if (arg_type == DBUS_TYPE_INVALID)
        return;   /* end of iteration */

      if (dbus_type_is_container (arg_type))
        {
          DBusMessageIter sub = DBUS_MESSAGE_ITER_INIT_CLOSED;
          int n_contained = -1;

          switch (arg_type)
            {
              case DBUS_TYPE_ARRAY:
                /* This is only allowed for arrays */
                n_contained = dbus_message_iter_get_element_count (iter);
                g_assert_cmpint (n_contained, >=, 0);
                break;

              case DBUS_TYPE_VARIANT:
                n_contained = 1;
                break;

              case DBUS_TYPE_STRUCT:
                break;

              case DBUS_TYPE_DICT_ENTRY:
                n_contained = 2;
                break;

              default:
                g_assert_not_reached ();
            }

          dbus_message_iter_recurse (iter, &sub);
          iterate_fully (&sub, n_contained);
        }
      else
        {
          DBusBasicValue value;

          dbus_message_iter_get_basic (iter, &value);

          if (arg_type == DBUS_TYPE_UNIX_FD && value.fd >= 0)
            {
              GError *error = NULL;

              g_close (value.fd, &error);
              g_assert_no_error (error);
            }
        }

      should_have_next = dbus_message_iter_has_next (iter);
      had_next = dbus_message_iter_next (iter);
      g_assert_cmpint (had_next, ==, should_have_next);
      g_assert_cmpint (had_next, ==,
                       (dbus_message_iter_get_arg_type (iter) != DBUS_TYPE_INVALID));
      i += 1;
    }

  if (n_elements >= 0)
    g_assert_cmpuint (n_elements, ==, i);
}

/* Return TRUE if the right thing happens, but the right thing might include
 * OOM. */
static dbus_bool_t
test_valid_message_blobs (void        *message_name,
                          dbus_bool_t  have_memory)
{
  gchar *path = NULL;
  gchar *contents = NULL;
  gsize len = 0;
  DBusMessage *m = NULL;
  DBusMessageIter iter = DBUS_MESSAGE_ITER_INIT_CLOSED;
  GError *error = NULL;
  DBusError e = DBUS_ERROR_INIT;
  dbus_bool_t ok = TRUE;
  gchar *filename = NULL;

  filename = g_strdup_printf ("%s.message-raw", (const char *) message_name);
  path = g_test_build_filename (G_TEST_DIST, "data", "valid-messages",
                                filename, NULL);
  g_file_get_contents (path, &contents, &len, &error);
  g_assert_no_error (error);
  g_assert_cmpuint (len, <, (gsize) INT_MAX);

  m = dbus_message_demarshal (contents, (int) len, &e);

  if (m == NULL)
    {
      if (dbus_error_has_name (&e, DBUS_ERROR_NO_MEMORY) && !have_memory)
        {
          g_test_message ("Out of memory (not a problem)");
          goto out;
        }

      g_test_message ("Parsing %s reported unexpected error %s: %s",
                      path, e.name, e.message);
      g_test_fail ();
      ok = FALSE;
      goto out;
    }

  g_test_message ("Successfully parsed %s", path);
  test_assert_no_error (&e);

  if (dbus_message_iter_init (m, &iter))
    g_assert_cmpint (dbus_message_iter_get_arg_type (&iter), !=, DBUS_TYPE_INVALID);
  else
    g_assert_cmpint (dbus_message_iter_get_arg_type (&iter), ==, DBUS_TYPE_INVALID);

  iterate_fully (&iter, -1);

out:
  dbus_clear_message (&m);
  dbus_error_free (&e);
  g_free (path);
  g_free (contents);
  g_free (filename);
  return ok;
}

/* Return TRUE if the right thing happens, but the right thing might include
 * OOM. */
static dbus_bool_t
test_invalid_message_blobs (void        *message_name,
                            dbus_bool_t  have_memory)
{
  gchar *path = NULL;
  gchar *contents = NULL;
  gsize len = 0;
  DBusMessage *m = NULL;
  GError *error = NULL;
  DBusError e = DBUS_ERROR_INIT;
  dbus_bool_t ok = TRUE;
  gchar *filename = NULL;

  filename = g_strdup_printf ("%s.message-raw", (const char *) message_name);
  path = g_test_build_filename (G_TEST_DIST, "data", "invalid-messages",
                                filename, NULL);
  g_file_get_contents (path, &contents, &len, &error);
  g_assert_no_error (error);
  g_assert_cmpuint (len, <, (gsize) INT_MAX);

  m = dbus_message_demarshal (contents, (int) len, &e);

  if (m != NULL)
    {
      g_test_message ("Parsing %s reported that it was valid", path);
      g_test_fail ();
      ok = FALSE;

      /* Attempt to reproduce dbus#413 */
      _dbus_message_remove_unknown_fields (m);

      goto out;
    }

  if (dbus_error_has_name (&e, DBUS_ERROR_NO_MEMORY) && !have_memory)
    {
      g_test_message ("Out of memory (not a problem)");
      goto out;
    }

  if (dbus_error_has_name (&e, DBUS_ERROR_INVALID_ARGS))
    {
      g_test_message ("Parsing %s reported error as expected: %s: %s",
                      path, e.name, e.message);
      goto out;
    }

  g_test_message ("Parsing %s reported unexpected error %s: %s",
                  path, e.name, e.message);
  g_test_fail ();
  ok = FALSE;

out:
  dbus_clear_message (&m);
  dbus_error_free (&e);
  g_free (path);
  g_free (contents);
  g_free (filename);
  return ok;
}

/* Similar to test_array(), but making use of
 * dbus_message_iter_abandon_container_if_open().
 *
 * Return TRUE if the right thing happens, but the right thing might include
 * OOM. */
static dbus_bool_t
test_zero_iter (void        *ignored,
                dbus_bool_t  have_memory)
{
  DBusMessage *m;
  DBusMessageIter iter = DBUS_MESSAGE_ITER_INIT_CLOSED;
  DBusMessageIter arr_iter = DBUS_MESSAGE_ITER_INIT_CLOSED;
  DBusMessageIter inner_iter;
  dbus_int32_t fortytwo = 42;
  dbus_bool_t message_should_be_complete = FALSE;

  /* This one was left uninitialized, just so we could exercise this
   * function */
  dbus_message_iter_init_closed (&inner_iter);

  m = dbus_message_new_signal ("/", "a.b", "c");

  if (m == NULL)
    goto out;

  dbus_message_iter_init_append (m, &iter);

  if (!dbus_message_iter_open_container (&iter, DBUS_TYPE_ARRAY,
                                         "ai", &arr_iter))
    goto out;

  if (!dbus_message_iter_open_container (&arr_iter, DBUS_TYPE_ARRAY, "i",
                                         &inner_iter))
    goto out;

  if (!dbus_message_iter_append_basic (&inner_iter, DBUS_TYPE_INT32, &fortytwo))
    goto out;

  if (!dbus_message_iter_close_container (&arr_iter, &inner_iter))
    goto out;

  if (!dbus_message_iter_close_container (&iter, &arr_iter))
    goto out;

  message_should_be_complete = TRUE;

out:
  dbus_message_iter_abandon_container_if_open (&arr_iter, &inner_iter);
  dbus_message_iter_abandon_container_if_open (&iter, &arr_iter);
  /* Redundant calls are OK */
  dbus_message_iter_abandon_container_if_open (&iter, &arr_iter);

  /* dbus_message_iter_abandon_container_if_open does not leave the message
   * in what seems to be consistently documented as a "hosed" state */
  if (message_should_be_complete)
    {
      DBusBasicValue read_back;

      _DBUS_ZERO (read_back);
      dbus_message_iter_init (m, &iter);
      dbus_message_iter_recurse (&iter, &arr_iter);
      dbus_message_iter_recurse (&arr_iter, &inner_iter);
      dbus_message_iter_get_basic (&inner_iter, &read_back);
      g_assert_cmpint (read_back.i32, ==, 42);
    }

  if (m != NULL)
    dbus_message_unref (m);

  dbus_shutdown ();
  g_assert_cmpint (_dbus_get_malloc_blocks_outstanding (), ==, 0);

  return !g_test_failed ();
}

typedef struct
{
  gchar *name;
  DBusTestMemoryFunction function;
  const void *data;
} OOMTestCase;

static void
oom_test_case_free (gpointer data)
{
  OOMTestCase *test = data;

  g_free (test->name);
  g_free (test);
}

static void
test_oom_wrapper (gconstpointer data)
{
  const OOMTestCase *test = data;

  if (!_dbus_test_oom_handling (test->name, test->function,
                                (void *) test->data))
    {
      g_test_message ("OOM test failed");
      g_test_fail ();
    }
}

static GQueue *test_cases_to_free = NULL;

static void
add_oom_test (const gchar *name,
              DBusTestMemoryFunction function,
              const void *data)
{
  /* By using GLib memory allocation here, we avoid being affected by
   * dbus_shutdown() or contributing to
   * _dbus_get_malloc_blocks_outstanding() */
  OOMTestCase *test_case = g_new0 (OOMTestCase, 1);

  test_case->name = g_strdup (name);
  test_case->function = function;
  test_case->data = data;
  g_test_add_data_func (name, test_case, test_oom_wrapper);
  g_queue_push_tail (test_cases_to_free, test_case);
}

static const char *valid_messages[] =
{
  "byteswap-fd-index",
  "minimal",
};

static const char *invalid_messages[] =
{
  "boolean-has-no-value",
  "fixed-array-not-divisible",
  "issue418",
  "mis-nested-sig",
  "truncated-variant-sig",
  "zero-length-variant-sig",
};

int
main (int argc,
      char **argv)
{
  int ret;
  gsize i;

  test_init (&argc, &argv);

  test_cases_to_free = g_queue_new ();
  add_oom_test ("/message/array/array", test_array, "ai");
  add_oom_test ("/message/array/dict", test_array, "{ss}");
  add_oom_test ("/message/array/variant", test_array, "v");
  add_oom_test ("/message/fd", test_fd, NULL);
  add_oom_test ("/message/zero-iter", test_zero_iter, NULL);

  for (i = 0; i < G_N_ELEMENTS (valid_messages); i++)
    {
      gchar *path = g_strdup_printf ("/message/valid/%s", valid_messages[i]);

      add_oom_test (path, test_valid_message_blobs, valid_messages[i]);
      g_free (path);
    }

  for (i = 0; i < G_N_ELEMENTS (invalid_messages); i++)
    {
      gchar *path = g_strdup_printf ("/message/invalid/%s", invalid_messages[i]);

      add_oom_test (path, test_invalid_message_blobs, invalid_messages[i]);
      g_free (path);
    }

  ret = g_test_run ();

  g_queue_free_full (test_cases_to_free, oom_test_case_free);
  dbus_shutdown ();
  return ret;
}
