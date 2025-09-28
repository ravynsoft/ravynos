#include <stdio.h>

static int
one (void)
{
  return -30;
}

int foo (void) __attribute__ ((ifunc ("resolve_foo")));

void *
resolve_foo (void)
{
  return (void *) one;
}

typedef int (*foo_p) (void);

foo_p foo_ptr = foo;

foo_p
__attribute__ ((noinline))
get_foo_p (void)
{
  return foo_ptr;
}

foo_p
__attribute__ ((noinline))
get_foo (void)
{
  return foo;
}

int
main (void)
{
  foo_p p;

  p = get_foo ();
  if (p != foo)
    __builtin_abort ();
  if ((*p) () != -30)
    __builtin_abort ();

  p = get_foo_p ();
  if (p != foo)
    __builtin_abort ();
  if ((*p) () != -30)
    __builtin_abort ();

  if (foo_ptr != foo)
    __builtin_abort ();
  if ((*foo_ptr) () != -30)
    __builtin_abort ();
  if (foo () != -30)
    __builtin_abort ();

  printf ("PASS\n");

  return 0;
}
