#include <stdlib.h>

extern int foo;
int foo2 = 2;

void
bar (void)
{
  if (foo != 30)
    abort ();
}
