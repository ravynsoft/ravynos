#include <stdio.h>

void
foo (void)
{
  printf ("OK\n");
}

void *
bar (void)
{
  foo ();
  return &foo;
}
