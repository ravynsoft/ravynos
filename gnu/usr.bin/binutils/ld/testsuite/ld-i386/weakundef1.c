#include <stdlib.h>
#include <stdio.h>

extern int visibility_var_weak
  __attribute__ ((weak, visibility ("hidden")));

int
main ()
{
  if (&visibility_var_weak != NULL)
    abort ();

  printf ("PASS\n");

  return 0;
}
