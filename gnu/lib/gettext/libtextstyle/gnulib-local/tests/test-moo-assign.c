/* Test assignments.
   Errors are signalled in C++ mode only, not by a C compiler.  */
#include "test-moo-sub2.h"

void foo ()
{
  root_t a;
  sub1_t b;
  sub2_t c;

  /* Test initializations.  */
  root_t a1 = a;
  root_t a2 = b;
  root_t a3 = c;
  sub1_t b1 = a;        /* ERROR */
  sub1_t b2 = b;
  sub1_t b3 = c;
  sub2_t c1 = a;        /* ERROR */
  sub2_t c2 = b;        /* ERROR */
  sub2_t c3 = c;

  /* Test assignments.  */
  a = a1;
  a = b1;
  a = c1;
  b = a2;       /* ERROR */
  b = b2;
  b = c2;
  c = a3;       /* ERROR */
  c = b3;       /* ERROR */
  c = c3;
}
