/* Test <stdckdint.h>.
   Copyright 2022-2023 Free Software Foundation, Inc.

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

/* Tell test-intprops.c to test <stdckdint.h> instead of <intprops.h>.  */

#define TEST_STDCKDINT 1

#define INT_ADD_WRAPV(a, b, r) ckd_add (r, a, b)
#define INT_SUBTRACT_WRAPV(a, b, r) ckd_sub (r, a, b)
#define INT_MULTIPLY_WRAPV(a, b, r) ckd_mul (r, a, b)

/* Luckily, test-intprops.c uses INT_NEGATE_OVERFLOW only on INT_MIN.  */
#define INT_NEGATE_OVERFLOW(a) ((a) < -INT_MAX)

#include "test-intprops.c"
