#include <stdlib.h>

extern int counter;

void
foo (void)
{
  counter++;
}

void
bar (void)
{
  abort ();
}
