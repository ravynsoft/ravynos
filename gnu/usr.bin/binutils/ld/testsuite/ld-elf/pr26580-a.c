#include <stdio.h>

extern void __attribute__ ((weak)) foo (void);

char x, y, z;

long
lowest_align (void *a, void *b, void *c)
{
  unsigned long bits = (long) a | (long) b | (long) c;
  return bits & -bits;
}

int
main (void)
{
  printf ("library %sloaded\n", &foo ? "" : "not ");
  printf ("alignment %ld\n", lowest_align (&x, &y, &z));
  return 0;
}
