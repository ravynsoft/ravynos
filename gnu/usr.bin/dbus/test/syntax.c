/* Simple sanity-check for D-Bus syntax validation.
 *
 * Author: Simon McVittie <simon.mcvittie@collabora.co.uk>
 * Copyright © 2010-2011 Nokia Corporation
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

#include <glib.h>

#include <dbus/dbus.h>

#include "test-utils-glib.h"

typedef struct {
    DBusError e;
} Fixture;

typedef struct {
    dbus_bool_t (*function) (const char *, DBusError *);
    const char * const * valid;
    const char * const * invalid;
} Test;

Test paths, interfaces, members, errors, bus_names, signatures,
     single_signatures, strings;

const char * const valid_paths[] = {
    "/",
    "/a",
    "/_",
    "/a/b/c",
    "/com/example/123",
    "/org/freedesktop/DBus",
    "/org/freedesktop/Telepathy/AccountManager",
    NULL
};

const char * const invalid_paths[] = {
    "",
    ".",
    "//",
    "/a/",
    "/-",
    "/com//example/MyApp",
    "/$",
    "/\xa9",      /* © in latin-1 */
    "/\xc2\xa9",  /* © in UTF-8 */
    NULL
};

const char * const valid_interfaces[] = {
    "com.example",
    "com.example.a0",
    "org.freedesktop.DBus",
    NULL
};

const char * const invalid_interfaces[] = {
   "",
    "com",
    "com.example.",
    "com.example..a0",
    "com.example.0a",
    "com.example.a$",
    "com.example.a\xa9",
    "com.example.a\xc2\xa9",
    NULL
};

const char * const valid_members[] = {
    "_",
    "a",
    "a0",
    "GetAll",
    "BadgerMushroomSnake",
    NULL
};

const char * const invalid_members[] = {
    "",
    "-",
    "a-",
    "0",
    "0_",
    "Badger.Mushroom",
    "a$",
    "a\xa9",
    "a\xc2\xa9",
    NULL
};

const char * const valid_errors[] = {
    "com.example",
    "com.example.a0",
    "org.freedesktop.DBus.NameHasNoOwner",
    NULL
};

const char * const invalid_errors[] = {
   "",
    "com",
    "com.example.",
    "com.example..a0",
    "com.example.0a",
    "com.example.a$",
    "com.example.a\xa9",
    "com.example.a\xc2\xa9",
    NULL
};

const char * const valid_bus_names[] = {
    "com.example",
    "com.example.a0",
    "com.example._",
    ":1.42",
    ":1.2.3.4.5",
    ":com.example",
    "org.freedesktop.DBus",
    NULL
};

const char * const invalid_bus_names[] = {
   "",
    "com",
    "com.example.",
    "com.example..a0",
    "com.example.0a",
    "com.example.a:b",
    "com.example.a\xa9",
    "com.example.a\xc2\xa9",
    NULL
};

const char * const valid_signatures[] = {
    "",
    "a{sv}",
    "a{s(i)}",
    "a(sa{ii})",
    NULL
};

const char * const invalid_signatures[] = {
    "a",
    "a{s_}",
    "a{s(i}",
    "a{s(i})",
    "a{s(i)",
    "a{s(i})",
    "a(sa{ii)",
    "a(sa{ii)}",
    ")",
    "}",
    NULL
};

const char * const valid_single_signatures[] = {
    "s",
    "a{sv}",
    NULL
};

const char * const invalid_single_signatures[] = {
    "",
    "a",
    "sv",
    "a{sv}as",
    NULL
};

const char * const valid_strings[] = {
    "",
    "\xc2\xa9",       /* UTF-8 (c) symbol */
    "\xef\xbf\xbe",   /* U+FFFE is reserved but Corrigendum 9 says it's OK */
    NULL
};

const char * const invalid_strings[] = {
    "\xa9",           /* Latin-1 (c) symbol */
    "\xed\xa0\x80",   /* UTF-16 surrogates are not valid in UTF-8 */
    NULL
};

static void
setup (Fixture *f,
    gconstpointer arg G_GNUC_UNUSED)
{
  dbus_error_init (&f->e);

#define FILL_TEST(name, func) \
  do { \
    (name).function = (func); \
    (name).valid = valid_ ## name; \
    (name).invalid = invalid_ ## name; \
  } while (0)

  FILL_TEST (paths, dbus_validate_path);
  FILL_TEST (interfaces, dbus_validate_interface);
  FILL_TEST (members, dbus_validate_member);
  FILL_TEST (errors, dbus_validate_error_name);
  FILL_TEST (bus_names, dbus_validate_bus_name);
  FILL_TEST (signatures, dbus_signature_validate);
  FILL_TEST (single_signatures, dbus_signature_validate_single);
  FILL_TEST (strings, dbus_validate_utf8);
}

static void
test_syntax (Fixture *f,
    gconstpointer arg)
{
  const Test *test = arg;
  int i;

  g_assert (test != NULL);
  g_assert (test->function != NULL);
  g_assert (test->valid != NULL);
  g_assert (test->invalid != NULL);

  for (i = 0; test->valid[i] != NULL; i++)
    {
      dbus_bool_t ok = test->function (test->valid[i], &f->e);

      if (dbus_error_is_set (&f->e))
        g_error ("%s was considered invalid: %s: %s", test->valid[i],
            f->e.name, f->e.message);

      if (!ok)
        g_error ("%s was considered invalid without an error", test->valid[i]);
    }

  for (i = 0; test->invalid[i] != NULL; i++)
    {
      dbus_bool_t ok = test->function (test->invalid[i], &f->e);

      if (ok)
        g_error ("%s should have been considered invalid", test->invalid[i]);

      if (!dbus_error_is_set (&f->e))
        g_error ("%s should have an error set", test->invalid[i]);

      if (!dbus_validate_error_name (f->e.name, NULL))
        g_error ("%s produced an invalid error name: %s",
            test->invalid[i], f->e.name);

      if (!dbus_validate_utf8 (f->e.message, NULL))
        g_error ("%s produced an invalid error message: %s",
            test->invalid[i], f->e.message);

      dbus_error_free (&f->e);
    }
}

static void
teardown (Fixture *f,
    gconstpointer arg G_GNUC_UNUSED)
{
  dbus_error_free (&f->e);
}

int
main (int argc,
    char **argv)
{
  int ret;

  test_init (&argc, &argv);

  g_test_add ("/syntax/path", Fixture, &paths, setup, test_syntax, teardown);
  g_test_add ("/syntax/interface", Fixture, &interfaces,
      setup, test_syntax, teardown);
  g_test_add ("/syntax/error", Fixture, &errors,
      setup, test_syntax, teardown);
  g_test_add ("/syntax/member", Fixture, &members,
      setup, test_syntax, teardown);
  g_test_add ("/syntax/bus-name", Fixture, &bus_names,
      setup, test_syntax, teardown);
  g_test_add ("/syntax/signature", Fixture, &signatures,
      setup, test_syntax, teardown);
  g_test_add ("/syntax/single-signature", Fixture, &single_signatures,
      setup, test_syntax, teardown);
  g_test_add ("/syntax/utf8", Fixture, &strings,
      setup, test_syntax, teardown);

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
