#include <stdio.h>

extern int foo(void);
extern int bar(void);

typedef int (*func_p) (void);

func_p foo_ptr = foo;

func_p
__attribute__((noinline))
get_bar (void)
{
  return bar;
}

int
main (void)
{
  func_p bar_ptr = get_bar ();
  if (bar_ptr != bar)
    __builtin_abort ();
  if (bar_ptr() != -1)
    __builtin_abort ();
  if (bar() != -1)
    __builtin_abort ();

  if (foo_ptr != foo)
    __builtin_abort ();
  if (foo_ptr() != 1)
    __builtin_abort ();
  if (foo() != 1)
    __builtin_abort ();

  printf ("OK\n");

  return 0;
}
