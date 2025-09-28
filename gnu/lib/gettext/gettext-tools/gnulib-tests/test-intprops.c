/* Test intprops.h.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

/* Tell gcc not to warn about the long expressions that the overflow
   macros expand to, or about the (X < 0) expressions.  */
#if 4 < __GNUC__ + (3 <= __GNUC_MINOR__)
# pragma GCC diagnostic ignored "-Woverlength-strings"
# pragma GCC diagnostic ignored "-Wtype-limits"

/* Work around a bug in GCC 6.1 and earlier; see:
   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=68971  */
# pragma GCC diagnostic ignored "-Woverflow"

#endif

#include <config.h>

#ifdef TEST_STDCKDINT
# include <stdckdint.h>
#else
# include "intprops.h"
#endif

#include <inttypes.h>
#include <limits.h>

#include "macros.h"

/* Compile-time verification of expression X.
   In this file, we need it as a statement, rather than as a declaration.  */
#define verify_stmt(x) do { static_assert (x); } while (0)

/* VERIFY (X) uses a static assertion for compilers that are known to work,
   and falls back on a dynamic assertion for other compilers.
   But it ignores X if testing stdckdint.h.
   These tests should be checkable via 'verify' rather than 'ASSERT', but
   using 'verify' would run into a bug with HP-UX 11.23 cc; see
   <https://lists.gnu.org/r/bug-gnulib/2011-05/msg00401.html>.  */
#ifdef TEST_STDCKDINT
# define VERIFY(x) ((void) 0)
#elif __GNUC__ || __clang__ || __SUNPRO_C
# define VERIFY(x) verify_stmt (x)
#else
# define VERIFY(x) ASSERT (x)
#endif

#define DONTCARE __LINE__

int int_minus_2 = -2;
int int_1 = 1;

int
main (void)
{
  /* Use VERIFY for tests that must be integer constant expressions,
     ASSERT otherwise.  */

#ifndef TEST_STDCKDINT
  /* TYPE_IS_INTEGER.  */
  ASSERT (TYPE_IS_INTEGER (bool));
  ASSERT (TYPE_IS_INTEGER (char));
  ASSERT (TYPE_IS_INTEGER (signed char));
  ASSERT (TYPE_IS_INTEGER (unsigned char));
  ASSERT (TYPE_IS_INTEGER (short int));
  ASSERT (TYPE_IS_INTEGER (unsigned short int));
  ASSERT (TYPE_IS_INTEGER (int));
  ASSERT (TYPE_IS_INTEGER (unsigned int));
  ASSERT (TYPE_IS_INTEGER (long int));
  ASSERT (TYPE_IS_INTEGER (unsigned long int));
  ASSERT (TYPE_IS_INTEGER (intmax_t));
  ASSERT (TYPE_IS_INTEGER (uintmax_t));
  ASSERT (! TYPE_IS_INTEGER (float));
  ASSERT (! TYPE_IS_INTEGER (double));
  ASSERT (! TYPE_IS_INTEGER (long double));

  /* TYPE_SIGNED.  */
  /* VERIFY (! TYPE_SIGNED (bool)); // not guaranteed by gnulib substitute */
  VERIFY (TYPE_SIGNED (signed char));
  VERIFY (! TYPE_SIGNED (unsigned char));
  VERIFY (TYPE_SIGNED (short int));
  VERIFY (! TYPE_SIGNED (unsigned short int));
  VERIFY (TYPE_SIGNED (int));
  VERIFY (! TYPE_SIGNED (unsigned int));
  VERIFY (TYPE_SIGNED (long int));
  VERIFY (! TYPE_SIGNED (unsigned long int));
  VERIFY (TYPE_SIGNED (intmax_t));
  VERIFY (! TYPE_SIGNED (uintmax_t));
  ASSERT (TYPE_SIGNED (float));
  ASSERT (TYPE_SIGNED (double));
  ASSERT (TYPE_SIGNED (long double));

  /* Integer representation.  Check that it is two's complement.  */
  VERIFY (INT_MIN + INT_MAX < 0);

  /* TYPE_MINIMUM, TYPE_MAXIMUM.  */
  VERIFY (TYPE_MINIMUM (char) == CHAR_MIN);
  VERIFY (TYPE_MAXIMUM (char) == CHAR_MAX);
  VERIFY (TYPE_MINIMUM (unsigned char) == 0);
  VERIFY (TYPE_MAXIMUM (unsigned char) == UCHAR_MAX);
  VERIFY (TYPE_MINIMUM (signed char) == SCHAR_MIN);
  VERIFY (TYPE_MAXIMUM (signed char) == SCHAR_MAX);
  VERIFY (TYPE_MINIMUM (short int) == SHRT_MIN);
  VERIFY (TYPE_MAXIMUM (short int) == SHRT_MAX);
  VERIFY (TYPE_MINIMUM (unsigned short int) == 0);
  VERIFY (TYPE_MAXIMUM (unsigned short int) == USHRT_MAX);
  VERIFY (TYPE_MINIMUM (int) == INT_MIN);
  VERIFY (TYPE_MAXIMUM (int) == INT_MAX);
  VERIFY (TYPE_MINIMUM (unsigned int) == 0);
  VERIFY (TYPE_MAXIMUM (unsigned int) == UINT_MAX);
  VERIFY (TYPE_MINIMUM (long int) == LONG_MIN);
  VERIFY (TYPE_MAXIMUM (long int) == LONG_MAX);
  VERIFY (TYPE_MINIMUM (unsigned long int) == 0);
  VERIFY (TYPE_MAXIMUM (unsigned long int) == ULONG_MAX);
  #ifdef LLONG_MAX
   verify_stmt (TYPE_MINIMUM (long long int) == LLONG_MIN);
   verify_stmt (TYPE_MAXIMUM (long long int) == LLONG_MAX);
   verify_stmt (TYPE_MINIMUM (unsigned long long int) == 0);
   verify_stmt (TYPE_MAXIMUM (unsigned long long int) == ULLONG_MAX);
  #endif
  VERIFY (TYPE_MINIMUM (intmax_t) == INTMAX_MIN);
  VERIFY (TYPE_MAXIMUM (intmax_t) == INTMAX_MAX);
  VERIFY (TYPE_MINIMUM (uintmax_t) == 0);
  VERIFY (TYPE_MAXIMUM (uintmax_t) == UINTMAX_MAX);

  /* TYPE_WIDTH.  */
  #ifdef CHAR_WIDTH
   verify_stmt (TYPE_WIDTH (char) == CHAR_WIDTH);
   verify_stmt (TYPE_WIDTH (signed char) == SCHAR_WIDTH);
   verify_stmt (TYPE_WIDTH (unsigned char) == UCHAR_WIDTH);
   verify_stmt (TYPE_WIDTH (short int) == SHRT_WIDTH);
   verify_stmt (TYPE_WIDTH (unsigned short int) == USHRT_WIDTH);
   verify_stmt (TYPE_WIDTH (int) == INT_WIDTH);
   verify_stmt (TYPE_WIDTH (unsigned int) == UINT_WIDTH);
   verify_stmt (TYPE_WIDTH (long int) == LONG_WIDTH);
   verify_stmt (TYPE_WIDTH (unsigned long int) == ULONG_WIDTH);
   #ifdef LLONG_WIDTH
    verify_stmt (TYPE_WIDTH (long long int) == LLONG_WIDTH);
    verify_stmt (TYPE_WIDTH (unsigned long long int) == ULLONG_WIDTH);
   #endif
  #endif

  /* INT_BITS_STRLEN_BOUND.  */
  VERIFY (INT_BITS_STRLEN_BOUND (1) == 1);
  VERIFY (INT_BITS_STRLEN_BOUND (2620) == 789);

  /* INT_STRLEN_BOUND, INT_BUFSIZE_BOUND.  */
  #ifdef INT32_MAX /* POSIX guarantees int32_t; this ports to non-POSIX.  */
  VERIFY (INT_STRLEN_BOUND (int32_t) == sizeof ("-2147483648") - 1);
  VERIFY (INT_BUFSIZE_BOUND (int32_t) == sizeof ("-2147483648"));
  #endif
  #ifdef INT64_MAX
  VERIFY (INT_STRLEN_BOUND (int64_t) == sizeof ("-9223372036854775808") - 1);
  VERIFY (INT_BUFSIZE_BOUND (int64_t) == sizeof ("-9223372036854775808"));
  #endif
#endif

  /* All the INT_<op>_RANGE_OVERFLOW tests are equally valid as
     INT_<op>_OVERFLOW tests, so define macros to do both.  OP is the
     operation, OPNAME its symbolic name, A and B its operands, T the
     result type, V the overflow flag, and VRES the result if V and if
     two's complement.  CHECK_BINOP is for most binary operations,
     CHECK_SBINOP for binary +, -, * when the result type is signed,
     and CHECK_UNOP for unary operations.  */
  #define CHECK_BINOP(op, opname, a, b, t, v, vres)                       \
    VERIFY (INT_##opname##_RANGE_OVERFLOW (a, b, TYPE_MINIMUM (t),        \
                                           TYPE_MAXIMUM (t))              \
            == (v));                                                      \
    VERIFY (INT_##opname##_OVERFLOW (a, b) == (v))
  #define CHECK_SBINOP(op, opname, a, b, t, v, vres)                      \
    CHECK_BINOP(op, opname, a, b, t, v, vres);                            \
    {                                                                     \
      t result;                                                           \
      ASSERT (INT_##opname##_WRAPV (a, b, &result) == (v));               \
      ASSERT (result == ((v) ? (vres) : ((a) op (b))));                   \
    }
  #define CHECK_UNOP(op, opname, a, t, v)                                 \
    VERIFY (INT_##opname##_RANGE_OVERFLOW (a, TYPE_MINIMUM (t),           \
                                           TYPE_MAXIMUM (t))              \
            == (v));                                                      \
    VERIFY (INT_##opname##_OVERFLOW (a) == (v))

  /* INT_<op>_RANGE_OVERFLOW, INT_<op>_OVERFLOW.  */
  VERIFY (INT_ADD_RANGE_OVERFLOW (INT_MAX, 1, INT_MIN, INT_MAX));
  VERIFY (INT_ADD_OVERFLOW (INT_MAX, 1));

  CHECK_SBINOP (+, ADD, INT_MAX, 1, int, true, INT_MIN);
  CHECK_SBINOP (+, ADD, INT_MAX, -1, int, false, INT_MAX - 1);
  CHECK_SBINOP (+, ADD, INT_MIN, 1, int, false, INT_MIN + 1);
  CHECK_SBINOP (+, ADD, INT_MIN, -1, int, true, INT_MAX);
  CHECK_BINOP (+, ADD, UINT_MAX, 1u, unsigned int, true, 0u);
  CHECK_BINOP (+, ADD, 0u, 1u, unsigned int, false, 1u);

  CHECK_SBINOP (-, SUBTRACT, INT_MAX, 1, int, false, INT_MAX - 1);
  CHECK_SBINOP (-, SUBTRACT, INT_MAX, -1, int, true, INT_MIN);
  CHECK_SBINOP (-, SUBTRACT, INT_MIN, 1, int, true, INT_MAX);
  CHECK_SBINOP (-, SUBTRACT, INT_MIN, -1, int, false, INT_MIN - -1);
  CHECK_BINOP (-, SUBTRACT, UINT_MAX, 1u, unsigned int, false, UINT_MAX - 1u);
  CHECK_BINOP (-, SUBTRACT, 0u, 1u, unsigned int, true, 0u - 1u);

  CHECK_UNOP (-, NEGATE, INT_MIN, int, true);
  CHECK_UNOP (-, NEGATE, 0, int, false);
  CHECK_UNOP (-, NEGATE, INT_MAX, int, false);
  CHECK_UNOP (-, NEGATE, 0u, unsigned int, false);
  CHECK_UNOP (-, NEGATE, 1u, unsigned int, true);
  CHECK_UNOP (-, NEGATE, UINT_MAX, unsigned int, true);

  CHECK_SBINOP (*, MULTIPLY, INT_MAX, INT_MAX, int, true, 1);
  CHECK_SBINOP (*, MULTIPLY, INT_MAX, INT_MIN, int, true, INT_MIN);
  CHECK_SBINOP (*, MULTIPLY, INT_MIN, INT_MAX, int, true, INT_MIN);
  CHECK_SBINOP (*, MULTIPLY, INT_MIN, INT_MIN, int, true, 0);
  CHECK_SBINOP (*, MULTIPLY, -1, INT_MIN, int,
                INT_NEGATE_OVERFLOW (INT_MIN), INT_MIN);
#if !defined __HP_cc
  CHECK_SBINOP (*, MULTIPLY, LONG_MIN / INT_MAX, (long int) INT_MAX,
                long int, false, LONG_MIN - LONG_MIN % INT_MAX);
#endif

  CHECK_BINOP (/, DIVIDE, INT_MIN, -1, int,
               INT_NEGATE_OVERFLOW (INT_MIN), INT_MIN);
  CHECK_BINOP (/, DIVIDE, INT_MAX, 1, int, false, INT_MAX);
  CHECK_BINOP (/, DIVIDE, (unsigned int) INT_MIN, -1u, unsigned int,
               false, INT_MIN / -1u);

  CHECK_BINOP (%, REMAINDER, INT_MIN, -1, int, INT_NEGATE_OVERFLOW (INT_MIN), 0);
  CHECK_BINOP (%, REMAINDER, INT_MAX, 1, int, false, 0);
  CHECK_BINOP (%, REMAINDER, (unsigned int) INT_MIN, -1u, unsigned int,
               false, INT_MIN % -1u);

  CHECK_BINOP (<<, LEFT_SHIFT, UINT_MAX, 1, unsigned int, true, UINT_MAX << 1);
  CHECK_BINOP (<<, LEFT_SHIFT, UINT_MAX / 2 + 1, 1, unsigned int, true,
               (UINT_MAX / 2 + 1) << 1);
  CHECK_BINOP (<<, LEFT_SHIFT, UINT_MAX / 2, 1, unsigned int, false,
               (UINT_MAX / 2) << 1);

  /* INT_<op>_OVERFLOW and INT_<op>_WRAPV with mixed types.  */
  #define CHECK_SUM(a, b, t, v, vres)                                     \
    CHECK_SUM1 (a, b, t, v, vres);                                        \
    CHECK_SUM1 (b, a, t, v, vres)
  #define CHECK_SUM_WRAPV(a, b, t, v, vres, okres)                        \
    CHECK_SUM_WRAPV1 (a, b, t, v, vres, okres);                           \
    CHECK_SUM_WRAPV1 (b, a, t, v, vres, okres)
  #define CHECK_SUM1(a, b, t, v, vres)                                    \
    VERIFY (INT_ADD_OVERFLOW (a, b) == (v));                              \
    CHECK_SUM_WRAPV1 (a, b, t, v, vres, (a) + (b))
  #define CHECK_SUM_WRAPV1(a, b, t, v, vres, okres)                       \
    {                                                                     \
      t result;                                                           \
      ASSERT (INT_ADD_WRAPV (a, b, &result) == (v));                      \
      ASSERT (result == ((v) ? (vres) : (okres)));                        \
    }
  CHECK_SUM (-1, LONG_MIN, long int, true, LONG_MAX);
  CHECK_SUM (-1, UINT_MAX, unsigned int, false, DONTCARE);
  CHECK_SUM (-1L, INT_MIN, long int, INT_MIN == LONG_MIN,
              INT_MIN == LONG_MIN ? INT_MAX : DONTCARE);
  CHECK_SUM (0u, -1, unsigned int, true, 0u + -1);
  CHECK_SUM (0u, 0, unsigned int, false, DONTCARE);
  CHECK_SUM (0u, 1, unsigned int, false, DONTCARE);
  CHECK_SUM (1, LONG_MAX, long int, true, LONG_MIN);
  CHECK_SUM (1, UINT_MAX, unsigned int, true, 0u);
  CHECK_SUM (1L, INT_MAX, long int, INT_MAX == LONG_MAX,
              INT_MAX == LONG_MAX ? INT_MIN : DONTCARE);
  CHECK_SUM (1u, INT_MAX, unsigned int, INT_MAX == UINT_MAX, 1u + INT_MAX);
  CHECK_SUM (1u, INT_MIN, unsigned int, true, 1u + INT_MIN);
  CHECK_SUM_WRAPV (-1, 1u, int, false, DONTCARE, 0);
  CHECK_SUM_WRAPV (-1, 1ul, int, false, DONTCARE, 0);
  CHECK_SUM_WRAPV (-1l, 1u, int, false, DONTCARE, 0);
  CHECK_SUM_WRAPV (-100, 1000u, int, false, DONTCARE, 900);
  CHECK_SUM_WRAPV (INT_MIN, UINT_MAX, int, false, DONTCARE, INT_MAX);
  CHECK_SUM_WRAPV (1u, INT_MAX, int, true, INT_MIN, DONTCARE);
  CHECK_SUM_WRAPV (INT_MAX, 1, long int, LONG_MAX <= INT_MAX, INT_MIN,
                   INT_MAX + 1L);
  CHECK_SUM_WRAPV (UINT_MAX, 1, long int, LONG_MAX <= UINT_MAX, 0,
                   UINT_MAX + 1L);
  CHECK_SUM_WRAPV (INT_MAX, 1, unsigned long int, ULONG_MAX <= INT_MAX, 0,
                   INT_MAX + 1uL);
  CHECK_SUM_WRAPV (UINT_MAX, 1, unsigned long int, ULONG_MAX <= UINT_MAX, 0,
                   UINT_MAX + 1uL);

  {
    long int result;
    ASSERT (INT_ADD_WRAPV (1, INT_MAX, &result) == (INT_MAX == LONG_MAX));
    ASSERT (INT_ADD_WRAPV (-1, INT_MIN, &result) == (INT_MIN == LONG_MIN));
  }

  #define CHECK_DIFFERENCE(a, b, t, v, vres)                              \
    VERIFY (INT_SUBTRACT_OVERFLOW (a, b) == (v))
  #define CHECK_SDIFFERENCE(a, b, t, v, vres)                             \
    CHECK_DIFFERENCE (a, b, t, v, vres);                                  \
    CHECK_SDIFFERENCE_WRAPV (a, b, t, v, vres)
  #define CHECK_SDIFFERENCE_WRAPV(a, b, t, v, vres)                       \
    {                                                                     \
      t result;                                                           \
      ASSERT (INT_SUBTRACT_WRAPV (a, b, &result) == (v));                 \
      ASSERT (result == ((v) ? (vres) : ((a) - (b))));                    \
    }
  CHECK_DIFFERENCE (INT_MAX, 1u, unsigned int, UINT_MAX < INT_MAX - 1,
                    INT_MAX - 1u);
  CHECK_DIFFERENCE (UINT_MAX, 1, unsigned int, false, UINT_MAX - 1);
  CHECK_DIFFERENCE (0u, -1, unsigned int, false, 0u - -1);
  CHECK_DIFFERENCE (UINT_MAX, -1, unsigned int, true, UINT_MAX - -1);
  CHECK_DIFFERENCE (INT_MIN, 1u, unsigned int, true, INT_MIN - 1u);
  CHECK_DIFFERENCE (-1, 0u, unsigned int, true, -1 - 0u);
  CHECK_SDIFFERENCE (-1, INT_MIN, int, false, -1 - INT_MIN);
  CHECK_SDIFFERENCE (-1, INT_MAX, int, false, -1 - INT_MAX);
  CHECK_SDIFFERENCE (0, INT_MIN, int, INT_MIN < -INT_MAX, INT_MIN);
  CHECK_SDIFFERENCE (0, INT_MAX, int, false, 0 - INT_MAX);
  CHECK_SDIFFERENCE_WRAPV (-1, 1u, int, false, DONTCARE);
  CHECK_SDIFFERENCE_WRAPV (-1, 1ul, int, false, DONTCARE);
  CHECK_SDIFFERENCE_WRAPV (-1l, 1u, int, false, DONTCARE);
  CHECK_SDIFFERENCE_WRAPV (0u, INT_MAX, int, false, DONTCARE);
  CHECK_SDIFFERENCE_WRAPV (1u, INT_MIN, int, true, 1u - INT_MIN);
  {
    long int result;
    ASSERT (INT_SUBTRACT_WRAPV (INT_MAX, -1, &result) == (INT_MAX == LONG_MAX));
    ASSERT (INT_SUBTRACT_WRAPV (INT_MIN, 1, &result) == (INT_MAX == LONG_MAX));
  }

  #define CHECK_PRODUCT(a, b, t, v, vres)                                 \
    CHECK_PRODUCT1 (a, b, t, v, vres);                                    \
    CHECK_PRODUCT1 (b, a, t, v, vres)
  #define CHECK_SPRODUCT(a, b, t, v, vres)                                \
    CHECK_SPRODUCT1 (a, b, t, v, vres);                                   \
    CHECK_SPRODUCT1 (b, a, t, v, vres)
  #define CHECK_SPRODUCT_WRAPV(a, b, t, v, vres)                          \
    CHECK_SPRODUCT_WRAPV1 (a, b, t, v, vres);                             \
    CHECK_SPRODUCT_WRAPV1 (b, a, t, v, vres)
  #define CHECK_PRODUCT1(a, b, t, v, vres)                                \
    VERIFY (INT_MULTIPLY_OVERFLOW (a, b) == (v))
  #define CHECK_SPRODUCT1(a, b, t, v, vres)                               \
    CHECK_PRODUCT1 (a, b, t, v, vres);                                    \
    CHECK_SPRODUCT_WRAPV1 (a, b, t, v, vres)
  #define CHECK_SPRODUCT_WRAPV1(a, b, t, v, vres)                         \
    {                                                                     \
      t result;                                                           \
      ASSERT (INT_MULTIPLY_WRAPV (a, b, &result) == (v));                 \
      ASSERT (result == ((v) ? (vres) : ((a) * (b))));                    \
    }
  CHECK_PRODUCT (-1, 1u, unsigned int, true, -1 * 1u);
  CHECK_SPRODUCT (-1, INT_MIN, int, INT_NEGATE_OVERFLOW (INT_MIN), INT_MIN);
  CHECK_PRODUCT (-1, UINT_MAX, unsigned int, true, -1 * UINT_MAX);
  CHECK_SPRODUCT (-32768, LONG_MAX / -32768 - 1, long int, true, LONG_MIN);
  CHECK_SPRODUCT (-12345, LONG_MAX / -12345, long int, false, DONTCARE);
  CHECK_SPRODUCT (0, -1, int, false, DONTCARE);
  CHECK_SPRODUCT (0, 0, int, false, DONTCARE);
  CHECK_PRODUCT (0, 0u, unsigned int, false, DONTCARE);
  CHECK_SPRODUCT (0, 1, int, false, DONTCARE);
  CHECK_SPRODUCT (0, INT_MAX, int, false, DONTCARE);
  CHECK_SPRODUCT (0, INT_MIN, int, false, DONTCARE);
  CHECK_PRODUCT (0, UINT_MAX, unsigned int, false, DONTCARE);
  CHECK_PRODUCT (0u, -1, unsigned int, false, DONTCARE);
  CHECK_PRODUCT (0u, 0, unsigned int, false, DONTCARE);
  CHECK_PRODUCT (0u, 0u, unsigned int, false, DONTCARE);
  CHECK_PRODUCT (0u, 1, unsigned int, false, DONTCARE);
  CHECK_PRODUCT (0u, INT_MAX, unsigned int, false, DONTCARE);
  CHECK_PRODUCT (0u, INT_MIN, unsigned int, false, DONTCARE);
  CHECK_PRODUCT (0u, UINT_MAX, unsigned int, false, DONTCARE);
  CHECK_SPRODUCT (1, INT_MAX, int, false, DONTCARE);
  CHECK_SPRODUCT (1, INT_MIN, int, false, DONTCARE);
  CHECK_PRODUCT (1, UINT_MAX, unsigned int, false, DONTCARE);
  CHECK_PRODUCT (1u, INT_MIN, unsigned int, true, 1u * INT_MIN);
  CHECK_PRODUCT (1u, INT_MAX, unsigned int, UINT_MAX < INT_MAX, 1u * INT_MAX);
  CHECK_PRODUCT (INT_MAX, UINT_MAX, unsigned int, true, INT_MAX * UINT_MAX);
  CHECK_PRODUCT (INT_MAX, ULONG_MAX, unsigned long int, true,
                 INT_MAX * ULONG_MAX);
#if !defined __HP_cc
  CHECK_SPRODUCT (INT_MIN, LONG_MAX / INT_MIN - 1, long int, true, LONG_MIN);
  CHECK_SPRODUCT (INT_MIN, LONG_MAX / INT_MIN, long int, false, DONTCARE);
#endif
  CHECK_PRODUCT (INT_MIN, UINT_MAX, unsigned int, true, INT_MIN * UINT_MAX);
  CHECK_PRODUCT (INT_MIN, ULONG_MAX, unsigned long int, true,
                 INT_MIN * ULONG_MAX);
  CHECK_SPRODUCT_WRAPV (-1, INT_MAX + 1u, int, false, DONTCARE);
  CHECK_SPRODUCT_WRAPV (-1, 1u, int, false, DONTCARE);
  CHECK_SPRODUCT (0, ULONG_MAX, int, false, DONTCARE);
  CHECK_SPRODUCT (0u, LONG_MIN, int, false, DONTCARE);
  {
    long int result;
    ASSERT (INT_MULTIPLY_WRAPV (INT_MAX, INT_MAX, &result)
            == (LONG_MAX / INT_MAX < INT_MAX));
    ASSERT (INT_MULTIPLY_WRAPV (INT_MAX, INT_MAX, &result)
            || result == INT_MAX * (long int) INT_MAX);
    ASSERT (INT_MULTIPLY_WRAPV (INT_MIN, INT_MIN, &result)
            || result == INT_MIN * (long int) INT_MIN);
  }

# ifdef LLONG_MAX
  {
    long long int result;
    ASSERT (INT_MULTIPLY_WRAPV (LONG_MAX, LONG_MAX, &result)
            == (LLONG_MAX / LONG_MAX < LONG_MAX));
    ASSERT (INT_MULTIPLY_WRAPV (LONG_MAX, LONG_MAX, &result)
            || result == LONG_MAX * (long long int) LONG_MAX);
    ASSERT (INT_MULTIPLY_WRAPV (LONG_MIN, LONG_MIN, &result)
            || result == LONG_MIN * (long long int) LONG_MIN);
  }
# endif

  /* Check for GCC bug 91450.  */
  {
    unsigned long long result;
    ASSERT (INT_MULTIPLY_WRAPV (int_minus_2, int_1, &result) && result == -2);
  }

  #define CHECK_QUOTIENT(a, b, v) VERIFY (INT_DIVIDE_OVERFLOW (a, b) == (v))

  CHECK_QUOTIENT (INT_MIN, -1L, INT_MIN == LONG_MIN);
  CHECK_QUOTIENT (INT_MIN, UINT_MAX, false);
  CHECK_QUOTIENT (INTMAX_MIN, UINTMAX_MAX, false);
  CHECK_QUOTIENT (INTMAX_MIN, UINT_MAX, false);
  CHECK_QUOTIENT (-11, 10u, true);
  CHECK_QUOTIENT (-10, 10u, true);
  CHECK_QUOTIENT (-9, 10u, false);
  CHECK_QUOTIENT (11u, -10, true);
  CHECK_QUOTIENT (10u, -10, true);
  CHECK_QUOTIENT (9u, -10, false);

  #define CHECK_REMAINDER(a, b, v) VERIFY (INT_REMAINDER_OVERFLOW (a, b) == (v))

  CHECK_REMAINDER (INT_MIN, -1L, INT_MIN == LONG_MIN);
  CHECK_REMAINDER (-1, UINT_MAX, true);
  CHECK_REMAINDER ((intmax_t) -1, UINTMAX_MAX, true);
  CHECK_REMAINDER (INTMAX_MIN, UINT_MAX,
                   (INTMAX_MAX < UINT_MAX
                    && - (unsigned int) INTMAX_MIN % UINT_MAX != 0));
  CHECK_REMAINDER (INT_MIN, ULONG_MAX, INT_MIN % ULONG_MAX != 1);
  CHECK_REMAINDER (1u, -1, false);
  CHECK_REMAINDER (37*39u, -39, false);
  CHECK_REMAINDER (37*39u + 1, -39, true);
  CHECK_REMAINDER (37*39u - 1, -39, true);
  CHECK_REMAINDER (LONG_MAX, -INT_MAX, false);

  return 0;
}
