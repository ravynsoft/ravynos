#include <stdio.h>
#include <stdint.h>

extern uintptr_t bar;

uintptr_t *
__attribute__ ((noinline, noclone))
get_bar (void)
{
  return &bar;
}

int
main ()
{
  if ((uintptr_t) get_bar () == 0xfffffff0U)
    printf ("PASS\n");
  return 0;
}
#include <stdio.h>
