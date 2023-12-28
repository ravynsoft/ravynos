#include <stdio.h>

static int foo (int x) __attribute__ ((ifunc ("resolve_foo")));

static int foo_impl(int x)
{
  return x;
}

static void *resolve_foo (void)
{
  return (void *) foo_impl;
}

int
main ()
{
  foo (0);
  puts ("PASS");
  return 0;
}
