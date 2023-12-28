#include <stdio.h>

extern int copy;
extern int get_copy (void);

int
main ()
{
  if (copy == get_copy ())
    printf ("PASS\n");

  return 0;
}
