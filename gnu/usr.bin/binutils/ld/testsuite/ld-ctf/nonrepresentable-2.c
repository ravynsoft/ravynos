#include <complex.h>

extern int foo (complex int);

int main (void)
{
  complex int a = 33.4;
  foo (a);

  return 1;
}
