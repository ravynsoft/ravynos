#include <stdio.h>

extern void foo (void);

void
bar (void)
{
  printf ("MAIN bar\n");
}

int
main (void)
{
  bar ();
  foo ();
  return 0;
}
