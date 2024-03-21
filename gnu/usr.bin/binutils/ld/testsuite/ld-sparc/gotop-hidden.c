#include <stdio.h>

extern unsigned int foo (void);

__attribute__((visibility("hidden"))) unsigned int var = 0xdeadbeef;

int main (void)
{
  if (var == foo ())
    puts ("PASS");

  return 0;
}
