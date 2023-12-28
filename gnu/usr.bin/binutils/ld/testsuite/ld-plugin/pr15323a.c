#include <stdio.h>

int x;
int y;

__attribute__((weak))
void foobar (void) { y++; x++; }

int main (void)
{
  foobar ();
  if (y == 0)
    {
      if (x == -1)
	printf ("OK\n");
    }
  else
    {
      if (x == 1)
	printf ("OK\n");
    }
  return 0;
}
