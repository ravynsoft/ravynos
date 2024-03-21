#include <stdio.h>
#include <stdlib.h>

int foo;
void bar (void);

int
main ()
{
  if (foo != 0)
    abort ();
  foo = 200; 
  bar ();
  if (foo == 200)
    printf ("PASS\n");
  return 0;
}
