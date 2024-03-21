#include <stdlib.h>
#include <stdio.h>

extern int foo (void);

extern __thread int yyy;

__thread int zzz = 20;

int
main (void)
{
  if (foo () != zzz)
    abort ();
  if (yyy != 100)
    abort ();
  printf ("PASS\n");
  return 0;
}
