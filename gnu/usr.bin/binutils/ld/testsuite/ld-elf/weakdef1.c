#include <stdio.h>
#include <stdlib.h>

extern int foo_alias;
extern void bar (void);

int
main (void)
{
  bar ();
  if (foo_alias != -1)
    abort ();
  printf ("PASS\n");
  return 0;
}
