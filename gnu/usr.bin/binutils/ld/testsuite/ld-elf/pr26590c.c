#include <stdio.h>

extern int select ();
extern int f2 (int);

int main (void)
{
  if (select () == 0 && f2 (0) == 22222)
    printf ("PASS\n");
  return 0;
}
