#include <stdio.h>

void
bar (void)
{
  printf ("DSO bar\n");
}

void
foo (void)
{
  bar ();
}
