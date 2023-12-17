/* Test of <limits.h> substitute.
   Copyright 2016-2023 Free Software Foundation, Inc.

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

/* Written by Paul Eggert.  */

#include <config.h>

#include <limits.h>

#if 4 < __GNUC__ + (3 <= __GNUC_MINOR__)
# pragma GCC diagnostic ignored "-Woverlength-strings"
#endif

#define verify_width(width, min, max) \
  static_assert ((max) >> ((width) - 1 - ((min) < 0)) == 1)

/* Macros borrowed from intprops.h.  */
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
#define TYPE_WIDTH(t) (sizeof (t) * CHAR_BIT)
#define TYPE_MINIMUM(t) ((t) ~ TYPE_MAXIMUM (t))
#define TYPE_MAXIMUM(t)                                                 \
  ((t) (! TYPE_SIGNED (t)                                               \
        ? (t) -1                                                        \
        : ((((t) 1 << (TYPE_WIDTH (t) - 2)) - 1) * 2 + 1)))

/* Type width macros.  */

int type_bits[] =
  {
    CHAR_BIT,
    WORD_BIT,
    LONG_BIT
  };
verify_width (CHAR_BIT, CHAR_MIN, CHAR_MAX);
verify_width (WORD_BIT, INT_MIN, INT_MAX);
verify_width (LONG_BIT, LONG_MIN, LONG_MAX);

/* Numerical limit macros.  */

char               limits1[]  = { CHAR_MIN, CHAR_MAX };
static_assert (TYPE_MINIMUM (char) == CHAR_MIN);
static_assert (TYPE_MAXIMUM (char) == CHAR_MAX);

signed char        limits2[]  = { SCHAR_MIN, SCHAR_MAX };
static_assert (TYPE_MINIMUM (signed char) == SCHAR_MIN);
static_assert (TYPE_MAXIMUM (signed char) == SCHAR_MAX);

unsigned char      limits3[]  = { UCHAR_MAX };
static_assert (TYPE_MINIMUM (unsigned char) == 0);
static_assert (TYPE_MAXIMUM (unsigned char) == UCHAR_MAX);

short              limits4[]  = { SHRT_MIN, SHRT_MAX };
static_assert (TYPE_MINIMUM (short int) == SHRT_MIN);
static_assert (TYPE_MAXIMUM (short int) == SHRT_MAX);

unsigned short     limits5[]  = { USHRT_MAX };
static_assert (TYPE_MINIMUM (unsigned short int) == 0);
static_assert (TYPE_MAXIMUM (unsigned short int) == USHRT_MAX);

int                limits6[]  = { INT_MIN, INT_MAX };
static_assert (TYPE_MINIMUM (int) == INT_MIN);
static_assert (TYPE_MAXIMUM (int) == INT_MAX);

unsigned int       limits7[]  = { UINT_MAX };
static_assert (TYPE_MINIMUM (unsigned int) == 0);
static_assert (TYPE_MAXIMUM (unsigned int) == UINT_MAX);

long               limits8[]  = { LONG_MIN, LONG_MAX };
static_assert (TYPE_MINIMUM (long int) == LONG_MIN);
static_assert (TYPE_MAXIMUM (long int) == LONG_MAX);

unsigned long      limits9[]  = { ULONG_MAX };
static_assert (TYPE_MINIMUM (unsigned long int) == 0);
static_assert (TYPE_MAXIMUM (unsigned long int) == ULONG_MAX);

long long          limits10[] = { LLONG_MIN, LLONG_MAX };
static_assert (TYPE_MINIMUM (long long int) == LLONG_MIN);
static_assert (TYPE_MAXIMUM (long long int) == LLONG_MAX);

unsigned long long limits11[] = { ULLONG_MAX };
static_assert (TYPE_MINIMUM (unsigned long long int) == 0);
static_assert (TYPE_MAXIMUM (unsigned long long int) == ULLONG_MAX);

/* Specified by POSIX, not by ISO C.  */

long long limits12[] = { SSIZE_MAX };

/* Macros specified by C23 and by ISO/IEC TS 18661-1:2014.  */

verify_width (CHAR_WIDTH, CHAR_MIN, CHAR_MAX);
verify_width (SCHAR_WIDTH, SCHAR_MIN, SCHAR_MAX);
verify_width (UCHAR_WIDTH, 0, UCHAR_MAX);
verify_width (SHRT_WIDTH, SHRT_MIN, SHRT_MAX);
verify_width (USHRT_WIDTH, 0, USHRT_MAX);
verify_width (INT_WIDTH, INT_MIN, INT_MAX);
verify_width (UINT_WIDTH, 0, UINT_MAX);
verify_width (LONG_WIDTH, LONG_MIN, LONG_MAX);
verify_width (ULONG_WIDTH, 0, ULONG_MAX);
verify_width (LLONG_WIDTH, LLONG_MIN, LLONG_MAX);
verify_width (ULLONG_WIDTH, 0, ULLONG_MAX);

/* Macros specified by C23.  */

int bool_attrs[] = { BOOL_MAX, BOOL_WIDTH };
static_assert (BOOL_MAX == (((1U << (BOOL_WIDTH - 1)) - 1) * 2) + 1);

static_assert (0 < MB_LEN_MAX);

/* Get ssize_t, size_t.  */
#include <sys/types.h>

static_assert (TYPE_MAXIMUM (ssize_t) == SSIZE_MAX);
/* Verify that ssize_t has the same width as size_t.  */
static_assert (TYPE_MAXIMUM (size_t) / 2 == SSIZE_MAX);

int
main (void)
{
  return 0;
}
