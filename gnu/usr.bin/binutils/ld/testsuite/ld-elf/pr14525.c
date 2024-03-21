#include <stdio.h>

extern void *__executable_start;
 
int
main()
{
  if ((void **) &main >= &__executable_start)
    printf ("OK\n");
  return 0;
}
