#include <stdio.h>

extern int bar_size;

int
main ()
{
  if (bar_size == 10)
    printf ("OK\n");

  return 0;
}
