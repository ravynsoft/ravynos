#include <stdlib.h>
#include <stdio.h>

extern int foo_alias;
extern char *bar ();

int
main ()
{
  if (foo_alias != 0)
    abort ();
  bar ();
  if (foo_alias != -1)
    abort ();
  printf ("PASS\n");
  return 0;
}
