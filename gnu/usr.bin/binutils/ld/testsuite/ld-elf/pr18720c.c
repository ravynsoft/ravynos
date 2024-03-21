#include <stdio.h>

extern void foo (void);

void
foo (void)
{
  printf ("DSO\n");
}

void
bar (void)
{
  foo ();
}
