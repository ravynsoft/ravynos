/* Regression test for atomic ops
 *
 * Author: Simon McVittie <simon.mcvittie@collabora.co.uk>
 * Copyright © 2013 Collabora Ltd.
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

#include <dbus/dbus.h>
#include <dbus/dbus-internals.h>
#include <dbus/dbus-sysdeps.h>
#include <dbus/dbus-test-tap.h>
#include <test/test-utils.h>

static dbus_bool_t
atomic_test_inc_dec (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusAtomic a = { 0 };
  DBusAtomic b = { 123 };
  dbus_int32_t old;

  _dbus_test_check (_dbus_atomic_get (&a) == 0);
  _dbus_test_check (_dbus_atomic_get (&b) == 123);

  _dbus_test_check (a.value == 0);
  old = _dbus_atomic_dec (&a);
  _dbus_test_check (old == 0);
  _dbus_test_check (a.value == -1);

  _dbus_test_check (b.value == 123);
  old = _dbus_atomic_inc (&b);
  _dbus_test_check (old == 123);
  _dbus_test_check (b.value == 124);
  return TRUE;
}

static dbus_bool_t
atomic_test_zero (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusAtomic a = { 0 };
  DBusAtomic b = { 123 };

  _dbus_atomic_set_nonzero (&a);
  /* careful: this is not necessarily 1 */
  _dbus_test_check (a.value != 0);

  _dbus_atomic_set_nonzero (&b);
  _dbus_test_check (b.value != 0);

  _dbus_atomic_set_zero (&a);
  _dbus_test_check (a.value == 0);

  _dbus_atomic_set_zero (&b);
  _dbus_test_check (b.value == 0);
  return TRUE;
}

static DBusTestCase tests[] =
{
  { "atomic_inc/dec", atomic_test_inc_dec },
  { "atomic_set_[non]zero", atomic_test_zero },
  { NULL }
};

int
main (int argc, char **argv)
{
  return _dbus_test_main (argc, argv, _DBUS_N_ELEMENTS (tests), tests,
                          DBUS_TEST_FLAGS_NONE,
                          NULL, NULL);
}
