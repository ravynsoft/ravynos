#include <stdio.h>

extern int cook(void);

int __wrap_cook(void)
{
  puts ("PASS");
  return 0;
}

int main()
{
  if (cook () == -1)
    __builtin_abort ();

  return 0;
}
