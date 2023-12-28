#include <stdio.h>

int foo = 1;

extern int *bar (void);
extern int bar_ifunc (void);

int
main (void)
{
  if (bar () == &foo && bar_ifunc () == 0xbadbeef)
    printf ("PASS\n");
  return 0;
}
