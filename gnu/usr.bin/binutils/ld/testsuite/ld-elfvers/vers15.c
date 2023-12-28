/*
 * Testcase to make sure that if we externally reference a versioned symbol
 * that we always get the right one.
 */
#include <stdio.h>
#include "vers.h"

int
foo_1()
{
  return 1034;
}

int
foo_2()
{
  return 1343;
}

int
foo_3()
{
  return 1334;
}

int
main()
{
  printf("Expect 4,    get %d\n", foo_1());
  printf("Expect 13,   get %d\n", foo_2());
  printf("Expect 103,  get %d\n", foo_3());
  return 0;
}

FUNC_SYMVER(foo_1, show_foo@);
FUNC_SYMVER(foo_2, show_foo@VERS_1.1);
FUNC_SYMVER(foo_3, show_foo@@VERS_1.2);
