#include <stdio.h>

extern void check_foo (void);
extern void check_bar (void);

int
main (void)
{
  check_foo ();
  check_bar ();

  printf ("OK\n");

  return 0;
}
