#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef ALIGN
# define ALIGN 0x800000
#endif

int
__attribute__ ((weak))
is_aligned (void *p, int align)
{
  return (((uintptr_t) p) & (align - 1)) == 0;
}

int foo __attribute__ ((aligned (ALIGN))) = 1;

int
main (void)
{
  if (!is_aligned (&foo, ALIGN))
    abort ();
  printf ("PASS\n");
  return 0;
}
