#include <stdio.h>

typedef void (*func_t) (void);

extern func_t get_foo (void);

void
foo (void)
{
}

int
main ()
{
  func_t p;

  foo ();
  p = get_foo ();
  p ();

  if (foo == p)
    printf ("PASS\n");

  return 0;
}
