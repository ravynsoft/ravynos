#include <stdio.h>

extern void foo(char);

void baz(int i)
{ 
  printf ("baz: %d\n", i);
}

int main(void)
{
  foo(42);
  return 0;
}
