#include <stdio.h>

__attribute__ ((weak))
void
foo (void)
{
  printf ("pr19553d\n");
}
