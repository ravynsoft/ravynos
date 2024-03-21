#include <stdio.h>

extern void foo(void);
extern int i;

int main()
{
  foo();
  if (i == 0x1234)
    printf ("PASS\n");
  return 0;
}
