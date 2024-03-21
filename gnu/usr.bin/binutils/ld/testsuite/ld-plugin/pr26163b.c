#include <stdio.h>

int counter;
extern void f(void);

void
real_g(void)
{
  counter++;
}

int main()
{
  real_g();
  f();
  if (counter == 3)
    printf ("PASS\n");
  return 0;
}
