#include <stdio.h>

extern char _etext[];

int main(void)
{
  printf ("%p: %d\n", _etext, _etext[0]);
  return 0;
}
