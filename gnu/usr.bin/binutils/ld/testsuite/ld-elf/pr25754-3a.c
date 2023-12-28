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
  if ((uintptr_t) get_bar () == -0x80000001LL)
    printf ("PASS\n");
  return 0;
}
