#include <stdio.h>

extern void y (void);

void
x (void)
{
  printf ("PASS\n");
}


int
main (void)
{
  y ();
  return 0;
}
