#include <stdio.h>

extern int bar (void);

int times = -1;
int time1;

int
main ()
{
  printf ("times: %d\n", times);
  times = 20;
  printf ("times: %d\n", times);

  printf ("time1: %d\n", time1);
  time1 = 10;
  printf ("time1: %d\n", time1);
  bar ();

  return 0;
}
