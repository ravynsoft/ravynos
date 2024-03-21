/* Test the "verify" module.

   Copyright (C) 2005, 2009-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible.  */

#include <config.h>

#include "verify.h"

#ifndef EXP_FAIL
# define EXP_FAIL 0
#endif

/* ======================= Test verify, verify_expr ======================= */

int gx;
enum { A, B, C };

#if EXP_FAIL == 1
verify (gx >= 0);                 /* should give ERROR: non-constant expression */
#endif
verify (C == 2);                  /* should be ok */
#if EXP_FAIL == 2
verify (1 + 1 == 3);              /* should give ERROR */
#endif
verify (1 == 1); verify (1 == 1); /* should be ok */

enum
{
  item = verify_expr (1 == 1, 10 * 0 + 17) /* should be ok */
};

static int
function (int n)
{
#if EXP_FAIL == 3
  verify (n >= 0);                  /* should give ERROR: non-constant expression */
#endif
  verify (C == 2);                  /* should be ok */
#if EXP_FAIL == 4
  verify (1 + 1 == 3);              /* should give ERROR */
#endif
  verify (1 == 1); verify (1 == 1); /* should be ok */

  if (n)
    return ((void) verify_expr (1 == 1, 1), verify_expr (1 == 1, 8)); /* should be ok */
#if EXP_FAIL == 5
  return verify_expr (1 == 2, 5); /* should give ERROR */
#endif
  return 0;
}

/* ============================== Test assume ============================== */

static int
f (int a)
{
  return a;
}

typedef struct { unsigned int context : 4; unsigned int halt : 1; } state;

void test_assume_expressions (state *s);
int test_assume_optimization (int x);
_Noreturn void test_assume_noreturn (void);

void
test_assume_expressions (state *s)
{
  /* Check that 'assume' accepts a function call, even of a non-const
     function.  */
  assume (f (1));
  /* Check that 'assume' accepts a bit-field expression.  */
  assume (s->halt);
}

int
test_assume_optimization (int x)
{
  /* Check that the compiler uses 'assume' for optimization.
     This function, when compiled with optimization, should have code
     equivalent to
       return x + 3;
     Use 'objdump --disassemble test-verify.o' to verify this.  */
  assume (x >= 4);
  return (x > 1 ? x + 3 : 2 * x + 10);
}

_Noreturn void
test_assume_noreturn (void)
{
  /* Check that the compiler's data-flow analysis recognizes 'assume (0)'.
     This function should not elicit a warning.  */
  assume (0);
}

/* ============================== Main ===================================== */
int
main (void)
{
  state s = { 0, 1 };
  test_assume_expressions (&s);
  test_assume_optimization (5);
  return !(function (0) == 0 && function (1) == 8);
}
