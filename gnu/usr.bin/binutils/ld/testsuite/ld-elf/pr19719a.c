#include <stdio.h>

extern int foo (void);
extern int bar (void);

int
main (void)
{
  if (foo () == bar ())
    printf ("PASS\n");
  return 0;
}
