#include <stdlib.h>

extern int counter;

void
foo (void)
{
  counter++;
}

__attribute__((weak))
void
bar (void)
{
  abort ();
}
