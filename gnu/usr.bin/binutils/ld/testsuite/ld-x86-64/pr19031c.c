#include <stdio.h>

extern void *h (void);
extern void *g (void);

int
main (void)
{

  if (h () == g ())
    printf ("OK\n");

  return 0;
}
