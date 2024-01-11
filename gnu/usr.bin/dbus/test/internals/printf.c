/* Regression test for _dbus_printf_string_upper_bound
 *
 * Author: Simon McVittie <simon.mcvittie@collabora.co.uk>
 * Copyright © 2013 Intel Corporation
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
#include <dbus/dbus-string.h>
#include "test-utils.h"

#include <stdio.h>
#include <stdlib.h>

static void do_test (int minimum,
                     const char *format,
                     ...) _DBUS_GNUC_PRINTF (2, 3);

static void
do_test (int minimum,
    const char *format,
    ...)
{
  va_list ap;
  int result;

  va_start (ap, format);
  result = _dbus_printf_string_upper_bound (format, ap);
  va_end (ap);

  if (result < minimum)
    {
      fprintf (stderr, "expected at least %d, got %d\n", minimum, result);
      abort ();
    }
}

#define X_TIMES_8 "XXXXXXXX"
#define X_TIMES_16   X_TIMES_8   X_TIMES_8
#define X_TIMES_32   X_TIMES_16  X_TIMES_16
#define X_TIMES_64   X_TIMES_32  X_TIMES_32
#define X_TIMES_128  X_TIMES_64  X_TIMES_64
#define X_TIMES_256  X_TIMES_128 X_TIMES_128
#define X_TIMES_512  X_TIMES_256 X_TIMES_256
#define X_TIMES_1024 X_TIMES_512 X_TIMES_512

/* This test outputs TAP syntax: http://testanything.org/ */
int
main (int argc,
    char **argv)
{
  char buf[] = X_TIMES_1024 X_TIMES_1024 X_TIMES_1024 X_TIMES_1024;
  int i;
  int test_num = 0;

  do_test (1, "%d", 0);
  printf ("ok %d\n", ++test_num);

  do_test (7, "%d", 1234567);
  printf ("ok %d\n", ++test_num);

  do_test (3, "%f", 3.5);
  printf ("ok %d\n", ++test_num);

  do_test (0, "%s", "");
  printf ("ok %d\n", ++test_num);

  do_test (1024, "%s", X_TIMES_1024);
  printf ("ok %d\n", ++test_num);

  do_test (1025, "%s", X_TIMES_1024 "Y");
  printf ("ok %d\n", ++test_num);

  for (i = 4096; i > 0; i--)
    {
      buf[i] = '\0';
      do_test (i, "%s", buf);
      do_test (i + 3, "%s:%d", buf, 42);
    }
  printf ("ok %d\n", ++test_num);

  /* Tell the TAP driver that we have done all the tests we plan to do.
   * This is how it can distinguish between an unexpected exit and
   * successful completion. */
  printf ("1..%d\n", test_num);
  dbus_shutdown ();
  return 0;
}
