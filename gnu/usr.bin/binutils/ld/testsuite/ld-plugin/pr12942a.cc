#include <stdio.h>
#include "pr12942a.h"

test_t b(void);

int
main(void)
{
  if (test != b ())
    __builtin_abort ();

  printf ("OK\n");
  return 0;
}
