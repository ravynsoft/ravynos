/* Test bool.
   Copyright (C) 2002-2007, 2009-2023 Free Software Foundation, Inc.

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

/* Define ADDRESS_CHECK_OKAY if it is OK to assign an address to a 'bool'
   and this does not generate a warning (because we want this test to succeed
   even when using gcc's -Werror).  */
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)) \
    || (__clang_major__ >= 4)
/* We can silence the warning.  */
# pragma GCC diagnostic ignored "-Waddress"
# define ADDRESS_CHECK_OKAY
#elif defined __GNUC__ || defined __clang__
/* There may be a warning.  */
#else
/* Ignore warnings from other compilers.  */
# define ADDRESS_CHECK_OKAY
#endif

#include <config.h>

#ifdef TEST_STDBOOL_H
# include <stdbool.h>
#endif

#if false
 "error: false is not 0"
#endif
#if true != 1
 "error: true is not 1"
#endif

/* Several tests cannot be guaranteed with gnulib's <stdbool.h>, at
   least, not for all compilers and compiler options.  */
#if ((HAVE_C_BOOL || defined __cplusplus \
      || HAVE_STDBOOL_H || 3 <= __GNUC__ || 4 <= __clang_major__) \
     && !(defined _MSC_VER || defined __SUNPRO_C))
# define WORKING_BOOL 1
#else
# define WORKING_BOOL 0
#endif

#if WORKING_BOOL
struct s { bool s: 1; bool t; } s;
#endif

char a[true == 1 ? 1 : -1];
char b[false == 0 ? 1 : -1];
#if WORKING_BOOL
char d[(bool) 0.5 == true ? 1 : -1];
# ifdef ADDRESS_CHECK_OKAY /* Avoid gcc warning.  */
/* C99 may plausibly be interpreted as not requiring support for a cast from
   a variable's address to bool in a static initializer.  So treat it like a
   GCC extension.  */
#  if defined __GNUC__ || defined __clang__
bool e = &s;
#  endif
# endif
char f[(bool) 0.0 == false ? 1 : -1];
#endif
char g[true];
char h[sizeof (bool)];
#if WORKING_BOOL
char i[sizeof s.t];
#endif
enum { j = false, k = true, l = false * true, m = true * 256 };
bool n[m];
char o[sizeof n == m * sizeof n[0] ? 1 : -1];
char p[-1 - (bool) 0 < 0 && -1 - (bool) 0 < 0 ? 1 : -1];
/* Catch a bug in an HP-UX C compiler.  See
   https://gcc.gnu.org/ml/gcc-patches/2003-12/msg02303.html
   https://lists.gnu.org/r/bug-coreutils/2005-11/msg00161.html
 */
bool q = true;
bool *pq = &q;

int
main ()
{
  int error = 0;

#if WORKING_BOOL
# ifdef ADDRESS_CHECK_OKAY /* Avoid gcc warning.  */
  /* A cast from a variable's address to bool is valid in expressions.  */
  {
    bool e1 = &s;
    if (!e1)
      error = 1;
  }
# endif
#endif

  /* Catch a bug in IBM AIX xlc compiler version 6.0.0.0
     reported by James Lemley on 2005-10-05; see
     https://lists.gnu.org/r/bug-coreutils/2005-10/msg00086.html
     This is a runtime test, since a corresponding compile-time
     test would rely on initializer extensions.  */
  {
    char digs[] = "0123456789";
    if (&(digs + 5)[-2 + (bool) 1] != &digs[4])
      error = 1;
  }

  return error;
}
