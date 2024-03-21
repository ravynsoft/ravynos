#include <stdio.h>

void
bar (void)
{
  printf ("bar\n");
}

void
foo (long *x)
{
  (void) x;
  printf ("foo\n");
  bar ();
}
