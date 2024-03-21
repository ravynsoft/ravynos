#include <stdio.h>

int foo[1];
int bar[2];

extern int *foo_p (void);
extern int *bar_p (void);

int
main ()
{
  if (foo[0] == 0 && foo == foo_p () && bar[0] == -1 && bar == bar_p ())
    printf ("PASS\n");
  return 0;
}
