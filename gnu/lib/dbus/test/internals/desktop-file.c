/*
 * Copyright 2018 Collabora Ltd.
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

/* The code we're testing isn't available in a convenience library yet
 * (see https://gitlab.freedesktop.org/dbus/dbus/merge_requests/1) and
 * older Automake versions don't like source files in other directories,
 * so we #include it. */
#include "bus/desktop-file.h"
#include "bus/desktop-file.c"
const char bus_no_memory_message[] = "no memory";

#include <glib.h>
#include <glib/gstdio.h>

#include "test-utils-glib.h"

typedef struct
{
  gchar *temp_file_name;
} Fixture;

typedef struct
{
  const char *title;
  const char *text;
  gssize length;
  const char *section;
  const char *key;
  const char *raw_value;
  const char *value;
} Test;

static const Test valid_content[] =
{
  { "newlines", "\n\n\n\n\n", -1 },
  { "spaces", "  ", -1 },
  { "normal",
    "[Foo]\n"
    "bar=baz\n",
    -1,
    "Foo",
    "bar",
    "baz",
    "baz" },
  { "empty value",
    "[Foo]\n"
    "bar=\n",
    -1,
    "Foo",
    "bar",
    "",
    "" },
  { "empty section",
    "[D-BUS Service]",
    -1,
    "D-BUS Service",
    "foo",
    NULL,
    NULL },
  { "empty section with newline",
    "[D-BUS Service]\n",
    -1 },
  { "odd whitespace",
    "\n\n    \n[D-BUS Service]\n    \n",
    -1 },
  { "Misc printable ASCII in section heading",
    "[abcxyzABCXYZ012789`!\"$%^&*()-_=+{}:;'@#~<,>./?|\\]",
    -1 },
  { "Backslash in section heading",
    /* Section name consists of a single backslash followed by literal n
     * (it is not a newline) */
    "[\\n]\n"
    "foo=bar",
    -1,
    "\\n",
    "foo",
    "bar",
    "bar" },
  { "empty", "", -1 }
};

static const Test invalid_content[] =
{
  { "unterminated section heading",
    "[D-BUS Service",
    -1 },
  { "newline in section heading",
    "[D-BUS Service\n]",
    -1 },
  { "tab in section heading",
    "[D-BUS\tService]",
    -1 },
  { "junk after section heading",
    "[Foo] banana",
    -1 },
  { "opening square bracket in section heading",
    "[Foo[]",
    -1 },
  { "closing square bracket in section heading",
    "[Foo]]",
    -1 },
  { "control character in section heading",
    "[Foo\001]",
    -1 },
  { "backspace in section heading",
    "[Foo\177]",
    -1 },
  { "NUL in section heading",
    "[Foo\000]",
    -1 },
  { "non-ASCII in section heading",
    "[Foo\xc2\xa3]",
    -1 },
  { "bare string not in section",
    "aaaa",
    -1 },
  { "key-value not in section",
    "foo=bar",
    -1 },
  { "contains control character",
    "[foo]\001",
    6 },
  { "contains nul",
    "[foo]\000",
    6 },
  { "empty section name",
    "[]",
    -1 }
};

static void
setup (Fixture *f,
       gconstpointer data)
{
  const Test *test = data;
  GError *error = NULL;
  gboolean ok;
  int fd;

  fd = g_file_open_tmp ("dbus-XXXXXX.desktop", &f->temp_file_name, &error);

  g_assert_no_error (error);
  g_assert_cmpint (fd, >=, 0);

  ok = g_close (fd, &error);
  g_assert_no_error (error);
  g_assert_true (ok);

  ok = g_file_set_contents (f->temp_file_name, test->text, test->length,
                            &error);
  g_assert_no_error (error);
  g_assert_true (ok);
}

/*
 * If @test specifies a section and key, check that it contains the
 * intended value.
 */
static void
test_content (const Test *test,
               BusDesktopFile *bdf)
{
  if (test->section != NULL)
    {
      const char *raw = NULL;
      char *val = NULL;
      dbus_bool_t ok;
      DBusError error = DBUS_ERROR_INIT;

      g_assert (test->key != NULL);

      ok = bus_desktop_file_get_raw (bdf, test->section, test->key, &raw);

      if (test->raw_value == NULL)
        {
          g_assert (test->value == NULL);
          g_assert_false (ok);
        }
      else
        {
          g_assert_true (ok);
          g_assert_cmpstr (raw, ==, test->raw_value);
        }

      ok = bus_desktop_file_get_string (bdf, test->section, test->key,
                                        &val, &error);

      if (test->value == NULL)
        {
          g_assert_nonnull (error.name);
          g_assert_nonnull (error.message);
          g_assert_false (ok);
          g_assert_null (val);
          dbus_error_free (&error);
        }
      else
        {
          test_assert_no_error (&error);
          g_assert_true (ok);
          g_assert_cmpstr (val, ==, test->value);
        }
      dbus_free (val);
    }
}

static void
test_valid (Fixture *f,
            gconstpointer data)
{
  const Test *test = data;
  BusDesktopFile *bdf;
  DBusString str;
  DBusError error = DBUS_ERROR_INIT;

  _dbus_string_init_const (&str, f->temp_file_name);

  bdf = bus_desktop_file_load (&str, &error);
  test_assert_no_error (&error);
  g_assert_nonnull (bdf);
  test_content (test, bdf);
  bus_desktop_file_free (bdf);

  /* Check that it's OK to ignore the error */
  bdf = bus_desktop_file_load (&str, NULL);
  g_assert_nonnull (bdf);
  test_content (test, bdf);
  bus_desktop_file_free (bdf);
}

static void
test_invalid (Fixture *f,
              gconstpointer data)
{
  BusDesktopFile *bdf;
  DBusString str;
  DBusError error = DBUS_ERROR_INIT;

  _dbus_string_init_const (&str, f->temp_file_name);

  bdf = bus_desktop_file_load (&str, &error);
  g_assert_nonnull (error.name);
  g_assert_nonnull (error.message);
  g_assert_null (bdf);
  dbus_error_free (&error);

  /* Check that it's OK to ignore the error */
  bdf = bus_desktop_file_load (&str, NULL);
  g_assert_null (bdf);
}

static void
teardown (Fixture *f,
          gconstpointer data)
{
  g_unlink (f->temp_file_name);
  g_free (f->temp_file_name);
}

int
main (int argc,
    char **argv)
{
  gsize i;
  int ret;

  test_init (&argc, &argv);

  for (i = 0; i < G_N_ELEMENTS (valid_content); i++)
    {
      gchar *title = g_strdup_printf ("/desktop-file/valid/%s",
                                      valid_content[i].title);

      g_test_add (title, Fixture, &valid_content[i],
                  setup, test_valid, teardown);
      g_free (title);
    }

  for (i = 0; i < G_N_ELEMENTS (invalid_content); i++)
    {
      gchar *title = g_strdup_printf ("/desktop-file/invalid/%s",
                                      invalid_content[i].title);

      g_test_add (title, Fixture, &invalid_content[i],
                  setup, test_invalid, teardown);
      g_free (title);
    }

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
