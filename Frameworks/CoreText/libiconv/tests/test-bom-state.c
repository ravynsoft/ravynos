/* Copyright (C) 2024 Free Software Foundation, Inc.
   This file is part of the GNU LIBICONV Library.

   The GNU LIBICONV Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.

   The GNU LIBICONV Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU LIBICONV Library; see the file COPYING.LIB.
   If not, see <https://www.gnu.org/licenses/>.  */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>

/* This test checks that iconv(cd,NULL,NULL,...) does not forget about
   the byte-order state in conversions from UCS-2, UCS-4, UTF-16, UTF-32.

   The POSIX specification
   <https://pubs.opengroup.org/onlinepubs/9799919799/functions/iconv.html>
   is clear that iconv(cd,NULL,NULL,...) has an effect for state-dependent
   encodings only. The manual page
   <https://www.kernel.org/doc/man-pages/online/pages/man3/iconv.3.html>
   is not so clear about it. But Ulrich Drepper states it correctly in
   <https://bugzilla.redhat.com/show_bug.cgi?id=165368>:
     "Flushing using iconv() only resets the shift state.  This is needed
      for stateful encodings with states where the caller wants a converted
      string to end in the initial state.  The BOM recognition has nothing
      to do with shift states.  Once the byte order is determined this is
      a property which stays with the iconv_t descriptor for its lifetime."

   Based on a bug report from Tomas Kalibera <tomas.kalibera@gmail.com> in
   <https://lists.gnu.org/archive/html/bug-gnu-libiconv/2024-12/msg00000.html>.
 */

static void test_one_input (const char *fromcode,
                            const char *input, size_t input_size)
{
  char outbuf1[3];
  char outbuf2[3];

  iconv_t cd = iconv_open ("UTF-8", fromcode);
  if (cd == (iconv_t)(-1))
    abort ();

  /* Convert the first character.  */
  char *inbuf = (char *) input;
  size_t inbytesleft = input_size;
  char *outbuf = outbuf1;
  size_t outbytesleft = sizeof (outbuf1);
  size_t ret = iconv (cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  if (!(ret == (size_t)(-1) && errno == E2BIG && outbytesleft == 0))
    abort ();
  if (!(memcmp (outbuf1, "\xe2\x94\xa6", 3) == 0)) /* should be U+2526 */
    abort ();

  /* Reset the shift state.  */
  ret = iconv (cd, NULL, NULL, NULL, NULL);
  if (!(ret == 0))
    abort ();

  /* Convert the second character.  */
  outbuf = outbuf2;
  outbytesleft = sizeof (outbuf2);
  ret = iconv (cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  if (!(ret == 0 && outbytesleft == 0))
    abort ();
  if (!(memcmp (outbuf2, "\xe2\x98\xa9", 3) == 0)) /* should be U+2629 */
    abort ();

  iconv_close (cd);
}

static void test_both_inputs (const char *fromcode,
                              const char *be_input, const char *le_input,
                              size_t input_size)
{
  test_one_input (fromcode, be_input, input_size);
  test_one_input (fromcode, le_input, input_size);
}

int main ()
{
  {
    static const char be_input[] = "\xfe\xff\x25\x26\x26\x29";
    static const char le_input[] = "\xff\xfe\x26\x25\x29\x26";
    #ifdef _LIBICONV_VERSION
    test_both_inputs ("UCS-2", be_input, le_input, 6);
    #endif
    test_both_inputs ("UTF-16", be_input, le_input, 6);
  }
  {
    static const char be_input[] =
      "\x00\x00\xfe\xff\x00\x00\x25\x26\x00\x00\x26\x29";
    static const char le_input[] =
      "\xff\xfe\x00\x00\x26\x25\x00\x00\x29\x26\x00\x00";
    #ifdef _LIBICONV_VERSION
    test_both_inputs ("UCS-4", be_input, le_input, 12);
    #endif
    test_both_inputs ("UTF-32", be_input, le_input, 12);
  }
  return 0;
}
