/* Test of character set conversion.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

#if HAVE_ICONV
# include <iconv.h>

# ifndef ICONV_CONST
#  define ICONV_CONST /* empty */
# endif

#include "signature.h"
SIGNATURE_CHECK (iconv, size_t, (iconv_t, ICONV_CONST char **, size_t *,
                                 char **, size_t *));
SIGNATURE_CHECK (iconv_close, int, (iconv_t x));
SIGNATURE_CHECK (iconv_open, iconv_t, (char const *, char const *));

#endif

#include <errno.h>
#include <string.h>

#include "macros.h"

int
main ()
{
#if HAVE_ICONV
  /* Assume that iconv() supports at least the encodings ASCII, ISO-8859-1,
     and UTF-8.  */
  iconv_t cd_88591_to_utf8 = iconv_open ("UTF-8", "ISO-8859-1");
  iconv_t cd_utf8_to_88591 = iconv_open ("ISO-8859-1", "UTF-8");

#if defined __MVS__ && defined __IBMC__
  /* String literals below are in ASCII, not EBCDIC.  */
# pragma convert("ISO8859-1")
# define CONVERT_ENABLED
#endif

  ASSERT (cd_88591_to_utf8 != (iconv_t)(-1));
  ASSERT (cd_utf8_to_88591 != (iconv_t)(-1));

  /* Test conversion from ISO-8859-1 to UTF-8 with no errors.  */
  {
    static const char input[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
    static const char expected[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
    char buf[50];
    const char *inptr = input;
    size_t inbytesleft = strlen (input);
    char *outptr = buf;
    size_t outbytesleft = sizeof (buf);
    size_t res = iconv (cd_88591_to_utf8,
                        (ICONV_CONST char **) &inptr, &inbytesleft,
                        &outptr, &outbytesleft);
    ASSERT (res == 0 && inbytesleft == 0);
    ASSERT (outptr == buf + strlen (expected));
    ASSERT (memcmp (buf, expected, strlen (expected)) == 0);
  }

  /* Test conversion from ISO-8859-1 to UTF-8 with E2BIG.  */
  {
    static const char input[] = "\304";
    static char buf[2] = { (char)0xDE, (char)0xAD };
    const char *inptr = input;
    size_t inbytesleft = 1;
    char *outptr = buf;
    size_t outbytesleft = 1;
    size_t res = iconv (cd_88591_to_utf8,
                        (ICONV_CONST char **) &inptr, &inbytesleft,
                        &outptr, &outbytesleft);
    ASSERT (res == (size_t)(-1) && errno == E2BIG);
    ASSERT (inbytesleft == 1);
    ASSERT (outbytesleft == 1);
    ASSERT ((unsigned char) buf[1] == 0xAD);
    ASSERT ((unsigned char) buf[0] == 0xDE);
  }

  /* Test conversion from UTF-8 to ISO-8859-1 with no errors.  */
  {
    static const char input[] = "\303\204rger mit b\303\266sen B\303\274bchen ohne Augenma\303\237";
    static const char expected[] = "\304rger mit b\366sen B\374bchen ohne Augenma\337";
    char buf[50];
    const char *inptr = input;
    size_t inbytesleft = strlen (input);
    char *outptr = buf;
    size_t outbytesleft = sizeof (buf);
    size_t res = iconv (cd_utf8_to_88591,
                        (ICONV_CONST char **) &inptr, &inbytesleft,
                        &outptr, &outbytesleft);
    ASSERT (res == 0 && inbytesleft == 0);
    ASSERT (outptr == buf + strlen (expected));
    ASSERT (memcmp (buf, expected, strlen (expected)) == 0);
  }

  /* Test conversion from UTF-8 to ISO-8859-1 with EILSEQ.  */
  {
    static const char input[] = "\342\202\254"; /* EURO SIGN */
    char buf[10];
    const char *inptr = input;
    size_t inbytesleft = strlen (input);
    char *outptr = buf;
    size_t outbytesleft = sizeof (buf);
    size_t res = iconv (cd_utf8_to_88591,
                        (ICONV_CONST char **) &inptr, &inbytesleft,
                        &outptr, &outbytesleft);
    if (res == (size_t)(-1))
      {
        ASSERT (errno == EILSEQ);
        ASSERT (inbytesleft == strlen (input) && outptr == buf);
      }
    else
      {
        ASSERT (res == 1);
        ASSERT (inbytesleft == 0);
      }
  }

  /* Test conversion from UTF-8 to ISO-8859-1 with EINVAL.  */
  {
    static const char input[] = "\342";
    char buf[10];
    const char *inptr = input;
    size_t inbytesleft = 1;
    char *outptr = buf;
    size_t outbytesleft = sizeof (buf);
    size_t res = iconv (cd_utf8_to_88591,
                        (ICONV_CONST char **) &inptr, &inbytesleft,
                        &outptr, &outbytesleft);
    ASSERT (res == (size_t)(-1) && errno == EINVAL);
    ASSERT (inbytesleft == 1 && outptr == buf);
  }

  iconv_close (cd_88591_to_utf8);
  iconv_close (cd_utf8_to_88591);

#ifdef CONVERT_ENABLED
# pragma convert(pop)
#endif

#endif /* HAVE_ICONV */

  return 0;
}
