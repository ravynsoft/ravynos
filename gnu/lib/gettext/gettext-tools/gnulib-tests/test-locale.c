/* Test of <locale.h> substitute.
   Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.

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

#include <locale.h>

int a[] =
  {
    LC_ALL,
    LC_COLLATE,
    LC_CTYPE,
    LC_MESSAGES,
    LC_MONETARY,
    LC_NUMERIC,
    LC_TIME
  };

/* Check that the 'struct lconv' type is defined.  */
struct lconv l;
int ls;

/* Check that NULL can be passed through varargs as a pointer type,
   per POSIX 2008.  */
static_assert (sizeof NULL == sizeof (void *));

int
main ()
{
#if HAVE_WORKING_NEWLOCALE
  /* Check that the locale_t type and the LC_GLOBAL_LOCALE macro are defined.  */
  locale_t b = LC_GLOBAL_LOCALE;
  (void) b;
#endif

  /* Check that 'struct lconv' has the ISO C and POSIX specified members.  */
  ls += sizeof (*l.decimal_point);
  ls += sizeof (*l.thousands_sep);
  ls += sizeof (*l.grouping);
  ls += sizeof (*l.mon_decimal_point);
  ls += sizeof (*l.mon_thousands_sep);
  ls += sizeof (*l.mon_grouping);
  ls += sizeof (*l.positive_sign);
  ls += sizeof (*l.negative_sign);
  ls += sizeof (*l.currency_symbol);
  ls += sizeof (l.frac_digits);
  ls += sizeof (l.p_cs_precedes);
  ls += sizeof (l.p_sign_posn);
  ls += sizeof (l.p_sep_by_space);
  ls += sizeof (l.n_cs_precedes);
  ls += sizeof (l.n_sign_posn);
  ls += sizeof (l.n_sep_by_space);
  ls += sizeof (*l.int_curr_symbol);
  ls += sizeof (l.int_frac_digits);
  ls += sizeof (l.int_p_cs_precedes);
  ls += sizeof (l.int_p_sign_posn);
  ls += sizeof (l.int_p_sep_by_space);
  ls += sizeof (l.int_n_cs_precedes);
  ls += sizeof (l.int_n_sign_posn);
  ls += sizeof (l.int_n_sep_by_space);

  return 0;
}
