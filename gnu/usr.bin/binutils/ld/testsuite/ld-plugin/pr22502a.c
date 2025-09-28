#include <stdio.h>

volatile int x;
extern void abort ();

__attribute__((weak))
void foobar (void) { x++; }

int main (void)
{
  foobar ();
  if (x != -1)
    abort ();
  printf ("PASS\n");
  return 0;
}
