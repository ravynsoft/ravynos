/* Test of fnmatch string matching function.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

/* Written by Simon Josefsson <simon@josefsson.org>, 2009,
   and Bruno Haible <bruno@clisp.org>, 2023.  */

#include <config.h>

#include <fnmatch.h>

#include "signature.h"
SIGNATURE_CHECK (fnmatch, int, (char const *, char const *, int));

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "macros.h"

int
main (int argc, char *argv[])
{
  /* configure should already have checked that the locale is supported.  */
  if (setlocale (LC_ALL, "") == NULL)
    return 1;

#ifdef __ANDROID__
  /* On Android ≥ 5.0, the default locale is the "C.UTF-8" locale, not the
     "C" locale.  Furthermore, when you attempt to set the "C" or "POSIX"
     locale via setlocale(), what you get is a "C" locale with UTF-8 encoding,
     that is, effectively the "C.UTF-8" locale.  */
  if (argc > 1 && strcmp (argv[1], "1") == 0 && MB_CUR_MAX > 1)
    argv[1] = "3";
#endif

  /* Patterns with ordinary characters.  */

  ASSERT (fnmatch ("", "", 0) == 0);
  ASSERT (fnmatch ("", "abc", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("xy", "xy", 0) == 0);
  ASSERT (fnmatch ("xy", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("xy", "wxy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("xy", "xyz", 0) == FNM_NOMATCH);

  /* Patterns with special pattern characters.  */

  ASSERT (fnmatch ("?", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("?", "a", 0) == 0);
  ASSERT (fnmatch ("?", "z", 0) == 0);
  ASSERT (fnmatch ("?", ".", 0) == 0);
  ASSERT (fnmatch ("?", "/", 0) == 0);
  ASSERT (fnmatch ("?", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("?", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("x?y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?y", "y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?y", "xay", 0) == 0);
  ASSERT (fnmatch ("x?y", "xzy", 0) == 0);
  ASSERT (fnmatch ("x?y", "x.y", 0) == 0);
  ASSERT (fnmatch ("x?y", "x/y", 0) == 0);
  ASSERT (fnmatch ("x?y", "xazy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?y", "x//y", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("*", "", 0) == 0);
  ASSERT (fnmatch ("*", "a", 0) == 0);
  ASSERT (fnmatch ("*", "z", 0) == 0);
  ASSERT (fnmatch ("*", ".", 0) == 0);
  ASSERT (fnmatch ("*", "*", 0) == 0);
  ASSERT (fnmatch ("*", "/", 0) == 0);
  ASSERT (fnmatch ("*", "az", 0) == 0);
  ASSERT (fnmatch ("*", "//", 0) == 0);
  ASSERT (fnmatch ("*", "some long text", 0) == 0);

  ASSERT (fnmatch ("x*y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*y", "y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*y", "xy", 0) == 0);
  ASSERT (fnmatch ("x*y", "xay", 0) == 0);
  ASSERT (fnmatch ("x*y", "xzy", 0) == 0);
  ASSERT (fnmatch ("x*y", "x.y", 0) == 0);
  ASSERT (fnmatch ("x*y", "x*y", 0) == 0);
  ASSERT (fnmatch ("x*y", "x/y", 0) == 0);
  ASSERT (fnmatch ("x*y", "xazy", 0) == 0);
  ASSERT (fnmatch ("x*y", "x//y", 0) == 0);

  ASSERT (fnmatch ("**", "", 0) == 0);
  ASSERT (fnmatch ("**", "a", 0) == 0);
  ASSERT (fnmatch ("**", "z", 0) == 0);
  ASSERT (fnmatch ("**", ".", 0) == 0);
  ASSERT (fnmatch ("**", "*", 0) == 0);
  ASSERT (fnmatch ("**", "/", 0) == 0);
  ASSERT (fnmatch ("**", "az", 0) == 0);
  ASSERT (fnmatch ("**", "//", 0) == 0);
  ASSERT (fnmatch ("**", "some long text", 0) == 0);

  ASSERT (fnmatch ("x**y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x**y", "y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x**y", "xy", 0) == 0);
  ASSERT (fnmatch ("x**y", "xay", 0) == 0);
  ASSERT (fnmatch ("x**y", "xzy", 0) == 0);
  ASSERT (fnmatch ("x**y", "x.y", 0) == 0);
  ASSERT (fnmatch ("x**y", "x*y", 0) == 0);
  ASSERT (fnmatch ("x**y", "x/y", 0) == 0);
  ASSERT (fnmatch ("x**y", "xazy", 0) == 0);
  ASSERT (fnmatch ("x**y", "x//y", 0) == 0);
  ASSERT (fnmatch ("x**y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x**y", "y", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("*.*", "foo.txt", 0) == 0);
  ASSERT (fnmatch ("*.*", "foo.", 0) == 0);
  ASSERT (fnmatch ("*.*", "foo", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("x*y*z", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*y*z", "x", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*y*z", "y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*y*z", "z", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*y*z", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*y*z", "xz", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*y*z", "yz", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*y*z", "xyz", 0) == 0);
  ASSERT (fnmatch ("x*y*z", "xayz", 0) == 0);
  ASSERT (fnmatch ("x*y*z", "xybz", 0) == 0);
  ASSERT (fnmatch ("x*y*z", "xaybz", 0) == 0);
  ASSERT (fnmatch ("x*y*z", "xxyz", 0) == 0);
  ASSERT (fnmatch ("x*y*z", "xyyz", 0) == 0);
  ASSERT (fnmatch ("x*y*z", "xyzz", 0) == 0);
  ASSERT (fnmatch ("x*y*z", "xyxyz", 0) == 0);

  ASSERT (fnmatch ("x***??y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x***??y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x***??y", "x1y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x***??y", "x12y", 0) == 0);
  ASSERT (fnmatch ("x***??y", "x123y", 0) == 0);

  ASSERT (fnmatch ("x**??*y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x**??*y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x**??*y", "x1y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x**??*y", "x12y", 0) == 0);
  ASSERT (fnmatch ("x**??*y", "x123y", 0) == 0);

  ASSERT (fnmatch ("x*??**y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*??**y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*??**y", "x1y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*??**y", "x12y", 0) == 0);
  ASSERT (fnmatch ("x*??**y", "x123y", 0) == 0);

  ASSERT (fnmatch ("x??***y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x??***y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x??***y", "x1y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x??***y", "x12y", 0) == 0);
  ASSERT (fnmatch ("x??***y", "x123y", 0) == 0);

  ASSERT (fnmatch ("x**?*?y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x**?*?y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x**?*?y", "x1y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x**?*?y", "x12y", 0) == 0);
  ASSERT (fnmatch ("x**?*?y", "x123y", 0) == 0);

  ASSERT (fnmatch ("x*?*?*y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*?*?*y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*?*?*y", "x1y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*?*?*y", "x12y", 0) == 0);
  ASSERT (fnmatch ("x*?*?*y", "x123y", 0) == 0);

  ASSERT (fnmatch ("x?*?**y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?*?**y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?*?**y", "x1y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?*?**y", "x12y", 0) == 0);
  ASSERT (fnmatch ("x?*?**y", "x123y", 0) == 0);

  ASSERT (fnmatch ("x*?**?y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*?**?y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*?**?y", "x1y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*?**?y", "x12y", 0) == 0);
  ASSERT (fnmatch ("x*?**?y", "x123y", 0) == 0);

  ASSERT (fnmatch ("x?**?*y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?**?*y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?**?*y", "x1y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?**?*y", "x12y", 0) == 0);
  ASSERT (fnmatch ("x?**?*y", "x123y", 0) == 0);

  ASSERT (fnmatch ("x?***?y", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?***?y", "xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?***?y", "x1y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?***?y", "x12y", 0) == 0);
  ASSERT (fnmatch ("x?***?y", "x123y", 0) == 0);

  /* Patterns with bracket expressions.  */

  ASSERT (fnmatch ("[a-z]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[a-z]", "a", 0) == 0);
  ASSERT (fnmatch ("[a-z]", "z", 0) == 0);
  ASSERT (fnmatch ("[a-z]", ".", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[a-z]", "/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[a-z]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[a-z]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[!a-z]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[!a-z]", "a", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[!a-z]", "z", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[!a-z]", ".", 0) == 0);
  ASSERT (fnmatch ("[!a-z]", "/", 0) == 0);
  ASSERT (fnmatch ("[!a-z]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[!a-z]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:alnum:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:alnum:]]", "a", 0) == 0);
  ASSERT (fnmatch ("[[:alnum:]]", "z", 0) == 0);
  ASSERT (fnmatch ("[[:alnum:]]", "7", 0) == 0);
  ASSERT (fnmatch ("[[:alnum:]]", ".", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:alnum:]]", "/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:alnum:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:alnum:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:alpha:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:alpha:]]", "a", 0) == 0);
  ASSERT (fnmatch ("[[:alpha:]]", "z", 0) == 0);
  ASSERT (fnmatch ("[[:alpha:]]", "7", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:alpha:]]", ".", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:alpha:]]", "/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:alpha:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:alpha:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:blank:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:blank:]]", "a", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:blank:]]", "z", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:blank:]]", "7", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:blank:]]", " ", 0) == 0);
  ASSERT (fnmatch ("[[:blank:]]", ".", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:blank:]]", "/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:blank:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:blank:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:cntrl:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:cntrl:]]", "a", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:cntrl:]]", "z", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:cntrl:]]", "7", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:cntrl:]]", "\n", 0) == 0);
  ASSERT (fnmatch ("[[:cntrl:]]", ".", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:cntrl:]]", "/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:cntrl:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:cntrl:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:digit:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:digit:]]", "a", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:digit:]]", "z", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:digit:]]", "7", 0) == 0);
  ASSERT (fnmatch ("[[:digit:]]", ".", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:digit:]]", "/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:digit:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:digit:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:graph:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:graph:]]", "a", 0) == 0);
  ASSERT (fnmatch ("[[:graph:]]", "z", 0) == 0);
  ASSERT (fnmatch ("[[:graph:]]", "7", 0) == 0);
  ASSERT (fnmatch ("[[:graph:]]", " ", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:graph:]]", ".", 0) == 0);
  ASSERT (fnmatch ("[[:graph:]]", "/", 0) == 0);
  ASSERT (fnmatch ("[[:graph:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:graph:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:lower:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:lower:]]", "a", 0) == 0);
  ASSERT (fnmatch ("[[:lower:]]", "z", 0) == 0);
  ASSERT (fnmatch ("[[:lower:]]", "A", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:lower:]]", "Z", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:lower:]]", "7", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:lower:]]", ".", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:lower:]]", "/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:lower:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:lower:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:print:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:print:]]", "a", 0) == 0);
  ASSERT (fnmatch ("[[:print:]]", "z", 0) == 0);
  ASSERT (fnmatch ("[[:print:]]", "7", 0) == 0);
  ASSERT (fnmatch ("[[:print:]]", " ", 0) == 0);
  ASSERT (fnmatch ("[[:print:]]", ".", 0) == 0);
  ASSERT (fnmatch ("[[:print:]]", "/", 0) == 0);
  ASSERT (fnmatch ("[[:print:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:print:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:punct:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:punct:]]", "a", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:punct:]]", "z", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:punct:]]", "7", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:punct:]]", ".", 0) == 0);
  ASSERT (fnmatch ("[[:punct:]]", "/", 0) == 0);
  ASSERT (fnmatch ("[[:punct:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:punct:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:space:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:space:]]", "a", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:space:]]", "z", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:space:]]", "7", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:space:]]", " ", 0) == 0);
  ASSERT (fnmatch ("[[:space:]]", "\t", 0) == 0);
  ASSERT (fnmatch ("[[:space:]]", ".", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:space:]]", "/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:space:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:space:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:upper:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:upper:]]", "a", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:upper:]]", "z", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:upper:]]", "A", 0) == 0);
  ASSERT (fnmatch ("[[:upper:]]", "Z", 0) == 0);
  ASSERT (fnmatch ("[[:upper:]]", "7", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:upper:]]", ".", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:upper:]]", "/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:upper:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:upper:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:xdigit:]]", "", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:xdigit:]]", "a", 0) == 0);
  ASSERT (fnmatch ("[[:xdigit:]]", "F", 0) == 0);
  ASSERT (fnmatch ("[[:xdigit:]]", "z", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:xdigit:]]", "7", 0) == 0);
  ASSERT (fnmatch ("[[:xdigit:]]", ".", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:xdigit:]]", "/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:xdigit:]]", "az", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:xdigit:]]", "//", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:alpha:]'[:alpha:]\0]", "a", 0) == FNM_NOMATCH);

  ASSERT (fnmatch ("[a[.\0.]]", "a", 0) == FNM_NOMATCH);

  /* Verify that an unmatched [ is treated as a literal, as POSIX requires.  */
  ASSERT (fnmatch ("[", "[", 0) == 0);
  ASSERT (fnmatch ("[", "]", 0) == FNM_NOMATCH);

  /* The FNM_PATHNAME flag.  */

  ASSERT (fnmatch ("x?y", "x/y", 0) == 0);
  ASSERT (fnmatch ("x?y", "x/y", FNM_PATHNAME) == FNM_NOMATCH);

  ASSERT (fnmatch ("x*y", "x/y", 0) == 0);
  ASSERT (fnmatch ("x*y", "x/y", FNM_PATHNAME) == FNM_NOMATCH);

  ASSERT (fnmatch ("[/-/]", "/", 0) == 0);
  ASSERT (fnmatch ("[/-/]", "/", FNM_PATHNAME) == FNM_NOMATCH);
  ASSERT (fnmatch ("[!-~]", "/", FNM_PATHNAME) == FNM_NOMATCH);

  ASSERT (fnmatch ("[[:graph:]]", "/", FNM_PATHNAME) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:print:]]", "/", FNM_PATHNAME) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:punct:]]", "/", FNM_PATHNAME) == FNM_NOMATCH);

  /* The FNM_NOESCAPE flag.  */

  ASSERT (fnmatch ("foo\\.txt", "foo.txt", 0) == 0);
  ASSERT (fnmatch ("foo\\.txt", "foo.txt", FNM_NOESCAPE) == FNM_NOMATCH);

  ASSERT (fnmatch ("x?y", "x/y", 0) == 0);
  ASSERT (fnmatch ("x\\?y", "x/y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x\\?y", "x/y", FNM_NOESCAPE) == FNM_NOMATCH);
  ASSERT (fnmatch ("x\\?y", "x?y", 0) == 0);
  ASSERT (fnmatch ("x\\?y", "x?y", FNM_NOESCAPE) == FNM_NOMATCH);
  ASSERT (fnmatch ("x\\?y", "x\\?y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x\\?y", "x\\?y", FNM_NOESCAPE) == 0);

  ASSERT (fnmatch ("x*y", "x/y", 0) == 0);
  ASSERT (fnmatch ("x\\*y", "x/y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x\\*y", "x/y", FNM_NOESCAPE) == FNM_NOMATCH);
  ASSERT (fnmatch ("x\\*y", "x*y", 0) == 0);
  ASSERT (fnmatch ("x\\*y", "x*y", FNM_NOESCAPE) == FNM_NOMATCH);
  ASSERT (fnmatch ("x\\*y", "x\\*y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x\\*y", "x\\*y", FNM_NOESCAPE) == 0);

  /* The FNM_PERIOD flag.  */

  ASSERT (fnmatch ("foo.rc", "foo.rc", 0) == 0);
  ASSERT (fnmatch ("foo.rc", "foo.rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("foo.rc", "foo.rc", FNM_PERIOD) == 0);
  ASSERT (fnmatch ("foo.rc", "foo.rc", FNM_PATHNAME | FNM_PERIOD) == 0);
  ASSERT (fnmatch ("foo?rc", "foo.rc", 0) == 0);
  ASSERT (fnmatch ("foo?rc", "foo.rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("foo?rc", "foo.rc", FNM_PERIOD) == 0);
  ASSERT (fnmatch ("foo?rc", "foo.rc", FNM_PATHNAME | FNM_PERIOD) == 0);
  ASSERT (fnmatch ("foo[[:punct:]]rc", "foo.rc", 0) == 0);
  ASSERT (fnmatch ("foo[[:punct:]]rc", "foo.rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("foo[[:punct:]]rc", "foo.rc", FNM_PERIOD) == 0);
  ASSERT (fnmatch ("foo[[:punct:]]rc", "foo.rc", FNM_PATHNAME | FNM_PERIOD) == 0);
  ASSERT (fnmatch ("foo*rc", "foo.rc", 0) == 0);
  ASSERT (fnmatch ("foo*rc", "foo.rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("foo*rc", "foo.rc", FNM_PERIOD) == 0);
  ASSERT (fnmatch ("foo*rc", "foo.rc", FNM_PATHNAME | FNM_PERIOD) == 0);

  ASSERT (fnmatch (".rc", ".rc", 0) == 0);
  ASSERT (fnmatch (".rc", ".rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch (".rc", ".rc", FNM_PERIOD) == 0);
  ASSERT (fnmatch (".rc", ".rc", FNM_PATHNAME | FNM_PERIOD) == 0);
  ASSERT (fnmatch ("?rc", ".rc", 0) == 0);
  ASSERT (fnmatch ("?rc", ".rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("?rc", ".rc", FNM_PERIOD) == FNM_NOMATCH);
  ASSERT (fnmatch ("?rc", ".rc", FNM_PATHNAME | FNM_PERIOD) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:punct:]]rc", ".rc", 0) == 0);
  ASSERT (fnmatch ("[[:punct:]]rc", ".rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("[[:punct:]]rc", ".rc", FNM_PERIOD) == FNM_NOMATCH);
  ASSERT (fnmatch ("[[:punct:]]rc", ".rc", FNM_PATHNAME | FNM_PERIOD) == FNM_NOMATCH);
  ASSERT (fnmatch ("*rc", ".rc", 0) == 0);
  ASSERT (fnmatch ("*rc", ".rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("*rc", ".rc", FNM_PERIOD) == FNM_NOMATCH);
  ASSERT (fnmatch ("*rc", ".rc", FNM_PATHNAME | FNM_PERIOD) == FNM_NOMATCH);

  ASSERT (fnmatch ("dir/.rc", "dir/.rc", 0) == 0);
  ASSERT (fnmatch ("dir/.rc", "dir/.rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("dir/.rc", "dir/.rc", FNM_PERIOD) == 0);
  ASSERT (fnmatch ("dir/.rc", "dir/.rc", FNM_PATHNAME | FNM_PERIOD) == 0);
  ASSERT (fnmatch ("dir/?rc", "dir/.rc", 0) == 0);
  ASSERT (fnmatch ("dir/?rc", "dir/.rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("dir/?rc", "dir/.rc", FNM_PERIOD) == 0);
  ASSERT (fnmatch ("dir/?rc", "dir/.rc", FNM_PATHNAME | FNM_PERIOD) == FNM_NOMATCH);
  ASSERT (fnmatch ("dir/[[:punct:]]rc", "dir/.rc", 0) == 0);
  ASSERT (fnmatch ("dir/[[:punct:]]rc", "dir/.rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("dir/[[:punct:]]rc", "dir/.rc", FNM_PERIOD) == 0);
  ASSERT (fnmatch ("dir/[[:punct:]]rc", "dir/.rc", FNM_PATHNAME | FNM_PERIOD) == FNM_NOMATCH);
  ASSERT (fnmatch ("dir/*rc", "dir/.rc", 0) == 0);
  ASSERT (fnmatch ("dir/*rc", "dir/.rc", FNM_PATHNAME) == 0);
  ASSERT (fnmatch ("dir/*rc", "dir/.rc", FNM_PERIOD) == 0);
  ASSERT (fnmatch ("dir/*rc", "dir/.rc", FNM_PATHNAME | FNM_PERIOD) == FNM_NOMATCH);
  ASSERT (fnmatch ("*rc", "dir/.rc", 0) == 0);
  ASSERT (fnmatch ("*rc", "dir/.rc", FNM_PATHNAME) == FNM_NOMATCH);
  ASSERT (fnmatch ("*rc", "dir/.rc", FNM_PERIOD) == 0);
  ASSERT (fnmatch ("*rc", "dir/.rc", FNM_PATHNAME | FNM_PERIOD) == FNM_NOMATCH);

  /* The FNM_LEADING_DIR flag (common extension).  */
  #if GNULIB_FNMATCH_GNU && defined FNM_LEADING_DIR

  ASSERT (fnmatch ("x", "x", 0) == 0);
  ASSERT (fnmatch ("x", "x", FNM_LEADING_DIR) == 0);
  ASSERT (fnmatch ("x", "x/", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x", "x/", FNM_LEADING_DIR) == 0);
  ASSERT (fnmatch ("x", "x/y", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("x", "x/y", FNM_LEADING_DIR) == 0);

  ASSERT (fnmatch ("x?y", "x/y", 0) == 0);
  ASSERT (fnmatch ("x?y", "x/y", FNM_LEADING_DIR) == 0);
  ASSERT (fnmatch ("x?y", "x/y", FNM_PATHNAME) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?y", "x/y", FNM_PATHNAME | FNM_LEADING_DIR) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?y", "x/y/z", FNM_PATHNAME | FNM_LEADING_DIR) == FNM_NOMATCH);

  #endif

  /* The FNM_CASEFOLD flag (common extension).  */
  #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD

  ASSERT (fnmatch ("xy", "xy", 0) == 0);
  ASSERT (fnmatch ("xy", "xy", FNM_CASEFOLD) == 0);
  ASSERT (fnmatch ("xy", "Xy", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("xy", "Xy", FNM_CASEFOLD) == 0);
  ASSERT (fnmatch ("xy", "xY", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("xy", "xY", FNM_CASEFOLD) == 0);
  ASSERT (fnmatch ("xy", "XY", 0) == FNM_NOMATCH);
  ASSERT (fnmatch ("xy", "XY", FNM_CASEFOLD) == 0);

  /* Locales other than the "C" locale may order the ASCII letters like this:
     AaBb...Mm...Zz .  Seen on FreeBSD and Solaris 11.  */
  if (argc == 1 || strcmp (argv[1], "1") == 0)
    ASSERT (fnmatch ("[a-z]", "M", 0) == FNM_NOMATCH);
  #if !defined GNULIB_defined_fnmatch_function
  ASSERT (fnmatch ("[a-z]", "M", FNM_CASEFOLD) == 0);
  #endif
  if (argc == 1 || strcmp (argv[1], "1") == 0)
    ASSERT (fnmatch ("[A-Z]", "m", 0) == FNM_NOMATCH);
  #if !defined GNULIB_defined_fnmatch_function
  ASSERT (fnmatch ("[A-Z]", "m", FNM_CASEFOLD) == 0);
  #endif

  #endif

  /* The FNM_EXTMATCH flag (GNU extension).  */
  #if GNULIB_FNMATCH_GNU && defined FNM_EXTMATCH

  ASSERT (fnmatch ("x?($|[[:digit:]])y", "", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?($|[[:digit:]])y", "y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?($|[[:digit:]])y", "xy", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x?($|[[:digit:]])y", "x$y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x?($|[[:digit:]])y", "x7y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x?($|[[:digit:]])y", "xay", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?($|[[:digit:]])y", "xzy", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?($|[[:digit:]])y", "x/y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?($|[[:digit:]])y", "x32y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?($|[[:digit:]])y", "x$1y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x?($|[[:digit:]])y", "x1$y", FNM_EXTMATCH) == FNM_NOMATCH);

  ASSERT (fnmatch ("x*($|[[:digit:]])y", "", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*($|[[:digit:]])y", "y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*($|[[:digit:]])y", "xy", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x*($|[[:digit:]])y", "x$y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x*($|[[:digit:]])y", "x7y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x*($|[[:digit:]])y", "xay", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*($|[[:digit:]])y", "xzy", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*($|[[:digit:]])y", "x/y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x*($|[[:digit:]])y", "x32y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x*($|[[:digit:]])y", "x$1y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x*($|[[:digit:]])y", "x1$y", FNM_EXTMATCH) == 0);

  ASSERT (fnmatch ("x+($|[[:digit:]])y", "", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x+($|[[:digit:]])y", "y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x+($|[[:digit:]])y", "xy", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x+($|[[:digit:]])y", "x$y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x+($|[[:digit:]])y", "x7y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x+($|[[:digit:]])y", "xay", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x+($|[[:digit:]])y", "xzy", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x+($|[[:digit:]])y", "x/y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x+($|[[:digit:]])y", "x32y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x+($|[[:digit:]])y", "x$1y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x+($|[[:digit:]])y", "x1$y", FNM_EXTMATCH) == 0);

  ASSERT (fnmatch ("x@($|[[:digit:]])y", "", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x@($|[[:digit:]])y", "y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x@($|[[:digit:]])y", "xy", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x@($|[[:digit:]])y", "x$y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x@($|[[:digit:]])y", "x7y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x@($|[[:digit:]])y", "xay", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x@($|[[:digit:]])y", "xzy", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x@($|[[:digit:]])y", "x/y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x@($|[[:digit:]])y", "x32y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x@($|[[:digit:]])y", "x$1y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x@($|[[:digit:]])y", "x1$y", FNM_EXTMATCH) == FNM_NOMATCH);

  ASSERT (fnmatch ("x!($|[[:digit:]])y", "", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x!($|[[:digit:]])y", "y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x!($|[[:digit:]])y", "xy", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x!($|[[:digit:]])y", "x$y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x!($|[[:digit:]])y", "x7y", FNM_EXTMATCH) == FNM_NOMATCH);
  ASSERT (fnmatch ("x!($|[[:digit:]])y", "xay", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x!($|[[:digit:]])y", "xzy", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x!($|[[:digit:]])y", "x/y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x!($|[[:digit:]])y", "x32y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x!($|[[:digit:]])y", "x$1y", FNM_EXTMATCH) == 0);
  ASSERT (fnmatch ("x!($|[[:digit:]])y", "x1$y", FNM_EXTMATCH) == 0);

  ASSERT (fnmatch ("**(!()", "**(!()", FNM_EXTMATCH) == 0);

  #endif

  /* Verify that the test cases from fnmatch.m4 are really working.  */
  {
    char const *Apat = 'A' < '\\' ? "[A-\\\\]" : "[\\\\-A]";
    char const *apat = 'a' < '\\' ? "[a-\\\\]" : "[\\\\-a]";
    static char const A_1[] = { 'A' - 1, 0 };
    static char const A01[] = { 'A' + 1, 0 };
    static char const a_1[] = { 'a' - 1, 0 };
    static char const a01[] = { 'a' + 1, 0 };
    static char const bs_1[] = { '\\' - 1, 0 };
    static char const bs01[] = { '\\' + 1, 0 };
    /* These are sanity checks. They all succeed on current platforms.  */
    ASSERT (fnmatch ("a*", "", 0) == FNM_NOMATCH);
    ASSERT (fnmatch ("a*", "abc", 0) == 0);
    ASSERT (fnmatch ("d*/*1", "d/s/1", FNM_PATHNAME) == FNM_NOMATCH);
    ASSERT (fnmatch ("a\\bc", "abc", 0) == 0);
    ASSERT (fnmatch ("a\\bc", "abc", FNM_NOESCAPE) == FNM_NOMATCH);
    ASSERT (fnmatch ("*x", ".x", 0) == 0);
    ASSERT (fnmatch ("*x", ".x", FNM_PERIOD) == FNM_NOMATCH);
    if (argc == 1 || strcmp (argv[1], "1") == 0)
      {
        /* glibc bug <https://sourceware.org/bugzilla/show_bug.cgi?id=361>
           exists in glibc 2.3.3, fixed in glibc 2.5.  */
        ASSERT (fnmatch (Apat, "\\", 0) == 0);
        ASSERT (fnmatch (Apat, "A", 0) == 0);
        ASSERT (fnmatch (apat, "\\", 0) == 0);
        ASSERT (fnmatch (apat, "a", 0) == 0);
        ASSERT (fnmatch (Apat, A_1, 0) == ('A' < '\\' ? FNM_NOMATCH : 0));
        ASSERT (fnmatch (apat, a_1, 0) == ('a' < '\\' ? FNM_NOMATCH : 0));
        ASSERT (fnmatch (Apat, A01, 0) == ('A' < '\\' ? 0 : FNM_NOMATCH));
        ASSERT (fnmatch (apat, a01, 0) == ('a' < '\\' ? 0 : FNM_NOMATCH));
        ASSERT (fnmatch (Apat, bs_1, 0) == ('A' < '\\' ? 0 : FNM_NOMATCH));
        ASSERT (fnmatch (apat, bs_1, 0) == ('a' < '\\' ? 0 : FNM_NOMATCH));
        ASSERT (fnmatch (Apat, bs01, 0) == ('A' < '\\' ? FNM_NOMATCH : 0));
        ASSERT (fnmatch (apat, bs01, 0) == ('a' < '\\' ? FNM_NOMATCH : 0));
      }
    /* glibc bug <https://sourceware.org/bugzilla/show_bug.cgi?id=12378>
       exists in glibc 2.12, fixed in glibc 2.13.  */
    ASSERT (fnmatch ("[/b", "[/b", 0) == 0);
    /* This test fails on FreeBSD 13.2, NetBSD 9.3, Cygwin 3.4.6.  */
    ASSERT (fnmatch ("[[:alnum:]]", "a", 0) == 0);
    #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD
    ASSERT (fnmatch ("xxXX", "xXxX", FNM_CASEFOLD) == 0);
    #endif
    #if GNULIB_FNMATCH_GNU && defined FNM_EXTMATCH
    ASSERT (fnmatch ("a++(x|yy)b", "a+xyyyyxb", FNM_EXTMATCH) == 0);
    #endif
    #if GNULIB_FNMATCH_GNU && defined FNM_FILE_NAME
    ASSERT (fnmatch ("d*/*1", "d/s/1", FNM_FILE_NAME) == FNM_NOMATCH);
    ASSERT (fnmatch ("*", "x", FNM_FILE_NAME | FNM_LEADING_DIR) == 0);
    ASSERT (fnmatch ("x*", "x/y/z", FNM_FILE_NAME | FNM_LEADING_DIR) == 0);
    ASSERT (fnmatch ("*c*", "c/x", FNM_FILE_NAME | FNM_LEADING_DIR) == 0);
    #endif
  }

  /* Locale specific recognition of multibyte characters.  */

  if (argc > 1)
    switch (argv[1][0])
      {
      case '1':
        /* C or POSIX locale.  */
        return 0;

      case '2':
        /* Locale encoding is ISO-8859-1 or ISO-8859-15.  */

        ASSERT (fnmatch ("x?y", "x\374y", 0) == 0); /* "xüy" */
        ASSERT (fnmatch ("x?y", "x\337y", 0) == 0); /* "xßy" */
        /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        ASSERT (fnmatch ("x[[:alnum:]]y", "x\330y", 0) == 0);
        /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        ASSERT (fnmatch ("x[[:alpha:]]y", "x\330y", 0) == 0);
        #if !(defined __FreeBSD__ || defined __DragonFly__)
        /* U+00B8 CEDILLA */
        ASSERT (fnmatch ("x[[:graph:]]y", "x\270y", 0) == 0);
        #endif
        /* U+00FF LATIN SMALL LETTER Y WITH DIAERESIS */
        ASSERT (fnmatch ("x[[:lower:]]y", "x\377y", 0) == 0);
        #if !(defined __FreeBSD__ || defined __DragonFly__)
        /* U+00B8 CEDILLA */
        ASSERT (fnmatch ("x[[:print:]]y", "x\270y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__)
        /* U+00BF INVERTED QUESTION MARK */
        ASSERT (fnmatch ("x[[:punct:]]y", "x\277y", 0) == 0);
        #endif
        /* U+00C9 LATIN CAPITAL LETTER E WITH ACUTE */
        ASSERT (fnmatch ("x[[:upper:]]y", "x\311y", 0) == 0);
        /* U+00D7 MULTIPLICATION SIGN */
        ASSERT (fnmatch ("x[[:alnum:]]y", "x\327y", 0) == FNM_NOMATCH);
        /* U+00D7 MULTIPLICATION SIGN */
        ASSERT (fnmatch ("x[[:alpha:]]y", "x\327y", 0) == FNM_NOMATCH);
        /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        ASSERT (fnmatch ("x[[:blank:]]y", "x\330y", 0) == FNM_NOMATCH);
        /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        ASSERT (fnmatch ("x[[:cntrl:]]y", "x\330y", 0) == FNM_NOMATCH);
        /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        ASSERT (fnmatch ("x[[:digit:]]y", "x\330y", 0) == FNM_NOMATCH);
        /* U+00B2 SUPERSCRIPT TWO */
        ASSERT (fnmatch ("x[[:lower:]]y", "x\262y", 0) == FNM_NOMATCH);
        /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        ASSERT (fnmatch ("x[[:punct:]]y", "x\330y", 0) == FNM_NOMATCH);
        /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        ASSERT (fnmatch ("x[[:space:]]y", "x\330y", 0) == FNM_NOMATCH);
        /* U+00B2 SUPERSCRIPT TWO */
        ASSERT (fnmatch ("x[[:upper:]]y", "x\262y", 0) == FNM_NOMATCH);
        /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        ASSERT (fnmatch ("x[[:xdigit:]]y", "x\330y", 0) == FNM_NOMATCH);

        #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD
        /* "Höhle" */
        ASSERT (fnmatch ("H\366hle", "H\326hLe", FNM_CASEFOLD) == 0);
        ASSERT (fnmatch ("H\326hLe", "H\366hle", FNM_CASEFOLD) == 0);
        ASSERT (fnmatch ("H\326hle", "H\366hLe", FNM_CASEFOLD) == 0);
        ASSERT (fnmatch ("H\366hLe", "H\326hle", FNM_CASEFOLD) == 0);
        #endif

        return 0;

      case '3':
        /* Locale encoding is UTF-8.  */

        ASSERT (fnmatch ("x?y", "x\303\274y", 0) == 0); /* "xüy" */
        ASSERT (fnmatch ("x?y", "x\303\237y", 0) == 0); /* "xßy" */
        ASSERT (fnmatch ("x?y", "x\360\237\230\213y", 0) == 0); /* "x😋y" */
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:alnum:]]y", "x\305\201y", 0) == 0);
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
        /* U+10330 GOTHIC LETTER AHSA */
        ASSERT (fnmatch ("x[[:alnum:]]y", "x\360\220\214\260y", 0) == 0);
        #endif
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:alpha:]]y", "x\305\201y", 0) == 0);
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
        /* U+10330 GOTHIC LETTER AHSA */
        ASSERT (fnmatch ("x[[:alpha:]]y", "x\360\220\214\260y", 0) == 0);
        #endif
        /* U+00B8 CEDILLA */
        ASSERT (fnmatch ("x[[:graph:]]y", "x\302\270y", 0) == 0);
        #if !defined __sun
        /* U+20000 <CJK Ideograph> */
        ASSERT (fnmatch ("x[[:graph:]]y", "x\360\240\200\200y", 0) == 0);
        #endif
        /* U+00FF LATIN SMALL LETTER Y WITH DIAERESIS */
        ASSERT (fnmatch ("x[[:lower:]]y", "x\303\277y", 0) == 0);
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
        /* U+10441 DESERET SMALL LETTER EF */
        ASSERT (fnmatch ("x[[:lower:]]y", "x\360\220\221\201y", 0) == 0);
        #endif
        /* U+00B8 CEDILLA */
        ASSERT (fnmatch ("x[[:print:]]y", "x\302\270y", 0) == 0);
        #if !defined __sun
        /* U+20000 <CJK Ideograph> */
        ASSERT (fnmatch ("x[[:print:]]y", "x\360\240\200\200y", 0) == 0);
        #endif
        /* U+00BF INVERTED QUESTION MARK */
        ASSERT (fnmatch ("x[[:punct:]]y", "x\302\277y", 0) == 0);
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
        /* U+1D100 MUSICAL SYMBOL SINGLE BARLINE */
        if (iswpunct (0x1D100))
          ASSERT (fnmatch ("x[[:punct:]]y", "x\360\235\204\200y", 0) == 0);
        #endif
        /* U+3000 IDEOGRAPHIC SPACE */
        ASSERT (fnmatch ("x[[:space:]]y", "x\343\200\200y", 0) == 0);
        /* U+0429 CYRILLIC CAPITAL LETTER SHCHA */
        ASSERT (fnmatch ("x[[:upper:]]y", "x\320\251y", 0) == 0);
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
        /* U+10419 DESERET CAPITAL LETTER EF */
        ASSERT (fnmatch ("x[[:upper:]]y", "x\360\220\220\231y", 0) == 0);
        #endif
        /* U+00D7 MULTIPLICATION SIGN */
        ASSERT (fnmatch ("x[[:alnum:]]y", "x\303\227y", 0) == FNM_NOMATCH);
        /* U+00D7 MULTIPLICATION SIGN */
        ASSERT (fnmatch ("x[[:alpha:]]y", "x\303\227y", 0) == FNM_NOMATCH);
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:blank:]]y", "x\305\201y", 0) == FNM_NOMATCH);
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:cntrl:]]y", "x\305\201y", 0) == FNM_NOMATCH);
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:digit:]]y", "x\305\201y", 0) == FNM_NOMATCH);
        /* U+2002 EN SPACE */
        ASSERT (fnmatch ("x[[:graph:]]y", "x\342\200\202y", 0) == FNM_NOMATCH);
        /* U+00B2 SUPERSCRIPT TWO */
        ASSERT (fnmatch ("x[[:lower:]]y", "x\302\262y", 0) == FNM_NOMATCH);
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:punct:]]y", "x\305\201y", 0) == FNM_NOMATCH);
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:space:]]y", "x\305\201y", 0) == FNM_NOMATCH);
        /* U+00B2 SUPERSCRIPT TWO */
        ASSERT (fnmatch ("x[[:upper:]]y", "x\302\262y", 0) == FNM_NOMATCH);
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:xdigit:]]y", "x\305\201y", 0) == FNM_NOMATCH);

        #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD
        /* "özgür" */
        {
          /* Some platforms, e.g. MSVC 14, lack the upper/lower mappings for
             these wide characters in the *.65001 locales.  */
          mbstate_t state;
          wchar_t wc;
          memset (&state, 0, sizeof (mbstate_t));
          if (mbrtowc (&wc, "\303\274", 2, &state) == 2
              && towupper (wc) != wc)
            {
              ASSERT (fnmatch ("\303\266zg\303\274r", "\303\226ZG\303\234R", FNM_CASEFOLD) == 0);
              ASSERT (fnmatch ("\303\226ZG\303\234R", "\303\266zg\303\274r", FNM_CASEFOLD) == 0);
              ASSERT (fnmatch ("\303\266Zg\303\234r", "\303\226zG\303\274R", FNM_CASEFOLD) == 0);
              ASSERT (fnmatch ("\303\226zG\303\274R", "\303\266Zg\303\234r", FNM_CASEFOLD) == 0);
            }
        }
        #endif

        return 0;

      case '4':
        /* Locale encoding is EUC-JP.  */

        ASSERT (fnmatch ("x?y", "x\306\374y", 0) == 0); /* "x日y" */
        ASSERT (fnmatch ("x?y", "x\313\334y", 0) == 0); /* "x本y" */
        ASSERT (fnmatch ("x?y", "x\270\354y", 0) == 0); /* "x語y" */
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__)
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:alnum:]]y", "x\217\251\250y", 0) == 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__)
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:alpha:]]y", "x\217\251\250y", 0) == 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__)
        /* U+00B8 CEDILLA */
        ASSERT (fnmatch ("x[[:graph:]]y", "x\217\242\261y", 0) == 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__)
        /* U+00FF LATIN SMALL LETTER Y WITH DIAERESIS */
        ASSERT (fnmatch ("x[[:lower:]]y", "x\217\253\363y", 0) == 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__)
        /* U+00B8 CEDILLA */
        ASSERT (fnmatch ("x[[:print:]]y", "x\217\242\261y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__)
        /* U+00D7 MULTIPLICATION SIGN */
        ASSERT (fnmatch ("x[[:punct:]]y", "x\241\337y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__)
        /* U+3000 IDEOGRAPHIC SPACE */
        ASSERT (fnmatch ("x[[:space:]]y", "x\241\241y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__)
        /* U+0429 CYRILLIC CAPITAL LETTER SHCHA */
        ASSERT (fnmatch ("x[[:upper:]]y", "x\247\273y", 0) == 0);
        #endif
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:alnum:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:alpha:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:blank:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:cntrl:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:digit:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3000 IDEOGRAPHIC SPACE */
        ASSERT (fnmatch ("x[[:graph:]]y", "x\241\241y", 0) == FNM_NOMATCH);
        /* U+3073 HIRAGANA LETTER BI */
        ASSERT (fnmatch ("x[[:lower:]]y", "x\244\323y", 0) == FNM_NOMATCH);
        /* U+00DF LATIN SMALL LETTER SHARP S */
        ASSERT (fnmatch ("x[[:punct:]]y", "x\217\251\316y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:space:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3073 HIRAGANA LETTER BI */
        ASSERT (fnmatch ("x[[:upper:]]y", "x\244\323y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:xdigit:]]y", "x\241\242y", 0) == FNM_NOMATCH);

        #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD
        /* "özgür" */
        {
          /* Some platforms, e.g. macOS 12.5 and NetBSD 9.3, lack the
             upper/lower mappings for these wide characters in the ja_JP.eucJP
             locale.  */
          mbstate_t state;
          wchar_t wc;
          memset (&state, 0, sizeof (mbstate_t));
          if (mbrtowc (&wc, "\217\253\344", 3, &state) == 3
              && towupper (wc) != wc)
            {
              ASSERT (fnmatch ("\217\253\323zg\217\253\344r", "\217\252\323ZG\217\252\344R", FNM_CASEFOLD) == 0);
              ASSERT (fnmatch ("\217\252\323ZG\217\252\344R", "\217\253\323zg\217\253\344r", FNM_CASEFOLD) == 0);
              ASSERT (fnmatch ("\217\253\323Zg\217\252\344r", "\217\252\323zG\217\253\344R", FNM_CASEFOLD) == 0);
              ASSERT (fnmatch ("\217\252\323zG\217\253\344R", "\217\253\323Zg\217\252\344r", FNM_CASEFOLD) == 0);
            }
        }
        #endif

        return 0;

      case '5':
        /* Locale encoding is GB18030.  */
        #if (defined __GLIBC__ && __GLIBC__ == 2 && __GLIBC_MINOR__ >= 13 && __GLIBC_MINOR__ <= 15)
        fputs ("Skipping test: The GB18030 converter in this system's iconv is broken.\n", stderr);
        return 77;
        #endif

        ASSERT (fnmatch ("x?y", "x\250\271y", 0) == 0); /* "xüy" */
        ASSERT (fnmatch ("x?y", "x\201\060\211\070y", 0) == 0); /* "xßy" */
        ASSERT (fnmatch ("x?y", "x\224\071\375\067y", 0) == 0); /* "x😋y" */
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:alnum:]]y", "x\201\060\221\071y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
        /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        ASSERT (fnmatch ("x[[:alpha:]]y", "x\201\060\221\071y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
        /* U+00B8 CEDILLA */
        ASSERT (fnmatch ("x[[:graph:]]y", "x\201\060\206\060y", 0) == 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
        /* U+20000 <CJK Ideograph> */
        ASSERT (fnmatch ("x[[:graph:]]y", "x\225\062\202\066y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
        /* U+00FF LATIN SMALL LETTER Y WITH DIAERESIS */
        ASSERT (fnmatch ("x[[:lower:]]y", "x\201\060\213\067y", 0) == 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
        /* U+10441 DESERET SMALL LETTER EF */
        ASSERT (fnmatch ("x[[:lower:]]y", "x\220\060\355\071y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
        /* U+00B8 CEDILLA */
        ASSERT (fnmatch ("x[[:print:]]y", "x\201\060\206\060y", 0) == 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
        /* U+20000 <CJK Ideograph> */
        ASSERT (fnmatch ("x[[:print:]]y", "x\225\062\202\066y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__)
        /* U+00D7 MULTIPLICATION SIGN */
        ASSERT (fnmatch ("x[[:punct:]]y", "x\241\301y", 0) == 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
        /* U+1D100 MUSICAL SYMBOL SINGLE BARLINE */
        ASSERT (fnmatch ("x[[:punct:]]y", "x\224\062\273\064y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__)
        /* U+3000 IDEOGRAPHIC SPACE */
        ASSERT (fnmatch ("x[[:space:]]y", "x\241\241y", 0) == 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__)
        /* U+0429 CYRILLIC CAPITAL LETTER SHCHA */
        ASSERT (fnmatch ("x[[:upper:]]y", "x\247\273y", 0) == 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
        /* U+10419 DESERET CAPITAL LETTER EF */
        ASSERT (fnmatch ("x[[:upper:]]y", "x\220\060\351\071y", 0) == 0);
        #endif
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:alnum:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:alpha:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:blank:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:cntrl:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:digit:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3000 IDEOGRAPHIC SPACE */
        ASSERT (fnmatch ("x[[:graph:]]y", "x\241\241y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:lower:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        ASSERT (fnmatch ("x[[:punct:]]y", "x\201\060\211\061y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:space:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:upper:]]y", "x\241\242y", 0) == FNM_NOMATCH);
        /* U+3001 IDEOGRAPHIC COMMA */
        ASSERT (fnmatch ("x[[:xdigit:]]y", "x\241\242y", 0) == FNM_NOMATCH);

        #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD
        /* "özgür" */
        {
          /* Some platforms, e.g. FreeBSD 13.2 and Solaris 11.4, lack the
             upper/lower mappings for these wide characters in the zh_CN.GB18030
             locale.  */
          mbstate_t state;
          wchar_t wc;
          memset (&state, 0, sizeof (mbstate_t));
          if (mbrtowc (&wc, "\201\060\213\062", 4, &state) == 4
              && towupper (wc) != wc)
            {
              ASSERT (fnmatch ("\201\060\213\062zg\250\271r", "\201\060\211\060ZG\201\060\211\065R", FNM_CASEFOLD) == 0);
              ASSERT (fnmatch ("\201\060\211\060ZG\201\060\211\065R", "\201\060\213\062zg\250\271r", FNM_CASEFOLD) == 0);
              ASSERT (fnmatch ("\201\060\213\062Zg\201\060\211\065r", "\201\060\211\060zG\250\271R", FNM_CASEFOLD) == 0);
              ASSERT (fnmatch ("\201\060\211\060zG\250\271R", "\201\060\213\062Zg\201\060\211\065r", FNM_CASEFOLD) == 0);
            }
        }
        #endif

        return 0;
      }

  return 1;
}
