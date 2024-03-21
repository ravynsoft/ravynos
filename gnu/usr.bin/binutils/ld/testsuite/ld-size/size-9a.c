#include <stdio.h>

char bar[20];
extern int bar_size;

int
main ()
{
  if (bar_size == 20)
    printf ("OK\n");

  return 0;
}
